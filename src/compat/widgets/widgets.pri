# Qt compat module

HEADERS +=      widgets/q3whatsthis.h \
		widgets/q3action.h \
		widgets/qwidgetstack.h \
		widgets/q3groupbox.h \
		widgets/q3buttongroup.h \
                widgets/q3menudata.h \
                widgets/q3menubar.h \
                widgets/q3popupmenu.h \
	 	../gui/widgets/qdatetimeedit.h \
		../gui/widgets/qdockarea.h \
		../gui/widgets/qdockwindow.h \
		../gui/widgets/qmainwindow.h \
		../gui/widgets/qtoolbar.h

SOURCES +=      widgets/q3whatsthis.cpp \
		widgets/q3action.cpp \
		widgets/qwidgetstack.cpp \
		widgets/q3groupbox.cpp \
		widgets/q3buttongroup.cpp \
                widgets/q3menudata.cpp \
                widgets/q3menubar.cpp \
                widgets/q3popupmenu.cpp \
	 	../gui/widgets/qdatetimeedit.cpp \
		../gui/widgets/qdockarea.cpp \
		../gui/widgets/qdockwindow.cpp \
		../gui/widgets/qmainwindow.cpp \
		../gui/widgets/qtoolbar.cpp

mac:SOURCES += widgets/q3menubar_mac.cpp
