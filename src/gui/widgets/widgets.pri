# Qt widgets module

HEADERS += \
        widgets/qbuttongroup.h \
        widgets/qdockseparator_p.h \
        widgets/qabstractbutton.h \
        widgets/qabstractbutton_p.h \
        widgets/qabstractslider.h \
        widgets/qabstractslider_p.h \
        widgets/qabstractspinbox.h \
        widgets/qabstractspinbox_p.h \
        widgets/qcheckbox.h \
        widgets/qcombobox.h \
        widgets/qcombobox_p.h \
        widgets/qdatetimeedit.h \
        widgets/qdial.h \
        widgets/qdialogbuttons_p.h \
        widgets/qdockwidget.h \
        widgets/qdockwidget_p.h \
        widgets/qdockwidgetlayout_p.h \
        widgets/qdockwidgetseparator_p.h \
        widgets/qframe.h \
        widgets/qframe_p.h \
        widgets/qgroupbox.h \
        widgets/qlabel.h \
        widgets/qlcdnumber.h \
        widgets/qlineedit.h \
        widgets/qlineedit_p.h \
        widgets/qmainwindow.h \
        widgets/qmainwindowlayout_p.h \
        widgets/qmenu.h \
        widgets/qmenubar.h \
        widgets/qprogressbar.h \
        widgets/qpushbutton.h \
        widgets/qradiobutton.h \
        widgets/qrubberband.h \
        widgets/qscrollbar.h \
        widgets/qsizegrip.h \
        widgets/qslider.h \
        widgets/qspinbox.h \
        widgets/qsplashscreen.h \
        widgets/qsplitter.h \
        widgets/qstackedwidget.h \
        widgets/qstatusbar.h \
        widgets/qtabbar.h \
        widgets/qtabwidget.h \
        widgets/qtextedit.h \
        widgets/qtextedit_p.h \
        widgets/qtextbrowser.h \
        widgets/qtoolbar.h \
        widgets/qtoolbar_p.h \
        widgets/qtoolbarextension_p.h \
        widgets/qtoolbarhandle_p.h \
        widgets/qtoolbarseparator_p.h \
        widgets/qtoolbox.h \
        widgets/qtoolbutton.h \
        widgets/qvalidator.h \
        widgets/qabstractscrollarea.h \
        widgets/qabstractscrollarea_p.h \
        widgets/qwidgetresizehandler_p.h \
        widgets/qfocusframe.h \
        widgets/qscrollarea.h \
        widgets/qworkspace.h

SOURCES += \
        widgets/qabstractbutton.cpp \
        widgets/qabstractslider.cpp \
        widgets/qabstractspinbox.cpp \
        widgets/qcheckbox.cpp \
        widgets/qcombobox.cpp \
        widgets/qdatetimeedit.cpp \
        widgets/qdial.cpp \
        widgets/qdialogbuttons.cpp \
        widgets/qdockseparator.cpp \
        widgets/qdockwidget.cpp \
        widgets/qdockwidgetlayout.cpp \
        widgets/qdockwidgetseparator.cpp \
        widgets/qeffects.cpp \
        widgets/qframe.cpp \
        widgets/qgroupbox.cpp \
        widgets/qlabel.cpp \
        widgets/qlcdnumber.cpp \
        widgets/qlineedit.cpp \
        widgets/qmainwindow.cpp \
        widgets/qmainwindowlayout.cpp \
        widgets/qmenu.cpp \
        widgets/qmenubar.cpp \
        widgets/qmenudata.cpp \
        widgets/qprogressbar.cpp \
        widgets/qpushbutton.cpp \
        widgets/qradiobutton.cpp \
        widgets/qrubberband.cpp \
        widgets/qscrollbar.cpp \
        widgets/qsizegrip.cpp \
        widgets/qslider.cpp \
        widgets/qspinbox.cpp \
        widgets/qsplashscreen.cpp \
        widgets/qsplitter.cpp \
        widgets/qstackedwidget.cpp \
        widgets/qstatusbar.cpp \
        widgets/qtabbar.cpp \
        widgets/qtabwidget.cpp \
        widgets/qtextedit.cpp \
        widgets/qtextbrowser.cpp \
        widgets/qtoolbar.cpp \
        widgets/qtoolbarextension.cpp \
        widgets/qtoolbarhandle.cpp \
        widgets/qtoolbarseparator.cpp \
        widgets/qtoolbox.cpp \
        widgets/qtoolbutton.cpp \
        widgets/qvalidator.cpp \
        widgets/qabstractscrollarea.cpp \
        widgets/qwidgetresizehandler.cpp \
        widgets/qfocusframe.cpp \
        widgets/qscrollarea.cpp \
        widgets/qworkspace.cpp


!embedded:mac:SOURCES += widgets/qmenu_mac.cpp

wince-* {
        SOURCES += widgets/ce/qcemainwindow.cpp
        HEADERS += widgets/ce/qcemainwindow.h

        SOURCES -= widgets/qsyntaxhighlighter.cpp \
                   widgets/qsplashscreen.cpp

        HEADERS -= widgets/qsyntaxhighlighter.h \
                   widgets/qsplashscreen.h
}
