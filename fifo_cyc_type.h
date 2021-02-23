#ifndef _FIFO_CYC_TYPE_H_
#define _FIFO_CYC_TYPE_H_

#include <stdio.h>
#include <stdint.h>
/*  队列数组的长度 注意 一定要是队列元素大小的倍数  */
#define FIFO_CYC_TYPE_MAX	60

typedef struct
{
	char dat[FIFO_CYC_TYPE_MAX];
	char head;
	char tail;
	int size;
}fifo_cyc_type_t;

int fifo_type_init(fifo_cyc_type_t* fifo, uint32_t size);
int fifo_type_is_empty(fifo_cyc_type_t* fifo);
int fifo_type_is_full(fifo_cyc_type_t* fifo);
int fifo_type_get_len(fifo_cyc_type_t* fifo);

int fifo_type_in(fifo_cyc_type_t* fifo, char *c);
int fifo_type_out(fifo_cyc_type_t* fifo, char *c);

#endif
