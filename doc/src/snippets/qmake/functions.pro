EXTRAS = handlers tests docs
for(dir, EXTRAS) {
    exists($$dir) {
        SUBDIRS += $$dir
    }
}

SOURCES = paintwidget_mac.cpp paintwidget_unix.cpp paintwidget_win.cpp
macx {
    SOURCES = $$find(SOURCES, "_mac")
}

HEADERS = model.h
HEADERS += $$OTHER_HEADERS
HEADERS = $$unique(HEADERS)

CONFIG += debug
options = $$find(CONFIG, "debug") $$find(CONFIG, "release")
count(options, 2) {
    message(Both release and debug specified.)
}
