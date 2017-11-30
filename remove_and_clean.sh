#!/bin/bash

RET=0
# require root privs
if [[ $UID != 0 ]]; then
	echo "Please run this script with sudo"
	exit 1
fi

# remove it
rmmod superhide

# cleanup

make clean

rm -f ./sysgen.h
