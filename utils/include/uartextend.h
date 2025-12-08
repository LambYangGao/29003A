#ifndef UARTEXTEND_H__
#define UARTEXTEND_H__

#ifdef __cplusplus
extern "C" {
#endif

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

//char *uart0 = "/dev/uartextend0";
//char *uart1 = "/dev/uartextend1";
//char *uart2 = "/dev/uartextend2";
//char *uart3 = "/dev/uartextend3";
//char *uart4 = "/dev/uartextend4";
//char *uart5 = "/dev/uartextend5";
//char *uart6 = "/dev/uartextend6";

#define RK_TO_TESTLAN_UART	                  "/dev/uartextend0"       //测试网口
#define RK_TO_FK_UART	                          "/dev/uartextend1"       //飞控串口 （控制与状态）
#define RK_TO_RHTXSZ_UART                      "/dev/uartextend2"      //融合图像时钟
#define RK_TO_SF_UART	                           "/dev/uartextend3"      //伺服控制
#define RK_TO_OUT_POWER_UART 	          "/dev/uartextend4"     //外仓电源控制
#define RK_TO_SELF_TIME_UART	              "/dev/uartextend5"      //自守时时间
#define RK_TO_SF_WTB_UART	                   "/dev/uartextend6"      //伺服外同步
#define RK_TO_SELF_TIMESTATE_UART	       "/dev/uartextend7"      //自守时时间状态    暂无
#define RK_TO_PPS_UART	                           "/dev/uartextend8"      //系统PPS           暂无


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

//初始化
int uartextend_init(int speed, int verify, char *uart_name);

int uartextend_open(const char *devname);
int uartextend_close(int fd);
int uartextend_set(int fd, struct user_config cfg);

//读取
int uartextend_read(int fd, void *obuf, int olen);
//写
int uartextend_write(int fd, const void *ibuf, int ilen);
int uartextend_wait_interrupt(int fd, int timeout);

#ifdef __cplusplus
}
#endif

#endif