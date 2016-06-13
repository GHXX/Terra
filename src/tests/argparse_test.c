
#include "stdafx.h"

#include "test_utils.h"

static TUInt8 inputArg;
static TUInt8 pArg;
static TUInt8 mathArg;

void argparse_arguments(void)
{
	inputArg = TArgParserAddArgument("input", 'c', '1', 'z', 0);
	pArg = TArgParserAddArgument("-p", 's', '1', 0, 0);
	mathArg = TArgParserAddArgument("--math", 0, '0', 0, 0);
}

void argparse_init(void)
{
	static const char *content[] = {
		"Terra.exe",
		"-p",
		"bleh",
		"a",
		"--math"
	};
	TArgParserInit(5,content);
}

void argparse_run(void)
{
	const void *data;
	TUInt8 out = 0;

	while((out = TArgParserNext())) {
		if (out == inputArg) {
			data = TArgParserNextParameter();
			TAssert(!data);
		} else if (out == pArg) {
			data = TArgParserNextParameter();
			TAssert(data);
			TAssert(!strcmp("bleh", (const char *)data));
		} else if (out) {
			printf("%d\n", out);
		}
	}
	printf("%d", TErrorGet());
}

void argparse_test(void)
{
	argparse_init();

	argparse_arguments();

	argparse_run();

	TArgParserDestroy();
}
