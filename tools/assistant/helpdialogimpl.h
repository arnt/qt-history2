/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
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

#ifndef HELPDIALOGIMPL_H
#define HELPDIALOGIMPL_H

#include "helpdialog.h"
#include <qlistbox.h>
#include <qlistview.h>
#include <qmap.h>
#include <qstringlist.h>

class QProgressBar;
class MainWindow;
class QTextBrowser;

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
    HelpDialog( QWidget *parent, MainWindow *h, QTextBrowser *v );

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

public slots:
    void toggleContents();
    void toggleIndex();
    void toggleBookmarks();

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
    void insertContents( const QString &filename, const QString &title,
			 HelpNavigationContentsItem *lastItem,
			 HelpNavigationContentsItem *handbook );

private:
    QMap<QString, QString> titleMap;
    bool indexDone, bookmarksInserted, contentsDone, contentsInserted;
    MainWindow *help;
    QTextBrowser *viewer;
    QString documentationPath;

};

#endif
