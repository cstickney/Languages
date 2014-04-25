#ifndef SCANNER_H
#define SCANNER_H

/* this enum holds all token types */
enum {   /* non-single char tokens */
  IDENT_ = 256,
  ASSIGN_=257,
  REAL_=258,
  NE_=259, LE_=260, GE_=261,
  OR_=262, AND_=263, NOT_=264,
  WHILE_=265, DO_=266, OD_=267,
  IF_=268, THEN_=269, ELSIF_=270, ELSE_=271, FI_=272,
  HOME_=273, PENUP_=274, PENDOWN_=275, FORWARD_=276, RIGHT_=277, LEFT_=278, PUSHSTATE_=279, POPSTATE_=280
};

/* LVAL can either be a real number or an identifier */
typedef union {     /* lexeme associated with certain tokens */
  float f;          /* REAL_ */
  char *s;          /* IDENT_ */
} LVAL;

extern int lineno;  /* current source code line number */

/*
 * Returns the next token/lexeme read from stdin.
 * Returns 0 when there are no more tokens.
 */
int nextToken(LVAL *lval);

#endif /* SCANNER_H */
