GUID 		= {37c65906-584c-498b-b801-cf3cc6ee3517}
TEMPLATE	= app
TARGET		= xform

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		=
SOURCES		= xform.cpp
