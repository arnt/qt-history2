TEMPLATE = subdirs
SUBDIRS += qtcore qtgui

scripts.files = script/*
scripts.path = $$[QT_INSTALL_PLUGINS]/script
INSTALLS += scripts
