/* a reasonable dialog */

#include <fcntl.h>
#include <popt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "dialogboxes.h"
#include "newt.h"

enum mode { MODE_NONE, MODE_INFOBOX, MODE_MSGBOX, MODE_YESNO, MODE_CHECKLIST,
            MODE_INPUTBOX, MODE_RADIOLIST, MODE_MENU, MODE_GAUGE,
            MODE_TEXTBOX };

#define OPT_MSGBOX              1000
#define OPT_CHECKLIST           1001
#define OPT_YESNO               1002
#define OPT_INPUTBOX            1003
#define OPT_FULLBUTTONS         1004
#define OPT_MENU                1005
#define OPT_RADIOLIST           1006
#define OPT_GAUGE               1007
#define OPT_INFOBOX             1008
#define OPT_TEXTBOX             1009

static void usage(void) {
    newtFinished(); 
    fprintf(stderr, "whiptail: bad parameters (see man whiptail(1) for details)\n");
    exit(DLG_ERROR);
}

char *
readTextFile(const char * filename)
{
    int fd = open(filename, O_RDONLY, 0);
    struct stat s;
    char * buf;

    if ( fd < 0 || fstat(fd, &s) != 0 ) {
       perror(filename);
       exit(DLG_ERROR);
     }

    if ( (buf = malloc(s.st_size)) == 0 )
       fprintf(stderr, "%s: too large to display.\n", filename);

    if ( read(fd, buf, s.st_size) != s.st_size ) {
        perror(filename);
        exit(DLG_ERROR);
    }
   close(fd);
   return buf;
}

