TEMPLATE 	= app
CONFIG		+= qt warn_off release
HEADERS		= frame.h \
		  qthumbwheel.h \
                  display.h \
		  textdrawing/textedit.h \
		  textdrawing/helpwindow.h \
		  dnd/dnd.h \
		  dnd/styledbutton.h \
		  dnd/iconview.h \
		  dnd/listview.h \
		  i18n/i18n.h \
		  i18n/wrapper.h \
		  ../aclock/aclock.h
SOURCES		= frame.cpp \
		  qthumbwheel.cpp \
               	  display.cpp \
		  textdrawing/textedit.cpp \
		  textdrawing/helpwindow.cpp \
		  dnd/dnd.cpp \
		  dnd/styledbutton.cpp \
		  dnd/iconview.cpp \
		  dnd/listview.cpp \
		  i18n/i18n.cpp \
		  ../aclock/aclock.cpp \
		  main.cpp

canvas {
	HEADERS += graph.h \
		  qasteroids/toplevel.h \
		  qasteroids/view.h \
		  qasteroids/ledmeter.h
	SOURCES += graph.cpp \
		  qasteroids/toplevel.cpp \
		  qasteroids/view.cpp \
		  qasteroids/ledmeter.cpp
}

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

table {
	INTERFACES += widgets/widgetsbase.ui
}

!table {
	INTERFACES += widgets/widgetsbase_pro.ui
}

include( ../../src/qt_professional.pri )

TARGET		= demo
INCLUDEPATH	+= .
DEPENDPATH	= ../../include
INTERFACES	+= dnd/dndbase.ui
unix:LIBS += -lm
REQUIRES=full-config nocrosscompiler
