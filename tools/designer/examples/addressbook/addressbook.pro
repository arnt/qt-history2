SOURCES	+= main.cpp 
unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
FORMS	= addressbook.ui addressdetails.ui search.ui 
IMAGES	= images/filenew.png images/fileopen.png images/filesave.png images/searchfind.png images/editcut.png
TEMPLATE	=app
CONFIG	+= qt warn_on release
DBFILE	= addressbook.db
LANGUAGE	= C++
