#obj-m += dispenser.o
obj-m := dispenser.o
dispenser-objs := chardev.o interface.o param.o interrupt.o
#list-multi := dispenser.o

#INSTALL_MOD_PATH = /lib/modules/$(shell uname -r)/

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	cp *.ko /lib/modules/$(shell uname -r)/
	depmod

test:
#dispenser.o : chardev.c interface.c param.c interrupt.c
#	$(CC) -c -v $@ -mcmodel=kernel -fpermissive -fno-builtin -fno-exceptions -fno-rtti -nostdinc $<

	#make -C $(INSTALL_MOD_PATH)/build M=$(PWD) modules_install
	#make -C /lib/modules/$(shell uname -r)/build M=/lib/modules/$(shell uname -r) modules_install
	#make -C /lib/modules/$(shell uname -r)/build M=$(PWD) install *.ko
	#make -C /lib/modules/5.15.44-v8+/build M=$(PWD) modules_install
