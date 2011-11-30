CFLAGS = -Wall -Werror -pedantic --std=c99 -O2 -pg -g
all: sdupdate

clean :
	rm -f sdupdate sdupdate.o copy1234 test1234 gmon.out testing/gmon.out

check :
	./test.sh
