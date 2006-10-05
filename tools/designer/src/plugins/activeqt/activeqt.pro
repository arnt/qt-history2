TARGET      = $$qtLibraryTarget(qaxwidget)
TEMPLATE    = lib
QTDIR_build:DESTDIR 	= $$QT_BUILD_TREE/plugins/designer

CONFIG     += qt warn_on qaxcontainer plugin designer debug_and_release

INCLUDEPATH += $$QT_SOURCE_TREE/src/activeqt/shared/ ../../lib/uilib

# Input
SOURCES += plugin.cpp \
    activeqt_extrainfo.cpp \
    $$QT_SOURCE_TREE/src/activeqt/shared/qaxtypes.cpp

HEADERS += activeqt_extrainfo.h \
    $$QT_SOURCE_TREE/src/activeqt/shared/qaxtypes.h

# install
target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target
