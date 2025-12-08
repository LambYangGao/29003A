#include <sys/ioctl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //read write
#include <sys/socket.h>
// #include <stdbool.h>   //use true false
#include <sys/poll.h> //poll
#include <time.h>
#include <unistd.h>
#include <errno.h>

#include "uartextend.h"

int uartextend_init(int speed, int verify, char *uart_name)
{
    int fd = -1;
    int ret = -1;
    struct user_config config = {0};
    fd = uartextend_open(uart_name);
    if (fd < 0)
    {
        printf("open %s failed\n", uart_name);
        return -1;
    }
    config.baud = speed;
    config.verify = verify;
    ret = uartextend_set(fd, config);
    if (ret)
    {
        printf("ioctl ret is %d\n", ret);
        return -1;
    }
    return fd;
}

int uartextend_open(const char *devname)
{
    return open(devname, O_RDWR | O_NONBLOCK);
}

int uartextend_close(int fd)
{
    if (fd > 0)
    {
        close(fd);
        return 0;
    }

    return -1;
}

int uartextend_set(int fd, struct user_config cfg)
{
    int ret = 0;

    if (fd < 0 || cfg.baud > PARAMENT_BAUD_MAX || cfg.verify > __PARAMENT_VERIFY_MAX)
    {
        return -1;
    }

    ret = ioctl(fd, IOCWREG, &cfg);
    if (ret)
    {
        printf("uartextend_set ioctl error ret is %d\n", ret);
    }

    return ret;
}

int uartextend_read(int fd, void *obuf, int olen)
{
	if (fd < 0 || NULL == obuf) {
		return -1;
	}

	return read(fd, obuf, olen);
}

int uartextend_write(int fd, const void *ibuf, int ilen)
{
	if (fd < 0 || NULL == ibuf) {
		return -1;
	}

	return write(fd, ibuf, ilen);
}

int uartextend_wait_interrupt(int fd, int timeout)
{
	struct pollfd fds[1];
	int ret = 0;

	do {
		if (fd < 0) {
			return -1;
		}

		fds[0].fd = fd;
		fds[0].events = POLLIN;

		ret = poll(fds, 1, timeout);

		if (ret < 0) {
			printf("uartextend_wait_interrupt error %s\n", strerror(errno));
			break;
		} else if (ret == 0) {
			break;
		} else {
			if (fds[0].revents & POLLIN) {
				ret = 1;
				break;
			}
		}


	} while (0);

	return ret;

}