# Qt widgets module

HEADERS += widgets/qbuttongroup.h \
	widgets/qbutton.h \
	widgets/qabstractbutton.h \
	widgets/qabstractbutton_p.h \
	widgets/qdialogbuttons_p.h \
	widgets/qcheckbox.h \
	widgets/qcombobox.h \
	widgets/qwidgetresizehandler_p.h \
	widgets/qdial.h \
	widgets/qdockarea.h \
	widgets/qdockwindow.h \
	widgets/qframe.h \
	widgets/qframe_p.h \
	widgets/qgrid.h \
	widgets/qgridview.h \
	widgets/qgroupbox.h \
	widgets/qhbuttongroup.h \
	widgets/qheader.h \
	widgets/qhgroupbox.h \
	widgets/qhbox.h \
	widgets/qlabel.h \
	widgets/qlcdnumber.h \
	widgets/qlineedit.h \
	widgets/qlineedit_p.h \
	widgets/qlistbox.h \
	widgets/qlistview.h \
	widgets/qmainwindow.h \
        widgets/qmenu.h \
	widgets/q3menubar.h \
	widgets/q3menudata.h \
	widgets/qpopupmenu.h \
	widgets/qmenubar.h \
	widgets/q3popupmenu.h \
	widgets/qprogressbar.h \
	widgets/qpushbutton.h \
	widgets/qradiobutton.h \
	widgets/qrangecontrol.h \
	widgets/qscrollbar.h \
	widgets/qscrollview.h \
        widgets/qscrollview_p.h \
	widgets/qsizegrip.h \
	widgets/qslider.h \
	widgets/qabstractslider.h \
	widgets/qviewport_p.h \
	widgets/qviewport.h \
	widgets/qwidgetview.h \
	widgets/qsplashscreen.h \
	widgets/qspinbox.h \
	widgets/qsplitter.h \
	widgets/qstatusbar.h \
	widgets/qstackedbox.h \
	widgets/qtabbar.h \
	widgets/qtabwidget.h \
	widgets/qtitlebar_p.h \
	widgets/qabstractslider_p.h \
	widgets/qtoolbar.h \
	widgets/qtoolbox.h \
	widgets/qtoolbutton.h \
	widgets/qvalidator.h \
	widgets/qvbox.h \
	widgets/qvbuttongroup.h \
	widgets/qvgroupbox.h \
	widgets/qwidgetstack.h \
	widgets/qdatetimeedit.h \
	widgets/qwidgetinterface_p.h \
	widgets/qwidgetplugin.h \
        widgets/qrubberband.h \
	widgets/qworkspace.h

SOURCES += widgets/qbuttongroup.cpp \
	widgets/qbutton.cpp \
	widgets/qabstractbutton.cpp \
	widgets/qdialogbuttons.cpp \
	widgets/qcheckbox.cpp \
	widgets/qcombobox.cpp \
	widgets/qwidgetresizehandler.cpp \
	widgets/qdial.cpp \
	widgets/qdockarea.cpp \
	widgets/qdockwindow.cpp \
	widgets/qframe.cpp \
	widgets/qgrid.cpp \
	widgets/qgridview.cpp \
	widgets/qgroupbox.cpp \
	widgets/qhbuttongroup.cpp \
	widgets/qheader.cpp \
	widgets/qhgroupbox.cpp \
	widgets/qhbox.cpp \
	widgets/qlabel.cpp \
	widgets/qlcdnumber.cpp \
	widgets/qlineedit.cpp \
	widgets/qlistbox.cpp \
	widgets/qlistview.cpp \
	widgets/qmainwindow.cpp \
	widgets/qmenudata.cpp \
        widgets/qmenu.cpp \
	widgets/q3menubar.cpp \
	widgets/q3menudata.cpp \
	widgets/qmenubar.cpp \
	widgets/q3popupmenu.cpp \
	widgets/qprogressbar.cpp \
	widgets/qpushbutton.cpp \
	widgets/qradiobutton.cpp \
	widgets/qrangecontrol.cpp \
	widgets/qscrollbar.cpp \
	widgets/qscrollview.cpp \
	widgets/qsizegrip.cpp \
	widgets/qslider.cpp \
	widgets/qabstractslider.cpp \
	widgets/qviewport.cpp \
	widgets/qwidgetview.cpp \
	widgets/qsplashscreen.cpp \
	widgets/qspinbox.cpp \
	widgets/qspinwidget.cpp \
	widgets/qsplitter.cpp \
	widgets/qstackedbox.cpp \
	widgets/qstatusbar.cpp \
	widgets/qtabbar.cpp \
	widgets/qtabwidget.cpp \
	widgets/qtitlebar.cpp \
	widgets/qtoolbar.cpp \
	widgets/qtoolbox.cpp \
	widgets/qtoolbutton.cpp \
	widgets/qvalidator.cpp \
	widgets/qvbox.cpp \
	widgets/qvbuttongroup.cpp \
	widgets/qvgroupbox.cpp \
	widgets/qwidgetstack.cpp \
	widgets/qdatetimeedit.cpp \
	widgets/qeffects.cpp \
	widgets/qwidgetplugin.cpp \
        widgets/qrubberband.cpp \
	widgets/qworkspace.cpp
   

!embedded:mac:SOURCES += widgets/q3menubar_mac.cpp widgets/qmenu_mac.cpp

wince-* {
	SOURCES += widgets/ce/qcemainwindow.cpp
	HEADERS += widgets/ce/qcemainwindow.h

	SOURCES -= widgets/qsyntaxhighlighter.cpp \
		   widgets/qsplashscreen.cpp

	HEADERS -= widgets/qsyntaxhighlighter.h \
		   widgets/qsplashscreen.h
}
