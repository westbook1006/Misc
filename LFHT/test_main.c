/*
 * Test program
 */
#include <stdio.h>

#include "hashtable.h"

int main(int argc, char **argv)
{
    hashtable_init();
    hashtable_insert();
    hashtable_search();
    hashtable_delete();

    printf("Hello world\n");
    return 0;
}
