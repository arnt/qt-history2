TARGET   = patternistdocexamples
TEMPLATE = app
CONFIG  += qtestlib
SOURCES  = DoxygenExamplesUnitTests.cpp

HEADERS = DoxygenExamplesUnitTests.h    \
          Docs.h

include(../common.pri)

LIBS += -lQtPatternist
