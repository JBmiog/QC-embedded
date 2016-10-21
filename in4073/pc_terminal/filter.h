/**
Hongjia Wu
*/
#include "in4073.h"
#ifndef MAFILTER_H
#define MAFILTER_H

#define MEMORY 3

typedef char WN

void initfilter(void);
void filter(void);

WN fp, fq, fr;
WN fax, fay, faz;
/*** FIFO QUEUE ***/
WN ax_buffer[MEMORY];
WN ay_buffer[MEMORY];
WN az_buffer[MEMORY];

WN p_buffer[MEMORY];
WN q_buffer[MEMORY];
WN r_buffer[MEMORY];

#endif