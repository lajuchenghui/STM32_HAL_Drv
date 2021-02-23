/* str_cmd.c - 字符串命令控制台
 * 
 * 功能 用于串口控制台 输入命令执行函数  
 *
 * 用法 在结构体cmd_list中填入命令字符串和对应函数 将函数str_cmd_resolve放在串口接收后处理之后
 *
 */
#include "str_cmd.h"
#include "lv_beat.h"

/*  调试输出信息  */
#define STR_CMD_PRINTF
#ifdef STR_CMD_PRINTF
#define str_cmd_printf		printf
#else
#define str_cmd_printf(...)
#endif

typedef struct
{
	const char *cmd;
	void (*func)(char *buf);
}cmd_line_t;


#define CMD_LINE_MAX				10

const char str_temp_virtual[] = "temp virtual";
const char str_p_prot_virtual[] = "p prot virtual";
const char str_rotary_virtual[] = "rotary virtual";

const char str_csl_report[] = "csl_report";
const char str_lv_beat[] = "lv_beat";
const char str_up[] = "up";
const char str_down[] = "down";

const char str_open[] = "open";
const char str_close[] = "close";
const char str_read[] = "read";
const char str_write[] = "write";

static void stc_cmd_csl_report_handler(char *buf);
static void stc_cmd_lv_beat_handler(char *buf);

static int str_cmd_enter(char *buf, int len);
static int str_cmd_check(char *buf);

/*  命令和对应函数的结构体 最后要预留一个{0, 0}, 作为结尾判断 注意宏CMD_LINE_MAX为命令最大数目  */
cmd_line_t cmd_list[CMD_LINE_MAX] = 
{
	{str_csl_report, stc_cmd_csl_report_handler},
	{str_lv_beat, stc_cmd_lv_beat_handler},
	{0, 0},
};


/* 功能 从buf寻找cmd_list成员cmd 并执行对应的函数func 这个buf必须以\n结尾
 * 参数 buf 要寻找的字符数组 len 字符数组长度
 * 用法 放在串口接收后处理之后 返回 0 1时清空字符数组
 * 返回 -1 还没找到结尾\n 1 找到\n并成功执行命令对应函数 0 找到\n但是没有找到对应命令
 */
int str_cmd_resolve(char *buf, int len)
{
	/*  找结尾\n  */
	if(str_cmd_enter(buf, len) == -1)
	{
		return -1;
	}
	str_cmd_printf("check enter\n");
	/*  寻找cmd并执行对应函数func  */
	if(str_cmd_check(buf) == 0)
	{
		str_cmd_printf("cmd succ\n");
		return 1;
	}
	else
	{
		str_cmd_printf("cmd fail\n");
		str_cmd_printf("buf = %s\n", buf);
		return 0;
	}
	
	return 1;
}

/*  寻找到字符串 csl_report 后执行的函数  */
static void stc_cmd_csl_report_handler(char *buf)
{
	str_cmd_printf("stc_cmd_csl_report_handler\n");
	csl_slave_send_irq();
}

/*  寻找到字符串 lv_beat 后执行的函数  */
/*  解析命令如 lv_beat up 20 down 50 的参数up down 值分别为 20 50  */
static void stc_cmd_lv_beat_handler(char *buf)
{
	char *p;
	
	char str_num[10] = {0}; 
	uint32_t i_num_up;
	uint32_t i_num_down;
	
	str_cmd_printf("stc_cmd_lv_beat_handler\n");
	
	/*  lv beat up  */
	p = strstr(buf, str_up);
	if(p != 0)
	{
		/*  最多取4位  */
		sscanf((char *)(p + strlen(str_up) + 1), "%4s", str_num);
		i_num_up = atoi(str_num);
		memset(str_num, 0, 10);
	}
	
	/*  lv beat down  */
	p = strstr(buf, str_down);
	if(p != 0)
	{
		/*  最多取4位  */
		sscanf((char *)(p + strlen(str_down) + 1), "%4s", str_num);
		i_num_down = atoi(str_num);
		//temp = temp_val_read(i_num);
	}
	envelope_extract_arg(i_num_up, i_num_down);
	str_cmd_printf("up = %d down = %d\n", i_num_up, i_num_down);
	
}

/*  找\n 成功返回位置索引 失败返回-1  */
static int str_cmd_enter(char *buf, int len)
{
	char *p;
	
	p = (char *)memchr(buf, '\n', len);
	
	if(p)
	{
		return (p - buf);
	}
	
	return -1;
}


/*  从结构体数组cmd_list中遍历成员cmd是否为参数buf的一个子字符串 是则执行结构体数组的成员函数func  */
/*  解析到命令 返回 0 失败返回 -1  */
static int str_cmd_check(char *buf)
{
	int i;
	char *p;
	
	for(i=0;i<CMD_LINE_MAX;i++)
	{
		if(cmd_list[i].cmd == 0)
		{
			return -1;
		}
		
		p = strstr(buf, cmd_list[i].cmd);
		if(p)
		{
			p += (strlen(cmd_list[i].cmd) + 1);
			/*  buf = "led on" 将o的指针传入参数  */
			if(cmd_list[i].func)
			{
				cmd_list[i].func(p);
			}
			
			return 0;
		}
	}
	
	return -1;
}
