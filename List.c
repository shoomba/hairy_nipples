/*  double linked list implementation
    group 1, wed 18 tuba.
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "Item.h"
#include "List.h"

typedef struct ListNode {
    Item value;
    struct ListNode *next;
    struct ListNode *prev;
} ListNode;

typedef struct ListRep {
    int nelems;
    ListNode *head;  // ptr to first node
    ListNode *tail;  // ptr to last node
} ListRep;

// create new empty List
List newList()
{
    List l;
    l = malloc(sizeof(ListRep));
    assert(l != NULL);
    l->head = NULL;
    l->tail = NULL;
    l->nelems = 0;
    return l;
}

// free memory used by List
void dropList(List L)
{
    ListNode *curr, *next;
    assert(L != NULL);
    // free list nodes
    curr = L->head;
    while (curr != NULL) {
        next = curr->next;
        free(curr);
        curr = next;
    }
    // free list rep
    free(L);
}

// return an array containing each element in list
Item * ListToArray(List L)
{
    assert(L != NULL);
    Item * a = malloc (sizeof (Item) * L->nelems);
    
    int i = 0;
    ListNode * curr = L->head;
    while (curr != NULL) {
        a[i] = ItemCopy (curr->value); i++;
        curr = curr->next;
    }

    return a;
}

// insert element into front of list
void ListInsertFront(List L, Item it) {
    assert(L != NULL);

    ListNode *new = malloc(sizeof(ListNode));
    assert(new != NULL);

    new->value = ItemCopy(it);
    new->next = NULL;
    new->prev = NULL;

    if (L->nelems == 0) {
        L->tail = L->head = new;
    } else {
        new->next = L->head;
        L->head->prev = new;
        
        L->head = new;
    }

    L->nelems++;
}

// insert element into end of list
void ListInsertEnd(List L, Item it) {
    assert(L != NULL);

    ListNode *new = malloc(sizeof(ListNode));
    assert(new != NULL);

    new->value = ItemCopy(it);
    new->next = NULL;
    new->prev = NULL;

    if (L->nelems == 0) {
        L->tail = L->head = new;
    } else {
        new->prev = L->tail;
        L->tail->next = new;
        L->tail = new;
    }
    L->nelems++;
}

// get element at index pos
Item ListGet(List L, int pos) {
    assert (L != NULL);
    assert (L->head != NULL);
    assert (pos >= 0 && pos < L->nelems);
    
    ListNode * curr = L->head;
    int i;
    for (i = 0; i < pos; i++) {
        curr = curr->next;
    }

    return ItemCopy (curr->value);
}

// remove element from front of list
Item ListRemoveFront(List L) {
    assert(L != NULL);
    assert(L->head != NULL);
    
    L->nelems--;

    Item it = ItemCopy(L->head->value);
    ListNode *old = L->head;

    if (L->nelems == 0) {
        L->tail = L->head = NULL;
    } else {
        L->head = old->next;
        L->head->prev = NULL;
    }

    free(old);
    return it;
}

// remove element from end of list
Item ListRemoveEnd(List L) {
    assert(L != NULL);
    assert(L->head != NULL);

    L->nelems--;

    Item it = ItemCopy(L->tail->value);
    ListNode *old = L->tail;

    if (L->nelems == 0) {
        L->tail = L->head = NULL;
    } else {
        L->tail = old->prev;
        L->tail->next = NULL;
    }
    
    free(old);
    return it;
}

// returns number of elements in list.
int ListSize(List L)
{
    assert(L != NULL);
    return L->nelems;
}

