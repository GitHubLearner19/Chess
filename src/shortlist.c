#include "shortlist.h"
#include <stdlib.h>

// construct shortlist
shortlist* cons(int first, shortlist* rest) {
    shortlist* res = (shortlist*) malloc(sizeof(shortlist));
    res->val = first;
    res->next = rest;
    return res;
}

// get last item of shortlist
shortlist* get_last_item(shortlist* l) {
    if (!l) {
        return NULL;
    }
    shortlist* tail = l;
    while (tail->next) {
        tail = tail->next;
    }
    return tail;
}

// append list to end of another list and update tail
void append_list(shortlist *newList, shortlist** listPtr, shortlist** tailPtr) {
    if (newList) {
        if (*listPtr) {
            (*tailPtr)->next = newList;
            *tailPtr = get_last_item((*tailPtr)->next);
        } else {
            *listPtr = newList;
            *tailPtr = get_last_item(newList);
        }
    }
}
