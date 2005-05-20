TEMPLATE      = subdirs
SUBDIRS       = cachedtable \
                querymodel \
                tablemodel \
                relationaltablemodel

# install
target.path = $$[QT_INSTALL_DATA]/examples/sql
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS sql.pro README
sources.path = $$[QT_INSTALL_DATA]/examples/sql
INSTALLS += target sources
