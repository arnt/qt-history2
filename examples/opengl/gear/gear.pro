TEMPLATE	= app
TARGET		= gear

CONFIG		+= qt warn_on release
QT         += opengl
!mac:unix:LIBS  += -lm
DEPENDPATH	= ../include

QTDIR_build:REQUIRES        = "contains(QT_CONFIG, opengl)"

HEADERS		=
SOURCES		= gear.cpp
