/*
 * Zipf implementation
 */
#include <stdlib.h>
#include <string.h>
#include "zipf.h"

struct probvals* 
zipf_init(int N)
{
    struct probvals *zdist;

    zdist = (struct probvals *)malloc(N * sizeof(struct probvals));
    memset(zdist, 0x00, sizeof(struct probvals) * N);

    return zdist;
}

void
zipf_free(struct probvals *zdist)
{
    free(zdist);
}

void 
get_zipf(float theta, 
         int N,
         struct probvals *zdist)
{
    float sum=0.0;  
    float c=0.0;
    float expo;
    float sumc =0.0;
    int i;
                  
    expo = 1 - theta;
 
    /*
     * zipfian - p(i) = c / i ^^ (1 - theta) At x
     * = 1, uniform * at x = 0, pure zipfian
     */
 
    for (i = 1; i <= N; i++) {
        sum += 1.0 /(float) pow((double) i, (double) (expo));
    }
    c = 1.0 / sum;
	       
    for (i = 0; i < N; i++) {
        zdist[i].prob = c / (float) pow((double) (i + 1), (double) (expo));
		sumc +=  zdist[i].prob;
        zdist[i].cum_prob = sumc;
    }
}
