TEMPLATE	= lib
CONFIG		= qt warn_on debug
win32:DEFINES	+= QT_DLL
win32:CONFIG    += dll

HEADERS		= qsql_mysql.h 

SOURCES		= main.cpp qsql_mysql.cpp 

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

unix:INCLUDEPATH += /usr/include/mysql 
win32:INCLUDEPATH += c:\home\db\src\mysql

unix:LIBS       += -lmysqlclient
win32:LIBS	+= libmySQL.lib

TARGET		= qsqlmysql
DESTDIR		= $(QTDIR)/plugins