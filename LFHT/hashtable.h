/*
 * Hashtable headers
 */
#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

#include <stdint.h>

int hashtable_init();
int hashtable_insert(char *key, char *value);
int hashtable_search(char *key);
int hashtable_delete(char *key);
int hashtable_dump();

#endif
