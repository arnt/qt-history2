x11 {
        contains(QT_CONFIG, nas): LIBS += -laudio -lXt
        contains(QT_CONFIG, fontconfig): QMAKE_INCDIR_X11 *= $$FREETYPE2_INCDIR
}
 
