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

opengl:HEADERS += opengl/glworkspace.h \
		  opengl/gltexobj.h \
		  opengl/glbox.h \
		  opengl/glgear.h
opengl:SOURCES += opengl/glworkspace.cpp \
		  opengl/gltexobj.cpp \
		  opengl/glbox.cpp \
		  opengl/glgear.cpp

opengl:INTERFACES += opengl/printpreview.ui

TARGET		= demo
DEPENDPATH	= ../../include
