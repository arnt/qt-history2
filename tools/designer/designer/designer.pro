TEMPLATE	= lib
CONFIG		+= qt warn_on staticlib
CONFIG 		-= dll

TARGET	= designer
win32:TARGET = designerlib

DEFINES	+= DESIGNER
DEFINES += QT_INTERNAL_XML
DEFINES += QT_INTERNAL_WORKSPACE
DEFINES += QT_INTERNAL_ICONVIEW
DEFINES += QT_INTERNAL_TABLE
table:win32-msvc:DEFINES+=Q_TEMPLATE_EXTERN=extern

include( ../../../src/qt_professional.pri )

SOURCES	+= command.cpp \
		 formwindow.cpp \
		defs.cpp \
		layout.cpp \
		mainwindow.cpp \
		mainwindowactions.cpp \
		metadatabase.cpp \
		pixmapchooser.cpp \
		propertyeditor.cpp \
		resource.cpp \
		sizehandle.cpp \
		orderindicator.cpp \
		widgetfactory.cpp \
		hierarchyview.cpp \
		listboxeditorimpl.cpp \
		newformimpl.cpp \
		workspace.cpp \
		listvieweditorimpl.cpp \
		customwidgeteditorimpl.cpp \
		paletteeditorimpl.cpp \
		styledbutton.cpp \
		iconvieweditorimpl.cpp \
		multilineeditorimpl.cpp \
		formsettingsimpl.cpp \
		asciivalidator.cpp \
		splashloader.cpp \
		designerapp.cpp \
		designerappiface.cpp \
		actioneditorimpl.cpp \
		actionlistview.cpp \
		actiondnd.cpp \
		project.cpp \
		projectsettingsimpl.cpp \
		sourceeditor.cpp \
		outputwindow.cpp \
		../shared/widgetdatabase.cpp \
		../shared/parser.cpp \
		config.cpp \
		pixmapcollection.cpp \
		previewframe.cpp \
		previewwidgetimpl.cpp \
		paletteeditoradvancedimpl.cpp \
		sourcefile.cpp \
		filechooser.cpp \
		wizardeditorimpl.cpp \
		qcompletionedit.cpp \
		timestamp.cpp \
		formfile.cpp \
		qcategorywidget.cpp \
		widgetaction.cpp \
		propertyobject.cpp \
		startdialogimpl.cpp \
		syntaxhighlighter_html.cpp \
		connectionitems.cpp \
		editfunctionsimpl.cpp \
		variabledialogimpl.cpp \
		listviewdnd.cpp \
		listviewitemdrag.cpp
HEADERS	+= command.h \
		defs.h \
		formwindow.h \
		layout.h \
		mainwindow.h \
		metadatabase.h \
		pixmapchooser.h \
		propertyeditor.h \
		resource.h \
		sizehandle.h \
		orderindicator.h \
		widgetfactory.h \
		hierarchyview.h \
		listboxeditorimpl.h \
		newformimpl.h \
		workspace.h \
		listvieweditorimpl.h \
		customwidgeteditorimpl.h \
		paletteeditorimpl.h \
		styledbutton.h \
		iconvieweditorimpl.h \
		multilineeditorimpl.h \
		formsettingsimpl.h \
		asciivalidator.h \
		splashloader.h \
		../interfaces/widgetinterface.h \
		../interfaces/actioninterface.h \
		../interfaces/filterinterface.h \
		../interfaces/designerinterface.h \
		designerapp.h \
		designerappiface.h \
		actioneditorimpl.h \
		actionlistview.h \
		actiondnd.h \
		project.h \
		projectsettingsimpl.h \
		sourceeditor.h \
		outputwindow.h \
		../shared/widgetdatabase.h \
		../shared/parser.h \
		config.h \
		previewframe.h \
		previewwidgetimpl.h \
		paletteeditoradvancedimpl.h \
		pixmapcollection.h \
		sourcefile.h \
		filechooser.h \
		wizardeditorimpl.h \
		qcompletionedit.h \
		timestamp.h \
		formfile.h \
		qcategorywidget.h \
		widgetaction.h \
		propertyobject.h \
		startdialogimpl.h \
		syntaxhighlighter_html.h \
		connectionitems.h \
		editfunctionsimpl.h \
		variabledialogimpl.h \
		listviewdnd.h \
		listviewitemdrag.h

FORMS		+= listboxeditor.ui \
		editfunctions.ui \
		newform.ui \
		listvieweditor.ui \
		customwidgeteditor.ui \
		paletteeditor.ui \
		iconvieweditor.ui \
		preferences.ui \
		multilineeditor.ui \
		formsettings.ui \
		about.ui \
		pixmapfunction.ui \
		createtemplate.ui \
		actioneditor.ui \
		projectsettings.ui \
		finddialog.ui \
		replacedialog.ui \
		gotolinedialog.ui \
		pixmapcollectioneditor.ui \
		previewwidget.ui \
		paletteeditoradvanced.ui \
		wizardeditor.ui \
		listeditor.ui \
		startdialog.ui \
		richtextfontdialog.ui \
		connectiondialog.ui \
		variabledialog.ui \
		configtoolboxdialog.ui

OBJECTS_DIR	= .

DEPENDPATH	+= $$QT_SOURCE_TREE/include
VERSION  	= 1.0.0
DESTDIR		= $$QT_BUILD_TREE/lib

aix-g++ {
	QMAKE_CFLAGS += -mminimal-toc
	QMAKE_CXXFLAGS += -mminimal-toc
}

sql {
	SOURCES  += database.cpp dbconnectionimpl.cpp dbconnectionsimpl.cpp
	HEADERS += database.h dbconnectionimpl.h dbconnectionsimpl.h
	FORMS += dbconnections.ui dbconnection.ui dbconnectioneditor.ui
}

table {
	HEADERS += tableeditorimpl.h
	SOURCES += tableeditorimpl.cpp
	FORMS += tableeditor.ui
}

INCLUDEPATH	+= ../shared ../uilib
win32:LIBS	+= $$QT_BUILD_TREE/lib/qui.lib $$QT_BUILD_TREE/lib/qassistantclient.lib
unix:LIBS		+= -L$$QT_BUILD_TREE/lib -lqui -lqassistantclient

TRANSLATIONS	= designer_de.ts designer_fr.ts

target.path=$$libs.path
INSTALLS += target
templates.path=$$data.path/templates
templates.files = ../templates/*
INSTALLS += templates
