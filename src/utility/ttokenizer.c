
#include "stdafx.h"

#include "ttokenizer.h"

#include "talloc.h"
#include "terror.h"
#include "tstring.h"


static const char *TTokenizerDefaultSeparators = " \n\t\r";

struct TTokenizer {
	TStream *content;
	TUInt8 flags;             // [ 0, 0, 0, 0, 0, empty, skip, freeContent ]

	const char *separators;
	char escapeChar;
	char *control;

	unsigned char *buffer;
	TSize bSize;
	TSize offset;
};

TTokenizer *TTokenizerNew(TStream *input, TInt8 freeInput)
{
	if(input) {
		TTokenizer *tokenizer = TAllocData(TTokenizer);
		if(tokenizer) {
			memset(tokenizer, 0, sizeof(struct TTokenizer));
			tokenizer->content = input;
			tokenizer->flags |= freeInput ? 0x3 : 0x2;
			tokenizer->separators = TTokenizerDefaultSeparators;
		}

		return tokenizer;
	}

	TErrorReport(T_ERROR_INVALID_INPUT);
	return 0;
}

void TTokenizerFree(TTokenizer *context) {
	if(context) {
		if (context->flags & 0x1) TStreamFree(context->content);
		TFree(context->control);
		TFree(context->buffer);
		TFree(context);
	}
}

void TTokenizerSkipEmpty(TTokenizer *context, TInt8 skip) {
	if (!context) { TError(T_ERROR_INVALID_INPUT); }

	context->flags &= ~0x2;
	context->flags |= skip ? 0x2 : 0;
}

void TTokenizerSetEscapeCharacter(TTokenizer *context, char escapeChar) {
	if (!context) { TError(T_ERROR_INVALID_INPUT); }

	context->escapeChar = escapeChar;
}

void TTokenizerSetSeparators(TTokenizer *context, const char *separators) {
	if(!context) { TError(T_ERROR_INVALID_INPUT); }

	if(!separators)	separators = TTokenizerDefaultSeparators;
	context->separators = separators;
}

const char *TTokenizerPrepareToken(TTokenizer *context, char *separator) {
	char *ptr;
	char *next;
	
	ptr = (char *)(context->buffer + context->offset);

	if (context->flags & 0x2) {
		//remove starting separators
		while (*ptr && strchr(context->separators, *ptr)) {
			ptr++;
			context->offset++;
		}
	}

	next = ptr;

	do {
		next = strpbrk(next, context->control);
		if(!next) {
			if(TStreamEOF(context->content)) {
				// reached the end
				context->flags |= 0x4;
				return !*ptr && context->flags & 0x2 ? 0 : ptr;
			}

			if(!context->offset) {
				// we are at the beginning and no separator can be found. Just kill this
				TErrorReport(T_ERROR_OUT_OF_MEMORY);
				context->flags |= 0x4;
				return 0;
			}

			{
				// advance the buffer
				TSize remainingSize = (context->bSize - 1 - context->offset);
				memcpy(context->buffer, ptr, remainingSize * sizeof(char));
				context->bSize = TStreamReadBlock(context->content, context->buffer + remainingSize, context->offset);
				context->buffer[remainingSize + context->bSize] = 0;
			}

			ptr = context->buffer;
			context->offset = 0;
			next = ptr;

		} else if (*next == context->escapeChar) {
			TSize shiftSize;

			// shift the data to the left
			shiftSize = (context->bSize + (context->buffer - next));
			memcpy(next, next + 1, shiftSize);
			next++;
			context->buffer[context->bSize - 1] = 0;
		} else {
			*separator = *next;
			*next = 0;
			context->offset += (next - ptr) + 1;
			return ptr;
		}
	} while(1);

	return 0;
}

const char *TTokenizerNext(TTokenizer *context, char *separator) {
	char dummy;

	if (!context) {
		TErrorReport(T_ERROR_NULL_POINTER);
		return 0;
	}

	if (context->flags & 0x4) {
		//nothing in there
		return 0;
	}

	if (!context->buffer) {
		TSize tSize = TStreamSize(context->content);

		context->bSize = TBUFSIZE;
		if (tSize && tSize + 1 < context->bSize) {
			context->bSize = tSize + 1;
		}
		context->buffer = TAlloc(context->bSize * sizeof(char));

		//feed buffer
		context->bSize = TStreamReadBlock(context->content, context->buffer, context->bSize - 1);
		context->buffer[context->bSize] = 0;

		if (!context->bSize) return 0;
	}

	
	//sanity control for separator
	if (!separator) separator = &dummy;
	*separator = 0;

	//build control if needed
	if (!context->control) {
		if (context->escapeChar)
			context->control = TStringAppendCharacter(context->separators, context->escapeChar);
		else
			context->control = TStringCopy(context->separators);
	}

	return TTokenizerPrepareToken(context, separator);
}
