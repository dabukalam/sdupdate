all:
	gcc -o sdupdate sdupdate.c
objects = sdupdate test1234 copy1234

clean :
	rm $(objects)

check :
	./test.sh
