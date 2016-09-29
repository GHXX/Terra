#include "stdafx.h"

#include "ttest.h"

#include "test.h"

int TEncodingTestNormalASCII(void) {
	const unsigned char *data = "ASCII String";

	TEncodingStats *stats = TEncodingGetStats(data, (strlen(data) + 1) * sizeof(unsigned char));

	TTestValidate(stats->encoding == T_ENCODING_ASCII);
	TTestValidate(stats->numChars == strlen(data));
	TTestValidate(stats->numOneByteChars == stats->numChars);
	TTestValidate(stats->numTwoByteChars == 0);
	TTestValidate(stats->numThreeByteChars == 0);
	TTestValidate(stats->numFourByteChars == 0);
	TTestValidate(stats->flags == (T_ENCODING_FLAG_NULL_TERMINATED | T_ENCODING_FLAG_VALID));

	TFree(stats);

	return 0;
}

int TEncodingTestASCIINoNullEnding(void) {
	const unsigned char *data = "ASCII String";

	TEncodingStats *stats = TEncodingGetStats(data, strlen(data) * sizeof(unsigned char));

	TTestValidate(stats->encoding == T_ENCODING_ASCII);
	TTestValidate(stats->numChars == strlen(data));
	TTestValidate(stats->numOneByteChars == stats->numChars);
	TTestValidate(stats->numTwoByteChars == 0);
	TTestValidate(stats->numThreeByteChars == 0);
	TTestValidate(stats->numFourByteChars == 0);
	TTestValidate(stats->flags == T_ENCODING_FLAG_VALID);

	TFree(stats);

	return 0;
}

int TEncodingTestASCIIEmpty(void) {
	const unsigned char *data = "";

	TEncodingStats *stats = TEncodingGetStats(data, 1);

	TTestValidate(stats->encoding == T_ENCODING_ASCII);
	TTestValidate(stats->numChars == 0);
	TTestValidate(stats->numOneByteChars == stats->numChars);
	TTestValidate(stats->numTwoByteChars == 0);
	TTestValidate(stats->numThreeByteChars == 0);
	TTestValidate(stats->numFourByteChars == 0);
	TTestValidate(stats->flags == (T_ENCODING_FLAG_NULL_TERMINATED | T_ENCODING_FLAG_VALID));

	TFree(stats);

	return 0;
}

int TEncodingTestUTF8Normal(void) {
	unsigned char *data = "UTF8 String: \xC2\xA9st";

	TEncodingStats *stats = TEncodingGetStats(data, (strlen(data) + 1) * sizeof(unsigned char));

	TTestValidate(stats->encoding == T_ENCODING_UTF8);
	TTestValidate(stats->numChars == strlen(data) - 1);
	TTestValidate(stats->numOneByteChars == stats->numChars - 1);
	TTestValidate(stats->numTwoByteChars == 1);
	TTestValidate(stats->numThreeByteChars == 0);
	TTestValidate(stats->numFourByteChars == 0);
	TTestValidate(stats->flags == (T_ENCODING_FLAG_NULL_TERMINATED | T_ENCODING_FLAG_VALID));

	TFree(stats);

	return 0;
}

int TEncodingTestUTF8Truncated(void) {
	unsigned char *data = "UTF8 String: \xC2";

	TEncodingStats *stats = TEncodingGetStats(data, (strlen(data)) * sizeof(unsigned char));

	TTestValidate(TErrorGet() == T_ENCODING_ERROR_TRUNCATED);
	TTestValidate(!(stats->flags & T_ENCODING_FLAG_VALID));
	TTestValidate(!(stats->flags & T_ENCODING_FLAG_NULL_TERMINATED));

	TFree(stats);

	return 0;
}

int TEncodingTestUTF8WithBOM(void) {
	const unsigned char *utf8BOM = TEncodingGetBOM(T_ENCODING_UTF8);
	unsigned char data[32];
	TEncodingStats *stats;
	
	snprintf(data, sizeof(data), "%sUTF8 String: \xC2\xA9", utf8BOM);

	stats = TEncodingGetStats(data, (strlen(data) + 1) * sizeof(unsigned char));

	TTestValidate(stats->encoding == T_ENCODING_UTF8);
	TTestValidate(stats->numChars == strlen(data) - 1 - TEncodingGetBOMSize(stats->encoding));
	TTestValidate(stats->numOneByteChars == stats->numChars - 1);
	TTestValidate(stats->numTwoByteChars == 1);
	TTestValidate(stats->numThreeByteChars == 0);
	TTestValidate(stats->numFourByteChars == 0);
	TTestValidate(stats->flags == (T_ENCODING_FLAG_BOM_PRESENT | T_ENCODING_FLAG_NULL_TERMINATED | T_ENCODING_FLAG_VALID));

	TFree(stats);

	return 0;
}

