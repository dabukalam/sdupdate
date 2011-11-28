CFLAGS = -Wall -Werror -pedantic --std=c99 -O2 -g -Werror=implicit-function-declaration -Werror=unused-variable
all: sdupdate
	gcc -o sdupdate sdupdate.c -pg

clean :
	rm -f sdupdate sdupdate.o

check :
	./test.sh
