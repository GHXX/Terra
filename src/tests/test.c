
#include "stdafx.h"

#include "test.h"

#include "ttest.h"



int main(int argc, char **argv)
{
	testLog = TLogNewStream(TStreamFromFilePointer(stdout, 0));

	TLogWrite(testLog, "Running tests...\n");

	//array_test();

	//rbtree_test();

	//memleak_test();

	hardware_test();

	//argparse_test();

	TFileSysTest();

	stream_test();

	TEncodingTest();

	TStringTest();

	TNetworkTest();

	TStackTest();

	TTokenizerTest();

	TLogWrite(testLog, "All Tests have been executed.\n");

	TLogFree(testLog);

	return 0;
}
