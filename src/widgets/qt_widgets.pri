# Qt widgets module

widgets {
	win32:WIDGETS_H	= ../include
	unix:WIDGETS_H	= widgets
	WIDGETS_P	= widgets
	unix:DEPENDPATH += :$$WIDGETS_H
	HEADERS += $$WIDGETS_H/qbuttongroup.h \
		  $$WIDGETS_H/qbutton.h \
		  $$WIDGETS_H/qcheckbox.h \
		  $$WIDGETS_H/qcdestyle.h \
		  $$WIDGETS_H/qcombobox.h \
		  $$WIDGETS_H/qcommonstyle.h \
		  $$WIDGETS_H/qwidgetresizehandler.h \
		  $$WIDGETS_H/qdial.h \
		  $$WIDGETS_H/qdockarea.h \
		  $$WIDGETS_H/qdockwindow.h \
		  $$WIDGETS_H/qframe.h \
		  $$WIDGETS_H/qgrid.h \
		  $$WIDGETS_H/qgroupbox.h \
		  $$WIDGETS_H/qhbuttongroup.h \
		  $$WIDGETS_H/qheader.h \
		  $$WIDGETS_H/qhgroupbox.h \
		  $$WIDGETS_H/qhbox.h \
		  $$WIDGETS_H/qlabel.h \
		  $$WIDGETS_H/qlcdnumber.h \
		  $$WIDGETS_H/qlineedit.h \
		  $$WIDGETS_H/qlistbox.h \
		  $$WIDGETS_H/qlistview.h \
		  $$WIDGETS_H/qmainwindow.h \
		  $$WIDGETS_H/qmenubar.h \
		  $$WIDGETS_H/qmenudata.h \
		  $$WIDGETS_H/qmotifstyle.h \
		  $$WIDGETS_H/qmotifplusstyle.h \
		  $$WIDGETS_H/qmultilineedit.h \
		  $$WIDGETS_H/qplatinumstyle.h \
		  $$WIDGETS_H/qpopupmenu.h \
		  $$WIDGETS_H/qprogressbar.h \
		  $$WIDGETS_H/qpushbutton.h \
		  $$WIDGETS_H/qradiobutton.h \
		  $$WIDGETS_H/qrangecontrol.h \
		  $$WIDGETS_H/qscrollbar.h \
		  $$WIDGETS_H/qscrollview.h \
		  $$WIDGETS_H/qsgistyle.h \
		  $$WIDGETS_H/qslider.h \
		  $$WIDGETS_H/qspinbox.h \
		  $$WIDGETS_H/qsplitter.h \
		  $$WIDGETS_H/qstatusbar.h \
		  $$WIDGETS_H/qtabbar.h \
		  $$WIDGETS_H/qtabwidget.h \
		  $$WIDGETS_H/qtableview.h \
		  $$WIDGETS_H/qtoolbar.h \
		  $$WIDGETS_H/qtoolbutton.h \
		  $$WIDGETS_H/qtooltip.h \
		  $$WIDGETS_H/qvalidator.h \
		  $$WIDGETS_H/qvbox.h \
		  $$WIDGETS_H/qvbuttongroup.h \
		  $$WIDGETS_H/qvgroupbox.h \
		  $$WIDGETS_H/qwhatsthis.h \
		  $$WIDGETS_H/qwidgetstack.h \
		  $$WIDGETS_H/qwindowsstyle.h \
		  $$WIDGETS_H/qaction.h

	embedded:HEADERS   += $$WIDGETS_H/qcompactstyle.h
	embedded:SOURCES += $$WIDGETS_H/qcompactstyle.cpp	

	SOURCES += widgets/qbuttongroup.cpp \
		  widgets/qbutton.cpp \
		  widgets/qcdestyle.cpp \
		  widgets/qcheckbox.cpp \
		  widgets/qcombobox.cpp \
		  widgets/qwidgetresizehandler.cpp \
		  widgets/qcommonstyle.cpp \
		  widgets/qdial.cpp \
		  widgets/qdockarea.cpp \
		  widgets/qdockwindow.cpp \
		  widgets/qframe.cpp \
		  widgets/qgrid.cpp \
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
		  widgets/qmenubar.cpp \
		  widgets/qmenudata.cpp \
		  widgets/qmotifstyle.cpp \
		  widgets/qmotifplusstyle.cpp \
		  widgets/qmultilineedit.cpp \
		  widgets/qplatinumstyle.cpp \
		  widgets/qpopupmenu.cpp \
		  widgets/qprogressbar.cpp \
		  widgets/qpushbutton.cpp \
		  widgets/qradiobutton.cpp \
		  widgets/qrangecontrol.cpp \
		  widgets/qscrollbar.cpp \
		  widgets/qscrollview.cpp \
		  widgets/qsgistyle.cpp \
		  widgets/qslider.cpp \
		  widgets/qspinbox.cpp \
		  widgets/qsplitter.cpp \
		  widgets/qstatusbar.cpp \
		  widgets/qtabbar.cpp \
		  widgets/qtabwidget.cpp \
		  widgets/qtableview.cpp \
		  widgets/qtoolbar.cpp \
		  widgets/qtoolbutton.cpp \
		  widgets/qtooltip.cpp \
		  widgets/qvalidator.cpp \
		  widgets/qvbox.cpp \
		  widgets/qvbuttongroup.cpp \
		  widgets/qvgroupbox.cpp \
		  widgets/qwhatsthis.cpp \
		  widgets/qwidgetstack.cpp \
		  widgets/qwindowsstyle.cpp \
		  widgets/qaction.cpp \
		  widgets/qeffects.cpp
	embedded:SOURCES += $$WIDGETS_H/qcompactstyle.cpp
}