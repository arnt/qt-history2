QT_SOURCE_TREE=$$fromfile($(QTDIR)/.qmake.cache,QT_SOURCE_TREE)
QT_BUILD_TREE=$$fromfile($(QTDIR)/.qmake.cache,QT_BUILD_TREE)
QT_INSTALL_PLUGINS=$$fromfile($(QTDIR)/.qmake.cache,QT_INSTALL_PLUGINS)

include($$QT_SOURCE_TREE/src/plugins/qpluginbase.pri)
