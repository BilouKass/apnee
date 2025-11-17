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

int powe(int a, int b){ //algo naif de l'exponentiation
    int out = 1;
    for (int i = 0; i < b; i++) {
        out *= a;
    }
    return out;
}
int combi(int n, int k) { //algo opti de K parmis N
    if (k > n) return 0;
    if (k == 0 || k == n) return 1;
    int cmp =  ((n - k) < k) ? n - k : k;
    int out = 1;
    for (int i = 1; i <= cmp; i++) {
        out = out * (n - cmp + i) / i;
    }
    return out;
}



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
        default: return "?";
    }
}

void erreur(const char* non_terminal, const char* expected) {
    Token* tok = currentToken();
    fprintf(stdout, "!!! ERREUR !!!");
    if (tok == NULL) {
        fprintf(stderr, "SYNTAX ERROR in %s: unexpected EOF\n", non_terminal);
        fprintf(stderr, "  Expected: %s\n", expected);
    } else {
        fprintf(stderr, "SYNTAX ERROR in %s: unexpected token '%s' (%s)\n", non_terminal, tok->text, token_name(tok->type));
        fprintf(stderr, "  Expected: %s\n", expected);
    }
    exit(-1);
}

void S(void){
    switch (currentToken()->type) {
    case VAR: //R0
    case INT: //R0
    case MINUS: //R0
    case OPAR: //R0
    case COMBINATION:
        A();
        break;
    default:
        erreur("S", "VAR, -, ( or INT");
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
    case COMBINATION:
        if (lookup(2) == NULL) //pour eviter le segfault du cas de fin du fichier sans ;
            erreur("A", ";");
        L();
        consume(SEMICOLON);
        if (currentToken() != NULL)
            A();
        break;
    case SEMICOLON: //R2
        break;
    default:
        erreur("A", "VAR, -, (, INT or ;");
        break;
    }
}
int L(void) {
    int a; char *v;
    switch (currentToken()->type)
    {
    case MINUS: //R4
    case OPAR: //R4
    case INT: //R4
    case COMBINATION:
        a = E();
        break;
    case VAR: //R3 ET R4

        // fonction de rec de variable et ajouter en mémoire si assignation
        if (lookup(2)->type == ASSIGN) { // si R3
            v = V();
            consume(ASSIGN);
            a = E();
            set_value(v, a);
        } else {
            a = E();
        }

        break;
    default:
        erreur("L", "VAR, -, ( or INT");
        break;
    }
    return a;
}

char *V(void) { // envisager un retour d'une adresse mémoire relié à la variable (peut être une adresse de tableau qu'on realloc en cas d'ajout)
    char *out;
    if (currentToken()->type != VAR) { //R5
        erreur("V", "VAR");
    }
    out = get_text(currentToken());
    consume(VAR);
    return out;
}

int E(void) {
    int a; char *v;
    switch (currentToken()->type) {
    case MINUS: //R6
    case OPAR: //R6
    case INT: //R6
    case COMBINATION:
    case VAR: //R6 OU R7 
        if (lookup(2)->type == ASSIGN) { //R7
            v = V();
            consume(ASSIGN);
            a = E();
            set_value(v, a);
            break;
        } 
        a = T(); //sinon R6 verifier cas var sans = donc faire en sorte de remonter la valeur de la variable
        a = Ep(a);
        break;

    default:
        erreur("E", "VAR, -, ( or INT");
        break;
    }
    return a;
}

int Ep(int a) {
    int b;
    switch (currentToken()->type) {
    case SEMICOLON: //R10
    case FPAR:  //R10
        break;
    
    case PLUS: //R9
        consume(PLUS);
        b = T();
        a = a + b;
        a = Ep(a);
        break;
    case MINUS: //R8
        consume(MINUS);
        b = T();
        a = a-b;
        a = Ep(a);
        break;

    default:
        erreur("E'", "+, -, ; or )");
        break;
    }
    return a;
}

int T(void) {
    int a;
    switch (currentToken()->type) {
    case MINUS: //R11
    case OPAR:
    case VAR:
    case INT: 
    case COMBINATION:
        a = G();
        a = Tp(a);
        break;
    
    default:
        erreur("T", "VAR, -, ( or INT");
        break;
    }
    return a;
}
int Tp(int a) {
    int b;
    switch (currentToken()->type) {
        case SEMICOLON: //R14
        case PLUS: //R14
        case MINUS: //R14
        case FPAR: //R14
            break;
        case DIV: //R13
            consume(DIV);
            b = G();
            a = a / b; 
            a = Tp(a);
            break;
        case MULT: //R12
            consume(MULT);
            b = G();
            a = a * b;
            a = Tp(a);
            break;
        default:
            erreur("T'", "/, *, ;, +, - or )");
            break;
    }
    return a;
}

int G(void) {
    int a; int b;
    switch (currentToken()->type) {
        case MINUS: //R15 et R16
        case VAR: //R15 et R16
        case INT: //R15 et R16
        case OPAR: //R15 et R16

            if (lookup(2)->type == EXPON) { //R15
                a = B();
                consume(EXPON);
                b = G(); //associativité à droite donc on récupère puis on opère
                a = pow(a, b);
            } else {    //R16
                a = B();
            }
            break;
        case COMBINATION:
            a = B();
            break;
        default:
            erreur("G", "-, VAR, INT or (");
            break;
    }
    return a;
}

int B(void) {
    int a; int b;
    switch (currentToken()->type) {
        case MINUS: //R18
        case OPAR:
        case INT:
        case VAR:
            a = F();
            break;
        case COMBINATION:
            consume(COMBINATION);
            if (currentToken()->type == UNDERSCORE) {
                consume(UNDERSCORE);
                a = B();
                consume(EXPON);
                b = F();
                a = combi(a,b);
            } else {
                consume(EXPON);
                a = B();
                consume(UNDERSCORE);
                b = F();
                a = combi(b,a);
            }
            //printf("%d\n", a);
            break;
        default:
            erreur("B", "-, (, INT or VAR");
            break;
    }
    return a;
}


int F(void) {
    int a;
    switch (currentToken()->type) {
        case MINUS: // R19
            consume(MINUS);
            a = -F();
            break;
        case VAR: // R20
            //lecture du contenu de var
            a = get_value(currentToken()->text);
            consume(VAR);
            break;
        case INT: // R21
            a = currentToken()->value;
            consume(INT);
            //return valu int
            break;
        case OPAR: // R22
            consume(OPAR);
            a = L();
            consume(FPAR);
            break;
        default:
            erreur("F", "-, VAR, INT, (");
            break;
    }
    return a;
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

