TEMPLATE = app
LANGUAGE = C++
TARGET         = assistant

CONFIG        += qt warn_on
QT += compat xml network

PROJECTNAME        = Assistant
DESTDIR            = ../../bin

FORMS += finddialog.ui \
        helpdialog.ui \
        mainwindow.ui \
        settingsdialog.ui \
        tabbedbrowser.ui \
        topicchooser.ui

SOURCES += main.cpp \
        helpwindow.cpp \
        topicchooser.cpp \
        docuparser.cpp \
        settingsdialog.cpp \
        index.cpp \
        profile.cpp \
        config.cpp \
        finddialog.cpp \
        helpdialog.cpp \
        mainwindow.cpp \
        tabbedbrowser.cpp

HEADERS        += helpwindow.h \
        topicchooser.h \
        docuparser.h \
        settingsdialog.h \
        index.h \
        profile.h \
        finddialog.h \
        helpdialog.h \
        mainwindow.h \
        tabbedbrowser.h \
        config.h

RESOURCES += assistant.qrc

DEFINES += QT_KEYWORDS
#DEFINES +=  QT_PALMTOPCENTER_DOCS
!network:DEFINES        += QT_INTERNAL_NETWORK
else:QT += network
!xml: DEFINES                += QT_INTERNAL_XML
else:QT += xml
include( ../../src/qt_professional.pri )

win32 {
    LIBS += shell32.lib
    RC_FILE = assistant.rc
}

mac:ICON = assistant.icns

#target.path = $$bins.path
#INSTALLS += target

#assistanttranslations.files = *.qm
#assistanttranslations.path = $$translations.path
#INSTALLS += assistanttranslations

TRANSLATIONS        = assistant_de.ts \
                  assistant_fr.ts

unix:!zlib:LIBS        += -lz

