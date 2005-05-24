TEMPLATE      = subdirs
SUBDIRS       = basicdrawing \
                concentriccircles \
                painterpaths \
                transformations

# install
target.path = $$[QT_INSTALL_EXAMPLES]/painting
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS painting.pro README
sources.path = $$[QT_INSTALL_EXAMPLES]/painting
INSTALLS += target sources
