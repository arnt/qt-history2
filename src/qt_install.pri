#always install the library
target.path=$$libs.path
INSTALLS += target

#headers
headers.files = ../include/*.h
isEmpty(headers_p.path):headers_p.path=$$headers.path/private
headers_p.files = ../include/private/*.h
INSTALLS += headers headers_p

#docs
htmldocs.files = ../doc/html/*
htmldocs.path = $$docs.path/html
INSTALLS += htmldocs

macx { #mac framework
    framework.path = /System/Library/Frameworks/Qt.framework
    framework.extra  = -cp -rf $$docs.path /Developer/Documentation/Qt;
    framework.extra += cp -rf $$target.path/$(TARGET) $$framework.path/Qt;
    framework.extra += cp -rf $$headers.path $$framework.path/Headers;
    framework.extra += cp -rf $$headers_p.path $$framework.path/Headers/private
    INSTALLS += framework
}
