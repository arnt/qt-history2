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

#ifndef AB_CENTRALWIDGET_H
#define AB_CENTRALWIDGET_H

#include <qwidget.h>
#include <qlistview.h>

class QTabWidget;
class QTreeView;
class QGridLayout;
class QLineEdit;
class QPushButton;
class QCheckBox;
class AddressBookModel;

class AddressView : public QWidget
{
    Q_OBJECT

public:
    AddressView(QWidget *parent = 0);

protected slots:
    void addEntry();
    void changeEntry();
    void itemSelected(const QModelIndex &index, int button);
    void selectionChanged();
    void toggleFirstName();
    void toggleLastName();
    void toggleAddress();
    void toggleEMail();
    void findEntries();

    void updateOutlook();

protected:
    void setupTabWidget();
    void setupTreeView();

    AddressBookModel *model;

    QGridLayout *mainGrid;
    QTabWidget *tabWidget;
    QTreeView *treeView;
    QPushButton *add, *change, *find;
    QLineEdit *iFirstName, *iLastName, *iAddress, *iEMail,
        *sFirstName, *sLastName, *sAddress, *sEMail;
    QCheckBox *cFirstName, *cLastName, *cAddress, *cEMail;
};

#endif
