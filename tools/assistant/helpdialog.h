/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Assistant.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include "ui_helpdialog.h"
#include "index.h"
#include "helpwindow.h"
#include "docuparser.h"

#include <qlistbox.h>
#include <qlistview.h>
#include <qmap.h>
#include <qstringlist.h>
#include <qvalidator.h>
#include <qpopupmenu.h>
#include <qhash.h>

class QProgressBar;
class MainWindow;
class QTextBrowser;

class HelpNavigationListItem : public QListBoxText
{
public:
    HelpNavigationListItem(QListBox *ls, const QString &txt);

    void addLink(const QString &link);
    QStringList links() const { return linkList; }
private:
    QStringList linkList;

};

class SearchValidator : public QValidator
{
    Q_OBJECT
public:
    SearchValidator(QObject *parent, const char *name = 0)
        : QValidator(parent, name) {}
    ~SearchValidator() {}
    QValidator::State validate(QString &str, int &) const;
};

class HelpNavigationContentsItem : public QListViewItem
{
public:
    HelpNavigationContentsItem(QListView *v, QListViewItem *after);
    HelpNavigationContentsItem(QListViewItem *v, QListViewItem *after);

    void setLink(const QString &lnk);
    QString link() const;

private:
    QString theLink;
};

class HelpDialog : public QWidget
{
    Q_OBJECT
public:
    HelpDialog(QWidget *parent, MainWindow *h);

    inline QTabWidget *tabWidget() const
    { return ui.tabWidget; }

    QString titleOfLink(const QString &link);
    bool eventFilter(QObject *, QEvent *);
    bool lastWindowClosed() { return lwClosed; }

    void timerEvent(QTimerEvent *e);

signals:
    void showLink(const QString &s);
    void showSearchLink(const QString &s, const QStringList &terms);

public slots:
    void initialize();
    void startSearch();
    void addBookmark();

private slots:
    void on_buttonAdd_clicked();
    void on_buttonRemove_clicked();
    void on_termsEdit_returnPressed();
    void on_helpButton_clicked();
    void on_searchButton_clicked();
    void on_resultBox_returnPressed(QListBoxItem*);
    void on_resultBox_mouseButtonClicked(int, QListBoxItem*, const QPoint &);

    void showResultPage(QListBoxItem *);

    void showTopic(int, QListBoxItem *, const QPoint &);
    void showTopic(int, QListViewItem *, const QPoint &);
    void showTopic(QListViewItem *);
    void loadIndexFile();
    void insertContents();
    void setupFullTextIndex();
    void currentTabChanged(int index);
    void currentIndexChanged(QListBoxItem *i);
    void showTopic();
    void searchInIndex(const QString &s);
    void currentBookmarkChanged(QListViewItem *i);
    void currentContentsChanged(QListViewItem *i);
    void toggleContents();
    void toggleIndex();
    void toggleBookmarks();
    void toggleSearch();
    void lastWinClosed();
    void setIndexingProgress(int prog);
    void showItemMenu(QListBoxItem *item, const QPoint &pos);
    void showItemMenu(QListViewItem *item, const QPoint &pos);
    void insertBookmarks();
    void processEvents();

private:
    typedef QList<ContentItem> ContentList;
    void removeOldCacheFiles();
    void buildKeywordDB();
    Q_UINT32 getFileAges();
    void showIndexTopic();
    void showBookmarkTopic();
    void setupTitleMap();
    void saveBookmarks();
    void showContentsTopic();
    void showInitDoneMessage();
    void buildContentDict();

private:
    Ui::HelpDialog ui;

    QMap<QString, QString> titleMap;
    bool indexDone, bookmarksInserted, titleMapDone, contentsInserted;
    bool lwClosed;
    MainWindow *help;
    QString documentationPath;
    Index *fullTextIndex;
    QStringList terms, foundDocs;
    bool initDoneMsgShown;
    void getAllContents();
    QHash<QString, ContentList> contentList;
    QPopupMenu *itemPopup;
    QString cacheFilesPath;

    QAction *actionOpenCurrentTab;
    QAction *actionOpenLinkInNewWindow;
    QAction *actionOpenLinkInNewTab;
};

#endif
