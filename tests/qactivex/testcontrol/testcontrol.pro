SOURCES	    = control.cpp
CONFIG	    += qt warn_off activeqt
TARGET	    = testcontrol

RC_FILE	    = $$QT_SOURCE_TREE/extensions/activeqt/control/qaxserver.rc

outproc {
  TEMPLATE    = app
}
inproc {
  TEMPLATE    = lib
  CONFIG     += dll
  DEF_FILE    = $$QT_SOURCE_TREE/extensions/activeqt/control/qaxserver.def
}
