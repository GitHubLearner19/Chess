#include "shortlist.h"
#include <stdlib.h>

// construct shortlist
shortlist* cons(int first, shortlist* rest) {
    shortlist* res = (shortlist*) malloc(sizeof(shortlist));
    res->val = first;
    res->next = rest;
    return res;
}

// get item at index
shortlist* get_item(shortlist* l, int i) {
    if (!l) {
        return NULL;
    }
    int count = 0;
    shortlist* tail = l;
    while (tail->next) {
        if (i == count) {
            return tail;
        }
        count ++;
        tail = tail->next;
    }
    return NULL;
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
void append_list(shortlist* newList, shortlist** listPtr, shortlist** tailPtr) {
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

// get length of list
int get_list_length(shortlist* l) {
    if (!l) {
        return 0;
    }
    int i = 1;
    shortlist* tail = l;
    while (tail->next) {
        tail = tail->next;
        i ++;
    }
    return i;
}

// free list and convert to array
short* list_to_arr(shortlist* l, int* lenPtr) {
    if (!l) {
        *lenPtr = 0;
        return NULL;
    }
    *lenPtr = get_list_length(l);
    short* result = (short*) malloc(sizeof(short) * *lenPtr);
    int i = 0;
    shortlist* tail = l;
    while (tail) {
        result[i] = tail->val;
        i ++;
        shortlist* temp = tail;
        tail = tail->next;
        free(temp);
    }
    return result;
}
