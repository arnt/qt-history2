# Project ID used by some IDEs
GUID 	 = {683ee727-4157-45d8-a244-729cacf22006}
CONFIG += qt console
SOURCES += keyinfo.cpp
keychk {
  TEMPLATE = lib
  TARGET = keychk
  DESTDIR = ../../../lib/
  CONFIG -= shared dll
  CONFIG += staticlib
} else {
  TEMPLATE = app
  TARGET = keygen
  SOURCES += main.cpp
}