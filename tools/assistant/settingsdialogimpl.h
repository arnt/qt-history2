/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Assistant.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef SETTINGSDIALOGIMPL_H
#define SETTINGSDIALOGIMPL_H

#include <qstringlist.h>
#include <qptrlist.h>
#include <qlistview.h>
#include "settingsdialog.h"

struct listItem {
    listItem( QString ln, QString sn, int de ) 
	: lname( ln ), sname( sn ), d( de ) {}    
    QString lname;
    QString sname;
    int d;
};

struct stateListItem {
    stateListItem( QListViewItem *i, bool state )
	: item( i ), isChecked( state ) {}
    QListViewItem *item;
    bool isChecked;
};


class CheckListItem : public QObject, public QCheckListItem 
{
    Q_OBJECT
        
public:
    CheckListItem( CheckListItem *parent, const QString &text, 
	const QString &fullcat );
    CheckListItem( QListView *parent, const QString &text,
	const QString &fullcat );
    QString getFullCategory();
    int rtti() const;
    CheckListItem* getCurrentItem( QListView *parent );
    CheckListItem* getCheckItem( QListViewItem* );
    void stateChange( bool state );
private:
    QString fullCategory;

};



class SettingsDialog : public SettingsDialogBase
{
    Q_OBJECT

public:
    SettingsDialog( QWidget *parent, const char* name = 0 );

protected slots:
    void selectColor();
    void addDocuFile();
    void removeDocuFile();
    void addCategory();
    void deleteCategory();
    void browseWebApp();
    void accept();
    void reject();
signals:
    void docuFilesChanged();
    void categoryChanged();

private:
    void init();
    void insertCategories();
    void makeCategoryList();
    void checkItem( CheckListItem* );
    QStringList getCheckedItemList();
    bool changed, selectionChanged;
    QStringList docuFileList, catListAvail, catListSel;
    QPtrList<CheckListItem> catItemList;
    CheckListItem *allItem;
};

#endif
