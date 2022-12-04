#!/bin/bash

#Nev: Jako Daniel
#Azonosito: jdim2141
#Csoport: 522/1


#Leellenorizem hogy vane leegendo parameter
if ! [ $# -eq 1 ]
then
	echo Hasznalat: $0 [parameter]\(allomanynev\)
	exit 1
fi

tmp="$(mktemp)"

find ~ -name "$1" -type f | grep -E -o "[^\/]*\/[^\/]*$" | cut -d'/' -f1 > $tmp

lines=$(wc -l $tmp | cut -d' ' -f1)

if [ $lines -eq 0 ]
then 
	echo 0
	
else 
	cat $tmp
fi

rm -f $tmp

exit 0
