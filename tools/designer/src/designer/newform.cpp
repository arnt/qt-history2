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

#include <QtCore/QFile>
#include <QtGui/QHeaderView>

#include <QtCore/qdebug.h>

enum
{
    TemplateNameRole = QAbstractItemModel::UserRole + 100
};

NewForm::NewForm(QDesignerWorkbench *workbench, QWidget *parentWidget)
    : QDialog(parentWidget),
      m_workbench(workbench)
{
    ui.setupUi(this);
    ui.treeWidget->setItemDelegate(new SheetDelegate(ui.treeWidget, this));
    ui.treeWidget->header()->hide();

    QTreeWidgetItem *trolltech = new QTreeWidgetItem(ui.treeWidget);
    trolltech->setText(0, tr("Trolltech"));

    QTreeWidgetItem *item = 00;

    item = new QTreeWidgetItem(trolltech);
    item->setText(0, tr("Dialog"));
    item->setData(0, TemplateNameRole, QString::fromUtf8(":/trolltech/designer/templates/forms/dialog.ui"));

    item = new QTreeWidgetItem(trolltech);
    item->setText(0, tr("Main Window"));
    item->setData(0, TemplateNameRole, QString::fromUtf8(":/trolltech/designer/templates/forms/mainwindow.ui"));

    item = new QTreeWidgetItem(trolltech);
    item->setText(0, tr("Widget"));
    item->setData(0, TemplateNameRole, QString::fromUtf8(":/trolltech/designer/templates/forms/widget.ui"));

    ui.treeWidget->setItemOpen(trolltech, true);
}

NewForm::~NewForm()
{
}

void NewForm::on_createButton_clicked()
{
    if (QTreeWidgetItem *item = ui.treeWidget->currentItem()) {
        close();

        QDesignerFormWindow *formWindow = workbench()->createFormWindow();
        if (AbstractFormWindow *editor = formWindow->editor()) {
            QString formTemplateName = item->data(0, TemplateNameRole).toString();
            qDebug() << "selected template:" << formTemplateName;
            QFile f(formTemplateName);
            if (f.open(QFile::ReadOnly)) {
                editor->setContents(&f);
                f.close();
            } else {
                qWarning() << "template not found:" << formTemplateName;
                editor->setContents(QString());
            }
        }
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

