TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= commands.h \
		  infotext.h \
		  launcher.h \
		  quickbutton.h
#		  qcppsyntaxhighlighter.h \
#		  sourceviewer.h
SOURCES		+= launcher.cpp \
		   main.cpp \
		   quickbutton.cpp
#		   qcppsyntaxhighlighter.cpp \
#		   sourceviewer.cpp
INTERFACES	= 
TARGET		= launcher
