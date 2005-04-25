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

#ifndef LISTWIDGETEDITOR_H
#define LISTWIDGETEDITOR_H

#include "ui_listwidgeteditor.h"

class QListWidget;
class QComboBox;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class ListWidgetEditor: public QDialog
{
    Q_OBJECT
public:
    ListWidgetEditor(QDesignerFormWindowInterface *form, QWidget *parent);
    virtual ~ListWidgetEditor();

    void fillContentsFromListWidget(QListWidget *listWidget);
    void fillContentsFromComboBox(QComboBox *comboBox);

    int count() const;
    QIcon icon(int row) const;
    QString text(int row) const;

private slots:
    void on_newItemButton_clicked();
    void on_deleteItemButton_clicked();
    void on_moveItemUpButton_clicked();
    void on_moveItemDownButton_clicked();
    void on_listWidget_currentRowChanged(int currentRow);
    void on_itemTextLineEdit_textChanged(const QString &text);
    void on_previewButton_clicked();
    void on_deleteButton_clicked();

private:
    Ui::ListWidgetEditor ui;
    QDesignerFormWindowInterface *m_form;
};

}  // namespace qdesigner_internal

#endif // LISTWIDGETEDITOR_H
