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

#include "tablewidgeteditor.h"
#include <findicondialog_p.h>
#include <iconloader_p.h>
#include <qdesigner_command_p.h>

#include <QtDesigner/QtDesigner>
#include <QtCore/QDir>
#include <QtCore/QQueue>

using namespace qdesigner_internal;

TableWidgetEditor::TableWidgetEditor(QDesignerFormWindowInterface *form, QWidget *parent)
    : QDialog(parent), m_updating(false)
{
    ui.setupUi(this);
    m_form = form;
    QIcon resetIcon = createIconSet(QString::fromUtf8("editdelete.png"));
    ui.deletePixmapItemButton->setIcon(resetIcon);
    ui.deletePixmapColumnButton->setIcon(resetIcon);
    ui.deletePixmapRowButton->setIcon(resetIcon);
    ui.deletePixmapItemButton->setEnabled(false);
    ui.deletePixmapColumnButton->setEnabled(false);
    ui.deletePixmapRowButton->setEnabled(false);

    QIcon upIcon = createIconSet(QString::fromUtf8("up.png"));
    QIcon downIcon = createIconSet(QString::fromUtf8("down.png"));
    QIcon minusIcon = createIconSet(QString::fromUtf8("minus.png"));
    QIcon plusIcon = createIconSet(QString::fromUtf8("plus.png"));
    ui.newColumnButton->setIcon(plusIcon);
    ui.deleteColumnButton->setIcon(minusIcon);
    ui.moveColumnUpButton->setIcon(upIcon);
    ui.moveColumnDownButton->setIcon(downIcon);
    ui.newRowButton->setIcon(plusIcon);
    ui.deleteRowButton->setIcon(minusIcon);
    ui.moveRowUpButton->setIcon(upIcon);
    ui.moveRowDownButton->setIcon(downIcon);

    ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
}

TableWidgetEditor::~TableWidgetEditor()
{
}

void TableWidgetEditor::fillContentsFromTableWidget(QTableWidget *tableWidget)
{
    m_updating = true;
    copyContents(tableWidget, ui.tableWidget);

    ui.columnsListWidget->clear();
    ui.rowsListWidget->clear();

    int colCount = ui.tableWidget->columnCount();
    for (int col = 0; col < colCount; col++) {
        QTableWidgetItem *headerItem = ui.tableWidget->horizontalHeaderItem(col);
        QListWidgetItem *item = new QListWidgetItem(ui.columnsListWidget);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        if (headerItem) {
            item->setText(headerItem->text());
            item->setIcon(headerItem->icon());
        } else
            item->setText(QString("%1").arg(col));
    }
    if (colCount > 0)
        ui.columnsListWidget->setCurrentRow(0);

    int rowCount = ui.tableWidget->rowCount();
    for (int row = 0; row < rowCount; row++) {
        QTableWidgetItem *headerItem = ui.tableWidget->verticalHeaderItem(row);
        QListWidgetItem *item = new QListWidgetItem(ui.rowsListWidget);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        if (headerItem) {
            item->setText(headerItem->text());
            item->setIcon(headerItem->icon());
        } else
            item->setText(QString("%1").arg(row));
    }
    if (rowCount > 0)
        ui.rowsListWidget->setCurrentRow(0);

    if (ui.tableWidget->columnCount() > 0 && ui.tableWidget->rowCount() > 0)
        ui.tableWidget->setCurrentCell(0, 0);
    m_updating = false;
    updateEditor();
}

void TableWidgetEditor::fillTableWidgetFromContents(QTableWidget *tableWidget)
{
    ChangeTableContentsCommand *cmd = new ChangeTableContentsCommand(m_form);
    cmd->init(tableWidget, ui.tableWidget);
    m_form->commandHistory()->push(cmd);
}

void TableWidgetEditor::copyContents(QTableWidget *sourceWidget, QTableWidget *destWidget)
{
    destWidget->clear();

    int colCount = sourceWidget->columnCount();
    destWidget->setColumnCount(colCount);
    for (int col = 0; col < colCount; col++) {
        QTableWidgetItem *origHeaderItem = sourceWidget->horizontalHeaderItem(col);
        QTableWidgetItem *headerItem = destWidget->horizontalHeaderItem(col);
        if (origHeaderItem) {
            if (!headerItem)
                headerItem = new QTableWidgetItem;
            headerItem->setText(origHeaderItem->text());
            headerItem->setIcon(origHeaderItem->icon());
            destWidget->setHorizontalHeaderItem(col, headerItem);
        } else {
            if (headerItem)
                delete headerItem;
        }
    }

    int rowCount = sourceWidget->rowCount();
    destWidget->setRowCount(rowCount);
    for (int row = 0; row < rowCount; row++) {
        QTableWidgetItem *origHeaderItem = sourceWidget->verticalHeaderItem(row);
        QTableWidgetItem *headerItem = destWidget->verticalHeaderItem(row);
        if (origHeaderItem) {
            if (!headerItem)
                headerItem = new QTableWidgetItem;
            headerItem->setText(origHeaderItem->text());
            headerItem->setIcon(origHeaderItem->icon());
            destWidget->setVerticalHeaderItem(row, headerItem);
        } else {
            if (headerItem)
                delete headerItem;
        }
    }

    for (int col = 0; col < colCount; col++) {
        for (int row = 0; row < rowCount; row++) {
            QTableWidgetItem *origItem = sourceWidget->item(row, col);
            QTableWidgetItem *newItem = destWidget->item(row, col);
            if (origItem) {
                QString text = origItem->text();
                QIcon icon = origItem->icon();
                if (text.isEmpty() && icon.isNull()) {
                    if (newItem)
                        delete newItem;
                } else {
                    if (!newItem)
                        newItem = new QTableWidgetItem;
                    newItem->setText(text);
                    newItem->setIcon(icon);
                    destWidget->setItem(row, col, newItem);
                }
            } else {
                if (newItem)
                    delete newItem;
            }
        }
    }
}

void TableWidgetEditor::on_tableWidget_currentCellChanged(int currentRow, int currentCol, int, int)
{
    if (m_updating)
        return;
    m_updating = true;
    ui.rowsListWidget->setCurrentRow(currentRow);
    ui.columnsListWidget->setCurrentRow(currentCol);
    m_updating = false;
    updateEditor();
    ui.itemTextLineEdit->selectAll();
    ui.itemTextLineEdit->setFocus();
}

void TableWidgetEditor::on_tableWidget_itemChanged(QTableWidgetItem *)
{
    if (m_updating)
        return;
    updateEditor();
}

void TableWidgetEditor::on_columnsListWidget_currentRowChanged(int col)
{
    if (m_updating)
        return;
    m_updating = true;
    QListWidgetItem *currentRow = ui.rowsListWidget->currentItem();
    if (currentRow) {
        int row = ui.rowsListWidget->currentRow();
        ui.tableWidget->setCurrentCell(row, col);
    }
    m_updating = false;
    updateEditor();
}

void TableWidgetEditor::on_columnsListWidget_itemChanged(QListWidgetItem *item)
{
    if (m_updating)
        return;
    QString str = item->text();
    int col = ui.columnsListWidget->row(item);
    QTableWidgetItem *headerItem = ui.tableWidget->horizontalHeaderItem(col);
    if (!headerItem)
        headerItem = new QTableWidgetItem;
    headerItem->setText(str);
    ui.tableWidget->setHorizontalHeaderItem(col, headerItem);
}

void TableWidgetEditor::on_rowsListWidget_currentRowChanged(int row)
{
    if (m_updating)
        return;
    m_updating = true;
    QListWidgetItem *currentColumn = ui.columnsListWidget->currentItem();
    if (currentColumn) {
        int col = ui.columnsListWidget->currentRow();
        ui.tableWidget->setCurrentCell(row, col);
    }
    m_updating = false;
    updateEditor();
}

void TableWidgetEditor::on_rowsListWidget_itemChanged(QListWidgetItem *item)
{
    if (m_updating)
        return;
    QString str = item->text();
    int row = ui.rowsListWidget->row(item);
    QTableWidgetItem *headerItem = ui.tableWidget->verticalHeaderItem(row);
    if (!headerItem)
        headerItem = new QTableWidgetItem;
    headerItem->setText(str);
    ui.tableWidget->setVerticalHeaderItem(row, headerItem);
}

