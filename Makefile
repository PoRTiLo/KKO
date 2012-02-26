#makefile
  # KKO- Kodovani a komprese
  # 9.2.2012
  # Autor: Bc. Jaroslav Sendler, FIT, xsendl00(at)stud.fit.vutbr.cz
  # Prelozeno gcc ...
  #

# jmeno prekladaneho programu
PROGRAM=gif2bmp

# jmeno knihovny
LIB=gif2bmp

# nazev projektu
PROJECT=kko.proj3.xsendl00

SRC=gif2bmp.c main.c
HEAD=gif2bmp.h

CCM=g++
CCMFLAGS=-std=c++98 -Wall -pedantic -Wextra
CFLAGS=-std=c99 -pedantic -Wall -Wextra $(ARG_EXTRA)
CC=gcc
all:  ${PROGRAM} 

${PROGRAM}: main.c
	$(CCM) $(CCMFLAGS) $(SRC) $(HEAD) -o $@
#	$(CC) $(CFLAGS) $(SRC) $(HEAD) -o $@

clean:
	rm -f *.o ${PROGRAM}

zip:
	zip ${PROJECT}  ${SRC} ${HEAD} Makefile ${PROGRAM}.pdf
