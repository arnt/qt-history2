#always install the library
target.path=$$libs.path
INSTALLS += target

#headers
headers.files  = $$QT_BUILD_TREE/include/*.h
headers.files += $$QT_BUILD_TREE/include/qconfig.h \
                 $$QT_BUILD_TREE/include/qmodules.h 
INSTALLS += headers

macx { #mac framework
    QtFramework = /System/Library/Frameworks/$${TARGET}.framework
    framework.path = $$QtFramework/Headers
    framework.extra += cp -rf $$target.path/$(TARGET) $(INSTALL_ROOT)/$$QtFramework/$${TARGET};
    framework.extra += cp -rf $$headers.files $(INSTALL_ROOT)/$$QtFramework/Headers;
    INSTALLS += framework
}
