TEMPLATE	= app
TARGET		= gear

CONFIG		+= qt opengl warn_on release
!mac:unix:LIBS  += -lm
DEPENDPATH	= ../include

QTDIR_build:REQUIRES        = opengl

HEADERS		=
SOURCES		= gear.cpp
