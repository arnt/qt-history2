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

HEADERS =	qfiledlg.h	qmsgbox.h	qprndlg.h
SOURCES =	qfiledlg.cpp	qmsgbox.cpp	qprndlg.cpp
OBJECTS =	qfiledlg.o	qmsgbox.o	qprndlg.o
SRCMETA =	mfiledlg.cpp	mmsgbox.cpp	mprndlg.cpp
OBJMETA =	mfiledlg.o	mmsgbox.o	mprndlg.o
TARGET = $(LIBDIR)/libqdialogs.a

#######	Implicit rules

.SUFFIXES:
.SUFFIXES: .cpp $(SUFFIXES)

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

showsources:
	@echo $(SOURCES)

moc: $(SRCMETA)

depend: $(SRCMETA)
	makedepend -I$(INCDIR) $(SOURCES) $(SRCMETA) 2> /dev/null

clean:
	rm -f *.o *.bak *~ *% #*
	rm -f $(SRCMETA)

#######	Meta classes

mfiledlg.cpp: qfiledlg.h
	moc qfiledlg.h -o mfiledlg.cpp

mmsgbox.cpp: qmsgbox.h
	moc qmsgbox.h -o mmsgbox.cpp

mprndlg.cpp: qprndlg.h
	moc qprndlg.h -o mprndlg.cpp


# DO NOT DELETE THIS LINE -- make depend depends on it.
