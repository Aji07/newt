/* This goofed-up box whacked into shape by Elliot Lee <sopwith@cuc.edu>
   (from the original listbox by Erik Troan <ewt@redhat.com>)
   and contributed to newt for use under the LGPL license.
   Copyright (C) 1996, 1997 Elliot Lee */

#include <slang/slang.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "newt.h"
#include "newt_pr.h"


/* Linked list of items in the listbox */
struct items {
    void *key;
    const void *data;
    unsigned char isSelected;
    struct items *next;
};

/* Holds all the relevant information for this listbox */
struct listbox {
    newtComponent sb;   /* Scrollbar on right side of listbox */
    int curWidth;	/* size of text w/o scrollbar or border*/
    int curHeight;	/* size of text w/o border */
    int sbAdjust;
    int bdxAdjust, bdyAdjust;
    int numItems, numSelected; 
    int userHasSetWidth;
    int currItem, startShowItem; /* startShowItem is the first item displayed
				   on the screen */
    int isActive; /* If we handle key events all the time, it seems
		     to do things even when they are supposed to be for
		     another button/whatever */
    struct items *boxItems;
    int grow;
    int flags; /* flags for this listbox, right now just
		  NEWT_FLAG_RETURNEXIT */
};

static void listboxDraw(newtComponent co);
static void listboxDestroy(newtComponent co);
static struct eventResult listboxEvent(newtComponent co, struct event ev);
static void newtListboxRealSetCurrent(newtComponent co);
static void listboxPlace(newtComponent co);
static inline void updateWidth(newtComponent co, struct listbox * li, 
				int maxField);

static struct componentOps listboxOps = {
    listboxDraw,
    listboxEvent,
    listboxDestroy,
    listboxPlace,
};

static void listboxPlace(newtComponent co) {
    struct listbox * li = co->data;

    if (li->sb) {
	li->sb->top = co->top;
	li->sb->left = co->left + co->width - 1;
	if (li->sb->ops->place) li->sb->ops->place(li->sb);
    }
}

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
    li->numSelected = 0;
    li->isActive = 0;
    li->userHasSetWidth = 0;
    li->startShowItem = 0;
    li->sbAdjust = 0;
    li->bdxAdjust = 0;
    li->bdyAdjust = 0;
    li->flags = flags & (NEWT_FLAG_RETURNEXIT | NEWT_FLAG_DOBORDER | 
			 NEWT_FLAG_MULTIPLE);

    if (li->flags & NEWT_FLAG_DOBORDER) {
	li->bdxAdjust = 2;
	li->bdyAdjust = 1;
    }

    if (height) {
	li->grow = 0;
	if (flags & NEWT_FLAG_NOSCROLL) {
	    sb = NULL;
	} else {
	    sb = newtVerticalScrollbar(left, top, height, COLORSET_LISTBOX,
				       COLORSET_ACTLISTBOX);
	    li->sbAdjust = 3;
	}
	co->height = height;
    } else {
	li->grow = 1;
	sb = NULL;
	co->height = 2 * li->bdyAdjust;
    }

    li->sb = sb;
    co->data = li;
    co->isMapped = 0;
    co->left = left;
    co->top = top;
    co->ops = &listboxOps;
    co->takesFocus = 1;
    co->callback = NULL;

    updateWidth(co, li, 5);
    li->curHeight = co->height - (2 * li->bdyAdjust);

    return co;
}

static inline void updateWidth(newtComponent co, struct listbox * li, 
				int maxField) {
    li->curWidth = maxField;
    co->width = li->curWidth + li->sbAdjust + 2 * li->bdxAdjust;

    if (li->sb)
	li->sb->left = co->left + co->width - 1;
}

void newtListboxSetCurrent(newtComponent co, int num)
{
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
    if(li->startShowItem < 0)
	li->startShowItem = 0;
    newtListboxRealSetCurrent(co);
}

static void
newtListboxRealSetCurrent(newtComponent co)
{
    struct listbox * li = co->data;
    if(li->sb)
	newtScrollbarSet(li->sb, li->currItem + 1, li->numItems);
    listboxDraw(co);
    if(co->callback) co->callback(co, co->callbackData);
}

