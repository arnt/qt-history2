# Project ID used by some IDEs
GUID 	 = {2a835e60-2ab0-4ade-9691-de69dfee633a}
TEMPLATE = app
LANGUAGE = C++

SOURCES	+= main.cpp
unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
FORMS	= mainform.ui \
	colornameform.ui \
	findform.ui \
	optionsform.ui
IMAGES	= images/filenew.png \
	images/fileopen.png \
	images/filesave.png \
	images/editcut.png \
	images/editcopy.png \
	images/searchfind.png \
	images/tabwidget.png \
	images/table.png \
	images/iconview.png \
	images/richtextedit.png \
	images/widgetstack.png \
	images/editraise.png
CONFIG	+= qt warn_on release
DBFILE	= colortool.db
