SOURCES	+= main.cpp 
unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
SOURCES += mainwindow.cpp dbconnection.cpp tableinfo.cpp distributionwidget.cpp
HEADERS += mainwindow.h dbconnection.h tableinfo.h distributionwidget.h
IMAGES	= images/table.png images/folder.png images/folder_open.png images/tablespace.png images/user.png images/database.png images/yes.png
TEMPLATE	=app
CONFIG	+= qt warn_on release
DBFILE	= dbview.db
LANGUAGE	= C++
