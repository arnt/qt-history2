TEMPLATE	= lib
CONFIG+= qt warn_on debug plugin

HEADERS		= qsql_tds.h \
		  ../shared/qsql_result.h

SOURCES		= main.cpp \
		  qsql_tds.cpp \
		  ../shared/qsql_result.cpp

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

isEmpty(LIBS) {
	message( "No TDS libraries specified for linking.  See the Qt SQL Module documentation for information on building SQL driver plugins." )
}

TARGET		= qsqltds
DESTDIR		= ../../../../plugins/sqldrivers

target.path=$$plugins.path/sqldrivers
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/sqldrivers
INSTALLS += target
