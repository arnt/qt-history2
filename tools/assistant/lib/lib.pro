TEMPLATE        = lib
QT += network
TARGET                = QtAssistantClient
VERSION                = 1.0

CONFIG                += qt warn_on
CONFIG                += debug_and_release
mac:unix:CONFIG       += explicitlib
CONFIG                -= dll

HEADERS         = qassistantclient.h
SOURCES         = qassistantclient.cpp

DESTDIR                = ../../../lib

unix {
        QMAKE_CFLAGS += $$QMAKE_CFLAGS_SHLIB
        QMAKE_CXXFLAGS += $$QMAKE_CXXFLAGS_SHLIB
}

#load up the headers info
CONFIG += qt_install_headers
HEADERS_PRI = $$QT_BUILD_TREE/include/QtAssistant/headers.pri
include($$HEADERS_PRI)|clear(HEADERS_PRI)

#mac frameworks
mac:!static:contains(QT_CONFIG, qt_framework) {
   QMAKE_FRAMEWORK_BUNDLE_NAME = $$TARGET
   CONFIG += lib_bundle qt_no_framework_direct_includes qt_framework
   CONFIG(debug, debug|release) {
      !build_pass:CONFIG += build_all
   } else { #release
      !debug_and_release|build_pass {
	  CONFIG -= qt_install_headers #no need to install these as well
	  FRAMEWORK_HEADERS.version = Versions
	  FRAMEWORK_HEADERS.files = $$SYNCQT.HEADER_FILES $$SYNCQT.HEADER_CLASSES
      	  FRAMEWORK_HEADERS.path = Headers
      }
      QMAKE_BUNDLE_DATA += FRAMEWORK_HEADERS
   }
}

!debug_and_release|build_pass {
   CONFIG(debug, debug|release) {
      unix:TARGET = $$member(TARGET, 0)_debug
      else:TARGET = $$member(TARGET, 0)d
   }
}

target.path=$$[QT_INSTALL_LIBS]
INSTALLS        += target

qt_install_headers {
    assistant_headers.files = $$SYNCQT.HEADER_FILES $$SYNCQT.HEADER_CLASSES
    assistant_headers.path = $$[QT_INSTALL_HEADERS]/QtAssistant
    INSTALLS        += assistant_headers
}
