x11:QTDIR_build:REQUIRES        = nas
TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= sound.h
SOURCES		= sound.cpp
TARGET		= sound
DEPENDPATH=../../include
QTDIR_build:REQUIRES=full-config
