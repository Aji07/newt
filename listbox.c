/* This goofed-up box whacked into shape by Elliot Lee <sopwith@cuc.edu> */

#include <slang/slang.h>
#include <stdlib.h>
#include <string.h>

#include "newt.h"
#include "newt_pr.h"


/* Linked list of items in the listbox */
struct items {
    void *key, *data;
    struct items *next;
};

/* Holds all the relevant information for this listbox */
struct listbox {
    newtComponent sb; /* Scrollbar on right side of listbox */
    int numItems, curWidth;
    int currItem, startShowItem; /* startShowItem is the first item displayed
				   on the screen */
    int isActive; /* If we handle key events all the time, it seems
		     to do things even when they are supposed to be for
		     another button/whatever */
    struct items *boxItems;
    int flags; /* flags for this listbox, right now just
		  NEWT_LISTBOX_RETURNEXIT */
};

static void listboxDraw(newtComponent co);
static void listboxDestroy(newtComponent co);
static struct eventResult listboxEvent(newtComponent co, struct event ev);

static struct componentOps listboxOps = {
    listboxDraw,
    listboxEvent,
    listboxDestroy,
};

newtComponent newtListbox(int left, int top, int height, int flags) {
    newtComponent co, sb;
    struct listbox * li;

    if (!(co = malloc(sizeof(*co))))
	return NULL;

    if (!(li = malloc(sizeof(struct listbox)))) {
	free(co);
	return NULL;
    }

    li->boxItems = NULL;
    li->numItems = 0;
    li->currItem = 0;
    li->isActive = 0;
    li->startShowItem = 0;
    li->flags = flags & (NEWT_LISTBOX_RETURNEXIT);

    if (height) 
	sb = newtVerticalScrollbar(left, top, height, COLORSET_LISTBOX,
				   COLORSET_ACTLISTBOX);
    else
	sb = NULL;

    li->sb = sb;
    co->data = li;
    co->left = left;
    co->top = top;
    co->height = height;
    li->curWidth = 5;
    co->ops = &listboxOps;
    co->takesFocus = 1;

    return co;
}

void newtListboxSetCurrent(newtComponent co, int num) {
    struct listbox * li = co->data;
    if (num >= li->numItems)
	li->currItem = li->numItems - 1;
    else if (num < 0)
	li->currItem = 0;
    else
	li->currItem = num;

    if (li->currItem < li->startShowItem)
	li->startShowItem = li->currItem;
    else if (li->currItem - li->startShowItem > co->height - 1)
	li->startShowItem = li->currItem - co->height + 1;
    if (li->startShowItem + co->height > li->numItems)
	li->startShowItem = li->numItems - co->height;

    newtScrollbarSet(li->sb, li->currItem + 1, li->numItems);
    listboxDraw(co);
}

void * newtListboxGetCurrent(newtComponent co) {
    struct listbox * li = co->data;
    int i;
    struct items *item;

    for(i = 0, item = li->boxItems; item != NULL && i < li->currItem;
	i++, item = item->next);

    if (item)
	return item->data;
    else
	return NULL;
}

void newtListboxSetText(newtComponent co, int num, char * text) {
    struct listbox * li = co->data;
    int i;
    struct items *item;

    for(i = 0, item = li->boxItems; item != NULL && i < num;
	i++, item = item->next);

    if(!item)
	return;
    else
	item->key = text;

    if (strlen(text) > li->curWidth) {
	co->width = li->curWidth = strlen(text);
	if (li->sb)
	    li->sb->left = co->left + co->width + 1;
    }

    if (num >= li->startShowItem && num <= li->startShowItem + co->height)
	listboxDraw(co);
}

void newtListboxSetEntry(newtComponent co, int num, char * text) {
    newtListboxSetText(co, num, text);
}

void newtListboxSetData(newtComponent co, int num, void * data) {
    struct listbox * li = co->data;
    int i;
    struct items *item;

    for(i = 0, item = li->boxItems; item != NULL && i < num;
	i++, item = item->next);

    item->data = data;
}

int newtListboxAddEntry(newtComponent co, char * text, void * data) {
    struct listbox * li = co->data;
    struct items *item;

    if(li->boxItems) {
	for (item = li->boxItems; item->next != NULL; item = item->next);

	item = item->next = malloc(sizeof(struct items));
    } else {
	item = li->boxItems = malloc(sizeof(struct items));
    }

    if (text && (strlen(text) > li->curWidth))
	li->curWidth = strlen(text);

    item->key = text; item->data = data; item->next = NULL;

    if (li->sb)
	li->sb->left = co->left + li->curWidth + 1;

    co->width = li->curWidth;
    li->numItems++;
    listboxDraw(co);

    return li->numItems;
}


int newtListboxInsertEntry(newtComponent co, char * text, void * data, 
			   int num) {
    struct listbox * li = co->data;
    struct items *item, *t;
    int i;

    if (li->boxItems) {
	for(i = 0, item = li->boxItems; item->next != NULL && i < num;
	    item = item->next, i++);

	t = item->next;
	item = item->next = malloc(sizeof(struct items));
	item->next = t;
    } else {
	item = li->boxItems = malloc(sizeof(struct items));
	item->next = NULL;
    }

    if (text && (strlen(text) > li->curWidth))
	li->curWidth = strlen(text);

    item->key = text; item->data = data;

    if (li->sb)
	li->sb->left = co->left + li->curWidth + 1;

    co->width = li->curWidth;
    li->numItems++;
    listboxDraw(co);

    return li->numItems;
}

