all:guid.so 
#Which complier
CC = gcc	-g -fpic 
#CC = gcc	-fpic 
#Where to install
INSTDIR     = 
#Where are include files kept
INCLUDE_BASE = .
#Options for development
CFLAGS = -Werror -Wall -ansi 
#Options for Release
#CFLAGS = -ljson -lpthread -ldl -lm -llua -lfcgame  -Wall -ansi

guid.so:guid.o file.o
	$(CC)  -shared -o guid.so guid.o file.o  $(CFLAGS)
guid.o:guid.c 
	$(CC) -I$(INCLUDE_BASE)  -c guid.c  -Wall
file.o:file.c 
	$(CC) -I$(INCLUDE_BASE)  -c file.c  -Wall

clean:
	-rm -f  *.o
