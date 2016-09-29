
#include "stdafx.h"

#include "test.h"

#include "ttest.h"



int main(int argc, char **argv)
{
	testLog = TLogNewStream(TStreamFromFilePointer(stdin, 0));

	TLogWrite(testLog, "Running tests...\n");

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

	TLogWrite(testLog, "All Tests have been executed.\n");

	TLogFree(testLog);

	return 0;
}
