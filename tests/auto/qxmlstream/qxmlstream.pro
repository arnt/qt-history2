load(qttest_p4)
SOURCES += tst_qxmlstream.cpp 

QT = core xml
mac {
    CONFIG -= app_bundle
}