KERNELDIR := /home/nagisa/linux/course
CURRENT_PATH := $(shell pwd)

OUTPUT_PATH := ./out

#APP_NAME := esp32App

DRIVER_NAME := jy901b.o

obj-m := $(DRIVER_NAME)

SYSNC_NFS_FILE += ./*.ko
NFS_DIR := /home/nagisa/linux/nfs/

build: kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
	rm ./*.mod.c ./*.o ./.*.cmd ./*.symvers ./*.order
	mv ./*.ko $(NFS_DIR)
	#arm-linux-gnueabihf-gcc ./$(APP_NAME).c -o $(NFS_DIR)/$(APP_NAME)
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean