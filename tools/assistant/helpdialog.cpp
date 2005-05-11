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

#include "helpdialog.h"
#include "helpwindow.h"
#include "topicchooser.h"
#include "docuparser.h"
#include "mainwindow.h"
#include "config.h"
#include "tabbedbrowser.h"

#include <QtGui/QtGui>
#include <QtCore/qdebug.h>

#include <stdlib.h>
#include <limits.h>

enum
{
    LinkRole = Qt::UserRole + 1000
};

static QString stripAmpersand(const QString &str)
{
    QString s(str);
    s = s.replace(QLatin1String("&"), QLatin1String(""));
    return s;
}

static bool verifyDirectory(const QString &str)
{
    QFileInfo dirInfo(str);
    if (!dirInfo.exists())
        return QDir().mkdir(str);
    if (!dirInfo.isDir()) {
        qWarning("'%s' exists but is not a directory", str.toLatin1().constData());
        return false;
    }
    return true;
}

struct IndexKeyword {
    IndexKeyword(const QString &kw, const QString &l)
        : keyword(kw), link(l) {}
    IndexKeyword() : keyword(QString()), link(QString()) {}
    bool operator<(const IndexKeyword &ik) const {
        return keyword.toLower() < ik.keyword.toLower();
    }
    bool operator<=(const IndexKeyword &ik) const {
        return keyword.toLower() <= ik.keyword.toLower();
    }
    bool operator>(const IndexKeyword &ik) const {
        return keyword.toLower() > ik.keyword.toLower();
    }
    Q_DUMMY_COMPARISON_OPERATOR(IndexKeyword)
    QString keyword;
    QString link;
};

QDataStream &operator>>(QDataStream &s, IndexKeyword &ik)
{
    s >> ik.keyword;
    s >> ik.link;
    return s;
}

QDataStream &operator<<(QDataStream &s, const IndexKeyword &ik)
{
    s << ik.keyword;
    s << ik.link;
    return s;
}

QValidator::State SearchValidator::validate(QString &str, int &) const
{
    for (int i = 0; i < (int) str.length(); ++i) {
        QChar c = str[i];
        if (!c.isLetterOrNumber() && c != QLatin1Char('\'') && c != QLatin1Char('`')
            && c != QLatin1Char('\"') && c != QLatin1Char(' ') && c != QLatin1Char('-') && c != QLatin1Char('_')
            && c!= QLatin1Char('*'))
            return QValidator::Invalid;
    }
    return QValidator::Acceptable;
}

class IndexListModel: public QStringListModel
{
public:
    IndexListModel(QObject *parent = 0)
        : QStringListModel(parent) {}

    void clear() { contents.clear(); setStringList(QStringList()); }

    QString description(int index) const { return stringList().at(index); }
    QStringList links(int index) const { return contents.values(stringList().at(index)); }
    void addLink(const QString &description, const QString &link) { contents.insert(description, link); }

    void publish() { filter(QString()); }

    QModelIndex filter(const QString &s);

    virtual Qt::ItemFlags flags(const QModelIndex &index) const
    { return QStringListModel::flags(index) & ~Qt::ItemIsEditable; }

private:
    QMultiMap<QString, QString> contents;
};

QModelIndex IndexListModel::filter(const QString &s)
{
    QMapIterator<QString, QString> it(contents);
    QStringList lst;
    QString lastKey, lastValue;

    int goodMatch = -1;
    int perfectMatch = -1;
    if (s.isEmpty())
        perfectMatch = 0;

    while (it.hasNext()) {
        it.next();

        if (it.key() == lastKey || it.value() == lastValue)
            continue;

        lastKey = it.key();
        lastValue = it.value();

        if (lastKey.contains(s, Qt::CaseInsensitive))
            lst.append(lastKey);

        if (perfectMatch == -1 && lastKey.startsWith(s, Qt::CaseInsensitive)) {
            if (goodMatch == -1)
                goodMatch = lst.count() - 1;
            if (s.length() == lastKey.length())
                perfectMatch = lst.count() - 1;
        }
    }
    setStringList(lst);

    int bestMatch = perfectMatch;
    if (bestMatch == -1)
        bestMatch = goodMatch;

    return index(qMax(0, bestMatch), 0, QModelIndex());
}

HelpNavigationListItem::HelpNavigationListItem(QListWidget *ls, const QString &txt)
    : QListWidgetItem(txt, ls)
{
}

void HelpNavigationListItem::addLink(const QString &link)
{
    QString lnk = HelpDialog::removeAnchorFromLink(link);
    if (linkList.filter(lnk, Qt::CaseInsensitive).count() > 0)
        return;
    linkList << link;
}

