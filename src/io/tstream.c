
#include "stdafx.h"

#include "tstream.h"

#include "talloc.h"
#include "terror.h"
#include "tthread.h"

#include "utility/tfilesys.h"

#ifdef PLATFORM_X86_64
#	ifdef _WINDOWS
#		define fseek _fseeki64
#		define ftell _ftelli64
#	else
#		define fseek fseeko
#		define ftell ftello
#	endif
#endif

//--- Default File Operations ------------------------------//

#define TStream_READ_ONLY_FLAG 1

struct _TStream {
    TStreamOps operations;
    TStreamContent content;
};

struct TStreamFile {
    FILE *f;
    unsigned char autoclose;
    TSize offset, size;
};

static TSize TStreamFileSize(TStreamContent *content)
{
    struct TStreamFile *streamFile;
	if (!content) return 0;
	streamFile = (struct TStreamFile *) content;

	if (streamFile->size)
		return streamFile->size - streamFile->offset;

	return 0;
}

static int TStreamFileSeek(TStreamContent *content, TLInt offset, int origin)
{
	struct TStreamFile *streamFile;
	if (!content) return 0;
	streamFile = (struct TStreamFile *) content;

	if (origin == SEEK_CUR) {
        streamFile->offset += offset;
	} else if (origin == SEEK_END) {
		if ((TSize)offset > streamFile->size) offset = streamFile->size;
		streamFile->offset = streamFile->size - offset;
    } else {
        streamFile->offset = offset;
    }
    streamFile->offset = TCLAMP(streamFile->offset, 0, streamFile->size);

    return fseek(streamFile->f, offset, origin);
}

static TLInt TStreamFileTell(TStreamContent *content)
{
	struct TStreamFile *streamFile;
	if (!content) return 0;
	streamFile = (struct TStreamFile *) content;

    return ftell(streamFile->f);
}

static unsigned char TStreamFileEOF(TStreamContent *content)
{
	struct TStreamFile *streamFile;
	if (!content) return 0;
	streamFile = (struct TStreamFile *) content;

    return feof(streamFile->f);
}

static TSize TStreamFileRead(TStreamContent *content, TPtr buffer, TSize size)
{
    int read;
	struct TStreamFile *streamFile;
	if (!content) return 0;
	streamFile = (struct TStreamFile *) content;

    if(feof(streamFile->f)) return 0;

    read = fread(buffer, 1, size, streamFile->f);
    streamFile->offset += read;

    return read;
}

static TSize TStreamFileWrite(TStreamContent *content, TCPtr buffer, TSize size)
{
    int written = 0;
	struct TStreamFile *streamFile;
	if (!content) return 0;
	streamFile = (struct TStreamFile *) content;

    written = fwrite(buffer, 1, size, streamFile->f);

    if(written) {
        streamFile->offset += written;
        if(streamFile->offset > streamFile->size)
            streamFile->size = streamFile->offset;
    }

    return written;
}

static int TStreamFileClose(TStreamContent *content)
{
	struct TStreamFile *streamFile;
	if (!content) return 0;
	streamFile = (struct TStreamFile *) content;

    if(streamFile->autoclose)
        return fclose(streamFile->f);

    return 0;
}

static TStreamOps TStreamFileOps = {
    TStreamFileSize,
    TStreamFileSeek,
    TStreamFileTell,
    TStreamFileEOF,
    TStreamFileRead,
    TStreamFileWrite,
    TStreamFileClose,
};

//--- Default Buffer Operations ------------------------------//

struct TStreamBuffer {
    unsigned char *buffer;
    TSize size;
    TSize offset;

    unsigned char autofree;
};

static TSize TStreamBufferSize(TStreamContent *content)
{
    struct TStreamBuffer *streamBuffer;
    if(!content) return 0;
	streamBuffer = (struct TStreamBuffer *) content;

    return streamBuffer->size - streamBuffer->offset;
}

