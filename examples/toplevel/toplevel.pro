# Project ID used by some IDEs
GUID 	 = {980c9f2c-f5e6-4194-a675-eebb288cddf5}
TEMPLATE = app
LANGUAGE = C++

CONFIG	+= qt warn_on release
unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}

REQUIRES = full-config

SOURCES	+= main.cpp
FORMS	 = options.ui
