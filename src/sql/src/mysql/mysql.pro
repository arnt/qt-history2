TEMPLATE	= lib
CONFIG+= qt warn_on debug plugin

HEADERS		= qsql_mysql.h 

SOURCES		= main.cpp qsql_mysql.cpp 

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

unix:INCLUDEPATH += /usr/include/mysql /usr/local/include/mysql
win32:INCLUDEPATH += c:\home\db\src\mysql

unix:LIBS       += -L/usr/local/lib/mysql -lmysqlclient
win32:LIBS	+= libmySQL.lib

TARGET		= qsqlmysql
DESTDIR		= ../../../../plugins/sqldrivers

target.path=$$plugins.path/sqldrivers
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/sqldrivers
INSTALLS += target
