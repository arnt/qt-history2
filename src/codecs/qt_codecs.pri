# Qt codecs module

tools {
	CODECS_P = codecs
	HEADERS += $$CODECS_H/qrtlcodec.h \
		   $$CODECS_H/qtextcodec.h \
		   $$CODECS_H/qtsciicodec.h \
		   $$CODECS_H/qutfcodec.h \
		   $$CODECS_H/qtextcodecinterface.h \
		   $$CODECS_H/qtextcodecfactory.h

	SOURCES += $$CODECS_CPP/qrtlcodec.cpp \
		   $$CODECS_CPP/qtextcodec.cpp \
		   $$CODECS_CPP/qtsciicodec.cpp \
		   $$CODECS_CPP/qutfcodec.cpp \
		   $$CODECS_CPP/qtextcodecfactory.cpp
}
