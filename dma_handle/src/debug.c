#include <linux/module.h>
#include <linux/string.h>
#include "debug.h"

int g_debug_level = 0;

#ifdef __KERNEL__
	module_param(g_debug_level, int, 0644);
#endif

void hexdump(char *msg, char *pdata, int len)
{
	int i = 0;
	char buf[512] = {0,};

	if (g_debug_level < 2) {
		return ;
	}

	if ((NULL == pdata) || (len <= 0)) {
		ERROR("pdata=%p, len=%d\n", pdata, len);
		return;
	}

	PRINT("================= %s START =================\n", msg ? msg : "");
	INFO("pdata=%p, len=%d\n", pdata, len);
	/* head style */
	memset(buf, 0, sizeof(buf));
	sprintf(buf + strlen(buf), "%-4s| ", "Hex.");
	for (i = 1; i <= 16; i++) {
		sprintf(buf + strlen(buf), "%02d ", i & 0xff);
	}
	PRINT("%s\n", buf);

	memset(buf, 0, sizeof(buf));
	sprintf(buf + strlen(buf), "%4s+-", "----");
	for (i = 0; i < 16; i++) {
		sprintf(buf + strlen(buf), "%2s-", "--");
	}
	PRINT("%s\n", buf);

	/* data info */
	memset(buf, 0, sizeof(buf));
	sprintf(buf + strlen(buf), "%04d| ", 0);
	for (i = 0; i < len ; i ++) {
		if ((i != 0) && (i % 16 == 0)) {
			PRINT("%s\n", buf);
			memset(buf, 0, sizeof(buf));
			sprintf(buf + strlen(buf), "%04d| ", i);
		}
		sprintf(buf + strlen(buf), "%02x ", pdata[i] & 0xff);
	}
	PRINT("%s\n", buf);
	PRINT("================= %s END   =================\n", msg ? msg : "");

	return;
}
