TARGET		= QtXml
QPRO_PWD        = $$PWD
QT = core gui

DEFINES += QT_BUILD_XML_LIB

PRECOMPILED_HEADER = ../gui/kernel/qt_gui_pch.h

include(../qbase.pri)

HEADERS += qxml.h qdom.h qpaintengine_svg_p.h
SOURCES += qxml.cpp qdom.cpp qpaintengine_svg.cpp
win32-borland {
        QMAKE_CFLAGS_WARN_ON	+= -w-use
        QMAKE_CXXFLAGS_WARN_ON	+= -w-use
}
