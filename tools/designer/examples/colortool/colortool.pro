SOURCES	+= main.cpp
unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
FORMS	= mainform.ui
IMAGES	= images/filenew \
	images/fileopen \
	images/filesave \
	images/print \
	images/undo \
	images/redo \
	images/editcut \
	images/editcopy \
	images/editpaste \
	images/searchfind \
	images/tabwidget.png \
	images/table.png \
	images/iconview.png \
	images/richtextedit.png \
	images/pixlabel.png
TEMPLATE	=app
CONFIG	+= qt warn_on release
DBFILE	= colortool.db
LANGUAGE	= C++
