TARGET	 = qgfxsnap
include(../../qpluginbase.pri)

DESTDIR	 = $$QT_BUILD_TREE/plugins/gfxdrivers

DEFINES	+= QT_QWS_VFB

HEADERS		= ../../../../include/Qt/qgfxsnap_qws.h
SOURCES		= main.cpp \
		  ../../../../src/gui/embedded/qgfxsnap_qws.cpp


target.path += $$[QT_INSTALL_PLUGINS]/gfxdrivers
INSTALLS += target
