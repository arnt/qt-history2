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

IMAGES		+= images/adjustsize.png \
		images/edithlayoutsplit.png \
		images/left.png \
		images/sizeall.png \
		images/arrow.png \
		images/editlower.png \
		images/line.png \
		images/sizeb.png \
		images/background.png \
		images/editpaste.png \
		images/lineedit.png \
		images/sizef.png \
		images/book.png \
		images/editraise.png \
		images/listbox.png \
		images/sizeh.png \
		images/buttongroup.png \
		images/editslots.png \
		images/listview.png \
		images/sizev.png \
		images/checkbox.png \
		images/editvlayout.png \
		images/multilineedit.png \
		images/slider.png \
		images/combobox.png \
		images/editvlayoutsplit.png \
		images/newform.png \
		images/spacer.png \
		images/connecttool.png \
		images/filenew.png \
		images/no.png \
		images/spinbox.png \
		images/cross.png \
		images/fileopen.png \
		images/ordertool.png \
		images/splash.png \
		images/customwidget.png \
		images/filesave.png \
		images/pixlabel.png \
		images/table.png \
		images/databrowser.png \
		images/form.png \
		images/pointer.png \
		images/tabwidget.png \
		images/datatable.png \
		images/frame.png \
		images/print.png \
		images/textbrowser.png \
		images/dataview.png \
		images/groupbox.png \
		images/progress.png \
		images/textedit.png \
		images/dateedit.png \
		images/hand.png \
		images/project.png \
		images/textview.png \
		images/datetimeedit.png \
		images/help.png \
		images/pushbutton.png \
		images/timeedit.png \
		images/dial.png \
		images/home.png \
		images/logo.png \
		images/toolbutton.png \
		images/down.png \
		images/hsplit.png \
		images/radiobutton.png \
		images/undo.png \
		images/editbreaklayout.png \
		images/ibeam.png \
		images/redo.png \
		images/up.png \
		images/resetproperty.png \
		images/editcopy.png \
		images/iconview.png \
		images/resetproperty.png \
		images/uparrow.png \
		images/editcut.png \
		images/image.png \
		images/richtextedit.png \
		images/vsplit.png \
		images/editdelete.png \
		images/label.png \
		images/right.png \
		images/wait.png \
		images/editgrid.png \
		images/layout.png \
		images/scrollbar.png \
		images/widgetstack.png \
		images/edithlayout.png \
		images/lcdnumber.png \
		images/searchfind.png \
		images/folder.png \
		images/setbuddy.png \
		images/textbold.png \
		images/textcenter.png \
		images/texth1.png \
		images/texth2.png \
		images/texth3.png \
		images/textitalic.png \
		images/textjustify.png \
		images/textlarger.png \
		images/textleft.png \
		images/textlinebreak.png \
		images/textparagraph.png \
		images/textright.png \
		images/textsmaller.png \
		images/textteletext.png \
		images/textunderline.png \
		images/wizarddata.png \
		images/wizarddialog.png \
		images/disabled_adjustsize.png \
		images/disabled_label.png \
		images/disabled_book.png \
		images/disabled_layout.png \
		images/disabled_buttongroup.png \
		images/disabled_lcdnumber.png \
		images/disabled_checkbox.png \
		images/disabled_left.png \
		images/disabled_combobox.png \
		images/disabled_line.png \
		images/disabled_connecttool.png \
		images/disabled_lineedit.png \
		images/disabled_customwidget.png \
		images/disabled_listbox.png \
		images/disabled_databrowser.png \
		images/disabled_listview.png \
		images/disabled_datatable.png \
		images/disabled_multilineedit.png \
		images/disabled_dataview.png \
		images/disabled_newform.png \
		images/disabled_dateedit.png \
		images/disabled_ordertool.png \
		images/disabled_datetimeedit.png \
		images/disabled_pixlabel.png \
		images/disabled_dial.png \
		images/disabled_pointer.png \
		images/disabled_down.png \
		images/disabled_print.png \
		images/disabled_editbreaklayout.png \
		images/disabled_progress.png \
		images/disabled_editcopy.png \
		images/disabled_project.png \
		images/disabled_editcut.png \
		images/disabled_pushbutton.png \
		images/disabled_editdelete.png \
		images/disabled_radiobutton.png \
		images/disabled_editgrid.png \
		images/disabled_redo.png \
		images/disabled_edithlayout.png \
		images/disabled_richtextedit.png \
		images/disabled_edithlayoutsplit.png \
		images/disabled_right.png \
		images/disabled_editlower.png \
		images/disabled_scrollbar.png \
		images/disabled_editpaste.png \
		images/disabled_searchfind.png \
		images/disabled_editraise.png \
		images/disabled_slider.png \
		images/disabled_editslots.png \
		images/disabled_spacer.png \
		images/disabled_editvlayout.png \
		images/disabled_spinbox.png \
		images/disabled_editvlayoutsplit.png \
		images/disabled_table.png \
		images/disabled_filenew.png \
		images/disabled_folder.png \
		images/disabled_tabwidget.png \
		images/disabled_fileopen.png \
		images/disabled_textbrowser.png \
		images/disabled_filesave.png \
		images/disabled_textedit.png \
		images/disabled_form.png \
		images/disabled_textview.png \
		images/disabled_frame.png \
		images/disabled_timeedit.png \
		images/disabled_groupbox.png \
		images/disabled_toolbutton.png \
		images/disabled_help.png \
		images/disabled_undo.png \
		images/disabled_home.png \
		images/disabled_up.png \
		images/disabled_iconview.png \
		images/disabled_widgetstack.png \
		images/disabled_setbuddy.png \
		images/disabled_textbold.png \
		images/disabled_texth1.png \
		images/disabled_texth2.png \
		images/disabled_texth3.png \
		images/disabled_textitalic.png \
		images/disabled_textjustify.png \
		images/disabled_textlarger.png \
		images/disabled_textleft.png \
		images/disabled_textlinebreak.png \
		images/disabled_textparagraph.png \
		images/disabled_textright.png \
		images/disabled_textsmaller.png \
		images/disabled_textteletext.png \
		images/disabled_textunderline.png \
		images/disabled_textcenter.png \
		images/disabled_wizarddata.png \
		images/disabled_wizarddialog.png \
		images/disabled_image.png


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
