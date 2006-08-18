load(qttest_p4)

exists(/usr/include/boost/spirit.hpp) {
    message("including boost headers in test")
    DEFINES += PARSE_BOOST
    # need to add explicitly so that it ends up in moc's search path
    INCLUDEPATH += /usr/include
}

DEFINES += SRCDIR=\\\"$$PWD\\\"

HEADERS += using-namespaces.h no-keywords.h task87883.h c-comments.h backslash-newlines.h oldstyle-casts.h \
           slots-with-void-template.h qinvokable.h namespaced-flags.h
SOURCES += tst_moc.cpp

QT += sql network svg
contains(QT_CONFIG, qt3support): QT += qt3support
