#include "stdafx.h"

#include "test_utils.h"

#include "utility/ttokenizer.h"

#include "test.h"

void tokenizer_test_1(void) {
	// uses a valid string
	const char *validString = "This is a valid string";
	const char *blocks[] = {
		"This",
		"is",
		"a",
		"valid",
		"string"
	};
	const char *token;
	int idx = 0;

	TStream *stream = TStreamFromConstMem(validString, strlen(validString));
	TTokenizer *tokenizer = TTokenizerNew(stream, 1);

	TTokenizerSetSeparators(tokenizer, " ");

	while ((token = TTokenizerNext(tokenizer, 0))) {
		TAssert(!strcmp(token, blocks[idx++]));
	}

	TTokenizerFree(tokenizer);
}

void tokenizer_test_2(void) {
	// uses a valid escaped string
	const char *validString = "This is\\ a valid\\ string";
	const char *blocks[] = {
		"This",
		"is a",
		"valid string",
	};
	const char *token;
	int idx = 0;

	TStream *stream = TStreamFromConstMem(validString, strlen(validString));
	TTokenizer *tokenizer = TTokenizerNew(stream, 1);

	TTokenizerSetEscapeCharacter(tokenizer, '\\');
	TTokenizerSetSeparators(tokenizer, " ");

	while ((token = TTokenizerNext(tokenizer, 0))) {
		TAssert(!strcmp(token, blocks[idx++]));
	}

	TTokenizerFree(tokenizer);
}

void tokenizer_test_3(void) {
	// uses a large string
	char *string;
	const char *token;

	string = TAlloc(sizeof(char) * (TBUFSIZE + 2));
	memset(string, 1, TBUFSIZE);
	string[TBUFSIZE + 1] = 0;

	TStream *stream = TStreamFromMem(string, TBUFSIZE + 2, 1);
	TTokenizer *tokenizer = TTokenizerNew(stream, 1);

	token = TTokenizerNext(tokenizer, 0);
	TAssert(!token);
	TAssert(TErrorGetCode() == T_ERROR_OUT_OF_MEMORY);

	TTokenizerFree(tokenizer);
}

void tokenizer_test_4(void) {
	// uses strings with adjacent separators
	const char *validString = "This  is\\ a  valid\\ string ";
	const char *blocks[] = {
		"This",
		"is a",
		"valid string"
	};
	const char *token;
	int idx = 0;

	TStream *stream = TStreamFromConstMem(validString, strlen(validString));
	TTokenizer *tokenizer = TTokenizerNew(stream, 1);

	TTokenizerSetEscapeCharacter(tokenizer, '\\');
	TTokenizerSetSeparators(tokenizer, " ");

	while ((token = TTokenizerNext(tokenizer, 0))) {
		TAssert(!strcmp(token, blocks[idx++]));
	}

	TTokenizerFree(tokenizer);
}

void tokenizer_test_5(void) {
	// uses strings with adjacent separators
	const char *validString = "This  is\\ a  valid\\ string ";
	const char *blocks[] = {
		"This",
		"",
		"is a",
		"",
		"valid string",
		"",
	};
	const char *token;
	int idx = 0;

	TStream *stream = TStreamFromConstMem(validString, strlen(validString));
	TTokenizer *tokenizer = TTokenizerNew(stream, 1);

	TTokenizerSetEscapeCharacter(tokenizer, '\\');
	TTokenizerSetSeparators(tokenizer, " ");
	TTokenizerSkipEmpty(tokenizer, 0);

	while ((token = TTokenizerNext(tokenizer, 0))) {
		TAssert(!strcmp(token, blocks[idx++]));
	}

	TTokenizerFree(tokenizer);
}

void tokenizer_test(void) {
	TLogWriteMain("Testing tokenizer ...\n");

	tokenizer_test_1();
	tokenizer_test_2();
	tokenizer_test_3();
	tokenizer_test_4();
	tokenizer_test_5();

	TLogWriteMain("tokenizer tests completed.\n");
}
