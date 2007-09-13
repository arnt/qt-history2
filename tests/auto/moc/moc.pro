load(qttest_p4)

#exists(/usr/include/boost/spirit.hpp) {
#    message("including boost headers in test")
#    DEFINES += PARSE_BOOST
#    # need to add explicitly so that it ends up in moc's search path
#    INCLUDEPATH += /usr/include
#}

DEFINES += SRCDIR=\\\"$$PWD\\\"
cross_compile: DEFINES += MOC_CROSS_COMPILED

HEADERS += using-namespaces.h no-keywords.h task87883.h c-comments.h backslash-newlines.h oldstyle-casts.h \
           slots-with-void-template.h qinvokable.h namespaced-flags.h trigraphs.h \
           escapes-in-string-literals.h cstyle-enums.h
if(*-g++*|*-icc*):!irix-*:!win32-g++: HEADERS += os9-newlines.h win-newlines.h
SOURCES += tst_moc.cpp

QT += sql network svg script
contains(QT_CONFIG, qt3support): QT += qt3support
contains(QT_CONFIG, qdbus){
    DEFINES += WITH_DBUS
    CONFIG += qdbus
}

DEFINES += QT_USE_USING_NAMESPACE

