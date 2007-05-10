TEMPLATE = lib
CONFIG += plugin

LIBS += -L/usr/local/ahi/lib -lahi -lahioem
INCLUDEPATH += /usr/local/ahi/include

TARGET = qahiscreen
target.path = $$[QT_INSTALL_PLUGINS]/gfxdrivers
INSTALLS += target

HEADERS	= qscreenahi_qws.h 
SOURCES	= qscreenahi_qws.cpp \
          qscreenahiplugin.cpp

