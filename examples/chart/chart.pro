# Project ID used by some IDEs
GUID 	 = {bc398457-9968-40c0-aafa-ba1fdbd5c463}
TEMPLATE = app

CONFIG  += warn_on

QTDIR_build:REQUIRES = full-config


HEADERS += element.h \
	   canvastext.h \
	   canvasview.h \
	   chartform.h \
	   optionsform.h \
	   setdataform.h
SOURCES += element.cpp \
	   canvasview.cpp \
	   chartform.cpp \
	   chartform_canvas.cpp \
	   chartform_files.cpp \
	   optionsform.cpp \
	   setdataform.cpp \
	   main.cpp
