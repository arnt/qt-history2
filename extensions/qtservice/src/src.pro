TEMPLATE	= lib
CONFIG		+= qt warn_on release
TARGET		= qtservice
DESTDIR         = $$QT_BUILD_TREE/lib
DLLDESTDIR      = $$QT_BUILD_TREE/bin

shared {
    CONFIG	+= dll
    DEFINES	+= QNTS_MAKEDLL
}

!contains( QT_PRODUCT, qt-(enterprise|internal) ) {
    message( "QtService requires a Qt/Enterprise license." )
}
contains( QT_PRODUCT, qt-(enterprise|internal) ) {
    HEADERS	= qtservice.h
    SOURCES	= qtservice.cpp
}
