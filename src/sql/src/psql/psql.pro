TEMPLATE	= lib
CONFIG		+= qt warn_on release plugin
HEADERS		= qsql_psql.h 
SOURCES		= main.cpp \
		  qsql_psql.cpp 
unix:OBJECTS_DIR	= .obj
win32 {
	OBJECTS_DIR	= obj
	LIBS	+= libpqdll.lib
}

!isEmpty(LIBS) {
	CONFIG += postgresql_libs_and_headers
}
!isEmpty(INCLUDEPATH) {
	CONFIG += postgresql_libs_and_headers
}

REQUIRES	= sql postgresql_libs_and_headers

TARGET		= qsqlpsql
DESTDIR		= ../../../../plugins/sqldrivers

target.path=$$plugins.path/sqldrivers
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/sqldrivers
INSTALLS += target
