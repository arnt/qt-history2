#always install the library
target.path=$$QT_INSTALL_LIBPATH
isEmpty(target.path):target.path=$$QT_PREFIX/lib
INSTALLS += target

#headers
isEmpty(headers.path):headers.path=$$QT_PREFIX/include
headers.files = ../include/*.h
isEmpty(headersp.path):headersp.path=$$QT_PREFIX/include/private
headersp.files = ../include/private/*.h
INSTALLS += headers headersp

#docs
isEmpty(docs.path):docs.path=$$QT_PREFIX/doc/html
docs.files = ../doc/html/*
INSTALLS += docs
