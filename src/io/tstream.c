
#include "stdafx.h"

#include "tstream.h"

#include "talloc.h"
#include "terror.h"
#include "tthread.h"

#include "utility/tfilesys.h"

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

static TSize TStreamFileSize(TStream *context)
{
    struct TStreamFile *content;
    if(!context->content) return 0;
    content = (struct TStreamFile *) context->content;

    return content->size - content->offset; 
}

static int TStreamFileSeek(TStream *context, TSize offset, int origin)
{
    struct TStreamFile *content;
    if(!context->content) return 0;
    content = (struct TStreamFile *) context->content;

    if(origin == SEEK_CUR) {
        content->offset += offset;
    } else if(origin == SEEK_END) {
        if(offset > content->size) offset = content->size;
        content->offset = content->size - offset;
    } else {
        content->offset = offset;
    }
    content->offset = TCLAMP(content->offset, 0, content->size);

    return fseek(content->f, offset, origin);
}

static int TStreamFileTell(TStream *context)
{
    struct TStreamFile *content;
    if(!context->content) return 0;
    content = (struct TStreamFile *) context->content;

    return ftell(content->f);
}

static unsigned char TStreamFileEOF(TStream *context)
{
    struct TStreamFile *content;
    if(!context->content) return 0;
    content = (struct TStreamFile *) context->content;

    return feof(content->f);
}

static TSize TStreamFileRead(TStream *context, TPtr buffer, TSize size)
{
    int read;
    struct TStreamFile *content;
    if(!context->content || !buffer || !size) return 0;
    content = (struct TStreamFile *) context->content;

    if(feof(content->f)) return 0;

    read = fread(buffer, 1, size, content->f);
    content->offset += read;

    return read;
}

static TSize TStreamFileWrite(TStream *context, TCPtr buffer, TSize size)
{
    int written = 0;
    struct TStreamFile *content;
    if(!context->content || !buffer || !size) return 0;
    content = (struct TStreamFile *) context->content;

    written = fwrite(buffer, 1, size, content->f);

    if(written) {
        content->offset += written;
        if(content->offset > content->size)
            content->size = content->offset;
    }

    return written;
}

static int TStreamFileClose(TStream *context)
{
    struct TStreamFile *content;
    if(!context->content) return 0;
    content = (struct TStreamFile *) context->content;

    if(content->autoclose)
        return fclose(content->f);

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

static TSize TStreamBufferSize(TStream *context)
{
    struct TStreamBuffer *content;
    if(!context->content) return 0;
    content = (struct TStreamBuffer *) context->content;

    return content->size - content->offset;
}

static int TStreamBufferSeek(TStream *context, TSize offset, int origin)
{
    struct TStreamBuffer *content;
    if(!context->content) return 0;
    content = (struct TStreamBuffer *) context->content;

    if(origin == SEEK_CUR) {
        content->offset += offset;
    } else if(origin == SEEK_END) {
        if(offset > content->size) offset = content->size;
        content->offset = content->size - offset;
    } else {
        content->offset = offset;
    }
    content->offset = TCLAMP(content->offset, 0, content->size);

    return 0;
}

static int TStreamBufferTell(TStream *context)
{
    struct TStreamBuffer *content;
    if(!context->content) return 0;
    content = (struct TStreamBuffer *) context->content;

    return content->offset;
}

static unsigned char TStreamBufferEOF(TStream *context)
{
    struct TStreamBuffer *content;
    if(!context->content) return 0;
    content = (struct TStreamBuffer *) context->content;

    return content->offset >= content->size;
}

static TSize TStreamBufferRead(TStream *context, TPtr buffer, TSize size)
{
    TSize cpysize;
    struct TStreamBuffer *content;
    if(!context->content || !buffer || !size) return 0;
    content = (struct TStreamBuffer *) context->content;

    if(content->offset >= content->size) return 0;

    cpysize = TMIN(size, (TSize) sizeof(unsigned char) * (content->size - content->offset));
    memcpy(buffer,content->buffer + content->offset, cpysize);
    content->offset += cpysize + 1;

    return cpysize;
}

static TSize TStreamBufferWrite(TStream *context, TCPtr buffer, TSize size)
{
    struct TStreamBuffer *content;
    if(!context->content || !buffer || !size) return 0;
    content = (struct TStreamBuffer *) context->content;

    if (content->offset + size > content->size) {
        //attempt to increase buffer size
        unsigned char *newBuffer = TRAlloc(content->buffer, content->offset + size);
        if (newBuffer) content->buffer = newBuffer;
        else size = (TSize) sizeof(char) * (content->size - content->offset);
    }

    memcpy(content->buffer + content->offset, buffer, size);
    content->offset = TCLAMP(size + 1, 0, content->size);

    return size;
}

static int TStreamBufferClose(TStream *context)
{
    struct TStreamBuffer *content;
    if (!context->content) return 0;
    content = (struct TStreamBuffer *) context->content;

    if (content->autofree)
        TFree(content->buffer);

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
            context->operations.close(context);
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
            return context->operations.size(context);
        }

        TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
        return 0;
    }

    TErrorReport(T_ERROR_NULL_POINTER);
    return 0;
}