static int TStreamBufferSeek(TStreamContent *content, TLInt offset, int origin)
{
	struct TStreamBuffer *streamBuffer;
	if (!content) return 0;
	streamBuffer = (struct TStreamBuffer *) content;

	if (origin == SEEK_CUR) {
        streamBuffer->offset += offset;
	} else if (origin == SEEK_END) {
		if ((TSize)offset > streamBuffer->size) offset = streamBuffer->size;
        streamBuffer->offset = streamBuffer->size - offset;
    } else {
        streamBuffer->offset = offset;
    }
    streamBuffer->offset = TCLAMP(streamBuffer->offset, 0, streamBuffer->size);

    return 0;
}

static TLInt TStreamBufferTell(TStreamContent *content)
{
	struct TStreamBuffer *streamBuffer;
	if (!content) return 0;
	streamBuffer = (struct TStreamBuffer *) content;

    return streamBuffer->offset;
}

static unsigned char TStreamBufferEOF(TStreamContent *content)
{
	struct TStreamBuffer *streamBuffer;
	if (!content) return 0;
	streamBuffer = (struct TStreamBuffer *) content;

    return streamBuffer->offset >= streamBuffer->size;
}

static TSize TStreamBufferRead(TStreamContent *content, TPtr buffer, TSize size)
{
    TSize cpysize;
	struct TStreamBuffer *streamBuffer;
	if (!content) return 0;
	streamBuffer = (struct TStreamBuffer *) content;

    if(streamBuffer->offset >= streamBuffer->size) return 0;

    cpysize = TMIN(size, (TSize) sizeof(unsigned char) * (streamBuffer->size - streamBuffer->offset));
    memcpy(buffer,streamBuffer->buffer + streamBuffer->offset, cpysize);
    streamBuffer->offset += cpysize + 1;

    return cpysize;
}

static TSize TStreamBufferWrite(TStreamContent *content, TCPtr buffer, TSize size)
{
	struct TStreamBuffer *streamBuffer;
	if (!content) return 0;
	streamBuffer = (struct TStreamBuffer *) content;

    if (streamBuffer->offset + size > streamBuffer->size) {
        //attempt to increase buffer size
        unsigned char *newBuffer = TRAlloc(streamBuffer->buffer, streamBuffer->offset + size);
        if (newBuffer) streamBuffer->buffer = newBuffer;
		else size = (TSize) sizeof(unsigned char) * (streamBuffer->size - streamBuffer->offset);
    }

    memcpy(streamBuffer->buffer + streamBuffer->offset, buffer, size);
    streamBuffer->offset = TCLAMP(size + 1, 0, streamBuffer->size);

    return size;
}

static int TStreamBufferClose(TStreamContent *content)
{
	struct TStreamBuffer *streamBuffer;
	if (!content) return 0;
	streamBuffer = (struct TStreamBuffer *) content;

    if (streamBuffer->autofree)
        TFree(streamBuffer->buffer);

    return 0;
}

static TStreamOps TStreamBufferOps = {
    TStreamBufferSize,
    TStreamBufferSeek,
    TStreamBufferTell,
    TStreamBufferEOF,
    TStreamBufferRead,
    TStreamBufferWrite,
    TStreamBufferClose,
};

//--- Terra Read Write ------------------------------//

enum TStreamTypes {
    TStream_UNKNOWN = 0,
    TStream_FILE,
    TStream_BUFFER,
};

TStream *TStreamFromFile(const char *filename, const char *mode)
{
    TStream *trw;
    if(!filename || !mode) return 0;

    trw = TStreamFromFilePointer(fopen(filename, mode), 1);

    if(trw) {
        //evaluate the size of the file
        ((struct TStreamFile *)trw->content)->size = TFileSysGetFileSize(filename);
    }

    return trw;
}

