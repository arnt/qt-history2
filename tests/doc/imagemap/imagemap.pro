TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= imagemap.h imagetexteditor.h \
		  imagefip.h
SOURCES		= main.cpp \
		  imagetexteditor.cpp \
		  imagemap.cpp \
		  imagefip.cpp
TARGET		= imagemap
DEPENDPATH=../../include
