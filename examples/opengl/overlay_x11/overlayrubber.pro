GUID 		= {6d446255-88fb-43a1-b395-7311395bbdbf}
TEMPLATE	= app
TARGET		= overlayrubber

CONFIG		+= qt opengl warn_on release
DEPENDPATH	= ../include

QTDIR_build:REQUIRES        = opengl full-config

HEADERS		= gearwidget.h \
		  rubberbandwidget.h
SOURCES		= gearwidget.cpp \
		  main.cpp \
		  rubberbandwidget.cpp
