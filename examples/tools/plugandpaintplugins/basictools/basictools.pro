TEMPLATE      = lib
CONFIG       += plugin
INCLUDEPATH  += ../..
HEADERS       = basictoolsplugin.h
SOURCES       = basictoolsplugin.cpp
TARGET        = pnp_basictools
DESTDIR       = ../../plugandpaint/plugins

# install
target.path = $$[QT_INSTALL_DATA]/examples/tools/plugandpaintplugins/basictools
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS basictools.pro
sources.path = $$[QT_INSTALL_DATA]/examples/tools/plugandpaintplugins/basictools
INSTALLS += target sources
