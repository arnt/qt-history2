TEMPLATE	= app
TARGET		= portedcanvas

CONFIG		+= qt warn_on

HEADERS		= canvas.h
SOURCES		= canvas.cpp main.cpp
QT +=  qt3support 

RESOURCES += portedcanvas.qrc
