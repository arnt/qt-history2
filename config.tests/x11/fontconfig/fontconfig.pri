for(p, QMAKE_INCDIR) {
    p = $$join(p, "", "", "/freetype2")
    exists($$p):INCLUDEPATH *= $$p
}
# freetype2 can (optionally) be installed together with X11
for(p, QMAKE_INCDIR_X11) {
    p = $$join(p, "", "", "/freetype2")
    exists($$p):INCLUDEPATH *= $$p
}
for(p, INCLUDEPATH) {
    p = $$join(p, "", "", "/freetype2")
    exists($$p):INCLUDEPATH *= $$p
}
