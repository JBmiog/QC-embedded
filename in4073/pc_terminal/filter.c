/**
Hongjia Wu
*/

#include "filter.h"


void initfilter()
{
    int i=0;

    for(i=0; i<MEMORY; i++)
        ax_buffer[i] = 0;
    for(i=0; i<MEMORY; i++)
        ay_buffer[i] = 0;
    for(i=0; i<MEMORY; i++)
        az_buffer[i] = 0;

    for(i=0; i<MEMORY; i++)
        p_buffer[i] = 0;
    for(i=0; i<MEMORY; i++)
        q_buffer[i] = 0;
    for(i=0; i<MEMORY; i++)
        r_buffer[i] = 0;
}

void filter()
{
    int i = 0;
    int sum = 0;

    /***Push new data***/
    for(i=0; i<MEMORY-1; i++)
        ax_buffer[i+1] = ax_buffer[i];
    for(i=0; i<MEMORY-1; i++)
        ay_buffer[i+1] = ay_buffer[i];
    for(i=0; i<MEMORY-1; i++)
        az_buffer[i+1] = az_buffer[i];

    ax_buffer[0] = sax;
    ay_buffer[0] = say;
    az_buffer[0] = saz;


    for(i=0; i<MEMORY-1; i++)
        p_buffer[i+1] = p_buffer[i];
    for(i=0; i<MEMORY-1; i++)
        q_buffer[i+1] = q_buffer[i];
    for(i=0; i<MEMORY-1; i++)
        r_buffer[i+1] = r_buffer[i];

    p_buffer[0] = sp;
    q_buffer[0] = sq;
    r_buffer[0] = sr;

    /***Output filtered data***/

    for(i=0, sum=0; i<MEMORY; i++)
        sum += ax_buffer[i];
		fax = (WN)(sum/MEMORY);

    for(i=0, sum=0; i<MEMORY; i++)
        sum += ay_buffer[i];
		fay = (WN)(sum/MEMORY);

    for(i=0, sum=0; i<MEMORY; i++)
        sum += az_buffer[i];
		faz = (WN)(sum/MEMORY);


    for(i=0, sum=0; i<MEMORY; i++)
        sum += p_buffer[i];
		fp = (WN)(sum/MEMORY);

    for(i=0, sum=0; i<MEMORY; i++)
        sum += q_buffer[i];
		fq = (WN)(sum/MEMORY);

    for(i=0, sum=0; i<MEMORY; i++)
        sum += r_buffer[i];
		fr = (WN)(sum/MEMORY);
		
}