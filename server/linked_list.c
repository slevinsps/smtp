#include <stdlib.h> 
#include <stdio.h>

#include "linked_list.h"
#include "error_fail.h"

node* linked_list_add_node( node *head, int data_to_add )
{
    node *new_node = ( node* ) malloc( sizeof( node ) );

    if( new_node == NULL ){
        handle_error("Unable to allocate memory for new fd node");
        return NULL;
    }

    new_node->data = data_to_add;
    new_node->next = NULL;

    // check for first insertion
    if ( head == NULL ) {
        head = new_node;
    } else {
        node *current = head;
        while ( 1 ) {
            if( current->next == NULL ) {
                current->next = new_node;
                break;
            }
            current = current->next;
        };
    }

    return head;
}

node* linked_list_delete_node( node *head, int data_to_delete )
{
    node* temp = head;
    node* prev = NULL;
  
    if (temp != NULL && temp->data == data_to_delete) { 
        head = temp->next;   // Change head 
        free(temp);          // free old head
        return head; 
    } 
 
    while (temp != NULL && temp->data != data_to_delete) { 
        prev = temp; 
        temp = temp->next; 
    } 
  
    if (temp == NULL) { 
        return head;
    } 
  
    if (prev) {
        prev->next = temp->next; 
    }
  
    free(temp); 

    return head;
}