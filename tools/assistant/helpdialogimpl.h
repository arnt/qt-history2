/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
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

#ifndef HELPDIALOGIMPL_H
#define HELPDIALOGIMPL_H

#include "helpdialog.h"
#include "index.h"
#include "helpwindow.h"
#include "docuparser.h"

#include <qlistbox.h>
#include <qlistview.h>
#include <qmap.h>
#include <qstringlist.h>
#include <qvalidator.h>
#include <qpopupmenu.h>
#include <qdict.h>
#include <qmime.h>
#include <qpopupmenu.h>

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

class SearchValidator : public QValidator
{
    Q_OBJECT
public:
    SearchValidator( QObject *parent, const char *name = 0 )
	: QValidator( parent, name ) {}
    ~SearchValidator() {}
    QValidator::State validate( QString &str, int & ) const;
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
    HelpDialog( QWidget *parent, MainWindow *h );

    QString titleOfLink( const QString &link );
    bool eventFilter( QObject *, QEvent * );
    bool lastWindowClosed() { return lwClosed; }
    static void removeDocFile( const QString &absFileName );
    QMimeSourceFactory* getContentFactory() const { return contentFactory; }

protected slots:
    void loadIndexFile();
    void insertContents();
    void setupFullTextIndex();
    void currentTabChanged( const QString &s );
    void currentIndexChanged( QListBoxItem *i );
    void generateNewDocu();
    void showTopic();
    void searchInIndex( const QString &s );
    void addBookmark();
    void removeBookmark();
    void currentBookmarkChanged( QListViewItem *i );
    void currentContentsChanged( QListViewItem *i );
    void startSearch();
    void showSearchHelp();

public slots:
    void initialize();
    void toggleContents();
    void toggleIndex();
    void toggleBookmarks();
    void toggleSearch();

signals:
    void showLink( const QString &s );
    void showSearchLink( const QString &s, const QStringList &terms );

private slots:
    void lastWinClosed();
    void showResultPage( int page );
    void showResultPage( QListBoxItem *i );
    void setIndexingProgress( int prog );
    void showItemMenu( QListBoxItem *item, const QPoint &pos );
    void showItemMenu( QListViewItem *item, const QPoint &pos );

private:
    typedef QValueList<ContentItem> ContentList;
    void buildKeywordDB();
    Q_UINT32 getFileAges();
    void showIndexTopic();
    void showBookmarkTopic();
    void insertBookmarks();
    void setupTitleMap();
    void saveBookmarks();
    void showContentsTopic();
    bool isValidCategory( QString category );
    void showInitDoneMessage();
    void buildContentDict();

    QMap<QString, QString> titleMap;
    QMap<QString, uint> categoryMap;
    bool indexDone, bookmarksInserted, titleMapDone, contentsInserted;
    bool lwClosed;
    MainWindow *help;
    QString documentationPath;
    Index *fullTextIndex;
    QStringList terms, foundDocs;
    bool newFullTextIndex;
    bool initDoneMsgShown;
    void getAllContents();
    QDict<ContentList> contentList;
    QMimeSourceFactory *contentFactory;
    QPopupMenu *itemPopup;
};

#endif
