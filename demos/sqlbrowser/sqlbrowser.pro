TEMPLATE        = app
TARGET          = sqlbrowser

QT              = core gui sql

CONFIG         += debug

HEADERS         = browserwidget.h connectionwidget.h qsqlconnectiondialog.h
SOURCES         = main.cpp browserwidget.cpp connectionwidget.cpp qsqlconnectiondialog.cpp

FORMS           = qsqlconnectiondialog.ui

