REQUIRES	= network full-config
TEMPLATE	= app
CONFIG		+= qt warn_on release

HEADERS		= ftpviewitem.h
SOURCES		= main.cpp \
		  ftpviewitem.cpp
FORMS		= ftpmainwindow.ui \
		  connectdialog.ui
IMAGES		= images/file.png \
		  images/folder.png

TARGET		= ftpclient
