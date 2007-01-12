
TEMPLATE = app
QT = core
TARGET = qlalr

SOURCES += compress.cpp \
    cppgenerator.cpp \
    dotgraph.cpp \
    lalr.cpp \
    main.cpp \
    parsetable.cpp \
    recognizer.cpp \
    grammar.cpp

HEADERS += compress.h \
    cppgenerator.h \
    dotgraph.h \
    lalr.h \
    parsetable.h \
    grammar_p.h
