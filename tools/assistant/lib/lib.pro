TEMPLATE        = lib
QT += network
TARGET                = QtAssistantClient
VERSION                = 1.0

CONFIG                += qt warn_on
CONFIG                += staticlib debug_and_release
mac:unix:CONFIG       += explicitlib
CONFIG                -= dll

HEADERS         = qassistantclient.h
SOURCES         = qassistantclient.cpp

DESTDIR                = ../../../lib

unix {
        QMAKE_CFLAGS += $$QMAKE_CFLAGS_SHLIB
        QMAKE_CXXFLAGS += $$QMAKE_CXXFLAGS_SHLIB
}

target.path=$$[QT_INSTALL_LIBS]
INSTALLS        += target

!debug_and_release|build_pass {
   CONFIG(debug, debug|release) {
      unix:TARGET = $$member(TARGET, 0)_debug
      else:TARGET = $$member(TARGET, 0)d
   }
}

target.path=$$[QT_INSTALL_LIBS]
INSTALLS        += target

include($$QT_SOURCE_TREE/include/QtAssistant/headers.pri)
assistant_headers.files = $$SYNCQT.HEADER_FILES $$SYNCQT.HEADER_CLASSES
assistant_headers.path = $$[QT_INSTALL_HEADERS]/QtAssistant
INSTALLS        += assistant_headers
