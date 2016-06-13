
#include "stdafx.h"

#include "test.h"

#include "test_utils.h"

int main(int argc, char **argv)
{
	TLogInit(0);

	TLogWriteMain("Running tests...\n");

	//array_test();

	//rbtree_test();

	//memleak_test();

	//stack_test();

	hardware_test();

	//argparse_test();

	//filesys_test();

	stream_test();

	TEncodingTest();

	TStringTest();

	tokenizer_test();

	TLogWriteMain("All Tests have been executed.\n");

	TLogDestroy();

	return 0;
}
