
#include "stdafx.h"

#include "ttokenizer.h"

#include "talloc.h"
#include "terror.h"
#include "tstring.h"

enum T_TOKENIZER_FLAGS {
	T_TOKENIZER_FLAG_FREE_CONTENT = 0x1,
	T_TOKENIZER_FLAG_SKIP = 0x2,
	T_TOKENIZER_FLAG_EMPTY = 0x4,
};

struct TTokenizer {
	TStream *content;
	TUInt8 flags;

	const char *separators;
	char escapeChar;
	char *control;

	const char *leadStrip;
	const char *trailStrip;

	unsigned char *buffer;
	const unsigned char *token;
	TSize bSize;
	TSize used;
	TSize offset;
};

static inline void TTokenizerBufferize(TTokenizer *context, TSize remainingSize) {
	TSize size = (context->bSize - 1) - remainingSize;

	context->used = TStreamReadBlock(context->content, context->buffer + remainingSize, size);

	context->used += remainingSize;
	context->buffer[context->used] = 0;
}

static inline char TTokenizerSetupBuffer(TTokenizer *context) {
	if (!context->buffer) {
		TSize tSize = TStreamSize(context->content);

		context->bSize = TBUFSIZE;
		if (tSize && tSize + 1 < context->bSize) {
			context->bSize = tSize + 1;
		}
		context->buffer = TAllocNData(unsigned char, context->bSize);

		//feed buffer
		TTokenizerBufferize(context, 0);
		context->buffer[context->bSize - 1] = 0;

		if (!context->bSize) return 1;
	}

	return 0;
}

TTokenizer *TTokenizerNew(TStream *input, TInt8 freeInput)
{
	if(input) {
		TTokenizer *tokenizer = TAllocData(TTokenizer);
		if(tokenizer) {
			memset(tokenizer, 0, sizeof(TTokenizer));
			tokenizer->content = input;
			tokenizer->flags |= freeInput ? T_TOKENIZER_FLAG_FREE_CONTENT : 0;
			tokenizer->separators = 0;
		}

		return tokenizer;
	}

	TErrorZero(T_ERROR_INVALID_INPUT);
}

void TTokenizerFree(TTokenizer *context) {
	if(context) {
		if (context->flags & T_TOKENIZER_FLAG_FREE_CONTENT) TStreamFree(context->content);
		TFree(context->control);
		TFree(context->buffer);
		TFree(context);
	}
}

void TTokenizerSeek(TTokenizer *context, TLInt offset, TInt8 origin) {
	if (!context) { TError(T_ERROR_INVALID_INPUT); }

	TStreamSeek(context->content, offset, origin);
	if (!context->buffer) TTokenizerSetupBuffer(context);
	else TTokenizerBufferize(context, 0);
	context->offset = 0;
}

TLInt TTokenizerTell(TTokenizer *context) {
	TLInt pos;

	if (!context) { TErrorZero(T_ERROR_INVALID_INPUT); }

	pos = TStreamTell(context->content);
	pos -= context->used - context->offset;

	return pos;
}

void TTokenizerSkipEmpty(TTokenizer *context, TInt8 skip) {
	if (!context) { TError(T_ERROR_INVALID_INPUT); }

	TBIT_CLEAR(context->flags, T_TOKENIZER_FLAG_SKIP);
	if(skip) context->flags |= T_TOKENIZER_FLAG_SKIP;
}

void TTokenizerSetStrip(TTokenizer *context, const char *leadStrip, const char *trailStrip) {
	if (!context) { TError(T_ERROR_INVALID_INPUT); }

	context->leadStrip = leadStrip;
	context->trailStrip = trailStrip;
}

void TTokenizerSetEscapeCharacter(TTokenizer *context, char escapeChar) {
	if (!context) { TError(T_ERROR_INVALID_INPUT); }

	context->escapeChar = escapeChar;
}

void TTokenizerSetSeparators(TTokenizer *context, const char *separators) {
	if(!context) { TError(T_ERROR_INVALID_INPUT); }
	context->separators = separators;
	if (separators) context->flags |= T_TOKENIZER_FLAG_SKIP;
	else TBIT_CLEAR(context->flags, T_TOKENIZER_FLAG_SKIP);
}

