
#ifndef __included_terra_tokenizer_h
#define __included_terra_tokenizer_h

#include "io/tio.h"

typedef struct TTokenizer TTokenizer;

/**
 * Create a new tokenizer
 *
 * @param input               The buffer that needs to be split in tokens.
 * @param freeInput           Auto deletes the stream reader when freed if not 0.
 *
 * @return                    A tokenizer structure.
 *
 */
TTokenizer *TTokenizerNew(TStream *input, TInt8 freeInput);

/**
 * Free a tokenizer
 *
 * @param context             The tokenizer structure.
 *
 */
void TTokenizerFree(TTokenizer *context);

/**
* Set the tokenizer behavior for empty tokens
*
* @param context             The tokenizer structure.
* @param skip                Will return empty strings if set to 0, skip if set to 1.
*
*/
void TTokenizerSkipEmpty(TTokenizer *context, TInt8 skip);

/**
* Set the character for escaping separators
*
* @param context             The tokenizer structure.
* @param escapeChar          The character used for escaping.
*
*/
void TTokenizerSetEscapeCharacter(TTokenizer *context, char escapeChar);

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
 * @param separator           Contains the separator character upon return.
 *
 * @return                    The next token.
 *
 */
const char *TTokenizerNext(TTokenizer *context, char *separator);

#endif
