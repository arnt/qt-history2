TEMPLATE      = lib
CONFIG       += plugin static
INCLUDEPATH  += ../..
HEADERS       = basictoolsplugin.h
SOURCES       = basictoolsplugin.cpp
TARGET        = $$qtLibraryTarget(pnp_basictools)
DESTDIR       = ../../plugandpaint/plugins

# install
target.path = $$[QT_INSTALL_EXAMPLES]/tools/plugandpaint/plugins
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS basictools.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/tools/plugandpaintplugins/basictools
INSTALLS += target sources