HelpDialog::HelpDialog(QWidget *parent, MainWindow *h)
    : QWidget(parent), lwClosed(false), help(h)
{
    ui.setupUi(this);
    ui.listContents->setUniformRowHeights(true);
    ui.listBookmarks->setUniformRowHeights(true);

    indexModel = new IndexListModel(this);
    ui.listIndex->setModel(indexModel);
}

void HelpDialog::initialize()
{
    connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));

    connect(ui.listContents, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(showTopic(QTreeWidgetItem*)));
    connect(ui.listContents, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(showTopic(QTreeWidgetItem*)));
    connect(ui.listContents, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showTreeItemMenu(QPoint)));

    connect(ui.editIndex, SIGNAL(returnPressed()), this, SLOT(showTopic()));
    connect(ui.editIndex, SIGNAL(textChanged(QString)), this, SLOT(searchInIndex(QString)));

    connect(ui.listIndex, SIGNAL(activated(QModelIndex)), this, SLOT(showTopic()));
    connect(ui.listIndex, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showListItemMenu(QPoint)));

    connect(ui.listBookmarks, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(showTopic(QTreeWidgetItem*)));
    connect(ui.listBookmarks, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showTreeItemMenu(QPoint)));

    connect(ui.resultBox, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showListItemMenu(QPoint)));

    cacheFilesPath = QDir::homePath() + QLatin1String("/.assistant"); //### Find a better location for the dbs

    ui.editIndex->installEventFilter(this);
    ui.listBookmarks->setItemHidden(ui.listBookmarks->headerItem(), true);
    ui.listContents->setItemHidden(ui.listContents->headerItem(), true);

    ui.framePrepare->hide();
    connect(qApp, SIGNAL(lastWindowClosed()), SLOT(lastWinClosed()));

    ui.termsEdit->setValidator(new SearchValidator(ui.termsEdit));

    actionOpenCurrentTab = new QAction(this);
    actionOpenCurrentTab->setText(tr("Open Link in Current Tab"));

    actionOpenLinkInNewWindow = new QAction(this);
    actionOpenLinkInNewWindow->setText(tr("Open Link in New Window"));

    actionOpenLinkInNewTab = new QAction(this);
    actionOpenLinkInNewTab->setText(tr("Open Link in New Tab"));

    itemPopup = new QMenu(this);
    itemPopup->addAction(actionOpenCurrentTab);
    itemPopup->addAction(actionOpenLinkInNewWindow);
    itemPopup->addAction(actionOpenLinkInNewTab);

    contentList.clear();

    initDoneMsgShown = false;
    fullTextIndex = 0;
    indexDone = false;
    titleMapDone = false;
    contentsInserted = false;
    bookmarksInserted = false;
    setupTitleMap();
}


void HelpDialog::processEvents()
{
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}


void HelpDialog::lastWinClosed()
{
    lwClosed = true;
}

void HelpDialog::removeOldCacheFiles()
{
    if (!verifyDirectory(cacheFilesPath)) {
        qWarning("Failed to created assistant directory");
        return;
    }
    QString pname = QLatin1String(".") + Config::configuration()->profileName();

    QStringList fileList;
    fileList << QLatin1String("indexdb40")
        << QLatin1String("indexdb40.dict")
        << QLatin1String("indexdb40.doc")
        << QLatin1String("contentdb40");

    QStringList::iterator it = fileList.begin();
    for (; it != fileList.end(); ++it) {
		if (QFile::exists(cacheFilesPath + QDir::separator() + *it + pname)) {
            QFile f(cacheFilesPath + QDir::separator() + *it + pname);
            f.remove();
        }
    }
}

void HelpDialog::timerEvent(QTimerEvent *e)
{
    Q_UNUSED(e);
    static int opacity = 255;
    help->setWindowOpacity((opacity-=4)/255.0);
    if (opacity<=0)
        qApp->quit();
}


