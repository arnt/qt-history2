TEMPLATE      = subdirs
SUBDIRS       = mousecalibration

# install
sources.files = README *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/embedded
INSTALLS += sources
