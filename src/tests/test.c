
#include "stdafx.h"

#include "test.h"

#include "test_utils.h"

int main(int argc, char **argv)
{
	TLogSetFile(stdout);

	TLogReport(T_LOG_PROGRESS,0,"Running tests...\n");

	//array_test();

	//rbtree_test();

	//memleak_test();

	//stack_test();

	hardware_test();

	//argparse_test();

	//string_test();

	//filesys_test();

	stream_test();

	tokenizer_test();

	TLogReport(T_LOG_PROGRESS,0,"All Tests have been executed.\n");

	return 0;
}