void HelpDialog::loadIndexFile()
{
    if (indexDone)
        return;

    setCursor(Qt::WaitCursor);
    indexDone = true;
    ui.labelPrepare->setText(tr("Prepare..."));
    ui.framePrepare->show();
    processEvents();

    QProgressBar *bar = ui.progressPrepare;
    bar->setMaximum(100);
    bar->setValue(0);


    QList<IndexKeyword> lst;
    QFile indexFile(cacheFilesPath + QDir::separator() + QLatin1String("indexdb40.") +
                     Config::configuration()->profileName());
    if (!indexFile.open(QFile::ReadOnly)) {
        buildKeywordDB();
        processEvents();
        if(lwClosed)
            return;
        if (!indexFile.open(QFile::ReadOnly)) {
            QMessageBox::warning(help, tr("Qt Assistant"), tr("Failed to load keyword index file\n"
                                                              "Assistant will not work!"));
#if defined Q_WS_WIN || defined Q_WS_MACX
            startTimer(50);
#endif
            return;
        }
    }

    QDataStream ds(&indexFile);
    quint32 fileAges;
    ds >> fileAges;
    if (fileAges != getFileAges()) {
        indexFile.close();
        buildKeywordDB();
        if (!indexFile.open(QFile::ReadOnly)) {
            QMessageBox::warning(help, tr("Qt Assistant"),
                tr("Cannot open the index file %1").arg(QFileInfo(indexFile).absoluteFilePath()));
            return;
        }
        ds.setDevice(&indexFile);
        ds >> fileAges;
    }
    ds >> lst;
    indexFile.close();

    bar->setValue(bar->maximum());
    processEvents();

    for (int i=0; i<lst.count(); ++i) {
        const IndexKeyword &idx = lst.at(i);

        indexModel->addLink(idx.keyword, idx.link);
    }

    indexModel->publish();

    ui.framePrepare->hide();
    showInitDoneMessage();
    setCursor(Qt::ArrowCursor);
}

quint32 HelpDialog::getFileAges()
{
    QStringList addDocuFiles = Config::configuration()->docFiles();
    QStringList::const_iterator i = addDocuFiles.begin();

    quint32 fileAges = 0;
    for(; i != addDocuFiles.end(); ++i) {
        QFileInfo fi(*i);
        if (fi.exists())
            fileAges += fi.lastModified().toTime_t();
    }

    return fileAges;
}

void HelpDialog::buildKeywordDB()
{
    QStringList addDocuFiles = Config::configuration()->docFiles();
    QStringList::iterator i = addDocuFiles.begin();

    int steps = 0;
    for(; i != addDocuFiles.end(); i++)
        steps += QFileInfo(*i).size();

    ui.labelPrepare->setText(tr("Prepare..."));
    ui.progressPrepare->setMaximum(steps);
    ui.progressPrepare->setValue(0);
    processEvents();

    QList<IndexKeyword> lst;
    quint32 fileAges = 0;
    for(i = addDocuFiles.begin(); i != addDocuFiles.end(); i++){
        QFile file(*i);
        if (!file.exists()) {
            QMessageBox::warning(this, tr("Warning"),
                tr("Documentation file %1 does not exist!\n"
                    "Skipping file.").arg(QFileInfo(file).absoluteFilePath()));
            continue;
        }
        fileAges += QFileInfo(file).lastModified().toTime_t();
        DocuParser *handler = DocuParser::createParser(*i);
        bool ok = handler->parse(&file);
        file.close();
        if(!ok){
            QString msg = QString::fromLatin1("In file %1:\n%2")
                          .arg(QFileInfo(file).absoluteFilePath())
                          .arg(handler->errorProtocol());
            QMessageBox::critical(this, tr("Parse Error"), tr(msg.toUtf8()));
            delete handler;
            continue;
        }

        QList<IndexItem*> indLst = handler->getIndexItems();
        int counter = 0;
        foreach (IndexItem *indItem, indLst) {
            QFileInfo fi(indItem->reference);
            lst.append(IndexKeyword(indItem->keyword, indItem->reference));
            if (ui.progressPrepare)
                ui.progressPrepare->setValue(ui.progressPrepare->value() +
                                             int(fi.absoluteFilePath().length() * 2));

            if(++counter%100 == 0) {
                processEvents();
                if(lwClosed) {
                    return;
                }
            }
        }
        delete handler;
    }
    if (!lst.isEmpty())
        qSort(lst);

    QFile indexout(cacheFilesPath + QDir::separator() + QLatin1String("indexdb40.")
		+ Config::configuration()->profileName());
    if (verifyDirectory(cacheFilesPath) && indexout.open(QFile::WriteOnly)) {
        QDataStream s(&indexout);
        s << fileAges;
        s << lst;
        indexout.close();
    }
}

void HelpDialog::setupTitleMap()
{
    if (titleMapDone)
        return;
    if (Config::configuration()->docRebuild()) {
        removeOldCacheFiles();
        Config::configuration()->setDocRebuild(false);
        Config::configuration()->saveProfile(Config::configuration()->profile());
    }
    if (contentList.isEmpty())
        getAllContents();

    titleMapDone = true;
    titleMap.clear();
    for(QList<QPair<QString, ContentList> >::Iterator it = contentList.begin(); it != contentList.end(); ++it) {
        ContentList lst = (*it).second;
        foreach (ContentItem item, lst) {
            titleMap[item.reference] = item.title.trimmed();
        }
    }
    processEvents();
}

