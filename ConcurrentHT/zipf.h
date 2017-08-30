/*
 * A simple code piece to generate the zipf distribution
 *
 * Original from Prof. Donald S. Fussell
 */
#ifndef _ZIPF_H
#define _ZIPF_H

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
#include <math.h>

struct probvals 
{
    float prob;                  /* the access probability */
    float cum_prob;              /* the cumulative access probability */
};

/*
 * @brief: Init zipf distribution array
 * @N: the number of elements
 */
struct probvals* zipf_init(int N);

/*
 * @brief: Free the zipf distribution array
 */
void zipf_free();

/*
 * @brief: Calcuate the zipf distribution array
 * @theta: Zipf distribution skewness
 * @N: the number of elements
 * @zdist: hte zipf distribution array
 */
void get_zipf(float theta, 
              int N,
              struct probvals *zdist);

#endif /* _ZIPF_H */
