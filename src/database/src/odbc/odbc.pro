TEMPLATE	= lib
CONFIG		= qt warn_on debug
DEFINES        	+= QT_SQL_SUPPORT
win32:DEFINES 	+= QT_DLL
win32:CONFIG   	+= dll

HEADERS		= qsql_odbc.h \
		../../qsqlresultinfo.h \
		../../qsqldriver.h \
		../../qsqlresult.h \
		../../qsql.h \
		../../qsqlerror.h

SOURCES		= main.cpp \
		  qsql_odbc.cpp \
		  ../../qsqlresultinfo.cpp \
		  ../../qsqldriver.cpp \
		  ../../qsqlresult.cpp \
		  ../../qsql.cpp \
		  ../../qsqlerror.cpp



unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

unix:INCLUDEPATH += ../../
win32:INCLUDEPATH +=..\..\

unix:LIBS       += -lodbc

win32:LIBS  	+= odbc32.lib

TARGET		= qsqlodbc
DESTDIR		= ../../lib/