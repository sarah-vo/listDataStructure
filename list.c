#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <printf.h>
#include "list.h"

static Node nodes[LIST_MAX_NUM_NODES];
static List lists[LIST_MAX_NUM_HEADS];
static int listCounter = 0;
static int nodeCounter = 0;
static bool LIST_INIT = false;
static Node* nextNodePtr = NULL;
static List* nextListPtr = NULL;

enum freeNodeOperation{
    NORMAL_NODE_CASE,
    DELETE_NODE_CASE,
    DELETE_NODE_EXCLUDE_ITEM_CASE,
};

enum freeListOperation{
    NORMAL_LIST_CASE,
    DELETE_LIST_CASE
};

// General Error Handling:
// Client code is assumed never to call these functions with a NULL List pointer, or
// bad List pointer. If it does, any behaviour is permitted (such as crashing).
// HINT: Use assert(pList != NULL); just to add a nice check, but not required.

//specifically for List_free

Node* nextFreeNode(Node* deletedNode, enum freeNodeOperation operation_case){
    Node* freeNode = nextNodePtr;
    if(operation_case == NORMAL_NODE_CASE){
        nextNodePtr = nextNodePtr->node_next;
    }
    if(operation_case == DELETE_NODE_CASE){
        deletedNode->node_prev = NULL;
        deletedNode->node_next = freeNode;
        nextNodePtr = deletedNode;
        deletedNode->item = NULL;
    }
    if(operation_case == DELETE_NODE_EXCLUDE_ITEM_CASE){
        deletedNode->node_prev = NULL;
        deletedNode->node_next = freeNode;
        nextNodePtr = deletedNode;
    }
    return freeNode;
}
/*Handles 2 cases: normal case and delete list case
 * normal case: Increments the free list pointer without deleting anything
 * delete list case: Assign deleted list as the next free list without handling the content deletion
*/
List* nextFreeList(List* deletedList, enum freeListOperation operation_case){
    List* freeList = nextListPtr;
    if(operation_case == NORMAL_LIST_CASE){
        nextListPtr = nextListPtr->next_list;
    }
    if(operation_case == DELETE_NODE_CASE){
        deletedList->first = NULL;
        deletedList->current = NULL;
        deletedList->last = NULL;
        deletedList->bound_status = LIST_EMPTY;
        deletedList->counter = 0;
        deletedList->prev_list = NULL;
        deletedList->next_list = freeList;
        nextListPtr = deletedList;
    }
    return freeList;
}

void* List_remove_exclude_item(List* pList){
    bool no_remove_case = (pList->bound_status != LIST_BOUNDED);
    bool current_pos_first = pList->current == pList->first;
    bool current_pos_last = pList->current == pList->last;
    if(no_remove_case){
        return NULL;
    }
    Node* deletedNode = pList->current;
    void* deletedItem = deletedNode->item;
    if(current_pos_first){
        pList->first = pList->first->node_next;
        pList->current = pList->first;
    }
    else if(current_pos_last){
        List_prev(pList);
        pList->current->node_next = NULL;
        pList->last = pList->current;
    }
    else{
        List_next(pList);
        pList->current->node_prev = deletedNode->node_prev;
    }

    nextFreeNode(deletedNode, DELETE_NODE_EXCLUDE_ITEM_CASE);
    pList->counter--;
    nodeCounter--;
    if(pList->counter == 0){
        pList->bound_status = LIST_EMPTY;
    }

    return deletedItem;
}

void initLists() {
    for(int i = 0; i < LIST_MAX_NUM_HEADS; i++){
        List* list = &lists[i];
        list->first = NULL;
        list->current = NULL;
        list->last = NULL;
        list->counter = 0;
        list->bound_status = LIST_EMPTY;

        //assign prev list
        if(i != 0){
            list->prev_list = &lists[i-1];
        }
        else{
            list->prev_list = NULL;
        }
        //assign next list
        if(i < LIST_MAX_NUM_HEADS-1){
            list->next_list = &lists[i+1];
        }
        else{
            list->next_list = NULL;
        }
    }
    nextListPtr = &lists[0];
}

void initNodes(){
    for(int i = 0; i < LIST_MAX_NUM_NODES; i++){
        Node* tempNode = &nodes[i];
        tempNode->item = NULL;

        //previous node assignment
        if(i == 0){
            tempNode->node_prev = NULL;
        }
        else if (i < LIST_MAX_NUM_NODES){
            tempNode->node_prev = &nodes[i-1];
        }

        //next_list node assignment
        if(i < LIST_MAX_NUM_NODES-1){
            tempNode->node_next = &nodes[i+1];
        }
        else {
            tempNode->node_next = NULL;
        }
    }
    nextNodePtr = &nodes[0];
}

