TEMPLATE	= app
TARGET		= fileiconview

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES        = iconview "contains(QT_CONFIG, full-config)"

HEADERS		= mainwindow.h \
		  qfileiconview.h \
		  ../dirview/dirview.h
SOURCES		= main.cpp \
		  mainwindow.cpp \
		  qfileiconview.cpp \
		  ../dirview/dirview.cpp
