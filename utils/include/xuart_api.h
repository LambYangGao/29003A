#ifndef XUART_API_H__
#define XUART_API_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/ioctl.h>
//#include </usr/include/x86_64-linux-gnu/bits/stdint-uintn.h>
//#include </usr/lib/gcc/x86_64-linux-gnu/9/include/stddef.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>

#define IOCTL_XDMA_PERF_V1 (1)
#define XDMA_ADDRMODE_MEMORY (0)
#define XDMA_ADDRMODE_FIXED (1)

/*
 * S means "Set" through a ptr,
 * T means "Tell" directly with the argument value
 * G means "Get": reply by setting through a pointer
 * Q means "Query": response is on the return value
 * X means "eXchange": switch G and S atomically
 * H means "sHift": switch T and Q atomically
 *
 * _IO(type,nr)		    no arguments
 * _IOR(type,nr,datatype)   read data from driver
 * _IOW(type,nr,datatype)   write data to driver
 * _IORW(type,nr,datatype)  read/write data
 *
 * _IOC_DIR(nr)		    returns direction
 * _IOC_TYPE(nr)	    returns magic
 * _IOC_NR(nr)		    returns number
 * _IOC_SIZE(nr)	    returns size
 */

struct xdma_performance_ioctl {
	/* IOCTL_XDMA_IOCTL_Vx */
	uint32_t version;
	uint32_t transfer_size;
	/* measurement */
	uint32_t stopped;
	uint32_t iterations;
	uint64_t clock_cycle_count;
	uint64_t data_cycle_count;
	uint64_t pending_count;
};

struct xdma_aperture_ioctl {
	uint64_t ep_addr;
	unsigned int aperture;
	unsigned long buffer;
	unsigned long len;
	int error;
	unsigned long done;
};


enum ID_t{
	CPU1_NUM = 0x00,//left
	CPU2_NUM = 0x01,//right
	FPGA_CTRL = 0x80,
	FPGA_PAYLOAD = 0x81
};

enum PORT_t{
	MONITOR_CTRL =0,
	POWER_CTRL,
	DISTANCE_CTRL,
	SAR_CTRL,
	NAVIGATION_CTRL,
	SERVER_CTRL,
	INFRARED_CTRL,
	CAPTURE_CTRL,
	IMAGE_CTRL
};

enum CMD_t{
	READ_CMD,
	WRITE_CMD,
	REPORT_CMD
};

enum CMD_ACK_t{
	READ_ACK_CMD,
	WRITE_ACK_CMD,
	REPORT_ACK_CMD
};									

enum CAM_FLAG_t{
	INFRARED_FLAG = 0x00,
	VISIBLE_FLAG = 0x01,
	RADAR_FALG = 0x10
};									//图像来源标识符

struct ctrl_frame_t
{
	uint8_t DID;						//数据去向标识符
	uint8_t SID;						//数据来源标识符
	uint8_t port;					//操作端口，设备
	uint8_t cmd_;						//操作类型
	uint8_t ack_;					//操作返回指令
	uint16_t H_sn;					//数据包在一帧图像中的序列号
	uint16_t frame_sn;				//图像的序号
	uint8_t camera_flag;			//图像来源标识		
	uint8_t rsv[6];
	uint8_t data[0];		
}__attribute__((packed));			//控制头定义

typedef enum ID_t ID_t;
typedef enum PORT_t PORT_t;
typedef enum CMD_t CMD_t;
typedef enum CMD_ACK_t CMD_ACK_t;
typedef enum CAM_FLAG_t CAM_FLAG_t;
typedef struct ctrl_frame_t ctrl_frame_t;

/* IOCTL codes */

#define IOCTL_XDMA_PERF_START   _IOW('q', 1, struct xdma_performance_ioctl *)
#define IOCTL_XDMA_PERF_STOP    _IOW('q', 2, struct xdma_performance_ioctl *)
#define IOCTL_XDMA_PERF_GET     _IOR('q', 3, struct xdma_performance_ioctl *)
#define IOCTL_XDMA_ADDRMODE_SET _IOW('q', 4, int)
#define IOCTL_XDMA_ADDRMODE_GET _IOR('q', 5, int)
#define IOCTL_XDMA_ALIGN_GET    _IOR('q', 6, int)
#define IOCTL_XDMA_APERTURE_R   _IOW('q', 7, struct xdma_aperture_ioctl *)
#define IOCTL_XDMA_APERTURE_W   _IOW('q', 8, struct xdma_aperture_ioctl *)


#define UART_INDEX_HK                0
#define UART_INDEX_ZPOWER        1
#define UART_INDEX_JGCZ             2
#define UART_INDEX_SAR              3
#define UART_INDEX_SFANGLE       4
#define UART_INDEX_IR                 5
#define UART_INDEX_CCD              6
#define UART_INDEX_CCDIR_POWER              7

int recv_from_dev(int dev_index, char *recv_data, int *recv_len, int verbose);
int send_to_dev(int dev_index, char *send_data, int send_len, int verbose);

void hex_dump(const char *prefix, const void *addr, size_t len, size_t line_len);
int pack_ctrl_frame_send(ID_t did, ID_t sid, PORT_t port, char *data, uint32_t data_len);

int send_to_dma(char *data, uint32_t data_len);
static int send_to_dma_(char *devname, uint64_t addr, uint64_t aperture,
		    uint64_t size, uint64_t offset, uint64_t count,
            char *data, uint32_t data_len);

uint64_t getopt_integer(char *optarg);
ssize_t read_to_buffer(char *fname, int fd, char *buffer, uint64_t size,
			uint64_t base);
ssize_t write_from_buffer(char *fname, int fd, char *buffer, uint64_t size,
			uint64_t base);
static int timespec_check(struct timespec *t);
void timespec_sub(struct timespec *t1, struct timespec *t2);

#ifdef __cplusplus
}
#endif

#endif