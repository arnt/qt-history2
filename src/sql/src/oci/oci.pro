TEMPLATE	= lib
CONFIG+= qt warn_on debug plugin
HEADERS		= qsql_oci.h
SOURCES		= main.cpp \
		  qsql_oci.cpp 

win32:OBJECTS_DIR	= obj
unix:OBJECTS_DIR	= .obj

isEmpty(LIBS) {
	message( "No OCI libraries specified for linking.  See the Qt SQL Module documentation for information on building SQL driver plugins." )
}

DESTDIR		= ../../../../plugins/sqldrivers
TARGET		= qsqloci

target.path=$$plugins.path/sqldrivers
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/sqldrivers
INSTALLS += target
