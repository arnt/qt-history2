TEMPLATE = lib

CONFIG  += plugin console
QT  = core sql
DESTDIR  = $$QT_BUILD_TREE/plugins/sqldrivers

target.path     += $$plugins.path/sqldrivers
INSTALLS        += target

DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII
