#ifndef _XVIDEO_API_H_
#define _XVIDEO_API_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include <stdio.h>

/**
int xvideo_open(int index);
	index:
		0.  1280 * 2 * 2, //video1 14bit, 红外
		1.  1280 * 2 * 1, //video2 8bit, 红外
		2.  1920 * 2 * 3, //video3 16bit, 可见光转RGB
		3.  1280 * 2 * 2, //video4 14bit, 红外
		4.  1280 * 2 * 2, //video5 14bit, 红外
		5.  1280 * 2 * 2, //video6 14bit, 红外
		6.  1280 * 2 * 2, //video7 14bit, 红外
		7.  1280 * 2 * 2, //video8 14bit, 红外
		8.  1280 * 2 * 1, //video9 8bit, 红外
		9. 1280 * 2 * 1, //video9 8bit, 红外
		10. 1280 * 2 * 1, //video9 8bit, 红外
		11. 1280 * 2 * 1, //video9 8bit, 红外
		12. 1280 * 2 * 1, //video9 8bit, 红外
	return:
		>=0, 成功,
		-1, 错误.
*/
// index :1 可见光  1920x1080x2 YUV-422
// index: 0红外    640x512  YUV—I420
int xvideo_open(int index);


/**
int xvideo_read(int fd, char obuf[], int olen);
	fd:
		xvideo_open() 的返回值

	obuf:
		读取数据存放空间

	olen:
		数据存放空间长度

	return:
		>0, 实际读取到的字节长度
		-1, 错误.
*/
int xvideo_read(int fd, char obuf[], int olen);


/**
int xvideo_close(int fd)
	fd:
		xvideo_open() 的返回值

	return:
		=0, 成功,
		-1, 错误.
*/
int xvideo_close(int fd);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* _XVIDEO_API_H_ */
