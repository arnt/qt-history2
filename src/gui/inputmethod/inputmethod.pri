# Qt inputmethod module

!mac {
    HEADERS +=inputmethod/qinputcontextfactory.h \
	      inputmethod/qinputcontextplugin.h \
	      inputmethod/qinputcontext.h
    SOURCES +=inputmethod/qinputcontextfactory.cpp \
	      inputmethod/qinputcontextplugin.cpp \
	      inputmethod/qinputcontext.cpp
    x11 {
	HEADERS += inputmethod/qximinputcontext_p.h
	SOURCES += inputmethod/qximinputcontext_x11.cpp
    }
    win32 {
	HEADERS += inputmethod/qwininputcontext_p.h
	SOURCES += inputmethod/qwininputcontext_win.cpp
    }
    embedded {
	HEADERS += inputmethod/qwsinputcontext_p.h
	SOURCES += inputmethod/qwsinputcontext_qws.cpp
    }
}

