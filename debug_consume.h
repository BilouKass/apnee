//
// Debug wrapper for consume function
//

#ifndef DEBUG_CONSUME_H
#define DEBUG_CONSUME_H

/**
 * Debug version of consume that prints the token being consumed
 */
void debug_consume(int tokenType);

/**
 * Returns a human-readable name for a token type
 */
const char* token_name_debug(int type);

#endif // DEBUG_CONSUME_H
