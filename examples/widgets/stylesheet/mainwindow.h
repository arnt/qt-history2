/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>

#include "ui_stylesheeteditor.h"

class MainWindow : public QObject
{
    Q_OBJECT

public:
    MainWindow();

private slots:
    void about();
    void loadStyleSheet(const QString&);
    void editStyle();
    void previewStyleSheet();
    void setStyle(const QString &style);

private:
    void loadLayout(const QString&);

    QMainWindow *mw;
    QDialog *styleEditorDialog;

    Ui::StyleSheetEditor sse;
    QString currentLayout;
};

#endif
