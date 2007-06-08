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
#include <qlineedit.h>

// Will try to wait for the condition while allowing event processing
// for a maximum of 2 seconds.
#define WAIT_FOR_CONDITION(expr, expected) \
    do { \
        const int step = 100; \
        for (int i = 0; i < 2000 && expr != expected; i+=step) { \
            QTest::qWait(step); \
        } \
    } while(0)

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
    void cleanup();

private slots:
    void args();
    void directory();
    void completor();
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
    void selectFiles();
    void selectFilter();
    void viewMode();
    void proxymodel();
    void setFilter();
    void focus();

    void historyBack();
    void historyForward();

    void disableSaveButton_data();
    void disableSaveButton();

    void clearLineEdit();
    void enableChooseButton();

private:
    QByteArray userSettings;
};

tst_QFiledialog::tst_QFiledialog()
{
}

tst_QFiledialog::~tst_QFiledialog()
{
}

void tst_QFiledialog::init()
{
    // Save the developers settings so they don't get mad when their sidebar folders are gone.
    QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
    settings.beginGroup(QLatin1String("Qt"));
    userSettings = settings.value(QLatin1String("filedialog")).toByteArray();
    settings.remove(QLatin1String("filedialog"));

    // populate it with some default settings
    QFileDialog fd;
}

void tst_QFiledialog::cleanup()
{
    QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
    settings.beginGroup(QLatin1String("Qt"));
    settings.setValue(QLatin1String("filedialog"), userSettings);
}

class MyAbstractItemDelegate : public QAbstractItemDelegate
{
public:
    MyAbstractItemDelegate() : QAbstractItemDelegate() {};
    void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const {}
    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const { return QSize(); }
};

void tst_QFiledialog::args()
{
    QWidget *parent = 0;
    QString caption = "caption";
    QString directory = QDir::tempPath();
    QString filter = "*.mp3";
    QFileDialog fd(parent, caption, directory, filter);
    QCOMPARE(fd.parent(), (QObject *)parent);
    QCOMPARE(fd.windowTitle(), caption);
    QCOMPARE(fd.directory(), QDir(directory));
    QCOMPARE(fd.filters(), QStringList(filter));
}

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

void tst_QFiledialog::completor()
{
    // ### flesh this out more
    QFileDialog fd;
    fd.show();
    QLineEdit *lineEdit = fd.findChild<QLineEdit*>("fileNameEdit");
    QVERIFY(lineEdit);
    int depth = QDir::currentPath().split('/').count();
    for (int i = 0; i <= depth * 3 + 1; ++i) {
        lineEdit->insert("../");
        qApp->processEvents();
    }
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
    QCOMPARE(fd.filters(), QStringList("All Files (*)"));

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
    QCOMPARE(fd.selectedFilter(), QString("All Files (*)"));
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
    if (fd.history() != history) {
        qDebug() << fd.history() << history;
        // quick and dirty output for windows failure.
        QListView* list = fd.findChild<QListView*>("listView");
        QVERIFY(list);
        QModelIndex root = list->rootIndex();
        while (root.isValid()) {
            qDebug() << root.data();
            root = root.parent();
        }
    }
    QCOMPARE(fd.history(), history);

    QStringList badHistory;
    badHistory << "junk";
    fd.setHistory(badHistory);
    badHistory << QDir::toNativeSeparators(QDir::current().absolutePath());
    QCOMPARE(fd.history(), badHistory);
}

