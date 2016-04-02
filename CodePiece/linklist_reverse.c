/*
 * Linklist reverse.
 * Emina uses this example to start her discussion on software reasoning.
 */

#include <stdio.h>
#include <stdlib.h>

#define NODE_NUM 64

typedef struct _node{
    int data;
    struct _node* next;
}node, *node_ptr;

void build_list(node_ptr *list)
{
    node_ptr head, tmp, p;

    head = tmp = p = NULL;
    for (int i = 0; i < NODE_NUM; i++) {
        tmp = (node_ptr)malloc(sizeof(node));
        tmp->data = i;
        tmp->next = NULL;

        if (!head) {
            head = tmp;
            p = head;
        } else {
            p->next = tmp;
            p = tmp;
        }
    }

    *list = head;
}

void print_list(node_ptr list)
{
    while(list) {
        printf("Data: %d\n", list->data);
        list = list->next;
    }
}

void reverse_list(node_ptr *list)
{
    node_ptr head, begin, mid, end;

    head = *list;

    if (head != NULL)
        begin = head;
    else
        return;

    if (head->next != NULL)
        mid = head->next;
    else
        return;

    end = mid->next;

    while (end) {
        mid->next = begin;

        begin = mid;
        mid = end;
        end = end->next;
    }

    mid->next = begin;
    head->next = NULL;
    head = mid;

    *list = head;
}

void free_list(node_ptr list)
{
    node_ptr p;

    while(list) {
       p = list;
       list = list->next;

       free(p);
    }
}

int main(int argc, char **argv)
{
    node_ptr list = NULL;

    build_list(&list);
    printf("Original linklist:\n");
    print_list(list);

    reverse_list(&list);
    printf("\nReverse linklist:\n");
    print_list(list);

    free_list(list);

    return 0;
}
