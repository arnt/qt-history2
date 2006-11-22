INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
SOURCES += $$PWD/qfiledialog.cpp $$PWD/qfileinfogatherer.cpp $$PWD/qfilesystemmodel.cpp $$PWD/qsidebar.cpp

HEADERS += $$PWD/qfiledialog.h $$PWD/qfiledialog_p.h $$PWD/qfileinfogatherer_p.h $$PWD/qfilesystemmodel_p.h $$PWD/qsidebar_p.h

!embedded:mac:SOURCES	+= $$PWD/qfiledialog_mac.cpp

win32 {
    SOURCES += $$PWD/qfiledialog_win.cpp
    !win32-borland:LIBS += -lshell32 -lcomdlg32 # the filedialog needs this library
}

