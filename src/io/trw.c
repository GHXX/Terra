
#include "stdafx.h"

#include "trw.h"

#include "talloc.h"
#include "terror.h"
#include "tthread.h"

#include "debugging/tdebug.h"

//--- Default File Operations ------------------------------//

#define TRW_READ_ONLY_FLAG 1

struct TRWFile {
	FILE *f;
	unsigned char autoclose;
};

static size_t TRWFileSize(TRW *context)
{
	int now;
	size_t size = 0;
	struct TRWFile *content;
	if(!context->content) return 0;
	content = (struct TRWFile *) context->content;

	now = context->operations.tell(context);
	fseek(content->f, 0, SEEK_END);
	size = context->operations.tell(context);
	fseek(content->f, now, SEEK_SET);

	return size;
}

static int TRWFileSeek(TRW *context, int offset, int origin)
{
	struct TRWFile *content;
	if(!context->content) return 0;
	content = (struct TRWFile *) context->content;

	return fseek(content->f, offset, origin);
}

static int TRWFileTell(TRW *context)
{
	struct TRWFile *content;
	if(!context->content) return 0;
	content = (struct TRWFile *) context->content;

	return ftell(content->f);
}

static char TRWFileEOF(TRW *context)
{
	struct TRWFile *content;
	if(!context->content) return 0;
	content = (struct TRWFile *) context->content;

	return feof(content->f);
}

static size_t TRWFileRead(TRW *context, void *buffer, size_t size)
{
	struct TRWFile *content;
	if(!context->content || !buffer || !size) return 0;
	content = (struct TRWFile *) context->content;

	if(feof(content->f)) return 0;

	return fread(buffer, 1, size, content->f);
}

static size_t TRWFileWrite(TRW *context, const void *buffer, size_t size)
{
	struct TRWFile *content;
	if(!context->content || !buffer || !size) return 0;
	content = (struct TRWFile *) context->content;

	return fwrite(buffer, 1, size, content->f);
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
	char *buffer;
	int size;
	int offset;
};

static size_t TRWBufferSize(TRW *context)
{
	struct TRWBuffer *content;
	if(!context->content) return 0;
	content = (struct TRWBuffer *) context->content;

	return content->size - content->offset;
}

static int TRWBufferSeek(TRW *context, int offset, int origin)
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
	content->offset = TCLAMP(content->offset,0,content->size);

	return 0;
}

static int TRWBufferTell(TRW *context)
{
	struct TRWBuffer *content;
	if(!context->content) return 0;
	content = (struct TRWBuffer *) context->content;

	return content->offset;
}

static char TRWBufferEOF(TRW *context)
{
	struct TRWBuffer *content;
	if(!context->content) return 0;
	content = (struct TRWBuffer *) context->content;

	return content->offset >= content->size;
}

static size_t TRWBufferRead(TRW *context, void *buffer, size_t size)
{
	size_t cpysize;
	struct TRWBuffer *content;
	if(!context->content || !buffer || !size) return 0;
	content = (struct TRWBuffer *) context->content;

	if(content->offset >= content->size) return 0;

	cpysize = TMIN(size,(size_t) (content->size - content->offset));
	memcpy(buffer,content->buffer + content->offset, sizeof(char) * cpysize);
	content->offset += cpysize + 1;

	return cpysize;
}

static size_t TRWBufferWrite(TRW *context, const void *buffer, size_t size)
{
	size_t cpysize;
	struct TRWBuffer *content;
	if(!context->content || !buffer || !size) return 0;
	content = (struct TRWBuffer *) context->content;

	cpysize = TMIN(size,(size_t) (content->size - content->offset));
	memcpy(content->buffer + content->offset,buffer, sizeof(char) * cpysize);
	content->offset += cpysize + 1;

	return cpysize;
}

static TRWOps TRWBufferOps = {
	TRWBufferSize,
	TRWBufferSeek,
	TRWBufferTell,
	TRWBufferEOF,
	TRWBufferRead,
	TRWBufferWrite,
	0,
};

//--- Terra Read Write ------------------------------//

enum TRWTypes {
	TRW_UNKNOWN = 0,
	TRW_FILE,
	TRW_BUFFER,
};

TRW *TRWFromFile(const char *filename, const char *mode)
{
	if(!filename || !mode) return 0;

	return TRWFromFilePointer(fopen(filename,mode),1);
}

