#!/bin/bash

# generate the file we need

smap="/boot/System.map-$(uname -r)"

symbline=$(cat $smap | grep '\Wsys_call_table$')
set $symbline

echo -e "#pragma once" > ./sysgen.h
echo -e "void** sys_call_table = (void**)0x$1;" >> ./sysgen.h

