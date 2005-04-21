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

#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <qdialog.h>

#include "ui_finddialog.h"

class MainWindow;
class QStatusBar;
class QTextBrowser;

class FindDialog : public QDialog
{
    Q_OBJECT
public:
    FindDialog(MainWindow *parent);
    virtual ~FindDialog();

    MainWindow *mainWindow() const;
    bool hasFindExpression() const;

public slots:
    void reset();
    void doFind(bool forward);
    void statusMessage(const QString &message);

private slots:
    void findButtonClicked();    

private:
    Ui::FindDialog ui;
    QWidget *contentsWidget;

    QStatusBar *sb;
    bool onceFound;
    QString findExpr;
    QTextBrowser *lastBrowser;
};

#endif

