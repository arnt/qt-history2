QTDIR_build:REQUIRES        = opengl
TEMPLATE	= app
CONFIG		+= qt opengl warn_on release
HEADERS		= 
SOURCES		= gear.cpp
TARGET		= gear
DEPENDPATH	= ../include
!mac:unix:LIBS		+= -lm

