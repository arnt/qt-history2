# Qt project file
TEMPLATE	= lib
TARGET		= qt
embedded:TARGET	= qte
VERSION		= 3.0.0
DESTDIR		= $$QMAKE_LIBDIR_QT
DLLDESTDIR	= ../bin

CONFIG		+= qt warn_on

win32-borland {
	mng:QMAKE_CFLAGS_WARN_ON	+= -w-par
	mng:QMAKE_CXXFLAGS_WARN_ON	+= -w-par
}

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
CODECS_CPP	= codecs
WORKSPACE_CPP	= workspace
XML_CPP	        = xml
STYLES_CPP	= styles

win32 {
	internal {
		SQL_H		= $$SQL_CPP
		KERNEL_H	= $$KERNEL_CPP
		WIDGETS_H	= $$WIDGETS_CPP
		TABLE_H		= $$TABLE_CPP
		DIALOGS_H	= $$DIALOGS_CPP
		ICONVIEW_H	= $$ICONVIEW_CPP
		NETWORK_H	= $$NETWORK_CPP
		OPENGL_H	= $$OPENGL_CPP
		TOOLS_H		= $$TOOLS_CPP
		CODECS_H	= $$CODECS_CPP
		WORKSPACE_H	= $$WORKSPACE_CPP
		XML_H		= $$XML_CPP
		CANVAS_H	= $$CANVAS_CPP
		STYLES_H	= $$STYLES_CPP
	} 
	!internal {
		WIN_ALL_H = ../include
		SQL_H		= $$WIN_ALL_H
		KERNEL_H	= $$WIN_ALL_H
		WIDGETS_H	= $$WIN_ALL_H
		TABLE_H		= $$WIN_ALL_H
		DIALOGS_H	= $$WIN_ALL_H
		ICONVIEW_H	= $$WIN_ALL_H
		NETWORK_H	= $$WIN_ALL_H
		OPENGL_H	= $$WIN_ALL_H
		TOOLS_H		= $$WIN_ALL_H
		CODECS_H	= $$WIN_ALL_H
		WORKSPACE_H	= $$WIN_ALL_H
		XML_H		= $$WIN_ALL_H
		CANVAS_H	= $$WIN_ALL_H
		STYLES_H	= $$WIN_ALL_H
	}

	CONFIG	+= zlib
	CONFIG -= incremental
	DEFINES += UNICODE
	INCLUDEPATH += tmp
	MOC_DIR	= tmp
	OBJECTS_DIR = tmp
	dll:DEFINES+=QT_MAKEDLL

}
win32-borland:INCLUDEPATH += kernel

unix {
	CANVAS_H	= $$CANVAS_CPP
	KERNEL_H	= $$KERNEL_CPP
	WIDGETS_H	= $$WIDGETS_CPP
	SQL_H		= $$SQL_CPP
	TABLE_H		= $$TABLE_CPP
	DIALOGS_H	= $$DIALOGS_CPP
	ICONVIEW_H	= $$ICONVIEW_CPP
	NETWORK_H	= $$NETWORK_CPP
	OPENGL_H	= $$OPENGL_CPP
	TOOLS_H		= $$TOOLS_CPP
	CODECS_H	= $$CODECS_CPP
	WORKSPACE_H	= $$WORKSPACE_CPP
	XML_H		= $$XML_CPP
	STYLES_H	= $$STYLES_CPP
	!mac:CONFIG	   += x11 x11inc
}

DEPENDPATH += ;$$NETWORK_H;$$KERNEL_H;$$WIDGETS_H;$$SQL_H;$$TABLE_H;$$DIALOGS_H; 
DEPENDPATH += $$ICONVIEW_H;$$OPENGL_H;$$TOOLS_H;$$CODECS_H;$$WORKSPACE_H;$$XML_H;
DEPENDPATH += $$CANVAS_H;$$STYLES_H

thread {
	!win32-borland:TARGET = qt-mt
	win32-borland:TARGET = qtmt
	embedded:TARGET = qte-mt
	DEFINES += QT_THREAD_SUPPORT
}

cups {
	# next few lines add cups support
	DEFINES += QT_CUPS_SUPPORT
	LIBS += -lcups
}

#here for compatability, should go away ####
include($$KERNEL_CPP/qt_compat.pri)

#platforms
x11:include($$KERNEL_CPP/qt_x11.pri)
mac:include($$KERNEL_CPP/qt_mac.pri)
embedded:include($$KERNEL_CPP/qt_qws.pri)

#modules
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
include($$CODECS_CPP/qt_codecs.pri)
include($$STYLES_CPP/qt_styles.pri)

#install directives
include(qt_install.pri)
