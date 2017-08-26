/*
 * Test example about concurrent hashtable.
 */

#include <stdio.h>

#include "concurrent_ht.h"

int main (int argc, char **argv)
{
    printf("Hello world!\n");

    concurrent_ht_t *ht = concur_hashtable_init(0);
    concur_hashtable_free(ht);

    return 0;
}
