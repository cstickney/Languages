Author: Chris Stickney
Email: c.stickney117@gmail.com

The program should be working correctly. Added more comments as per the feedback on assignment 1. Changed how the program executes block statements, so that the symbol table is backed up before a block is executed, and restored afterwards. Added init stmts, and only allow them to be used at the beginning of a stmt_seq or block_stmt. Changed assign statements to no longer allow variable creation. Updated destroy statements to handle INITs. Added functions to copy and restore old symbol tables.

The program requires the new scanner.c/h files, because VAR was added as a new reserved word.

Note: all destroy statements have been disabled because i didn't have time to debug them, and it was said that these weren't being graded on how well we free variables.

compile the program with "make"
run the program with ./interpreter < (SOURCE FILE.turtle) > (DESTINATION FILE.pgm) 

Included Files:
parser_rcd.c
scanner.c
symtab.c
turtle.c
scanner.h
symtab.h
turtle.h
Makefile
README.txt
Design.docx
