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


#include "finddialog.h"
#include "mainwindow.h"
#include "tabbedbrowser.h"
#include "helpwindow.h"

#include <qtextbrowser.h>
#include <qstatusbar.h>
#include <qlineedit.h>

FindDialog::FindDialog(MainWindow *parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    lastBrowser = 0;
    onceFound = false;
    findExpr.clear();
    sb = new QStatusBar(this);

    if (layout())
        layout()->addWidget(sb);

    sb->message(tr("Enter the text you are looking for."));
}

FindDialog::~FindDialog()
{
}

void FindDialog::on_findButton_clicked()
{
    doFind(ui.radioForward->isChecked());
}

void FindDialog::on_closeButton_clicked()
{
    reject();
}

void FindDialog::doFind(bool forward)
{
    QTextBrowser *browser = static_cast<QTextBrowser*>(mainWindow()->browsers()->currentBrowser());
    sb->clear();

    if (ui.comboFind->currentText() != findExpr || lastBrowser != browser)
        onceFound = false;
    findExpr = ui.comboFind->currentText();

    bool found;
    if (browser->hasSelectedText()) { // Search either forward or backward from cursor.
        found = browser->find(findExpr, ui.checkCase->isChecked(), ui.checkWords->isChecked(),
                              forward);
    } else {
        int para = forward ? 0 : INT_MAX;
        int index = forward ? 0 : INT_MAX;
        found = browser->find(findExpr, ui.checkCase->isChecked(), ui.checkWords->isChecked(),
                              forward, &para, &index);
    }

    if (!found) {
        if (onceFound) {
            if (forward)
                statusMessage(tr("Search reached end of the document"));
            else
                statusMessage(tr("Search reached start of the document"));
        } else {
            statusMessage(tr( "Text not found" ));
        }
    }
    onceFound |= found;
    lastBrowser = browser;
}

bool FindDialog::hasFindExpression() const
{
    return !findExpr.isEmpty();
}

void FindDialog::statusMessage(const QString &message)
{
    if (isVisible())
        sb->message(message);
    else
        static_cast<MainWindow*>(parent())->statusBar()->message(message, 2000);
}

MainWindow *FindDialog::mainWindow() const
{
    return static_cast<MainWindow*>(parentWidget());
}

void FindDialog::reset()
{
    ui.comboFind->setFocus();
    ui.comboFind->lineEdit()->setSelection(
        0, ui.comboFind->lineEdit()->text().length());
}

