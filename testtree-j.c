#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "newt.h"

int main(void) {
    newtGrid grid;
    newtComponent checktree;
    newtComponent button;
    newtComponent form;
    newtComponent answer;
    void ** result, **ptr;
    int numselected, i, j;
    int * list;
    
    newtInit();
    newtCls();

    checktree = newtCheckboxTreeMulti(-1, -1, 10, " ab", NEWT_FLAG_SCROLL);
    newtCheckboxTreeAddItem(checktree, "�ʥ�С�", (void *) 2, 0,
    			    NEWT_ARG_APPEND, NEWT_ARG_LAST);
    newtCheckboxTreeAddItem(checktree, "������������Ĺ�����",
			   (void *) 3, 0, NEWT_ARG_APPEND, NEWT_ARG_LAST);
    newtCheckboxTreeAddItem(checktree, "�ʥ�С���", (void *) 5, 
    			    NEWT_FLAG_SELECTED, 
    			    NEWT_ARG_APPEND, NEWT_ARG_LAST);
    newtCheckboxTreeAddItem(checktree, "�ʥ�С���", (void *) 6, 0, 
    			    NEWT_ARG_APPEND, NEWT_ARG_LAST);
    newtCheckboxTreeAddItem(checktree, "�ʥ�С���", (void *) 7, 
    			    NEWT_FLAG_SELECTED, 
    			    NEWT_ARG_APPEND, NEWT_ARG_LAST);
    newtCheckboxTreeAddItem(checktree, "�ʥ�С���", (void *) 8, 0, 
    			    NEWT_ARG_APPEND, NEWT_ARG_LAST);
    newtCheckboxTreeAddItem(checktree, "�ʥ�С���", (void *) 9, 0, 
    			    NEWT_ARG_APPEND, NEWT_ARG_LAST);
    newtCheckboxTreeAddItem(checktree, "�ʥ�С�����", (void *) 10,
    			    NEWT_FLAG_SELECTED,
    			    NEWT_ARG_APPEND, NEWT_ARG_LAST);
    newtCheckboxTreeAddItem(checktree, "�ʥ�С�����", (void *) 11, 0, 
    			    NEWT_ARG_APPEND, NEWT_ARG_LAST);
    newtCheckboxTreeAddItem(checktree, "�ʥ�С�����", (void *) 12,
    			    NEWT_FLAG_SELECTED,
    			    NEWT_ARG_APPEND, NEWT_ARG_LAST);

    newtCheckboxTreeAddItem(checktree, "���顼", (void *) 1, 0,
    			    0, NEWT_ARG_LAST);
    newtCheckboxTreeAddItem(checktree, "�ֿ�", (void *) 100, 0,
    			    0, NEWT_ARG_APPEND, NEWT_ARG_LAST);
    newtCheckboxTreeAddItem(checktree, "��", (void *) 101, 0,
    			    0, NEWT_ARG_APPEND, NEWT_ARG_LAST);
    newtCheckboxTreeAddItem(checktree, "�Ŀ�", (void *) 102, 0,
    			    0, NEWT_ARG_APPEND, NEWT_ARG_LAST);

    newtCheckboxTreeAddItem(checktree, "�ʥ�С���", (void *) 4, 0,
    			    3, NEWT_ARG_LAST);

    newtCheckboxTreeAddItem(checktree, "���ο���", (void *) 200, 0,
    			    1, NEWT_ARG_APPEND, NEWT_ARG_LAST);
    newtCheckboxTreeAddItem(checktree, "��", (void *) 201, 0,
    			    1, 0, NEWT_ARG_APPEND, NEWT_ARG_LAST);
    newtCheckboxTreeAddItem(checktree, "��", (void *) 202, 0,
    			    1, 0, NEWT_ARG_APPEND, NEWT_ARG_LAST);
    newtCheckboxTreeAddItem(checktree, "��", (void *) 203, 0,
    			    1, 0, NEWT_ARG_APPEND, NEWT_ARG_LAST);
    newtCheckboxTreeAddItem(checktree, "��", (void *) 204, 0,
    			    1, 0, NEWT_ARG_APPEND, NEWT_ARG_LAST);

    newtCheckboxTreeAddItem(checktree, "���ο���", (void *) 300, 0,
    			    1, NEWT_ARG_APPEND, NEWT_ARG_LAST);
    newtCheckboxTreeAddItem(checktree, "��", (void *) 210, 0,
    			    1, 1, NEWT_ARG_APPEND, NEWT_ARG_LAST);
    newtCheckboxTreeAddItem(checktree, "����", (void *) 211, 0,
    			    1, 1, NEWT_ARG_APPEND, NEWT_ARG_LAST);
    newtCheckboxTreeAddItem(checktree, "����", (void *) 212, 0,
    			    1, 1, NEWT_ARG_APPEND, NEWT_ARG_LAST);
    newtCheckboxTreeAddItem(checktree, "����", (void *) 213, 0,
    			    1, 1, NEWT_ARG_APPEND, NEWT_ARG_LAST);

    button = newtButton(-1, -1, "��λ");
    
    grid = newtCreateGrid(1, 2);
    newtGridSetField(grid, 0, 0, NEWT_GRID_COMPONENT, checktree, 0, 0, 0, 1, 
		     NEWT_ANCHOR_RIGHT, 0);
    newtGridSetField(grid, 0, 1, NEWT_GRID_COMPONENT, button, 0, 0, 0, 0, 
		     0, 0);

    newtGridWrappedWindow(grid, "�����å��ܥå����ĥ꡼�ƥ���");
    newtGridFree(grid, 1);

    form = newtForm(NULL, NULL, 0);
    newtFormAddComponents(form, checktree, button, NULL);

    answer = newtRunForm(form);

    newtFinished();

    result = newtCheckboxTreeGetSelection(checktree, &numselected);
    ptr = result;
    if (!result || !numselected)
	printf("none selected\n");
    else
	printf("Current selection (all) (%d):\n", numselected);
    for (i = 0; i < numselected; i++) {
	j = (int) *ptr++;
	printf("%d\n", j);
    }
    result = newtCheckboxTreeGetMultiSelection(checktree, &numselected, 'b');
    ptr = result;
    if (!result || !numselected)
	printf("none selected\n");
    else
	printf("Current selection (b) (%d):\n",numselected);
    for (i = 0; i < numselected; i++) {
	j = (int) *ptr++;
	printf("%d\n", j);
    }
	
    if (result)
	free(result);

    list = newtCheckboxTreeFindItem(checktree, (void *) 213);
    printf("path:");
    for (i = 0; list && list[i] != NEWT_ARG_LAST; i++)
        printf(" %d", list[i]);
    printf("\n");
    
    newtFormDestroy(form);
    
    return 0;
}
