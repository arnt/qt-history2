TEMPLATE	= lib
CONFIG+= qt warn_on release plugin
HEADERS		= ../../../../src/sql/drivers/oci/qsql_oci.h
SOURCES		= main.cpp \
		  ../../../../src/sql/drivers/oci/qsql_oci.cpp 

win32 {
	OBJECTS_DIR	= obj
	LIBS	+= oci.lib
}
unix:OBJECTS_DIR	= .obj

!isEmpty(LIBS) {
	CONFIG += oci_libs_and_headers
}
!isEmpty(INCLUDEPATH) {
	CONFIG += oci_libs_and_headers
}

REQUIRES	= sql oci_libs_and_headers

DESTDIR		= ../../../sqldrivers
TARGET		= qsqloci

target.path=$$plugins.path/sqldrivers
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/sqldrivers
INSTALLS += target
