# superhide


An example of a Loadable Kernel Module (LKM) that hooks the system call table.


This module will hide any userspace files that start with a certain prefix from any program that uses the getdents system call to list a directories files.


To use this, run `sudo build_and_install.sh` in the superhide folder. Remeber where the folder is, because it will be hidden now.

To remove this, run `sudo remove_and_clean.sh` in the superhide folder.

This program only captures the getdents syscall for hiding files, it doesn't hook the getdents64 call because just doing getdents was enough for a proof of concept. Turns out most things just use the getdents syscall though.

Note: I have found this to not work on some newer kernels.
