TEMPLATE	= lib
CONFIG 		= xbase
include(../include/shared.pri)
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
SOURCES	        += ../src/sqlinterpreter.cpp \
		../src/filedriver_xbase.cpp \
		../src/op.cpp \
		../src/parser.cpp \
		../src/qdb.cpp 

TARGET          += qdb
VERSION		+= 1.0.0
DESTDIR		+= ../lib

# install
target.path=$$QDB_INSTALL_LIBPATH
isEmpty(target.path):target.path=/usr/local/lib
INSTALLS += target

#headers
headers.path=$$QDB_INSTALL_HEADERPATH
isEmpty(headers.path):headers.path=/usr/local/include/qdb
headers.files = ../include/*.h
INSTALLS += headers