TStream *TStreamFromFilePointer(FILE *f, unsigned char autoclose)
{
    TStream *trw;
    struct TStreamFile *file;
    if(!f) return 0;

    trw = TAllocData(TStream);
    if(!trw) return 0;

    file = TAllocData(struct TStreamFile);
    if(!file) {
        TFree(trw);
        return 0;
    }

    file->f = f;
    file->autoclose = autoclose;
    file->offset = 0;
    file->size = 0;

    trw->content = file;
    trw->operations = TStreamFileOps;

#ifdef _WINDOWS
    if (f->_flag&TStream_READ_ONLY_FLAG)
        trw->operations.write = 0;
#endif

    return trw;
}

TStream *TStreamFromMem(unsigned char *buffer, TSize size, unsigned char autofree)
{
    TStream *trw;
    struct TStreamBuffer *buf;
    if(!buffer || size <= 0) return 0;

    trw = TAllocData(TStream);
    if(!trw) return 0;

    buf = TAllocData(struct TStreamBuffer);
    if(!buf) {
        TFree(trw);
        return 0;
    }

    buf->buffer = buffer;
    buf->size = size;
    buf->offset = 0;
    buf->autofree = autofree;

    trw->content = buf;
    trw->operations = TStreamBufferOps;

    return trw;
}

TStream *TStreamFromConstMem(const unsigned char *buffer, TSize size)
{
    TStream *trw;
    if(!buffer || size <= 0) return 0;

    trw = TStreamFromMem((unsigned char *)buffer, size, 0);
    if(!trw) return 0;

    trw->operations.write = 0;

    return trw;
}

TStream *TStreamFromContent(TStreamContent content, const TStreamOps ops)
{
    TStream *trw;
    if(!content) return 0;

    trw = TAllocData(TStream);
    if(!trw) return 0;

    trw->content = content;
    trw->operations = ops;

    return trw;
}

void TStreamFree(TStream *context)
{
    if(context) {
        if(context->operations.close) {
            context->operations.close(context->content);
        }
        TFree(context->content);
        TFree(context);
    }
}

void TStreamSetOps(TStream *context, const TStreamOps ops)
{
    if(context) {
        context->operations = ops;
    }
}

TSize TStreamSize(TStream *context)
{
    if(context) {
        if(context->operations.size) {
			return context->operations.size(context->content);
        }

		TErrorZero(T_ERROR_OPERATION_NOT_SUPPORTED);
    }

	TErrorZero(T_ERROR_NULL_POINTER);
}

int TStreamSeek(TStream *context, TLInt offset, int origin)
{
    if(context) {
        if(context->operations.seek) {
			return context->operations.seek(context->content, offset, origin);
        }

		TErrorZero(T_ERROR_OPERATION_NOT_SUPPORTED);
    }

	TErrorZero(T_ERROR_NULL_POINTER);
}

TLInt TStreamTell(TStream *context)
{
    if(context) {
        if(context->operations.tell) {
			return context->operations.tell(context->content);
        }

		TErrorZero(T_ERROR_OPERATION_NOT_SUPPORTED);
    }

	TErrorZero(T_ERROR_NULL_POINTER);
}

int TStreamEOF(TStream *context)
{
    if(context) {
        if(context->operations.eof) {
			return context->operations.eof(context->content);
        }

		TErrorZero(T_ERROR_OPERATION_NOT_SUPPORTED);
    }

	TErrorZero(T_ERROR_NULL_POINTER);
}

TUInt8 TStreamRead8(TStream *context)
{
    if(context) {
        if(context->operations.read) {
			TUInt8 buf;
			context->operations.read(context->content, &buf, sizeof(TUInt8));
            return buf;
        }

		TErrorZero(T_ERROR_OPERATION_NOT_SUPPORTED);
    }

	TErrorZero(T_ERROR_NULL_POINTER);
}

