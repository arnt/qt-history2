TEMPLATE	= lib
CONFIG		+= qt warn_on release plugin

SOURCES		+= main.cpp 
FORMS		= sqlformwizard.ui mainwindowwizard.ui 

sql {
	HEADERS		+= sqlformwizardimpl.h
	SOURCES		+= sqlformwizardimpl.cpp
	}
DESTDIR		= ../../../../plugins/designer

IMAGEFILE	= images.cpp
PROJECTNAME	= Wizards
IMAGES		= images/down.png images/left.png images/logo.png images/qtwizards_menu_1.png images/qtwizards_menu_2.png images/qtwizards_menu_3.png images/qtwizards_table_1.png images/qtwizards_table_2.png images/qtwizards_table_3.png images/qtwizards_table_4.png images/re-sort.png images/right.png images/up.png 
INCLUDEPATH	+= ../../interfaces
DBFILE		= wizards.db
LANGUAGE	= C++

isEmpty(plugins.path):plugins.path=$$QT_PREFIX/plugins
target.path += $$plugins.path/designer
INSTALLS 	+= target
