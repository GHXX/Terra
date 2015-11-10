
#ifndef __included_terra_tokenizer_h
#define __included_terra_tokenizer_h

#include "io/trw.h"

typedef struct TTokenizer TTokenizer;

/**
 * Create a new tokenizer
 *
 * @param input               The buffer that needs to be split in tokens
 * @return                    A tokenizer structure.
 *
 */
TTokenizer *TTokenizerNew(TRW *input);

/**
 * Free a tokenizer
 *
 * @param context             The tokenizer structure.
 *
 */
void TTokenizerFree(TTokenizer *context);

/**
 * Set the separators used to split tokens
 *
 * @param context             The tokenizer structure.
 * @param separators          The separators.
 *
 */
void TTokenizerSetSeparators(TTokenizer *context, const char *separators);

/**
 * Returns the next token
 *
 * @param context             The tokenizer structure.
 *
 * @return                    The next token.
 *
 */
const char *TTokenizerNext(TTokenizer *context);

#endif
