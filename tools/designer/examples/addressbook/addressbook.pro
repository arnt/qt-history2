SOURCES	+= main.cpp 
unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
FORMS	= addressbook.ui addressdetails.ui search.ui 
IMAGES	= images/filenew images/fileopen images/filesave images/print images/undo images/redo images/editcut images/editcopy images/editpaste images/searchfind 
TEMPLATE	=app
CONFIG	+= qt warn_on release
DBFILE	= addressbook.db
LANGUAGE	= C++
