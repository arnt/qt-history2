#always install the library
target.path=$$QT_INSTALL_LIBPATH
isEmpty(target.path):target.path=$$QT_PREFIX/lib
INSTALLS += target

#headers
isEmpty(headers.path):headers.path=$$QT_PREFIX/include
headers.files = ../include/*.h
isEmpty(headers_p.path):headers_p.path=$$headers.path/private
headers_p.files = ../include/private/*.h
INSTALLS += headers headers_p

#docs
isEmpty(docs.path):docs.path=$$QT_PREFIX/doc/html
docs.files = ../doc/html/*
INSTALLS += docs

macx { #mac framework
    framework.path = /System/Library/Frameworks/Qt.framework
    framework.extra  = -ln -sf $$docs.path /Developer/Documentation/Qt;
    framework.extra += ln -sf $$target.path/$(TARGET) $$framework.path/Qt;
    framework.extra += ln -sf $$headers.path $$framework.path/Headers;
    framework.extra += ln -sf $$headers_p.path $$framework.path/Headers/private
    INSTALLS += framework
}
