#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "Python.h"
#include "newt.h"

typedef struct snackWidget_s snackWidget;
typedef struct snackGrid_s snackGrid;
typedef struct snackForm_s snackForm;

static void emptyDestructor(PyObject * s);

static snackWidget * buttonWidget(PyObject * s, PyObject * args);
static PyObject * centeredWindow(PyObject * s, PyObject * args);
static snackWidget * checkboxWidget(PyObject * s, PyObject * args);
static PyObject * choiceWindow(PyObject * s, PyObject * args);
static snackWidget * entryWidget(PyObject * s, PyObject * args);
static PyObject * drawRootText(PyObject * s, PyObject * args);
static snackForm * formCreate(PyObject * s, PyObject * args);
static snackGrid * gridCreate(PyObject * s, PyObject * args);
static PyObject * gridWrappedWindow(PyObject * s, PyObject * args);
static PyObject * finishScreen(PyObject * s, PyObject * args);
static PyObject * initScreen(PyObject * s, PyObject * args);
static snackWidget * labelWidget(PyObject * s, PyObject * args);
static snackWidget * listboxWidget(PyObject * s, PyObject * args);
static PyObject * messageWindow(PyObject * s, PyObject * args);
static PyObject * openWindow(PyObject * s, PyObject * args);
static PyObject * popHelpLine(PyObject * s, PyObject * args);
static PyObject * popWindow(PyObject * s, PyObject * args);
static PyObject * pushHelpLine(PyObject * s, PyObject * args);
static snackWidget * radioButtonWidget(PyObject * s, PyObject * args);
static PyObject * refreshScreen(PyObject * s, PyObject * args);
static PyObject * reflowText(PyObject * s, PyObject * args);
static snackWidget * textWidget(PyObject * s, PyObject * args);
static PyObject * ternaryWindow(PyObject * s, PyObject * args);

static PyMethodDef snackModuleMethods[] = {
    { "button", (PyCFunction) buttonWidget, METH_VARARGS, NULL },
    { "checkbox", (PyCFunction) checkboxWidget, METH_VARARGS, NULL },
    { "choice", choiceWindow, METH_VARARGS, NULL },
    { "centeredwindow", centeredWindow, METH_VARARGS, NULL },
    { "drawroottext", drawRootText, METH_VARARGS, NULL },
    { "entry", (PyCFunction) entryWidget, METH_VARARGS, NULL },
    { "finish", finishScreen, METH_VARARGS, NULL },
    { "form", (PyCFunction) formCreate, METH_VARARGS, NULL },
    { "grid", (PyCFunction) gridCreate, METH_VARARGS, NULL },
    { "gridwrappedwindow", gridWrappedWindow, METH_VARARGS, NULL },
    { "init", initScreen, METH_VARARGS, NULL },
    { "label", (PyCFunction) labelWidget, METH_VARARGS, NULL },
    { "listbox", (PyCFunction) listboxWidget, METH_VARARGS, NULL },
    { "message", messageWindow, METH_VARARGS, NULL },
    { "openwindow", openWindow, METH_VARARGS, NULL },
    { "pophelpline", popHelpLine, METH_VARARGS, NULL },
    { "popwindow", popWindow, METH_VARARGS, NULL },
    { "pushhelpline", pushHelpLine, METH_VARARGS, NULL },
    { "radiobutton", (PyCFunction) radioButtonWidget, METH_VARARGS, NULL },
    { "reflow", (PyCFunction) reflowText, METH_VARARGS, NULL },
    { "refresh", refreshScreen, METH_VARARGS, NULL },
    { "ternary", ternaryWindow, METH_VARARGS, NULL },
    { "textbox", (PyCFunction) textWidget, METH_VARARGS, NULL },
    { NULL }
} ;

struct snackGrid_s {
    PyObject_HEAD;
    newtGrid grid;
} ;

