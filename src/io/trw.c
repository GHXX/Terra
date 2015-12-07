
#include "stdafx.h"

#include "trw.h"

#include "talloc.h"
#include "terror.h"
#include "tthread.h"

#include "utility/tfilesys.h"

//--- Default File Operations ------------------------------//

#define TRW_READ_ONLY_FLAG 1

struct _TRW {
    TRWOps operations;
    TRWContent content;
};

struct TRWFile {
    FILE *f;
    unsigned char autoclose;
    TSize offset, size;
};

static TSize TRWFileSize(TRW *context)
{
    struct TRWFile *content;
    if(!context->content) return 0;
    content = (struct TRWFile *) context->content;

    return content->size - content->offset; 
}

static int TRWFileSeek(TRW *context, TSize offset, int origin)
{
    struct TRWFile *content;
    if(!context->content) return 0;
    content = (struct TRWFile *) context->content;

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

static int TRWFileTell(TRW *context)
{
    struct TRWFile *content;
    if(!context->content) return 0;
    content = (struct TRWFile *) context->content;

    return ftell(content->f);
}

static unsigned char TRWFileEOF(TRW *context)
{
    struct TRWFile *content;
    if(!context->content) return 0;
    content = (struct TRWFile *) context->content;

    return feof(content->f);
}

static TSize TRWFileRead(TRW *context, TPtr buffer, TSize size)
{
    int read;
    struct TRWFile *content;
    if(!context->content || !buffer || !size) return 0;
    content = (struct TRWFile *) context->content;

    if(feof(content->f)) return 0;

    read = fread(buffer, 1, size, content->f);
    content->offset += read;

    return read;
}

static TSize TRWFileWrite(TRW *context, TCPtr buffer, TSize size)
{
    int written = 0;
    struct TRWFile *content;
    if(!context->content || !buffer || !size) return 0;
    content = (struct TRWFile *) context->content;

    written = fwrite(buffer, 1, size, content->f);

    if(written) {
        content->offset += written;
        if(content->offset > content->size)
            content->size = content->offset;
    }

    return written;
}

static int TRWFileClose(TRW *context)
{
    struct TRWFile *content;
    if(!context->content) return 0;
    content = (struct TRWFile *) context->content;

    if(content->autoclose)
        return fclose(content->f);

    return 0;
}

static TRWOps TRWFileOps = {
    TRWFileSize,
    TRWFileSeek,
    TRWFileTell,
    TRWFileEOF,
    TRWFileRead,
    TRWFileWrite,
    TRWFileClose,
};

//--- Default Buffer Operations ------------------------------//

struct TRWBuffer {
    unsigned char *buffer;
    TSize size;
    TSize offset;

    unsigned char autofree;
};

static TSize TRWBufferSize(TRW *context)
{
    struct TRWBuffer *content;
    if(!context->content) return 0;
    content = (struct TRWBuffer *) context->content;

    return content->size - content->offset;
}

static int TRWBufferSeek(TRW *context, TSize offset, int origin)
{
    struct TRWBuffer *content;
    if(!context->content) return 0;
    content = (struct TRWBuffer *) context->content;

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

static int TRWBufferTell(TRW *context)
{
    struct TRWBuffer *content;
    if(!context->content) return 0;
    content = (struct TRWBuffer *) context->content;

    return content->offset;
}

static unsigned char TRWBufferEOF(TRW *context)
{
    struct TRWBuffer *content;
    if(!context->content) return 0;
    content = (struct TRWBuffer *) context->content;

    return content->offset >= content->size;
}

static TSize TRWBufferRead(TRW *context, TPtr buffer, TSize size)
{
    TSize cpysize;
    struct TRWBuffer *content;
    if(!context->content || !buffer || !size) return 0;
    content = (struct TRWBuffer *) context->content;

    if(content->offset >= content->size) return 0;

    cpysize = TMIN(size, (TSize) sizeof(unsigned char) * (content->size - content->offset));
    memcpy(buffer,content->buffer + content->offset, cpysize);
    content->offset += cpysize + 1;

    return cpysize;
}

static TSize TRWBufferWrite(TRW *context, TCPtr buffer, TSize size)
{
    struct TRWBuffer *content;
    if(!context->content || !buffer || !size) return 0;
    content = (struct TRWBuffer *) context->content;

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

static int TRWBufferClose(TRW *context)
{
    struct TRWBuffer *content;
    if (!context->content) return 0;
    content = (struct TRWBuffer *) context->content;

    if (content->autofree)
        TFree(content->buffer);

    return 0;
}

static TRWOps TRWBufferOps = {
    TRWBufferSize,
    TRWBufferSeek,
    TRWBufferTell,
    TRWBufferEOF,
    TRWBufferRead,
    TRWBufferWrite,
    TRWBufferClose,
};

//--- Terra Read Write ------------------------------//

enum TRWTypes {
    TRW_UNKNOWN = 0,
    TRW_FILE,
    TRW_BUFFER,
};

TRW *TRWFromFile(const char *filename, const char *mode)
{
    TRW *trw;
    if(!filename || !mode) return 0;

    trw = TRWFromFilePointer(fopen(filename, mode), 1);

    if(trw) {
        //evaluate the size of the file
        ((struct TRWFile *)trw->content)->size = TFileSysGetFileSize(filename);
    }

    return trw;
}

TRW *TRWFromFilePointer(FILE *f, unsigned char autoclose)
{
    TRW *trw;
    struct TRWFile *file;
    if(!f) return 0;

    trw = TAllocData(TRW);
    if(!trw) return 0;

    file = TAllocData(struct TRWFile);
    if(!file) {
        TFree(trw);
        return 0;
    }

    file->f = f;
    file->autoclose = autoclose;
    file->offset = 0;
    file->size = 0;

    trw->content = file;
    trw->operations = TRWFileOps;

#ifdef _WINDOWS
    if (f->_flag&TRW_READ_ONLY_FLAG)
        trw->operations.write = 0;
#endif

    return trw;
}

TRW *TRWFromMem(unsigned char *buffer, TSize size, unsigned char autofree)
{
    TRW *trw;
    struct TRWBuffer *buf;
    if(!buffer || size <= 0) return 0;

    trw = TAllocData(TRW);
    if(!trw) return 0;

    buf = TAllocData(struct TRWBuffer);
    if(!buf) {
        TFree(trw);
        return 0;
    }

    buf->buffer = buffer;
    buf->size = size;
    buf->offset = 0;
    buf->autofree = autofree;

    trw->content = buf;
    trw->operations = TRWBufferOps;

    return trw;
}

TRW *TRWFromConstMem(const unsigned char *buffer, TSize size)
{
    TRW *trw;
    if(!buffer || size <= 0) return 0;

    trw = TRWFromMem((unsigned char *)buffer, size, 1);
    if(!trw) return 0;

    trw->operations.write = 0;

    return trw;
}

TRW *TRWFromContent(TRWContent content, const TRWOps ops)
{
    TRW *trw;
    if(!content) return 0;

    trw = TAllocData(TRW);
    if(!trw) return 0;

    trw->content = content;
    trw->operations = ops;

    return trw;
}

void TRWFree(TRW *context)
{
    if(context) {
        if(context->operations.close) {
            context->operations.close(context);
        }
        TFree(context->content);
        TFree(context);
    }
}

void TRWSetOps(TRW *context, const TRWOps ops)
{
    if(context) {
        context->operations = ops;
    }
}

TSize TRWSize(TRW *context)
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

int TRWSeek(TRW *context, TSize offset, int origin)
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

int TRWTell(TRW *context)
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

unsigned char TRWEOF(TRW *context)
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

unsigned char TRWRead8(TRW *context)
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

unsigned short TRWRead16(TRW *context)
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

unsigned int TRWRead32(TRW *context)
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

unsigned long long TRWRead64(TRW *context)
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

TSize TRWReadBlock(TRW *context, unsigned char *buffer, TSize count)
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

int TRWWrite8(TRW *context, unsigned char data)
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

int TRWWrite16(TRW *context, unsigned short data)
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

int TRWWrite32(TRW *context, unsigned int data)
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

int TRWWrite64(TRW *context, unsigned long long data)
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

int TRWWriteBlock(TRW *context, const unsigned char *buffer, TSize size)
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

int TRWWriteString(TRW *context, const char *buffer, TSize size)
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
