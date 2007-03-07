/****************************************************************************
**
** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef ADDBUTTONDIALOG_H
#define ADDBUTTONDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>

#include "ui_addbuttondialog.h"

class AddButtonDialog : public QDialog, Ui::AddButtonDialog
{
    Q_OBJECT

public:
    AddButtonDialog(QDialogButtonBox *box, QWidget *parent = 0);

    static QString roleToString(QDialogButtonBox::ButtonRole role);

public slots:
    void addButton();

private slots:
    void standardButtonSelected(const QString &text);

private:
    void fillStandardButtonCombo();
    void fillCustomButtonCombo();
    bool contains(QAbstractButton *button, QList<QAbstractButton *> buttons);

    QDialogButtonBox *dialogBox;
};

#endif
