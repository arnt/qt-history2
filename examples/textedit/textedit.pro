TEMPLATE	= app
TARGET		= textedit

CONFIG		+= qt warn_on release
QT = compat

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, full-config)"

HEADERS		= textedit.h
SOURCES		= textedit.cpp \
		  main.cpp
IMAGES 		= editcopy.xpm editcut.xpm editpaste.xpm editredo.xpm editundo.xpm filenew.xpm fileopen.xpm fileprint.xpm filesave.xpm textbold.xpm textcenter.xpm textitalic.xpm textjustify.xpm textleft.xpm textright.xpm textunder.xpm
