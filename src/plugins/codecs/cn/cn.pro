TEMPLATE = lib
TARGET	 = qcncodecs

CONFIG	+= qt warn_on plugin
DESTDIR	 = $$QT_BUILD_TREE/plugins/codecs

QTDIR_build:REQUIRES = "!contains(QT_CONFIG, bigcodecs)"

HEADERS		= ../../../../include/qgb18030codec.h \
		  ../../../../include/private/qfontcodecs_p.h

SOURCES		= ../../../../src/codecs/qgb18030codec.cpp \
		  ../../../../src/codecs/qfontcncodec.cpp \
		  main.cpp


target.path += $$plugins.path/codecs
INSTALLS += target
