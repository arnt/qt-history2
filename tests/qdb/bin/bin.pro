TEMPLATE	= app
CONFIG		= xbase
include(../include/shared.pri)
SOURCES	        += main.cpp
QMAKE_LIBDIR		+= ../lib
LIBS		+= -lqdb
INCLUDEPATH	+= ../include
DESTDIR		= ../bin
TARGET          = qdb

# install
target.path=$$QDB_INSTALL_BINPATH
isEmpty(target.path):target.path=/usr/local/bin
INSTALLS += target




