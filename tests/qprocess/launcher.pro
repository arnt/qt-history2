TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= commands.h \
		  infotext.h \
		  launcher.h \
		  qcppsyntaxhighlighter.h \
		  qrichtext_p.h \
		  qtextedit.h \
		  quickbutton.h \
		  sourceviewer.h
SOURCES		+= launcher.cpp \
		   main.cpp \
		   qcppsyntaxhighlighter.cpp \
		   qrichtext.cpp \
		   qtextedit.cpp \
		   quickbutton.cpp \
		   sourceviewer.cpp
INTERFACES	= 
TARGET		= launcher
