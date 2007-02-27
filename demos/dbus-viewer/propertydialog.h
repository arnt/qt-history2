/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PROPERTYDIALOG_H
#define PROPERTYDIALOG_H

#include <QLabel>
#include <QDialog>
#include <QTableWidget>
#include <QDialogButtonBox>

class PropertyDialog: public QDialog
{
    Q_OBJECT
public:
    explicit PropertyDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);

    void addProperty(const QString &name, QVariant::Type type);
    void setInfo(const QString &caption);

    QList<QVariant> values() const;

    int exec();

private:
    QLabel *label;
    QTableWidget *propertyTable;
    QDialogButtonBox *buttonBox;
};

#endif

