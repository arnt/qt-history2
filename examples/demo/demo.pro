TEMPLATE         = app
TARGET           = demo

DEFINES         += QT_MODULE_CANVAS

QT              += compat xml network opengl
CONFIG          += network warn_on release uic3
unix:LIBS       += -lm
INCLUDEPATH     += . ./dnd ./i18n ./opengl ./qasteroids ./sql ./textdrawing ./widgets

HEADERS                = frame.h \
                  categoryinterface.h \
                  qthumbwheel.h \
                  display.h \
                  textdrawing/textedit.h \
                  textdrawing/helpwindow.h \
                  dnd/dnd.h \
                  dnd/styledbutton.h \
                  dnd/iconview.h \
                  dnd/listview.h \
                  i18n/i18n.h \
                  i18n/wrapper.h \
                  ../aclock/aclock.h
SOURCES                = frame.cpp \
                  qthumbwheel.cpp \
                         display.cpp \
                  textdrawing/textedit.cpp \
                  textdrawing/helpwindow.cpp \
                  dnd/dnd.cpp \
                  dnd/styledbutton.cpp \
                  dnd/iconview.cpp \
                  dnd/listview.cpp \
                  i18n/i18n.cpp \
                  ../aclock/aclock.cpp \
                  main.cpp

FORMS                = dnd/dndbase.ui

    HEADERS         +=graph.h \
                  qasteroids/toplevel.h \
                  qasteroids/view.h \
                  qasteroids/ledmeter.h
    SOURCES         +=graph.cpp \
                  qasteroids/toplevel.cpp \
                  qasteroids/view.cpp \
                  qasteroids/ledmeter.cpp

contains(QT_CONFIG, opengl) {
    HEADERS         +=opengl/glworkspace.h \
                  opengl/glcontrolwidget.h \
                  opengl/gltexobj.h \
                  opengl/glbox.h \
                  opengl/glgear.h \
                  opengl/gllandscape.h \
                  opengl/fbm.h \
                  opengl/glinfo.h \
                  opengl/glinfotext.h
    SOURCES         +=opengl/glworkspace.cpp \
                   opengl/glcontrolwidget.cpp \
                  opengl/gltexobj.cpp \
                  opengl/glbox.cpp \
                  opengl/glgear.cpp \
                  opengl/gllandscape.cpp \
                  opengl/fbm.c
    win32 {
        SOURCES +=opengl/glinfo_win.cpp
    } mac {
        SOURCES +=opengl/glinfo_mac.cpp
        LIBS         +=-framework Carbon
    } else:unix {
        SOURCES +=opengl/glinfo_x11.cpp
    }

    FORMS         +=opengl/printpreview.ui \
                  opengl/gllandscapeviewer.ui

    CONFIG -= dlopen_opengl
}

contains(QT_CONFIG, sql) {
    FORMS         +=sql/connect.ui \
                  sql/sqlex.ui
}

FORMS         +=widgets/widgetsbase.ui

TRANSLATIONS        = translations/demo_ar.ts \
                  translations/demo_de.ts \
                  translations/demo_fr.ts \
                  translations/demo_iw.ts