int newtListboxDeleteEntry(newtComponent co, int num) {
    struct listbox * li = co->data;
    int i, widest = 0, t;
    struct items *item, *item2;

    if (!li->boxItems)
	return -1;

    if (num <= 0) { 
	item = li->boxItems;
	li->boxItems = item->next;

	/* Fix things up for the width-finding loop near the bottom */
	item2 = li->boxItems;
	widest = strlen(item2->key);
    } else {
	for(i = 0, item = li->boxItems; item != NULL && i != li->currItem;
	    i++, item = item->next) {
	    if((t = strlen(item->key)) > widest) widest = t;
	    item2 = item;
	}

	if (!item)
	    return -1;

	item2->next = item->next;
    }

    free(item);
    li->numItems--;
    
    for (item = item2->next; item != NULL; item = item->next)
	if((t = strlen(item->key)) > widest) widest = t;

    /* Adjust the listbox width */
    co->width = li->curWidth = widest;
    if (li->sb)
	li->sb->left = co->left + widest + 1;

    listboxDraw(co);

    return li->numItems;
}

/* If you don't want to get back the text, pass in NULL for the ptr-ptr. Same
   goes for the data. */
void newtListboxGetEntry(newtComponent co, int num, char **text, void **data) {
    struct listbox * li = co->data;
    int i;
    struct items *item;

    if (!li->boxItems || num >= li->numItems) {
	if(text)
	    *text = NULL;
	if(data)
	    *data = NULL;
	return;
    }

    i = 0;
    item = li->boxItems; 
    while (item && i < num) {
	i++, item = item->next;
    }

    if (item) {
	if (text)
	    *text = item->key;
	if (data)
	    *data = item->data; 
    }
}

static void listboxDraw(newtComponent co)
{
    struct listbox * li = co->data;
    struct items *item;
    int i, j;

    SLsmg_set_color(NEWT_COLORSET_LISTBOX);

    if(li->sb)
	li->sb->ops->draw(li->sb);

    for(i = 0, item = li->boxItems; item != NULL && i < li->startShowItem;
	i++, item = item->next);

    j = i;
    newtGotorc(co->top - 1, co->left);

    for (i = 0; item != NULL && i < co->height; i++, item = item->next) {
	if (!item->key) continue;

	newtGotorc(co->top + i, co->left + 1);
	if(j + i == li->currItem)
	    SLsmg_set_color(NEWT_COLORSET_ACTLISTBOX);

	SLsmg_write_nstring(item->key, li->curWidth);

	if(j + i == li->currItem)
	    SLsmg_set_color(NEWT_COLORSET_LISTBOX);
    }
}

static struct eventResult listboxEvent(newtComponent co, struct event ev) {
    struct eventResult er;
    struct listbox * li = co->data;

    er.result = ER_IGNORED;
	       
    switch(ev.event) {
      case EV_KEYPRESS:
	if (!li->isActive) break;

	switch(ev.u.key) {
	  case NEWT_KEY_ENTER:
	    if(li-> flags & NEWT_LISTBOX_RETURNEXIT)
		er.result = ER_EXITFORM;
	    break;

	  case NEWT_KEY_UP:
	    if(li->currItem > 0) {
		li->currItem--;
		if(li->currItem < li->startShowItem)
		    li->startShowItem = li->currItem;
		listboxDraw(co);
	    }

	    newtGotorc(co->top + li->currItem - li->startShowItem,
		       co->left);
	    newtScrollbarSet(li->sb, li->currItem + 1, li->numItems);
	    er.result = ER_SWALLOWED;
	    break;

	  case NEWT_KEY_DOWN:
	    if(li->currItem < li->numItems - 1) {
		li->currItem++;
		if(li->currItem > (li->startShowItem + co->height - 1)) {
		    li->startShowItem = li->currItem - co->height + 1;
		    if(li->startShowItem + co->height > li->numItems)
			li->startShowItem = li->numItems - co->height;
		}
		listboxDraw(co);
	    }

	    newtGotorc(co->top + li->currItem - li->startShowItem,
		       co->left);
	    newtScrollbarSet(li->sb, li->currItem + 1, li->numItems);
	    er.result = ER_SWALLOWED;
	    break;

	  case NEWT_KEY_PGUP:
	    newtListboxSetCurrent(co, li->currItem - co->height + 1);
	    er.result = ER_SWALLOWED;
	    break;

	  case NEWT_KEY_PGDN:
	    newtListboxSetCurrent(co, li->currItem + co->height - 1);
	    er.result = ER_SWALLOWED;
	    break;

	  default:
	    /* keeps gcc quiet */
	}
	break;
	
      case EV_FOCUS:
	newtGotorc(co->top + li->currItem - li->startShowItem, co->left);
	li->isActive = 1;
	er.result = ER_SWALLOWED;
	break;

      case EV_UNFOCUS:
	li->isActive = 0;
	er.result = ER_SWALLOWED;
	break;
    }

    return er;
}

static void listboxDestroy(newtComponent co) {
    struct listbox * li = co->data;
    struct items * item, * nextitem;

    nextitem = item = li->boxItems;

    while (item != NULL) {
	nextitem = item->next;
	free(item);
	item = nextitem;
    }

    free(li);
    free(co);
}

