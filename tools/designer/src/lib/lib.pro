TEMPLATE=lib
TARGET=QtDesigner
QT += xml
contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols
CONFIG += qt
win32|mac: CONFIG += debug_and_release
DESTDIR = ../../../../lib
DLLDESTDIR = ../../../../bin

isEmpty(QT_MAJOR_VERSION) {
   VERSION=4.3.0
} else {
   VERSION=$${QT_MAJOR_VERSION}.$${QT_MINOR_VERSION}.$${QT_PATCH_VERSION}
}
QMAKE_TARGET_COMPANY = Trolltech ASA
QMAKE_TARGET_PRODUCT = Designer
QMAKE_TARGET_DESCRIPTION = Graphical user interface designer.
QMAKE_TARGET_COPYRIGHT = Copyright (C) 2003-2006 Trolltech ASA

!contains(CONFIG, static) {
    CONFIG += dll

    DEFINES += \
        QDESIGNER_SDK_LIBRARY \
        QDESIGNER_EXTENSION_LIBRARY \
        QDESIGNER_UILIB_LIBRARY \
        QDESIGNER_SHARED_LIBRARY
} else {
    DEFINES += QT_DESIGNER_STATIC
}

#load up the headers info
CONFIG += qt_install_headers
HEADERS_PRI = $$QT_BUILD_TREE/include/QtDesigner/headers.pri
include($$HEADERS_PRI)|clear(HEADERS_PRI)

#mac frameworks
mac:CONFIG += explicitlib
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

include(extension/extension.pri)
include(sdk/sdk.pri)
include(uilib/uilib.pri)
include(shared/shared.pri)
PRECOMPILED_HEADER=lib_pch.h

include(../components/component.pri)
include(../sharedcomponents.pri)

target.path=$$[QT_INSTALL_LIBS]
INSTALLS        += target

qt_install_headers {
    designer_headers.files = $$SYNCQT.HEADER_FILES $$SYNCQT.HEADER_CLASSES
    designer_headers.path = $$[QT_INSTALL_HEADERS]/QtDesigner
    INSTALLS        += designer_headers
}
