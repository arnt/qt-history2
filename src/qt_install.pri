#always install the library
target.path=$$libs.path
INSTALLS += target

#headers
headers.files  = $$QT_BUILD_TREE/include/*.h
headers.files += $$QT_BUILD_TREE/include/qconfig.h \
                 $$QT_BUILD_TREE/include/qmodules.h 
isEmpty(headers_p.path):headers_p.path=$$headers.path/private
headers_p.files = $$QT_BUILD_TREE/include/private/*.h
headers_arch.files = $$ARCH_H/qatomic.h
headers_arch.path = $$headers.path/arch
INSTALLS += headers headers_p headers_arch

#docs
htmldocs.files = $$QT_BUILD_TREE/doc/html/*
htmldocs.path = $$docs.path/html
INSTALLS += htmldocs

#translations
translations.files = $$QT_BUILD_TREE/translations/*.qm
INSTALLS += translations

macx { #mac framework
    QtFramework = /System/Library/Frameworks/Qt.framework
    QtDocs      = /Developer/Documentation/Qt
    framework.path = $$QtFramework/Headers/private $$QtDocs
    framework.extra  = -cp -rf $$htmldocs.files $(INSTALL_ROOT)/$$QtDocs;
    framework.extra += cp -rf $$target.path/$(TARGET) $(INSTALL_ROOT)/$$QtFramework/Qt;
    framework.extra += cp -rf $$headers.files $(INSTALL_ROOT)/$$QtFramework/Headers;
    framework.extra += cp -rf $$headers_p.files $(INSTALL_ROOT)/$$QtFramework/Headers/private
    INSTALLS += framework
}
