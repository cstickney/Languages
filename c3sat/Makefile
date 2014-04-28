all: threesat

threesat: 3sat.c list.h list.c
	gcc 3sat.c list.c -o 3sat

listaxioms: listaxioms.c list.h list.c
	gcc listaxioms.c list.c -o listaxioms

clean: 
	rm -f 3sat listaxioms
