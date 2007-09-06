TARGET     = QtXml
QPRO_PWD   = $$PWD
QT         = core
DEFINES   += QT_BUILD_XML_LIB
win32-msvc*|win32-icc:QMAKE_LFLAGS += /BASE:0x61000000

include(../qbase.pri)

PRECOMPILED_HEADER = ../corelib/global/qt_pch.h

HEADERS += qxmlutils_p.h
SOURCES += qxmlutils_p.cpp
win32-borland {
        QMAKE_CFLAGS_WARN_ON        += -w-use
        QMAKE_CXXFLAGS_WARN_ON        += -w-use
}

include(dom/dom.pri)
include(sax/sax.pri)
include(stream/stream.pri)

# Patternist use member templates
!win32-msvc: include(query/query.pri)
