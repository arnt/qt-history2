# Project ID used by some IDEs
GUID 		= {92ac8d5c-b76d-484c-8d2b-b12aae8b094e}
TEMPLATE	= app
TARGET		= textedit

CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES	= full-config nocrosscompiler

HEADERS		= textedit.h
SOURCES		= textedit.cpp \
		  main.cpp
IMAGES 		= editcopy.xpm editcut.xpm editpaste.xpm editredo.xpm editundo.xpm filenew.xpm fileopen.xpm fileprint.xpm filesave.xpm textbold.xpm textcenter.xpm textitalic.xpm textjustify.xpm textleft.xpm textright.xpm textunder.xpm
