#include "video_io.h"

VideoXMDA::VideoXMDA()
{
	index_ir = 0;
	index_ccd = 1;
}

VideoXMDA::~VideoXMDA()
{

}

int VideoXMDA::init_xmda(int mainView)
{
	int fd_ccd=-1;
	int fd_ir = -1;
	//IR index
	if (mainView == 0)
	{
		fd_ccd = xvideo_open(index_ccd);
		if (fd_ccd < 0) {
			printf("video%d fail to open\n", index_ccd);
			return -1;
		}
		else
		{
			return fd_ccd;
		}
	}
	else
	{
		fd_ir = xvideo_open(index_ir);
		if (fd_ir < 0) {
			printf("video%d fail to open\n", index_ir);
			return -1;
		}
		else
		{
			return fd_ir;
		}
	}

	return 0;
}

bool isSavePic = false;
int VideoXMDA::readImgBuf(int mainView,int fpga_fd, void* data, int size)
{
	int res = 0;
	if (mainView == 0)  
	{
		memset(data, 0, size);
		res = xvideo_read(fpga_fd, (char*)data, size);
	}
	else if (mainView == 1)
	{
		//红外是Y通道
		memset(data, 0x80, size);
		res = xvideo_read(fpga_fd, (char*)data, size*2/3);
	}
	//printf("index : %d, xvideo_read res : %d \n", mainView, res);
	return res;
}

int VideoXMDA::close_fd(int fpga_fd)
{
	int ret= xvideo_close(fpga_fd);
	return ret;
}
