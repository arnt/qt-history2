# Qt codecs module

tools {
	CODECS_P		= codecs
	HEADERS += $$CODECS_H/qbig5codec.h \
		  $$CODECS_H/qeucjpcodec.h \
		  $$CODECS_H/qeuckrcodec.h \
		  $$CODECS_H/qgbkcodec.h \
		  $$CODECS_H/qjiscodec.h \
		  $$CODECS_H/qjpunicode.h \
		  $$CODECS_H/qrtlcodec.h \
		  $$CODECS_H/qsjiscodec.h \
		  $$CODECS_H/qtextcodec.h \
		  $$CODECS_H/qtsciicodec.h \
		  $$CODECS_H/qutfcodec.h \
		   $$CODECS_H/qtextcodecinterface.h \
		   $$CODECS_H/qtextcodecfactory.h

	SOURCES += $$CODECS_CPP/qbig5codec.cpp \
		  $$CODECS_CPP/qeucjpcodec.cpp \
		  $$CODECS_CPP/qeuckrcodec.cpp \
		  $$CODECS_CPP/qgbkcodec.cpp \
		  $$CODECS_CPP/qjiscodec.cpp \
		  $$CODECS_CPP/qjpunicode.cpp \
		  $$CODECS_CPP/qrtlcodec.cpp \
		  $$CODECS_CPP/qsjiscodec.cpp \
		  $$CODECS_CPP/qtextcodec.cpp \
		  $$CODECS_CPP/qtsciicodec.cpp \
		  $$CODECS_CPP/qutfcodec.cpp \
		   $$CODECS_CPP/qtextcodecfactory.cpp
}
