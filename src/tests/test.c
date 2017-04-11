
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

	TRandTest();

	TNetworkTest();

	TStackTest();

	TTokenizerTest();

	rand_test();

	TLogWrite(testLog, "All Tests have been executed.\n");

	TLogFree(testLog);

	printf("Press any key to close this window...");
	getchar();

	return 0;
}
