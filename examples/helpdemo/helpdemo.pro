GUID 	 = {2a668b0a-eddc-4c7f-bd14-4506968a68ec}
TEMPLATE = app

CONFIG	+= qt warn_on debug
LIBS    += -lqassistantclient
unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}

QTDIR_build:REQUIRES = full-config

SOURCES += helpdemo.cpp main.cpp
HEADERS += helpdemo.h
FORMS	 = helpdemobase.ui
