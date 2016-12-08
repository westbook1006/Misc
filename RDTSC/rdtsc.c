/*
 * RDTSC instruciton test. Two aspects:
 *   - Basic functionalities
 *   - Invariant TSC (frequency has nearly no impact)
 */

#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


static uint64_t rdtsc(void)
{
    uint32_t eax, edx;
    __asm volatile("rdtsc" : "=a" (eax), "=d" (edx));
    return ((uint64_t)edx << 32) | eax;
}

#define BUF_LEN 4096 // 1 Page

int main(int argc, char **argv)
{
    uint64_t begin, end;
    char *str;
    cpu_set_t affinity;

    // Pin to the 1st core
    CPU_ZERO(&affinity);
    CPU_SET(0, &affinity);
    int ret = sched_setaffinity(getpid(), sizeof(affinity), &affinity);
    assert(ret != -1);

    str = (char *)malloc(sizeof(char) * BUF_LEN);

    begin = rdtsc();
    memset(str, 0x41, BUF_LEN);
    end = rdtsc();
    
    printf("Buffer set time %ld\n", end - begin);
    free(str);
    return 0;
}
