x11 {
        contains(QT_CONFIG, nas): LIBS += -laudio -lXt
        contains(QT_CONFIG, xft): QMAKE_INCDIR_X11 *= $$FREETYPE2_INCDIR
        contains(QT_CONFIG, xft2header):DEFINES +=   XFT2_H
}
 