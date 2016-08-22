/*
 * A lock-free hashtable implementation
 *
 * Reference: High Performance Dynamic Lock-Free Hash Tables and List-Based Sets, SPAA'02
 */
#include <stdio.h>

int 
hashtable_init()
{
    printf("Hashtable init\n");
    return 0;
}

int
hashtable_insert(char *key, char *value)
{
    printf("HT INSERT key: %s value: %s\n", key, value);
    return 0;
}

int 
hashtable_search(char *key)
{
    printf("HT SEARCH key: %s\n", key);
    return 0;
}

int 
hashtable_delete(char *key)
{
    printf("HT DELETE key: %s\n", key);
    return 0;
}
