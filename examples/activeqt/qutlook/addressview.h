/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_COMMERCIAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef ADDRESSVIEW_H
#define ADDRESSVIEW_H

#include <QWidget>

class AddressBookModel;
QT_DECLARE_CLASS(QLineEdit)
QT_DECLARE_CLASS(QModelIndex)
QT_DECLARE_CLASS(QPushButton)
QT_DECLARE_CLASS(QTreeView)

class AddressView : public QWidget
{
    Q_OBJECT

public:
    AddressView(QWidget *parent = 0);

protected slots:
    void addEntry();
    void changeEntry();
    void itemSelected(const QModelIndex &index);

    void updateOutlook();

protected:
    AddressBookModel *model;

    QTreeView *treeView;
    QPushButton *add, *change;
    QLineEdit *iFirstName, *iLastName, *iAddress, *iEMail;
};

#endif // ADDRESSVIEW_H
