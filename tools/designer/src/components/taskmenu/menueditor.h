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

#ifndef MENUEDITOR_H
#define MENUEDITOR_H

#include "ui_menueditor.h"

class QMenuBar;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class MenuEditor: public QDialog
{
    Q_OBJECT
public:
    MenuEditor(QDesignerFormWindowInterface *form, QWidget *parent);
    virtual ~MenuEditor();

    void fillContentsFromWidget(QWidget *widget);

private slots:
    void on_newItemButton_clicked();
    void on_newSubItemButton_clicked();
    void on_newSeparatorItemButton_clicked();
    void on_deleteItemButton_clicked();
    void on_moveItemUpButton_clicked();
    void on_moveItemDownButton_clicked();

    void on_treeWidget_currentItemChanged(QTreeWidgetItem *current,
            QTreeWidgetItem *previous);
    void on_treeWidget_itemChanged(QTreeWidgetItem *item);

    void on_itemTextLineEdit_textEdited(const QString &text);
    void on_previewPixmapItemButton_clicked();
    void on_deletePixmapItemButton_clicked();

private:
    void updateEditor();

    Ui::MenuEditor ui;
    QDesignerFormWindowInterface *m_form;
    bool m_updating;
    QWidget *m_widget;
    QMap<QAction *, QTreeWidgetItem *> m_actionToItem;
    QMap<QTreeWidgetItem *, QAction *> m_itemToAction;
};

}  // namespace qdesigner_internal

#endif // MENUEDITOR_H
