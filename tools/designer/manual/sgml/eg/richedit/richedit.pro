SOURCES	+= main.cpp 
FORMS	= richedit.ui 
IMAGES	= images/editcopy images/editcut images/editpaste images/filenew images/fileopen images/filesave images/print images/redo images/searchfind images/textbold images/textcenter images/textitalic images/textleft images/textright images/textunder images/undo 
TEMPLATE	=app
CONFIG	+= qt warn_on release
DBFILE	= richedit.db
LANGUAGE	= C++
unix {
 UI_DIR = .ui
 MOC_DIR = .moc
 OBJECTS_DIR = .obj
}
