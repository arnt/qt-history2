TEMPLATE    = subdirs
SUBDIRS     = draggableicons \
              draggabletext \
              dropsite \
              fridgemagnets \
              puzzle

# install
sources.files = README *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/draganddrop
INSTALLS += sources
DEFINES += QT_USE_USING_NAMESPACE
