GUID 		= {ab5f3bbf-6d38-4c6a-8f55-44c8a5fa51d0}
TEMPLATE	= app
TARGET		= fileiconview

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES        = iconview full-config

HEADERS		= mainwindow.h \
		  qfileiconview.h \
		  ../dirview/dirview.h
SOURCES		= main.cpp \
		  mainwindow.cpp \
		  qfileiconview.cpp \
		  ../dirview/dirview.cpp
