TEMPLATE	= app
TARGET		= overlayrubber

CONFIG		+= qt opengl warn_on release
QCONFIG         += opengl
DEPENDPATH	= ../include

QTDIR_build:REQUIRES        = opengl full-config

HEADERS		= gearwidget.h \
		  rubberbandwidget.h
SOURCES		= gearwidget.cpp \
		  main.cpp \
		  rubberbandwidget.cpp
