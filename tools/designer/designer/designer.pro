TEMPLATE	= app
CONFIG		+= qt warn_on release
OBJECTS_DIR	= .
HEADERS	= command.h \
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
		  connectioneditorimpl.h \
		  newformimpl.h \
		  formlist.h \
		  help.h \
		  editslotsimpl.h \
		  listvieweditorimpl.h \
		  connectionviewerimpl.h \
		  customwidgeteditorimpl.h \
		  paletteeditorimpl.h \
		  styledbutton.h \
		  previewstack.h \
		  iconvieweditorimpl.h \
		  helpdialogimpl.h \
		  topicchooserimpl.h \
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
		  tableeditorimpl.h \
		  sourceeditor.h \
		  outputwindow.h \
		  ../shared/widgetdatabase.h \
		  ../shared/parser.h \
		  config.h

SOURCES	= command.cpp \
		  formwindow.cpp \
		  defs.cpp \
		  layout.cpp \
		  main.cpp \
		  mainwindow.cpp \
		  metadatabase.cpp \
		  pixmapchooser.cpp \
		  propertyeditor.cpp \
		  resource.cpp \
		  sizehandle.cpp \
		  orderindicator.cpp \
		  widgetfactory.cpp \
		  hierarchyview.cpp \
		  listboxeditorimpl.cpp \
		  connectioneditorimpl.cpp \
		  newformimpl.cpp \
		  formlist.cpp \
		  help.cpp \
		  editslotsimpl.cpp \
		  listvieweditorimpl.cpp \
		  connectionviewerimpl.cpp \
		  customwidgeteditorimpl.cpp \
		  paletteeditorimpl.cpp \
		  styledbutton.cpp \
		  previewstack.cpp \
		  iconvieweditorimpl.cpp \
		  helpdialogimpl.cpp \
		  topicchooserimpl.cpp \
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
		  tableeditorimpl.cpp \
		  sourceeditor.cpp \
		  outputwindow.cpp \
		  ../shared/widgetdatabase.cpp \
		  ../shared/parser.cpp \
		  config.cpp

TARGET	= designer
INCLUDEPATH	= ../shared ../util ../resource ../../../src/3rdparty/zlib/
unix:LIBS	+= -lqutil -L$(QTDIR)/lib -lqresource
win32:LIBS	+= $(QTDIR)/lib/qutil.lib $(QTDIR)/lib/qresource.lib
DEFINES	+= DESIGNER
DESTDIR	= $(QTDIR)/bin
win32-msvc:RC_FILE = designer.rc
INTERFACES	= listboxeditor.ui connectioneditor.ui editslots.ui newform.ui listvieweditor.ui connectionviewer.ui customwidgeteditor.ui texteditpreview.ui listviewpreview.ui paletteeditor.ui radiopreview.ui checkboxpreview.ui widgetpreview.ui iconvieweditor.ui preferences.ui helpdialog.ui topicchooser.ui multilineeditor.ui formsettings.ui about.ui pixmapfunction.ui createtemplate.ui actioneditor.ui projectsettings.ui tableeditor.ui finddialog.ui replacedialog.ui 
PROJECTNAME	= Designer

sql {
	SOURCES  +=		  database.cpp		  dbconnectionsimpl.cpp
	HEADERS += 		  database.h		  dbconnectionsimpl.h   propertyeditorsql.h
	INTERFACES +=             dbconnections.ui
}

target.path=$$QT_INSTALL_BINPATH
isEmpty(target.path):target.path=$$QT_PREFIX/bin
INSTALLS        += target
