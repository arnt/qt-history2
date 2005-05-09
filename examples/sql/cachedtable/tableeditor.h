/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef TABLEEDITOR_H
#define TABLEEDITOR_H

#include <QDialog>

class QPushButton;
class QSqlTableModel;

class TableEditor : public QDialog
{
    Q_OBJECT

public:
    TableEditor(const QString &tableName, QWidget *parent = 0);

private slots:
    void submit();

private:
    QPushButton *submitButton;
    QPushButton *revertButton;
    QPushButton *quitButton;
    QSqlTableModel *model;
};

#endif
