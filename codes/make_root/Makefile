obj-m += mk_root.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	sudo insmod ./mk_root.ko
	sudo mknod /dev/dev_root c 250 0
