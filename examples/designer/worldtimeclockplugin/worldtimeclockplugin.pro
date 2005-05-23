CONFIG      += designer plugin
QT          += xml
TEMPLATE    = lib
DESTDIR     = $$QT_BUILD_TREE/plugins/designer

HEADERS     = worldtimeclock.h \
              worldtimeclockplugin.h
SOURCES     = worldtimeclock.cpp \
              worldtimeclockplugin.cpp

# install
target.path = $$[QT_INSTALL_PLUGINS]/designer
sources.files = $$SOURCES $$HEADERS *.pro
sources.path = $$[QT_INSTALL_DATA]/examples/designer/worldtimeclockplugin
INSTALLS += target sources
