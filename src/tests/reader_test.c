#include "stdafx.h"

#include "io/trw.h"
#include "utility/tfilesys.h"
#include "terror.h"

#include "utility/tstring.h"

#include "test_utils.h"

#include "reader_test.h"

void test_rw_file(void)
{
	const char data[] = "a 42 this is        a test";
	const char filename[] = "test_file_for_testing";
	TRW *testrw;
	unsigned char character = 0;
	char buffer[32];

	test_create_file(filename,data);

	testrw = TRWFromFile(filename,"rb");
	TAssert(testrw);

	TAssert(TRWSize(testrw) == strlen(data));

	character = TRWRead8(testrw);
	TAssert(character == 'a');

	TRWReadBlock(testrw,buffer,32);

	TAssert(!strcmp(data+1,buffer));

	TRWSeek(testrw,0,SEEK_SET);

	TRWWrite(testrw,"bleh");
	TAssert(TErrorGetCode() == T_ERROR_OPERATION_NOT_SUPPORTED);

	TRWFree(testrw);
	TFileSysDelete(filename);
}

void test_rw_buffer(void)
{
	const char data[] = "amazing sample";
	TRW *testrw;
	char buf[32];
	char *cpy = TStringCopy(data);

	testrw = TRWFromMem(cpy, sizeof(data), 1);
	TAssert(testrw);
	TAssert(TRWSize(testrw) == sizeof(data));

	TRWReadBlock(testrw,buf,32);
	TAssert(!strcmp(cpy,buf));

	TRWSeek(testrw,0,SEEK_SET);
	TRWWriteBlock(testrw,"4m4z1ng", 6);
	TRWSeek(testrw,0,SEEK_SET);
	TRWReadBlock(testrw,buf,32);

	TAssert(!strcmp("4m4z1ng sample",buf));

	TRWFree(testrw);
}

void rw_test(void)
{
	test_rw_file();
	test_rw_buffer();
}
