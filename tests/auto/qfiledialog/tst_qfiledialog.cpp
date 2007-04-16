/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qcoreapplication.h>
#include <qdebug.h>
#include <qfiledialog.h>
#include <qabstractitemdelegate.h>
#include <qdirmodel.h>
#include <qitemdelegate.h>
#include <qlistview.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qtreeview.h>
#include <qaction.h>
#include <qdialogbuttonbox.h>
#include <qsortfilterproxymodel.h>

//TESTED_CLASS=
//TESTED_FILES=qfiledialog.h

class tst_QFiledialog : public QObject
{
Q_OBJECT

public:
    tst_QFiledialog();
    virtual ~tst_QFiledialog();

public slots:
    void init();

private slots:
    void directory();
    void acceptMode();
    void confirmOverwrite();
    void defaultSuffix();
    void fileMode();
    void filters();
    void history();
    void iconProvider();
    void isReadOnly();
    void itemDelegate();
    void labelText();
    void resolveSymlinks();
    void selectFile();
    void selectFilter();
    void viewMode();
    void proxymodel();
    void setFilter();
    void focus();

    void disableSaveButton_data();
    void disableSaveButton();
};

tst_QFiledialog::tst_QFiledialog()
{
}

tst_QFiledialog::~tst_QFiledialog()
{
}

void tst_QFiledialog::init()
{
    QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
    settings.beginGroup(QLatin1String("Qt"));
    settings.remove(QLatin1String("filedialog"));
}

class MyAbstractItemDelegate : public QAbstractItemDelegate
{
public:
    MyAbstractItemDelegate() : QAbstractItemDelegate() {};
    void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const {}
    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const { return QSize(); }
};

void tst_QFiledialog::directory()
{
    QFileDialog fd;
    QCOMPARE(QDir::current().absolutePath(), fd.directory().absolutePath());
    QDir temp = QDir::temp();
    fd.setDirectory(temp);
    QCOMPARE(temp.absolutePath(), fd.directory().absolutePath());

    // Check my way
    QList<QListView*> list = fd.findChildren<QListView*>("listView");
    QVERIFY(list.count() > 0);
    QCOMPARE(list.at(0)->rootIndex().data().toString(), temp.dirName());
}

void tst_QFiledialog::acceptMode()
{
    QFileDialog fd;
    fd.show();

    QToolButton* newButton = fd.findChild<QToolButton*>("newFolderButton");
    QVERIFY(newButton);

    // default
    QCOMPARE(fd.acceptMode(), QFileDialog::AcceptOpen);
    QCOMPARE(newButton && newButton->isVisible(), true);

    //fd.setDetailsExpanded(true);
    fd.setAcceptMode(QFileDialog::AcceptSave);
    QCOMPARE(fd.acceptMode(), QFileDialog::AcceptSave);
    QCOMPARE(newButton->isVisible(), true);

    fd.setAcceptMode(QFileDialog::AcceptOpen);
    QCOMPARE(fd.acceptMode(), QFileDialog::AcceptOpen);
    QCOMPARE(newButton->isVisible(), true);
}

void tst_QFiledialog::confirmOverwrite()
{
    QFileDialog fd;
    QCOMPARE(fd.confirmOverwrite(), true);
    fd.setConfirmOverwrite(true);
    QCOMPARE(fd.confirmOverwrite(), true);
    fd.setConfirmOverwrite(false);
    QCOMPARE(fd.confirmOverwrite(), false);
    fd.setConfirmOverwrite(true);
    QCOMPARE(fd.confirmOverwrite(), true);
}

void tst_QFiledialog::defaultSuffix()
{
    QFileDialog fd;
    QCOMPARE(fd.defaultSuffix(), QString());
    fd.setDefaultSuffix("txt");
    QCOMPARE(fd.defaultSuffix(), QString("txt"));
    fd.setDefaultSuffix(QString());
    QCOMPARE(fd.defaultSuffix(), QString());
}

void tst_QFiledialog::fileMode()
{
    QFileDialog fd;
    QCOMPARE(fd.fileMode(), QFileDialog::AnyFile);
    fd.setFileMode(QFileDialog::ExistingFile);
    QCOMPARE(fd.fileMode(), QFileDialog::ExistingFile);
    fd.setFileMode(QFileDialog::Directory);
    QCOMPARE(fd.fileMode(), QFileDialog::Directory);
    fd.setFileMode(QFileDialog::DirectoryOnly);
    QCOMPARE(fd.fileMode(), QFileDialog::DirectoryOnly);
    fd.setFileMode(QFileDialog::ExistingFiles);
    QCOMPARE(fd.fileMode(), QFileDialog::ExistingFiles);
}