// Makes a new, empty list, and returns its reference on success.
// Returns a NULL pointer on failure.
List* List_create(){
    bool listFull = listCounter == LIST_MAX_NUM_HEADS-1;
    if(listFull){
        return NULL;
    }
    if(!LIST_INIT) {
        initNodes();
        initLists();
    }
    LIST_INIT = true;
    listCounter++;

    return nextFreeList(NULL, NORMAL_LIST_CASE);
}



// Returns the number of items in pList.
int List_count(List* pList){
    return pList->counter;
}

// Returns a pointer to the first item in pList and makes the first item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_first(List* pList){
    bool list_empty = pList->bound_status == LIST_EMPTY;
    if(list_empty){
        return NULL;
    }
    pList->current = pList->first;
    return pList->current->item;
}

// Returns a pointer to the last item in pList and makes the last item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_last(List* pList){
    bool list_empty = pList->bound_status == LIST_EMPTY;
    if(list_empty){
        return NULL;
    }
    pList->current = pList->last;
    return pList->current->item;
}

// Advances pList's current item by one, and returns a pointer to the new current item.
// If this operation advances the current item beyond the end of the pList, a NULL pointer
// is returned and the current item is set to be beyond end of pList.
void* List_next(List* pList){
    pList->current = pList->current->node_next;
    bool oob_end = (pList->current == NULL || pList->current->item == NULL);
    if(oob_end){
        pList->bound_status = LIST_OOB_END;
        return NULL;
    }
    return pList->current->item;
}

// Backs up pList's current item by one, and returns a pointer to the new current item.
// If this operation backs up the current item beyond the start of the pList, a NULL pointer
// is returned and the current item is set to be before the start of pList.
void* List_prev(List* pList){
    pList->current = pList->current->node_prev;
    bool oob_start = ( pList->current == NULL  || pList->current->item == NULL);
    if(oob_start){
        pList->bound_status = LIST_OOB_START;
        return NULL;
    }
    return pList->current->item;
}

// Returns a pointer to the current item in pList.
void* List_curr(List* pList){
    return pList->current->item;
}

// Adds the new item to pList directly after the current item, and makes item the current item.
// If the current pointer is before the start of the pList, the item is added at the start. If
// the current pointer is beyond the end of the pList, the item is added at the end.
// Returns 0 on success, -1 on failure.
int List_insert_after(List* pList, void* pItem){
    bool list_empty = pList->bound_status == LIST_EMPTY;
    bool list_oob_start = pList->bound_status == LIST_OOB_START;
    bool list_oob_end = pList->bound_status == LIST_OOB_END;
    bool list_bounded = pList->bound_status == LIST_BOUNDED;
    bool max_node = nodeCounter == LIST_MAX_NUM_NODES-1;

    if(max_node){
        return LIST_FAIL;
    }


    //if empty assign next free node to current node,
    //then assign item and first-last node to current node
    //then increment next free node
    if(list_empty){
        pList->current = nextFreeNode(NULL, NORMAL_NODE_CASE);
        pList->current->node_prev = pList->current->node_next = NULL;
        pList->current->item = pItem;
        pList->first = pList->last = pList->current;
        pList->bound_status = LIST_BOUNDED;
    }
    if(list_oob_start){
//        Node* newNode = nextFreeNode(NULL, NORMAL_NODE_CASE);
//        newNode->node_prev = NULL;
//        newNode->item = pItem;
//
//        newNode->node_next = pList->first;
//        //assign first node's node_prev to newNode before assigning newNode to first node
//        pList->first->node_prev = newNode;
//        pList->first = newNode;
//        pList->current = pList->first;

        pList->current = nextFreeNode(NULL, NORMAL_NODE_CASE);

        pList->current->item = pItem;
        pList->current->node_prev = NULL;
        pList->current->node_next = pList->first;
        pList->first->node_prev = pList->current;
        pList->first = pList->current;

        pList->bound_status = LIST_BOUNDED;
    }
    if(list_oob_end){
        pList->current = nextFreeNode(NULL, NORMAL_NODE_CASE);

        pList->current->item = pItem;
        pList->current->node_next = NULL;
        pList->current->node_prev = pList->last;
        pList->last = pList->current;

        pList->bound_status = LIST_BOUNDED;
    }
    if(list_bounded){
        bool current_is_last = pList->current == pList->last;
        Node* newNode = nextFreeNode(NULL, NORMAL_NODE_CASE);
        newNode->item = pItem;
        newNode->node_prev = pList->current;
        newNode->node_next = pList->current->node_next;
        pList->current->node_next = newNode;
        pList->current = newNode;

        if(current_is_last){
            pList->last = pList->current;
            pList->current->node_next = NULL;
        }
        pList->bound_status = LIST_BOUNDED;
    }
    pList->counter++;
    nodeCounter++;
    return LIST_SUCCESS;

}

