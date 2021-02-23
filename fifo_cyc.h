#ifndef _FIFO_CYC_H_
#define _FIFO_CYC_H_

#include <stdio.h>

/*  队列数组的长度  */
#define FIFO_CYC_MAX	50

typedef struct
{
	char dat[FIFO_CYC_MAX];
	char head;
	char tail;
}fifo_cyc_t;

int fifo_is_empty(fifo_cyc_t* fifo);
int fifo_is_full(fifo_cyc_t* fifo);
int fifo_get_len(fifo_cyc_t* fifo);
int fifo_in(fifo_cyc_t* fifo, char c);
int fifo_out(fifo_cyc_t* fifo, char *c);

#endif
