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

#include "newform.h"

NewForm::NewForm(QWidget *parentWidget)
    : QDialog(parentWidget)
{
    ui.setupUi(this);
}

NewForm::~NewForm()
{
}

void NewForm::on_createButton_clicked()
{
    close();
}

void NewForm::on_closeButton_clicked()
{
    close();
}

