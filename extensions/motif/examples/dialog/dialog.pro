# Project ID used by some IDEs
GUID 	 = {c6f29521-fa7e-4eca-8281-14aef3a4caea}
TEMPLATE = app

CONFIG  += qt x11 warn_on
LIBS    += -lqmotif

HEADERS = mainwindow.h dialog.h
SOURCES = main.cpp mainwindow.cpp dialog.cpp
