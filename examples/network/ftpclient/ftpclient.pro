# Project ID used by some IDEs
GUID 		= {f34fbdf0-70e7-4a41-bf1c-632a198b1f09}
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
