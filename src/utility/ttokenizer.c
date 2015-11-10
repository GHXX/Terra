
#include "stdafx.h"

#include "ttokenizer.h"

#include "talloc.h"
#include "terror.h"

static const char *TTokenizerDefaultSeparators = " \n\t\r";
static const TSize TTokenizerBufferMaxSize = 512;

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
	if(!context) { TError(T_ERROR_INVALID_INPUT); }

	if(!separators)	separators = TTokenizerDefaultSeparators;
	context->separators = separators;
}

const char *prepareToken(TTokenizer *context) {
	char *ptr;
	TSize next;

	do {
		ptr = context->buffer + context->offset;

		//remove starting separators
		while(*ptr && strchr(context->separators, *ptr)) {
			ptr++;
			context->offset++;
		}

		next = strcspn(ptr, context->separators);
		if(*(ptr+next) == 0) {
			if(TRWEOF(context->content)) {
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
				TRWReadBlock(context->content, (unsigned char *)context->buffer + remainingSize, context->offset);
				context->offset = 0;
				remainingSize = context->bsize - 1;
				next = 0;
			}
		} else {
			*(ptr+next) = 0;
			if(context->tsize)
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

	if(TRWEOF(context->content)) {
		//nothing in there
		return 0;
	}

	if(!context->buffer) {
		context->bsize = TTokenizerBufferMaxSize;
		if(context->tsize && context->tsize < context->bsize)
			context->bsize = context->tsize;
		context->buffer = TAlloc(context->bsize * sizeof(char));

		//feed buffer
		TRWReadBlock(context->content, (unsigned char *) context->buffer, context->bsize);
	}

	return prepareToken(context);
}
