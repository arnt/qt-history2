TARGET          = QtCanvas
QPRO_PWD        = $$PWD
include(../qbase.pri)

DEFINES += QT_BUILD_CANVAS_LIB
QT = core gui

HEADERS += qcanvas.h
SOURCES += qcanvas.cpp
