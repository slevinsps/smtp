#include <stdlib.h> 
#include <stdio.h>

#include "linked_list.h"
#include "error_fail.h"

node* linked_list_add_node( node *head, int data_to_add )
{
    node *new_node = ( node* ) malloc( sizeof( node ) );

    if( new_node == NULL ){
        handle_error("Unable to allocate memory for new fd node");
    }

    new_node->data = data_to_add;
    new_node->next = NULL;

    // check for first insertion
    if ( head == NULL ) {
        head = new_node;
        printf( "Node fd added to fd list as head.\n" );
    } else {
        // else loop through the list and find the last
        // node, insert next to it
        node *current = head;
        while ( 1 ) {
            if( current->next == NULL ) {
                current->next = new_node;
                printf( "Node fd added to fd list.\n" );
                break;
            }
            current = current->next;
        };
    }

    return head;
}

node* linked_list_delete_node( node *head, int data_to_delete )
{
    // Store head node 
    node* temp = head, *prev; 
  
    // If head node itself holds the key to be deleted 
    if (temp != NULL && temp->data == data_to_delete) { 
        head = temp->next;   // Change head 
        free(temp);          // free old head
        printf( "Head node is deleted.\n" );
        return head; 
    } 

    // Search for the key to be deleted, keep track of the 
    // previous node as we need to change 'prev->next' 
    while (temp != NULL && temp->data != data_to_delete) { 
        prev = temp; 
        temp = temp->next; 
    } 
  
    // If key was not present in linked list 
    if (temp == NULL) { 
        printf( "Nothing is deleted!!!\n" );
        return head;
    } 
  
    // Unlink the node from linked list 
    if (prev) {
        prev->next = temp->next; 
    }
    
  
    free(temp); 
    printf( "Node is deleted.\n" );

    return head;
}