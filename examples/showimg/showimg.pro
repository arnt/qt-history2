GUID 		= {d2208650-a3b7-4b00-beed-298598ff8a09}
TEMPLATE	= app
TARGET		= showimg

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		= showimg.h imagetexteditor.h \
		  imagefip.h
SOURCES		= main.cpp \
		  imagetexteditor.cpp \
		  showimg.cpp \
		  imagefip.cpp
