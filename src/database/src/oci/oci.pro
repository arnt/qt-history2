TEMPLATE	= lib
CONFIG		= qt warn_on debug
DEFINES        += QT_SQL_SUPPORT
win32:DEFINES  += QT_DLL
win32:CONFIG   += dll
HEADERS		= qsql_oci.h \
		../../qsqlresultinfo.h \
		../../qsqldriver.h \
		../../qsqlresult.h \
		../../qsql.h \
		../../qsqlerror.h

SOURCES		= main.cpp \
		  qsql_oci.cpp \
		  ../../qsqlresultinfo.cpp \
		  ../../qsqldriver.cpp \
		  ../../qsqlresult.cpp \
		  ../../qsql.cpp \
		  ../../qsqlerror.cpp

win32:OBJECTS_DIR	= obj

win32:INCLUDEPATH +=../../ \
		    c:\oracle\Ora81\OCI\include

win32:LIBS       += c:\oracle\Ora81\OCI\lib\msvc\oci.lib

DESTDIR		= ../../lib/
TARGET		= qsqloci