TEMPLATE 	= app
CONFIG		+= qt warn_on release
HEADERS		= frame.h \
		  graph.h \
		  textdrawing/textedit.h
SOURCES		= frame.cpp \
		  graph.cpp \
		  textdrawing/textedit.cpp \
		  main.cpp 
TARGET		= demo
DEPENDPATH	= ../../include