void tst_QFiledialog::iconProvider()
{
    QFileDialog *fd = new QFileDialog();
    QVERIFY(fd->iconProvider() != 0);
    QFileIconProvider *ip = new QFileIconProvider();
    fd->setIconProvider(ip);
    QCOMPARE(fd->iconProvider(), ip);
    delete fd;
    delete ip;
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
    QCOMPARE(fd.itemDelegate(), (QAbstractItemDelegate *)id);
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

void tst_QFiledialog::selectFiles()
{
    QFileDialog fd;
    fd.show();
    fd.setFileMode(QFileDialog::ExistingFiles);

    // Get a list of files in the view and then get the corresponding index's
    QStringList list = fd.directory().entryList(QDir::Files);
    QModelIndexList toSelect;
    QVERIFY(list.count() > 1);
    QListView* listView = fd.findChild<QListView*>("listView");
    QVERIFY(listView);
    for (int i = 0; i < list.count(); ++i) {
        fd.selectFile(fd.directory().path() + "/" + list.at(i));
        toSelect.append(listView->selectionModel()->selectedRows().last());
    }

    listView->selectionModel()->clear();

    // select the indexes
    for (int i = 0; i < toSelect.count(); ++i) {
        listView->selectionModel()->select(toSelect.at(i),
                QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
    QCOMPARE(fd.selectedFiles().count(), toSelect.count());
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
    QCOMPARE(fd.proxyModel(), (QAbstractProxyModel *)proxyModel);

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
    WAIT_FOR_CONDITION(treeView.at(0)->hasFocus(), true);
    QCOMPARE(treeView.at(0)->hasFocus(), true);
}

#include "../../../src/gui/dialogs/qfilesystemmodel_p.h"

void tst_QFiledialog::historyBack()
{
    QFileDialog fd;
    QToolButton *backButton = fd.findChild<QToolButton*>("backButton");
    QVERIFY(backButton);
    QToolButton *forwardButton = fd.findChild<QToolButton*>("forwardButton");
    QVERIFY(forwardButton);

    QFileSystemModel *model = fd.findChild<QFileSystemModel*>("qt_filesystem_model");
    QVERIFY(model);
    QSignalSpy spy(model, SIGNAL(rootPathChanged(const QString &)));

    QString home = fd.directory().absolutePath();
    QString desktop = QDir::homePath();
    QString temp = QDir::tempPath();

    QCOMPARE(backButton->isEnabled(), false);
    QCOMPARE(forwardButton->isEnabled(), false);
    fd.setDirectory(temp);
    qApp->processEvents();
    QCOMPARE(backButton->isEnabled(), true);
    QCOMPARE(forwardButton->isEnabled(), false);
    fd.setDirectory(desktop);
    QCOMPARE(spy.count(), 2);

    backButton->click();
    qApp->processEvents();
    QCOMPARE(backButton->isEnabled(), true);
    QCOMPARE(forwardButton->isEnabled(), true);
    QCOMPARE(spy.count(), 3);
    QString currentPath = qVariantValue<QString>(spy.last().first());
    QCOMPARE(currentPath, temp);

    backButton->click();
    currentPath = qVariantValue<QString>(spy.last().first());
    QCOMPARE(currentPath, home);
    QCOMPARE(backButton->isEnabled(), false);
    QCOMPARE(forwardButton->isEnabled(), true);
    QCOMPARE(spy.count(), 4);

    // nothing should change at this point
    backButton->click();
    QCOMPARE(spy.count(), 4);
    QCOMPARE(backButton->isEnabled(), false);
    QCOMPARE(forwardButton->isEnabled(), true);
}

void tst_QFiledialog::historyForward()
{
    QFileDialog fd;
    QToolButton *backButton = fd.findChild<QToolButton*>("backButton");
    QVERIFY(backButton);
    QToolButton *forwardButton = fd.findChild<QToolButton*>("forwardButton");
    QVERIFY(forwardButton);

    QFileSystemModel *model = fd.findChild<QFileSystemModel*>("qt_filesystem_model");
    QVERIFY(model);
    QSignalSpy spy(model, SIGNAL(rootPathChanged(const QString &)));

    QString home = fd.directory().absolutePath();
    QString desktop = QDir::homePath();
    QString temp = QDir::tempPath();

    fd.setDirectory(home);
    fd.setDirectory(temp);
    fd.setDirectory(desktop);

    backButton->click();
    QCOMPARE(forwardButton->isEnabled(), true);
    QCOMPARE(qVariantValue<QString>(spy.last().first()), temp);

    forwardButton->click();
    QCOMPARE(qVariantValue<QString>(spy.last().first()), desktop);
    QCOMPARE(backButton->isEnabled(), true);
    QCOMPARE(forwardButton->isEnabled(), false);
    QCOMPARE(spy.count(), 4);

    backButton->click();
    QCOMPARE(qVariantValue<QString>(spy.last().first()), temp);
    QCOMPARE(backButton->isEnabled(), true);

    backButton->click();
    QCOMPARE(qVariantValue<QString>(spy.last().first()), home);
    QCOMPARE(backButton->isEnabled(), false);
    QCOMPARE(forwardButton->isEnabled(), true);
    QCOMPARE(spy.count(), 6);

    forwardButton->click();
    QCOMPARE(qVariantValue<QString>(spy.last().first()), temp);
    backButton->click();
    QCOMPARE(qVariantValue<QString>(spy.last().first()), home);
    QCOMPARE(spy.count(), 8);

    forwardButton->click();
    QCOMPARE(qVariantValue<QString>(spy.last().first()), temp);
    forwardButton->click();
    QCOMPARE(qVariantValue<QString>(spy.last().first()), desktop);

    backButton->click();
    QCOMPARE(qVariantValue<QString>(spy.last().first()), temp);
    backButton->click();
    QCOMPARE(qVariantValue<QString>(spy.last().first()), home);
    fd.setDirectory(desktop);
    QCOMPARE(forwardButton->isEnabled(), false);
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

void tst_QFiledialog::clearLineEdit()
{
    QFileDialog fd(0, "caption", "foo");
    fd.setFileMode(QFileDialog::AnyFile);
    fd.show();

    QLineEdit *lineEdit = fd.findChild<QLineEdit*>("fileNameEdit");
    QVERIFY(lineEdit);
    QVERIFY(lineEdit->text() == "foo");
    fd.setDirectory(QDir::home());

    QTest::qWait(200);

    QListView* list = fd.findChild<QListView*>("listView");
    QVERIFY(list);
    QModelIndex root = list->rootIndex();
    QModelIndex subdir;
    for (int i = 0; i < list->model()->rowCount(root); ++i)
        if (list->model()->hasChildren(list->model()->index(i, 0, root)))
                subdir = list->model()->index(i, 0, root);
    QVERIFY(subdir.isValid());

    // saving a file the text shouldn't be cleared
    fd.setDirectory(QDir::home());
    QTest::mouseDClick(list->viewport(), Qt::LeftButton, 0, list->visualRect(subdir).center());
    QTest::mouseDClick(list->viewport(), Qt::LeftButton, 0, list->visualRect(subdir).center());
    QVERIFY(fd.directory().absolutePath() != QDir::home().absolutePath());
    QVERIFY(!lineEdit->text().isEmpty());

    // selecting a dir the text should be cleared so one can just hit ok
    // and it selects that directory
    fd.setFileMode(QFileDialog::Directory);
    fd.setDirectory(QDir::home());
    QTest::qWait(100);
    QTest::mouseDClick(list->viewport(), Qt::LeftButton, 0, list->visualRect(subdir).center());
    QTest::mouseDClick(list->viewport(), Qt::LeftButton, 0, list->visualRect(subdir).center());
    QVERIFY(fd.directory().absolutePath() != QDir::home().absolutePath());
    QVERIFY(lineEdit->text().isEmpty());
}

void tst_QFiledialog::enableChooseButton()
{
    QFileDialog fd;
    fd.setFileMode(QFileDialog::Directory);
    fd.show();
    QDialogButtonBox *buttonBox = fd.findChild<QDialogButtonBox*>("buttonBox");
    QPushButton *button = buttonBox->button(QDialogButtonBox::Open);
    QVERIFY(button);
    QCOMPARE(button->isEnabled(), true);
}

QTEST_MAIN(tst_QFiledialog)
#include "tst_qfiledialog.moc"
