#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"
#include "strdup.h"

SymTab *symtab = NULL;    /* global symbol table */

/*
 * symLookup()
 * Looks for 'sym' in 'symtab' (case sensitive search) and
 * returns the corresponding symbol table entry if found.
 * If the symbol is not found, is creates a new symbol table
 * entry and inserts it into the table and return the new entry.
 */
SymTab *symLookup(char *sym) {
  SymTab *s;
  for (s = symtab; s != NULL; s = s->next)
    if (strcmp(s->sym, sym) == 0)
      return s;
  s = (SymTab *) malloc(sizeof(SymTab));
  if (s == NULL) {
    perror("malloc()");
    exit(-1);
  }
  s->sym = strdup(sym);
  s->val = 0.0;
  s->next = symtab;
  symtab = s;
  return s;
}

