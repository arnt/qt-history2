# Qt tools module

tools {

	TOOLS_P		= tools
	HEADERS +=  \
		  $$TOOLS_H/qbitarray.h \
		  $$TOOLS_H/qbytearray.h \
		  $$TOOLS_H/qbuffer.h \
		  $$TOOLS_H/qcache.h \
		  $$TOOLS_H/qchar.h \
		  $$TOOLS_H/qcleanuphandler.h \
		  $$TOOLS_P/qcomlibrary_p.h \
		  $$TOOLS_H/qdatastream.h \
		  $$TOOLS_H/qdatetime.h \
		  $$TOOLS_H/qdir.h \
		  $$TOOLS_P/qdir_p.h \
		  $$TOOLS_H/qfile.h \
		  $$TOOLS_P/qfiledefs_p.h \
		  $$TOOLS_H/qfileinfo.h \
		  $$TOOLS_H/qglobal.h \
		  $$TOOLS_H/qdebug.h \
		  $$TOOLS_H/qhash.h \
		  $$TOOLS_H/qlinkedlist.h \
		  $$TOOLS_H/qlist.h \
		  $$TOOLS_P/qgpluginmanager_p.h \
		  $$TOOLS_H/qiodevice.h \
		  $$TOOLS_H/qlibrary.h \
		  $$TOOLS_P/qlibrary_p.h \
		  $$TOOLS_H/qlocale.h \
		  $$TOOLS_P/qlocale_p.h \
		  $$TOOLS_H/qmap.h \
		  $$TOOLS_P/qpluginmanager_p.h \
		  $$TOOLS_H/qregexp.h \
		  $$TOOLS_H/qsettings.h \
		  $$TOOLS_P/qsettings_p.h \
		  $$TOOLS_H/qshared.h \
		  $$TOOLS_H/qstack.h \
		  $$TOOLS_H/qstackarray.h \
		  $$TOOLS_H/qstring.h \
		  $$TOOLS_H/qstringlist.h \
		  $$TOOLS_H/qtextstream.h \
		  $$TOOLS_P/qunicodetables_p.h \
		  $$TOOLS_P/qcom_p.h \
		  $$TOOLS_H/quuid.h \
		  $$TOOLS_H/qvector.h

	win32:SOURCES += $$TOOLS_CPP/qdir_win.cpp \
	 	  $$TOOLS_CPP/qfile_win.cpp \
		  $$TOOLS_CPP/qfileinfo_win.cpp \
		  $$TOOLS_CPP/qlibrary_win.cpp \
		  $$TOOLS_CPP/qsettings_win.cpp

	wince-* {
		SOURCES -= $$TOOLS_CPP/qdir_win.cpp \
			   $$TOOLS_CPP/qfile_win.cpp \
			   $$TOOLS_CPP/qfileinfo_win.cpp
		SOURCES += $$TOOLS_CPP/qdir_wce.cpp \
			   $$TOOLS_CPP/qfile_wce.cpp \
			   $$TOOLS_CPP/qfileinfo_wce.cpp
	}

	else:unix:SOURCES += $$TOOLS_CPP/qdir_unix.cpp \
		  $$TOOLS_CPP/qfile_unix.cpp \
		  $$TOOLS_CPP/qfileinfo_unix.cpp

        mac:!x11:!embedded:SOURCES += $$TOOLS_CPP/qsettings_mac.cpp
	unix:SOURCES += $$TOOLS_CPP/qlibrary_unix.cpp \
		        $$TOOLS_CPP/qcrashhandler.cpp

	SOURCES += $$TOOLS_CPP/qbitarray.cpp \
		  $$TOOLS_CPP/qbytearray.cpp \
		  $$TOOLS_CPP/qbuffer.cpp \
		  $$TOOLS_CPP/qchar.cpp \
		  $$TOOLS_CPP/qcomlibrary.cpp \
		  $$TOOLS_CPP/qdatastream.cpp \
		  $$TOOLS_CPP/qdatetime.cpp \
		  $$TOOLS_CPP/qdir.cpp \
		  $$TOOLS_CPP/qfile.cpp \
		  $$TOOLS_CPP/qfileinfo.cpp \
		  $$TOOLS_CPP/qglobal.cpp \
		  $$TOOLS_CPP/qhash.cpp \
		  $$TOOLS_CPP/qlinkedlist.cpp \
		  $$TOOLS_CPP/qlist.cpp \
		  $$TOOLS_CPP/qgpluginmanager.cpp \
		  $$TOOLS_CPP/qiodevice.cpp \
		  $$TOOLS_CPP/qlibrary.cpp \
		  $$TOOLS_CPP/qlocale.cpp \
		  $$TOOLS_CPP/qmap.cpp \
		  $$TOOLS_CPP/qregexp.cpp \
		  $$TOOLS_CPP/qshared.cpp \
		  $$TOOLS_CPP/qdebug.cpp \
		  $$TOOLS_CPP/qstack.cpp \
		  $$TOOLS_CPP/qstring.cpp \
		  $$TOOLS_CPP/qsemaphore.cpp \
		  $$TOOLS_CPP/qsettings.cpp \
		  $$TOOLS_CPP/qstringlist.cpp \
		  $$TOOLS_CPP/qtextstream.cpp \
		  $$TOOLS_CPP/qunicodetables.cpp \
		  $$TOOLS_CPP/quuid.cpp \
		  $$TOOLS_CPP/qvector.cpp
}

# qconfig.cpp
exists($$QT_BUILD_TREE/src/tools/qconfig.cpp) {
    SOURCES += $$QT_BUILD_TREE/src/tools/qconfig.cpp
}
