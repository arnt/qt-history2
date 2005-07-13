TARGET = tslibmousehandler
include(../../qpluginbase.pri)

DESTDIR  = $$QT_BUILD_TREE/plugins/mousedrivers
DEFINES += QT_QWS_TSLIB

HEADERS = tslibmousedriverplugin.h tslibmousehandler.h
SOURCES = tslibmousedriverplugin.cpp tslibmousehandler.cpp

LIBS   += -lts

target.path=$$plugins.path/mousedrivers
INSTALL += target
