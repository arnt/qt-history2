TEMPLATE	= lib
CONFIG		+= qt warn_on release plugin
HEADERS		= qsql_mysql.h 
SOURCES		= main.cpp qsql_mysql.cpp 

unix:OBJECTS_DIR	= .obj
win32 {
	OBJECTS_DIR	= obj
	LIBS	+= libmysql.lib
}

!isEmpty(LIBS) {
	CONFIG += mysql_libs_and_headers
}
!isEmpty(INCLUDEPATH) {
	CONFIG += mysql_libs_and_headers
}

REQUIRES	= sql mysql_libs_and_headers


TARGET		= qsqlmysql
DESTDIR		= ../../../../plugins/sqldrivers

target.path=$$plugins.path/sqldrivers
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/sqldrivers
INSTALLS += target
