TEMPLATE    =app
CONFIG	    += qt warn_on debug
SOURCES	    += helpdemo.cpp main.cpp
HEADERS	    += helpdemo.h
FORMS	    = helpdemobase.ui

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}

win32{
	win32-g++ {
	   LIBS  += $$QT_BUILD_TREE/lib/libqassistantclient.a
	} else :LIBS  += $$QT_BUILD_TREE/lib/qassistantclient.lib
}
unix:LIBS   += $$QT_BUILD_TREE/lib/libqassistantclient.a
