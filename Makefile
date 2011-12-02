prefix = /usr/local
bindir = $(prefix)/bin
mandir = $(prefix)/share/man
man1dir = $(mandir)/man1

CFLAGS = -Wall -Werror -pedantic --std=c99 -O2 -pg -g
all: sdupdate

clean :
	rm -f sdupdate sdupdate.o copy1234 test1234 gmon.out testing/gmon.out lipsumbyte

check :
	./test.sh

install: all
	install -d $(DESTDIR)$(bindir)
	install sdupdate $(DESTDIR)$(bindir)
	install -d $(DESTDIR)$(man1dir)
	install sdupdate.1 $(DESTDIR)$(man1dir)

