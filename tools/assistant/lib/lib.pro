TEMPLATE        = lib
TARGET                = qassistantclient
VERSION                = 1.0

CONFIG                += qt warn_on
CONFIG                += staticlib
CONFIG                -= dll

HEADERS         = qassistantclient.h
SOURCES         = qassistantclient.cpp

QT += network
include( ../../../src/qt_professional.pri )

DESTDIR                = ../../../lib

unix {
        QMAKE_CFLAGS += $$QMAKE_CFLAGS_SHLIB
        QMAKE_CXXFLAGS += $$QMAKE_CXXFLAGS_SHLIB
        target.path=$$libs.path
        INSTALLS        += target
}

!debug_and_release|build_pass {
   CONFIG(debug, debug|release) {
      unix:TARGET = $$member(TARGET, 0)_debug
      else:TARGET = $$member(TARGET, 0)d
   }
}
