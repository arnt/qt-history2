TEMPLATE = lib
TARGET   = qaxwidget

CONFIG  += plugin qaxcontainer
VERSION  = 1.0.0
DESTDIR  = $$QT_BUILD_TREE/plugins/designer
INCLUDEPATH += $$QT_SOURCE_TREE/tools/designer/interfaces

REQUIRES   = shared

!contains( QT_PRODUCT, qt-(enterprise|internal) ) {
    message( "ActiveQt requires a Qt/Enterprise license." )
}

SOURCES  = plugin.cpp