void tst_QFiledialog::filters()
{
    QFileDialog fd;
    QCOMPARE(fd.filters(), QStringList("All Files (*.*)"));

    // effects
    QList<QComboBox*> views = fd.findChildren<QComboBox*>("fileTypeCombo");
    QVERIFY(views.count() == 1);
    QCOMPARE(views.at(0)->isVisible(), false);

    QStringList filters;
    filters << "Image files (*.png *.xpm *.jpg)"
         << "Text files (*.txt)"
         << "Any files (*.*)";
    fd.setFilters(filters);
    QCOMPARE(views.at(0)->isVisible(), false);
    fd.show();
    fd.setAcceptMode(QFileDialog::AcceptSave);
    QCOMPARE(views.at(0)->isVisible(), true);
    QCOMPARE(fd.filters(), filters);
    fd.setFilter("Image files (*.png *.xpm *.jpg);;Text files (*.txt);;Any files (*.*)");
    QCOMPARE(fd.filters(), filters);
}

void tst_QFiledialog::selectFilter()
{
    QFileDialog fd;
    QCOMPARE(fd.selectedFilter(), QString("All Files (*.*)"));
    QStringList filters;
    filters << "Image files (*.png *.xpm *.jpg)"
         << "Text files (*.txt)"
         << "Any files (*.*)";
    fd.setFilters(filters);
    QCOMPARE(fd.selectedFilter(), filters.at(0));
    fd.selectFilter(filters.at(1));
    QCOMPARE(fd.selectedFilter(), filters.at(1));
    fd.selectFilter(filters.at(2));
    QCOMPARE(fd.selectedFilter(), filters.at(2));

    fd.selectFilter("bob");
    QCOMPARE(fd.selectedFilter(), filters.at(2));
    fd.selectFilter("");
    QCOMPARE(fd.selectedFilter(), filters.at(2));
}

void tst_QFiledialog::history()
{
    QFileDialog fd;
    QCOMPARE(fd.history(), QStringList(QDir::toNativeSeparators(QDir::current().absolutePath())));
    QStringList history;
    history << QDir::toNativeSeparators(QDir::current().absolutePath())
            << QDir::toNativeSeparators(QDir::home().absolutePath())
            << QDir::toNativeSeparators(QDir::temp().absolutePath());
    fd.setHistory(history);
    if (fd.history() != history)
        qDebug() << fd.history() << history << QDir::currentPath();
    QCOMPARE(fd.history(), history);

    QStringList badHistory;
    badHistory << "junk";
    fd.setHistory(badHistory);
    badHistory << QDir::toNativeSeparators(QDir::current().absolutePath());
    QCOMPARE(fd.history(), badHistory);
}

void tst_QFiledialog::iconProvider()
{
    QFileDialog fd;
    QVERIFY(fd.iconProvider() != 0);
    QFileIconProvider ip;
    fd.setIconProvider(&ip);
    QCOMPARE(fd.iconProvider(), &ip);
}

void tst_QFiledialog::isReadOnly()
{
    QFileDialog fd;

    QPushButton* newButton = fd.findChild<QPushButton*>("newFolderButton");
    QAction* renameAction = fd.findChild<QAction*>("qt_rename_action");
    QAction* deleteAction = fd.findChild<QAction*>("qt_delete_action");

    QCOMPARE(fd.isReadOnly(), false);

    // This is dependent upon the file/dir, find cross platform way to test
    //fd.setDirectory(QDir::home());
    //QCOMPARE(newButton && newButton->isEnabled(), true);
    //QCOMPARE(renameAction && renameAction->isEnabled(), true);
    //QCOMPARE(deleteAction && deleteAction->isEnabled(), true);

    fd.setReadOnly(true);
    QCOMPARE(fd.isReadOnly(), true);

    QCOMPARE(newButton && newButton->isEnabled(), false);
    QCOMPARE(renameAction && renameAction->isEnabled(), false);
    QCOMPARE(deleteAction && deleteAction->isEnabled(), false);
}

void tst_QFiledialog::itemDelegate()
{
    QFileDialog fd;
    QVERIFY(fd.itemDelegate() != 0);
    QItemDelegate *id = new QItemDelegate(&fd);
    fd.setItemDelegate(id);
    QCOMPARE(fd.itemDelegate(), id);
}

void tst_QFiledialog::labelText()
{
    QFileDialog fd;
    QCOMPARE(fd.labelText(QFileDialog::LookIn), QString("Look in:"));
    QCOMPARE(fd.labelText(QFileDialog::FileName), QString("File &name:"));
    QCOMPARE(fd.labelText(QFileDialog::FileType), QString("Files of type:"));
    QCOMPARE(fd.labelText(QFileDialog::Accept), QString("&Open"));
    QCOMPARE(fd.labelText(QFileDialog::Reject), QString("Cancel"));

    fd.setLabelText(QFileDialog::LookIn, "1");
    QCOMPARE(fd.labelText(QFileDialog::LookIn), QString("1"));
    fd.setLabelText(QFileDialog::FileName, "2");
    QCOMPARE(fd.labelText(QFileDialog::FileName), QString("2"));
    fd.setLabelText(QFileDialog::FileType, "3");
    QCOMPARE(fd.labelText(QFileDialog::FileType), QString("3"));
    fd.setLabelText(QFileDialog::Accept, "4");
    QCOMPARE(fd.labelText(QFileDialog::Accept), QString("4"));
    fd.setLabelText(QFileDialog::Reject, "5");
    QCOMPARE(fd.labelText(QFileDialog::Reject), QString("5"));
}