void HelpDialog::getAllContents()
{
    QFile contentFile(cacheFilesPath + QDir::separator() + QLatin1String("contentdb40.")
		+ Config::configuration()->profileName());
    contentList.clear();
    if (!contentFile.open(QFile::ReadOnly)) {
        buildContentDict();
        return;
    }

    QDataStream ds(&contentFile);
    quint32 fileAges;
    ds >> fileAges;
    if (fileAges != getFileAges()) {
        contentFile.close();
        buildContentDict();
        return;
    }
    QString key;
    QList<ContentItem> lst;
    while (!ds.atEnd()) {
        ds >> key;
        ds >> lst;
        contentList += qMakePair(key, QList<ContentItem>(lst));
    }
    contentFile.close();
    processEvents();

}

void HelpDialog::buildContentDict()
{
    QStringList docuFiles = Config::configuration()->docFiles();

    quint32 fileAges = 0;
    for(QStringList::iterator it = docuFiles.begin(); it != docuFiles.end(); it++) {
        QFile file(*it);
        if (!file.exists()) {
            QMessageBox::warning(this, tr("Warning"),
            tr("Documentation file %1 does not exist!\n"
                "Skipping file.").arg(QFileInfo(file).absoluteFilePath()));
            continue;
        }
        fileAges += QFileInfo(file).lastModified().toTime_t();
        DocuParser *handler = DocuParser::createParser(*it);
        if(!handler) {
            QMessageBox::warning(this, tr("Warning"),
            tr("Documentation file %1 is not compatible!\n"
                "Skipping file.").arg(QFileInfo(file).absoluteFilePath()));
            continue;
        }
        bool ok = handler->parse(&file);
        file.close();
        if(ok) {
            contentList += qMakePair(*it, QList<ContentItem>(handler->getContentItems()));
            delete handler;
        } else {
            QString msg = QString::fromLatin1("In file %1:\n%2")
                          .arg(QFileInfo(file).absoluteFilePath())
                          .arg(handler->errorProtocol());
            QMessageBox::critical(this, tr("Parse Error"), tr(msg.toUtf8()));
            continue;
        }
    }

    QFile contentOut(cacheFilesPath + QDir::separator() + QLatin1String("contentdb40.")
		+ Config::configuration()->profileName());
    if (contentOut.open(QFile::WriteOnly)) {
        QDataStream s(&contentOut);
        s << fileAges;
        for(QList<QPair<QString, ContentList> >::Iterator it = contentList.begin(); it != contentList.end(); ++it) {
            s << *it;
        }
        contentOut.close();
    }
}

void HelpDialog::currentTabChanged(int index)
{
    QString s = ui.tabWidget->tabText(index);
    if (stripAmpersand(s).contains(tr("Index")))
        QTimer::singleShot(0, this, SLOT(loadIndexFile()));
    else if (stripAmpersand(s).contains(tr("Bookmarks")))
        insertBookmarks();
    else if (stripAmpersand(s).contains(tr("Contents")))
        QTimer::singleShot(0, this, SLOT(insertContents()));
    else if (stripAmpersand(s).contains(tr("Search")))
        QTimer::singleShot(0, this, SLOT(setupFullTextIndex()));
}

void HelpDialog::showInitDoneMessage()
{
    if (initDoneMsgShown)
        return;
    initDoneMsgShown = true;
    help->statusBar()->showMessage(tr("Done"), 3000);
}

void HelpDialog::showTopic(QTreeWidgetItem *item)
{
    if(item)
        showTopic();
}

void HelpDialog::showTopic()
{
    QString text = ui.tabWidget->tabText(ui.tabWidget->currentIndex());

    if (stripAmpersand(text).contains(tr("Index")))
        showIndexTopic();
    else if (stripAmpersand(text).contains(tr("Bookmarks")))
        showBookmarkTopic();
    else if (stripAmpersand(text).contains(tr("Contents")))
        showContentsTopic();
}

