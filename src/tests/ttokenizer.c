#include "stdafx.h"

#include "ttest.h"

#include "utility/ttokenizer.h"
#include "talloc.h"
#include "terror.h"

#include "test.h"

int tokenizer_test_1(void) {
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
		if (strcmp(token, blocks[idx++])) {
			TTokenizerFree(tokenizer);
			TTestMustValidate(1);
		}
	}

	TTokenizerFree(tokenizer);

	return 0;
}

int tokenizer_test_2(void) {
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
		if (strcmp(token, blocks[idx++])) {
			TTokenizerFree(tokenizer);
			TTestMustValidate(1);
		}
	}

	TTokenizerFree(tokenizer);

	return 0;
}

int tokenizer_test_3(void) {
	// uses a large string
	char *string;
	const char *token;
	char sep;

	string = TAlloc(sizeof(char) * (TBUFSIZE + 2));
	memset(string, 1, TBUFSIZE);
	string[TBUFSIZE + 1] = 0;

	TStream *stream = TStreamFromMem(&string, TBUFSIZE + 2, 1);
	TTokenizer *tokenizer = TTokenizerNew(stream, 1);

	token = TTokenizerNext(tokenizer, &sep);
	if (!token) {
		TTokenizerFree(tokenizer);
		TTestMustValidate(1);
	}

	TTestMustValidate(!sep);

	return 0;
}

int tokenizer_test_4(void) {
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
		if (strcmp(token, blocks[idx++])) {
			TTokenizerFree(tokenizer);
			TTestMustValidate(1);
		}
	}

	TTokenizerFree(tokenizer);

	return 0;
}

int tokenizer_test_5(void) {
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
		if (strcmp(token, blocks[idx++])) {
			TTokenizerFree(tokenizer);
			TTestMustValidate(1);
		}
	}

	TTokenizerFree(tokenizer);

	return 0;
}

void TTokenizerTest(void) {

	TestFunc tests[] = {
		tokenizer_test_1,
		tokenizer_test_2,
		tokenizer_test_3,
		tokenizer_test_4,
		tokenizer_test_5,

	};

	TTestRun("Tokenizer", tests, sizeof(tests) / sizeof(TestFunc));
}
