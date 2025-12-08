#ifndef UARTEXTEND_H__
#define UARTEXTEND_H__

#ifdef __KERNEL__
#include <linux/delay.h>
#include <linux/ioctl.h>
#else
#include <linux/ioctl.h>
#endif

/* 定义设备类型 */
#define IOC_MAGIC  'c'

/* 初始化设备 */
#define IOCINIT    _IO(IOC_MAGIC, 0)

/* 读寄存器 */
#define IOCGREG    _IOW(IOC_MAGIC, 1, int)

/* 写寄存器 */
#define IOCWREG    _IOR(IOC_MAGIC, 2, int)

#define IOC_MAXNR  5


#define PARAMENT_BAUD_MAX 460800
#define PAYLOAD_MAX 1024

struct user_config {
	unsigned int verify;
	unsigned int baud;
};

enum PARAMENT_VERIFY {
	PARAMENT_VERIFY_NONE,
	PARAMENT_VERIFY_ODD,
	PARAMENT_VERIFY_EVEN,
	__PARAMENT_VERIFY_MAX
};

#endif