void HelpDialog::showIndexTopic()
{
    int row = ui.listIndex->currentIndex().row();
    if (row == -1)
        return;

    QString description = indexModel->description(row);

    bool blocked = ui.editIndex->blockSignals(true);
    ui.editIndex->setText(description);
    ui.editIndex->blockSignals(blocked);

    QStringList links = indexModel->links(row);
    if (links.count() == 1) {
        emit showLink(links.first());
    } else {
        qSort(links);
        QStringList::Iterator it = links.begin();
        QStringList linkList;
        QStringList linkNames;
        for (; it != links.end(); ++it) {
            linkList << *it;
            linkNames << titleOfLink(*it);
        }
        QString link = TopicChooser::getLink(this, linkNames, linkList, description);
        if (!link.isEmpty())
            emit showLink(link);
    }

    indexModel->publish();
    ui.listIndex->setCurrentIndex(indexModel->index(indexModel->stringList().indexOf(description)));
    ui.listIndex->scrollTo(ui.listIndex->currentIndex(), QAbstractItemView::PositionAtTop);
}

void HelpDialog::searchInIndex(const QString &s)
{
    ui.listIndex->setCurrentIndex(indexModel->filter(s));
}

QString HelpDialog::titleOfLink(const QString &link)
{
    QString s = HelpDialog::removeAnchorFromLink(link);
    s = titleMap[s];
    if (s.isEmpty())
        return link;
    return s;
}

bool HelpDialog::eventFilter(QObject * o, QEvent * e)
{
    if (!o || !e)
        return true;

    if (o == ui.editIndex && e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = (QKeyEvent*)e;
        if (ke->key() == Qt::Key_Up) {
            QModelIndex index = ui.listIndex->currentIndex();
            int row = index.row();
            if (--row >= 0) {
                ui.listIndex->setCurrentIndex(indexModel->index(row, 0, QModelIndex()));
                bool blocked = ui.editIndex->blockSignals(true);
                ui.editIndex->setText(indexModel->description(row));
                ui.editIndex->blockSignals(blocked);
            }
            return true;
        } else if (ke->key() == Qt::Key_Down) {
            QModelIndex index = ui.listIndex->currentIndex();
            int row = index.row();
            if (++row < indexModel->rowCount()) {
                ui.listIndex->setCurrentIndex(indexModel->index(row, 0, QModelIndex()));
                bool blocked = ui.editIndex->blockSignals(true);
                ui.editIndex->setText(indexModel->description(row));
                ui.editIndex->blockSignals(blocked);
            }
            return true;
        } else if (ke->key() == Qt::Key_PageDown || ke->key() == Qt::Key_PageUp) {
            QModelIndex index = ui.listIndex->currentIndex();
            int row = index.row();
            QApplication::sendEvent(ui.listIndex, e);
            if (row != -1) {
                bool blocked = ui.editIndex->blockSignals(true);
                ui.editIndex->setText(indexModel->description(row));
                ui.editIndex->blockSignals(blocked);
            }
        }
    }

    return QWidget::eventFilter(o, e);
}

void HelpDialog::addBookmark()
{
    if (!bookmarksInserted)
        insertBookmarks();
    QString link = help->browsers()->currentBrowser()->source().toString();
    QString title = help->browsers()->currentBrowser()->documentTitle();
    if (title.isEmpty())
        title = titleOfLink(link);

    QTreeWidgetItem *i = new QTreeWidgetItem(ui.listBookmarks, 0);
    i->setText(0, title);
    i->setData(0, LinkRole, link);
    saveBookmarks();
    help->updateBookmarkMenu();
}

void HelpDialog::on_buttonAdd_clicked()
{
    addBookmark();
}

void HelpDialog::on_buttonRemove_clicked()
{
    if (!ui.listBookmarks->currentItem())
        return;

    delete ui.listBookmarks->currentItem();
    saveBookmarks();
    if (ui.listBookmarks->topLevelItemCount() != 0) {
        ui.listBookmarks->setCurrentItem(ui.listBookmarks->topLevelItem(0));
    }
    help->updateBookmarkMenu();
}

void HelpDialog::insertBookmarks()
{
    if (bookmarksInserted)
        return;
    bookmarksInserted = true;
    ui.listBookmarks->clear();
    QFile f(cacheFilesPath + QDir::separator() + QLatin1String("bookmarks.")
		+ Config::configuration()->profileName());
    if (!f.open(QFile::ReadOnly))
        return;
    QTextStream ts(&f);
    while (!ts.atEnd()) {
        QTreeWidgetItem *i = new QTreeWidgetItem(ui.listBookmarks, 0);
        i->setText(0, ts.readLine());
        i->setData(0, LinkRole, ts.readLine());
    }
    help->updateBookmarkMenu();
    showInitDoneMessage();
}

void HelpDialog::showBookmarkTopic()
{
    if (!ui.listBookmarks->currentItem())
        return;

    QTreeWidgetItem *i = (QTreeWidgetItem*)ui.listBookmarks->currentItem();
    emit showLink(i->data(0, LinkRole).toString());
}

