# Project ID used by some IDEs
GUID 		= {7f62fa8c-29e3-4359-a470-aa745c862e34}
TEMPLATE	= app
TARGET		= qdir

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		= qdir.h ../dirview/dirview.h
SOURCES		= qdir.cpp ../dirview/dirview.cpp
