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

#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include "ui_helpdialog.h"
#include "index.h"
#include "helpwindow.h"
#include "docuparser.h"

#include <qlist.h>
#include <qpair.h>
#include <qlistwidget.h>
#include <qtreewidget.h>
#include <qmap.h>
#include <qstringlist.h>
#include <qvalidator.h>
#include <qmenu.h>
#include <qhash.h>

class QProgressBar;
class MainWindow;
class QTextBrowser;
class IndexListModel;

class HelpNavigationListItem : public QListWidgetItem
{
public:
    HelpNavigationListItem(QListWidget *ls, const QString &txt);

    void addLink(const QString &link);
    QStringList links() const { return linkList; }
private:
    QStringList linkList;

};

class SearchValidator : public QValidator
{
    Q_OBJECT
public:
    SearchValidator(QObject *parent)
        : QValidator(parent) {}
    ~SearchValidator() {}
    QValidator::State validate(QString &str, int &) const;
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
    static QString removeAnchorFromLink(const QString &link);

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
    void on_resultBox_itemActivated(QListWidgetItem*);

    void showResultPage(QListWidgetItem *);

    void showTopic(QTreeWidgetItem *);
    void loadIndexFile();
    void insertContents();
    void setupFullTextIndex();
    void currentTabChanged(int index);
    void showTopic();
    void searchInIndex(const QString &s);
    void toggleContents();
    void toggleIndex();
    void toggleBookmarks();
    void toggleSearch();
    void lastWinClosed();
    void setIndexingProgress(int prog);
    void showListItemMenu(const QPoint &pos);
    void showTreeItemMenu(const QPoint &pos);
    void insertBookmarks();
    void processEvents();

private:
    typedef QList<ContentItem> ContentList;
    void removeOldCacheFiles();
    void buildKeywordDB();
    quint32 getFileAges();
    void showIndexTopic();
    void showBookmarkTopic();
    void setupTitleMap();
    void saveBookmarks();
    void showContentsTopic();
    void showInitDoneMessage();
    void buildContentDict();

private:
    Ui::HelpDialog ui;

    IndexListModel *indexModel;
    QMap<QString, QString> titleMap;
    bool indexDone, bookmarksInserted, titleMapDone, contentsInserted;
    bool lwClosed;
    MainWindow *help;
    QString documentationPath;
    Index *fullTextIndex;
    QStringList terms, foundDocs;
    bool initDoneMsgShown;
    void getAllContents();
    QList<QPair<QString, ContentList> > contentList;
    QMenu *itemPopup;
    QString cacheFilesPath;

    QAction *actionOpenCurrentTab;
    QAction *actionOpenLinkInNewWindow;
    QAction *actionOpenLinkInNewTab;
};

#endif
