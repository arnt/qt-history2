# Qt codecs module

tools {
	CODECS_P		= codecs
	HEADERS += $$TOOLS_H/qbig5codec.h \
		  $$TOOLS_H/qeucjpcodec.h \
		  $$TOOLS_H/qeuckrcodec.h \
		  $$TOOLS_H/qgbkcodec.h \
		  $$TOOLS_H/qjiscodec.h \
		  $$TOOLS_H/qjpunicode.h \
		  $$TOOLS_H/qrtlcodec.h \
		  $$TOOLS_H/qsjiscodec.h \
		  $$TOOLS_H/qtextcodec.h \
		  $$TOOLS_H/qtsciicodec.h \
		  $$TOOLS_H/qutfcodec.h

	SOURCES += $$TOOLS_CPP/qbig5codec.cpp \
		  $$TOOLS_CPP/qeucjpcodec.cpp \
		  $$TOOLS_CPP/qeuckrcodec.cpp \
		  $$TOOLS_CPP/qgbkcodec.cpp \
		  $$TOOLS_CPP/qjiscodec.cpp \
		  $$TOOLS_CPP/qjpunicode.cpp \
		  $$TOOLS_CPP/qrtlcodec.cpp \
		  $$TOOLS_CPP/qsjiscodec.cpp \
		  $$TOOLS_CPP/qtextcodec.cpp \
		  $$TOOLS_CPP/qtsciicodec.cpp \
		  $$TOOLS_CPP/qutfcodec.cpp
}
