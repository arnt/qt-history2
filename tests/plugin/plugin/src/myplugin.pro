TEMPLATE	= lib
CONFIG		= qt warn_on release 
win32:CONFIG   += dll
HEADERS		= previewstack.h \
		  styledbutton.h \
		  customaction.h
SOURCES		= init.cpp \
		  previewstack.cpp \
		  styledbutton.cpp \
		  customaction.cpp
TARGET		= myplugin
DESTDIR	        = ..