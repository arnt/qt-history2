TEMPLATE      = subdirs
SUBDIRS       = dombookmarks \
                saxbookmarks \
                xmlstreamlint

# install
target.path = $$[QT_INSTALL_EXAMPLES]/xml
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS xml.pro README
sources.path = $$[QT_INSTALL_EXAMPLES]/xml
INSTALLS += target sources
