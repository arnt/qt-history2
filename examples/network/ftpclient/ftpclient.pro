REQUIRES	= network full-config
TEMPLATE	= app
CONFIG		+= qt warn_on release

SOURCES		= main.cpp
FORMS		= ftpmainwindow.ui \
		  connectdialog.ui
IMAGES		= images/file.png \
		  images/folder.png

TARGET		= ftpclient
