# Project ID used by some IDEs
GUID		= {ea5c487d-fc04-4f9e-8a49-5ff9200a708c}
TEMPLATE	= app
TARGET		= qfd

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		= fontdisplayer.h
SOURCES		= fontdisplayer.cpp \
		  qfd.cpp
