KERNELDIR := /home/nagisa/linux/course
CURRENT_PATH := $(shell pwd)

DRIVER_NAME := ov5640.o
DRIVER_NAME += mx6s_capture.o
DRIVER_NAME += mxc_mipi_csi.o
DRIVER_NAME += mxc_vadc.o
DRIVER_NAME += ov5640_mipi.o

obj-m := $(DRIVER_NAME)

SYSNC_NFS_FILE += ./*.ko
NFS_DIR := /home/nagisa/linux/nfs/

build: kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
	rm ./*.mod.c ./*.o ./.*.cmd ./*.symvers ./*.order
	mv ./*.ko $(NFS_DIR)
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean