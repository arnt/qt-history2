TEMPLATE      = subdirs
SUBDIRS       = dombookmarks \
                saxbookmarks

# install
target.path = $$[QT_INSTALL_DATA]/examples/xml
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS xml.pro README
sources.path = $$[QT_INSTALL_DATA]/examples/xml
INSTALLS += target sources