void newtListboxSetWidth(newtComponent co, int width) {
    struct listbox * li = co->data;
    
    co->width = width;
    li->curWidth = co->width - li->sbAdjust - 2 * li->bdxAdjust;
    li->userHasSetWidth = 1;
    li->sb->left = co->width + co->left - 1;
    listboxDraw(co);
}

void * newtListboxGetCurrent(newtComponent co) {
    struct listbox * li = co->data;
    int i;
    struct items *item;

    for(i = 0, item = li->boxItems; item != NULL && i < li->currItem;
	i++, item = item->next);

    if (item)
	return (void *)item->data;
    else
	return NULL;
}

void newtListboxSelectItem(newtComponent co, int item,
	enum newtFlagsSense sense)
{
    struct listbox * li = co->data;
    int i;
    struct items *iitem;
    
    for(i = 0, iitem = li->boxItems; iitem != NULL && i < item;
	i++, iitem = iitem->next);

    if (iitem) {
	if(iitem->isSelected && sense != NEWT_FLAGS_SET)
	    li->numSelected--;
	else if(!iitem->isSelected && sense != NEWT_FLAGS_RESET)
	    li->numSelected++;
	switch(sense) {
		case NEWT_FLAGS_RESET:
			iitem->isSelected = 0; break;
		case NEWT_FLAGS_SET:
			iitem->isSelected = 1; break;
		case NEWT_FLAGS_TOGGLE:
			iitem->isSelected = !iitem->isSelected;
	}
    }
    listboxDraw(co);
}

void newtListboxClearSelection(newtComponent co)
{
    struct items *item;
    struct listbox * li = co->data;

    for(item = li->boxItems; item != NULL;
	item = item->next)
	item->isSelected = 0;
    li->numSelected = 0;
    listboxDraw(co);
}

/* Free the returned array after use, but NOT the values in the array */
void ** newtListboxGetSelection(newtComponent co, int *numitems)
{
    struct listbox * li;
    int i;
    void **retval;
    struct items *item;

    if(!co || !numitems) return NULL;

    li = co->data;
    if(!li || !li->numSelected) return NULL;

    retval = malloc(li->numSelected * sizeof(void *));
    for(i = 0, item = li->boxItems; item != NULL;
	item = item->next)
	if(item->isSelected)
	    retval[i++] = (void *)item->data;
    *numitems = li->numSelected;
    return retval;
}

void newtListboxSetText(newtComponent co, int num, const char * text) {
    struct listbox * li = co->data;
    int i;
    struct items *item;

    for(i = 0, item = li->boxItems; item != NULL && i < num;
	i++, item = item->next);

    if(!item)
	return;
    else {
	free(item->key);
	item->key = strdup(text);
    }
    if (li->userHasSetWidth == 0 && strlen(text) > li->curWidth) {
	updateWidth(co, li, strlen(text));
    }

    if (num >= li->startShowItem && num <= li->startShowItem + co->height)
	listboxDraw(co);
}