void TableWidgetEditor::updateEditor()
{
    QListWidgetItem *currentColumn = ui.columnsListWidget->currentItem();
    QListWidgetItem *currentRow = ui.rowsListWidget->currentItem();

    bool itemsEnabled = false;
    bool currentItemEnabled = false;

    bool currentColumnEnabled = false;
    bool moveColumnUpEnabled = false;
    bool moveColumnDownEnabled = false;

    bool currentRowEnabled = false;
    bool moveRowUpEnabled = false;
    bool moveRowDownEnabled = false;

    if (currentColumn) {
        currentColumnEnabled = true;
        int idx = ui.columnsListWidget->currentRow();
        if (idx > 0)
            moveColumnUpEnabled = true;
        if (idx < ui.columnsListWidget->count() - 1)
            moveColumnDownEnabled = true;
    }

    if (currentRow) {
        currentRowEnabled = true;
        int idx = ui.rowsListWidget->currentRow();
        if (idx > 0)
            moveRowUpEnabled = true;
        if (idx < ui.rowsListWidget->count() - 1)
            moveRowDownEnabled = true;
    }

    if (currentColumn && currentRow)
        currentItemEnabled = true;
    if (currentColumn || currentRow)
        itemsEnabled = true;

    ui.itemsBox->setEnabled(itemsEnabled);
    ui.textLabel->setEnabled(currentItemEnabled);
    ui.pixmapLabel->setEnabled(currentItemEnabled);
    ui.deletePixmapItemButton->setEnabled(currentItemEnabled);
    ui.previewPixmapItemButton->setEnabled(currentItemEnabled);
    ui.itemTextLineEdit->setEnabled(currentItemEnabled);

    ui.renameColumnButton->setEnabled(currentColumnEnabled);
    ui.deleteColumnButton->setEnabled(currentColumnEnabled);
    ui.pixmapColumnLabel->setEnabled(currentColumnEnabled);
    ui.previewPixmapColumnButton->setEnabled(currentColumnEnabled);
    ui.moveColumnUpButton->setEnabled(moveColumnUpEnabled);
    ui.moveColumnDownButton->setEnabled(moveColumnDownEnabled);

    ui.renameRowButton->setEnabled(currentRowEnabled);
    ui.deleteRowButton->setEnabled(currentRowEnabled);
    ui.pixmapRowLabel->setEnabled(currentRowEnabled);
    ui.previewPixmapRowButton->setEnabled(currentRowEnabled);
    ui.moveRowUpButton->setEnabled(moveRowUpEnabled);
    ui.moveRowDownButton->setEnabled(moveRowDownEnabled);

    QString itemText;
    QIcon itemIcon, columnIcon, rowIcon;

    if (currentColumn) {
        int col = ui.columnsListWidget->currentRow();
        if (ui.tableWidget->horizontalHeaderItem(col))
            columnIcon = ui.tableWidget->horizontalHeaderItem(col)->icon();
    }

    if (currentRow) {
        int row = ui.rowsListWidget->currentRow();
        if (ui.tableWidget->verticalHeaderItem(row))
            rowIcon = ui.tableWidget->verticalHeaderItem(row)->icon();
    }

    if (currentColumn && currentRow) {
        QTableWidgetItem *current = ui.tableWidget->item(ui.rowsListWidget->currentRow(),
                    ui.columnsListWidget->currentRow());
        if (current) {
            itemText = current->text();
            itemIcon = current->icon();
        }
    }

    ui.itemTextLineEdit->setText(itemText);
    ui.previewPixmapItemButton->setIcon(itemIcon);
    ui.deletePixmapItemButton->setEnabled(!itemIcon.isNull());
    ui.previewPixmapColumnButton->setIcon(columnIcon);
    ui.deletePixmapColumnButton->setEnabled(!columnIcon.isNull());
    ui.previewPixmapRowButton->setIcon(rowIcon);
    ui.deletePixmapRowButton->setEnabled(!rowIcon.isNull());
}

