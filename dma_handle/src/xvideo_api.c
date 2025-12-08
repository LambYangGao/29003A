#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <libgen.h>
#include <errno.h>

#include "xvideo_api.h"
#include "debug.h"

#define MAX_VIDEO_NUM 2
#define XVIDEO_DEV_PREFIX "xvideo"
//index 0红外640*512 I420
//index 1可见光1920*1080*2 YUV422
int xvideo_open(int index)
{
	int fd = 0;
	char devname[128] = {0};

	do {
		if (index >= MAX_VIDEO_NUM || index < 0) {
			printf("invalid arg: index=%d, expect 0-%d\n", index, MAX_VIDEO_NUM - 1);
			fd = -1;
			break;
		}

		snprintf(devname, sizeof(devname) - 1, "/dev/%s%d", XVIDEO_DEV_PREFIX, index);
		fd = open(devname, O_RDWR);
		if (fd == -1) {
			printf("open %s: %s", devname, strerror(errno));
			break;
		}
	} while (0);

	return fd;
}

int xvideo_read(int fd, char obuf[], int olen)
{
	if (fd < 0 ||  NULL == obuf) {
		return -1;
	}

	return read(fd, obuf, olen);
}

int xvideo_close(int fd)
{
	if (fd > 0) {
		close(fd);
		return 0;
	}

	return -1;
}
