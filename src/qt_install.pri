#always install the library
isEmpty(INSTALLtarget_PATH):INSTALLtarget_PATH=/home/sam/blah/qt/lib
INSTALLS += target

#headers
isEmpty(INSTALLheaders_PATH):INSTALLheaders_PATH=$$INSTALLtarget_PATH/../headers
INSTALLheaders = ../include/*.h
INSTALLS += headers

#plugins
isEmpty(INSTALLplugins_PATH):INSTALLplugins_PATH=$$INSTALLtarget_PATH/../plugins
INSTALLplugins = $(QTDIR)/plugins/*
INSTALLS += plugins

#docs
isEmpty(INSTALLdocs_PATH):INSTALLdocs_PATH=$$INSTALLtarget_PATH/../docs
INSTALLdocs = ../doc/*
INSTALLS += docs
