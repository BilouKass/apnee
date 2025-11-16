//
// Created by Gilles Sérasset on 09/10/2019.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tokens.h"
#include "lookahead_lexer.h"
#include "uthash.h"

#include "assignations.h"

/*********** Managing Symbol Table with UTHash Hash table ***************/

struct Symbol {
    int value;
    char var[100];     /* we'll use this field as the key */
    UT_hash_handle hh; /* makes this structure hashable */
};

struct Symbol *symbols = NULL;

void set_value(char var[], int val) {
    struct Symbol* symbol;
    HASH_FIND_STR( symbols, var, symbol );
    if (symbol == NULL) {
        // the symbol does not exist yet
        symbol = calloc(1, sizeof(struct Symbol));
        strcpy(symbol->var, var);
        HASH_ADD_STR( symbols, var, symbol );
    }
    symbol->value = val;
}

int get_value(char var[]) {
    struct Symbol* symbol;
    HASH_FIND_STR( symbols, var, symbol );
    if (symbol == NULL) {
        return 0;
    } else {
        return symbol->value;
    }
}

void print_symbols() {
    struct Symbol *s;

    for(s=symbols; s != NULL; s=s->hh.next) {
        fprintf(stdout, "%s = %d\n", s->var, s->value);
    }
}

/************** Grammaire *****************/

const char* token_name(int type) {
    switch(type) {
        case INT: return "INT";
        case PLUS: return "PLUS";
        case MINUS: return "MINUS";
        case MULT: return "MULT";
        case DIV: return "DIV";
        case EXPON: return "EXPON";
        case SEMICOLON: return "SEMICOLON";
        case OPAR: return "OPAR";
        case FPAR: return "FPAR";
        case VAR: return "VAR";
        case ASSIGN: return "ASSIGN";
        case UNDERSCORE: return "UNDERSCORE";
        default: return "UNKNOWN";
    }
}

void syntax_error(const char* non_terminal, const char* expected) {
    Token* tok = currentToken();
    if (tok == NULL) {
        fprintf(stderr, "SYNTAX ERROR in %s: unexpected end of file\n", non_terminal);
        fprintf(stderr, "  Expected: %s\n", expected);
    } else {
        fprintf(stderr, "SYNTAX ERROR in %s: unexpected token '%s' (%s)\n", 
                non_terminal, tok->text, token_name(tok->type));
        fprintf(stderr, "  Expected: %s\n", expected);
    }
}

void S(void){
    switch (currentToken()->type) {
    case VAR: //R0
        A();
        consume(SEMICOLON);
        break;
    default:
        syntax_error("S", "VAR, MINUS, OPAR or INT");
        break;
    }
}
void A(void) {
    switch (currentToken()->type) // verif les FEOF en cas de NULL
    {
    case VAR: //R1
    case INT: //R1
    case MINUS: //R1
    case OPAR: //R1
        L();
        consume(SEMICOLON);
        if (currentToken() != NULL)
            A();
        break;
    case SEMICOLON: //R2
        break;
    default:
        syntax_error("A", "VAR, MINUS, OPAR, INT or SEMICOLON");
        break;
    }
}
void L(void) {
    switch (currentToken()->type)
    {
    case MINUS: //R4
    case OPAR: //R4
    case INT: //R4
        E();
        break;
    case VAR: //R3 ET R4
        // fonction de rec de variable et ajouter en mémoire si assignation
        V();
        if (lookup(2)->type != ASSIGN) { // si R3
            consume(ASSIGN);
        } 
        E();
        break;
    default:
        syntax_error("L", "VAR, MINUS, OPAR or INT");
        break;
    }
}

void V(void) { // envisager un retour d'une adresse mémoire relié à la variable (peut être une adresse de tableau qu'on realloc en cas d'ajout)
    if (currentToken()->type != VAR) { //R5
        syntax_error("V", "VAR");
    }
    consume(VAR);
}

