TEMPLATE	= lib
CONFIG		= qt warn_on debug plugin
HEADERS		= qsql_oci.h
SOURCES		= main.cpp \
		  qsql_oci.cpp 

win32:OBJECTS_DIR	= obj
unix:OBJECTS_DIR	= .obj

win32:INCLUDEPATH += c:\oracle\Ora81\OCI\include

win32:LIBS      += oci.lib
unix:LIBS	+= -lclntsh

DESTDIR		= $(QTDIR)/plugins
TARGET		= qsqloci