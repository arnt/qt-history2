# Qt compat module

HEADERS += \
    widgets/q3action.h \
    widgets/q3buttongroup.h \
    widgets/q3datetimeedit.h \
    widgets/q3dockarea.h \
    widgets/q3dockwindow.h \
    widgets/q3groupbox.h \
    widgets/q3header.h \
    widgets/q3mainwindow.h \
    widgets/q3mainwindow_p.h \
    widgets/q3menubar.h \
    widgets/q3menudata.h \
    widgets/q3popupmenu.h \
    widgets/q3toolbar.h \
    widgets/q3whatsthis.h \
    widgets/qgridview.h \
	widgets/qrangecontrol.h \
    widgets/qwidgetstack.h

SOURCES += \
    widgets/q3action.cpp \
    widgets/q3buttongroup.cpp \
    widgets/q3datetimeedit.cpp \
    widgets/q3dockarea.cpp \
    widgets/q3dockwindow.cpp \
    widgets/q3groupbox.cpp \
    widgets/q3header.cpp \
    widgets/q3mainwindow.cpp \
    widgets/q3menubar.cpp \
    widgets/q3menudata.cpp \
    widgets/q3popupmenu.cpp \
    widgets/q3toolbar.cpp \
    widgets/q3whatsthis.cpp \
	widgets/qgridview.cpp \
    widgets/qrangecontrol.cpp \
    widgets/qspinwidget.cpp \
    widgets/qwidgetstack.cpp

mac:SOURCES += widgets/q3menubar_mac.cpp
