TEMPLATE	= lib
CONFIG		+= qt plugin
HEADERS		= ../../../../src/sql/drivers/mysql/qsql_mysql.h 
SOURCES		= main.cpp \
		  ../../../../src/sql/drivers/mysql/qsql_mysql.cpp 

unix {
	OBJECTS_DIR	= .obj
	isEmpty(LIBS) {
		LIBS	+= -lmysqlclient
	}
}
win32 {
	OBJECTS_DIR	= obj
	isEmpty(LIBS) {
		LIBS	+= libmysql.lib
	}
}

REQUIRES	= sql

TARGET		= qsqlmysql
DESTDIR		= ../../../sqldrivers

target.path=$$plugins.path/sqldrivers
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/sqldrivers
INSTALLS += target
