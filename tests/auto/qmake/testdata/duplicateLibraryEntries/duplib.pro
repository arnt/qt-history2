LIBS = -lqui -lqtest -lqui
CONFIG -= link_prl
JOINEDLIBS = $$join(LIBS, "_")

!contains(JOINEDLIBS, -lqui_-lqtest_-lqui) {
   message("FAILED: duplibs")
}


DEFINES += QT_USE_USING_NAMESPACE