static inline void TTokenizerStrip(TTokenizer *context, unsigned char **ptr, unsigned char *next) {
	if (context->leadStrip) {
		while (strchr(context->leadStrip, **ptr)) (*ptr)++;
	}

	if (context->trailStrip) {
		do next--; while (strchr(context->trailStrip, *next));
	}
}

const unsigned char *TTokenizerPrepareToken(TTokenizer *context, char *separator) {
	unsigned char *ptr;
	unsigned char *next;
	
	ptr = context->buffer + context->offset;

	if (context->flags & T_TOKENIZER_FLAG_SKIP) {
		//remove starting separators
		while (*ptr && strchr(context->separators, *ptr)) {
			ptr++;
			context->offset++;
		}
	}

	next = ptr;

	do {
		next = (unsigned char *)strpbrk((char *)next, context->control);
		if(!next) {
			if(TStreamEOF(context->content)) {
				// reached the end
				context->flags |= T_TOKENIZER_FLAG_EMPTY;
				context->offset = context->used + 1;
				return !*ptr && context->flags & T_TOKENIZER_FLAG_SKIP ? 0 : ptr;
			}

			if(!context->offset) {
				// we are at the beginning and no separator can be found. Return the token with a null separator
				context->offset = context->used + 1;
				TTokenizerStrip(context, &ptr, next);
				return ptr;
			}

			{
				TSize remainingSize = 0;
				//push back remaining data
				if (context->offset < context->used) {
					remainingSize = (context->used - context->offset);
					memcpy(context->buffer, ptr, remainingSize * sizeof(unsigned char));
				}
				// advance the buffer
				TTokenizerBufferize(context, remainingSize);
			}

			ptr = context->buffer;
			context->offset = 0;
			next = ptr;

		} else if (*next == context->escapeChar) {
			TSize shiftSize;

			// shift the data to the left
			shiftSize = (context->used + (context->buffer - next));
			memcpy(next, next + 1, shiftSize);
			next++;
			context->buffer[context->used - 1] = 0;
		} else {
			*separator = *next;
			*next = 0;
			context->offset += (next - ptr) + 1;

			TTokenizerStrip(context, &ptr, next);
			return ptr;
		}
	} while(1);

	return 0;
}

const unsigned char *TTokenizerGet(TTokenizer *context) {
	if (!context) { TErrorZero(T_ERROR_NULL_POINTER); }

	return context->token;
}

const unsigned char *TTokenizerNext(TTokenizer *context, char *separator) {
	char dummy;

	//sanity control for separator
	if (!separator) separator = &dummy;
	*separator = 0;

	if (!context) { TErrorZero(T_ERROR_NULL_POINTER); }

	if (context->flags & T_TOKENIZER_FLAG_EMPTY) {
		if (TStreamEOF(context->content)) return 0;
		TBIT_CLEAR(context->flags, T_TOKENIZER_FLAG_EMPTY);
	}

	if (TTokenizerSetupBuffer(context)) return 0;

	//build control if needed
	if (!context->control) {
		if (context->separators) {
			if (context->escapeChar)
				context->control = TStringAppendCharacter(context->separators, context->escapeChar);
			else
				context->control = TStringCopy(context->separators);
		} else {
			context->control = TStringCopy("");
		}
	}

	context->token = TTokenizerPrepareToken(context, separator);
	return context->token;
}

const unsigned char *TTokenizerJump(TTokenizer *context, char separator) {
	char control[3] = {0};
	char *tmp;

	if (!context) {
		TErrorZero(T_ERROR_NULL_POINTER);
	}

	if (context->flags & T_TOKENIZER_FLAG_EMPTY) {
		if (TStreamEOF(context->content)) return 0;
		TBIT_CLEAR(context->flags, T_TOKENIZER_FLAG_EMPTY);
	}

	if (TTokenizerSetupBuffer(context)) return 0;

	//build temporary control
	control[0] = separator;
	control[1] = context->escapeChar;

	tmp = context->control;
	context->control = control;
	context->token = TTokenizerPrepareToken(context, &separator);
	context->control = tmp;


	return context->token;
}
