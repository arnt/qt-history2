TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= mywidget.h
SOURCES		= main.cpp \
		  mywidget.cpp
TARGET		= i18n
TRANSLATIONS	= mywidget_cs.ts \
		  mywidget_de.ts \
		  mywidget_el.ts \
		  mywidget_en.ts \
		  mywidget_eo.ts \
		  mywidget_fr.ts \
		  mywidget_it.ts \
		  mywidget_jp.ts \
		  mywidget_ko.ts \
		  mywidget_no.ts \
		  mywidget_ru.ts \
		  mywidget_zh.ts
DEPENDPATH=../../include
REQUIRES=full-config
