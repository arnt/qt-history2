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
#include "qdesigner_workbench.h"
#include "qdesigner_formwindow.h"

#include <sheet_delegate.h>

#include <abstractformwindow.h>

NewForm::NewForm(QDesignerWorkbench *workbench, QWidget *parentWidget)
    : QDialog(parentWidget),
      m_workbench(workbench)
{
    ui.setupUi(this);
    ui.treeWidget->setItemDelegate(new SheetDelegate(ui.treeWidget, this));
}

NewForm::~NewForm()
{
}

void NewForm::on_createButton_clicked()
{
    close();

    QDesignerFormWindow *formWindow = workbench()->createFormWindow();
    if (AbstractFormWindow *editor = formWindow->editor()) {
        editor->setContents(QString());
    }
}

void NewForm::on_closeButton_clicked()
{
    close();
}

QDesignerWorkbench *NewForm::workbench() const
{
    return m_workbench;
}

