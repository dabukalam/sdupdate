CFLAGS = -Wall -Werror -pedantic --std=c99 -O2 -g
all: sdupdate
	gcc -o sdupdate sdupdate.c -pg

clean :
	rm -f sdupdate sdupdate.o

check :
	./test.sh
