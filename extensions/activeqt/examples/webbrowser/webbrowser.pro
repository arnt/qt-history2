TEMPLATE = app

CONFIG	+= uic3
QT      += compat
LIBS	+= -lqaxcontainer

QTDIR_build:REQUIRES = shared

SOURCES	 = main.cpp
FORMS	 = mainwindow.ui
