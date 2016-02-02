#include "stdafx.h"

#include "io/tstream.h"
#include "utility/tfilesys.h"
#include "terror.h"

#include "utility/tstring.h"

#include "test_utils.h"

#include "test.h"

void test_rw_file(void)
{
	const char data[] = "a 42 this is        a test";
	const char filename[] = "test_file_for_testing";
	TStream *testrw;
	unsigned char character = 0;
	unsigned char buffer[32];
	TSize sizeRead;

	test_create_file(filename, data);

	testrw = TStreamFromFile(filename, "rb");
	TAssert(testrw);

	TAssert(TStreamSize(testrw) == strlen(data));

	character = TStreamRead8(testrw);
	TAssert(character == 'a');

	sizeRead = TStreamReadBlock(testrw, buffer, 32);
	buffer[sizeRead] = 0;

	TAssert(!strcmp(data + 1, (char *)buffer));

	TStreamSeek(testrw,0,SEEK_SET);

	TStreamWriteBlock(testrw, "bleh", sizeof(char) * 4);
	TAssert(TErrorGetCode() == T_ERROR_OPERATION_NOT_SUPPORTED);

	TStreamFree(testrw);
	TFileSysDelete(filename);
}

void test_rw_buffer(void)
{
	const char data[] = "amazing sample";
	TStream *testrw;
	unsigned char buf[32];
	char *cpy = TStringCopy(data);

	testrw = TStreamFromMem((unsigned char *) cpy, sizeof(data), 1);
	TAssert(testrw);
	TAssert(TStreamSize(testrw) == sizeof(data));

	TStreamReadBlock(testrw, buf, 32);
	TAssert(!strcmp(cpy, (char *)buf));

	TStreamSeek(testrw,0,SEEK_SET);
	TStreamWriteBlock(testrw, (const unsigned char *)"4m4z1ng", 6);
	TStreamSeek(testrw,0,SEEK_SET);
	TStreamReadBlock(testrw, buf, 32);

	TAssert(!strcmp("4m4z1ng sample", (char *)buf));

	TStreamFree(testrw);
}

void stream_test(void)
{
	test_rw_file();
	test_rw_buffer();
}
