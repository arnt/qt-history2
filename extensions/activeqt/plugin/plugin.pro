GUID 	 = {53a64bf3-48e7-43e8-b046-c3a61c4ed9a1}
TEMPLATE = lib
TARGET   = qaxwidget

CONFIG  += plugin
LIBS	+= -lqaxcontainer
VERSION  = 1.0.0
DESTDIR  = $$QT_BUILD_TREE/plugins/designer
INCLUDEPATH += $$QT_SOURCE_TREE/tools/designer/interfaces

REQUIRES   = shared

!contains( QT_PRODUCT, qt-(enterprise|internal) ) {
    message( "ActiveQt requires a Qt/Enterprise license." )
}

SOURCES  = plugin.cpp
