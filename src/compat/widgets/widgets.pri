# Qt compat module

HEADERS +=      widgets/q3whatsthis.h \
		widgets/q3action.h \
		widgets/qwidgetstack.h \
		widgets/q3groupbox.h \
		widgets/q3buttongroup.h \
                widgets/q3menudata.h \
                widgets/q3menubar.h \
                widgets/q3popupmenu.h

SOURCES +=      widgets/q3whatsthis.cpp \
		widgets/q3action.cpp \
		widgets/qwidgetstack.cpp \
		widgets/q3groupbox.cpp \
		widgets/q3buttongroup.cpp \
                widgets/q3menudata.cpp \
                widgets/q3menubar.cpp \
                widgets/q3popupmenu.cpp
		
mac:SOURCES += widgets/q3menubar_mac.cpp
