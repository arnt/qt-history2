TEMPLATE = app

CONFIG	+= qt warn_off
LIBS	+= -lqaxcontainer

QTDIR_build:REQUIRES = shared

SOURCES	 = main.cpp
FORMS	 = mainwindow.ui
