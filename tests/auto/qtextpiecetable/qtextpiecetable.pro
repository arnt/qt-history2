load(qttest_p4)
SOURCES  += tst_qtextpiecetable.cpp

unix:contains(QT_CONFIG, reduce_exports) {
    DEFINES += QTEST_REDUCED_EXPORTS
}
win32 {
    DEFINES += QTEST_REDUCED_EXPORTS
}

