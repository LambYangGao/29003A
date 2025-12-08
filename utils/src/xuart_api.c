#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h> 
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include "xuart_api.h"

#define DEVICE_NAME_DEFAULT "/dev/xdma_drv_send"
#define SIZE_DEFAULT (32)
#define COUNT_DEFAULT (1)
#define RW_MAX_SIZE	0x7ffff000

int recv_from_dev(int dev_index, char *recv_data, int *recv_len, int verbose){
	char dev_name[128];	
	int fpga_fd;
	int rc;

	//fd_set readfds;
	//struct timeval tv;
			
	if (dev_index <=0 || dev_index >=8)
	{
		perror("dev index out of range");
        return -EINVAL;
	}
	
	if (!recv_data)
	{
		perror("data buffer is NULL");
        return -EINVAL;
	}

	sprintf(dev_name, "/dev/xuart%d", dev_index);

	fpga_fd = open(dev_name, O_RDWR | O_TRUNC);
	if (fpga_fd < 0) {
        fprintf(stderr, "unable to open device %s, %d.\n",
                dev_name, fpga_fd);
		perror("open device");
                return -EINVAL;
    }

	// set timeout
	//tv.tv_sec = 5;
	//tv.tv_usec = 0;
	//FD_ZERO(&readfds);
	//FD_SET(fpga_fd, &readfds);
	//rc = select(fpga_fd + 1, &readfds, NULL, NULL, &tv);
	//if (rc == -1)
	//{

	//}

	/* read data from file into memory buffer */
	rc = read(fpga_fd, recv_data, 4096);
	if (rc == -1) {
		perror("read error");
		goto out;
	}

	*recv_len = rc;

	if (verbose)
	{
		hex_dump("recv_data:", recv_data, *recv_len, 32);
	}
	
	

out:
	close(fpga_fd);
	return rc;

}

int send_to_dev(int dev_index, char *send_data, int send_len, int verbose){
	int rc;

	if (verbose)
	{
		hex_dump("sendData:", send_data, send_len, 32);
	}
	
	rc = pack_ctrl_frame_send(0x81, 0, dev_index, send_data, send_len);

	return rc;
}

void hex_dump(const char *prefix, const void *addr, size_t len, size_t line_len) {
    const unsigned char *ptr = addr;
    size_t i, j;

    // 如果没有指定前缀，默认设置为空字符串
    if (prefix == NULL) {
        prefix = "";
    }

    for (i = 0; i < len; i += line_len) {
        // 打印前缀
        printf("%s", prefix);

        // 打印地址（按行显示）
        printf("%08zx  ", i);

        // 打印十六进制数据
        for (j = 0; j < line_len && i + j < len; j++) {
            printf("%02x ", ptr[i + j]);
        }

        // 填充不足 line_len 字节的十六进制部分
        for (; j < line_len; j++) {
            printf("   ");
        }

        // 打印 ASCII 字符（不可打印字符显示为 '.'）
        printf(" |");
        for (j = 0; j < line_len && i + j < len; j++) {
            unsigned char c = ptr[i + j];
            printf("%c", isprint(c) ? c : '.');
        }
        printf("|\n");
    }
}

int pack_ctrl_frame_send(ID_t did, ID_t sid, PORT_t port, char *data, uint32_t data_len){
	int ret = 0;
	size_t frame_size = sizeof(struct ctrl_frame_t) + data_len;
    struct ctrl_frame_t *ctrl_frame = malloc(frame_size);

    if (ctrl_frame == NULL) {
        perror("ctrl_frame Memory allocation failed");
        return -1;
    }

	memset(ctrl_frame, 0, frame_size);
	ctrl_frame->DID = did;
	ctrl_frame->SID = sid;
	ctrl_frame->port= port;
	ctrl_frame->cmd_= WRITE_CMD;
	ctrl_frame->ack_= READ_ACK_CMD;
	ctrl_frame->H_sn = 0;
	ctrl_frame->frame_sn = 0;
	ctrl_frame->camera_flag = 0;
	memcpy(ctrl_frame->data, data, data_len);

	ret = send_to_dma((char*)ctrl_frame, frame_size);
	
	if(ret < 0){
		printf("send_to_dma error\n");
	}

	return 0;
}

