TEMPLATE	= app
CONFIG		+= qt warn_on release
# CONFIG	+= pim
HEADERS	= qcppsyntaxhighlighter.h \
		  qtextedit.h \
		  qrichtext_p.h \
		  qsimplerichtext.h \
		  
pim:HEADERS	+= ../../../qpim/words/qdawg.h qspellchecker.h
SOURCES	= main.cpp \
		  qcppsyntaxhighlighter.cpp \
		  qtextedit.cpp \
		  qrichtext.cpp \
		  qsimplerichtext.cpp \
		  
pim:SOURCES	+= ../../../qpim/words/qdawg.cpp qspellchecker.cpp
TARGET		= qtextedit
