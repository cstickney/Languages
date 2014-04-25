#ifndef SYMTAB_H
#define SYMTAB_H

typedef struct SymTab {
  char *sym;
  float val;
  struct SymTab *next;
} SymTab;

extern SymTab *symtab;  /* global symbol table */

SymTab *symLookup(char *sym);

#endif /* SYMTAB_H */
