#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct node node;
struct node {
    int data;
    node *next;
};

node* linked_list_add_node( node *head, int data_to_add );
node* linked_list_delete_node( node *head, int data_to_delete );

#endif