#!/bin/bash -e
rm -f test1234
echo This is a test file for sdupdate.c > test1234
./sdupdate test1234 copy1234
if cmp -s test1234 copy1234 
	then
		echo "It works!"
		rm -f test1234 copy1234
fi
