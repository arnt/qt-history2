GUID 		= {9f0b4770-f3a2-4c65-a2ae-cb75d92263a1}
TEMPLATE	= app
TARGET		= gear

CONFIG		+= qt opengl warn_on release
!mac:unix:LIBS  += -lm
DEPENDPATH	= ../include

QTDIR_build:REQUIRES        = opengl

HEADERS		=
SOURCES		= gear.cpp
