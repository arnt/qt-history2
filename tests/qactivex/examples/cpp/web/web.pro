SOURCES	+= main.cpp 
unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
FORMS	= mainwindow.ui 
TEMPLATE	=app
CONFIG	+= qt warn_on release
INCLUDEPATH	+= ../../..
LIBS	+= $(QTDIR)/plugins/designer/qactivex.lib
DBFILE	= web.db
LANGUAGE	= C++
