REQUIRES = !qt_one_lib
TARGET		= qsqlkernel

include(qbase.pri)
QCONFIG = kernel gui
include($$SQL_CPP/qt_sql.pri)
