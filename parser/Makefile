all: interpreter

interpreter: parser_rcd.c scanner.h scanner.c turtle.h turtle.c
	gcc -std=c99 -o interpreter parser_rcd.c scanner.c turtle.c -lm

cunion: cunion.c
	gcc -std=c99 -o cunion cunion.c

goto: goto.c
	gcc -std=c99 -o goto goto.c

clean: 
	rm -f interpreter cunion
