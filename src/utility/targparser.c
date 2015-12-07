
#include "stdafx.h"

#include "targparser.h"

#include "talloc.h"
#include "terror.h"

#include "structure/tarray.h"

#include "tstring.h"

#include <limits.h>

typedef struct {
	const char *name;

	TUInt8 type;
	TUInt8 amount;
	TUInt8 def;

	const char *help;

	unsigned char used : 1;
} TArg;

typedef struct {
	TPtr param;
	size_t remainingParams;
	char paramtype;
	char paramdef;
} TParamHolder;

static struct {
	TArray *argFormat;

	TParamHolder ph;

	const char **argv;
	int argc;

	int idx;
} TArgParser;

void TArgParserInit(int argc, const char **argv)
{
	memset(&TArgParser,0, sizeof(TArgParser));
	TArgParser.argFormat = TArrayNew(1);
	TArgParser.argc = argc - 1;
	TArgParser.argv = argv + 1;
}

void TArgParserDestroy(void)
{
	TArrayFree(TArgParser.argFormat,TFree);
	TFree(TArgParser.ph.param);
}

TUInt8 TArgParserAddArgument(const char *name, TUInt8 type, TUInt8 amount, TUInt8 def, const char *help)
{
	TArg *arg;

	if (TArgParser.argFormat->len > UCHAR_MAX - 2) {
		TErrorReport(T_ERROR_SIZE_EXCEEDED);
		return -1;
	}

	arg = (TArg *) TAlloc(sizeof(TArg));
	if(arg) {
		arg->name = name;
		arg->type = type;
		arg->amount = amount;
		arg->def = def;
		arg->help = help;
		arg->used = 0; 

		return (TUInt8) TArrayAppend(TArgParser.argFormat, arg);
	}

	return -1;
}

void TArgParserHelp(void)
{
	//TODO
	//fprintf(stdout,"");
}

static inline unsigned char TArgParserIsSwitch(const char *arg)
{
	return (*arg == '-');
}

static TArg *TArgParserMatchSwitch(const char *pattern, TUInt8 *index)
{
	TArg *c = 0;
	void **data;
	TUInt8 len, idx = 0;

	data = TArgParser.argFormat->data;
	len = (TUInt8) TArgParser.argFormat->len;
	
	while (idx < len) {
		c = (TArg *) data[idx++];
		if (c->name && !strcmp(c->name, pattern)) {
			if (index) *index = idx - 1;
			return c;
		}
	}

	return 0;
}

static TArg *TArgParserMatchNonSwitch(TUInt8 *index)
{
	TArg *c = 0;
	void **data;
	TUInt8 len, idx = 0;

	data = TArgParser.argFormat->data;
	len = (TUInt8) TArgParser.argFormat->len;
	
	while (idx < len) {
		c = (TArg *) data[idx++];
		if (c->name && !TArgParserIsSwitch(c->name)) {
			if (index) *index = idx - 1;
			return c;
		}
	}

	return 0;
}

static TUInt32 TArgParserNumData(int idx, TUInt32 limit)
{
	int peek = idx;
	TUInt32 amount = 0;

	while(peek < TArgParser.argc) {
		if(TArgParserIsSwitch(TArgParser.argv[peek++])) break;
		amount++;
		if(amount >= limit) break;
	}

	return amount;
}

static TUInt32 TArgParserAmountLimit(const char format)
{
	if(!format || format == '0') {
		return 0;
	} else if(format == '?') {
		return 1;
	} else if(TStringIsDigit(format)) {
		return format - '0';
	}

	return TArgParser.argc - TArgParser.idx;
}

static unsigned char TArgParserIsAmountValid(const char format, TUInt32 amount)
{
	TUInt32 limit = TArgParserAmountLimit(format);

	if (TStringIsDigit(format))
		return amount == limit;

	return amount <= limit;
}

static unsigned char TArgParserIsTypeValid(const char type, const char *arg)
{
	if(type == 'd' || type == 'i') {
		if(*arg == '-') arg++;

		for(;*arg;arg++)
			if (*arg < '0' || *arg > '9') return 0;
	} else if(type == 'u') {
		for(;*arg;arg++)
			if (*arg < '0' || *arg > '9') return 0;
	} else if(type == 'x') {
		for(;*arg;arg++)
			if ((*arg < '0' || *arg > '9') && (*arg < 'a' || *arg > 'f')) return 0;
	} else if(type == 'X') {
		for(;*arg;arg++)
			if ((*arg < '0' || *arg > '9') && (*arg < 'A' || *arg > 'F')) return 0;
	} else if(type == 'f' || type == 'F') {
		unsigned char hasdot = 0;
		for(;*arg;arg++)  {
			if ((*arg < '0' || *arg > '9') && (*arg != '.' || hasdot)) return 0;
			if(*arg == '.') hasdot = 1;
		}
	} else if(type == 'c') {
		return *arg && !*(arg+1);
	}

	return 1;
}

