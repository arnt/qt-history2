# Qt project file
TEMPLATE	= lib
TARGET		= qt
VERSION		= 3.0.0
DESTDIR		= $$QMAKE_LIBDIR_QT
DLLDESTDIR	= ../bin

CONFIG		+= qt warn_on release

win32 {
	CONFIG	+= png zlib
	CONFIG -= jpeg
	DEFINES += QT_NO_IMAGEIO_JPEG UNICODE
	INCLUDEPATH      += tmp
	MOC_DIR	  = tmp
	OBJECTS_DIR = tmp
	DEPENDPATH = ../include
}
win32-borland:INCLUDEPATH += kernel

unix {
	CONFIG	   += x11 x11inc
	DEFINES    += QT_FATAL_ASSERT
	LIBS += -ldl
}

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

include(kernel/qt_compat.pri):
include(kernel/qt_gfx.pri):
include(kernel/qt_embedded.pri):
include(tools/qt_tools.pri):
include(kernel/qt_kernel.pri):
include(widgets/qt_widgets.pri):
include(dialogs/qt_dialogs.pri):
include(iconview/qt_iconview.pri):
include(workspace/qt_workspace.pri):
include(network/qt_network.pri):
include(canvas/qt_canvas.pri):
include(table/qt_table.pri):
include(xml/qt_xml.pri):
include(opengl/qt_opengl.pri):
include(sql/qt_sql.pri):




