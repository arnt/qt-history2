TEMPLATE	= app
TARGET		= ftpclient

QCONFIG         += network
CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES	= network full-config nocrosscompiler

HEADERS		= ftpviewitem.h
SOURCES		= main.cpp \
		  ftpviewitem.cpp
FORMS		= ftpmainwindow.ui \
		  connectdialog.ui
IMAGES		= images/file.png \
		  images/folder.png
