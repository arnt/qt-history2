TEMPLATE = subdirs
SUBDIRS += qtcore qtgui

contains(QT_CONFIG, qdbus): SUBDIRS += qtdbus

scripts.files = script/*
scripts.path = $$[QT_INSTALL_PLUGINS]/script
INSTALLS += scripts
