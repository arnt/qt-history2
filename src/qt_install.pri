#always install the library
target.path=$$libs.path
INSTALLS += target

#headers
headers.files  = ../include/*.h
headers.files += $$QT_BUILD_TREE/include/qconfig.h \
                 $$QT_BUILD_TREE/include/qmodules.h 
isEmpty(headers_p.path):headers_p.path=$$headers.path/private
headers_p.files = ../include/private/*.h
INSTALLS += headers headers_p

#docs
htmldocs.files = ../doc/html/*
htmldocs.path = $$docs.path/html
INSTALLS += htmldocs

macx { #mac framework
    QtFramework = /System/Library/Frameworks/Qt.framework
    QtDocs      = /Developer/Documentation/Qt
    framework.path = $$QtFramework/Headers/private $$QtDocs
    framework.extra  = -cp -rf $$htmldocs.files $$QtDocs;
    framework.extra += cp -rf $$target.path/$(TARGET) $$QtFramework/Qt;
    framework.extra += cp -rf $$headers.files $$QtFramework/Headers;
    framework.extra += cp -rf $$headers_p.files $$QtFramework/Headers/private
    INSTALLS += framework
}
