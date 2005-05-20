TEMPLATE      = subdirs
SUBDIRS       = basictools \
                extrafilters

# install
target.path = $$[QT_INSTALL_DATA]/examples/tools/plugandpaintplugins
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS plugandpaintplugins.pro
sources.path = $$[QT_INSTALL_DATA]/examples/tools/plugandpaintplugins
INSTALLS += target sources
