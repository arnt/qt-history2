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

#ifndef NEWFORMDIALOG_H
#define NEWFORMDIALOG_H

#include <QDialog>

class NewFormTree;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;

class NewFormDialog : public QDialog
{
    Q_OBJECT
public:
    NewFormDialog(QWidget *parent);
    ~NewFormDialog();

signals:
    void needOpen();
    void itemPicked(const QString &widgetClass);

private slots:
    void handleClass(const QString &strClass);
    void createThisOne();
    void handleDoubleClick(QTreeWidgetItem *item);
    void fixButton(QTreeWidgetItem *item);

private:
    QTreeWidget *mWidgetTree;
    QPushButton *btnCreate;
};

#endif // NEWFORMDIALOG_H
