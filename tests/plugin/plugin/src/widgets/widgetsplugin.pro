TEMPLATE	= lib
CONFIG		= qt warn_on release
WIN32:CONFIG   += dll
HEADERS		= previewstack.h \
		  styledbutton.h
SOURCES		= main.cpp \
		  previewstack.cpp \
		  styledbutton.cpp
INTERFACES	=
DESTDIR		= ../../
