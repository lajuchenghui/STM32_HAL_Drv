#ifndef _DBGS_H_
#define _DBGS_H_
/*
通过配置一个uart进行printf打印
可以分打印等级 可以打印出函数所在文件路径和行号
在keil中要打开microlib 在iar中general options的library config中的库等级设置为full
*/
#include "stdio.h"
#include "stm32f0xx_hal.h"

#define debug_printf					printf

#define EPDK_DEBUG_LEVEL_GATE        	3

#define EPDK_DEBUG_LEVEL_NOCHECK     	0            /* No run time checks are performed                             */
#define EPDK_DEBUG_LEVEL_CHECK_ALL   	1            /* Parameter checks and consistency checks are performed        */
#define EPDK_DEBUG_LEVEL_LOG_ERRORS  	2            /* Errors are recorded                                          */
#define EPDK_DEBUG_LEVEL_LOG_WARNINGS	3            /* Errors & Warnings are recorded                               */
#define EPDK_DEBUG_LEVEL_LOG_ALL     	4            /* Errors, Warnings and Messages are recorded.                  */

#if(EPDK_DEBUG_LEVEL_GATE == 0)

#elif(EPDK_DEBUG_LEVEL_GATE == 1)
#define __ASSERT
#elif(EPDK_DEBUG_LEVEL_GATE == 2)
#define __ERR
#define __ASSERT
#elif(EPDK_DEBUG_LEVEL_GATE == 3)
#define __ERR
#define __ASSERT
#define __WRN
#elif(EPDK_DEBUG_LEVEL_GATE == 4)
#define __ERR
#define __ASSERT
#define __WRN
#define __MSG
#define __HERE
#define __INF

#else
#error EPDK_DEBUG_LEVEL_GATE is error, please config DEBUGLEVEL to 0/1/2/3!
#endif

#ifdef __ASSERT
#define __ast(condition)    (if(!condition)                                                     \
                             debug_printf("AST:L%d(%s): condition err!", __LINE__, __FILE__)   )
#else
#define __ast(condition)
#endif

#if defined(__MSG)
#define __msg(...)    		(debug_printf("MSG:L%d(%s):", __LINE__, __FILE__),                 \
                           debug_printf(__VA_ARGS__)									        )
#else
#define __msg(...)
#endif

#if defined(__ERR)
#define __err(...)          (debug_printf("ERR:L%d(%s):", __LINE__, __FILE__),                 \
                             debug_printf(__VA_ARGS__),                                          
                             //reset
#else
#define __err(...)    		 //reset   
#endif

#if defined(__WRN)
#define __wrn(...)          (debug_printf("WRN:L%d(%s):", __LINE__, __FILE__),                 \
                             debug_printf(__VA_ARGS__)									        )
#else
#define __wrn(...)
#endif

#if defined(__HERE)
#define __here__            debug_printf("@L%d(%s)\n", __LINE__, __FILE__);
#define __wait__            (debug_printf("@L%d(%s)(press any key:\n", __LINE__, __FILE__),                 \
                             __getc())
#else
#define __here__
#define __wait__
#endif

#if defined(__INF)
#define __inf(...)           debug_printf(__VA_ARGS__)
#else
#define __inf(...)
#endif

#define __log(...)          debug_printf(__VA_ARGS__)

int32_t dbg_seek_arg(char *buf, char *buf_dir, int32_t def);

#endif  /* _DBGS_H_ */
