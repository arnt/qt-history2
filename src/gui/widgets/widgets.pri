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
        widgets/qdockwindow.h \
	widgets/qdockwindow_p.h \
        widgets/qdockwindowlayout_p.h \
        widgets/qdockwindowseparator_p.h \
        widgets/qframe.h \
        widgets/qframe_p.h \
        widgets/qgridwidget.h \
        widgets/qgroupbox.h \
        widgets/qhboxwidget.h \
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
        widgets/qstackedbox.h \
        widgets/qstatusbar.h \
        widgets/qtabbar.h \
        widgets/qtabwidget.h \
        widgets/qtextedit.h \
        widgets/qtextedit_p.h \
        widgets/qtextbrowser.h \
        widgets/qtitlebar_p.h \
        widgets/qtoolbar.h \
	widgets/qtoolbar_p.h \
        widgets/qtoolbarbutton_p.h \
	widgets/qtoolbarextension_p.h \
	widgets/qtoolbarhandle_p.h \
	widgets/qtoolbarseparator_p.h \
        widgets/qtoolbox.h \
        widgets/qtoolbutton.h \
        widgets/qvalidator.h \
        widgets/qvboxwidget.h \
        widgets/qviewport.h \
        widgets/qviewport_p.h \
        widgets/qwidgetplugin.h \
        widgets/qwidgetresizehandler_p.h \
        widgets/qwidgetview.h \
        widgets/qworkspace.h 

SOURCES += \
        widgets/qabstractbutton.cpp \
        widgets/qabstractslider.cpp \
        widgets/qabstractspinbox.cpp \
        widgets/qbuttongroup.cpp \
        widgets/qcheckbox.cpp \
        widgets/qcombobox.cpp \
        widgets/qdatetimeedit.cpp \
        widgets/qdial.cpp \
        widgets/qdialogbuttons.cpp \
        widgets/qdockseparator.cpp \
        widgets/qdockwindow.cpp \
        widgets/qdockwindowlayout.cpp \
        widgets/qdockwindowseparator.cpp \
        widgets/qeffects.cpp \
        widgets/qframe.cpp \
        widgets/qgridwidget.cpp \
        widgets/qgroupbox.cpp \
        widgets/qhboxwidget.cpp \
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
        widgets/qstackedbox.cpp \
        widgets/qstatusbar.cpp \
        widgets/qtabbar.cpp \
        widgets/qtabwidget.cpp \
        widgets/qtextedit.cpp \
        widgets/qtextbrowser.cpp \
        widgets/qtitlebar.cpp \
        widgets/qtoolbar.cpp \
        widgets/qtoolbarbutton.cpp \
	widgets/qtoolbarextension.cpp \
	widgets/qtoolbarhandle.cpp \
	widgets/qtoolbarseparator.cpp \
        widgets/qtoolbox.cpp \
        widgets/qtoolbutton.cpp \
        widgets/qvalidator.cpp \
        widgets/qvboxwidget.cpp \
        widgets/qviewport.cpp \
        widgets/qwidgetplugin.cpp \
        widgets/qwidgetresizehandler.cpp \
        widgets/qwidgetview.cpp \
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
