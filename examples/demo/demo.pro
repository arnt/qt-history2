TEMPLATE 	= app
CONFIG		+= qt warn_on debug
HEADERS		= frame.h \
		  graph.h \
                  display.h \
		  textdrawing/textedit.h \
		  textdrawing/helpwindow.h \
		  dnd/dnd.h \
		  dnd/styledbutton.h \
		  dnd/iconview.h \
		  dnd/listview.h \
		  i18n/i18n.h \
		  i18n/wrapper.h
SOURCES		= frame.cpp \
		  graph.cpp \
               	  display.cpp \
		  textdrawing/textedit.cpp \
		  textdrawing/helpwindow.cpp \
		  dnd/dnd.cpp \
		  dnd/styledbutton.cpp \
		  dnd/iconview.cpp \
		  dnd/listview.cpp \
		  i18n/i18n.cpp \
		  main.cpp

opengl {
	HEADERS += opengl/glworkspace.h \
		   opengl/glcontrolwidget.h \
		   opengl/gltexobj.h \
		   opengl/glbox.h \
		   opengl/glgear.h \
		   opengl/gllandscape.h \
		   opengl/fbm.h
	SOURCES += opengl/glworkspace.cpp \
	 	   opengl/glcontrolwidget.cpp \
		   opengl/gltexobj.cpp \
		   opengl/glbox.cpp \
		   opengl/glgear.cpp \
		   opengl/gllandscape.cpp \
		   opengl/fbm.c

	INTERFACES += opengl/printpreview.ui \
		      opengl/gllandscapeviewer.ui
}

sql {
	INTERFACES += sql/book.ui \
		      sql/editbook.ui \
	              sql/connect.ui
}

TARGET		= demo
INCLUDEPATH	+= .
DEPENDPATH	= ../../include
INTERFACES	+= dnd/dndbase.ui
