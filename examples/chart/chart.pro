TEMPLATE = app

CONFIG  += warn_on

QTDIR_build:REQUIRES = "contains(QT_CONFIG, full-config)"


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
