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

#include "listwidgeteditor.h"
#include <QtCore/qdebug.h>

ListWidgetEditor::ListWidgetEditor(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
}

ListWidgetEditor::~ListWidgetEditor()
{
}

void ListWidgetEditor::fillContentsFromListWidget(QListWidget *listWidget)
{
    for (int i=0; i<listWidget->count(); ++i) {
        QListWidgetItem *item = listWidget->item(i);
        ui.listWidget->addItem(item->clone());
    }

    ui.listWidget->setCurrentRow(ui.listWidget->count() - 1);
}

void ListWidgetEditor::fillContentsFromComboBox(QComboBox *comboBox)
{
}

void ListWidgetEditor::on_newItemButton_clicked()
{
    int row = ui.listWidget->currentRow() + 1;

    if (row < ui.listWidget->count()) {
        ui.listWidget->insertItem(row, tr("New Item"));
    } else {
        ui.listWidget->addItem(tr("New Item"));
        ui.listWidget->setCurrentRow(ui.listWidget->count() - 1);
        row = ui.listWidget->count() - 1;
    }

    ui.listWidget->setCurrentRow(row);
}

void ListWidgetEditor::on_deleteItemButton_clicked()
{
    int row = ui.listWidget->currentRow();

    if (row != -1)
        delete ui.listWidget->takeItem(row);

    if (row < ui.listWidget->count())
        ui.listWidget->setCurrentRow(row);
}

void ListWidgetEditor::on_listWidget_currentRowChanged(int currentRow)
{
    QString text;
    QIcon icon;

    if (currentRow != -1) {
        QListWidgetItem *item = ui.listWidget->item(currentRow);
        text = item->text();
        icon = item->icon();
    }

    ui.itemTextLineEdit->setText(text);
    ui.previewButton->setIcon(icon);

    ui.itemTextLineEdit->selectAll();
    ui.itemTextLineEdit->setFocus();
}

void ListWidgetEditor::on_itemTextLineEdit_textChanged(const QString &text)
{
    int currentRow = ui.listWidget->currentRow();
    if (currentRow != -1) {
        QListWidgetItem *item = ui.listWidget->item(currentRow);
        item->setText(text);
    }
}

int ListWidgetEditor::count() const
{
    return ui.listWidget->count();
}

QIcon ListWidgetEditor::icon(int row) const
{
    return ui.listWidget->item(row)->icon();
}

QString ListWidgetEditor::text(int row) const
{
    return ui.listWidget->item(row)->text();
}
