!x11:mac {
   macx-g++:QMAKE_LFLAGS_PREBIND    = -seg1addr 0x20000000
   macx-pbuilder:PRECOMPH = kernel/qt_mac.pch
   else:contains(QT_PRODUCT, qt-internal):native_precompiled_headers:contains($$list($$system(c++ -v 2>&1)), 3.3):message(oink):PRECOMPH = kernel/qt_mac.pch
   LIBS += -framework Carbon -framework QuickTime -lz
   *-mwerks:INCLUDEPATH += compat
   DEFINES += QMAC_ONE_PIXEL_LOCK
}
