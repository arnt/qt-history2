GUID 	 = {6a820438-e5d8-46d9-9b8b-0e9e2467922a}
TEMPLATE = app

CONFIG	+= qt warn_off
LIBS	+= -lqaxcontainer

QTDIR_build:REQUIRES = shared

SOURCES	 = main.cpp
FORMS	 = mainwindow.ui