void tst_QFiledialog::resolveSymlinks()
{
    QFileDialog fd;

    // default
#ifndef Q_OS_WIN
    QCOMPARE(fd.resolveSymlinks(), false);
#else
    QCOMPARE(fd.resolveSymlinks(), true);
#endif
    fd.setResolveSymlinks(true);
#ifdef Q_OS_WIN
    QCOMPARE(fd.resolveSymlinks(), true);
#else
    QCOMPARE(fd.resolveSymlinks(), false);
#endif

    fd.setResolveSymlinks(false);
    QCOMPARE(fd.resolveSymlinks(), false);

    // the file dialog doesn't do anything based upon this, just passes it to the model
    // the model should fully test it, don't test it here
}

void tst_QFiledialog::selectFile()
{
    QFileDialog fd;

    // default value
    QCOMPARE(fd.selectedFiles().count(), 1);
}

void tst_QFiledialog::viewMode()
{
    QFileDialog fd;
    fd.show();

    // find widgets
    QList<QTreeView*> treeView = fd.findChildren<QTreeView*>("treeView");
    QCOMPARE(treeView.count(), 1);
    QList<QListView*> listView = fd.findChildren<QListView*>("listView");
    QCOMPARE(listView.count(), 1);
    QList<QToolButton*> listButton = fd.findChildren<QToolButton*>("listModeButton");
    QCOMPARE(listButton.count(), 1);
    QList<QToolButton*> treeButton = fd.findChildren<QToolButton*>("detailModeButton");
    QCOMPARE(treeButton.count(), 1);

    // default value
    QCOMPARE(fd.viewMode(), QFileDialog::List);

    // detail
    fd.setViewMode(QFileDialog::ViewMode(QFileDialog::Detail));

    QCOMPARE(QFileDialog::ViewMode(QFileDialog::Detail), fd.viewMode());
    QCOMPARE(listView.at(0)->isVisible(), false);
    QCOMPARE(listButton.at(0)->isDown(), false);
    QCOMPARE(treeView.at(0)->isVisible(), true);
    QCOMPARE(treeButton.at(0)->isDown(), true);

    // list
    fd.setViewMode(QFileDialog::ViewMode(QFileDialog::List));

    QCOMPARE(QFileDialog::ViewMode(QFileDialog::List), fd.viewMode());
    QCOMPARE(treeView.at(0)->isVisible(), false);
    QCOMPARE(treeButton.at(0)->isDown(), false);
    QCOMPARE(listView.at(0)->isVisible(), true);
    QCOMPARE(listButton.at(0)->isDown(), true);
}

void tst_QFiledialog::proxymodel()
{
    QFileDialog fd;
    QCOMPARE(fd.proxyModel(), (QAbstractProxyModel*)0);

    fd.setProxyModel(0);
    QCOMPARE(fd.proxyModel(), (QAbstractProxyModel*)0);

    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(&fd);
    fd.setProxyModel(proxyModel);
    QCOMPARE(fd.proxyModel(), proxyModel);

    fd.setProxyModel(0);
    QCOMPARE(fd.proxyModel(), (QAbstractProxyModel*)0);
}

void tst_QFiledialog::setFilter()
{
    QFileDialog fd;
    fd.setFilter(QString());
    fd.setFilters(QStringList());
}

void tst_QFiledialog::focus()
{
    QFileDialog fd;
    fd.show();
    qApp->processEvents();

    QList<QWidget*> treeView = fd.findChildren<QWidget*>("fileNameEdit");
    QCOMPARE(treeView.count(), 1);
    QVERIFY(treeView.at(0));
    QCOMPARE(treeView.at(0)->hasFocus(), true);
}

void tst_QFiledialog::disableSaveButton_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("isEnabled");

    QTest::newRow("valid path") << QDir::temp().absolutePath() + QDir::separator() + "foo" << true;
    QTest::newRow("no path") << "" << false;
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC) && !defined(Q_OS_OPENBSD)
    QTest::newRow("too long path") << "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii" << false;
#endif
}

void tst_QFiledialog::disableSaveButton()
{
    QFETCH(QString, path);
    QFETCH(bool, isEnabled);

    QFileDialog fd(0, "caption", path);
    fd.setAcceptMode(QFileDialog::AcceptSave);
    QDialogButtonBox *buttonBox = fd.findChild<QDialogButtonBox*>("buttonBox");
    QPushButton *button = buttonBox->button(QDialogButtonBox::Save);
    QVERIFY(button);
    QCOMPARE(button->isEnabled(), isEnabled);
}

QTEST_MAIN(tst_QFiledialog)
#include "tst_qfiledialog.moc"