void TableWidgetEditor::on_itemTextLineEdit_textEdited(const QString &text)
{
    if (m_updating)
        return;
    QListWidgetItem *currentColumn = ui.columnsListWidget->currentItem();
    QListWidgetItem *currentRow = ui.rowsListWidget->currentItem();
    if (!currentColumn || !currentRow)
        return;

    int row = ui.rowsListWidget->currentRow();
    int col = ui.columnsListWidget->currentRow();
    QTableWidgetItem *curItem = ui.tableWidget->item(row, col);
    if (!curItem)
        curItem = new QTableWidgetItem;
    curItem->setText(text);

    ui.tableWidget->setItem(row, col, curItem);
}

void TableWidgetEditor::on_deletePixmapItemButton_clicked()
{
    QListWidgetItem *currentColumn = ui.columnsListWidget->currentItem();
    QListWidgetItem *currentRow = ui.rowsListWidget->currentItem();
    if (!currentColumn || !currentRow)
        return;

    int row = ui.rowsListWidget->currentRow();
    int col = ui.columnsListWidget->currentRow();
    QTableWidgetItem *curItem = ui.tableWidget->item(row, col);
    if (!curItem)
        curItem = new QTableWidgetItem;

    curItem->setIcon(QIcon());
    ui.tableWidget->setItem(row, col, curItem);
    ui.previewPixmapItemButton->setIcon(QIcon());
    ui.deletePixmapItemButton->setEnabled(false);
}

void TableWidgetEditor::on_previewPixmapItemButton_clicked()
{
    QListWidgetItem *currentColumn = ui.columnsListWidget->currentItem();
    QListWidgetItem *currentRow = ui.rowsListWidget->currentItem();
    if (!currentColumn || !currentRow)
        return;

    int row = ui.rowsListWidget->currentRow();
    int col = ui.columnsListWidget->currentRow();
    QTableWidgetItem *curItem = ui.tableWidget->item(row, col);

    FindIconDialog dialog(m_form, this);
    QString file_path;
    QString qrc_path;

    QIcon icon;
    if (curItem)
        icon = curItem->icon();
    if (icon.isNull()) {
        file_path = m_form->absoluteDir().absolutePath();
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
            if (!curItem)
                curItem = new QTableWidgetItem;
            curItem->setIcon(icon);
            ui.tableWidget->setItem(row, col, curItem);
            ui.previewPixmapItemButton->setIcon(icon);
            ui.deletePixmapItemButton->setEnabled(!icon.isNull());
        }
    }
}

void TableWidgetEditor::moveColumnsLeft(int fromColumn, int toColumn)
{
    if (fromColumn >= toColumn)
        return;

    QTableWidgetItem *lastItem = ui.tableWidget->takeHorizontalHeaderItem(toColumn);
    for (int i = toColumn; i > fromColumn; i--) {
        ui.tableWidget->setHorizontalHeaderItem(i,
                    ui.tableWidget->takeHorizontalHeaderItem(i - 1));
    }
    ui.tableWidget->setHorizontalHeaderItem(fromColumn, lastItem);

    for (int i = 0; i < ui.tableWidget->rowCount(); i++) {
        QTableWidgetItem *lastItem = ui.tableWidget->takeItem(i, toColumn);
        for (int j = toColumn; j > fromColumn; j--)
            ui.tableWidget->setItem(i, j, ui.tableWidget->takeItem(i, j - 1));
        ui.tableWidget->setItem(i, fromColumn, lastItem);
    }
}

void TableWidgetEditor::moveColumnsRight(int fromColumn, int toColumn)
{
    if (fromColumn >= toColumn)
        return;

    QTableWidgetItem *lastItem = ui.tableWidget->takeHorizontalHeaderItem(fromColumn);
    for (int i = fromColumn; i < toColumn; i++) {
        ui.tableWidget->setHorizontalHeaderItem(i,
                    ui.tableWidget->takeHorizontalHeaderItem(i + 1));
    }
    ui.tableWidget->setHorizontalHeaderItem(toColumn, lastItem);

    for (int i = 0; i < ui.tableWidget->rowCount(); i++) {
        QTableWidgetItem *lastItem = ui.tableWidget->takeItem(i, fromColumn);
        for (int j = fromColumn; j < toColumn; j++)
            ui.tableWidget->setItem(i, j, ui.tableWidget->takeItem(i, j + 1));
        ui.tableWidget->setItem(i, toColumn, lastItem);
    }
}

