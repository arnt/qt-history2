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

HEADERS =	qfiledlg.h	qmsgbox.h	qprndlg.h	qtabdlg.h
SOURCES =	qfiledlg.cpp	qmsgbox.cpp	qprndlg.cpp	qtabdlg.cpp
BETA	= 	qtabdlg.cpp qtabdlg.h
BETA	=
OBJECTS =	qfiledlg.o	qmsgbox.o	qprndlg.o	qtabdlg.o
SRCMETA =	mfiledlg.cpp	mmsgbox.cpp	mprndlg.cpp	mtabdlg.cpp
OBJMETA =	mfiledlg.o	mmsgbox.o	mprndlg.o	mtabdlg.o
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

showfiles:
	@echo $(HEADERS) $(SOURCES) Makefile

include:
	ln -s `echo $(HEADERS) | fmt -1 | sed 's-^-../src/dialogs/-'` $(INCDIR)

showheaders:
	@echo $(HEADERS)

showbeta:
	@echo $(BETA)

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


-include .depend
