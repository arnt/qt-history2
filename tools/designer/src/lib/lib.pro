TEMPLATE=lib
TARGET=QtDesigner
QT += xml
CONFIG += qt debug_and_release
DESTDIR = ../../../../lib
DLLDESTDIR = ../../../../bin

isEmpty(QT_MAJOR_VERSION) {
   VERSION=4.1.0
} else {
   VERSION=$${QT_MAJOR_VERSION}.$${QT_MINOR_VERSION}.$${QT_PATCH_VERSION}
}
QMAKE_TARGET_COMPANY = Trolltech AS
QMAKE_TARGET_PRODUCT = Designer
QMAKE_TARGET_DESCRIPTION = Graphical user interface designer.
QMAKE_TARGET_COPYRIGHT = Copyright (c) 2003-2005 Trolltech

!contains(CONFIG, static) {
    CONFIG += dll

    DEFINES += \
        QT_SDK_LIBRARY \
        QT_EXTENSION_LIBRARY \
        QT_UILIB_LIBRARY \
        QT_SHARED_LIBRARY \
        QT_PROPERTYEDITOR_LIBRARY
} else {
    DEFINES += QT_DESIGNER_STATIC
}

include(extension/extension.pri)
include(sdk/sdk.pri)
include(uilib/uilib.pri)
include(shared/shared.pri)
PRECOMPILED_HEADER=lib_pch.h

### include(propertyeditor/propertyeditor.pri)

include(../components/component.pri)
include(../sharedcomponents.pri)

target.path=$$[QT_INSTALL_LIBS]
INSTALLS        += target

include($$QT_SOURCE_TREE/include/QtDesigner/headers.pri)
designer_headers.files = $$SYNCQT.HEADER_FILES $$SYNCQT.HEADER_CLASSES
designer_headers.path = $$[QT_INSTALL_HEADERS]/QtDesigner
INSTALLS        += designer_headers
