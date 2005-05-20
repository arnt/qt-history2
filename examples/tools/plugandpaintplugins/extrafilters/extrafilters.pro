TEMPLATE      = lib
CONFIG       += plugin
INCLUDEPATH  += ../..
HEADERS       = extrafiltersplugin.h
SOURCES       = extrafiltersplugin.cpp
TARGET        = pnp_extrafilters
DESTDIR       = ../../plugandpaint/plugins

# install
target.path = $$[QT_INSTALL_DATA]/examples/tools/plugandpaintplugins/extrafilters
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS extrafilters.pro
sources.path = $$[QT_INSTALL_DATA]/examples/tools/plugandpaintplugins/extrafilters
INSTALLS += target sources
