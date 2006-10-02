load(qttest_p4)
TEMPLATE = app
DEPENDPATH += parser
INCLUDEPATH += . parser 

# Input
HEADERS +=  parser/parser.h
SOURCES += tst_qxmlsimplereader.cpp parser/parser.cpp 

QT += xml
CONFIG += no_batch
QT += qt3support
QT += network
