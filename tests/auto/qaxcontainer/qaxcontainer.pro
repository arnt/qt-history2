load(qttest_p4)


SOURCES += tst_qaxcontainer.cpp

win32 {
    exists($$(QTDIR)/bin/dumpcpp.exe) {
        CONFIG += qaxcontainer

        MULTIPLEAX = $$system(dumpcpp -getfile {98DE28B6-6CD3-4e08-B9FA-3D1DB43F1D2F})

        exists($$MULTIPLEAX) {
            TYPELIBS += $$MULTIPLEAX
        }

        TYPELIBS += $$(SystemRoot)/system32/dmocx.dll
        # If there's a C:\Program Files (x86)\Common Files, then rather use that, since we're on a 64bit OS
        exists("C:\Program Files (x86)\Common Files\Microsoft Shared\Speech\sapi.dll") {
            TYPELIBS += "C:\Program Files (x86)\Common Files\Microsoft Shared\Speech\sapi.dll"
        } else:exists("C:\Program Files\Common Files\Microsoft Shared\Speech\sapi.dll") {
            TYPELIBS += "C:\Program Files\Common Files\Microsoft Shared\Speech\sapi.dll"
        }

        exists($$(SystemRoot)/system32/wmp.dll) {
            DEFINES  += TEST_WMP
            TYPELIBS += $$(SystemRoot)/system32/wmp.dll
        }

        EXCEL_TYPELIB = $$system(dumpcpp -getfile {00020813-0000-0000-C000-000000000046})
        !isEmpty(EXCEL_TYPELIB): {
            DEFINES += TEST_EXCEL
            TYPELIBS += $$EXCEL_TYPELIB
        } else {
            message("Skipping test for Excel meta object, as Excel is not installed")
        }
    } else {
        DEFINES += NO_DUMPCPP
        message("dumpcpp is not installed")
    }
}
