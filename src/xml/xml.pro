# Qt xml module

REQUIRES = !qt_one_lib
TARGET		= qxml
QCONFIG = core gui

DEFINES += QT_BUILD_XML_LIB

PRECOMPILED_HEADER = ../gui/base/qt_gui_pch.h

include(../qbase.pri)

xml {
	HEADERS += qxml.h qdom.h qsvgdevice_p.h
	SOURCES += qxml.cpp qdom.cpp qsvgdevice.cpp
	win32-borland {
		QMAKE_CFLAGS_WARN_ON	+= -w-use
		QMAKE_CXXFLAGS_WARN_ON	+= -w-use
	}
}
