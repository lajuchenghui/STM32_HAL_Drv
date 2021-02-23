/* str_cmd.c - �ַ����������̨
 * 
 * ���� ���ڴ��ڿ���̨ ��������ִ�к���  
 *
 * �÷� �ڽṹ��cmd_list�����������ַ����Ͷ�Ӧ���� ������str_cmd_resolve���ڴ��ڽ��պ���֮��
 *
 */
#include "str_cmd.h"
#include "lv_beat.h"

/*  ���������Ϣ  */
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

/*  ����Ͷ�Ӧ�����Ľṹ�� ���ҪԤ��һ��{0, 0}, ��Ϊ��β�ж� ע���CMD_LINE_MAXΪ���������Ŀ  */
cmd_line_t cmd_list[CMD_LINE_MAX] = 
{
	{str_csl_report, stc_cmd_csl_report_handler},
	{str_lv_beat, stc_cmd_lv_beat_handler},
	{0, 0},
};


/* ���� ��bufѰ��cmd_list��Աcmd ��ִ�ж�Ӧ�ĺ���func ���buf������\n��β
 * ���� buf ҪѰ�ҵ��ַ����� len �ַ����鳤��
 * �÷� ���ڴ��ڽ��պ���֮�� ���� 0 1ʱ����ַ�����
 * ���� -1 ��û�ҵ���β\n 1 �ҵ�\n���ɹ�ִ�������Ӧ���� 0 �ҵ�\n����û���ҵ���Ӧ����
 */
int str_cmd_resolve(char *buf, int len)
{
	/*  �ҽ�β\n  */
	if(str_cmd_enter(buf, len) == -1)
	{
		return -1;
	}
	str_cmd_printf("check enter\n");
	/*  Ѱ��cmd��ִ�ж�Ӧ����func  */
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

/*  Ѱ�ҵ��ַ��� csl_report ��ִ�еĺ���  */
static void stc_cmd_csl_report_handler(char *buf)
{
	str_cmd_printf("stc_cmd_csl_report_handler\n");
	csl_slave_send_irq();
}

/*  Ѱ�ҵ��ַ��� lv_beat ��ִ�еĺ���  */
/*  ���������� lv_beat up 20 down 50 �Ĳ���up down ֵ�ֱ�Ϊ 20 50  */
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
		/*  ���ȡ4λ  */
		sscanf((char *)(p + strlen(str_up) + 1), "%4s", str_num);
		i_num_up = atoi(str_num);
		memset(str_num, 0, 10);
	}
	
	/*  lv beat down  */
	p = strstr(buf, str_down);
	if(p != 0)
	{
		/*  ���ȡ4λ  */
		sscanf((char *)(p + strlen(str_down) + 1), "%4s", str_num);
		i_num_down = atoi(str_num);
		//temp = temp_val_read(i_num);
	}
	envelope_extract_arg(i_num_up, i_num_down);
	str_cmd_printf("up = %d down = %d\n", i_num_up, i_num_down);
	
}

/*  ��\n �ɹ�����λ������ ʧ�ܷ���-1  */
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


/*  �ӽṹ������cmd_list�б�����Աcmd�Ƿ�Ϊ����buf��һ�����ַ��� ����ִ�нṹ������ĳ�Ա����func  */
/*  ���������� ���� 0 ʧ�ܷ��� -1  */
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
			/*  buf = "led on" ��o��ָ�봫�����  */
			if(cmd_list[i].func)
			{
				cmd_list[i].func(p);
			}
			
			return 0;
		}
	}
	
	return -1;
}
