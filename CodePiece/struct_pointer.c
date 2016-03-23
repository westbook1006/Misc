/*
 * A simple example on collocation
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct _ppointer {
    char *src;
    char *des;
} ppointer;

int main(int argc, char **argv)
{
    ppointer *t;

    t = (ppointer *)malloc(sizeof(ppointer) + 4 * sizeof(char));
    memset(t, 0x00, sizeof(ppointer) + 4 * sizeof(char));

    t->src = (char *)(t + sizeof(ppointer));
    t->des = (char *)(t + sizeof(ppointer) + sizeof(char));

    memcpy(t->src, "ABC\0", 4);
    memcpy(t->des, "abc\0", 4);


    printf("src: %s\n", t->src);
    printf("des: %s\n", t->des);

    return 0;
}
