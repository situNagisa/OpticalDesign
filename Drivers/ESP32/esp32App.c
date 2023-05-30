#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "sys/ioctl.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include <poll.h>
#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>
#include "Register.h"
#include <stdlib.h>
/***************************************************************
Copyright © ALIENTEK Co., Ltd. 1998-2029. All rights reserved.
文件名		: ap3216cApp.c
作者	  	: 左忠凯
版本	   	: V1.0
描述	   	: ap3216c设备测试APP。
其他	   	: 无
使用方法	 ：./ap3216cApp /dev/ap3216c
论坛 	   	: www.openedv.com
日志	   	: 初版V1.0 2019/9/20 左忠凯创建
***************************************************************/

/*
 * @description		: main主程序
 * @param - argc 	: argv数组元素个数
 * @param - argv 	: 具体参数
 * @return 			: 0 成功;其他 失败
 */
int main(int argc, char* argv[])
{
	int fd;
	char* filename;
	unsigned short databuf[3];
	unsigned short ir, als, ps;
	int ret = 0;
	size_t i = 2;
	size_t times = 1;
	unsigned char data[36] = {};
	unsigned char command[] = { EOD_HEADER,EOD_SET_LINEAR_VELOCITY_PERCENT,0,0,0,0 };

	if (argc < 2) {
		printf("Error Usage!\r\n");
		return -1;
	}

	filename = argv[1];
	fd = open(filename, O_RDWR);
	if (fd < 0) {
		printf("can't open file %s\r\n", filename);
		return -1;
	}
	i = 2;
	for (; i < argc; i++)
	{
		unsigned char* endptr = 0;
		command[i - 1] = strtol(argv[i], &endptr, 16);
	}
	if (argc > 2) {
		ret = write(fd, command, sizeof(command));
	}

	while (times--) {
		ret = read(fd, data, 36);
	}
	i = 0;
	for (; i < 6; i++)
	{
		size_t j = 0;
		for (; j < 6; j++)
		{
			printf("0x%02x ", data[i * 6 + j]);
		}
		printf("\n");
	}




	close(fd);	/* 关闭文件 */
	return 0;
}

