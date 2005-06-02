TARGET      = qaxwidget
TEMPLATE    = lib
DESTDIR 	= $$QT_BUILD_TREE/plugins/designer

CONFIG     += qt warn_on qaxcontainer plugin designer debug_and_release

CONFIG(debug, debug|release) {
    unix:TARGET = $$member(TARGET, 0)_debug
    else:TARGET = $$member(TARGET, 0)d
}

# Input
SOURCES += plugin.cpp \
    activeqt_extrainfo.cpp

HEADERS += activeqt_extrainfo.h

# install
target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target
