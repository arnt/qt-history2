/**********************************************************************
**   Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
**   This file is part of Qt GUI Designer.
**
**   This file may be distributed under the terms of the GNU General
**   Public License version 2 as published by the Free Software
**   Foundation and appearing in the file COPYING included in the
**   packaging of this file. If you did not get the file, send email
**   to info@trolltech.com
**
**   The file is provided AS IS with NO WARRANTY OF ANY KIND,
**   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR
**   A PARTICULAR PURPOSE.
**
**********************************************************************/

#ifndef HELPDIALOGIMPL_H
#define HELPDIALOGIMPL_H

#include "helpdialog.h"
#include <qlistbox.h>
#include <qlistview.h>
#include <qmap.h>

class MainWindow;
class QProgressBar;
class Help;

class HelpNavigationListItem : public QListBoxText
{
public:
    HelpNavigationListItem( QListBox *ls, const QString &txt );

    void addLink( const QString &link );
    QStringList links() const { return linkList; }

private:
    QStringList linkList;

};

class HelpNavigationContentsItem : public QListViewItem
{
public:
    HelpNavigationContentsItem( QListView *v, QListViewItem *after );
    HelpNavigationContentsItem( QListViewItem *v, QListViewItem *after );

    void setLink( const QString &lnk );
    QString link() const;

private:
    QString theLink;

};

class HelpDialog : public HelpDialogBase
{
    Q_OBJECT

public:
    HelpDialog( QWidget *parent, MainWindow *mw, Help *h );
    QString titleOfLink( const QString &link );
    bool eventFilter( QObject *, QEvent * );

protected slots:
    void loadIndexFile();
    void currentTabChanged( const QString &s );
    void currentIndexChanged( QListBoxItem *i );
    void showTopic();
    void searchInIndex( const QString &s );
    void addBookmark();
    void removeBookmark();
    void currentBookmarkChanged( QListViewItem *i );
    void currentContentsChanged( QListViewItem *i );
    
signals:
    void showLink( const QString &s, const QString& t );

private:
    void showIndexTopic();
    void showBookmarkTopic();
    void insertBookmarks();
    void insertContents();
    void setupTitleMap();
    void saveBookmarks();
    void showContentsTopic();

private:
    MainWindow *mainWindow;
    QMap<QString, QString> titleMap;
    bool indexDone, bookmarksInserted, contentsDone, contentsInserted;
    Help *help;

};

#endif