static PyObject * gridGetAttr(PyObject * s, char * name);
static PyObject * gridPlace(snackGrid * s, PyObject * args);
static PyObject * gridSetField(snackGrid * s, PyObject * args);

static PyMethodDef gridMethods[] = {
    { "place", (PyCFunction) gridPlace, METH_VARARGS, NULL },
    { "setfield", (PyCFunction) gridSetField, METH_VARARGS, NULL },
    { NULL }
};

static PyTypeObject snackGridType = {
        PyObject_HEAD_INIT(&PyType_Type)
        0,                              /* ob_size */
        "snackgrid",                    /* tp_name */
        sizeof(snackGrid),              /* tp_size */
        0,                              /* tp_itemsize */
        emptyDestructor,      			/* tp_dealloc */
        0,                              /* tp_print */
        gridGetAttr,    		/* tp_getattr */
        0,                              /* tp_setattr */
        0,                              /* tp_compare */
        0,                              /* tp_repr */
        0,                              /* tp_as_number */
        0,                              /* tp_as_sequence */
        0,                		/* tp_as_mapping */
};

struct snackForm_s {
    PyObject_HEAD;
    newtComponent fo;
} ;

static PyObject * formGetAttr(PyObject * s, char * name);
static PyObject * formAdd(snackForm * s, PyObject * args);
static PyObject * formRun(snackForm * s, PyObject * args);
static PyObject * formHotKey(snackForm * s, PyObject * args);

static PyMethodDef formMethods[] = {
    { "add", (PyCFunction) formAdd, METH_VARARGS, NULL },
    { "run", (PyCFunction) formRun, METH_VARARGS, NULL },
    { "addhotkey", (PyCFunction) formHotKey, METH_VARARGS, NULL },
    { NULL }
};

static PyTypeObject snackFormType = {
        PyObject_HEAD_INIT(&PyType_Type)
        0,                              /* ob_size */
        "snackform",                    /* tp_name */
        sizeof(snackForm),              /* tp_size */
        0,                              /* tp_itemsize */
        emptyDestructor,      		/* tp_dealloc */
        0,                              /* tp_print */
        formGetAttr,    		/* tp_getattr */
        0,                              /* tp_setattr */
        0,                              /* tp_compare */
        0,                              /* tp_repr */
        0,                              /* tp_as_number */
        0,                              /* tp_as_sequence */
        0,                		/* tp_as_mapping */
};

struct snackWidget_s {
    PyObject_HEAD;
    newtComponent co;
    char achar;
    void * apointer;
    int anint;
} ;

static PyObject * widgetGetAttr(PyObject * s, char * name);
static PyObject * widgetEntrySetValue(snackWidget * s, PyObject * args);
static PyObject * widgetListboxSetW(snackWidget * s, PyObject * args);
static PyObject * widgetListboxAdd(snackWidget * s, PyObject * args);
static PyObject * widgetListboxIns(snackWidget * s, PyObject * args);
static PyObject * widgetListboxDel(snackWidget * s, PyObject * args);
static PyObject * widgetListboxGet(snackWidget * s, PyObject * args);

static PyMethodDef widgetMethods[] = {
    { "entrySetValue", (PyCFunction) widgetEntrySetValue, METH_VARARGS, NULL },
    { "listboxAddItem", (PyCFunction) widgetListboxAdd, METH_VARARGS, NULL },
    { "listboxInsertItem", (PyCFunction) widgetListboxIns, METH_VARARGS, NULL },
    { "listboxGetCurrent", (PyCFunction) widgetListboxGet, METH_VARARGS, NULL },
    { "listboxSetWidth", (PyCFunction) widgetListboxSetW, METH_VARARGS, NULL },
    { "listboxDeleteItem", (PyCFunction) widgetListboxDel, METH_VARARGS, NULL },
    { NULL }
};

