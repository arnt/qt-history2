REQUIRES   = shared
!contains( QT_PRODUCT, qt-(enterprise|internal) ) {
    message( "ActiveQt requires a Qt/Enterprise license." )
}

TEMPLATE   = lib
CONFIG    += plugin
VERSION    = 1.0.0
TARGET     = qaxwidget
DESTDIR    = $$QT_BUILD_TREE/plugins/designer
LIBS	  += -lqaxcontainer
INCLUDEPATH     += $$QT_SOURCE_TREE/tools/designer/interfaces

SOURCES  = plugin.cpp
