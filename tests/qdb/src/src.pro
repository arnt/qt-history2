TEMPLATE	= lib
include(../include/shared.pri)
SOURCES	        += sqlinterpreter.cpp environment.cpp filedriver_xbase.cpp op.cpp qdb.cpp 

# xbase 
XBASE_ROOT	= ../xdb-1.2.0
XBASE_PATH	= $$XBASE_ROOT/xdb
DEFINES		+= HAVE_CONFIG_H
SOURCES		+= $$XBASE_PATH/dbf.cpp \
		$$XBASE_PATH/fields.cpp\
		$$XBASE_PATH/index.cpp\
		$$XBASE_PATH/ndx.cpp\
		$$XBASE_PATH/xbase.cpp\
		$$XBASE_PATH/xbstring.cpp\
		$$XBASE_PATH/stack.cpp\
		$$XBASE_PATH/exp.cpp\
		$$XBASE_PATH/expproc.cpp\
		$$XBASE_PATH/expfunc.cpp\
		$$XBASE_PATH/xdate.cpp
INCLUDEPATH	+= $$XBASE_PATH $$XBASE_ROOT

HEADERS		+= ../include/sqlinterpreter.h ../include/environment.h ../include/op.h ../include/qdb.h
TARGET          += qdb
VERSION		+= 1.0.0
DESTDIR		+= ../lib

# install
target.path=$$QDB_INSTALL_LIBPATH
isEmpty(target.path):target.path=/usr/local/lib
INSTALLS += target