void TableWidgetEditor::moveRowsDown(int fromRow, int toRow)
{
    if (fromRow >= toRow)
        return;

    QTableWidgetItem *lastItem = ui.tableWidget->takeVerticalHeaderItem(toRow);
    for (int i = toRow; i > fromRow; i--) {
        ui.tableWidget->setVerticalHeaderItem(i,
                    ui.tableWidget->takeVerticalHeaderItem(i - 1));
    }
    ui.tableWidget->setVerticalHeaderItem(fromRow, lastItem);

    for (int i = 0; i < ui.tableWidget->columnCount(); i++) {
        QTableWidgetItem *lastItem = ui.tableWidget->takeItem(toRow, i);
        for (int j = toRow; j > fromRow; j--)
            ui.tableWidget->setItem(j, i, ui.tableWidget->takeItem(j - 1, i));
        ui.tableWidget->setItem(fromRow, i, lastItem);
    }
}

void TableWidgetEditor::moveRowsUp(int fromRow, int toRow)
{
    if (fromRow >= toRow)
        return;

    QTableWidgetItem *lastItem = ui.tableWidget->takeVerticalHeaderItem(fromRow);
    for (int i = fromRow; i < toRow; i++) {
        ui.tableWidget->setVerticalHeaderItem(i,
                    ui.tableWidget->takeVerticalHeaderItem(i + 1));
    }
    ui.tableWidget->setVerticalHeaderItem(toRow, lastItem);

    for (int i = 0; i < ui.tableWidget->columnCount(); i++) {
        QTableWidgetItem *lastItem = ui.tableWidget->takeItem(fromRow, i);
        for (int j = fromRow; j < toRow; j++)
            ui.tableWidget->setItem(j, i, ui.tableWidget->takeItem(j + 1, i));
        ui.tableWidget->setItem(toRow, i, lastItem);
    }
}

void TableWidgetEditor::on_newColumnButton_clicked()
{
    m_updating = true;
    QListWidgetItem *currentColumn = ui.columnsListWidget->currentItem();
    int idx = ui.columnsListWidget->count();
    if (currentColumn)
        idx = ui.columnsListWidget->currentRow() + 1;

    QString newColumnString = tr("New Column");

    int columnCount = ui.tableWidget->columnCount();
    ui.tableWidget->setColumnCount(columnCount + 1);

    QTableWidgetItem *headerItem = ui.tableWidget->horizontalHeaderItem(columnCount);
    if (!headerItem)
        headerItem = new QTableWidgetItem;
    headerItem->setText(newColumnString);
    ui.tableWidget->setHorizontalHeaderItem(columnCount, headerItem);
    moveColumnsLeft(idx, columnCount);

    QListWidgetItem *currentRow = ui.rowsListWidget->currentItem();
    if (currentRow) {
        int row = ui.rowsListWidget->currentRow();
        ui.tableWidget->setCurrentCell(row, idx);
    }

    QListWidgetItem *item = new QListWidgetItem();
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    item->setText(newColumnString);
    ui.columnsListWidget->insertItem(idx, item);
    ui.columnsListWidget->setCurrentItem(item);

    m_updating = false;
    updateEditor();

    ui.columnsListWidget->editItem(item);
}

void TableWidgetEditor::on_renameColumnButton_clicked()
{
    QListWidgetItem *currentColumn = ui.columnsListWidget->currentItem();
    if (!currentColumn)
        return;

    ui.columnsListWidget->editItem(currentColumn);
}

