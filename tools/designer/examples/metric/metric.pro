# Project ID used by some IDEs
GUID 	 = {691cf4db-bc40-4178-9b11-3a4b8bcaf829}
TEMPLATE = app
LANGUAGE = C++

SOURCES	+= main.cpp
unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
FORMS	= metric.ui
CONFIG	+= qt warn_on release
DBFILE	= metric.db
