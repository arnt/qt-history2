TEMPLATE        = app
TARGET          = sqlbrowser

QT              += sql

HEADERS         = browser.h connectionwidget.h qsqlconnectiondialog.h
SOURCES         = main.cpp browser.cpp connectionwidget.cpp qsqlconnectiondialog.cpp

FORMS           = browserwidget.ui qsqlconnectiondialog.ui
build_all:CONFIG += release

# install
target.path = $$[QT_INSTALL_DATA]/demos/sqlbrowser
sources.files = $$SOURCES $$HEADERS $$FORMS *.pro
sources.path = $$[QT_INSTALL_DATA]/demos/sqlbrowser
INSTALLS += target sources
