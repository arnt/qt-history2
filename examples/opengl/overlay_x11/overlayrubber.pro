QTDIR_build:REQUIRES        = opengl full-config
TEMPLATE	= app
CONFIG		+= qt opengl warn_on release
HEADERS		= gearwidget.h \
		  rubberbandwidget.h
SOURCES		= gearwidget.cpp \
		  main.cpp \
		  rubberbandwidget.cpp
TARGET		= overlayrubber
DEPENDPATH	= ../include
