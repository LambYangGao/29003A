#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <stdint.h>
#include <time.h>
#include "serial.h"




//综合控制板卡A 模拟串口
#define UART_INDEX_TESTLAN		        0		 //测试网口
#define UART_INDEX_FK	                    1		 //飞控串口 （控制与状态）
#define UART_INDEX_RHTXSZ		        2		 //融合图像时钟
#define UART_INDEX_SF		                3		 //伺服控制
#define UART_INDEX_OUT_POWER		    4		 //外仓电源控制
#define UART_INDEX_SELF_TIME		    5		 //自守时时间
#define UART_INDEX_SF_WTB		        6		     //伺服外同步
#define UART_INDEX_SELF_TIMESTATE   7		            //自守时时间状态    暂无
#define UART_INDEX_PPS		                 8		            //系统PPS             暂无

#define RK_TO_TESTLAN_UART	                  "/dev/uartextend0"       //测试网口
#define RK_TO_FK_UART	                          "/dev/uartextend1"       //飞控串口 （控制与状态）
#define RK_TO_RHTXSZ_UART                      "/dev/uartextend2"      //融合图像时钟
#define RK_TO_SF_UART	                           "/dev/uartextend3"      //伺服控制
#define RK_TO_OUT_POWER_UART 	          "/dev/uartextend4"     //外仓电源控制
#define RK_TO_SELF_TIME_UART	              "/dev/uartextend5"      //自守时时间
#define RK_TO_SF_WTB_UART	                   "/dev/uartextend6"      //伺服外同步
#define RK_TO_SELF_TIMESTATE_UART	       "/dev/uartextend7"      //自守时时间状态    暂无
#define RK_TO_PPS_UART	                           "/dev/uartextend8"      //系统PPS           暂无
 
//char* uart0 = "/dev/uartextend0";
//char* uart1 = "/dev/uartextend1";
//char* uart2 = "/dev/uartextend2";
//char* uart3 = "/dev/uartextend3";
//char* uart4 = "/dev/uartextend4";
//char* uart5 = "/dev/uartextend5";
//char* uart6 = "/dev/uartextend6";


int sp_init();
int sp_open(int index, int baudrate, int threshold, int parity);
void sp_clear(int index);

int32_t sp_write(const int index, uint8_t* buffer, uint32_t length);

int sp_read(const int index, uint8_t* out_buffer, const int buffer_lens, const uint8_t* head, const int head_len);
int sp_read_bylen(const int index, uint8_t* out_buffer, const int buffer_lens);//按长度读，可能会错位

void sp_close(int index);
void sp_uninit();

#endif