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
#include <qstring.h>

class QTabWidget;
class Q3ListView;
class QGridLayout;
class QLineEdit;
class QPushButton;
class Q3ListViewItem;
class QCheckBox;

class ABCentralWidget : public QWidget
{
    Q_OBJECT

public:
    ABCentralWidget( QWidget *parent, const char *name = 0 );

    void save( const QString &filename );
    void load( const QString &filename );

protected slots:
    void addEntry();
    void changeEntry();
    void itemSelected( Q3ListViewItem* );
    void selectionChanged();
    void toggleFirstName();
    void toggleLastName();
    void toggleAddress();
    void toggleEMail();
    void findEntries();

protected:
    void setupTabWidget();
    void setupListView();

    QGridLayout *mainGrid;
    QTabWidget *tabWidget;
    Q3ListView *listView;
    QPushButton *add, *change, *find;
    QLineEdit *iFirstName, *iLastName, *iAddress, *iEMail,
        *sFirstName, *sLastName, *sAddress, *sEMail;
    QCheckBox *cFirstName, *cLastName, *cAddress, *cEMail;

};

#endif
