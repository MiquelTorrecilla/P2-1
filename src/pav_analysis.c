#include <math.h>
#include "pav_analysis.h"
float compute_power (const float *x, unsigned int N){
   float p =0;
    for (int i=0;i<N;i++){
       p = p + x[i]*x[i]; 
    }
    p =10*log10(p/N);
    return p;
}

float compute_am (const float *x, unsigned int N) {
   float p=0;
    for ( int i=0;i<N;i++){
        p = p + fabs(x[i]); 
    }
    p =p/N;
    return p;
}

float compute_zcr (const float *x, unsigned int N, float fm){
    float p=0, y=0;
    for (int i=0;i<N;i++){
        if((x[i]<0 && (x[i-1])> 0) || (x[i]>0 && (x[i-1]) < 0)){
            y = y + 1;   
        } 
    }
        p = fm/(2*(N-1))*y;
    return p;
}