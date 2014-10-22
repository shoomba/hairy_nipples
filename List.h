#ifndef LIST_H
#define LIST_H

#include "Item.h"

typedef struct ListRep *List;

List newList(); // create new empty list
void dropList(List); // free memory used by list
Item *ListToArray(List); // return an array containing each element in list
void ListInsertFront(List L, Item it); // insert element into front of list
void ListInsertEnd(List L, Item it); // insert element into end of list
Item ListGet(List L, int pos); // get element at index pos
Item ListRemoveFront(List L); // remove element from front of list
Item ListRemoveEnd(List L); // remove element from end of list
int ListSize(List); // get number of elements in list

#endif
