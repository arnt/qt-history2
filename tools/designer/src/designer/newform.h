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

#ifndef NEWFORM_H
#define NEWFORM_H

#include "ui_newform.h"

#include <QDialog>

class QDesignerWorkbench;

class NewForm: public QDialog
{
    Q_OBJECT
public:
    NewForm(QDesignerWorkbench *workbench, QWidget *parentWidget);
    virtual ~NewForm();

    QDesignerWorkbench *workbench() const;

private slots:
    void on_createButton_clicked();
    void on_closeButton_clicked();
    void on_openButton_clicked();
    void on_treeWidget_itemActivated(QTreeWidgetItem *item);
    void on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *);

private:
    QIcon formPreviewIcon(const QString &fileName);
    void loadFrom(const QString &path);

private:
    QDesignerWorkbench *m_workbench;
    Ui::NewForm ui;
};

#endif // NEWFORM_H