static void store(QTreeWidgetItem *i, QTextStream &ts)
{
    ts << i->text(0) << endl;
    ts << i->data(0, LinkRole).toString() << endl;

    for (int index = 0; index < i->childCount(); ++index)
        store(i->child(index), ts);
}

static void store(QTreeWidget *tw, QTextStream &ts)
{
    for (int index = 0; index < tw->topLevelItemCount(); ++index)
        store(tw->topLevelItem(index), ts);
}

void HelpDialog::saveBookmarks()
{
    QFile f(cacheFilesPath + QDir::separator() + QLatin1String("bookmarks.")
		+ Config::configuration()->profileName());
    if (!f.open(QFile::WriteOnly))
        return;

    QTextStream ts(&f);
    store(ui.listBookmarks, ts);
    f.close();
}

void HelpDialog::insertContents()
{
    if (contentsInserted)
        return;

    if (contentList.isEmpty())
        getAllContents();

    contentsInserted = true;
    ui.listContents->clear();
    setCursor(Qt::WaitCursor);
    if (!titleMapDone)
        setupTitleMap();

#if 0 // ### port me
    ui.listContents->setSorting(-1);
#endif

    for(QList<QPair<QString, ContentList> >::Iterator it = contentList.begin(); it != contentList.end(); ++it) {
        QTreeWidgetItem *newEntry;

        QTreeWidgetItem *contentEntry;
        QStack<QTreeWidgetItem*> stack;
        stack.clear();
        int depth = 0;
        bool root = false;

        QTreeWidgetItem *lastItem[64];
        for(int j = 0; j < 64; ++j)
            lastItem[j] = 0;

        ContentList lst = (*it).second;
        for (ContentList::ConstIterator it = lst.begin(); it != lst.end(); ++it) {
            ContentItem item = *it;
            if (item.depth == 0) {
                newEntry = new QTreeWidgetItem(ui.listContents, 0);
                newEntry->setIcon(0, QIcon(QString::fromUtf8(":/trolltech/assistant/images/win/book.png")));
                newEntry->setText(0, item.title);
                newEntry->setData(0, LinkRole, item.reference);
                stack.push(newEntry);
                depth = 1;
                root = true;
            }
            else{
                if((item.depth > depth) && root) {
                    depth = item.depth;
                    stack.push(contentEntry);
                }
                if(item.depth == depth) {
                    contentEntry = new QTreeWidgetItem(stack.top(), lastItem[ depth ]);
                    lastItem[ depth ] = contentEntry;
                    contentEntry->setText(0, item.title);
                    contentEntry->setData(0, LinkRole, item.reference);
                }
                else if(item.depth < depth) {
                    stack.pop();
                    depth--;
                    item = *(--it);
                }
            }
        }
        processEvents();
    }
    setCursor(Qt::ArrowCursor);
    showInitDoneMessage();
}

void HelpDialog::showContentsTopic()
{
    QTreeWidgetItem *i = (QTreeWidgetItem*)ui.listContents->currentItem();
    if (!i)
        return;
    emit showLink(i->data(0, LinkRole).toString());
}

void HelpDialog::toggleContents()
{
    if (!isVisible() || ui.tabWidget->currentIndex() != 0) {
        ui.tabWidget->setCurrentIndex(0);
        parentWidget()->show();
    }
    else
        parentWidget()->hide();
}

void HelpDialog::toggleIndex()
{
    if (!isVisible() || ui.tabWidget->currentIndex() != 1 || !ui.editIndex->hasFocus()) {
        ui.tabWidget->setCurrentIndex(1);
        parentWidget()->show();
        ui.editIndex->setFocus();
    }
    else
        parentWidget()->hide();
}

void HelpDialog::toggleBookmarks()
{
    if (!isVisible() || ui.tabWidget->currentIndex() != 2) {
        ui.tabWidget->setCurrentIndex(2);
        parentWidget()->show();
    }
    else
        parentWidget()->hide();
}

void HelpDialog::toggleSearch()
{
    if (!isVisible() || ui.tabWidget->currentIndex() != 3) {
        ui.tabWidget->setCurrentIndex(3);
        parentWidget()->show();
    }
    else
        parentWidget()->hide();
}

