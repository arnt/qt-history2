load(qttest_p4)

SOURCES += tst_qtwidgets.cpp mainwindow.cpp
HEADERS += mainwindow.h
QT += network
RESOURCES = qtstyles.qrc
FORMS += advanced.ui system.ui standard.ui

DEFINES += QT_USE_USING_NAMESPACE

