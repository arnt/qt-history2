# Project ID used by some IDEs
GUID 	 = {b6e4d329-27a5-4367-aec1-35cf9ce18c74}
TEMPLATE = app
LANGUAGE = C++
TARGET   = receiver

CONFIG	+= qt warn_on release
INCLUDEPATH	+= $$QT_SOURCE_TREE/tools/designer/uilib
LIBS	+= -lqui
SOURCES	+= main.cpp
IMAGEFILE	= images.cpp
FORMS	= mainform.ui
DBFILE	= receiver.db
