SOURCES	+= main.cpp
unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
FORMS	= mainform.ui \
	dialogform.ui \
	extension.ui
TEMPLATE	=app
CONFIG	+= qt warn_on release
DBFILE	= extension.db
LANGUAGE	= C++
