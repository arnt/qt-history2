# Qt remote module

remote {
	REMOTE_P	= remote

	HEADERS	+= $$REMOTE_H/qtestcontrol_p.h \
		  $$REMOTE_H/qtestlistbox_p.h \
		  $$REMOTE_H/qtestmenubar_p.h \
		  $$REMOTE_H/qtestpopupmenu_p.h \
		  $$REMOTE_H/qremotemessage_p.h \
		  $$REMOTE_H/qtestwidgets_p.h
			

	SOURCES += $$REMOTE_CPP/qtestcontrol.cpp \
		  $$REMOTE_CPP/qtestlistbox.cpp \
		  $$REMOTE_CPP/qtestmenubar.cpp \
		  $$REMOTE_CPP/qtestpopupmenu.cpp \
		  $$REMOTE_CPP/qremotemessage.cpp \
		  $$REMOTE_CPP/qtestwidgets.cpp
}
else:DEFINES += QT_NO_REMOTE
