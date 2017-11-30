
#TODO create a .sh file to generate our .h file with offsets we need to get before building?
#ifneq ($(KERNELRELEASE),)

obj-m += superhide.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
