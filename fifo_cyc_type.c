/* fifo_cyc_type.c - 数组实现的循环队列 元素大小可自定义
 *
 * 功能 循环队列
 *
 * 用法 使用前先调用初始化函数定义元素大小
 *
 */

#include "fifo_cyc_type.h"

/*  初始化一个循环队列 定义元素大小  */
int fifo_type_init(fifo_cyc_type_t* fifo, uint32_t size)
{
	fifo->size = size;
}

/*
判断空
1 空
0 非空
-1错误
*/
int fifo_type_is_empty(fifo_cyc_type_t* fifo)
{
	if(fifo == NULL)
		return -1;
	
	return (fifo->head % FIFO_CYC_TYPE_MAX == fifo->tail % FIFO_CYC_TYPE_MAX ? 1 : 0);
}

/*
判断满
1 满
0 未满
-1错误
*/
int fifo_type_is_full(fifo_cyc_type_t* fifo)
{
	if(fifo == NULL)
		return -1;
	
	return ((fifo->tail + fifo->size) % FIFO_CYC_TYPE_MAX == (fifo->head % FIFO_CYC_TYPE_MAX) ? 1 : 0);
}

/*  获得队列元素个数  */
int fifo_type_get_len(fifo_cyc_type_t* fifo)
{
	if(fifo == NULL)
		return -1;
	
	return ((fifo->tail - fifo->head) % FIFO_CYC_TYPE_MAX / fifo->size);
}

/*  入列
1 满 清空
0 成功
-1错误
*/
int fifo_type_in(fifo_cyc_type_t* fifo, char *c)
{	
	static int res;
	
	res = fifo_type_is_full(fifo);
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
	
	//fifo->dat[fifo->tail] = c;
	memcpy(&fifo->dat[fifo->tail], c, fifo->size);
	fifo->tail += fifo->size;
	fifo->tail %= FIFO_CYC_TYPE_MAX;
	
	return 0;
}

/*  出列
1 满 空
0 成功
-1错误
*/
int fifo_type_out(fifo_cyc_type_t* fifo, char *c)
{	
	static int res;
	
	res = fifo_type_is_empty(fifo);
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
	
	//*c = fifo->dat[fifo->head];
	memcpy(c, &fifo->dat[fifo->head], fifo->size);
	fifo->head += fifo->size;
	fifo->head %= FIFO_CYC_TYPE_MAX;
	
	return 0;
}
