#############################################################################
# Makefile for building qgl
# Generated by tmake at 12:40, 1997/09/29
#     Project: qgl
#    Template: lib
#############################################################################

####### Compiler, tools and options

CC	=	cl
CFLAGS	=	-nologo -O1
INCPATH	=	-I"$(QTDIR)\include"
LIB	=	lib
MOC	=	moc

####### Files

HEADERS =	..\..\..\include\qgl.h
SOURCES =	qgl.cpp
OBJECTS =	qgl.obj
SRCMOC	=	moc_qgl.cpp
OBJMOC	=	moc_qgl.obj
TARGET	=	qgl.lib

####### Implicit rules

.SUFFIXES: .cpp .cxx .cc .c

.cpp.obj:
	$(CC) -c $(CFLAGS) $(INCPATH) -Fo$@ $<

.cxx.obj:
	$(CC) -c $(CFLAGS) $(INCPATH) -Fo$@ $<

.cc.obj:
	$(CC) -c $(CFLAGS) $(INCPATH) -Fo$@ $<

.c.obj:
	$(CC) -c $(CFLAGS) $(INCPATH) -Fo$@ $<

####### Build rules

all: $(TARGET) 

$(TARGET): $(OBJECTS) $(OBJMOC)
	$(LIB) /OUT:$(TARGET) @<<
	    $(OBJECTS) $(OBJMOC)
<<
	-copy $(TARGET) ..\..\..\lib

moc: $(SRCMOC)

tmake: qgl.mak

qgl.mak: qgl.pro
	tmake qgl.pro -nodepend -o qgl.mak

clean:
	-del qgl.obj
	-del moc_qgl.cpp
	-del moc_qgl.obj
	-del $(TARGET)

####### Compile

qgl.obj: qgl.cpp

moc_qgl.obj: moc_qgl.cpp \
		..\..\..\include\qgl.h
	$(CC) -c $(CFLAGS) $(INCPATH) -Fomoc_qgl.obj moc_qgl.cpp

moc_qgl.cpp: ..\..\..\include\qgl.h
	$(MOC) ..\..\..\include\qgl.h -o moc_qgl.cpp
