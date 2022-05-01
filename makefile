ARCH  ?= arm

ifeq ($(ARCH),x86)
	CC := gcc
else
	CC := arm-none-eabi-gcc
endif


KERNEL_ROOT := $(shell pwd)
export KERNEL_ROOT

#定义变量，用于保存编译选项和头文件保存路径
header_file := -fno-builtin -I$(KERNEL_ROOT)/include \
	-I$(KERNEL_ROOT)/drivers/include \
	-I$(KERNEL_ROOT)/freertos_9.0.0/include \
	-I$(KERNEL_ROOT)/fatfs/include \
	-I$(KERNEL_ROOT)/app/include \
	-I$(KERNEL_ROOT)/LVGL/include \
	-I$(KERNEL_ROOT)/letter_shell/include \
	-I$(KERNEL_ROOT)/lwip/include #

export header_file


all : start.o main.o drivers/drivers.o  #freertos_9_0_0/freertos_9_0_0
	arm-none-eabi-ld -Tcode.lds $^ -o imx6ull.elf 
	arm-none-eabi-objcopy -O binary -S -g imx6ull.elf imx6ull.bin

%.o : %.S
	arm-none-eabi-gcc -g -c $^ 
%.o : %.c
	@arm-none-eabi-gcc $(header_file) -c $^ 	

#调用其他文件的makefile
 drivers/drivers.o :
	make -C drivers all
 freertos_9_0_0/freertos_9_0_0.o :
	make -C freertos_9_0_0 all

#定义清理伪目标
.PHONY: clean
clean:
	make -C drivers clean
#	make -C freertos_9_0_0 clean
	-rm -f *.o *.elf *.bin 


	


#mkdir   -p    my/file1              递归的创建文件目录，如果上级目录不存在，就按照目录层级，递归创见