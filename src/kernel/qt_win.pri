# Qt/Windows only configuration file
# --------------------------------------------------------------------

	# Enable PCH when compiling Qt
	# PRECOMPILED_HEADER = kernel/qt_pch.h

	wince-* {
		HEADERS += $$KERNEL_H/qfunctions_wce.h
		SOURCES -= $$KERNEL_CPP/qfontengine_win.cpp \
			   $$KERNEL_CPP/qregion_win.cpp
		SOURCES += $$KERNEL_CPP/qfunctions_wce.cpp \
			   $$KERNEL_CPP/qfontengine_wce.cpp \			   
			   $$KERNEL_CPP/qregion_wce.cpp
	}
