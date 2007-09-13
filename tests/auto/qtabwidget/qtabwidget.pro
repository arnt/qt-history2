load(qttest_p4)

INCLUDEPATH += ../

HEADERS +=  
SOURCES += tst_qtabwidget.cpp 

win32:LIBS += -luser32

###QT += qt3support

DEFINES += QT_USE_USING_NAMESPACE