void HelpDialog::setupFullTextIndex()
{
    if (fullTextIndex)
        return;

    QMap<QString, QString>::ConstIterator it = titleMap.begin();
    QStringList documentList;
    QString doc;
    for (; it != titleMap.end(); ++it) {
        doc = HelpDialog::removeAnchorFromLink(it.key());
        if (documentList.filter(doc, Qt::CaseInsensitive).count() == 0)
            documentList << doc;
    }

    QString pname = Config::configuration()->profileName();
    fullTextIndex = new Index(documentList, QDir::homePath()); // ### Is this correct ?
    if (!verifyDirectory(cacheFilesPath)) {
        QMessageBox::warning(help, tr("Qt Assistant"),
                             tr("Failed to save fulltext search index\n"
                                "Assistant will not work!"));
        return;
    }
    fullTextIndex->setDictionaryFile(cacheFilesPath + QDir::separator() + QLatin1String("indexdb40.dict.") + pname);
    fullTextIndex->setDocListFile(cacheFilesPath + QDir::separator() + QLatin1String("indexdb40.doc.") + pname);
    processEvents();

    connect(fullTextIndex, SIGNAL(indexingProgress(int)),
             this, SLOT(setIndexingProgress(int)));
    QFile f(cacheFilesPath + QDir::separator() + QLatin1String("indexdb40.dict.") + pname);
    if (!f.exists()) {
        help->statusBar()->clearMessage();
        setCursor(Qt::WaitCursor);
        ui.labelPrepare->setText(tr("Indexing files..."));
        ui.progressPrepare->setMaximum(100);
        ui.progressPrepare->reset();
        ui.progressPrepare->show();
        ui.framePrepare->show();
        processEvents();
        if (fullTextIndex->makeIndex() == -1)
            return;
        fullTextIndex->writeDict();
        ui.progressPrepare->setValue(100);
        ui.framePrepare->hide();
        setCursor(Qt::ArrowCursor);
        showInitDoneMessage();
    } else {
        setCursor(Qt::WaitCursor);
        help->statusBar()->showMessage(tr("Reading dictionary..."));
        processEvents();
        fullTextIndex->readDict();
        help->statusBar()->showMessage(tr("Done"), 3000);
        setCursor(Qt::ArrowCursor);
    }
}

void HelpDialog::setIndexingProgress(int prog)
{
    ui.progressPrepare->setValue(prog);
    processEvents();
}

void HelpDialog::startSearch()
{
    QString str = ui.termsEdit->text();
    str = str.simplified();
    str = str.replace(QLatin1String("\'"), QLatin1String("\""));
    str = str.replace(QLatin1String("`"), QLatin1String("\""));
    QString buf = str;
    str = str.replace(QLatin1String("-"), QLatin1String(" "));
    str = str.replace(QRegExp(QLatin1String("\\s[\\S]?\\s")), QLatin1String(" "));
    terms = str.split(QLatin1Char(' '));
    QStringList termSeq;
    QStringList seqWords;
    QStringList::iterator it = terms.begin();
    for (; it != terms.end(); ++it) {
        (*it) = (*it).simplified();
        (*it) = (*it).toLower();
        (*it) = (*it).replace(QLatin1String("\""), QLatin1String(""));
    }
    if (str.contains(QLatin1Char('\"'))) {
        if ((str.count(QLatin1Char('\"')))%2 == 0) {
            int beg = 0;
            int end = 0;
            QString s;
            beg = str.indexOf(QLatin1Char('\"'), beg);
            while (beg != -1) {
                beg++;
                end = str.indexOf(QLatin1Char('\"'), beg);
                s = str.mid(beg, end - beg);
                s = s.toLower();
                s = s.simplified();
                if (s.contains(QLatin1Char('*'))) {
                    QMessageBox::warning(this, tr("Full Text Search"),
                        tr("Using a wildcard within phrases is not allowed."));
                    return;
                }
                seqWords += s.split(QLatin1Char(' '));
                termSeq << s;
                beg = str.indexOf(QLatin1Char('\"'), end + 1);
            }
        } else {
            QMessageBox::warning(this, tr("Full Text Search"),
                tr("The closing quotation mark is missing."));
            return;
        }
    }
    setCursor(Qt::WaitCursor);
    foundDocs.clear();
    foundDocs = fullTextIndex->query(terms, termSeq, seqWords);
    QString msg = QString::fromLatin1("%1 documents found.").arg(foundDocs.count());
    help->statusBar()->showMessage(tr(msg.toUtf8()), 3000);
    ui.resultBox->clear();
    for (it = foundDocs.begin(); it != foundDocs.end(); ++it)
        ui.resultBox->addItem(fullTextIndex->getDocumentTitle(*it));

    terms.clear();
    bool isPhrase = false;
    QString s;
    for (int i = 0; i < (int)buf.length(); ++i) {
        if (buf[i] == QLatin1Char('\"')) {
            isPhrase = !isPhrase;
            s = s.simplified();
            if (!s.isEmpty())
                terms << s;
            s = QLatin1String("");
        } else if (buf[i] == QLatin1Char(' ') && !isPhrase) {
            s = s.simplified();
            if (!s.isEmpty())
                terms << s;
            s = QLatin1String("");
        } else
            s += buf[i];
    }
    if (!s.isEmpty())
        terms << s;

    setCursor(Qt::ArrowCursor);
}