int send_to_dma(char *data, uint32_t data_len){
	// int cmd_opt;
	char *device = DEVICE_NAME_DEFAULT;
	uint64_t address = 0;
	uint64_t aperture = 0;
	uint64_t size = SIZE_DEFAULT;
	uint64_t offset = 0;
	uint64_t count = COUNT_DEFAULT;

	return send_to_dma_(device, address, aperture, size, offset, count,
			data, data_len);
}

static int send_to_dma_(char *devname, uint64_t addr, uint64_t aperture,
		    uint64_t size, uint64_t offset, uint64_t count,
            char *data, uint32_t data_len)
		    // char *infname, char *ofname)
{
	uint64_t i;
	ssize_t rc;
	size_t bytes_done = 0;
	// size_t out_offset = 0;
	char *buffer = NULL;
	char *allocated = NULL;
	struct timespec ts_start, ts_end;
	int infile_fd = -1;
	int outfile_fd = -1;
	int fpga_fd = open(devname, O_RDWR);
	long total_time = 0;
	float result;
	float avg_time = 0;
	int underflow = 0;

	if (fpga_fd < 0) {
		fprintf(stderr, "unable to open device %s, %d.\n",
			devname, fpga_fd);
		perror("open device");
		return -EINVAL;
	}

    if(data == NULL){
        goto out;
    }

    if(data_len > 4096){
        printf("send_data size too long\n");
        return -1;
    }
	posix_memalign((void **)&allocated, 4096 /*alignment */ , size + 4096);
	if (!allocated) {
		fprintf(stderr, "OOM %lu.\n", size + 4096);
		rc = -ENOMEM;
		goto out;
	}

	buffer = allocated + offset;

    memcpy(buffer, data, data_len);
	for (i = 0; i < count; i++) {
		/* write buffer to AXI MM address using SGDMA */
		rc = clock_gettime(CLOCK_MONOTONIC, &ts_start);

		if (aperture) {
			struct xdma_aperture_ioctl io;

			io.buffer = (unsigned long)buffer;
			io.len = size;
			io.ep_addr = addr;
			io.aperture = aperture;
			io.done = 0UL;

			rc = ioctl(fpga_fd, IOCTL_XDMA_APERTURE_W, &io);
			if (rc < 0 || io.error) {
				fprintf(stdout,
					"#%ld: aperture W ioctl failed %ld,%d.\n",
					i, rc, io.error);
				goto out;
			}

			bytes_done = io.done;
		} else {
			rc = write_from_buffer(devname, fpga_fd, buffer, data_len,
				      	 	addr);
			if (rc < 0)
				goto out;
			bytes_done = rc;
		}

		rc = clock_gettime(CLOCK_MONOTONIC, &ts_end);

		if (bytes_done < data_len) {
			printf("#%ld: underflow %ld/%ld.\n",
				i, bytes_done, size);
			underflow = 1;
		}

		/* subtract the start time from the end time */
		timespec_sub(&ts_end, &ts_start);
		total_time += ts_end.tv_nsec;
	}

out:
	close(fpga_fd);
	if (infile_fd >= 0)
		close(infile_fd);
	if (outfile_fd >= 0)
		close(outfile_fd);
	free(allocated);

	if (rc < 0)
		return rc;
	/* treat underflow as error */
	return underflow ? -EIO : 0;
}

uint64_t getopt_integer(char *optarg)
{
	int rc;
	uint64_t value;

	rc = sscanf(optarg, "0x%lx", &value);
	if (rc <= 0)
		rc = sscanf(optarg, "%lu", &value);
	//printf("sscanf() = %d, value = 0x%lx\n", rc, value);

	return value;
}