int TEncodingTestNormalASCIIStream(void) {
	const unsigned char *data = "ASCII String";
	TStream *stream = TStreamFromConstMem(data, (strlen(data) + 1) * sizeof(unsigned char));

	TEncodingStats *stats = TEncodingGetStreamStats(stream);

	TTestValidate(stats->encoding == T_ENCODING_ASCII);
	TTestValidate(stats->numChars == strlen(data));
	TTestValidate(stats->numOneByteChars == stats->numChars);
	TTestValidate(stats->numTwoByteChars == 0);
	TTestValidate(stats->numThreeByteChars == 0);
	TTestValidate(stats->numFourByteChars == 0);
	TTestValidate(stats->flags == (T_ENCODING_FLAG_NULL_TERMINATED | T_ENCODING_FLAG_VALID));

	TFree(stats);
	TStreamFree(stream);

	return 0;
}

int TEncodingTestNormalUTF8Stream(void) {
	unsigned char *data = "UTF8 String: \xC2\xA9st";
	TStream *stream = TStreamFromConstMem(data, (strlen(data) + 1) * sizeof(unsigned char));

	TEncodingStats *stats = TEncodingGetStreamStats(stream);

	TTestValidate(stats->encoding == T_ENCODING_UTF8);
	TTestValidate(stats->numChars == strlen(data) - 1);
	TTestValidate(stats->numOneByteChars == stats->numChars - 1);
	TTestValidate(stats->numTwoByteChars == 1);
	TTestValidate(stats->numThreeByteChars == 0);
	TTestValidate(stats->numFourByteChars == 0);
	TTestValidate(stats->flags == (T_ENCODING_FLAG_NULL_TERMINATED | T_ENCODING_FLAG_VALID));

	TFree(stats);
	TStreamFree(stream);

	return 0;
}

int TEncodingTestUTF8TruncatedStream(void) {
	unsigned char *data = "UTF8 String: \xC2";
	TStream *stream = TStreamFromConstMem(data, (strlen(data)) * sizeof(unsigned char));

	TEncodingStats *stats = TEncodingGetStreamStats(stream);

	TTestValidate(TErrorGet() == T_ENCODING_ERROR_TRUNCATED);
	TTestValidate(!(stats->flags & T_ENCODING_FLAG_VALID));
	TTestValidate(!(stats->flags & T_ENCODING_FLAG_NULL_TERMINATED));

	TFree(stats);
	TStreamFree(stream);

	return 0;
}

int TEncodingTestUTF8WithBOMStream(void) {
	const unsigned char *utf8BOM = TEncodingGetBOM(T_ENCODING_UTF8);
	unsigned char data[32];
	TStream *stream;
	TEncodingStats *stats;

	snprintf(data, sizeof(data), "%sUTF8 String: \xC2\xA9", utf8BOM);
	stream = TStreamFromConstMem(data, (strlen(data) + 1) * sizeof(unsigned char));

	stats = TEncodingGetStreamStats(stream);

	TTestValidate(stats->encoding == T_ENCODING_UTF8);
	TTestValidate(stats->numChars == strlen(data) - 1 - TEncodingGetBOMSize(stats->encoding));
	TTestValidate(stats->numOneByteChars == stats->numChars - 1);
	TTestValidate(stats->numTwoByteChars == 1);
	TTestValidate(stats->numThreeByteChars == 0);
	TTestValidate(stats->numFourByteChars == 0);
	TTestValidate(stats->flags == (T_ENCODING_FLAG_BOM_PRESENT | T_ENCODING_FLAG_NULL_TERMINATED | T_ENCODING_FLAG_VALID));

	TFree(stats);

	return 0;
}

int TEncodingTestASCIIToUTF16LE(void) {
	const unsigned char *data = "ASCII String";
	unsigned char *result, *ptr;
	TSize i, size, out;
	unsigned char expRes[] = {
		65, 0,
		83, 0,
		67, 0,
		73, 0,
		73, 0,
		32, 0,
		83, 0,
		116, 0,
		114, 0,
		105, 0,
		110, 0,
		103, 0,
		0, 0,
	};

	TEncodingStats *stats;

	size = (strlen(data) + 1) * sizeof(unsigned char);
	stats = TEncodingGetStats(data, size);

	result = TEncodingToUTF16LE(data, size, &out, stats);

	TTestValidate(out == size * 2);

	ptr = result;
	for (i = 0; i < out; ++i) {
		TTestValidate(*(ptr++) == expRes[i]);
	}

	TFree(result);
	TFree(stats);

	return 0;
}