void HelpDialog::on_helpButton_clicked()
{
    emit showLink(MainWindow::urlifyFileName(
                  Config::configuration()->assistantDocPath() +
                  QLatin1String("/assistant-manual.html#full-text-searching")));
}

void HelpDialog::on_resultBox_itemActivated(QListWidgetItem *item)
{
    showResultPage(item);
}

void HelpDialog::showResultPage(QListWidgetItem *item)
{
    if (item)
        emit showSearchLink(foundDocs[ui.resultBox->row(item)], terms);
}

void HelpDialog::showListItemMenu(const QPoint &pos)
{
    QListWidget *listWidget = qobject_cast<QListWidget*>(sender());

    if (!listWidget)
        return;

    QListWidgetItem *item = listWidget->itemAt(pos);
    if (!item)
        return;

    QString currentTabText = ui.tabWidget->tabText(ui.tabWidget->currentIndex());

    QAction *action = itemPopup->exec(listWidget->viewport()->mapToGlobal(pos));
    if (action == actionOpenCurrentTab) {
        if (stripAmpersand(currentTabText).contains(tr("Index")))
            showTopic();
        else {
            showResultPage(item);
        }
    } else if (action) {
        HelpWindow *hw = help->browsers()->currentBrowser();
        if (stripAmpersand(currentTabText).contains(tr("Index"))) {
            bool blocked = ui.editIndex->blockSignals(true);
            ui.editIndex->setText(item->text());
            ui.editIndex->blockSignals(blocked);

            HelpNavigationListItem *hi = (HelpNavigationListItem*)item;

            QStringList links = hi->links();
            if (links.count() == 1) {
                if (action == actionOpenLinkInNewWindow)
                    hw->openLinkInNewWindow(links.first());
                else
                    hw->openLinkInNewPage(links.first());
            } else {
                QStringList::Iterator it = links.begin();
                QStringList linkList;
                QStringList linkNames;
                for (; it != links.end(); ++it) {
                    linkList << *it;
                    linkNames << titleOfLink(*it);
                }
                QString link = TopicChooser::getLink(this, linkNames, linkList, item->text());
                if (!link.isEmpty()) {
                    if (action == actionOpenLinkInNewWindow)
                        hw->openLinkInNewWindow(link);
                    else
                        hw->openLinkInNewPage(link);
                }
            }
        } else {
            QString link = foundDocs[ ui.resultBox->row(item) ];
            if (action == actionOpenLinkInNewWindow)
                hw->openLinkInNewWindow(link);
            else
                hw->openLinkInNewPage(link);
        }
    }
}

void HelpDialog::showTreeItemMenu(const QPoint &pos)
{
    QTreeWidget *treeWidget = qobject_cast<QTreeWidget*>(sender());

    if (!treeWidget)
        return;

    QTreeWidgetItem *item = treeWidget->itemAt(pos);

    if (!item)
        return;

    QAction *action = itemPopup->exec(treeWidget->viewport()->mapToGlobal(pos));
    if (action == actionOpenCurrentTab) {
        if (stripAmpersand(ui.tabWidget->tabText(ui.tabWidget->currentIndex())).contains(tr("Contents")))
            showContentsTopic();
        else
            showBookmarkTopic();
    } else if (action) {
        QTreeWidgetItem *i = (QTreeWidgetItem*)item;
        if (action == actionOpenLinkInNewWindow)
            help->browsers()->currentBrowser()->openLinkInNewWindow(i->data(0, LinkRole).toString());
        else
            help->browsers()->currentBrowser()->openLinkInNewPage(i->data(0, LinkRole).toString());
    }
}

void HelpDialog::on_termsEdit_returnPressed()
{
    startSearch();
}

void HelpDialog::on_searchButton_clicked()
{
    startSearch();
}

QString HelpDialog::removeAnchorFromLink(const QString &link)
{
    int i = link.length();
	int j = link.lastIndexOf('/');
    int l = link.lastIndexOf(QDir::separator());
    if (l > j)
        j = l;
	if (j > -1) {
		QString fileName = link.mid(j+1);
		int k = fileName.lastIndexOf('#');
		if (k > -1)
			i = j + k + 1;
	}
	return link.left(i);
}
