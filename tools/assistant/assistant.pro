TEMPLATE = app
LANGUAGE = C++
TARGET         = assistant

CONFIG        += qt warn_off
QT += compat xml

uic4.output  = ui_${QMAKE_FILE_BASE}.h
uic4.commands = uic4 ${QMAKE_FILE_NAME} -o ${QMAKE_FILE_OUT}
uic4.input = UI_SOURCES
uic4.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += uic4

PROJECTNAME        = Assistant
DESTDIR            = ../../bin
INCLUDEPATH += $(QTDIR)/include/flat

PRE_TARGETDEPS += ui_sources

ui_sources.depends = ui_finddialog.h \
        ui_helpdialog.h \
        ui_mainwindow.h \
        ui_settingsdialog.h \
        ui_tabbedbrowser.h \
        ui_topicchooser.h

QMAKE_EXTRA_TARGETS += ui_sources

UI_SOURCES += finddialog.ui \
        helpdialog.ui \
        mainwindow.ui \
        settingsdialog.ui \
        tabbedbrowser.ui \
        topicchooser.ui

SOURCES        += main.cpp \
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

DEFINES += QT_KEYWORDS
#DEFINES +=  QT_PALMTOPCENTER_DOCS
!network:DEFINES        += QT_INTERNAL_NETWORK
else:QT += network
!xml: DEFINES                += QT_INTERNAL_XML
else:QT += xml
include( ../../src/qt_professional.pri )

win32:RC_FILE = assistant.rc
mac:RC_FILE = assistant.icns

#target.path = $$bins.path
#INSTALLS += target

#assistanttranslations.files = *.qm
#assistanttranslations.path = $$translations.path
#INSTALLS += assistanttranslations

TRANSLATIONS        = assistant_de.ts \
                  assistant_fr.ts

unix:!zlib:LIBS        += -lz

IMAGES        = images/editcopy.png \
        images/find.png \
        images/home.png \
        images/next.png \
        images/previous.png \
        images/print.png \
        images/whatsthis.xpm \
        images/book.png \
        images/designer.png \
        images/assistant.png \
        images/linguist.png \
        images/qt.png \
        images/zoomin.png \
        images/zoomout.png \
        images/splash.png \
        images/appicon.png \
        images/addtab.png \
        images/closetab.png \
        images/d_closetab.png
