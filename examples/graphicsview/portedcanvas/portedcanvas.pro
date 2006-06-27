TEMPLATE	= app
TARGET		= portedcanvas

CONFIG		+= qt warn_on release

HEADERS		= canvas.h
SOURCES		= canvas.cpp main.cpp
QT +=  qt3support 

RESOURCES += portedcanvas.qrc
