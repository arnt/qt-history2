TEMPLATE = app
INCLUDEPATH += .

# Input
HEADERS += ledmeter.h sprites.h toplevel.h view.h
SOURCES += ledmeter.cpp main.cpp toplevel.cpp view.cpp
#The following line was inserted by qt3to4
QT +=  qt3support 

HEADERS += animateditem.h
SOURCES += animateditem.cpp

RESOURCES += portedasteroids.qrc
