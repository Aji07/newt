#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "newt.h"

int main(void) {
    newtComponent b1, b2, b3, b4;
    newtComponent answer, f, t;
    newtGrid grid, subgrid;
    char * flowedText;
    int textWidth, textHeight, rc;
    char * menuContents[] = { "��", "��", "��", "��", "��", NULL };
    char * entries[10];
    struct newtWinEntry autoEntries[] = {
	{ "����ȥ�", entries + 0, 0 },
	{ "�̤Υ���ȥ�", entries + 1, 0 },
	{ "�����ܤΥ���ȥ�", entries + 2, 0 },
	{ "�����ܤΥ���ȥ�", entries + 3, 0 },
	{ NULL, NULL, 0 } };

    memset(entries, 0, sizeof(entries));

    newtInit();
    newtCls();

    b1 = newtCheckbox(-1, -1, "�ƥ��ȤΤ���Τ��ʤ�Ĺ�������å��ܥå���", ' ', NULL, NULL);
    b2 = newtButton(-1, -1, "�̤Υܥ���");
    b3 = newtButton(-1, -1, "��������������");
    b4 = newtButton(-1, -1, "��������������");

    f = newtForm(NULL, NULL, 0);

    grid = newtCreateGrid(2, 2);
    newtGridSetField(grid, 0, 0, NEWT_GRID_COMPONENT, b1, 0, 0, 0, 0, 
			NEWT_ANCHOR_RIGHT, 0);
    newtGridSetField(grid, 0, 1, NEWT_GRID_COMPONENT, b2, 0, 0, 0, 0, 0, 0);
    newtGridSetField(grid, 1, 0, NEWT_GRID_COMPONENT, b3, 0, 0, 0, 0, 0, 0);
    newtGridSetField(grid, 1, 1, NEWT_GRID_COMPONENT, b4, 0, 0, 0, 0, 0, 0);


    newtFormAddComponents(f, b1, b2, b3, b4, NULL);

    newtGridWrappedWindow(grid, "�����ܤΥ�����ɥ�");
    newtGridFree(grid, 1);

    answer = newtRunForm(f);
	
    newtFormDestroy(f);
    newtPopWindow();

    flowedText = newtReflowText("����Ϥ��ʤ�ƥ����Ȥ餷����ΤǤ���40�����"
			  	"��Ĺ���ǡ���åԥ󥰤��Ԥ��ޤ���"
			  	"���ᤤ���㿧�θѤ��Τ�ޤʸ�������"
			  	"�ۤ����Τ��ΤäƤ뤫��?\n\n"
				"¾�ˤ��Τ餻���뤳�ȤȤ��ơ�Ŭ���˲��Ԥ򤹤�"
				"���Ȥ����פǤ���",
				40, 5, 5, &textWidth, &textHeight);
    t = newtTextbox(-1, -1, textWidth, textHeight, NEWT_FLAG_WRAP);
    newtTextboxSetText(t, flowedText);
    free(flowedText);

    
    b1 = newtButton(-1, -1, "λ��");
    b2 = newtButton(-1, -1, "����󥻥�");

    grid = newtCreateGrid(1, 2);
    subgrid = newtCreateGrid(2, 1);

    newtGridSetField(subgrid, 0, 0, NEWT_GRID_COMPONENT, b1, 0, 0, 0, 0, 0, 0);
    newtGridSetField(subgrid, 1, 0, NEWT_GRID_COMPONENT, b2, 0, 0, 0, 0, 0, 0);

    newtGridSetField(grid, 0, 0, NEWT_GRID_COMPONENT, t, 0, 0, 0, 1, 0, 0);
    newtGridSetField(grid, 0, 1, NEWT_GRID_SUBGRID, subgrid, 0, 0, 0, 0, 0,
			NEWT_GRID_FLAG_GROWX);
    newtGridWrappedWindow(grid, "�̤���");
    newtGridDestroy(grid, 1);

    f = newtForm(NULL, NULL, 0);
    newtFormAddComponents(f, b1, t, b2, NULL);
    answer = newtRunForm(f);

    newtPopWindow();
    newtFormDestroy(f);

    newtWinMessage("����ץ�", "λ��", "����ϥ���ץ�ʥ�å�����������ɥ��Ǥ�");
    newtWinChoice("����ץ�", "λ��", "����󥻥�", "����ϥ���ץ�����򥦥���ɥ��Ǥ�");

    textWidth = 0;
    rc = newtWinMenu("�ƥ��ȥ�˥塼", "����� newtWinMenu() ������Υ���ץ�"
		     "�Ǥ��� ��������С���ɬ�פ˱����ƤĤ����ꡢ "
		     "�Ĥ��ʤ��ä��ꤷ�ޤ���", 50, 5, 5, 3, 
		     menuContents, &textWidth, "λ��", "����󥻥�", NULL);

    rc = newtWinEntries("�ƥ����� newtWinEntries()", "����� newtWinEntries()"
		     "������Υ���ץ�Ǥ��������ؤ��ñ�ˤ�����������Ϥ�"
		     "�������Ȥ��Ǥ��ޤ���", 50, 5, 5, 20, autoEntries, "λ��", 
		     "����󥻥�", NULL);

    newtFinished();

    printf("rc = 0x%x item = %d\n", rc, textWidth);

    return 0;
}
