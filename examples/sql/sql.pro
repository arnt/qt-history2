TEMPLATE      = subdirs
SUBDIRS       = cachedtable \
                querymodel \
                tablemodel \
                relationaltablemodel

# install
sources.files = connection.h sql.pro README
sources.path = $$[QT_INSTALL_DATA]/examples/sql
INSTALLS += target sources
