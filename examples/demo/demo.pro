TEMPLATE 	= app
CONFIG		+= qt warn_on release
HEADERS		= frame.h \
		  graph.h \
                  display.h \
		  textdrawing/textedit.h \
		  textdrawing/helpwindow.h
SOURCES		= frame.cpp \
		  graph.cpp \
                  display.cpp \
		  textdrawing/textedit.cpp \
		  textdrawing/helpwindow.cpp \
		  main.cpp
TARGET		= demo
DEPENDPATH	= ../../include
