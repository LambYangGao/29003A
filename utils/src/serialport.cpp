#include "serialport.h"
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
//#include "mylib.h"
#include <time.h>


serial_t* testlan_serial;  //测试网口接口
serial_t* fk_serial;         //飞控接口
serial_t* sf_serial;         //伺服接口
serial_t* rhsz_serial;      //融合时钟

serial_t* power_serial;        //外仓电源模块
serial_t* selftime_serial;      //自守时时间
serial_t* sfwtb_serial;          //伺服外同步


unsigned long GetTickCount()
{
	struct timespec ts = { 0 };
	clock_gettime(CLOCK_MONOTONIC, &ts);//此处可以判断一下返回
	return (ts.tv_sec * 1000 + ts.tv_nsec / (1000 * 1000));
}

int sp_init()
{
	testlan_serial = serial_new();
	if (!testlan_serial)
	{
		printf("gjjk_serial serial_new(): failed\n");
		return -1;
	}
	fk_serial = serial_new();
	if (!fk_serial)
	{
		printf("fk_serial serial_new(): failed\n");
		return -1;
	}
	sf_serial = serial_new();
	if (!sf_serial)
	{
		printf("sf_serial serial_new(): failed\n");
		return -1;
	}
	rhsz_serial = serial_new();
	if (!rhsz_serial)
	{
		printf("rhsz_serial serial_new(): failed\n");
		return -1;
	}
	power_serial = serial_new();
	if (!power_serial)
	{
		printf("power_serial serial_new(): failed\n");
		return -1;
	}
	selftime_serial = serial_new();
	if (!selftime_serial)
	{
		printf("selftime_serial serial_new(): failed\n");
		return -1;
	}
	sfwtb_serial = serial_new();
	if (!sfwtb_serial)
	{
		printf("sfwtb_serial serial_new(): failed\n");
		return -1;
	}
	return 0;
}

int sp_open(int index, int baudrate, int threshold, int parity)
{
	switch (index)
	{
	case UART_INDEX_TESTLAN:
		if (serial_open_WF3588(testlan_serial, RK_TO_TESTLAN_UART, baudrate) < 0)
		{
			fprintf(stderr, "serial_open(): %s\n", serial_errmsg(testlan_serial));
			return -1;
		}
		break;
	case UART_INDEX_FK:
		if (serial_open_WF3588(fk_serial, RK_TO_FK_UART, baudrate) < 0)
		{
			fprintf(stderr, "serial_open(): %s\n", serial_errmsg(fk_serial));
			return -1;
		}
		break;
	case UART_INDEX_RHTXSZ:
		if (serial_open_WF3588(rhsz_serial, RK_TO_RHTXSZ_UART, baudrate) < 0)
		{
			fprintf(stderr, "serial_open(): %s\n", serial_errmsg(rhsz_serial));
			return -1;
		}
		break;
	case UART_INDEX_SF:
		if (serial_open_WF3588(sf_serial, RK_TO_SF_UART, baudrate) < 0)
		{
			fprintf(stderr, "serial_open(): %s\n", serial_errmsg(sf_serial));
			return -1;
		}
		break;
	case UART_INDEX_OUT_POWER:
		if (serial_open_WF3588(power_serial, RK_TO_OUT_POWER_UART, baudrate) < 0)
		{
			fprintf(stderr, "serial_open(): %s\n", serial_errmsg(power_serial));
			return -1;
		}
		break;
	case UART_INDEX_SELF_TIME:
		if (serial_open_WF3588(selftime_serial, RK_TO_SELF_TIME_UART, baudrate) < 0)
		{
			fprintf(stderr, "serial_open(): %s\n", serial_errmsg(selftime_serial));
			return -1;
		}
		break;
	case UART_INDEX_SF_WTB:
		if (serial_open_WF3588(sfwtb_serial, RK_TO_SF_WTB_UART, baudrate) < 0)
		{
			fprintf(stderr, "serial_open(): %s\n", serial_errmsg(sfwtb_serial));
			return -1;
		}
		break;
	default:
		break;
	}
	return 0;
}

void sp_clear(int index)
{
	//switch (index)
	//{
	//case UART_INDEX_GJJK:
	//case UART_INDEX_SF:
	//case UART_INDEX_KZSB:
	//case UART_INDEX_GD:
	//case UART_INDEX_ZTDY:
	//default:
	//	break;
	//}
	
}

int32_t sp_write(int index, uint8_t* buffer, uint32_t length)
{
	int ret = -1;
	//printf("len:%d\n", length);
	switch (index)
	{
	case UART_INDEX_TESTLAN:
		ret = serial_write(testlan_serial, buffer, length);
		break;
	case UART_INDEX_FK:
		ret = serial_write(fk_serial, buffer, length);
		break;
	case UART_INDEX_RHTXSZ:
		ret = serial_write(rhsz_serial, buffer, length);
		break;
	case UART_INDEX_SF:
	{
		//printf("UART_INDEX_SF\n");
		ret = serial_write(sf_serial, buffer, length);
		break;
	}
	case UART_INDEX_OUT_POWER:
	{
		//printf("sp_write power_serial\n");
		ret = serial_write(power_serial, buffer, length);
		break;
	}
	case UART_INDEX_SELF_TIME:
		ret = serial_write(selftime_serial, buffer, length);
		break;
	case UART_INDEX_SF_WTB:
		ret = serial_write(sfwtb_serial, buffer, length);
		break;
	default:
		break;
	}
	return ret;
}

