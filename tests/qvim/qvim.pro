TEMPLATE	= lib
CONFIG		+= qt warn_on debug plugin

SOURCES		= buffer.c \
		charset.c	\
		digraph.c\
		edit.c\
		eval.c\
		ex_cmds.c\
		ex_docmd.c\
		ex_getln.c\
		fileio.c\
		getchar.c\
		gui.c\
		if_cscope.c\
		main.c\
		mark.c\
		memfile.c\
		memline.c\
		menu.c\
		message.c\
		misc1.c\
		misc2.c\
		#multbyte.c\
		normal.c\
		ops.c\
		option.c\
		os_unix.c\
		pathdef.c\
		pty.c\
		quickfix.c\
		regexp.c\
		screen.c\
		search.c\
		syntax.c\
		tag.c\
		term.c\
		ui.c\
		undo.c\
		version.c\
		window.c\
		gui_qt.cc\
		gui_qt_widget.cc\
		gui_qt_x11.cc \
		editorinterfaceimpl.cpp \
		languageinterfaceimpl.cpp \
		common.cpp

HEADERS		= gui_qt_widget.h \
		  editorinterfaceimpl.h \
		  languageinterfaceimpl.h

INCLUDEPATH	+= proto/
DEFINES		+= HAVE_CONFIG_H USE_GUI_QT
LIBS		+= -lncurses
		
TARGET		= qvim
DESTDIR		= ../../../../plugins/designer
VERSION		= 1.0.0
unix:LIBS	+= -leditor
INCLUDEPATH	+= $(QTDIR)/src/kernel $(QTDIR)/tools/designer/interfaces $(QTDIR)/tools/designer/editor

target.path=$$plugins.path
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/designer
INSTALLS += target
