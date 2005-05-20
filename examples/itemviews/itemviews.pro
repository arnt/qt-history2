TEMPLATE      = subdirs
SUBDIRS       = chart \
                dirview \
                pixelator \
                simpletreemodel \
                spinboxdelegate

# install
sources.files = README *.pro
sources.path = $$[QT_INSTALL_DATA]/examples/itemviews
INSTALLS += sources
