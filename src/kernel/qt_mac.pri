!x11:mac {
   macx-g++:QMAKE_LFLAGS_PREBIND    = -seg1addr 0x20000000
   LIBS += -framework Carbon -framework QuickTime -lz
   *-mwerks:INCLUDEPATH += compat
   DEFINES += QMAC_ONE_PIXEL_LOCK
}
