#############################################################################
# Qt Makefile - dialogs
#############################################################################

#######	Directories

BASEDIR	=	$(QTDIR)
INCDIR	=	$(BASEDIR)/include
LIBDIR	=	$(BASEDIR)/lib

#######	Compiler

CFLAGS	=	-O2
CC	=	gcc

#######	Files

INTERNAL=	qfontdialog.h	qfontdialog.cpp	qprogdlg.h	qprogdlg.cpp
HEADERS =	qfiledlg.h	qmsgbox.h	qprndlg.h	qtabdlg.h \
		qfontdialog.h	qprogdlg.h
SOURCES =	qfiledlg.cpp	qmsgbox.cpp	qprndlg.cpp	qtabdlg.cpp \
		qfontdialog.cpp	qprogdlg.cpp
OBJECTS =	qfiledlg.o	qmsgbox.o	qprndlg.o	qtabdlg.o \
		qfontdialog.o	qprogdlg.o
SRCMETA =	mfiledlg.cpp	mmsgbox.cpp	mprndlg.cpp	mtabdlg.cpp \
		mfontdialog.cpp	mprogdlg.cpp
OBJMETA =	mfiledlg.o	mmsgbox.o	mprndlg.o	mtabdlg.o \
		mfontdialog.o	mprogdlg.o
TARGET = $(LIBDIR)/libqdialogs.a

#######	Implicit rules

.SUFFIXES: .cpp

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

#######	Build rules

all: $(HEADERS) $(SOURCES) $(TARGET)

obj: $(HEADERS) $(SOURCES) $(OBJECTS) $(OBJMETA)

$(TARGET): $(OBJECTS) $(OBJMETA)
	rm -f $(TARGET)
	ar q $(TARGET) $(OBJECTS) $(OBJMETA)
	ranlib $(TARGET)

include:
	ln -s `echo $(HEADERS) | fmt -1 | sed 's-^-../src/dialogs/-'` $(INCDIR)

showfiles:
	@echo $(HEADERS) $(SOURCES) Makefile

showinternal:
	@echo $(INTERNAL)

showheaders:
	@echo $(HEADERS)

showsources:
	@echo $(SOURCES)

moc: $(SRCMETA)

depend: $(SRCMETA)
	gcc -I$(INCDIR) -M $(SOURCES) $(SRCMETA) > .depend

clean:
	rm -f *.o .depend *.bak *~ *% #*
	rm -f $(SRCMETA)

#######	Meta classes

mfiledlg.cpp: qfiledlg.h
	moc qfiledlg.h -o mfiledlg.cpp

mmsgbox.cpp: qmsgbox.h
	moc qmsgbox.h -o mmsgbox.cpp

mprndlg.cpp: qprndlg.h
	moc qprndlg.h -o mprndlg.cpp

mtabdlg.cpp: qtabdlg.h
	moc qtabdlg.h -o mtabdlg.cpp

mfontdialog.cpp: qfontdialog.h
	moc qfontdialog.h -o mfontdialog.cpp

mprogdlg.cpp: qprogdlg.h
	moc qprogdlg.h -o mprogdlg.cpp

-include .depend
