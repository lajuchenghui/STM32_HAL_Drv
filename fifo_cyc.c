/* fifo_cyc.c - 数组实现的循环队列 元素大小固定为1
 *
 * 功能 循环队列
 *
 * 用法 使用前先调用初始化函数定义元素大小
 *
 */
 
#include "fifo_cyc.h"

/*
判断空
1 空
0 非空
-1错误
*/
int fifo_is_empty(fifo_cyc_t* fifo)
{
	if(fifo == NULL)
		return -1;
	
	return (fifo->head % FIFO_CYC_MAX == fifo->tail % FIFO_CYC_MAX ? 1 : 0);
}

/*
判断满
1 满
0 未满
-1错误
*/
int fifo_is_full(fifo_cyc_t* fifo)
{
	if(fifo == NULL)
		return -1;
	
	return ((fifo->tail + 1) % FIFO_CYC_MAX == (fifo->head % FIFO_CYC_MAX) ? 1 : 0);
}

/*  获得队列元素个数  */
int fifo_get_len(fifo_cyc_t* fifo)
{
	if(fifo == NULL)
		return -1;
	
	return (fifo->tail - fifo->head) % FIFO_CYC_MAX;
}

/*  入列
1 满 清空
0 成功
-1错误
*/
int fifo_in(fifo_cyc_t* fifo, char c)
{	
	static int res;
	
	res = fifo_is_full(fifo);
	/*  错误  */
	if(res == -1)
	{
		return -1;	
	}
	/*  fifo满了  */
	else if(res == 1)
	{
		/*  清空  */
		fifo->head = fifo->tail;
		return 1;
	}
	
	fifo->dat[fifo->tail] = c;
	fifo->tail++;
	fifo->tail %= FIFO_CYC_MAX;
	
	return 0;
}

/*  出列
1 满 空
0 成功
-1错误
*/
int fifo_out(fifo_cyc_t* fifo, char *c)
{	
	static int res;
	
	res = fifo_is_empty(fifo);
	/*  错误  */
	if(res == -1)
	{
		return -1;	
	}
	/*  fifo空  */
	else if(res == 1)
	{
		/*  清空  */
		return 1;
	}
	
	*c = fifo->dat[fifo->head];
	fifo->head++;
	fifo->head %= FIFO_CYC_MAX;
	
	return 0;
}
