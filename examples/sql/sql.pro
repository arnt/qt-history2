TEMPLATE      = subdirs
unset(EXAMPLES_SQL_SUBDIRS)
EXAMPLES_SQL_SUBDIRS = examples_sql_cachedtable \
                       examples_sql_querymodel \
                       examples_sql_tablemodel \
                       examples_sql_relationaltablemodel

# install
EXAMPLES_SQL_install_sources.files = connection.h sql.pro README
EXAMPLES_SQL_install_sources.path = $$[QT_INSTALL_EXAMPLES]/sql
INSTALLS += EXAMPLES_SQL_install_sources

#subdirs
examples_sql_cachedtable.subdir = $$QT_BUILD_TREE/examples/sql/cachedtable
examples_sql_cachedtable.depends =  src_corelib src_gui src_sql
examples_sql_querymodel.subdir = $$QT_BUILD_TREE/examples/sql/querymodel
examples_sql_querymodel.depends =  src_corelib src_gui src_sql
examples_sql_tablemodel.subdir = $$QT_BUILD_TREE/examples/sql/tablemodel
examples_sql_tablemodel.depends =  src_corelib src_gui src_sql
examples_sql_relationaltablemodel.subdir = $$QT_BUILD_TREE/examples/sql/relationaltablemodel
examples_sql_relationaltablemodel.depends =  src_corelib src_gui src_sql
EXAMPLES_SUB_SUBDIRS += $$EXAMPLES_SQL_SUBDIRS
SUBDIRS += $$EXAMPLES_SQL_SUBDIRS
