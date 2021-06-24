ifeq (${KERNELRELEASE},)
	KERNEL_SOURCE := /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
default:
	${MAKE} -C ${KERNEL_SOURCE} ARCH=arm M=${PWD} modules

install:
	${MAKE} -C ${KERNEL_SOURCE} ARCH=arm M=${PWD} modules_install
	depmod

clean:
	${MAKE} -C ${KERNEL_SOURCE} ARCH=arm M=${PWD} clean

else
	obj-m := customled.o
endif

prog:
	gcc led_activator.c -o led_activator
