# superhide


An example of a Loadable Kernel Module (LKM) that hooks the system call table.


This module will hide any userspace files that start with a certain prefix from any program that uses the getdents system call to list a directories files.


To use this, run prepare_before_make.sh before running make, then use insmod inorder to add the module. Use rmmod to remove it again.
