# Project ID used by some IDEs
GUID 		= {d9e5f065-a75d-484f-bb2a-94c9617a14d2}
TEMPLATE	= app
TARGET		= life

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= medium-config

HEADERS		= life.h \
		  lifedlg.h
SOURCES		= life.cpp \
		  lifedlg.cpp \
		  main.cpp
