
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
* Set the tokenizer stream to a new position
*
* @param context             The tokenizer contex.
* @param offset              The offset from the origin.
* @param origin              The starting position(SEEK_CUR, SEEK_SET, SEEK_END).
*
*/
void TTokenizerSeek(TTokenizer *context, TLInt offset, TInt8 origin);

/**
* Get the current stream position.
*
* @param context             The tokenizer context.
*
* @return                    The stream position.
*
*/
TLInt TTokenizerTell(TTokenizer *context);

/**
* Set the tokenizer behavior for empty tokens
*
* @param context             The tokenizer structure.
* @param skip                Will return empty strings if set to 0, skip if set to 1. Default is 0.
*
*/
void TTokenizerSkipEmpty(TTokenizer *context, TInt8 skip);

/**
* Set the tokenizer behavior for character stripping
*
* @param context             The tokenizer structure.
* @param leadStrip           Set the leading characters to be removed.
* @param trailStrip          Set the trailing characters to be removed.
*
*/
void TTokenizerSetStrip(TTokenizer *context, const char *leadStrip, const char *trailStrip);

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
* Returns the current token
*
* @param context             The tokenizer structure.
*
* @return                    The next token.
*
*/
const unsigned char *TTokenizerGet(TTokenizer *context);

/**
 * Returns the next token
 *
 * @param context             The tokenizer structure.
 * @param separator           Contains the separator character upon return.
 *
 * @return                    The next token.
 *
 */
const unsigned char *TTokenizerNext(TTokenizer *context, char *separator);

/**
* Returns the token up to next matching separator
*
* @param context             The tokenizer structure.
* @param separator           The separator to jump to.
*
* @return                    The next token.
*
*/
const unsigned char *TTokenizerJump(TTokenizer *context, char separator);

#endif
