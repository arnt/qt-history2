TEMPLATE = lib
CONFIG += console debug dll
CONFIG -= qt app_bundle uic
DEFINES += QT_BUILD_QMAKE_LIBRARY
DESTDIR = $(QTDIR)/lib

OBJECTS_DIR = .
MOC_DIR = .

#guts
bootstrap { 
    VPATH += $(QTDIR)/src/core/global $(QTDIR)/src/core/tools \
             $(QTDIR)/src/core/kernel $(QTDIR)/src/core/plugin \
	     $(QTDIR)/src/core/io 
    INCPATH += $(QTDIR)/include/QtCore 
}
INCPATH += $(QTDIR)/qmake/generators $(QTDIR)/qmake/generators/unix \
           $(QTDIR)/qmake/generators/win32 $(QTDIR)/qmake/generators/mac 
VPATH += $(QTDIR)/qmake
INCPATH += $(QTDIR)/qmake
include($(QTDIR)/qmake/qmake.pri)|error(Could not get qmake.pri!!!)
SOURCES -= main.cpp #we don't need the main.cpp
#actual code introduced by the library
HEADERS += qmakeinterface.h
SOURCES += qmakeinterface.cpp

include(../../sharedcomponents.pri)
