TEMPLATE	= app
CONFIG		= qt warn_on release
HEADERS		= commands.h \
		  infotext.h \
		  launcher.h \
		  qcppsyntaxhighlighter.h \
		  qprocess.h \
		  qrichtext_p.h \
		  qtextedit.h \
		  qtexteditintern_p.h \
		  quickbutton.h \
		  sourceviewer.h
SOURCES		= launcher.cpp \
		  main.cpp \
		  qcppsyntaxhighlighter.cpp \
		  qprocess.cpp \
		  qrichtext.cpp \
		  qtextedit.cpp \
		  qtexteditintern.cpp \
		  quickbutton.cpp \
		  sourceviewer.cpp
INTERFACES	= 
TARGET		= launcher
