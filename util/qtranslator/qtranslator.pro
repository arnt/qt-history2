TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= mergetr.h \
		  msg2qm.h \
		  qtaddlangdia.h \
		  qtconfig.h \
		  qtmainwindow.h \
		  qtmessageview.h \
		  qtprefdia.h \
		  qtpreferences.h
SOURCES		= main.cpp \
		  mergetr.cpp \
		  msg2qm.cpp \
		  qtaddlangdia.cpp \
		  qtconfig.cpp \
		  qtmainwindow.cpp \
		  qtmessageview.cpp \
		  qtprefdia.cpp \
		  qtpreferences.cpp
TARGET		= qtranslator
