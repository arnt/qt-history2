TEMPLATE      = lib
CONFIG       += plugin
INCLUDEPATH  += ../..
HEADERS       = extrafiltersplugin.h
SOURCES       = extrafiltersplugin.cpp
TARGET        = pnp_extrafilters
DESTDIR       = ../../plugandpaint/plugins

contains(TEMPLATE,lib) {
   CONFIG(debug, debug|release) {
      mac:TARGET = $$member(TARGET, 0)_debug
      win32:TARGET = $$member(TARGET, 0)d
   }
}

# install
target.path = $$[QT_INSTALL_EXAMPLES]/tools/plugandpaint/plugins
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS extrafilters.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/tools/plugandpaintplugins/extrafilters
INSTALLS += target sources
