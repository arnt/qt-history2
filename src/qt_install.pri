#always install the library
target.path=$$QT_INSTALL_LIBPATH
isEmpty(target.path):target.path=$$QT_PREFIX/lib
INSTALLS += target

#headers
isEmpty(headers.path):headers.path=$$QT_PREFIX/headers
headers.files = ../include/*.h
INSTALLS += headers

#docs
isEmpty(docs.path):docs.path=$$QT_PREFIX/docs
docs.files = ../doc/*
INSTALLS += docs
