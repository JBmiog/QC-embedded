/**
  Hongjia Wu
*/

#include "butterworth.h"

WN FIX (WN a, WN b){
    WN result;
    int temp;
    temp = a * b; 
    temp += K;
    // Correct by dividing by base
    result = (WN)temp >> A;
    return result;
}

void initbuterworth()
{
	
	int i,j;
	for (i=0; i<3; i++){
          for (j=0;j<3;j++){
              x[i][j]= 0;
              y[i][j]= 0;
            }
       }
	
}

void butterworth()
{



  int i, j;
  WN x0_1, x0_2, x0_3;
  WN x1_1, x1_2, x1_3;
  WN x2_1, x2_2, x2_3; 
  WN y1_1, y1_2, y1_3; 
  WN y2_1, y2_2, y2_3;

 

 /**update the history**/

  for (j=2; j>0; j--){
          for (i=0;i<3;i++){
              x[j][i]=x[j-1][i];
              y[j][i]=y[j-1][i];
            }
       }
  /**read data ******/
  x[0][0] = sp;
  x[0][1] = sq;
  x[0][2] = sr;
  
 /*readin the variables ***/
  x0_1 = x[0][0];
  x0_2 = x[0][1];
  x0_3 = x[0][2];
  x1_1 = x[1][0];
  x1_2 = x[1][1];
  x1_3 = x[1][2];
  x2_1 = x[2][0];
  x2_2 = x[2][1];
  x2_3 = x[2][2];
  y1_1 = y[1][0];
  y1_2 = y[1][1];
  y1_3 = y[1][2];
  y2_1 = y[2][0];
  y2_2 = y[2][1];
  y2_3 = y[2][2];
/********Calculate the values *****/

 y[0][0] = (FIX(a0,x0_1)+FIX(a1,x1_1)+FIX(a2,x2_1)-FIX(b1,y1_1)-FIX(b2,y2_1));
 y[0][1] = (FIX(a0,x0_2)+FIX(a1,x1_2)+FIX(a2,x2_2)-FIX(b1,y1_2)-FIX(b2,y2_2));
 y[0][2] = (FIX(a0,x0_3)+FIX(a1,x1_3)+FIX(a2,x2_3)-FIX(b1,y1_3)-FIX(b2,y2_3));


 /********Write the results to structure ******/
 fp = y[0][0];
 fq = y[0][1];
 fr = y[0][2]; 
 

}