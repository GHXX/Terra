
#include "stdafx.h"

#include "ttokenizer.h"

#include "talloc.h"
#include "terror.h"

static const char *TTokenizerDefaultSeparators = " \n\t\r";
static const size_t TTokenizerBufferMaxSize = 512;

struct TTokenizer {
	TRW *content;

	const char *separators;

	char *buffer;
	TSize bsize, tsize;
	int offset;
};

TTokenizer *TTokenizerNew(TRW *input)
{
	if(input) {
		TTokenizer *tokenizer = TAllocData(TTokenizer);
		if(tokenizer) {
			memset(tokenizer,0,sizeof(struct TTokenizer));
			tokenizer->content = input;
			tokenizer->separators = TTokenizerDefaultSeparators;
			tokenizer->tsize = TRWSize(input);
		}

		return tokenizer;
	}

	TErrorReport(T_ERROR_INVALID_INPUT);
	return 0;
}

void TTokenizerFree(TTokenizer *context) {
	if(context) {
		TFree(context->buffer);
		TFree(context);
	}
}

void TTokenizerSetSeparators(TTokenizer *context, const char *separators) {
	if(!context) TError(T_ERROR_INVALID_INPUT);	

	if(!separators) separators = TTokenizerDefaultSeparators;
	context->separators = separators;
}

const char *prepareToken(TTokenizer *context) {
	char *ptr;
	size_t next;
	
	do {
		ptr = context->buffer + context->offset;
		next = strcspn(ptr, context->separators);
		if(*(ptr+next) == 0) {
			if(!context->tsize) {
				//reached the end
				return !next ? 0 : ptr;
			}

			if(!context->offset) {
				//we are at the beginning and no separator. Just kill this
				TErrorReport(T_ERROR_OUT_OF_MEMORY);
				return 0;
			}

			// advance the buffer
			{
				TSize remainingSize = (context->bsize - context->offset) - 1;
				memcpy(context->buffer, ptr, remainingSize * sizeof(char));
				TRWReadBlock(context->content, context->buffer + remainingSize, context->offset);
				context->offset = 0;
				remainingSize = context->bsize - 1;
				next = 0;
			}
		} else {
			*(ptr+next) = 0;
			context->tsize -= next + 1;
			context->offset += next + 1;
		}
	} while(!next);

	return ptr;
}

const char *TTokenizerNext(TTokenizer *context) {
	if(!context) {
		TErrorReport(T_ERROR_NULL_POINTER);
		return 0;
	}

	if(!context->tsize) {
		//nothing in there
		return 0;
	}

	if(!context->buffer) {
		context->bsize = TMIN(context->tsize + 1,TTokenizerBufferMaxSize);
		context->buffer = TAlloc(context->bsize * sizeof(char));
		
		//feed buffer
		TRWReadBlock(context->content, context->buffer, context->bsize);
	}

	return prepareToken(context);
}
