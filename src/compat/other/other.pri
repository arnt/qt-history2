# Qt compat module

compat {
	COMPAT_P	= compat

	HEADERS += \
		other/qdropsite.h \
		other/qguardedptr.h \
		other/qlocalfs.h \
		other/qnetworkprotocol.h \
		other/qurloperator.h \
		../gui/dialogs/qfiledialog.h 

	SOURCES += \
		other/qdropsite.cpp \
		other/qlocalfs.cpp \
		other/qnetworkprotocol.cpp \
		other/qurloperator.cpp \
		../gui/dialogs/qfiledialog.cpp 
	
	win32:SOURCES += ../gui/dialogs/qfiledialog_win.cpp
        !embedded:mac:SOURCES  += ../gui/dialogs/qfiledialog_mac.cpp
}
