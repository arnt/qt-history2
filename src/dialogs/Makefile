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

rcs:: ./RCS/$$@,v

showfiles:
	@echo $(HEADERS) $(SOURCES) Makefile

include:
	ln -s `echo $(HEADERS) | fmt -1 | sed 's-^-../src/dialogs/-'` $(INCDIR)

showheaders:
	@echo $(HEADERS)

showsources:
	@echo $(SOURCES)

depend: $(SRCMETA)
	makedepend -I$(INCDIR) $(SOURCES) $(SRCMETA) 2> /dev/null

clean:
	rm -f *.o *.bak *BAK *~ *% #*
	rm -f $(SRCMETA)

# 'make checkout' updates files that MUST be old
# 'make checkout CODATE=-d1994/12/24' recreates 1994/12/24
# 'make checkout CODATE=-d2000/1/1 updates all files
checkout:
	@for rcs in RCS/*,v ; do \
	    file=`basename $$rcs ,v` ;\
	    [ -n "${CODATE}" ] || \
	    [ '!' -f $$file ] || \
	    ( [ '!' -w $$file ] && [ $$rcs -nt $$file ] ) && \
	    co ${CODATE} $$file ;\
	done; \
	true	# tell make it worked

#######	Meta classes

mfiledlg.cpp: qfiledlg.h
	$(QTDIR)/bin/moc qfiledlg.h -o mfiledlg.cpp

mmsgbox.cpp: qmsgbox.h
	$(QTDIR)/bin/moc qmsgbox.h -o mmsgbox.cpp

mprndlg.cpp: qprndlg.h
	$(QTDIR)/bin/moc qprndlg.h -o mprndlg.cpp


# DO NOT DELETE THIS LINE -- make depend depends on it.

