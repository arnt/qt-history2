/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>

#include "dialog.h"
#include "wigglywidget.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
{
    WigglyWidget *wigglyWidget = new WigglyWidget;
    QLineEdit *lineEdit = new QLineEdit;

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(wigglyWidget);
    layout->addWidget(lineEdit);
    setLayout(layout);

    connect(lineEdit, SIGNAL(textChanged(QString)),
            wigglyWidget, SLOT(setText(QString)));

    lineEdit->setText(tr("Hello world!"));

    setWindowTitle(tr("Wiggly"));
    resize(360, 145);
}
