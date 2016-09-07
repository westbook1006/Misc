/*
 * Hashtable headers
 */
#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

#include <stdint.h>
#include "util.h"

int hashtable_init();
int hashtable_insert(char *key, char *value); // Thread-safe
node* hashtable_find(char *key); // Thread-safe
int hashtable_find_end(node *node); // Thread-safe
int hashtable_delete(char *key); // Thread-safe
int hashtable_dump();

#endif
