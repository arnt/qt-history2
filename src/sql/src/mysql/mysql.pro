TEMPLATE	= lib
CONFIG		+= qt warn_on debug plugin

HEADERS		= qsql_mysql.h 

SOURCES		= main.cpp qsql_mysql.cpp 

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

TARGET		= qsqlmysql
DESTDIR		= ../../../../plugins/sqldrivers

isEmpty(LIBS) {
	message( "No MySQL libraries specified for linking.  See the Qt SQL Module documentation for information on building SQL driver plugins." )
}

target.path=$$plugins.path/sqldrivers
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/sqldrivers
INSTALLS += target