TUInt16 TStreamRead16(TStream *context)
{
    if(context) {
        if(context->operations.read) {
			TUInt16 buf;
			context->operations.read(context->content, &buf, sizeof(TUInt16));
            return buf;
        }

		TErrorZero(T_ERROR_OPERATION_NOT_SUPPORTED);
    }

	TErrorZero(T_ERROR_NULL_POINTER);
}

TUInt32 TStreamRead32(TStream *context)
{
    if(context) {
        if(context->operations.read) {
			TUInt32 buf;
			context->operations.read(context->content, &buf, sizeof(TUInt32));
            return buf;
        }

		TErrorZero(T_ERROR_OPERATION_NOT_SUPPORTED);
    }

	TErrorZero(T_ERROR_NULL_POINTER);
}

TUInt64 TStreamRead64(TStream *context)
{
    if(context) {
        if(context->operations.read) {
			TUInt64 buf;
			context->operations.read(context->content, &buf, sizeof(TUInt64));
            return buf;
        }

		TErrorZero(T_ERROR_OPERATION_NOT_SUPPORTED);
    }

	TErrorZero(T_ERROR_NULL_POINTER);
}

TSize TStreamReadBlock(TStream *context, unsigned char *buffer, TSize count)
{
    if(context && buffer && count) {
        if(context->operations.read) {
			return context->operations.read(context->content, buffer, count);
        }

		TErrorZero(T_ERROR_OPERATION_NOT_SUPPORTED);
    }

	TErrorZero(T_ERROR_NULL_POINTER);
}

int TStreamWrite8(TStream *context, TUInt8 data)
{
    if(context) {
        if(context->operations.write) {
			return context->operations.write(context->content, &data, sizeof(TUInt8));
        }

		TErrorReportDefault(T_ERROR_OPERATION_NOT_SUPPORTED);
		return 1;
    }

	TErrorReportDefault(T_ERROR_NULL_POINTER);
    return 1;
}

int TStreamWrite16(TStream *context, TUInt16 data)
{
    if(context) {
        if(context->operations.write) {
			return context->operations.write(context->content, &data, sizeof(TUInt16));
        }

		TErrorReportDefault(T_ERROR_OPERATION_NOT_SUPPORTED);
        return 1;
    }

	TErrorReportDefault(T_ERROR_NULL_POINTER);
    return 1;
}

int TStreamWrite32(TStream *context, TUInt32 data)
{
    if(context) {
        if(context->operations.write) {
			return context->operations.write(context->content, &data, sizeof(TUInt32));
        }

		TErrorReportDefault(T_ERROR_OPERATION_NOT_SUPPORTED);
        return 1;
    }

	TErrorReportDefault(T_ERROR_NULL_POINTER);
    return 1;
}

int TStreamWrite64(TStream *context, TUInt64 data)
{
    if(context) {
        if(context->operations.write) {
			return context->operations.write(context->content, &data, sizeof(TUInt64));
        }

		TErrorReportDefault(T_ERROR_OPERATION_NOT_SUPPORTED);
        return 1;
    }

	TErrorReportDefault(T_ERROR_NULL_POINTER);
    return 1;
}

int TStreamWriteBlock(TStream *context, const unsigned char *buffer, TSize size)
{
    if(context && buffer && size) {
        if(context->operations.write) {
			return context->operations.write(context->content, buffer, size);
        }

		TErrorReportDefault(T_ERROR_OPERATION_NOT_SUPPORTED);
        return 1;
    }

	TErrorReportDefault(T_ERROR_NULL_POINTER);
    return 1;
}

int TStreamWriteString(TStream *context, const char *buffer, TSize size)
{
    if (context && buffer) {
        if (context->operations.write) {
            if (!size) size = sizeof(char) * strlen(buffer);
			return context->operations.write(context->content, buffer, size);
        }

		TErrorReportDefault(T_ERROR_OPERATION_NOT_SUPPORTED);
        return 1;
    }

	TErrorReportDefault(T_ERROR_NULL_POINTER);
    return 1;
}