static unsigned char TArgParserCheck(TArg *pat, const char *arg)
{
	size_t i = 0,ndata;
	size_t limit;

	if(!pat) return 1;

	// check if the pattern was already used

	if(pat->used) {
		TErrorReportFull(1,"Pattern for switch already used.");
		TArgParserHelp();
		return 1;
	}
	pat->used = 1;

	// check if the amount of data is correct
	limit = TArgParserAmountLimit(pat->amount);

	ndata = TArgParserNumData(TArgParser.idx, limit);
	if(!TArgParserIsAmountValid(pat->amount,ndata)) {
		TErrorReportFull(1,"Invalid number of arguments.");
		return 1;
	}

	// check if the type is correct
	for(; i < ndata; ++i) {
		arg = TArgParser.argv[TArgParser.idx + i];
		if(!TArgParserIsTypeValid(pat->type,arg)) {
			TErrorReportFull(1,"Invalid argument.");
			return 1;
		}
	}

	return 0;
}

static inline void TArgParserStoreParam(char type, char def, size_t amount)
{
	TFree(TArgParser.ph.param);
	TArgParser.ph.param = 0;
	TArgParser.ph.paramtype = type;
	TArgParser.ph.paramdef = def;
	TArgParser.ph.remainingParams = amount;
}

TUInt8 TArgParserNext(void)
{
	TUInt8 arg;

	// maybe we have reached the end already
	if(!TArgParser.argv || (TArgParser.idx >= TArgParser.argc)) {
		if(TArgParser.ph.param) {
			TFree(TArgParser.ph.param);
			TArgParser.ph.param = 0;
		}
		return -1;
	}

	if(TArgParser.ph.remainingParams) {
		TArgParser.idx += TArgParser.ph.remainingParams;
		TArgParserStoreParam(0,0,0);
	}

	while((TArgParser.idx < TArgParser.argc)) {
		TArg *pat;

		const char *next = TArgParser.argv[TArgParser.idx];
		if(!next) {
			TErrorReport(T_ERROR_NULL_POINTER);
			return -1;
		}

		if(TArgParserIsSwitch(next)) {
			//argument is a switch
			//find matching pattern
			pat = TArgParserMatchSwitch(next, &arg);
			TArgParser.idx++;
		} else {
			pat = TArgParserMatchNonSwitch(&arg);
		}

		if (TArgParserCheck(pat, next)) return -1;

		{
			// keep parameters ready
			size_t limit = TArgParserAmountLimit(pat->amount);
			size_t amount = TArgParserNumData(TArgParser.idx, limit);
			if (amount) TArgParserStoreParam(pat->type, pat->def, amount);
		}

		break;
	}

	return arg;
}

const void *TArgParserNextParameter(void)
{
	const void *data;
	const char *arg;
	char type = TArgParser.ph.paramtype;

	if(!TArgParser.ph.remainingParams) return 0;

	TFree(TArgParser.ph.param);

	arg = TArgParser.argv[TArgParser.idx];

	if(type == 'd' || type == 'i') {
		int *p;
		TArgParser.ph.param = TAlloc(sizeof(int));
		p = (int *) TArgParser.ph.param;
		
		sscanf(arg,"%d",p);
		data = TArgParser.ph.param;
	} else if(type == 'u') {
		unsigned int *p;
		TArgParser.ph.param = TAlloc(sizeof(unsigned int));
		p = (unsigned int *) TArgParser.ph.param;
		
		sscanf(arg,"%u",p);
		data = TArgParser.ph.param;
	} else if(type == 'x' || type == 'X') {
		unsigned int *p;
		TArgParser.ph.param = TAlloc(sizeof(unsigned int));
		p = (unsigned int *) TArgParser.ph.param;

		sscanf(arg, type == 'x' ? "%x" : "%X",p);
		data = TArgParser.ph.param;
	} else if(type == 'f' || type == 'F') {
		double *p;
		TArgParser.ph.param = TAlloc(sizeof(double));
		p = (double *) TArgParser.ph.param;

		sscanf(arg, "%lf",p);
		data = TArgParser.ph.param;
	} else if(type == 'c') {
		char *p;
		TArgParser.ph.param = TAlloc(sizeof(char));
		p = (char *) TArgParser.ph.param;

		sscanf(arg, "%c",p);
		data = TArgParser.ph.param;
	} else {
		data = arg;
		TArgParser.ph.param = 0;
	}

	TArgParser.ph.remainingParams--;
	TArgParser.idx++;
	return data;
}
