# Qt project file
TEMPLATE	= lib
TARGET		= qt
VERSION		= 3.0.0
DESTDIR		= $$QMAKE_LIBDIR_QT
DLLDESTDIR	= ../bin

CONFIG		+= qt warn_on release

KERNEL_CPP	= kernel	
CANVAS_CPP      = canvas
WIDGETS_CPP	= widgets
SQL_CPP	        = sql
TABLE_CPP	= table
DIALOGS_CPP	= dialogs
ICONVIEW_CPP	= iconview
NETWORK_CPP	= network
OPENGL_CPP	= opengl
TOOLS_CPP	= tools
WORKSPACE_CPP	= workspace
XML_CPP	        = xml
STYLES_CPP	= styles

win32 {
	WIN_ALL_H = ../include
	SQL_H	        = $$WIN_ALL_H
	KERNEL_H	= $$WIN_ALL_H
	WIDGETS_H	= $$WIN_ALL_H
	TABLE_H	        = $$WIN_ALL_H
	DIALOGS_H	= $$WIN_ALL_H
	ICONVIEW_H	= $$WIN_ALL_H
	NETWORK_H	= $$WIN_ALL_H
	OPENGL_H	= $$WIN_ALL_H
	TOOLS_H	        = $$WIN_ALL_H
	WORKSPACE_H	= $$WIN_ALL_H
	XML_H	        = $$WIN_ALL_H
	CANVAS_H	= $$WIN_ALL_H
	STYLES_H	= $$WIN_ALL_H

	CONFIG	+= png zlib
	CONFIG -= jpeg
	DEFINES += UNICODE
	INCLUDEPATH      += tmp
	MOC_DIR	  = tmp
	OBJECTS_DIR = tmp
}
win32-borland:INCLUDEPATH += kernel

unix {
	CANVAS_H	= $$CANVAS_CPP
	KERNEL_H	= $$KERNEL_CPP
	WIDGETS_H	= $$WIDGETS_CPP
	SQL_H	        = $$SQL_CPP
	TABLE_H	        = $$TABLE_CPP
	DIALOGS_H	= $$DIALOGS_CPP
	ICONVIEW_H	= $$ICONVIEW_CPP
	NETWORK_H	= $$NETWORK_CPP
	OPENGL_H	= $$OPENGL_CPP
	TOOLS_H	        = $$TOOLS_CPP
	WORKSPACE_H	= $$WORKSPACE_CPP
	XML_H	        = $$XML_CPP
	STYLES_H	= $$STYLES_CPP

	CONFIG	   += x11 x11inc
	DEFINES    += QT_FATAL_ASSERT
	!macx:LIBS += -ldl
}

DEPENDPATH += :$$NETWORK_H:$$KERNEL_H:$$WIDGETS_H:$$SQL_H:$$TABLE_H:$$DIALOGS_H:
DEPENDPATH += $$ICONVIEW_H:$$OPENGL_H:$$TOOLS_H:$$WORKSPACE_H:$$XML_H:$$CANVAS_H:$$STYLES_H

thread {
	TARGET = qt-mt
	DEFINES += QT_THREAD_SUPPORT
}

nas {
	DEFINES     += QT_NAS_SUPPORT
	LIBS	+= -laudio -lXt
}

!x11sm:DEFINES += QT_NO_SM_SUPPORT

cups {
	# next few lines add cups support
	DEFINES += QT_CUPS_SUPPORT
	LIBS += -lcups
}

include($$KERNEL_CPP/qt_compat.pri)
include($$KERNEL_CPP/qt_embedded.pri)
include($$KERNEL_CPP/qt_kernel.pri)
include($$WIDGETS_CPP/qt_widgets.pri)
include($$DIALOGS_CPP/qt_dialogs.pri)
include($$ICONVIEW_CPP/qt_iconview.pri)
include($$WORKSPACE_CPP/qt_workspace.pri)
include($$NETWORK_CPP/qt_network.pri)
include($$CANVAS_CPP/qt_canvas.pri)
include($$TABLE_CPP/qt_table.pri)
include($$XML_CPP/qt_xml.pri)
include($$OPENGL_CPP/qt_opengl.pri)
include($$SQL_CPP/qt_sql.pri)
include($$KERNEL_CPP/qt_gfx.pri)
include($$TOOLS_CPP/qt_tools.pri)
include($$STYLES_CPP/qt_styles.pri)
