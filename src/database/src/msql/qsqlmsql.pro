TEMPLATE	= lib
CONFIG		= qt warn_on debug
DEFINES        += QT_SQL_SUPPORT  
WIN32:CONFIG   += dll
HEADERS		= qsql_mysql.h \
		../../qsql_base.h 
SOURCES		= main.cpp \
		  qsql_mysql.cpp \
		  ../../qsql_base.cpp

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

unix:INCLUDEPATH += /usr/include/mysql \
		    ../../

 win32:INCLUDEPATH += c:\home\db\src\mysql \
			../../

unix:LIBS        += -lmysqlclient

win32:LIBS		+= C:\HOME\DB\SRC\MYSQL\LIB\libmySQL.lib

DESTDIR		= ../../lib/
