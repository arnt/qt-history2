# Project ID used by some IDEs
GUID 		= {dbf9f81d-bb97-42c8-8a9b-dc45f4c28e27}
TEMPLATE	= app
TARGET		= addressbook

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		= centralwidget.h \
		  mainwindow.h
SOURCES		= centralwidget.cpp \
		  main.cpp \
		  mainwindow.cpp
