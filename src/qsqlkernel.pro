REQUIRES = !qt_one_lib
TARGET		= qsqlkernel
QCONFIG = kernel gui

DEFINES += QT_BUILD_SQL_LIB

include(qbase.pri)
include($$SQL_CPP/qt_sql.pri)
