TEMPLATE = app
LANGUAGE = C++

SOURCES += finddialog.cpp

TRANSLATIONS        += project_no.ts

exists( $$TRANSLATIONS ) {
    win32 : RES = $$system(del $$TRANSLATIONS)
    unix : RES = $$system(rm $$TRANSLATIONS)
}

