# Project ID used by some IDEs
GUID 	 = {a4228583-38d7-48bb-8c43-60fe2a3df9a0}
TEMPLATE = lib
TARGET	 = qcncodecs

CONFIG	+= qt warn_on plugin
DESTDIR	 = ../../../codecs

QTDIR_build:REQUIRES = !bigcodecs

HEADERS		= ../../../../include/qgb18030codec.h \
		  ../../../../include/private/qfontcodecs_p.h

SOURCES		= ../../../../src/codecs/qgb18030codec.cpp \
		  ../../../../src/codecs/qfontcncodec.cpp \
		  main.cpp


target.path += $$plugins.path/codecs
INSTALLS += target
