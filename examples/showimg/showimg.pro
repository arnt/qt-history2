TEMPLATE	= app
TARGET		= showimg

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, full-config)"

HEADERS		= showimg.h imagetexteditor.h \
		  imagefip.h
SOURCES		= main.cpp \
		  imagetexteditor.cpp \
		  showimg.cpp \
		  imagefip.cpp
QT	+= compat