int sp_read(const int index, uint8_t* out_buffer, const int buffer_lens, const uint8_t* head, const int head_len)
{
	uint8_t c = 0;
	int ret = 0;
	int left_len = buffer_lens;
	int read_len = 0;
	unsigned long start_time = GetTickCount();

	if (head && head_len)
	{
		for (int i = 0; i < head_len; )
		{
			if (GetTickCount() - start_time > 50)
			{
				return 0;
			}
			switch (index)
			{
			case UART_INDEX_TESTLAN:
				ret = serial_read(testlan_serial, (uint8_t*)&c, 1, 200);
				break;
			case UART_INDEX_FK:
				ret = serial_read(fk_serial, (uint8_t*)&c, 1, 200);
				break;
			case UART_INDEX_RHTXSZ:
				ret = serial_read(rhsz_serial, (uint8_t*)&c, 1, 200);
				break;
			case UART_INDEX_SF:
				ret = serial_read(sf_serial, (uint8_t*)&c, 1, 200);
				break;
			case UART_INDEX_OUT_POWER:
				ret = serial_read(power_serial, (uint8_t*)&c, 1, 200);
				break;
			case UART_INDEX_SELF_TIME:
				ret = serial_read(selftime_serial, (uint8_t*)&c, 1, 200);
				break;
			case UART_INDEX_SF_WTB:
				ret = serial_read(sfwtb_serial, (uint8_t*)&c, 1, 200);
				break;
			default:
				break;
			}

			if (ret > 0)
			{
				if (c != head[i])
				{
					i = 0;
					continue;
				}
				else
				{
					i++;
				}
			}
		}

		read_len += head_len;
		left_len -= head_len;
		memcpy(out_buffer, head, head_len);
	}

	while (left_len > 0)
	{
		if (GetTickCount() - start_time > 200)
		{
			return 0;
		}
		switch (index)
		{
		case UART_INDEX_TESTLAN:
			ret = serial_read(testlan_serial, (uint8_t*)out_buffer + read_len, left_len, 200);
			break;
		case UART_INDEX_FK:
			ret = serial_read(fk_serial, (uint8_t*)out_buffer + read_len, left_len, 200);
			break;
		case UART_INDEX_RHTXSZ:
			ret = serial_read(rhsz_serial, (uint8_t*)out_buffer + read_len, left_len, 200);
			break;
		case UART_INDEX_SF:
			ret = serial_read(sf_serial, (uint8_t*)out_buffer + read_len, left_len, 200);
			break;
		case UART_INDEX_OUT_POWER:
			ret = serial_read(power_serial, (uint8_t*)out_buffer + read_len, left_len, 200);
			break;
		case UART_INDEX_SELF_TIME:
			ret = serial_read(selftime_serial, (uint8_t*)out_buffer + read_len, left_len, 200);
			break;
		case UART_INDEX_SF_WTB:
			ret = serial_read(sfwtb_serial, (uint8_t*)out_buffer + read_len, left_len, 200);
			break;
		default:
			ret = 0;
			break;
		}

		if (ret > 0 && ret <= left_len)
		{
			read_len += ret;
			left_len -= ret;
		}
		else
		{
			usleep(10);
		}
	}
	return read_len;
}

int sp_read_bylen(const int index, uint8_t* out_buffer, const int buffer_lens)
{
	uint8_t c = 0;
	int ret = 0;
	switch (index)
	{
	case UART_INDEX_TESTLAN:
		ret = serial_read(testlan_serial, (uint8_t*)out_buffer, buffer_lens, 200);
		break;
	case UART_INDEX_FK:
		ret = serial_read(fk_serial, (uint8_t*)out_buffer, buffer_lens, 200);
		break;
	case UART_INDEX_RHTXSZ:
		ret = serial_read(rhsz_serial, (uint8_t*)out_buffer,buffer_lens, 200);
		break;
	case UART_INDEX_SF:
		ret = serial_read(sf_serial, (uint8_t*)out_buffer, buffer_lens, 200);
		break;
	case UART_INDEX_OUT_POWER:
		ret = serial_read(power_serial, (uint8_t*)out_buffer, buffer_lens, 200);
		break;
	case UART_INDEX_SELF_TIME:
		ret = serial_read(selftime_serial, (uint8_t*)out_buffer, buffer_lens, 200);
		break;
	case UART_INDEX_SF_WTB:
		ret = serial_read(sfwtb_serial, (uint8_t*)out_buffer, buffer_lens, 200);
		break;
	default:
		ret = 0;
		break;
	}
	return ret;
}

void sp_close(int index)
{
	switch (index)
	{
	case UART_INDEX_TESTLAN:
		serial_close(testlan_serial);
		break;
	case UART_INDEX_FK:
		serial_close(fk_serial);
		break;
	case UART_INDEX_RHTXSZ:
		serial_close(rhsz_serial);
		break;
	case UART_INDEX_SF:
		serial_close(sf_serial);
		break;
	case UART_INDEX_OUT_POWER:
		serial_close(power_serial);
		break;
	case UART_INDEX_SELF_TIME:
		serial_close(selftime_serial);
		break;
	case UART_INDEX_SF_WTB:
		serial_close(sfwtb_serial);
		break;
	default:
		break;
	}
}

void sp_uninit()
{
	serial_free(testlan_serial);
	serial_free(sf_serial);
	serial_free(fk_serial);
	serial_free(rhsz_serial);
	serial_free(power_serial);
	serial_free(selftime_serial);
	serial_free(sfwtb_serial);
}
