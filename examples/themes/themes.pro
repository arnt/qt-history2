TEMPLATE	= app
CONFIG		= qt warn_on release
HEADERS		= themes.h \
		  ../buttons_groups/buttons_groups.h \
		  ../linedits/linedits.h \
		  ../listbox_combo/listbox_combo.h \
		  ../checklists/checklists.h \
		  ../progressbar/progressbar.h \
		  ../rangecontrols/rangecontrols.h \
		  ../richtext/richtext.h \
		  wood.h \
		  methal.h
		
SOURCES		= themes.cpp \
		  main.cpp \
		  ../buttons_groups/buttons_groups.cpp \
		  ../linedits/linedits.cpp \
		  ../listbox_combo/listbox_combo.cpp \
		  ../checklists/checklists.cpp \
		  ../progressbar/progressbar.cpp \
		  ../rangecontrols/rangecontrols.cpp \
		  ../richtext/richtext.cpp \
		  wood.cpp \
		  metal.cpp

TARGET		= themes
DEPENDPATH=../../include
