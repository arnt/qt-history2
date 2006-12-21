TEMPLATE      = subdirs
SUBDIRS       = cachedtable \
                drilldown \
                querymodel \
                tablemodel \
                relationaltablemodel

# install
sources.files = connection.h sql.pro README
sources.path = $$[QT_INSTALL_EXAMPLES]/sql
INSTALLS += sources
