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
    void applyStyle();
    void setStyle(const QString&);
    void loadLayout(const QString&);
    void loadEditor(const QString&);
    void editStyle();
    void previewStyleSheet();

private:
    QMainWindow *mw;
    QDialog *styleEditorDialog;

    Ui::StyleSheetEditor sse;
};

#endif
