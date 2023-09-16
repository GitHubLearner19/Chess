#ifndef SHORTLIST
#define SHORTLIST

typedef struct shortlist shortlist;

struct shortlist {
    int val;
    shortlist* next;
};

// construct shortlist
shortlist* cons(int first, shortlist* rest);

// get last item of shortlist
shortlist* get_last_item(shortlist* l);

// append list to end of another list and update tail
void append_list(shortlist *newList, shortlist** listPtr, shortlist** tailPtr);

#endif
