TEMPLATE	= app
CONFIG		+= qt warn_on debug
HEADERS		= qskin.h \
		  themes.h \
		  ../../examples/buttongroups/buttongroups.h \
		  ../../examples/lineedits/lineedits.h \
		  ../../examples/listboxcombo/listboxcombo.h \
		  ../../examples/checklists/checklists.h \
		  ../../examples/progressbar/progressbar.h \
		  ../../examples/rangecontrols/rangecontrols.h \
		  ../../examples/richtext/richtext.h
SOURCES		= qskin.cpp \
		  main.cpp \
		  themes.cpp \
		  ../../examples/buttongroups/buttongroups.cpp \
		  ../../examples/lineedits/lineedits.cpp \
		  ../../examples/listboxcombo/listboxcombo.cpp \
		  ../../examples/checklists/checklists.cpp \
		  ../../examples/progressbar/progressbar.cpp \
		  ../../examples/rangecontrols/rangecontrols.cpp \
		  ../../examples/richtext/richtext.cpp
TARGET		= skins
REQUIRES=full-config
