SOURCES	+= main.cpp 
sql {
	HEADERS		+= sqlformwizardimpl.h
	SOURCES		+= sqlformwizardimpl.cpp
	}
DESTDIR		= ../../../../plugins/designer

target.path=$$plugins.path
isEmpty(target.path):target.path=$$QT_PREFIX/plugins
INSTALLS += target
IMAGEFILE	= images.cpp
PROJECTNAME	= Wizards
FORMS	= sqlformwizard.ui mainwindowwizard.ui 
IMAGES	= images/down images/left images/logo images/qtwizards_menu_1 images/qtwizards_menu_2 images/qtwizards_menu_3 images/qtwizards_table_1 images/qtwizards_table_2 images/qtwizards_table_3 images/qtwizards_table_4 images/re-sort.png images/right images/up 
TEMPLATE	=lib
CONFIG	+= qt warn_on release plugin
INCLUDEPATH	+= ../../interfaces
DBFILE	= wizards.db
LANGUAGE	= C++