int main(int argc, const char ** argv) {
    enum mode mode = MODE_NONE;
    poptContext optCon;
    int arg;
    const char * optArg;
    const char * text;
    const char * nextArg;
    char * end;
    int height;
    int width;
    int fd = -1;
    int needSpace = 0;
    int noCancel = 0;
    int noTags = 0;
    int noItem = 0;
    int clear = 0;
    int scrollText = 0;
    int rc = 1;
    int flags = 0;
    int defaultNo = 0;
    int separateOutput = 0;
    int outputfd = 2;
    FILE *output = stderr;
    const char * result;
    const char ** selections, ** next;
    char * title = NULL;
    char * backtitle = NULL;
    struct poptOption optionsTable[] = {
            { "backtitle", '\0', POPT_ARG_STRING, &backtitle, 0 },
            { "checklist", '\0', 0, 0, OPT_CHECKLIST },
            { "clear", '\0', 0, &clear, 0 },
            { "defaultno", '\0', 0, &defaultNo, 0 },
            { "inputbox", '\0', 0, 0, OPT_INPUTBOX },
            { "fb", '\0', 0, 0, OPT_FULLBUTTONS },
            { "fullbuttons", '\0', 0, 0, OPT_FULLBUTTONS },
            { "gauge", '\0', 0, 0, OPT_GAUGE },
            { "infobox", '\0', 0, 0, OPT_INFOBOX },
            { "menu", '\0', 0, 0, OPT_MENU },
            { "msgbox", '\0', 0, 0, OPT_MSGBOX },
            { "nocancel", '\0', 0, &noCancel, 0 },
            { "noitem", '\0', 0, &noItem, 0 },
            { "notags", '\0', 0, &noTags, 0 },
            { "radiolist", '\0', 0, 0, OPT_RADIOLIST },
            { "scrolltext", '\0', 0, &scrollText, 0 },
            { "separate-output", '\0', 0, &separateOutput, 0 },
            { "title", '\0', POPT_ARG_STRING, &title, 0 },
            { "yesno", '\0', 0, 0, OPT_YESNO },
            { "textbox", '\0', 0, 0, OPT_TEXTBOX },
            { 0, 0, 0, 0, 0 } 
    };
    
    optCon = poptGetContext("whiptail", argc, argv, optionsTable, 0);

    while ((arg = poptGetNextOpt(optCon)) > 0) {
        optArg = poptGetOptArg(optCon);

        switch (arg) {
          case OPT_INFOBOX:
            if (mode != MODE_NONE) usage();
            mode = MODE_INFOBOX;
            break;

          case OPT_MENU:
            if (mode != MODE_NONE) usage();
            mode = MODE_MENU;
            break;

          case OPT_MSGBOX:
            if (mode != MODE_NONE) usage();
            mode = MODE_MSGBOX;
            break;
          case OPT_TEXTBOX:
            if (mode != MODE_NONE) usage();
            mode = MODE_TEXTBOX;
            break;
          case OPT_RADIOLIST:
            if (mode != MODE_NONE) usage();
            mode = MODE_RADIOLIST;
            break;

          case OPT_CHECKLIST:
            if (mode != MODE_NONE) usage();
            mode = MODE_CHECKLIST;
            break;

          case OPT_FULLBUTTONS:
            useFullButtons(1);
            break;

          case OPT_YESNO:
            if (mode != MODE_NONE) usage();
            mode = MODE_YESNO;
            break;

          case OPT_GAUGE:
            if (mode != MODE_NONE) usage();
            mode = MODE_GAUGE;
            break;

          case OPT_INPUTBOX:
            if (mode != MODE_NONE) usage();
            mode = MODE_INPUTBOX;
            break;
        }
    }
    
    if (arg < -1) {
        fprintf(stderr, "%s: %s\n", 
                poptBadOption(optCon, POPT_BADOPTION_NOALIAS), 
                poptStrerror(arg));
        exit(1);
    }

    output = fdopen (outputfd, "w");
    if (output == NULL ) {
        perror ("Cannot open output-fd\n");
        exit (DLG_ERROR);
    }

    if (mode == MODE_NONE) usage();

    if (!(text = poptGetArg(optCon))) usage();

    if ( mode == MODE_TEXTBOX ) text = readTextFile(text);

    if (!(nextArg = poptGetArg(optCon))) usage();
    height = strtoul(nextArg, &end, 10);
    if (*end) usage();

    if (!(nextArg = poptGetArg(optCon))) usage();
    width = strtoul(nextArg, &end, 10);
    if (*end) usage();

    if (mode == MODE_GAUGE) {
        fd = dup(0);
        close(0);
        if (open("/dev/tty", O_RDWR) != 0) perror("open /dev/tty");
    }

    newtInit();
    newtCls();
    width -= 2;
    height -= 2;
    newtOpenWindow((80 - width) / 2, (24 - height) / 2, width, height, title);

    if (backtitle)
        newtDrawRootText(0, 0, backtitle);

    if (noCancel) flags |= FLAG_NOCANCEL;
    if (noItem) flags |= FLAG_NOITEM;
    if (noTags) flags |= FLAG_NOTAGS;
    if (scrollText) flags |= FLAG_SCROLL_TEXT;
    if (defaultNo) flags |= FLAG_DEFAULT_NO;

    switch (mode) {
      case MODE_MSGBOX:
      case MODE_TEXTBOX:
        rc = messageBox(text, height, width, MSGBOX_MSG, flags);
        break;

      case MODE_INFOBOX:
        rc = messageBox(text, height, width, MSGBOX_INFO, flags);
        break;

      case MODE_YESNO:
        rc = messageBox(text, height, width, MSGBOX_YESNO, flags);
        break;

      case MODE_INPUTBOX:
        rc = inputBox(text, height, width, optCon, flags, &result);
        if (rc == DLG_OKAY) fprintf(output, "%s", result);
        break;

      case MODE_MENU:
        rc = listBox(text, height, width, optCon, flags, &result);
        if (rc == DLG_OKAY) fprintf(output, "%s", result);
        break;

      case MODE_RADIOLIST:
        rc = checkList(text, height, width, optCon, 1, flags, &selections);
        if (rc == DLG_OKAY) {
            fprintf(output, "%s", selections[0]);
            free(selections);
        }
        break;

      case MODE_CHECKLIST:
        rc = checkList(text, height, width, optCon, 0, flags, &selections);

        if (!rc) {
            for (next = selections; *next; next++) {
                if (!separateOutput) {
                    if (needSpace) putc(' ', output);
                    fprintf(output, "\"%s\"", *next);
                    needSpace = 1;
                } else {
                    fprintf(output, "%s\n", *next);
                }
            }

            free(selections);
        }
        break;

      case MODE_GAUGE:
        rc = gauge(text, height, width, optCon, fd, flags);
        break;

      default:
        usage();
    }

    if (rc == -1) usage();

    if (clear)
        newtPopWindow();
    newtFinished();

    return rc;
}
