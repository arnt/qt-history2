TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= helpfinddialog.h \
		  helpmainwindow.h \
		  helpnavigation.h \
		  helptopichcooser.h \
		  helpview.h
SOURCES		= helpfinddialog.cpp \
		  helpmainwindow.cpp \
		  helpnavigation.cpp \
		  helptopichcooser.cpp \
		  helpview.cpp \
		  main.cpp
TARGET		= qdoc