TRW *TRWFromFilePointer(FILE *f, unsigned char autoclose)
{
	TRW *trw;
	struct TRWFile *file;
	if(!f) return 0;

	trw = (TRW *) TAlloc(sizeof(TRW));
	if(!trw) return 0;

	file = (struct TRWFile *) TAlloc(sizeof(struct TRWFile));
	if(!file) {
		TFree(trw);
		return 0;
	}

	file->f = f;
	file->autoclose = autoclose;

	trw->content = file;
	trw->operations = TRWFileOps;

#ifdef _WINDOWS
	if(f->_flag&TRW_READ_ONLY_FLAG)
		trw->operations.write = 0;
#endif

	return trw;
}

TRW *TRWFromMem(char *buffer, int size)
{
	TRW *trw;
	struct TRWBuffer *buf;
	if(!buffer || size <= 0) return 0;

	trw = (TRW *) TAlloc(sizeof(TRW));
	if(!trw) return 0;

	buf = (struct TRWBuffer *) TAlloc(sizeof(struct TRWBuffer));
	if(!buf) {
		TFree(trw);
		return 0;
	}

	buf->buffer = buffer;
	buf->size = size;
	buf->offset = 0;

	trw->content = buf;
	trw->operations = TRWBufferOps;

	return trw;
}

TRW *TRWFromConstMem(const char *buffer, int size)
{
	TRW *trw;
	if(!buffer || size <= 0) return 0;

	trw = TRWFromMem((char *)buffer,size);
	if(!trw) return 0;

	trw->operations.write = 0;

	return trw;
}

TRW *TRWFromContent(TRWContent content)
{
	TRW *trw;
	if(!content) return 0;

	trw = (TRW *) TAlloc(sizeof(TRW));
	if(!trw) return 0;

	trw->content = content;
	memset(&trw->operations,0,sizeof(TRWOps));

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

void TRWSetOps(TRW *context, const TRWOps *ops)
{
	if(context) {
		context->operations = *ops;
	}
}

size_t TRWSize(TRW *context)
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

int TRWSeek(TRW *context, int offset, int origin)
{
	if(context) {
		if(context->operations.seek) {
			return context->operations.seek(context,offset,origin);
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

char TRWEOF(TRW *context)
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
			context->operations.read(context,&buf,sizeof(char));
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
			context->operations.read(context,&buf,sizeof(short));
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
			context->operations.read(context,&buf,sizeof(int));
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
			context->operations.read(context,&buf,sizeof(long long));
			return buf;
		}

		TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
		return 0;
	}

	TErrorReport(T_ERROR_NULL_POINTER);
	return 0;
}

size_t TRWReadBlock(TRW *context, char *buffer, size_t count)
{
	if(context && buffer && count) {
		if(context->operations.read) {
			size_t end = context->operations.read(context,buffer,count);
			buffer[TMIN(end,count-1)] = '\0';
			return end;
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
			return context->operations.write(context,&data,sizeof(char));
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
			return context->operations.write(context,&data,sizeof(short));
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
			return context->operations.write(context,&data,sizeof(int));
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
			return context->operations.write(context,&data,sizeof(long long));
		}

		TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
		return 1;
	}

	TErrorReport(T_ERROR_NULL_POINTER);
	return 1;
}

int TRWWriteBlock(TRW *context, const void *buffer, size_t size)
{
	if(context && buffer && size) {
		if(context->operations.write) {
			return context->operations.write(context,buffer,sizeof(char) * size);
		}

		TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
		return 0;
	}

	TErrorReport(T_ERROR_NULL_POINTER);
	return 0;
}

void TRWWrite(TRW *context, const char *format,...)
{
	if(context && format) {
		va_list list;
		va_start(list,format);

		TRWWriteV(context,format,list);

		va_end(list);
		return;
	}

	TErrorReport(T_ERROR_NULL_POINTER);
	return;
}

void TRWWriteV(TRW *context, const char *format, va_list list)
{
	if(context && format) {
		if(context->operations.write) {
			char buffer[256];
			int res = vsnprintf(buffer,sizeof(buffer),format,list);
			context->operations.write(context,buffer,res);

			if(res > sizeof(buffer)) {
				//TODO
			}
		}

		TErrorReport(T_ERROR_OPERATION_NOT_SUPPORTED);
		return;
	}

	TErrorReport(T_ERROR_NULL_POINTER);
	return;
}