void TableWidgetEditor::on_deleteColumnButton_clicked()
{
    QListWidgetItem *currentColumn = ui.columnsListWidget->currentItem();
    if (!currentColumn)
        return;

    m_updating = true;

    int idx = ui.columnsListWidget->currentRow();
    int columnCount = ui.tableWidget->columnCount();

    moveColumnsRight(idx, columnCount - 1);
    ui.tableWidget->setColumnCount(columnCount - 1);

    delete currentColumn;
    if (idx == columnCount - 1)
        idx--;
    if (idx >= 0)
        ui.columnsListWidget->setCurrentRow(idx);

    m_updating = false;
    updateEditor();
}

void TableWidgetEditor::on_moveColumnUpButton_clicked()
{
    QListWidgetItem *currentColumn = ui.columnsListWidget->currentItem();
    if (!currentColumn)
        return;

    int idx = ui.columnsListWidget->currentRow();

    if (idx == 0)
        return;

    m_updating = true;

    moveColumnsRight(idx - 1, idx);
    ui.columnsListWidget->takeItem(idx);
    ui.columnsListWidget->insertItem(idx - 1, currentColumn);
    ui.columnsListWidget->setCurrentItem(currentColumn);

    m_updating = false;
    updateEditor();
}

void TableWidgetEditor::on_moveColumnDownButton_clicked()
{
    QListWidgetItem *currentColumn = ui.columnsListWidget->currentItem();
    if (!currentColumn)
        return;

    int idx = ui.columnsListWidget->currentRow();
    int columnCount = ui.tableWidget->columnCount();

    if (idx == columnCount - 1)
        return;

    m_updating = true;

    moveColumnsLeft(idx, idx + 1);
    ui.columnsListWidget->takeItem(idx);
    ui.columnsListWidget->insertItem(idx + 1, currentColumn);
    ui.columnsListWidget->setCurrentItem(currentColumn);

    m_updating = false;
    updateEditor();
}

void TableWidgetEditor::on_previewPixmapColumnButton_clicked()
{
    QListWidgetItem *currentColumn = ui.columnsListWidget->currentItem();
    if (!currentColumn)
        return;

    int currentRow = ui.columnsListWidget->currentRow();

    FindIconDialog dialog(m_form, this);
    QString file_path;
    QString qrc_path;

    QIcon icon = currentColumn->icon();
    if (icon.isNull()) {
        file_path = m_form->absoluteDir().absolutePath();
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
            currentColumn->setIcon(icon);
            QTableWidgetItem *headerItem = ui.tableWidget->horizontalHeaderItem(currentRow);
            if (!headerItem) {
                headerItem = new QTableWidgetItem;
            }
            headerItem->setIcon(icon);
            ui.tableWidget->setHorizontalHeaderItem(currentRow, headerItem);
            ui.previewPixmapColumnButton->setIcon(icon);
            ui.deletePixmapColumnButton->setEnabled(!icon.isNull());
        }
    }

}

void TableWidgetEditor::on_deletePixmapColumnButton_clicked()
{
    QListWidgetItem *curItem = ui.columnsListWidget->currentItem();
    if (!curItem)
        return;

    curItem->setIcon(QIcon());
    int col = ui.columnsListWidget->currentRow();
    QTableWidgetItem *headerItem = ui.tableWidget->horizontalHeaderItem(col);
    if (headerItem)
        headerItem->setIcon(QIcon());
    ui.previewPixmapColumnButton->setIcon(QIcon());
    ui.deletePixmapColumnButton->setEnabled(false);
}

void TableWidgetEditor::on_newRowButton_clicked()
{
    m_updating = true;

    QListWidgetItem *currentRow = ui.rowsListWidget->currentItem();
    int idx = ui.rowsListWidget->count();
    if (currentRow)
        idx = ui.rowsListWidget->currentRow() + 1;

    QString newRowString = tr("New Row");

    int rowCount = ui.tableWidget->rowCount();
    ui.tableWidget->setRowCount(rowCount + 1);

    QTableWidgetItem *headerItem = ui.tableWidget->verticalHeaderItem(rowCount);
    if (!headerItem)
        headerItem = new QTableWidgetItem;
    headerItem->setText(newRowString);
    ui.tableWidget->setVerticalHeaderItem(rowCount, headerItem);
    moveRowsDown(idx, rowCount);

    QListWidgetItem *currentCol = ui.columnsListWidget->currentItem();
    if (currentCol) {
        int col = ui.columnsListWidget->currentRow();
        ui.tableWidget->setCurrentCell(idx, col);
    }

    QListWidgetItem *item = new QListWidgetItem();
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    item->setText(newRowString);
    ui.rowsListWidget->insertItem(idx, item);
    ui.rowsListWidget->setCurrentItem(item);

    m_updating = false;
    updateEditor();

    ui.rowsListWidget->editItem(item);
}

