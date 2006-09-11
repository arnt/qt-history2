TEMPLATE = lib
CONFIG += plugin

LIBS += -lvgagl -lvga

TARGET = svgalibscreen
target.path = $$[QT_INSTALL_PLUGINS]/gfxdrivers
INSTALLS += target

HEADERS	= svgalibscreen.h 
SOURCES	= svgalibscreen.cpp \
          svgalibplugin.cpp
