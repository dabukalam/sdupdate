#!/bin/bash
if [ -a test1234 ]
	then
		rm test1234
fi
echo This is a test file for sdupdate.c > test1234
./sdupdate test1234 copy1234
testsum=$(md5sum test1234 | awk '{ print $1 }')
copysum=$(md5sum copy1234 | awk '{ print $1 }')
echo $testsum
echo $copysum
if [[ $testsum == $copysum && $copysum != "" ]]
	then
		echo "It works!"
	else
		echo "It didn't work :-("
fi
