TEMPLATE	= lib
include(../include/shared.pri)
LIBS		+= -lxdb
SOURCES	        += sqlinterpreter.cpp environment.cpp filedriver_xbase.cpp
HEADERS		+= ../include/sqlinterpreter.h ../include/environment.h
TARGET          += qdb
VERSION		+= 1.0.0
DESTDIR		+= ../lib

# install
target.path=$$QDB_INSTALL_LIBPATH
isEmpty(target.path):target.path=/usr/local/lib
INSTALLS += target

