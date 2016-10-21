/**
  Hongjia Wu
*/

#ifndef BUTTERWORTH_H
#define BUTTERWORTH_H

#include <stdlib.h>

typedef char WN

#define a0 3384 //Second-Order Butterworth constants in q14 Sampleing Frequentie = 500Hz, Cutoff @100Hz
#define a1 6769
#define a2 3384
#define b1 -6054
#define b2 3208
#define A 14
#define K (1 << (A - 1))


int Sample=0;

  
WN fp;
WN fq;
WN fr;


WN x[3][3];
WN y[3][3];

void initbuterworth(void);
void butterworth(void);

#endif//BUTTERWORTH_H