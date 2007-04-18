# Include those manually as they do not contain any directory specification
APP_DIR = ..
FORMS += APP_DIR/helpdialog.ui \
         APP_DIR/mainwindow.ui \
         APP_DIR/tabbedbrowser.ui \
         APP_DIR/topicchooser.ui

SOURCES += APP_DIR/main.cpp \
           APP_DIR/helpwindow.cpp \
           APP_DIR/topicchooser.cpp \
           APP_DIR/docuparser.cpp \
           APP_DIR/index.cpp \
           APP_DIR/profile.cpp \
           APP_DIR/config.cpp \
           APP_DIR/helpdialog.cpp \
           APP_DIR/mainwindow.cpp \
           APP_DIR/tabbedbrowser.cpp \
           APP_DIR/fontsettingsdialog.cpp

SOURCES += ../../shared/fontpanel/fontpanel.cpp

HEADERS += APP_DIR/helpwindow.h \
           APP_DIR/topicchooser.h \
           APP_DIR/docuparser.h \
           APP_DIR/index.h \
           APP_DIR/profile.h \
           APP_DIR/helpdialog.h \
           APP_DIR/mainwindow.h \
           APP_DIR/tabbedbrowser.h \
           APP_DIR/config.h \
           APP_DIR/fontsettingsdialog.h


TRANSLATIONS=$$[QT_INSTALL_TRANSLATIONS]/assistant_de.ts $$[QT_INSTALL_TRANSLATIONS]/assistant_untranslated.ts
error("This is a dummy profile to be used for translations ONLY.")
