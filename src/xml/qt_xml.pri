# Qt xml module

xml {
	XML_P = xml
	HEADERS += $$XML_H/qxml.h $$XML_H/qdom.h $$XML_P/qsvgdevice_p.h
	SOURCES += $$XML_CPP/qxml.cpp $$XML_CPP/qdom.cpp $$XML_CPP/qsvgdevice.cpp
}
