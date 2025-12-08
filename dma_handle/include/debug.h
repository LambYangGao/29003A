#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#ifdef __KERNEL__
#include <linux/printk.h>
#define PRINT printk
#else
#include <stdio.h>
#define PRINT printf
#endif

#undef NOTE
#undef DEBUG
#undef INFO
#undef WARN
#undef ERROR

extern int g_debug_level;

#define NOTE(fmt,args...) do\
		{\
			if (g_debug_level >= 4) \
			{\
				PRINT("[NOTE ][%s,%d]:"fmt,__FUNCTION__,__LINE__,##args);\
			}\
		} while(0)

#define DEBUG(fmt,args...) do\
		{\
			if (g_debug_level >= 3) \
			{\
				PRINT("[DEBUG][%s,%d]:"fmt,__FUNCTION__,__LINE__,##args);\
			}\
		} while(0)

#define INFO(fmt,args...) do\
		{\
			if (g_debug_level >= 2) \
			{\
				PRINT("[INFO ][%s,%d]:"fmt,__FUNCTION__,__LINE__,##args);\
			}\
		} while(0)

#define WARN(fmt,args...) do\
		{\
			if (g_debug_level >= 1) \
			{\
				PRINT("[WARN ][%s,%d]:"fmt,__FUNCTION__,__LINE__,##args);\
			}\
		} while(0)

#define ERROR(fmt,args...) do\
		{\
			if (g_debug_level >= 0) \
			{\
				PRINT("[ERROR][%s,%d]:"fmt,__FUNCTION__,__LINE__,##args);\
			}\
		} while(0)

#define EnterFunction() \
	do{\
		PRINT(KERN_INFO "Enter: %s, %d\n",  __FUNCTION__, __LINE__); \
	} while (0)

#define LeaveFunction()  \
	do {\
		PRINT(KERN_INFO "Leave: %s, %d\n", 	__FUNCTION__, __LINE__);\
	} while (0)

void hexdump(char *msg, char *pdata, int len);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* _DEBUG_H_ */
