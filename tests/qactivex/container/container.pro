TEMPLATE	= lib
CONFIG		+= qt warn_on
HEADERS		= qaxbase.h qaxwidget.h qaxobject.h ../shared/types.h
SOURCES		= qaxbase.cpp qaxwidget.cpp qaxobject.cpp ../shared/types.cpp
FORMS		= qactivexselect.ui

DESTDIR		= $$QT_BUILD_TREE/lib
DLLDESTDIR	= $$QT_BUILD_TREE/bin
TARGET		= qaxcontainer

shared {
    CONFIG	+= plugin
    SOURCES	+= plugin.cpp
    DLLDESTDIR	+= $$QT_BUILD_TREE/plugins/designer
    INCLUDEPATH	+= $$QT_SOURCE_TREE/tools/designer/interfaces
}

win32-borland:INCLUDEPATH += $(BCB)/include/Atl
