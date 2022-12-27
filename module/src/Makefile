#.PRECIOUS: dispenser.o //keep tmp files
#obj-m += dispenser.o
obj-m := dispenser.o
targets += $(dtb-y)
#dispenser-objs := chardev.o interface.o param.o interrupt.o
#list-multi := dispenser.o
#EXTRA_CFLAGS=-I$(PWD)/../../include -S -save-temps -I/lib/modules/$(shell uname -r)/source/ //keep intermediate files
EXTRA_CFLAGS=-I$(PWD)/../../include -I/lib/modules/$(shell uname -r)/source/
#INSTALL_MOD_PATH = /lib/modules/$(shell uname -r)/

all: dt module
	echo "Building Device tree and kernel module"

module:
	#make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	make -C /usr/src/rpi/linux-raspberrypi-kernel_1.20200723-1/ ARCH=arm CROSS_COMPILE=/opt/gcc-linaro-11.3.1-2022.06-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf- M=/usr/local/src/Dispenser/module/src modules

dt: rpioverlay.dts
	$(CPP) -nostdinc -I include -I arch -undef -x assembler-with-cpp -I /lib/modules/$(shell uname -r)/source/include rpioverlay.dts > rpioverlay.dts.preprocessed
	dtc -@ -I dts -i /lib/modules/$(shell uname -r)/source/include -O dtb -o dispenser-rpi4.dtbo rpioverlay.dts.preprocessed

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm rpioverlay.dts.preprocessed

install:
	cp *.ko /lib/modules/$(shell uname -r)/
	cp -a dispenser-rpi4.dtbo /boot/overlays/
	depmod

test:
#dispenser.o : chardev.c interface.c param.c interrupt.c
#	$(CC) -c -v $@ -mcmodel=kernel -fpermissive -fno-builtin -fno-exceptions -fno-rtti -nostdinc $<

        #make -C $(INSTALL_MOD_PATH)/build M=$(PWD) modules_install
	#make -C /lib/modules/$(shell uname -r)/build M=/lib/modules/$(shell uname -r) modules_install
	#make -C /lib/modules/$(shell uname -r)/build M=$(PWD) install *.ko
	#make -C /lib/modules/5.15.44-v8+/build M=$(PWD) modules_install
