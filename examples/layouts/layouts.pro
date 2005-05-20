TEMPLATE      = subdirs
SUBDIRS       = basiclayouts \
                borderlayout \
                flowlayout

# install
sources.files = README *.pro
sources.path = $$[QT_INSTALL_DATA]/examples/layouts
INSTALLS += sources
