TARGET      = qaxwidget
TEMPLATE    = lib
DESTDIR     = $$QT_BUILD_TREE/plugins/designer

# Input
SOURCES += plugin.cpp

CONFIG += qt warn_on qaxcontainer
CONFIG += designer plugin

# install
target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target
