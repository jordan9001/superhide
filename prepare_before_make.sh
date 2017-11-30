#!/bin/bash

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
