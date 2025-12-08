#pragma once
#include "xvideo_api.h"
#include "opencv2/opencv.hpp"
class VideoXMDA {
public:
	VideoXMDA();
	~VideoXMDA();

	int init_xmda(int mainView);
	int readImgBuf(int mainView, int fpga_fd,void *data,int size);

	int close_fd(int fpga_fd);
private:
	int index_ir;
	int index_ccd;
};