# Project ID used by some IDEs
GUID 	 	= {ee430114-1a03-433b-a5a5-3589eafefba2}
TEMPLATE 	= lib
TARGET      	= trivial
win32:TARGET	= nptrivial

CONFIG	       += qt dll release
LIBS	       += -lqnp
unix:LIBS      += -lXt

HEADERS		=
SOURCES		= trivial.cpp
DEF_FILE	= trivial.def
RC_FILE		= trivial.rc
