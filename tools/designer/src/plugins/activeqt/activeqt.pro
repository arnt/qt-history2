TARGET      = qaxwidget
TEMPLATE    = lib
DESTDIR 	= $$QT_BUILD_TREE/plugins/designer

CONFIG     += qt warn_on qaxcontainer plugin designer debug_and_release

CONFIG(debug, debug|release) {
    unix:TARGET = $$member(TARGET, 0)_debug
    else:TARGET = $$member(TARGET, 0)d
}

INCLUDEPATH += $$QT_SOURCE_TREE/src/activeqt/shared/

# Input
SOURCES += plugin.cpp \
    activeqt_extrainfo.cpp \
    $$QT_SOURCE_TREE/src/activeqt/shared/qaxtypes.cpp

HEADERS += activeqt_extrainfo.h \
    $$QT_SOURCE_TREE/src/activeqt/shared/qaxtypes.h

# install
target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target
