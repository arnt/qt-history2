TARGET		= Qt3Compat
QPRO_PWD        = $$PWD
QT              = core gui network sql
include(../qbase.pri)

DEFINES += QT_BUILD_COMPAT_LIB 

include(tools/tools.pri)
include(sql/sql.pri)
include(other/other.pri)
include(text/text.pri)
include(itemviews/itemviews.pri)
include(widgets/widgets.pri)
include(dialogs/dialogs.pri)

mac:LIBS += -framework Carbon

QMAKE_LIBS += $$QMAKE_LIBS_COMPAT
DEFINES -= QT_COMPAT_WARNINGS
DEFINES += QT_COMPAT