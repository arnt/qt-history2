SOURCES	+= command.cpp formwindow.cpp defs.cpp layout.cpp main.cpp mainwindow.cpp mainwindowactions.cpp metadatabase.cpp pixmapchooser.cpp propertyeditor.cpp resource.cpp sizehandle.cpp orderindicator.cpp widgetfactory.cpp hierarchyview.cpp listboxeditorimpl.cpp connectioneditorimpl.cpp newformimpl.cpp workspace.cpp editslotsimpl.cpp listvieweditorimpl.cpp connectionviewerimpl.cpp customwidgeteditorimpl.cpp paletteeditorimpl.cpp styledbutton.cpp iconvieweditorimpl.cpp multilineeditorimpl.cpp formsettingsimpl.cpp asciivalidator.cpp splashloader.cpp designerapp.cpp designerappiface.cpp actioneditorimpl.cpp actionlistview.cpp actiondnd.cpp project.cpp projectsettingsimpl.cpp sourceeditor.cpp outputwindow.cpp ../shared/widgetdatabase.cpp ../shared/parser.cpp config.cpp pixmapcollection.cpp previewframe.cpp previewwidgetimpl.cpp paletteeditoradvancedimpl.cpp sourcefile.cpp filechooser.cpp wizardeditorimpl.cpp qcompletionedit.cpp timestamp.cpp formfile.cpp 
HEADERS	+= command.h defs.h formwindow.h layout.h mainwindow.h metadatabase.h pixmapchooser.h propertyeditor.h resource.h sizehandle.h orderindicator.h widgetfactory.h hierarchyview.h listboxeditorimpl.h connectioneditorimpl.h newformimpl.h workspace.h editslotsimpl.h listvieweditorimpl.h connectionviewerimpl.h customwidgeteditorimpl.h paletteeditorimpl.h styledbutton.h iconvieweditorimpl.h multilineeditorimpl.h formsettingsimpl.h asciivalidator.h splashloader.h ../interfaces/widgetinterface.h ../interfaces/actioninterface.h ../interfaces/filterinterface.h ../interfaces/designerinterface.h designerapp.h designerappiface.h actioneditorimpl.h actionlistview.h actiondnd.h project.h projectsettingsimpl.h sourceeditor.h outputwindow.h ../shared/widgetdatabase.h ../shared/parser.h config.h previewframe.h previewwidgetimpl.h paletteeditoradvancedimpl.h pixmapcollection.h sourcefile.h filechooser.h wizardeditorimpl.h qcompletionedit.h timestamp.h formfile.h 
OBJECTS_DIR	= .


include( ../../../src/qt_professional.pri )

TARGET	= designer
DEPENDPATH	+= $(QTDIR)/include
DESTDIR	= ../../../bin
win32-msvc:RC_FILE = designer.rc

sql {
	SOURCES  +=		  database.cpp dbconnectionimpl.cpp dbconnectionsimpl.cpp
	HEADERS += 		  database.h dbconnectionimpl.h dbconnectionsimpl.h   propertyeditorsql.h
	FORMS +=	dbconnections.ui dbconnection.ui dbconnectioneditor.ui 
	}
table {
	HEADERS += tableeditorimpl.h
	SOURCES += tableeditorimpl.cpp
      FORMS += tableeditor.ui 
	}

target.path=$$QT_INSTALL_BINPATH
isEmpty(target.path):target.path=$$QT_PREFIX/bin
INSTALLS        += target
PROJECTNAME	= Designer
FORMS	+= listboxeditor.ui connectioneditor.ui editslots.ui newform.ui listvieweditor.ui connectionviewer.ui customwidgeteditor.ui paletteeditor.ui iconvieweditor.ui preferences.ui multilineeditor.ui formsettings.ui about.ui pixmapfunction.ui createtemplate.ui actioneditor.ui projectsettings.ui finddialog.ui replacedialog.ui gotolinedialog.ui pixmapcollectioneditor.ui previewwidget.ui paletteeditoradvanced.ui wizardeditor.ui listeditor.ui 
TEMPLATE	=app
CONFIG	+= qt warn_on release
DEFINES	+= DESIGNER
INCLUDEPATH	+= ../shared ../uilib ../../../src/3rdparty/zlib/
win32:LIBS	+= $(QTDIR)/lib/qui.lib
unix:LIBS	+= -L$(QTDIR)/lib -lqui -lz
DBFILE	= designer.db
LANGUAGE	= C++
