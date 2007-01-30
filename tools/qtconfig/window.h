#ifndef WINDOW_H
#define WINDOW_H

/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QMainWindow>
#include <QWidget>
#include <QApplication>
#include "ui_window.h"
class Window : public QMainWindow, public Ui_Window
{
    Q_OBJECT
public:
    Window(QWidget *parent = 0);
    void showEvent(QShowEvent *e);
    void closeEvent(QCloseEvent *e);
    bool eventFilter(QObject *o, QEvent *e);
public slots:
    void onCurrentTabIndexChanged(int index);
    void about();
    void aboutQt();
    void setModified(bool m = true);
    void save();
    void revert();
    void exit();
private:
    bool modified;
    QPalette editPalette;
};


#endif
