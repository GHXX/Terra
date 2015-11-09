
#ifndef __included_terra_arg_parser_h
#define __included_terra_arg_parser_h

void TArgParserInit(int argc, const char **argv);
void TArgParserDestroy(void);

/**
 * Allow a new argument to the parser
 *
 * @param name                The name of the argument. Only used for switches.
 * @param type                The type of the argument. Check printf.
 * @param amount              The amount of arguments allowed. '?' for 0 or 1, '0-9' for fixed or '+' for any.
 * @param def                 The default argument if none is provided (unused)
 * @param help                A description of the argument. used when printing usage.
 *
 * @return                    A value reprensenting the argument
 *
 */
TUInt8 TArgParserAddArgument(const char *name, TUInt8 type, TUInt8 amount, TUInt8 def, const char *help);

/**
 * Output the usage to stdout
 *
 */
void TArgParserHelp(void);

/**
 * Parse and return the next argument.
 *
 * @return                    The next argument.
 *
 */
TUInt8 TArgParserNext(void);

/**
 * Parse and return the next argument parameter.
 *
 * @return                    The current parameter for the argument.
 *
 */
TCPtr TArgParserNextParameter(void);

#endif
