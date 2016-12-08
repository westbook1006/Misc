/*
 * A simple code piece to generate the zipf distribution
 *
 * Original from Prof. Donald S. Fussell
 */

/*************************************************************************
* This is a simple program to generate a Zipfian probability distribution
* for a set of N  objects. The Zipf distribution is determined by the
* skew parameter theta and the number of objects N.  
* The Zipf distribution has the following probability distribution
* p(i) = c / i ^(1 - theta), where ^ is the exponent operator
* The skew parameter theta ranges from [0.0, 1.0] with a higher skew for 
* smaller values of theta. A theta of 1.0 results in a uniform distribution  
* To run this program compile using "gcc -o zipf zipf.c -lm"
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void get_zipf(float, int);      /* the zipf probability generator function */

struct probvals 
{
    float prob;                  /* the access probability */
    float cum_prob;              /* the cumulative access probability */
};
 
struct probvals *zdist;         /* the probability distribution  */

void
main(int argc, char** argv){
    int   i;
    float theta;                 /* the skew parameter */
    int   N;                     /* the number of objects */
   
    if(argc < 3){
        printf("Usage: zipf [theta] [N]\n");
        exit(0);
    } 
    theta = atof(argv[1]);
    N = atoi(argv[2]);
 
    if(N <= 0 || theta < 0.0 || theta > 1.0){
        printf("Error in parameters \n");
        exit(0);
    }

    zdist = (struct probvals *)malloc(N*sizeof(struct probvals));
   

    get_zipf(theta, N);          /* generate the distribution */

    for(i=0; i < N; i++) {
        printf(" p[%d] = %f cum[%d] = %f \n", i, zdist[i].prob, i, 
                zdist[i].cum_prob);
    }
    
} 


void get_zipf(float theta, int N){
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
     


