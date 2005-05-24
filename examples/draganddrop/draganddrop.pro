TEMPLATE    = subdirs
SUBDIRS     = draggableicons \
              draggabletext \
              fridgemagnets \
              puzzle

# install
sources.files = README *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/draganddrop
INSTALLS += sources
