#always install the library
target.path=$$QT_INSTALL_LIBPATH
isEmpty(target.path):target.path=$$QT_PREFIX/lib
INSTALLS += target

#headers
isEmpty(headers.path):headers.path=$$QT_PREFIX/include
headers.files = ../include/*.h ../include/private
INSTALLS += headers

#docs
isEmpty(docs.path):docs.path=$$QT_PREFIX/doc/html
docs.files = ../doc/html/*
INSTALLS += docs
