TEMPLATE      = subdirs
SUBDIRS       = chart \
                dirview \
                pixelator \
                puzzle \
                simpledommodel \
                simpletreemodel \
                sortingmodel \
                spinboxdelegate

# install
sources.files = README *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/itemviews
INSTALLS += sources