// Adds item to pList directly before the current item, and makes the new item the current one.
// If the current pointer is before the start of the pList, the item is added at the start.
// If the current pointer is beyond the end of the pList, the item is added at the end.
// Returns 0 on success, -1 on failure.
int List_insert_before(List* pList, void* pItem){
    if(pList->bound_status == LIST_BOUNDED) {
        List_prev(pList);
    }
    List_insert_after(pList, pItem);
}

// Adds item to the end of pList, and makes the new item the current one.
// Returns 0 on success, -1 on failure.
int List_append(List* pList, void* pItem){
    if(pList->bound_status == LIST_BOUNDED) {
        List_last(pList);
    }
    List_insert_after(pList, pItem);
}

// Adds item to the front of pList, and makes the new item the current one.
// Returns 0 on success, -1 on failure.
int List_prepend(List* pList, void* pItem){
    List_first(pList);
    List_insert_before(pList, pItem);
}

// Return current item and take it out of pList. Make the next_list item the current one.
// If the current pointer is before the start of the pList, or beyond the end of the pList,
// then do not change the pList and return NULL.
void* List_remove(List* pList){
    bool no_remove_case = (pList->bound_status != LIST_BOUNDED);
    bool current_pos_first = pList->current == pList->first;
    bool current_pos_last = pList->current == pList->last;
    if(no_remove_case){
        return NULL;
    }
    Node* deletedNode = pList->current;
    void* deletedItem = deletedNode->item;
    if(current_pos_first){
        pList->first = pList->first->node_next;
        pList->current = pList->first;
    }
    else if(current_pos_last){
        List_prev(pList);
        pList->current->node_next = NULL;
        pList->last = pList->current;
    }
    else{
        List_next(pList);
        pList->current->node_prev = deletedNode->node_prev;
}
//    nextFreeNode(deletedNode, DELETE_NODE_CASE);
//As clarified by A1 Clarification post, it is not the responsibility for this function
//to free the memory. Therefore, the NORMAL_CASE is used to traverse to next free node
    nextFreeNode(NULL, NORMAL_NODE_CASE);
    pList->counter--;
    nodeCounter--;
    if(pList->counter == 0){
        pList->bound_status = LIST_EMPTY;
    }

    return deletedItem;
}

// Return last item and take it out of pList. Make the new last item the current one.
// Return NULL if pList is initially empty.
void* List_trim(List* pList){
    if(pList->bound_status == LIST_EMPTY){
        return NULL;
    }
    List_last(pList);
    void* returnedItem = List_remove(pList);
    return returnedItem;
}

// Adds pList2 to the end of pList1. The current pointer is set to the current pointer of pList1.
// pList2 no longer exists after the operation; its head is available
// for future operations.
void List_concat(List* pList1, List* pList2){
    pList2->first->node_prev = pList1->last;
    pList1->last->node_next = pList2->first;
    pList1->last = pList2->last;
    pList1->counter += pList2->counter;

    //removing pList2 details without removing nodes
    nextFreeList(pList2, DELETE_LIST_CASE);
}



// Delete pList. pItemFreeFn is a pointer to a routine that frees an item.
// It should be invoked (within List_free) as: (*pItemFreeFn)(itemToBeFreedFromNode);
// pList and all its nodes no longer exists after the operation; its head and nodes are
// available for future operations.
typedef void (*FREE_FN)(void* pItem);
void List_free(List* pList, FREE_FN pItemFreeFn){
    //traverse current to first node of pList
    while(pList->bound_status == LIST_BOUNDED){
        (*pItemFreeFn)(pList->current->item);
        //deletes current item
        List_remove_exclude_item(pList);

    }
    nextFreeList(pList, DELETE_LIST_CASE);
}

// Search pList, starting at the current item, until the end is reached or a match is found.
// In this context, a match is determined by the comparator parameter. This parameter is a
// pointer to a routine that takes as its first argument an item pointer, and as its second
// argument pComparisonArg. Comparator returns 0 if the item and comparisonArg don't match,
// or 1 if they do. Exactly what constitutes a match is up to the implementor of comparator.
//
// If a match is found, the current pointer is left at the matched item and the pointer to
// that item is returned. If no match is found, the current pointer is left beyond the end of
// the list and a NULL pointer is returned.
//
// If the current pointer is before the start of the pList, then start searching from
// the first node in the list (if any).
typedef bool (*COMPARATOR_FN)(void* pItem, void* pComparisonArg);
void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg){


    while(pList->bound_status != LIST_OOB_END){
        bool match = (*pComparator)(pList->current->item, pComparisonArg);
        if(match){
            return pList->current->item;
        }
        else{
            //if we traverse and goes OOB at the end, List_next would mark it
            List_next(pList);
        }
    }
    return NULL;

}


