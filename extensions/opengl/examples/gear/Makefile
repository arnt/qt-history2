####### This section was automatically generated from
#######    /home/hanord/qt/Makefile for building on
#######    linux-gcc-shared on Fri Feb 28 17:08:50 MET 1997

INCDIR = $(QTDIR)/include
CFLAGS = -O2 -fno-strength-reduce -Wall -W
LIBCFLAGS = -fPIC
YACCCFLAGS = -Wno-unused -Wno-parentheses
LFLAGS = -L$(QTDIR)/lib -lqt
CC = gcc
MOC = moc
SHELL =	/bin/sh

####### End of automatically generated section
#
# $Source: /tmp/cvs/qt/extensions/opengl/examples/gear/Makefile,v $
#

####### Compiler, tools and options

# Include path and libraries for Mesa.
# For SGI OpenGL: replace MesaGL with GL and MesaGLU with GLU.
GL_INC	=	-I/usr/X11R6/include
GL_LIBS =	-L/usr/X11R6/lib -lMesaGL -lMesaGLU -lXmu -lXext -lm

INCOPT	=	-I$(INCDIR) $(GL_INC)
LIBS	=	$(LFLAGS) -lqgl $(GL_LIBS)

####### Files

HEADERS =	
SOURCES =	gear.cpp
OBJECTS =	gear.o
SRCMOC	=	
OBJMOC	=	
TARGET	=	gear

####### Implicit rules

.SUFFIXES: .cpp .C

.cpp.o:
	$(CC) -c $(CFLAGS) $(INCOPT) -o $@ $<

.C.o:
	$(CC) -c $(CFLAGS) $(INCOPT) -o $@ $<

####### Build rules

all: $(TARGET) 

$(TARGET): $(OBJECTS) $(OBJMOC)
	$(CC) -o $(TARGET) $(OBJECTS) $(OBJMOC) $(LIBS)

moc: $(SRCMOC)

clean:
	-rm -f $(OBJECTS) $(OBJMOC) $(SRCMOC) $(TARGET)

####### Compile

gear.o: gear.cpp
