TEMPLATE 	= app
CONFIG		+= qt warn_on
CONFIG -= opengl
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
		  opengl/glcontrolwidget.h \
		  opengl/gltexobj.h \
		  opengl/glbox.h \
		  opengl/glgear.h \
		  opengl/gllandscape.h \
		  opengl/fbm.h
opengl:SOURCES += opengl/glworkspace.cpp \
		  opengl/glcontrolwidget.cpp \
		  opengl/gltexobj.cpp \
		  opengl/glbox.cpp \
		  opengl/glgear.cpp \
		  opengl/gllandscape.cpp \
		  opengl/fbm.c

opengl:INTERFACES += opengl/printpreview.ui \
		     opengl/gllandscapeviewer.ui

sql {
	INTERFACES += sql/book.ui \
		      sql/editbook.ui \
		      sql/connect.ui
}

TARGET		= demo
INCLUDEPATH	+= .
DEPENDPATH	= ../../include
