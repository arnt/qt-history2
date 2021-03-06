/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef NEWFORM_H
#define NEWFORM_H

#include "ui_newform.h"

#include <QtGui/QDialog>

QT_BEGIN_NAMESPACE

class QDesignerWorkbench;

class NewForm: public QDialog
{
    Q_OBJECT
public:
    NewForm(QDesignerWorkbench *workbench, QWidget *parentWidget, const QString &fileName = QString());
    virtual ~NewForm();

    QDesignerWorkbench *workbench() const;

private slots:
    void on_buttonBox_clicked(QAbstractButton *btn);
    void on_treeWidget_itemActivated(QTreeWidgetItem *item);
    void on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *);
    void on_treeWidget_itemPressed(QTreeWidgetItem *item);
    void recentFileChosen();

private:
    QPixmap formPreviewPixmap(const QString &fileName);
    void loadFrom(const QString &path, bool resourceFile, const QString &uiExtension);

private:
    bool openTemplate(const QString &templateFileName);

    QDesignerWorkbench *m_workbench;
    Ui::NewForm ui;
    QPushButton *m_createButton;
    QPushButton *m_recentButton;
    QString m_fileName;
};

QT_END_NAMESPACE

#endif // NEWFORM_H
