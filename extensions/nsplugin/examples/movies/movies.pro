TEMPLATE	= lib
CONFIG		+= qt warn_on release dll
HEADERS		= 
SOURCES		= main.cpp
win32:LIBS	+= $$QT_BUILD_TREE\lib\qnp.lib $$QT_BUILD_TREE\lib\qtmain.lib
unix:LIBS	= -lqnp -lXt
TARGET		= npmovies
DEPENDPATH=../../include
