# Project ID used by some IDEs
GUID 		= {157a23b3-1788-4f56-bd23-95ecb4123f3d}
TEMPLATE	= app
TARGET		= launcher

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES        = embedded large-config

HEADERS		=
SOURCES		= launcher.cpp
