TEMPLATE = lib

CONFIG  += plugin console
CONFIG  -= opengl x11sm
QCONFIG  = core sql
DESTDIR  = $$QT_BUILD_TREE/plugins/sqldrivers

target.path     += $$plugins.path/sqldrivers
INSTALLS        += target

DEFINES += QT_NO_CAST_TO_ASCII
