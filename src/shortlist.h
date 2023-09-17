#ifndef SHORTLIST
#define SHORTLIST

typedef struct shortlist shortlist;

struct shortlist {
    int val;
    shortlist* next;
};

// construct shortlist
shortlist* cons(int first, shortlist* rest);

// get item at index
shortlist* get_item(shortlist* l, int i);

// get last item of shortlist
shortlist* get_last_item(shortlist* l);

// get length of list
int get_list_length(shortlist* l);

// append list to end of another list and update tail
void append_list(shortlist* newList, shortlist** listPtr, shortlist** tailPtr);

// free list and convert to array
short* list_to_arr(shortlist* l, int* lenPtr);

#endif
