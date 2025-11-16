//
// Debug wrapper for consume function
//

#include <stdio.h>
#include "lookahead_lexer.h"
#include "tokens.h"

const char* token_name_debug(int type) {
    switch(type) {
        case INT: return "INT";
        case FLOAT: return "FLOAT";
        case PLUS: return "PLUS";
        case MINUS: return "MINUS";
        case MULT: return "MULT";
        case DIV: return "DIV";
        case EXPON: return "EXPON";
        case SEMICOLON: return "SEMICOLON";
        case OPAR: return "OPAR";
        case FPAR: return "FPAR";
        case VAR: return "VAR";
        case MODULO: return "MODULO";
        case ASSIGN: return "ASSIGN";
        case UNDERSCORE: return "UNDERSCORE";
        case OCROCH: return "OCROCH";
        case FCROCH: return "FCROCH";
        case COMBINATION: return "COMBINATION";
        case SINE: return "SINE";
        case COSINE: return "COSINE";
        case PI: return "PI";
        default: return "UNKNOWN";
    }
}

void debug_consume(int tokenType) {
    Token *t = currentToken();
    if (t != NULL) {
        fprintf(stdout, "[DEBUG] Consuming token: '%s' (%s)\n", t->text, token_name_debug(t->type));
    }
    consume(tokenType);
}
