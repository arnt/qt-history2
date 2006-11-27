TEMPLATE      = subdirs
SUBDIRS       = basicsortfiltermodel \
                chart \
                customsortfiltermodel \
                dirview \
                pixelator \
                puzzle \
                simpledommodel \
                simpletreemodel \
                spinboxdelegate \
                stardelegate

# install
sources.files = README *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/itemviews
INSTALLS += sources
