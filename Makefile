LIBS = -L. -lnewt -lslang -lm #-lefence
CFLAGS = -g -Wall

VERSION = 0.1

PROGS = test
OBJS = test.o
LIBNEWT = libnewt.a
LIBOBJS = newt.o button.o form.o checkbox.o entry.o label.o listbox.o \
          scrollbar.o textbox.o

prefix = /usr
includedir = $(prefix)/include
libdir = $(prefix)/lib

#--------------------------------------

SOURCES = $(subst .o,.c,$(OBJS) $(LIBOBJS)) 

ifeq (.depend,$(wildcard .depend))
TARGET=$(PROGS)
else
TARGET=depend $(PROGS)
endif

all:	$(TARGET)

test:	$(OBJS) $(LIBNEWT)
	gcc -g -o test $(OBJS) $(LIBS)

$(LIBNEWT): $(LIBNEWT)($(LIBOBJS))

newt.o: newt.c Makefile
	$(CC) $(CFLAGS) -DVERSION=\"$(VERSION)\" -c -o $@ $<

veryclean: clean
	rm -f .depend

clean:
	rm -f $(PROGS) $(OBJS) $(LIBOBJS) $(LIBNEWT) core

depend:
	$(CPP) $(CFLAGS) -M $(SOURCES) > .depend

install:
	install -m 755 -o 0 -g 0 -d $(libdir)
	install -m 755 -o 0 -g 0 -d $(includedir)
	install -m 644 -o 0 -g 0 newt.h $(includedir)
	install -m 644 -o 0 -g 0 $(LIBNEWT) $(libdir)

archive: 
	@rm -rf /tmp/newt-$(VERSION)
	@mkdir /tmp/newt-$(VERSION)
	@tar cSpf - * | (cd /tmp/newt-$(VERSION); tar xSpf -)
	@cd /tmp/newt-$(VERSION); \
	    make clean; \
	    find . -name "RCS" -exec rm {} \;  ; \
	    find . -name ".depend" -exec rm {} \;  ; \
	    rm -f *gz test
	@cd /tmp; tar czSpf newt-$(VERSION).tar.gz newt-$(VERSION)
	@rm -rf /tmp/newt-$(VERSION)
	@cp /tmp/newt-$(VERSION).tar.gz .
	@rm -f /tmp/newt-$(VERSION).tar.gz 
	@echo " "
	@echo "The final archive is ./newt-$(VERSION).tar.gz. You should run"
	@echo "-n$(VERSION): RCS/*,v on all of the directories btw."

ifeq (.depend,$(wildcard .depend))
include .depend
endif