ssize_t read_to_buffer(char *fname, int fd, char *buffer, uint64_t size,
			uint64_t base)
{
	ssize_t rc;
	uint64_t count = 0;
	char *buf = buffer;
	off_t offset = base;
	int loop = 0;

	while (count < size) {
		uint64_t bytes = size - count;

		if (bytes > RW_MAX_SIZE)
			bytes = RW_MAX_SIZE;

		if (offset) {
			rc = lseek(fd, offset, SEEK_SET);
			if (rc != offset) {
				fprintf(stderr, "%s, seek off 0x%lx != 0x%lx.\n",
					fname, rc, offset);
				perror("seek file");
				return -EIO;
			}
		}

		/* read data from file into memory buffer */
		rc = read(fd, buf, bytes);
		if (rc < 0) {
			fprintf(stderr, "%s, read 0x%lx @ 0x%lx failed %ld.\n",
				fname, bytes, offset, rc);
			perror("read file");
			return -EIO;
		}

		count += rc;
		if (rc != bytes) {
			fprintf(stderr, "%s, read underflow 0x%lx/0x%lx @ 0x%lx.\n",
				fname, rc, bytes, offset);
			break;
		}

		buf += bytes;
		offset += bytes;
		loop++;
	}

	if (count != size && loop)
		fprintf(stderr, "%s, read underflow 0x%lx/0x%lx.\n",
			fname, count, size);
	return count;
}

ssize_t write_from_buffer(char *fname, int fd, char *buffer, uint64_t size,
			uint64_t base)
{
	ssize_t rc;
	uint64_t count = 0;
	char *buf = buffer;
	off_t offset = base;
	int loop = 0;

	while (count < size) {
		uint64_t bytes = size - count;

		if (bytes > RW_MAX_SIZE)
			bytes = RW_MAX_SIZE;

		if (offset) {
			rc = lseek(fd, offset, SEEK_SET);
			if (rc != offset) {
				fprintf(stderr, "%s, seek off 0x%lx != 0x%lx.\n",
					fname, rc, offset);
				perror("seek file");
				return -EIO;
			}
		}

		/* write data to file from memory buffer */
		rc = write(fd, buf, bytes);
		if (rc < 0) {
			fprintf(stderr, "%s, write 0x%lx @ 0x%lx failed %ld.\n",
				fname, bytes, offset, rc);
			perror("write file");
			return -EIO;
		}

		count += rc;
		if (rc != bytes) {
			fprintf(stderr, "%s, write underflow 0x%lx/0x%lx @ 0x%lx.\n",
				fname, rc, bytes, offset);
			break;
		}
		buf += bytes;
		offset += bytes;

		loop++;
	}	

	if (count != size && loop)
		fprintf(stderr, "%s, write underflow 0x%lx/0x%lx.\n",
			fname, count, size);

	return count;
}

/* Subtract timespec t2 from t1
 *void timespec_sub(struct timespec *t1, struct timespec *t2)
 * Both t1 and t2 must already be normalized
 * i.e. 0 <= nsec < 1000000000
 */
static int timespec_check(struct timespec *t)
{
	if ((t->tv_nsec < 0) || (t->tv_nsec >= 1000000000))
		return -1;
	return 0;

}

void timespec_sub(struct timespec *t1, struct timespec *t2)
{
	if (timespec_check(t1) < 0) {
		fprintf(stderr, "invalid time #1: %lld.%.9ld.\n",
			(long long)t1->tv_sec, t1->tv_nsec);
		return;
	}
	if (timespec_check(t2) < 0) {
		fprintf(stderr, "invalid time #2: %lld.%.9ld.\n",
			(long long)t2->tv_sec, t2->tv_nsec);
		return;
	}
	t1->tv_sec -= t2->tv_sec;
	t1->tv_nsec -= t2->tv_nsec;
	if (t1->tv_nsec >= 1000000000) {
		t1->tv_sec++;
		t1->tv_nsec -= 1000000000;
	} else if (t1->tv_nsec < 0) {
		t1->tv_sec--;
		t1->tv_nsec += 1000000000;
	}
}
