TEMPLATE      = subdirs
SUBDIRS       = basicdrawing \
                concentriccircles \
                painterpaths \
                transformations

# install
target.path = $$[QT_INSTALL_DATA]/examples/painting
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS painting.pro README
sources.path = $$[QT_INSTALL_DATA]/examples/painting
INSTALLS += target sources
