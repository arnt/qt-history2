# Qt compat module

compat {
	COMPAT_P	= compat

	HEADERS += \
		other/qdropsite.h \
		other/qurloperator.h \
		other/qlocalfs.h \
		other/qnetworkprotocol.h \
		../gui/dialogs/qfiledialog.h 

	SOURCES += \
		other/qdropsite.cpp \
		other/qurloperator.cpp \
		other/qlocalfs.cpp \
		other/qnetworkprotocol.cpp \
		../gui/dialogs/qfiledialog.cpp 
	
	win32:SOURCES += ../gui/dialogs/qfiledialog_win.cpp
        !embedded:mac:SOURCES  += ../gui/dialogs/qfiledialog_mac.cpp
}
