GUID 		= {8e5eb874-838c-4a4a-b750-d6964c69ed10}
TEMPLATE	= lib
TARGET		= grapher
win32:TARGET	= npgrapher

CONFIG	       += qt dll release
LIBS	       += -lqnp
unix:LIBS      += -lXt

HEADERS		=
SOURCES		= grapher.cpp
DEF_FILE	= grapher.def
RC_FILE		= grapher.rc
