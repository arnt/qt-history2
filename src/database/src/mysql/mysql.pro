TEMPLATE	= lib
CONFIG		= qt warn_on debug
DEFINES        	+= QT_SQL_SUPPORT
win32:DEFINES	+= QT_DLL
win32:CONFIG   += dll

HEADERS		= qsql_mysql.h \
		../../qsqlresultinfo.h \
		../../qsqldriver.h \
		../../qsqlresult.h \
		../../qsql.h \
		../../qsqlerror.h

SOURCES		= main.cpp \
		  qsql_mysql.cpp \
		  ../../qsqlresultinfo.cpp \
		  ../../qsqldriver.cpp \
		  ../../qsqlresult.cpp \
		  ../../qsql.cpp \
		  ../../qsqlerror.cpp

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

unix:INCLUDEPATH += /usr/include/mysql \
		    ../../

win32:INCLUDEPATH += c:\home\db\src\mysql \
			../../

unix:LIBS        += -lmysqlclient

win32:LIBS		+= C:\HOME\DB\SRC\MYSQL\LIB\libmySQL.lib

TARGET		= qsqlmysql
DESTDIR		= ../../lib/