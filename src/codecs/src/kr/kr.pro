TEMPLATE	= lib
CONFIG		= qt warn_on plugin

HEADERS		= $(QTDIR)/src/codecs/qeuckrcodec.h \
		  $(QTDIR)/src/codecs/qfontcodecs.h

SOURCES		= $(QTDIR)/src/codecs/qeuckrcodec.cpp \
		  $(QTDIR)/src/codecs/qfontkrcodec.cpp \
		  main.cpp

TARGET		= qkrcodecs
DESTDIR		= ../../../../plugins/codecs

target.path=$$plugins.path
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/codecs
INSTALLS += target

