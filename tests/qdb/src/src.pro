TEMPLATE	= lib
CONFIG 		= xbase
include(../include/shared.pri)
TARGET          += qdb
VERSION		+= 1.0.0
DESTDIR		+= ../lib

# install
target.path=$$QDB_INSTALL_LIBPATH
isEmpty(target.path):target.path=/usr/local/lib
INSTALLS += target