void newtListboxSetEntry(newtComponent co, int num, const char * text) {
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

int newtListboxAddEntry(newtComponent co, const char * text,
	const void * data) {
    struct listbox * li = co->data;
    struct items *item;

    if(li->boxItems) {
	for (item = li->boxItems; item->next != NULL; item = item->next);

	item = item->next = malloc(sizeof(struct items));
    } else {
	item = li->boxItems = malloc(sizeof(struct items));
    }

    if (!li->userHasSetWidth && text && (strlen(text) > li->curWidth))
	updateWidth(co, li, strlen(text));

    item->key = strdup(text); item->data = data; item->next = NULL;
    item->isSelected = 0;
    
    if (li->grow)
	co->height++, li->curHeight++;
    li->numItems++;

    return li->numItems;
}


int newtListboxInsertEntry(newtComponent co, const char * text,
	const void * data, int num) {
    struct listbox * li = co->data;
    struct items *item, *t;
    int i;
    if(num > li->numItems)
	num = li->numItems;

    if (li->boxItems) {
	if(num > 0) {
	    for(i = 0, item = li->boxItems; item->next != NULL && i < num - 1;
		item = item->next, i++);
	    t = item->next;
	    item = item->next = malloc(sizeof(struct items));
	    item->next = t;
	} else {
	    t = li->boxItems;
	    item = li->boxItems = malloc(sizeof(struct items));
	    item->next = t;
	}
    } else {
	item = li->boxItems = malloc(sizeof(struct items));
	item->next = NULL;
    }

    if (!li->userHasSetWidth && text && (strlen(text) > li->curWidth))
	updateWidth(co, li, strlen(text));

    item->key = strdup(text?text:"(null)"); item->data = data;
    item->isSelected = 0;
    
    if (li->sb)
	li->sb->left = co->left + co->width - 1;
    li->numItems++;

    listboxDraw(co);

    return li->numItems;
}

int newtListboxDeleteEntry(newtComponent co, int num) {
    struct listbox * li = co->data;
    int i, widest = 0, t;
    struct items *item, *item2 = NULL;

    if(num > li->numItems)
	num = li->numItems;

    if (li->boxItems == NULL || li->numItems <= 0)
	return 0;

    if (num <= 1) { 
	item = li->boxItems;
	li->boxItems = item->next;

	/* Fix things up for the width-finding loop near the bottom */
	item2 = li->boxItems;
	widest = strlen(item2?item2->key:"");
    } else {
	for(i = 0, item = li->boxItems; item != NULL && i < num - 1;
	    i++, item = item->next) {
	    if((t = strlen(item->key)) > widest) widest = t;
	    item2 = item;
	}

	if (!item)
	    return -1;

	item2->next = item->next;
    }
    free(item->key);
    free(item);
    li->numItems--;
    if(li->currItem >= num)
	li->currItem--;
    for (item = item2?item2->next:item2; item != NULL; item = item->next)
	if((t = strlen(item->key)) > widest) widest = t;

    /* Adjust the listbox width */
    if (!li->userHasSetWidth) {
	updateWidth(co, li, widest);
    }

    listboxDraw(co);

    return li->numItems;
}

void newtListboxClear(newtComponent co)
{
    struct listbox * li;
    struct items *anitem, *nextitem;
    if(co == NULL || (li = co->data) == NULL)
	return;
    for(anitem = li->boxItems; anitem != NULL; anitem = nextitem) {
	nextitem = anitem->next;
	free(anitem->key);
	free(anitem);
    }
    li->numItems = li->numSelected = li->currItem = li->startShowItem = 0;
    li->boxItems = NULL;
    if (!li->userHasSetWidth) 
	updateWidth(co, li, 5);
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
	    *data = (void *)item->data; 
    }
}

static void listboxDraw(newtComponent co)
{
    struct listbox * li = co->data;
    struct items *item;
    int i, j;

    if (!co->isMapped) return ;

    if(li->sb)
	li->sb->ops->draw(li->sb);

    if(li->flags & NEWT_FLAG_DOBORDER) {
      if(li->isActive)
	  SLsmg_set_color(NEWT_COLORSET_ACTLISTBOX);
      else
          SLsmg_set_color(NEWT_COLORSET_LISTBOX);

      newtDrawBox(co->left, co->top, co->width, co->height, 0);
    }

    SLsmg_set_color(NEWT_COLORSET_LISTBOX);
    
    for(i = 0, item = li->boxItems; item != NULL && i < li->startShowItem;
	i++, item = item->next);

    j = i;

    for (i = 0; item != NULL && i < li->curHeight; i++, item = item->next) {
	if (!item->key) continue;

	newtGotorc(co->top + i + li->bdyAdjust, co->left + li->bdxAdjust);
	if(j + i == li->currItem) {
	    if(item->isSelected)
		SLsmg_set_color(NEWT_COLORSET_ACTSELLISTBOX);
	    else
		SLsmg_set_color(NEWT_COLORSET_ACTLISTBOX);
	} else if(item->isSelected)
	    SLsmg_set_color(NEWT_COLORSET_SELLISTBOX);
	else
	    SLsmg_set_color(NEWT_COLORSET_LISTBOX);
	    
	SLsmg_write_nstring(item->key, li->curWidth);

    }
    newtGotorc(co->top + (li->currItem - li->startShowItem), co->left);
}

