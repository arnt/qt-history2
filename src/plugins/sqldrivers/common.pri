TEMPLATE = lib

CONFIG  += plugin
QCONFIG  = core sql
DESTDIR  = $$QT_BUILD_TREE/plugins/sqldrivers

QTDIR_build:REQUIRES    = sql

target.path     += $$plugins.path/sqldrivers
INSTALLS        += target

DEFINES += QT_NO_CAST_TO_ASCII
