TEMPLATE 	= app
CONFIG		+= qt warn_on
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

opengl:HEADERS += opengl/globjwin.h \
		  opengl/gltexobj.h
opengl:SOURCES += opengl/globjwin.cpp \
		  opengl/gltexobj.cpp

TARGET		= demo
DEPENDPATH	= ../../include
