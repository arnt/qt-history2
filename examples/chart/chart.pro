TEMPLATE = app 
CONFIG  += warn_on 

HEADERS +=  element.h \
	    canvastext.h \
	    canvasview.h \
	    chartform.h \
	    optionsform.h \
	    setdataform.h 
SOURCES +=  element.cpp \
	    canvasview.cpp \
	    chartform.cpp \
	    chartform_canvas.cpp \
	    chartform_files.cpp \
	    optionsform.cpp \
	    setdataform.cpp \
	    main.cpp
QTDIR_build:REQUIRES=full-config
