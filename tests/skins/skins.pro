TEMPLATE	= app
CONFIG		+= qt warn_on debug
HEADERS		= qskin.h \
		  themes.h \
		  ../../examples/buttongroups/buttongroups.h \
		  ../../examples/lineedits/lineedits.h \
		  ../../examples/listboxcombo/listboxcombo.h \
		  ../../examples/checklists/checklists.h \
		  ../../examples/progressbar/progressbar.h \
		  rangecontrols.h \
		  ../../examples/richtext/richtext.h
SOURCES		= qskin.cpp \
		  main.cpp \
		  themes.cpp \
		  ../../examples/buttongroups/buttongroups.cpp \
		  ../../examples/lineedits/lineedits.cpp \
		  ../../examples/listboxcombo/listboxcombo.cpp \
		  ../../examples/checklists/checklists.cpp \
		  ../../examples/progressbar/progressbar.cpp \
		  rangecontrols.cpp \
		  ../../examples/richtext/richtext.cpp
TARGET		= skins
QTDIR_build:REQUIRES=full-config
