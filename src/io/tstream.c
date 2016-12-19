
#include "stdafx.h"

#include "tstream.h"

#include "talloc.h"
#include "terror.h"
#include "tthread.h"

#include "utility/tinteger.h"
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

//--- NULL Operations --------------------------------------//

static TSize TStreamNullSize(TStreamContent *content) { TErrorZero(T_ERROR_OPERATION_NOT_SUPPORTED); return 0; }

static int TStreamNullSeek(TStreamContent *content, TLInt offset, TInt8 origin) { TErrorZero(T_ERROR_OPERATION_NOT_SUPPORTED); return 0; }

static TLInt TStreamNullTell(TStreamContent *content) { TErrorZero(T_ERROR_OPERATION_NOT_SUPPORTED); return 0; }

static unsigned char TStreamNullEOF(TStreamContent *content) { TErrorZero(T_ERROR_OPERATION_NOT_SUPPORTED); return 0; }

static TSize TStreamNullRead(TStreamContent *content, TPtr buffer, TSize size) { TErrorZero(T_ERROR_OPERATION_NOT_SUPPORTED); return 0; }

static TSize TStreamNullWrite(TStreamContent *content, TCPtr buffer, TSize size) { TErrorZero(T_ERROR_OPERATION_NOT_SUPPORTED); return 0; }

static int TStreamNullClose(TStreamContent *content) { TErrorZero(T_ERROR_OPERATION_NOT_SUPPORTED); return 0; }

//--- Default File Operations ------------------------------//

struct _TStream {
    TStreamOps operations;
    TStreamContent content;
};

struct TStreamFile {
    FILE *f;
    unsigned char autoclose;
    TSize offset, size;
};

static TSize TStreamFileSize(TStreamContent *content) {
	struct TStreamFile *streamFile;
	streamFile = (struct TStreamFile *) content;

	if (streamFile->size) return streamFile->size - streamFile->offset;

	return 0;
}

