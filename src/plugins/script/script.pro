TEMPLATE = subdirs
SUBDIRS += qtcore qtgui qtxml

scripts.files = script/*
scripts.path = $$[QT_INSTALL_PLUGINS]/script
INSTALLS += scripts
