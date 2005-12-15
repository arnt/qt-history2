# ====================================================
# Enter the location of tslib headers and libraries:

#TSLIB_INCLUDE = /usr/local/tslib/include
#TSLIB_LIBS = -L/usr/local/tslib/lib -lts

# ====================================================

isEmpty(TSLIB_INCLUDE) | isEmpty(TSLIB_LIBS) {
	warning(Please edit tslibmousehandler.pro to specify the location)
	warning(of tslib headers and libraries.)
	isEmpty(TSLIB_INCLUDE): error(Empty TSLIB_INCLUDE)
	isEmpty(TSLIB_LIBS): error(Empty TSLIB_LIBS)
}

TARGET = qtslibmousehandler
include(../../qpluginbase.pri)

DESTDIR  = $$QT_BUILD_TREE/plugins/mousedrivers
DEFINES += QT_QWS_TSLIB

HEADERS = tslibmousedriverplugin.h tslibmousehandler.h
SOURCES = tslibmousedriverplugin.cpp tslibmousehandler.cpp

INCLUDEPATH += $$TSLIB_INCLUDE
LIBS += $$TSLIB_LIBS

target.path=$$plugins.path/mousedrivers
INSTALL += target