static PyTypeObject snackWidgetType = {
        PyObject_HEAD_INIT(&PyType_Type)
        0,                              /* ob_size */
        "snackwidget",                  /* tp_name */
        sizeof(snackWidget),            /* tp_size */
        0,                              /* tp_itemsize */
        emptyDestructor,      		/* tp_dealloc */
        0,                              /* tp_print */
        widgetGetAttr,  		/* tp_getattr */
        0,                              /* tp_setattr */
        0,                              /* tp_compare */
        0,                              /* tp_repr */
        0,                              /* tp_as_number */
        0,                              /* tp_as_sequence */
        0,                		/* tp_as_mapping */
};

static PyObject * initScreen(PyObject * s, PyObject * args) {
    newtInit();
    newtCls();

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * finishScreen(PyObject * s, PyObject * args) {
    newtFinished();
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * refreshScreen(PyObject * s, PyObject * args) {
    newtRefresh();
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * drawRootText(PyObject * s, PyObject * args) {
    int left, top;
    char * text;

    if (!PyArg_ParseTuple(args, "iis", &left, &top, &text))
	return NULL;

    newtDrawRootText(left, top, NULL);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * popHelpLine(PyObject * s, PyObject * args) {
    newtPopHelpLine();
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * pushHelpLine(PyObject * s, PyObject * args) {
    char * text;

    if (!PyArg_ParseTuple(args, "s", &text))
	return NULL;

    if (!strcmp(text, "*default*"))
	newtPushHelpLine(NULL);
    else
	newtPushHelpLine(text);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * reflowText(PyObject * s, PyObject * args) {
    char * text, * new;
    int width, minus = 5, plus = 5;
    int realWidth, realHeight;
    PyObject * tuple;

    if (!PyArg_ParseTuple(args, "si|ii", &text, &width, &minus, &plus))
	return NULL;

    new = newtReflowText(text, width, minus, plus, &realWidth, &realHeight);

    tuple = Py_BuildValue("(sii)", new, realWidth, realHeight);
    free(new);

    return tuple;
}

static PyObject * centeredWindow(PyObject * s, PyObject * args) {
    int width, height;
    char * title;

    if (!PyArg_ParseTuple(args, "iis", &width, &height, &title))
	return NULL;

    newtCenteredWindow(width, height, title);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * gridWrappedWindow(PyObject * s, PyObject * args) {
    snackGrid * grid;
    char * title;

    if (!PyArg_ParseTuple(args, "O!s", &snackGridType, &grid, &title))
	return NULL;

    newtGridWrappedWindow(grid->grid, title);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * openWindow(PyObject * s, PyObject * args) {
    int left, top, width, height;
    char * title;

    if (!PyArg_ParseTuple(args, "iiiis", &left, &top, &width, &height, &title))
	return NULL;

    newtOpenWindow(left, top, width, height, title);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * popWindow(PyObject * s, PyObject * args) {
    newtPopWindow();
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * messageWindow(PyObject * s, PyObject * args) {
    char * title, * text;
    char * okbutton = "Ok";

    if (!PyArg_ParseTuple(args, "ss|s", &title, &text, &okbutton)) 
	return NULL;

    newtWinMessage(title, okbutton, text);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * choiceWindow(PyObject * s, PyObject * args) {
    char * title, * text;
    char * okbutton = "Ok";
    char * cancelbutton = "Cancel";
    int rc;

    if (!PyArg_ParseTuple(args, "ss|ss", &title, &text, &okbutton, 
			  &cancelbutton)) 
	return NULL;

    rc = newtWinChoice(title, okbutton, cancelbutton, text);

    switch (rc) {
      case 0: return Py_BuildValue("i", 1);
      case 1: return Py_BuildValue("i", 2);
    }

    return Py_BuildValue("i", 0);
}

static PyObject * ternaryWindow(PyObject * s, PyObject * args) {
    char * title, * text, * button1, * button2, * button3;
    int rc;

    if (!PyArg_ParseTuple(args, "sssss", &title, &text, &button1, &button2, 
			  &button3)) 
	return NULL;

    rc = newtWinTernary(title, button1, button2, button3, text);

    return Py_BuildValue("i", rc);
}

static snackWidget * buttonWidget(PyObject * s, PyObject * args) {
    snackWidget * widget;
    char * label;

    if (!PyArg_ParseTuple(args, "s", &label)) return NULL;

    widget = PyObject_NEW(snackWidget, &snackWidgetType);
    widget->co = newtButton(-1, -1, label);

    return widget;
}

static snackWidget * labelWidget(PyObject * s, PyObject * args) {
    char * label;
    snackWidget * widget;

    if (!PyArg_ParseTuple(args, "s", &label)) return NULL;

    widget = PyObject_NEW(snackWidget, &snackWidgetType);
    widget->co = newtLabel(-1, -1, label);

    return widget;
}

static snackWidget * listboxWidget(PyObject * s, PyObject * args) {
    snackWidget * widget;
    int height;
    int doScroll = 0, returnExit = 0 ;

    if (!PyArg_ParseTuple(args, "i|ii", &height, &doScroll, &returnExit))
	return NULL;

    widget = PyObject_NEW(snackWidget, &snackWidgetType);
    widget->co = newtListbox(-1, -1, height,
				(doScroll ? 0 : NEWT_FLAG_NOSCROLL) |
				(returnExit ? NEWT_FLAG_RETURNEXIT : 0));
    widget->anint = 1;
    
    return widget;
}

static snackWidget * textWidget(PyObject * s, PyObject * args) {
    char * text;
    int width, height;
    int scrollBar = 0;
    snackWidget * widget;
    
    if (!PyArg_ParseTuple(args, "iis|i", &width, &height, &text, &scrollBar))
	return NULL;

    widget = PyObject_NEW(snackWidget, &snackWidgetType);
    widget->co = newtTextbox(-1, -1, width, height,
				scrollBar ? NEWT_FLAG_SCROLL : 0);
    newtTextboxSetText(widget->co, text);
    
    return widget;
}

static snackWidget * radioButtonWidget(PyObject * s, PyObject * args) {
    snackWidget * widget, * group;
    char * text;
    int isOn;

    if (!PyArg_ParseTuple(args, "sOi", &text, &group, &isOn)) 
		return NULL;

    widget = PyObject_NEW(snackWidget, &snackWidgetType);

    if ((PyObject *) group == Py_None)
	widget->co = newtRadiobutton(-1, -1, text, isOn, NULL);
    else
	widget->co = newtRadiobutton(-1, -1, text, isOn, group->co);

    return widget;
}

static snackWidget * checkboxWidget(PyObject * s, PyObject * args) {
    snackWidget * widget;
    char * text;
    int isOn;

    if (!PyArg_ParseTuple(args, "si", &text, &isOn)) return NULL;

    widget = PyObject_NEW(snackWidget, &snackWidgetType);
    widget->co = newtCheckbox(-1, -1, text, isOn ? '*' : ' ', NULL, 
				&widget->achar);

    return widget;
}

static snackWidget * entryWidget(PyObject * s, PyObject * args) {
    snackWidget * widget;
    int width;
    char * initial;
    int isHidden, isScrolled;

    if (!PyArg_ParseTuple(args, "isii", &width, &initial,
			  &isHidden, &isScrolled)) return NULL;

    widget = PyObject_NEW(snackWidget, &snackWidgetType);
    widget->co = newtEntry(-1, -1, initial, width, (char **) &widget->apointer, 
			   (isHidden ? NEWT_FLAG_HIDDEN : 0) |
			   (!isScrolled ? NEWT_FLAG_NOSCROLL : 0));

    return widget;
}

static snackForm * formCreate(PyObject * s, PyObject * args) {
    snackForm * form;

    form = PyObject_NEW(snackForm, &snackFormType);
    form->fo = newtForm(NULL, NULL, 0);

    return form;
}

static snackGrid * gridCreate(PyObject * s, PyObject * args) {
    int rows, cols;
    snackGrid * grid;

    if (!PyArg_ParseTuple(args, "ii", &cols, &rows)) return NULL;

    grid = PyObject_NEW(snackGrid, &snackGridType);
    grid->grid = newtCreateGrid(cols, rows);

    return grid;
}

static PyObject * gridGetAttr(PyObject * s, char * name) {
    return Py_FindMethod(gridMethods, s, name);
}

static PyObject * gridPlace(snackGrid * grid, PyObject * args) {
    int x, y;

    if (!PyArg_ParseTuple(args, "ii", &x, &y)) return NULL;

    newtGridPlace(grid->grid, x, y);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * gridSetField(snackGrid * grid, PyObject * args) {
    snackWidget * w;
    snackGrid * g;
    int x, y;
    int pLeft = 0, pTop = 0, pRight = 0, pBottom = 0;
    int anchorFlags = 0, growFlags = 0;

    if (!PyArg_ParseTuple(args, "iiO|(iiii)ii", &x, &y, 
				&w, &pLeft, &pTop, &pRight, &pBottom,
				&anchorFlags, &growFlags)) 
	return NULL;

    if (w->ob_type == &snackWidgetType) {
	newtGridSetField(grid->grid, x, y, NEWT_GRID_COMPONENT,
			 w->co, pLeft, pTop, pRight, pBottom, anchorFlags, 
			 growFlags);
    } else {
	g = (snackGrid *) w;
	newtGridSetField(grid->grid, x, y, NEWT_GRID_SUBGRID,
			 g->grid, pLeft, pTop, pRight, pBottom, anchorFlags, 
			 growFlags);
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * formGetAttr(PyObject * s, char * name) {
    return Py_FindMethod(formMethods, s, name);
}

static PyObject * formAdd(snackForm * s, PyObject * args) {
    snackWidget * w;
    int size = PyTuple_Size(args), i;
    
    if (!size) {
	/* this is a hack, I should give an error directly */
	if (!PyArg_ParseTuple(args, "O!", &snackWidgetType, &w)) 
	    return NULL;
    }

    for (i = 0; i < size; i++) {
	w = (snackWidget *) PyTuple_GET_ITEM(args, i);
	newtFormAddComponent(s->fo, w->co);
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * formRun(snackForm * s, PyObject * args) {
    struct newtExitStruct result;

    newtFormRun(s->fo, &result);

    if (result.reason == NEWT_EXIT_HOTKEY)
	return Py_BuildValue("(si)", "hotkey", result.u.key);
    else
	return Py_BuildValue("(si)", "widget", result.u.co);
}

static PyObject * formHotKey(snackForm * s, PyObject * args) {
    int key;

    if (!PyArg_ParseTuple(args, "i", &key))
	return NULL;

    newtFormAddHotKey(s->fo, key);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * widgetGetAttr(PyObject * s, char * name) {
    snackWidget * w = (snackWidget *) s;

    if (!strcmp(name, "key")) {
	return Py_BuildValue("i", w->co);
    } else if (!strcmp(name, "entryValue")) {
	return Py_BuildValue("s", w->apointer);
    } else if (!strcmp(name, "checkboxValue")) {
	return Py_BuildValue("i", w->achar == ' ' ? 0 : 1);
    } else if (!strcmp(name, "radioValue")) {
	return Py_BuildValue("i", newtRadioGetCurrent(w->co));
    }

    return Py_FindMethod(widgetMethods, s, name);
}

static PyObject * widgetEntrySetValue(snackWidget * s, PyObject * args) {
    char * val;

    if (!PyArg_ParseTuple(args, "s", &val))
	return NULL;

    newtEntrySet(s->co, val, 1);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * widgetListboxAdd(snackWidget * s, PyObject * args) {
    char * text;
    
    if (!PyArg_ParseTuple(args, "s", &text))
	return NULL;

    newtListboxAddEntry(s->co, text, (void *) s->anint);

    return PyInt_FromLong(s->anint++);
}

static PyObject * widgetListboxIns(snackWidget * s, PyObject * args) {
    char * text;
    int key;
    
    if (!PyArg_ParseTuple(args, "si", &text, &key))
	return NULL;

    newtListboxInsertEntry(s->co, text, (void *) s->anint, (void *) key);

    return PyInt_FromLong(s->anint++);
}

static PyObject * widgetListboxDel(snackWidget * s, PyObject * args) {
    int key;
    
    if (!PyArg_ParseTuple(args, "i", &key))
	return NULL;

    newtListboxDeleteEntry(s->co, (void *) key);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * widgetListboxGet(snackWidget * s, PyObject * args) {
    if (!PyArg_ParseTuple(args, ""))
	return NULL;

    return PyInt_FromLong((long) newtListboxGetCurrent(s->co));
}

static PyObject * widgetListboxSetW(snackWidget * s, PyObject * args) {
    int width;

    if (!PyArg_ParseTuple(args, "i", &width))
	return NULL;

    newtListboxSetWidth(s->co, width);

    Py_INCREF(Py_None);
    return Py_None;
}

static void emptyDestructor(PyObject * s) {
}

void init_snack(void) {
    PyObject * d, * m;

    m = Py_InitModule("_snack", snackModuleMethods);
    d = PyModule_GetDict(m);

    PyDict_SetItemString(d, "ANCHOR_LEFT", PyInt_FromLong(NEWT_ANCHOR_LEFT));
    PyDict_SetItemString(d, "ANCHOR_TOP", PyInt_FromLong(NEWT_ANCHOR_TOP));
    PyDict_SetItemString(d, "ANCHOR_RIGHT", PyInt_FromLong(NEWT_ANCHOR_RIGHT));
    PyDict_SetItemString(d, "ANCHOR_BOTTOM", 
			 PyInt_FromLong(NEWT_ANCHOR_BOTTOM));
    PyDict_SetItemString(d, "GRID_GROWX", PyInt_FromLong(NEWT_GRID_FLAG_GROWX));
    PyDict_SetItemString(d, "GRID_GROWY", PyInt_FromLong(NEWT_GRID_FLAG_GROWY));

    PyDict_SetItemString(d, "FORM_EXIT_HOTKEY", PyString_FromString("hotkey"));
    PyDict_SetItemString(d, "FORM_EXIT_WIDGET", PyString_FromString("widget"));

    PyDict_SetItemString(d, "KEY_F1", PyInt_FromLong(NEWT_KEY_F1));
    PyDict_SetItemString(d, "KEY_F2", PyInt_FromLong(NEWT_KEY_F2));
    PyDict_SetItemString(d, "KEY_F3", PyInt_FromLong(NEWT_KEY_F3));
    PyDict_SetItemString(d, "KEY_F4", PyInt_FromLong(NEWT_KEY_F4));
    PyDict_SetItemString(d, "KEY_F5", PyInt_FromLong(NEWT_KEY_F5));
    PyDict_SetItemString(d, "KEY_F6", PyInt_FromLong(NEWT_KEY_F6));
    PyDict_SetItemString(d, "KEY_F7", PyInt_FromLong(NEWT_KEY_F7));
    PyDict_SetItemString(d, "KEY_F8", PyInt_FromLong(NEWT_KEY_F8));
    PyDict_SetItemString(d, "KEY_F9", PyInt_FromLong(NEWT_KEY_F9));
    PyDict_SetItemString(d, "KEY_F10", PyInt_FromLong(NEWT_KEY_F10));
    PyDict_SetItemString(d, "KEY_F11", PyInt_FromLong(NEWT_KEY_F11));
    PyDict_SetItemString(d, "KEY_F12", PyInt_FromLong(NEWT_KEY_F12));
}
