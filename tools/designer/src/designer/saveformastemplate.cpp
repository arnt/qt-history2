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

#include "saveformastemplate.h"

SaveFormAsTemplate::SaveFormAsTemplate(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
}

SaveFormAsTemplate::~SaveFormAsTemplate()
{
}

void SaveFormAsTemplate::on_okButton_clicked()
{
    accept();
}

void SaveFormAsTemplate::on_cancelButton_clicked()
{
    reject();
}

