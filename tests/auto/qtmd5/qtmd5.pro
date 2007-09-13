include(../solutions.pri)

include($${SOLUTIONBASEDIR}/utils/qtmd5/src/qtmd5.pri)

load(qttest_p4)

SOURCES += tst_qtmd5.cpp


QT = core

DEFINES += QT_USE_USING_NAMESPACE

