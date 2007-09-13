TEMPLATE      = subdirs
SUBDIRS       = arrowpad \
                hellotr \
                trollprint

# install
sources.files = README *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/linguist
INSTALLS += sources
DEFINES += QT_USE_USING_NAMESPACE