void TableWidgetEditor::on_renameRowButton_clicked()
{
    QListWidgetItem *currentRow = ui.rowsListWidget->currentItem();
    if (!currentRow)
        return;

    ui.rowsListWidget->editItem(currentRow);
}

void TableWidgetEditor::on_deleteRowButton_clicked()
{
    m_updating = true;

    QListWidgetItem *currentRow = ui.rowsListWidget->currentItem();
    if (!currentRow)
        return;

    int idx = ui.rowsListWidget->currentRow();
    int rowCount = ui.tableWidget->rowCount();

    moveRowsUp(idx, rowCount - 1);
    ui.tableWidget->setRowCount(rowCount - 1);

    delete currentRow;
    if (idx == rowCount - 1)
        idx--;
    if (idx >= 0)
        ui.rowsListWidget->setCurrentRow(idx);

    m_updating = false;
    updateEditor();
}

void TableWidgetEditor::on_moveRowUpButton_clicked()
{
    QListWidgetItem *currentRow = ui.rowsListWidget->currentItem();
    if (!currentRow)
        return;

    int idx = ui.rowsListWidget->currentRow();

    if (idx == 0)
        return;

    m_updating = true;

    moveRowsUp(idx - 1, idx);
    ui.rowsListWidget->takeItem(idx);
    ui.rowsListWidget->insertItem(idx - 1, currentRow);
    ui.rowsListWidget->setCurrentItem(currentRow);

    m_updating = false;
    updateEditor();
}

void TableWidgetEditor::on_moveRowDownButton_clicked()
{
    QListWidgetItem *currentRow = ui.rowsListWidget->currentItem();
    if (!currentRow)
        return;

    int idx = ui.rowsListWidget->currentRow();
    int rowCount = ui.tableWidget->rowCount();

    if (idx == rowCount - 1)
        return;

    m_updating = true;

    moveRowsDown(idx, idx + 1);
    ui.rowsListWidget->takeItem(idx);
    ui.rowsListWidget->insertItem(idx + 1, currentRow);
    ui.rowsListWidget->setCurrentItem(currentRow);

    m_updating = false;
    updateEditor();
}

void TableWidgetEditor::on_previewPixmapRowButton_clicked()
{
    QListWidgetItem *currentRow = ui.rowsListWidget->currentItem();
    if (!currentRow)
        return;

    int current = ui.rowsListWidget->currentRow();

    FindIconDialog dialog(m_form, this);
    QString file_path;
    QString qrc_path;

    QIcon icon = currentRow->icon();
    if (icon.isNull()) {
        file_path = m_form->absoluteDir().absolutePath();
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
            currentRow->setIcon(icon);
            QTableWidgetItem *headerItem = ui.tableWidget->verticalHeaderItem(current);
            if (!headerItem)
                headerItem = new QTableWidgetItem;
            headerItem->setIcon(icon);
            ui.tableWidget->setVerticalHeaderItem(current, headerItem);
            ui.previewPixmapRowButton->setIcon(icon);
            ui.deletePixmapRowButton->setEnabled(!icon.isNull());
        }
    }

}

void TableWidgetEditor::on_deletePixmapRowButton_clicked()
{
    QListWidgetItem *curItem = ui.rowsListWidget->currentItem();
    if (!curItem)
        return;

    curItem->setIcon(QIcon());
    int row = ui.rowsListWidget->currentRow();
    QTableWidgetItem *headerItem = ui.tableWidget->verticalHeaderItem(row);
    if (headerItem)
        headerItem->setIcon(QIcon());
    ui.previewPixmapRowButton->setIcon(QIcon());
    ui.deletePixmapRowButton->setEnabled(false);
}



