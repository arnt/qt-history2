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
#include <findicondialog.h>
#include <iconloader.h>
#include <QtDesigner/abstracticoncache.h>
#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractformwindow.h>
#include <QtCore/QDir>
#include <QtCore/qdebug.h>

ListWidgetEditor::ListWidgetEditor(QDesignerFormWindowInterface *form, QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    m_form = form;
    ui.deleteButton->setIcon(createIconSet("editdelete.png"));
    ui.deleteButton->setEnabled(false);
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
    for (int i=0; i<comboBox->count(); ++i) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setText(comboBox->itemText(i));
        item->setIcon(comboBox->itemIcon(i));
        ui.listWidget->addItem(item);
    }

    ui.listWidget->setCurrentRow(ui.listWidget->count() - 1);
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

void ListWidgetEditor::on_moveItemUpButton_clicked()
{
    int row = ui.listWidget->currentRow();
    if (row <= 0)
        return; // nothing to do

    ui.listWidget->insertItem(row - 1, ui.listWidget->takeItem(row));
    ui.listWidget->setCurrentRow(row - 1);
}

void ListWidgetEditor::on_moveItemDownButton_clicked()
{
    int row = ui.listWidget->currentRow();
    if (row == -1 || row == ui.listWidget->count() - 1)
        return; // nothing to do

    ui.listWidget->insertItem(row + 1, ui.listWidget->takeItem(row));
    ui.listWidget->setCurrentRow(row + 1);
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
    ui.deleteButton->setEnabled(!icon.isNull());
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

void ListWidgetEditor::on_deleteButton_clicked()
{
    int currentRow = ui.listWidget->currentRow();
    if (currentRow == -1)
        return;
    QListWidgetItem *item = ui.listWidget->item(currentRow);

    item->setIcon(QIcon());
    ui.previewButton->setIcon(QIcon());
    ui.deleteButton->setEnabled(false);
}

void ListWidgetEditor::on_previewButton_clicked()
{
    int currentRow = ui.listWidget->currentRow();
    if (currentRow == -1)
        return;
    QListWidgetItem *item = ui.listWidget->item(currentRow);

    FindIconDialog dialog(m_form, 0);
    QString file_path;
    QString qrc_path;

    QIcon icon = item->icon();
    if (icon.isNull()) {
        file_path = m_form->absolutePath(QString()) + QDir::separator();
    } else {
        file_path = m_form->core()->iconCache()->iconToFilePath(icon);
        qrc_path = m_form->core()->iconCache()->iconToQrcPath(icon);
    }

    dialog.setPaths(qrc_path, file_path);
    if (dialog.exec()) {
        file_path = dialog.filePath();
        qrc_path = dialog.qrcPath();
        if (!file_path.isEmpty()) {
            icon = m_form->core()->iconCache()->nameToIcon(file_path, qrc_path);
            item->setIcon(icon);
            ui.previewButton->setIcon(icon);
            ui.deleteButton->setEnabled(!icon.isNull());
        }
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
