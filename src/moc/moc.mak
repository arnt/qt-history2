#############################################################################
# Qt Makefile for the Meta Object Compiler (moc)
#
# The files mocgen.cpp and lex.yy.c are generated by yacc and lex.
#
# You'll need the Win32 GNU package in order to compile the moc from scratch.
# Get it from ftp://ftp.cygnus.com/pub/gnu-win32/win32
#
#############################################################################

####### Compiler, tools and options

CC	=	cl
CFLAGS	=	-O1 -nologo
INCPATH	=	-I. -I..\tools
LINK	=	link
LFLAGS	=	/SUBSYSTEM:console /NOLOGO
LIBS	=	

####### Files

HEADERS =	
SOURCES =	
OBJECTS =	qbuffer.obj \
		qcollect.obj \
		qdatetm.obj \
		qdstream.obj \
		qgarray.obj \
		qgdict.obj \
		qglist.obj \
		qglobal.obj \
		qgvector.obj \
		qiodev.obj \
		qstring.obj \
		mocgen.obj
TARGET	=	moc.exe

####### Implicit rules

.SUFFIXES: .cpp .c

.cpp.obj:
	$(CC) -c $(CFLAGS) $(INCPATH) -Fo$@ $<

.c.obj:
	$(CC) -c $(CFLAGS) $(INCPATH) -Fo$@ $<

####### Make targets

all: $(TARGET) 

$(TARGET): $(OBJECTS)
	$(LINK) $(LFLAGS) /OUT:$(TARGET) @<<
	    $(OBJECTS) $(LIBS)
<<

clean:
	-del qbuffer.obj
	-del qcollect.obj
	-del qdatetm.obj
	-del qdstream.obj
	-del qgarray.obj
	-del qgdict.obj
	-del qglist.obj
	-del qglobal.obj
	-del qgvector.obj
	-del qiodev.obj
	-del qstring.obj
	-del mocgen.obj
	-del moc.exe

####### Compile


####### Lex/yacc programs and options

LEX	=	flex
YACC	=	byacc -d

####### Lex/yacc files

LEXIN	=	moc.l
LEXOUT  =	lex.yy.c
YACCIN	=	moc.y
YACCOUT =	y.tab.c
YACCHDR =	y.tab.h
MOCGEN  =	mocgen.cpp

####### Process lex/yacc files

$(LEXOUT): $(LEXIN)
	$(LEX) $(LEXIN)

$(MOCGEN): moc.y $(LEXOUT)
	$(YACC) moc.y
	-del $(MOCGEN)
	-ren $(YACCOUT) $(MOCGEN)

####### Compile the C++ sources

mocgen.obj: mocgen.cpp
	$(CC) -c $(CFLAGS) $(INCPATH) $? -o $@

qbuffer.obj: ..\tools\qbuffer.cpp
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $?

qcollect.obj: ..\tools\qcollect.cpp
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $?

qdatetm.obj: ..\tools\qdatetm.cpp
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $?

qdstream.obj: ..\tools\qdstream.cpp
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $?

qgarray.obj: ..\tools\qgarray.cpp
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $?

qgdict.obj: ..\tools\qgdict.cpp
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $?

qglist.obj: ..\tools\qglist.cpp
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $?

qglobal.obj: ..\tools\qglobal.cpp
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $?

qgvector.obj: ..\tools\qgvector.cpp
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $?

qiodev.obj: ..\tools\qiodev.cpp
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $?

qstring.obj: ..\tools\qstring.cpp
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $?