static int TStreamFileSeek(TStreamContent *content, TLInt offset, TInt8 origin) {
	struct TStreamFile *streamFile;
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

static TLInt TStreamFileTell(TStreamContent *content) {
	struct TStreamFile *streamFile;
	streamFile = (struct TStreamFile *) content;

	return ftell(streamFile->f);
}

static unsigned char TStreamFileEOF(TStreamContent *content) {
	int read;
	unsigned char buffer;

	struct TStreamFile *streamFile;
	streamFile = (struct TStreamFile *) content;

	read = fread(&buffer, 1, 1, streamFile->f);
	if (read) fseek(streamFile->f, -1, SEEK_CUR);

	return !read;
}

static TSize TStreamFileRead(TStreamContent *content, TPtr buffer, TSize size) {
	int read;
	struct TStreamFile *streamFile;
	streamFile = (struct TStreamFile *) content;

	read = fread(buffer, 1, size, streamFile->f);
	streamFile->offset += read;

	return read;
}

static TSize TStreamFileWrite(TStreamContent *content, TCPtr buffer, TSize size) {
	int written = 0;
	struct TStreamFile *streamFile;
	streamFile = (struct TStreamFile *) content;

	written = fwrite(buffer, 1, size, streamFile->f);

	if (written) {
		streamFile->offset += written;
		if (streamFile->offset > streamFile->size)
			streamFile->size = streamFile->offset;
	}

	return written;
}

static int TStreamFileClose(TStreamContent *content) {
	struct TStreamFile *streamFile;
	streamFile = (struct TStreamFile *) content;

	if (streamFile->autoclose)
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
    unsigned char *buffer, **bufPtr;
    TSize size;
    TSize offset;

    unsigned char autofree;
};

static TSize TStreamBufferSize(TStreamContent *content) {
	struct TStreamBuffer *streamBuffer;
	streamBuffer = (struct TStreamBuffer *) content;

	return streamBuffer->size - streamBuffer->offset;
}

static int TStreamBufferSeek(TStreamContent *content, TLInt offset, TInt8 origin) {
	struct TStreamBuffer *streamBuffer;
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

static TLInt TStreamBufferTell(TStreamContent *content) {
	struct TStreamBuffer *streamBuffer;
	streamBuffer = (struct TStreamBuffer *) content;

	return streamBuffer->offset;
}

static unsigned char TStreamBufferEOF(TStreamContent *content) {
	struct TStreamBuffer *streamBuffer;
	streamBuffer = (struct TStreamBuffer *) content;

	return streamBuffer->offset >= streamBuffer->size;
}

static TSize TStreamBufferRead(TStreamContent *content, TPtr buffer, TSize size) {
	TSize cpysize;
	struct TStreamBuffer *streamBuffer;
	streamBuffer = (struct TStreamBuffer *) content;

	if (streamBuffer->offset >= streamBuffer->size) return 0;

	cpysize = TMIN(size, (TSize) sizeof(unsigned char) * (streamBuffer->size - streamBuffer->offset));
	memcpy(buffer, streamBuffer->buffer + streamBuffer->offset, cpysize);
	streamBuffer->offset += cpysize;

	return cpysize;
}

static TSize TStreamBufferWrite(TStreamContent *content, TCPtr buffer, TSize size) {
	struct TStreamBuffer *streamBuffer;
	streamBuffer = (struct TStreamBuffer *) content;

	if (streamBuffer->offset + size > streamBuffer->size) {
		//attempt to increase buffer size
		unsigned char *newBuffer;
		TSize newSize = TIntegerUpperPowerOfTwo((streamBuffer->offset + size) / sizeof(unsigned char)) * sizeof(unsigned char);
		newBuffer = TRAlloc(streamBuffer->buffer, newSize);
		if (newBuffer) {
			streamBuffer->buffer = newBuffer;
			if (streamBuffer->bufPtr) *streamBuffer->bufPtr = newBuffer;
			streamBuffer->size = newSize;
		} else {
			size = (TSize) sizeof(unsigned char) * (streamBuffer->size - streamBuffer->offset);
		}
	}

	memcpy(streamBuffer->buffer + streamBuffer->offset, buffer, size);
	streamBuffer->offset += size;

	return size;
}

static int TStreamBufferClose(TStreamContent *content) {
	struct TStreamBuffer *streamBuffer;
	streamBuffer = (struct TStreamBuffer *) content;

	if (streamBuffer->autofree) {
		TFree(streamBuffer->buffer);
		if (streamBuffer->bufPtr) *streamBuffer->bufPtr = 0;
	}

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

TStream *TStreamNew(void) {
	TStream *trw;
	struct TStreamBuffer *buf;

	trw = TAllocData(TStream);
	if (!trw) return 0;

	buf = TAllocData(struct TStreamBuffer);
	if (!buf) {
		TFree(trw);
		return 0;
	}

	memset(buf, 0, sizeof(struct TStreamBuffer));
	buf->bufPtr = 0;
	buf->buffer = 0;
	buf->size = 0;
	buf->autofree = 1;

	trw->content = buf;
	trw->operations = TStreamBufferOps;

	return trw;
}

TStream *TStreamFromFile(const char *filename, const char *mode) {
	TStream *trw;
	if (!filename || !mode) return 0;

	trw = TStreamFromFilePointer(TFileSysOpen(filename, mode), 1);

	if (trw) {
		//evaluate the size of the file
		((struct TStreamFile *)trw->content)->size = TFileSysGetFileSize(filename);
	}

	return trw;
}

TStream *TStreamFromFilePointer(FILE *f, unsigned char autoclose) {
	TStream *trw;
	struct TStreamFile *file;
	if (!f) return 0;

	trw = TAllocData(TStream);
	if (!trw) return 0;

	file = TAllocData(struct TStreamFile);
	if (!file) {
		TFree(trw);
		return 0;
	}

	file->f = f;
	file->autoclose = autoclose;
	file->offset = 0;
	{
		long pos, end;
		pos = ftell(f);
		fseek(f, 0, SEEK_END);
		end = ftell(f);
		fseek(f, pos, SEEK_SET);
		file->size = end - pos;
	}

	trw->content = file;
	trw->operations = TStreamFileOps;

	if(TFileSysIsReadOnly(f))
		trw->operations.write = TStreamNullWrite;

	return trw;
}

TStream *TStreamFromMem(unsigned char **buffer, TSize size, unsigned char autofree) {
	TStream *trw;
	struct TStreamBuffer *buf;
	if (!buffer) return 0;

	trw = TAllocData(TStream);
	if (!trw) return 0;

	buf = TAllocData(struct TStreamBuffer);
	if (!buf) {
		TFree(trw);
		return 0;
	}

	memset(buf, 0, sizeof(struct TStreamBuffer));
	buf->buffer = *buffer;
	buf->bufPtr = buffer;
	buf->size = size;
	buf->autofree = autofree;

	trw->content = buf;
	trw->operations = TStreamBufferOps;

	return trw;
}

TStream *TStreamFromConstMem(const unsigned char *buffer, TSize size)
{
	TStream *trw = TStreamFromMem(&(unsigned char *)buffer, size, 0);

	if (trw) {
		trw->operations.write = TStreamNullWrite;
		((struct TStreamBuffer *)trw->content)->bufPtr = 0;
	}

    return trw;
}

TStream *TStreamFromContent(TStreamContent content, const TStreamOps ops) {
	TStream *trw;
	if (!content) return 0;

	trw = TAllocData(TStream);
	if (!trw) return 0;

	trw->content = content;
	trw->operations = ops;

	return trw;
}

void TStreamFree(TStream *context) {
	if (context) {
		context->operations.close(context->content);
		TFree(context->content);
		TFree(context);
	}
}

void TStreamSetOps(TStream *context, const TStreamOps ops) {
	if (context) {
		context->operations = ops;
		if (!context->operations.size) context->operations.size = TStreamNullSize;
		if (!context->operations.seek) context->operations.seek = TStreamNullSeek;
		if (!context->operations.tell) context->operations.tell = TStreamNullTell;
		if (!context->operations.eof) context->operations.eof = TStreamNullEOF;
		if (!context->operations.read) context->operations.read = TStreamNullRead;
		if (!context->operations.write) context->operations.write = TStreamNullWrite;
		if (!context->operations.close) context->operations.close = TStreamNullClose;
	}
}

TSize TStreamSize(TStream *context) {
	if (context) {
		return context->operations.size(context->content);
	}

	TErrorZero(T_ERROR_NULL_POINTER);
}

int TStreamSeek(TStream *context, TLInt offset, TInt8 origin) {
	if (context) {
		return context->operations.seek(context->content, offset, origin);
	}

	TErrorZero(T_ERROR_NULL_POINTER);
}

TLInt TStreamTell(TStream *context) {
	if (context) {
		return context->operations.tell(context->content);
	}

	TErrorZero(T_ERROR_NULL_POINTER);
}

int TStreamEOF(TStream *context) {
	if (context) {
		return context->operations.eof(context->content);
	}

	TErrorZero(T_ERROR_NULL_POINTER);
}

TUInt8 TStreamRead8(TStream *context) {
	if (context) {
		TUInt8 buf = 0;
		context->operations.read(context->content, &buf, sizeof(TUInt8));
		return buf;
	}

	TErrorZero(T_ERROR_NULL_POINTER);
}

TUInt16 TStreamRead16(TStream *context) {
	if (context) {
		TUInt16 buf = 0;
		context->operations.read(context->content, &buf, sizeof(TUInt16));
		return buf;
	}

	TErrorZero(T_ERROR_NULL_POINTER);
}

TUInt32 TStreamRead32(TStream *context) {
	if (context) {
		TUInt32 buf = 0;
		context->operations.read(context->content, &buf, sizeof(TUInt32));
		return buf;
	}

	TErrorZero(T_ERROR_NULL_POINTER);
}

TUInt64 TStreamRead64(TStream *context) {
	if (context) {
		TUInt64 buf = 0;
		context->operations.read(context->content, &buf, sizeof(TUInt64));
		return buf;
	}

	TErrorZero(T_ERROR_NULL_POINTER);
}

TSize TStreamReadBlock(TStream *context, unsigned char *buffer, TSize count) {
	if (context && buffer && count) {
		return context->operations.read(context->content, buffer, count);
	}

	TErrorZero(T_ERROR_NULL_POINTER);
}

int TStreamWrite8(TStream *context, TUInt8 data) {
	if (context) {
		return context->operations.write(context->content, &data, sizeof(TUInt8));
	}

	TErrorZero(T_ERROR_NULL_POINTER);
}

int TStreamWrite16(TStream *context, TUInt16 data) {
	if (context) {
		return context->operations.write(context->content, &data, sizeof(TUInt16));
	}

	TErrorZero(T_ERROR_NULL_POINTER);
}

int TStreamWrite32(TStream *context, TUInt32 data) {
	if (context) {
		return context->operations.write(context->content, &data, sizeof(TUInt32));
	}

	TErrorZero(T_ERROR_NULL_POINTER);
}

int TStreamWrite64(TStream *context, TUInt64 data) {
	if (context) {
		return context->operations.write(context->content, &data, sizeof(TUInt64));
	}

	TErrorZero(T_ERROR_NULL_POINTER);
}

int TStreamWriteBlock(TStream *context, const unsigned char *buffer, TSize size) {
	if (context && buffer && size) {
		return context->operations.write(context->content, buffer, size);
	}

	TErrorZero(T_ERROR_NULL_POINTER);
}