static struct eventResult listboxEvent(newtComponent co, struct event ev) {
    struct eventResult er;
    struct listbox * li = co->data;

    er.result = ER_IGNORED;
	       
    if(ev.when == EV_EARLY || ev.when == EV_LATE) {
	return er;
    }
		       
    switch(ev.event) {
      case EV_KEYPRESS:
	if (!li->isActive) break;

	switch(ev.u.key) {
	  case ' ':
	    if(!(li->flags & NEWT_FLAG_MULTIPLE)) break;
	    newtListboxSelectItem(co, li->currItem, NEWT_FLAGS_TOGGLE);
	    er.result = ER_SWALLOWED;
	    /* We don't break here, because it is cool to be able to
	       hold space to select a bunch of items in a list at once */

	  case NEWT_KEY_DOWN:
	    if(li->numItems <= 0) break;
	    if(li->currItem < li->numItems - 1) {
		li->currItem++;
		if(li->currItem > (li->startShowItem + li->curHeight - 1)) {
		    li->startShowItem = li->currItem - li->curHeight + 1;
		    if(li->startShowItem + li->curHeight > li->numItems)
			li->startShowItem = li->numItems - li->curHeight;
		}
		if(li->sb)
		    newtScrollbarSet(li->sb, li->currItem + 1, li->numItems);
		listboxDraw(co);
	    }
	    if(co->callback) co->callback(co, co->callbackData);
	    er.result = ER_SWALLOWED;
	    break;

	  case NEWT_KEY_ENTER:
	    if(li->numItems <= 0) break;
	    if(li->flags & NEWT_FLAG_RETURNEXIT)
		er.result = ER_EXITFORM;
	    break;

	  case NEWT_KEY_UP:
	    if(li->numItems <= 0) break;
	    if(li->currItem > 0) {
		li->currItem--;
		if(li->currItem < li->startShowItem)
		    li->startShowItem = li->currItem;
		if(li->sb)
		    newtScrollbarSet(li->sb, li->currItem + 1, li->numItems);
		listboxDraw(co);
	    }
	    if(co->callback) co->callback(co, co->callbackData);
	    er.result = ER_SWALLOWED;
	    break;

	  case NEWT_KEY_PGUP:
	    if(li->numItems <= 0) break;
	    li->startShowItem -= li->curHeight - 1;
	    if(li->startShowItem < 0)
		li->startShowItem = 0;
	    li->currItem -= li->curHeight - 1;
	    if(li->currItem < 0)
		li->currItem = 0;
	    newtListboxRealSetCurrent(co);
	    er.result = ER_SWALLOWED;
	    break;

	  case NEWT_KEY_PGDN:
	    if(li->numItems <= 0) break;
	    li->startShowItem += li->curHeight;
	    if(li->startShowItem > (li->numItems - li->curHeight)) {
		li->startShowItem = li->numItems - li->curHeight;
	    }
	    li->currItem += li->curHeight;
	    if(li->currItem > li->numItems) {
		li->currItem = li->numItems - 1;
	    }
	    newtListboxRealSetCurrent(co);
	    er.result = ER_SWALLOWED;
	    break;

	  case NEWT_KEY_HOME:
	    if(li->numItems <= 0) break;
	    newtListboxSetCurrent(co, 0);
	    er.result = ER_SWALLOWED;
	    break;

	  case NEWT_KEY_END:
	    if(li->numItems <= 0) break;
	    newtListboxSetCurrent(co, li->numItems - 1);
	    er.result = ER_SWALLOWED;
	    break;
	  default:
	    /* keeps gcc quiet */
	}
	break;
	
      case EV_FOCUS:
	li->isActive = 1;
	listboxDraw(co);
	er.result = ER_SWALLOWED;
	break;

      case EV_UNFOCUS:
	li->isActive = 0;
	listboxDraw(co);
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
	free(item->key);
	free(item);
	item = nextitem;
    }

    free(li);
    free(co);
}

