TEMPLATE	= lib
CONFIG		+= qt warn_on plugin

HEADERS		= $(QTDIR)/src/codecs/qgbkcodec.h \
		  $(QTDIR)/src/codecs/qfontcodecs_p.h

SOURCES		= $(QTDIR)/src/codecs/qgbkcodec.cpp \
		  $(QTDIR)/src/codecs/qfontcncodec.cpp \
		  main.cpp

TARGET		= qcncodecs
DESTDIR		= ../../../../plugins/codecs

target.path=$$plugins.path
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/codecs
INSTALLS += target

