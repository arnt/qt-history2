# Qt inputmethod module

contains(QT_CONFIG, inputmethod) {
	HEADERS +=inputmethod/qinputcontextfactory.h \
		  inputmethod/qinputcontextplugin.h 
	SOURCES +=inputmethod/qinputcontextfactory.cpp \
		  inputmethod/qinputcontextplugin.cpp 
	x11 {
		HEADERS += inputmethod/qximinputcontext_p.h
		SOURCES += inputmethod/qximinputcontext_x11.cpp
	}
}

embedded {
	HEADERS += inputmethod/qwsinputcontext_p.h
	SOURCES += inputmethod/qwsinputcontext_qws.cpp
}
