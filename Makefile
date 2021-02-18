obj-m += stratopifan.o

stratopifan-objs := module.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean

install:
	sudo install -m 644 -c stratopifan.ko /lib/modules/$(shell uname -r)
	sudo depmod
