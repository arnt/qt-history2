HEADERS += Test.h
SOURCES += main.cpp Test.cpp
QT += network qt3support

CONFIG -= app_bundle
CONFIG += console
DESTDIR = ./
MOC_DIR = .moc/
TMP_DIR = .tmp/