int TStreamSeek(TStream *context, TSize offset, int origin)
{
    if(context) {
        if(context->operations.seek) {
            return context->operations.seek(context, offset, origin);
        }

        TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
        return 0;
    }

    TErrorReport(T_ERROR_NULL_POINTER);
    return 0;
}

int TStreamTell(TStream *context)
{
    if(context) {
        if(context->operations.tell) {
            return context->operations.tell(context);
        }

        TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
        return 0;
    }

    TErrorReport(T_ERROR_NULL_POINTER);
    return 0;
}

unsigned char TStreamEOF(TStream *context)
{
    if(context) {
        if(context->operations.eof) {
            return context->operations.eof(context);
        }

        TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
        return 0;
    }

    TErrorReport(T_ERROR_NULL_POINTER);
    return 0;
}

unsigned char TStreamRead8(TStream *context)
{
    if(context) {
        if(context->operations.read) {
            unsigned char buf;
            context->operations.read(context, &buf, sizeof(char));
            return buf;
        }

        TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
        return 0;
    }

    TErrorReport(T_ERROR_NULL_POINTER);
    return 0;
}

unsigned short TStreamRead16(TStream *context)
{
    if(context) {
        if(context->operations.read) {
            unsigned short buf;
            context->operations.read(context, &buf, sizeof(short));
            return buf;
        }

        TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
        return 0;
    }

    TErrorReport(T_ERROR_NULL_POINTER);
    return 0;
}

unsigned int TStreamRead32(TStream *context)
{
    if(context) {
        if(context->operations.read) {
            unsigned int buf;
            context->operations.read(context, &buf, sizeof(int));
            return buf;
        }

        TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
        return 0;
    }

    TErrorReport(T_ERROR_NULL_POINTER);
    return 0;
}

unsigned long long TStreamRead64(TStream *context)
{
    if(context) {
        if(context->operations.read) {
            unsigned long long buf;
            context->operations.read(context, &buf, sizeof(long long));
            return buf;
        }

        TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
        return 0;
    }

    TErrorReport(T_ERROR_NULL_POINTER);
    return 0;
}

TSize TStreamReadBlock(TStream *context, unsigned char *buffer, TSize count)
{
    if(context && buffer && count) {
        if(context->operations.read) {
            return context->operations.read(context, buffer, count);
        }

        TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
        return 0;
    }

    TErrorReport(T_ERROR_NULL_POINTER);
    return 0;
}

int TStreamWrite8(TStream *context, unsigned char data)
{
    if(context) {
        if(context->operations.write) {
            return context->operations.write(context, &data, sizeof(char));
        }

        TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
        return 1;
    }

    TErrorReport(T_ERROR_NULL_POINTER);
    return 1;
}

int TStreamWrite16(TStream *context, unsigned short data)
{
    if(context) {
        if(context->operations.write) {
            return context->operations.write(context, &data, sizeof(short));
        }

        TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
        return 1;
    }

    TErrorReport(T_ERROR_NULL_POINTER);
    return 1;
}

int TStreamWrite32(TStream *context, unsigned int data)
{
    if(context) {
        if(context->operations.write) {
            return context->operations.write(context, &data, sizeof(int));
        }

        TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
        return 1;
    }

    TErrorReport(T_ERROR_NULL_POINTER);
    return 1;
}

int TStreamWrite64(TStream *context, unsigned long long data)
{
    if(context) {
        if(context->operations.write) {
            return context->operations.write(context, &data, sizeof(long long));
        }

        TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
        return 1;
    }

    TErrorReport(T_ERROR_NULL_POINTER);
    return 1;
}

int TStreamWriteBlock(TStream *context, const unsigned char *buffer, TSize size)
{
    if(context && buffer && size) {
        if(context->operations.write) {
            return context->operations.write(context, buffer, size);
        }

        TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
        return 0;
    }

    TErrorReport(T_ERROR_NULL_POINTER);
    return 0;
}

int TStreamWriteString(TStream *context, const char *buffer, TSize size)
{
    if (context && buffer) {
        if (context->operations.write) {
            if (!size) size = sizeof(char) * strlen(buffer);
            return context->operations.write(context, buffer, size);
        }

        TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
        return 0;
    }

    TErrorReport(T_ERROR_NULL_POINTER);
    return 0;
}
