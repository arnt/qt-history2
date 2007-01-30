#ifndef PRINTERPAGE_H
#define PRINTERPAGE_H

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

#include <QFrame>
#include <QWidget>
#include <QKeyEvent>
#include "ui_printerpage.h"
class PrinterPage : public QFrame, public Ui_PrinterPage
{
    Q_OBJECT
public:
    PrinterPage(QWidget *parent = 0);
    void save();
    void load();
public slots:
    void remove();
    void onBrowseClicked();
    void add();
    void keyPressEvent(QKeyEvent *e);
    void onLineEditTextChanged(const QString &str);
    void onCurrentChanged();
signals:
    void changed();
};

#endif
