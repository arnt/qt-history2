TEMPLATE      = lib
CONFIG       += plugin
INCLUDEPATH  += ../..
HEADERS       = extrafiltersplugin.h
SOURCES       = extrafiltersplugin.cpp
TARGET        = $$qtLibraryTarget(pnp_extrafilters)
DESTDIR       = ../../plugandpaint/plugins

# install
target.path = $$[QT_INSTALL_EXAMPLES]/tools/plugandpaint/plugins
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS extrafilters.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/tools/plugandpaintplugins/extrafilters
INSTALLS += target sources
DEFINES += QT_USE_USING_NAMESPACE