void E(void) {
    switch (currentToken()->type)
    {
    case MINUS: //R6
    case OPAR: //R6
    case INT: //R6
    case VAR: //R6 OU R7 
        if (lookup(2)->type == ASSIGN) { //R7
            V();
            consume(ASSIGN);
            E();
            break;
        } 
        T(); //sinon R6 verifier cas var sans = donc faire en sorte de remonter la valeur de la variable
        Ep();
        break;

    
    default:
        syntax_error("E", "VAR, MINUS, OPAR or INT");
        break;
    }
}

void Ep(void) {
    switch (currentToken()->type)
    {
    case SEMICOLON: //R10
    case FPAR:  //R10
        break;
    
    case PLUS: //R9
        consume(PLUS);
        T();
        Ep();
        break;
    case MINUS: //R8
        consume(MINUS);
        T();
        Ep();
        break;

    default:
        syntax_error("E'", "PLUS, MINUS, SEMICOLON or FPAR");
        break;
    }
}

void T(void) {
    switch (currentToken()->type)
    {
    case MINUS: //R11
    case OPAR: //R11
    case VAR: //R11
    case INT: //R11
        G();
        Tp();
        break;
    
    default:
        syntax_error("T", "VAR, MINUS, OPAR or INT");
        break;
    }
}
void Tp(void) {
    switch (currentToken()->type) {
        case SEMICOLON: //R14
        case PLUS: //R14
        case MINUS: //R14
        case FPAR: //R14
            break;
        case DIV: //R13
            consume(DIV);
            G();
            Tp();
            break;
        case MULT: //R12
            consume(MULT);
            G();
            Tp();
            break;
        default:
            syntax_error("T'", "DIV, MULT, SEMICOLON, PLUS, MINUS or FPAR");
            break;
    }
}

void G(void) {
    switch (currentToken()->type) {
        case MINUS: //R15 et R16
        case VAR: //R15 et R16
        case INT: //R15 et R16
        case OPAR: //R15 et R16
            if (lookup(2)->type == EXPON) { //R16
                B();
                consume(EXPON);
                G();
                break;
            }  //R15
            B();
            break;
        default:
            syntax_error("G", "MINUS, VAR, INT or OPAR");
            break;
    }
}

void B(void) {
    switch (currentToken()->type) {
        case MINUS: //R17
        case OPAR:
        case INT:
        case VAR:
            F();
            Bp();
            break;
        default:
            syntax_error("B", "MINUS, OPAR, INT or VAR");
            break;
    }
}

void Bp(void)  {
    switch (currentToken()->type) {
        case UNDERSCORE: //R18
            consume(UNDERSCORE);
            F();
            Bp();
            break;
        case SEMICOLON: //R19
        case PLUS:
        case MINUS:
        case DIV:
        case MULT:
        case EXPON:
        case FPAR:
            break;
        default:
            syntax_error("B'", "UNDERSCORE, SEMICOLON, PLUS, MINUS, DIV, MULT, EXPON");
            break;
    }
}

void F(void) {
    switch (currentToken()->type) {
        case MINUS: // R20
            if (currentToken()->type == MINUS) {
                consume(MINUS);
            }
            F();
            break;
        case VAR: // R21
            //lecture du contenu de var
            consume(VAR);
            break;
        case INT: // R22
            consume(INT);
            //return valu int
            break;
        case OPAR: // R23
            consume(OPAR);
            L();
            consume(FPAR);
            break;
        default:
            syntax_error("F", "MINUS, VAR, INT, OPAR");
            break;
    }
}
/********* PROGRAMME PRINCIPAL **************/
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <source-file>\n", argv[0]);
        return 1;
    }
    // fprintf(stderr, "Lexing from %s.\n", argv[1]);
    initLexer(argv[1]);
    // APPEL A LA SOURCE DE LA GRAMMAIRE
    // A FAIRE !!!
    S();
    // ON VERIFIE QUE LA GRAMMAIRE A BIEN TERMINE SON TRAVAIL A LA FIN DU MOT A ANALYSER
    if (currentToken() != NULL) {
        fprintf(stderr, "Unexpected input after assignation.\n");
        fprintf(stdout, "!!! ERREUR !!!\n");
        return 1;
    }
    // ET ON AFFICHE LA TABLE DES SYMBOLES
    print_symbols();
    return 0;
}