int TEncodingTestUTF8ToUTF16LE(void) {
	const unsigned char *data = "\xC5\xB6 \xE0\xB8\x9C \xF0\xA8\x9B\xB9";
	unsigned char *result, *ptr;
	TSize i, size, out;
	unsigned char expRes[] = {
		118, 1,
		32, 0,
		28, 14,
		32, 0,
		97, 216, 249, 222,
		0, 0,
	};

	TEncodingStats *stats;
	size = (strlen(data) + 1) * sizeof(unsigned char);
	stats = TEncodingGetStats(data, size);

	result = TEncodingToUTF16LE(data, size, &out, stats);

	wchar_t *validate = (wchar_t *)result;
	wprintf(validate);

	TTestValidate(out == 14);

	ptr = result;
	for (i = 0; i < out; ++i) {
		TTestValidate(*(ptr++) == expRes[i]);
	}

	TFree(result);
	TFree(stats);

	return 0;
}

int TEncodingTestGetChUTF8(void) {
	const unsigned char *data = "\xC5\xB6 \xE0\xB8\x9C \xF0\xA8\x9B\xB9", *ptr;
	TSize size, i;
	TUInt32 res;
	TUInt32 expRes[] = {
		374,
		32,
		3612,
		32,
		165625
	};

	size = (strlen(data) + 1) * sizeof(unsigned char);
	ptr = data;
	i = 0;

	while (res = TEncodingUTF8GetChr(&ptr, &size)) {
		TTestValidate(res == expRes[i++]);
	}

	return 0;
}

int TEncodingTestGetPreChUTF8(void) {
	const unsigned char *data = "\xC5\xB6 \xE0\xB8\x9C \xF0\xA8\x9B\xB9", *ptr;
	TSize size, i;
	TUInt32 res;
	TUInt32 expRes[] = {
		374,
		32,
		3612,
		32,
		165625
	};

	size = (strlen(data) + 1) * sizeof(unsigned char);
	ptr = data + size - 1;
	i = sizeof(expRes) / sizeof(TUInt32) - 1;
	size = 0;

	while (i > 0) {
		res = TEncodingUTF8GetPreviousChr(&ptr, &size);
		TTestValidate(res == expRes[i--]);
	}

	res = TEncodingUTF8GetPreviousChr(&ptr, &size);
	TTestValidate(res == expRes[i]);
	TTestValidate(ptr == data);

	return 0;
}

int TEncodingTestIncrementASCII(void) {
	const unsigned char *data = "ASCII String", *ptr;
	TSize size = (strlen(data) + 1) * sizeof(unsigned char);

	ptr = data;
	TEncodingASCIIIncrement(&ptr, &size, 1);
	TTestValidate(size == (strlen(data)) * sizeof(unsigned char));
	TTestValidate(ptr == data + 1);

	TEncodingASCIIIncrement(&ptr, &size, -1);
	TTestValidate(size == (strlen(data) + 1) * sizeof(unsigned char));
	TTestValidate(ptr == data);

	TEncodingASCIIIncrement(&ptr, &size, size);
	TTestValidate(size == 0);

	size = (strlen(data) + 1) * sizeof(unsigned char);
	TTestValidate(ptr == data + size);

	TEncodingASCIIIncrement(&ptr, &size, 0);
	TTestValidate(size == (strlen(data) + 1) * sizeof(unsigned char));
	TTestValidate(ptr == data + size);

	return 0;
}

int TEncodingTestIncrementUTF8(void) {
	const unsigned char *data = "\xC5\xB6 \xE0\xB8\x9C \xF0\xA8\x9B\xB9", *ptr;
	TSize size = (strlen(data) + 1) * sizeof(unsigned char);

	ptr = data;
	TEncodingUTF8Increment(&ptr, &size, 1);
	TTestValidate(ptr == data + 2);

	TEncodingUTF8Increment(&ptr, &size, -1);
	TTestValidate(ptr == data);

	TEncodingUTF8Increment(&ptr, &size, 4);
	TTestValidate(ptr == data + 7);

	TEncodingUTF8Increment(&ptr, &size, 0);
	TTestValidate(ptr == data + 7);

	return 0;
}

void TEncodingTest(void) {
	TestFunc tests[] = {
		TEncodingTestNormalASCII,
		TEncodingTestASCIINoNullEnding,
		TEncodingTestASCIIEmpty,

		TEncodingTestUTF8Normal,
		TEncodingTestUTF8Truncated,
		TEncodingTestUTF8WithBOM,

		TEncodingTestNormalASCIIStream,

		TEncodingTestNormalUTF8Stream,
		TEncodingTestUTF8TruncatedStream,
		TEncodingTestUTF8WithBOMStream,

		TEncodingTestASCIIToUTF16LE,
		TEncodingTestUTF8ToUTF16LE,

		TEncodingTestGetChUTF8,
		TEncodingTestGetPreChUTF8,

		TEncodingTestIncrementASCII,
		TEncodingTestIncrementUTF8,
	};

	TTestRun("encoding", tests, sizeof(tests) / sizeof(TestFunc));
}
