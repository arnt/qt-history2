/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qfiledialog.h"
#include <qcombobox.h>
#include <qdirmodel.h>
#include <qframe.h>
#include <qgenericheader.h>
#include <qgenericlistview.h>
#include <qgenerictreeview.h>
#include <qitemselectionmodel.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmenu.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qsignal.h>
#include <qtoolbutton.h>
#ifdef Q_WS_MAC
#include <qmacstyle_mac.h>
#endif

#include <private/qdialog_p.h>

/* XPM */
static const char * const start_xpm[]={
    "16 15 8 1",
    "a c #cec6bd",
    "# c #000000",
    "e c #ffff00",
    "b c #999999",
    "f c #cccccc",
    "d c #dcdcdc",
    "c c #ffffff",
    ". c None",
    ".....######aaaaa",
    "...bb#cccc##aaaa",
    "..bcc#cccc#d#aaa",
    ".bcef#cccc#dd#aa",
    ".bcfe#cccc#####a",
    ".bcef#ccccccccc#",
    "bbbbbbbbbbbbccc#",
    "bccccccccccbbcc#",
    "bcefefefefee#bc#",
    ".bcefefefefef#c#",
    ".bcfefefefefe#c#",
    "..bcfefefefeeb##",
    "..bbbbbbbbbbbbb#",
    "...#############",
    "................"};

/* XPM */
static const char * const end_xpm[]={
    "16 15 9 1",
    "d c #a0a0a0",
    "c c #c3c3c3",
    "# c #cec6bd",
    ". c #000000",
    "f c #ffff00",
    "e c #999999",
    "g c #cccccc",
    "b c #ffffff",
    "a c None",
    "......####aaaaaa",
    ".bbbb..###aaaaaa",
    ".bbbb.c.##aaaaaa",
    ".bbbb....ddeeeea",
    ".bbbbbbb.bbbbbe.",
    ".bbbbbbb.bcfgfe.",
    "eeeeeeeeeeeeefe.",
    "ebbbbbbbbbbeege.",
    "ebfgfgfgfgff.ee.",
    "aebfgfgfgfgfg.e.",
    "aebgfgfgfgfgf.e.",
    "aaebgfgfgfgffe..",
    "aaeeeeeeeeeeeee.",
    "aaa.............",
    "aaaaaaaaaaaaaaaa"};

/* XPM */
static const char* const cdtoparent_xpm[]={
    "15 13 3 1",
    ". c None",
    "* c #000000",
    "a c #ffff99",
    "..*****........",
    ".*aaaaa*.......",
    "***************",
    "*aaaaaaaaaaaaa*",
    "*aaaa*aaaaaaaa*",
    "*aaa***aaaaaaa*",
    "*aa*****aaaaaa*",
    "*aaaa*aaaaaaaa*",
    "*aaaa*aaaaaaaa*",
    "*aaaa******aaa*",
    "*aaaaaaaaaaaaa*",
    "*aaaaaaaaaaaaa*",
    "***************"};

/* XPM */
static const char* const newfolder_xpm[] = {
    "15 14 4 1",
    "        c None",
    ".        c #000000",
    "+        c #FFFF00",
    "@        c #FFFFFF",
    "          .    ",
    "               ",
    "          .    ",
    "       .     . ",
    "  ....  . . .  ",
    " .+@+@.  . .   ",
    "..........  . .",
    ".@+@+@+@+@..   ",
    ".+@+@+@+@+. .  ",
    ".@+@+@+@+@.  . ",
    ".+@+@+@+@+.    ",
    ".@+@+@+@+@.    ",
    ".+@+@+@+@+.    ",
    "...........    "};

/* XPM */
static const char* const detailedview_xpm[]={
    "14 11 3 1",
    ". c None",
    "* c #000000",
    "a c #000099",
    ".****.***.***.",
    "..............",
    "aaaaaaaaaaaaaa",
    "..............",
    ".****.***.***.",
    "..............",
    ".****.***.***.",
    "..............",
    ".****.***.***.",
    "..............",
    ".****.***.***."};

/* XPM */
static const char* const previewinfoview_xpm[]={
    "13 13 4 1",
    ". c #00007f",
    "a c black",
    "# c #cec6bd",
    "b c #000000",
    "..#####aaaaaa",
    ".#.#bb#a#####",
    "...####a#bbb#",
    "#######a#####",
    "#######a#bb##",
    "..#####a#####",
    ".#.#bb#a#bbb#",
    "...####a#####",
    "#######a#bb##",
    "#######a#####",
    "..#####a#bbb#",
    ".#.#bb#a#####",
    "...####aaaaaa"};

/* XPM */
static const char* const previewcontentsview_xpm[]={
    "14 13 5 1",
    ". c #00007f",
    "a c black",
    "c c #7f007f",
    "# c #cec6bd",
    "b c #000000",
    "..#####aaaaaaa",
    ".#.#bb#a#####a",
    "...####a#ccc#a",
    "#######a#ccc#a",
    "#######a#####a",
    "..#####a#bbb#a",
    ".#.#bb#a#####a",
    "...####a#bbb#a",
    "#######a#####a",
    "#######a#bbb#a",
    "..#####a#####a",
    ".#.#bb#a#####a",
    "...####aaaaaaa"};

/* XPM */
static const char* const mclistview_xpm[]={
    "15 11 4 1",
    "* c None",
    "b c #000000",
    ". c #000099",
    "a c #ffffff",
    "...*****...****",
    ".a.*bbb*.a.*bbb",
    "...*****...****",
    "***************",
    "...*****...****",
    ".a.*bbb*.a.*bbb",
    "...*****...****",
    "***************",
    "...*****...****",
    ".a.*bbb*.a.*bbb",
    "...*****...****"};

/* XPM */
static const char * const back_xpm [] = {
    "13 11 3 1",
    "a c #00ffff",
    "# c #000000",
    ". c None",
    ".....#.......",
    "....##.......",
    "...#a#.......",
    "..#aa########",
    ".#aaaaaaaaaa#",
    "#aaaaaaaaaaa#",
    ".#aaaaaaaaaa#",
    "..#aa########",
    "...#a#.......",
    "....##.......",
    ".....#......."};

const char *qt_file_dialog_filter_reg_exp =
    "([a-zA-Z0-9]*)\\(([a-zA-Z0-9_.*? +;#\\[\\]]*)\\)$";

#include <qlistbox.h>

static int qt_combo_insert_unique(QComboBox *combo, const QString &text)
{
    if (text.isEmpty())
        return -1;
    QListBox *box = combo->listBox();
    QListBoxItem *itm = box->findItem(text, Qt::ExactMatch);
    if (itm && itm->text() == text) // FIXME: bug in findItem
        return box->index(itm);
    combo->insertItem(text);
    return combo->count() - 1;
}

class QFileDialogLineEdit : public QLineEdit
{
public:
    QFileDialogLineEdit(QWidget *parent)
        : QLineEdit(parent), key(0) {}
    inline int lastKeyPressed() { return key; }
protected:
    virtual void keyPressEvent(QKeyEvent *e)
    {
        key = e->key();
        QLineEdit::keyPressEvent(e);
    }
private:
    int key;
};

class QFileDialogPrivate : public QDialogPrivate
{
    Q_DECLARE_PUBLIC(QFileDialog);
public:
    QFileDialogPrivate()
        : QDialogPrivate(),
          viewMode(QFileDialog::Detail) {}

    void setup();

    void updateButtons(const QModelIndex &index);

    void setRoot(const QModelIndex &index);
    QModelIndex root() const;

    void setCurrent(const QModelIndex &index);
    QModelIndex current() const;

    void setDirSorting(int spec);
    void setDirFilter(int spec);
    void setSelectionMode(int mode);

    inline QString tr(const char *text) const { return QObject::tr(text); }

    QDirModel *model;
    QGenericListView *lview;
    QGenericTreeView *tview;
    QFileDialog::ViewMode viewMode;
    QFileDialog::FileMode fileMode;

    QModelIndexList history;

    QFrame *frame;
    QComboBox *lookIn;
    QFileDialogLineEdit *fileName;
    QFileDialogLineEdit *lookInEdit;
    QComboBox *fileType;

    QAction *openAction;
    QAction *renameAction;
    QAction *deleteAction;

    QAction *reloadAction;
    QAction *sortByNameAction;
    QAction *sortBySizeAction;
    QAction *sortByDateAction;
    QAction *unsortedAction;
    QAction *showHiddenAction;

    QToolButton *back;
    QToolButton *toParent;
    QToolButton *newFolder;
    QToolButton *detailMode;
    QToolButton *listMode;
};

#define d d_func()
#define q q_func()

QFileDialog::QFileDialog(QWidget *parent)
    : QDialog(*new QFileDialogPrivate, parent)
{
    d->setup();
    setDirectory(QDir::home());
}

QFileDialog::~QFileDialog()
{

}

void QFileDialog::setDirectory(const QDir &directory)
{
    QModelIndex index = d->model->index(directory.absolutePath());
    if (index.isValid()) {
        d->setRoot(index);
        d->updateButtons(index);
    }
}

QDir QFileDialog::directory() const
{
    QFileInfo info = d->model->fileInfo(d->root());
    return QDir(info.filePath(), d->model->nameFilters(), d->model->filter(), d->model->sorting());
}

void QFileDialog::selectFile(const QString &filename)
{
    QStringList entries = directory().entryList(d->model->filter(), d->model->sorting());
    QModelIndex index = d->model->index(entries.indexOf(filename), 0, d->root());
    d->lview->selectionModel()->select(index, QItemSelectionModel::Select);
}

QStringList QFileDialog::selectedFiles() const
{
    QModelIndexList items = d->lview->selectionModel()->selectedItems();
    QStringList files;
    int r = -1;
    for (int i = 0; i < items.count(); ++i) {
        if (items.at(i).row() != r) {
            files.append(d->model->fileInfo(items.at(i)).filePath());
            r = items.at(i).row();
        }
    }
    if (d->fileMode == AnyFile && files.count() <= 0) // a new filename
        files.append(d->fileName->text());
    return files;
}

void QFileDialog::setFilter(const QString &filter)
{
    d->fileType->clear();
    d->fileType->insertItem(filter);
    useFilter(filter);
}

void QFileDialog::setFilters(const QStringList &filters)
{
    d->fileType->clear();
    d->fileType->insertStringList(filters);
    useFilter(filters.first());
}

QStringList QFileDialog::filters() const
{
    QStringList items;
    for (int i = 0; i < d->fileType->count(); ++i)
        items.append(d->fileType->text(i));
    return items;
}

void QFileDialog::selectFilter(const QString &filter)
{
    d->fileType->setCurrentText(filter);
    setFilter(filter);
}

QString QFileDialog::selectedFilter() const
{
    return d->fileType->currentText();
}

void QFileDialog::setViewMode(ViewMode mode)
{
    d->viewMode = mode;
    if (mode == Detail)
        showDetail();
    else
        showList();
}

QFileDialog::ViewMode QFileDialog::viewMode() const
{
    return d->viewMode;
}

void QFileDialog::setFileMode(FileMode mode)
{
    d->fileMode = mode;
    QDir::FilterSpec spec;
    switch (mode) {
    case DirectoryOnly:
        d->fileType->clear();
        d->fileType->insertItem(QFileDialog::tr("Directories"));
        d->fileType->setEnabled(false);
        d->setSelectionMode(QAbstractItemView::SingleSelection);
        spec = QDir::Dirs;
        break;
    case Directory:
    case AnyFile:
    case ExistingFile:
        d->setSelectionMode(QAbstractItemView::SingleSelection);
        spec = QDir::All;
        break;
    case ExistingFiles:
    default:
        d->setSelectionMode(QAbstractItemView::ExtendedSelection);
        spec = QDir::All;
    }
    d->model->setFilter(spec);
}

QFileDialog::FileMode QFileDialog::fileMode() const
{
    return d->fileMode;
}

void QFileDialog::done(int result)
{
    QDialog::done(result);
}

void QFileDialog::accept()
{
    QStringList files = selectedFiles();
    if (files.count() < 1)
        return; // nothing was selected
    switch (d->fileMode) {
    case DirectoryOnly:
    case Directory:
        if (QFileInfo(files.first()).isDir())
            QDialog::accept();
        return;
    case AnyFile:// FIXME: not supported, as we rely on selected files
        if (!QFileInfo(files.first()).isDir())
            QDialog::accept();
        return;
    case ExistingFile:
    case ExistingFiles:
        for (int i = 0; i < files.count(); ++i) {
            QFileInfo info(files.at(i));
            if (!info.exists() || info.isDir())
                return;
        }
        QDialog::accept();
        return;
    }
}

void QFileDialog::reject()
{
    QDialog::reject();
}

void QFileDialog::back()
{
    QModelIndex root = d->history.back();
    d->history.pop_back();
    d->setRoot(root);
    d->updateButtons(root);
}

void QFileDialog::up()
{
    QModelIndex root = d->root();
    if (!root.isValid())
        return;
    d->history.push_back(root);
    QModelIndex parent = d->model->parent(root);
    d->setRoot(parent);
    d->updateButtons(parent);
}

void QFileDialog::mkdir()
{
    QModelIndex parent = d->root();
    QString path = d->model->path(parent);
    d->lview->clearSelections(); // FIXME: this is because the selection model doesn't use persistent indices yet

    QModelIndex index = d->model->mkdir(parent, "New Folder");
    if (!index.isValid())
        return;
    if (!index.isValid()) {
        d->setCurrent(d->model->topLeft(d->root()));
        return;
    }
    d->setCurrent(index);
    if (d->listMode->isDown())
        d->lview->edit(index);
    else
        d->tview->edit(index);
}

void QFileDialog::showList()
{
    d->listMode->setDown(true);
    d->detailMode->setDown(false);
    d->lview->show();
    d->lview->doItemsLayout();
    d->tview->hide();
}

void QFileDialog::showDetail()
{
    d->listMode->setDown(false);
    d->detailMode->setDown(true);
    d->tview->show();
    d->tview->doItemsLayout();
    d->lview->hide();
}

void QFileDialog::doubleClicked(const QModelIndex &index)
{
    if (index.type() == QModelIndex::View
        && d->model->isDir(index)) {
        d->history.push_back(d->root());
        d->setRoot(index);
        d->updateButtons(index);
    } else {
        accept();
    }
}

void QFileDialog::deletePressed(const QModelIndex &index)
{
    if (d->model->isDir(index))
        d->model->rmdir(index);
    else
        d->model->remove(index);
}

void QFileDialog::currentChanged(const QModelIndex &, const QModelIndex &current)
{
    if (!d->fileName->hasFocus() && current.isValid()) {
        QString text = d->model->data(current, QAbstractItemModel::Display).toString();
        d->fileName->setText(text);
    }
}

void QFileDialog::fileNameChanged(const QString &text)
{
    if (d->fileName->hasFocus() && !text.isEmpty()) {
        QModelIndex current = d->current();
        if (!current.isValid())
            current = d->model->topLeft(d->root());
        QModelIndexList indices = d->model->match(current, QAbstractItemModel::Display, text);
        int key = d->fileName->lastKeyPressed();
        if (indices.count() <= 0) { // no matches
            d->lview->selectionModel()->clear();
        } else if (key != Qt::Key_Delete && key != Qt::Key_Backspace) {
            d->setCurrent(indices.first());
            QString completed = d->model->data(indices.first(),
                                               QAbstractItemModel::Display).toString();
            int start = completed.length();
            int length = text.length() - start; // negative length
            bool block = d->fileName->blockSignals(true);
            d->fileName->setText(completed);
            d->fileName->setSelection(start, length);
            d->fileName->blockSignals(block);
        }
    }
}

void QFileDialog::lookInChanged(const QString &text)
{
    if (d->lookInEdit->hasFocus() && !text.isEmpty()) {
        QString pth = text.left(text.lastIndexOf('/'));
        QModelIndex dirIndex = d->model->index(pth);
        QString searchText = text.section('/', -1);
        int rowCount = d->model->rowCount(dirIndex);
        QModelIndexList indices = d->model->match(d->model->topLeft(dirIndex),
                                                  QAbstractItemModel::Display,
                                                  searchText, rowCount);
        int key = d->lookInEdit->lastKeyPressed();
        QModelIndex result;
        for (int i = 0; i < indices.count(); ++i) {
            if (d->model->isDir(indices.at(i))) {
                result = indices.at(i);
                break;
            }
        }
        if (result.isValid() && key != Qt::Key_Delete && key != Qt::Key_Backspace) {
            QString completed = d->model->path(result);
            int start = completed.length();
            int length = text.length() - start; // negative length
            bool block = d->lookInEdit->blockSignals(true);
            d->lookInEdit->setText(completed);
            d->lookInEdit->setSelection(start, length);
            d->lookInEdit->blockSignals(block);
        }
    }
}

void QFileDialog::useFilter(const QString &filter)
{
    QRegExp regexp(QString::fromLatin1(qt_file_dialog_filter_reg_exp));
    QString f = filter;
    int i = regexp.indexIn(f);
    if (i >= 0)
        f = regexp.cap(2);
    QStringList filters = f.split(' ', QString::SkipEmptyParts);
    d->model->setNameFilters(filters);
}

void QFileDialog::setCurrentDir(const QString &path)
{
    QModelIndex index = d->model->index(path);
    d->setRoot(index);
    d->updateButtons(index);
}

void QFileDialog::populateContextMenu(QMenu *menu, const QModelIndex &index) const
{
    if (index.isValid()) {
        // file context menu
        menu->addAction(d->openAction);
        menu->addSeparator();
        menu->addAction(d->renameAction);
        menu->addAction(d->deleteAction);
    } else {
        // view context menu
        menu->addAction(d->reloadAction);
        QMenu *sort = new QMenu();
        menu->addMenu("Sort", sort);
        sort->addAction(d->sortByNameAction);
        sort->addAction(d->sortBySizeAction);
        sort->addAction(d->sortByDateAction);
        sort->addSeparator();
        sort->addAction(d->unsortedAction);
        menu->addSeparator();
        menu->addAction(d->showHiddenAction);
    }
}

void QFileDialog::headerClicked(int section)
{
    int spec = -1;
    switch (section) {
    case 0:
        spec = QDir::Name;
        break;
    case 1:
        spec = QDir::Size;
        break;
    case 2:
        qDebug("sorting on type is not implemented yet");
        return;
    case 3:
        spec = QDir::Time;
        break;
    default:
        return;
    }
    QGenericHeader *header = d->tview->header();
    Qt::SortOrder order = (header->sortIndicatorSection() == section
                       && header->sortIndicatorOrder() == Qt::DescendingOrder)
                      ? Qt::AscendingOrder : Qt::DescendingOrder;
    bool sortByName = (spec & QDir::SortByMask) == QDir::Name;
    bool reverse = (order == Qt::AscendingOrder ? sortByName : !sortByName);
    if (reverse) {
        spec |= QDir::Reversed;
        spec |= QDir::DirsLast;
        spec &= ~QDir::DirsFirst;
    } else {
        spec &= ~QDir::Reversed;
        spec &= ~QDir::DirsLast;
        spec |= QDir::DirsFirst;
    }

    d->setDirSorting(spec);
    header->setSortIndicator(section, order);
}

void QFileDialog::renameCurrent()
{
    QAbstractItemView *view = d->listMode->isDown()
                              ? static_cast<QAbstractItemView*>(d->lview)
                              : static_cast<QAbstractItemView*>(d->tview);
    view->edit(d->current());
}

void QFileDialog::deleteCurrent()
{
    deletePressed(d->current());
}

void QFileDialog::reload()
{
    d->model->refresh(d->root());
}

void QFileDialog::sortByName()
{
    d->setDirSorting(QDir::Name|QDir::DirsFirst|(d->model->filter() & QDir::Reversed));
}

void QFileDialog::sortBySize()
{
    d->setDirSorting(QDir::Size|QDir::DirsFirst|(d->model->filter() & QDir::Reversed));
}

void QFileDialog::sortByDate()
{
    d->setDirSorting(QDir::Time|QDir::DirsFirst|(d->model->filter() & QDir::Reversed));
}

void QFileDialog::setUnsorted()
{
    d->setDirSorting(QDir::Unsorted|QDir::DirsFirst|(d->model->filter() & QDir::Reversed));
}

void QFileDialog::showHidden()
{
    int filter = (d->showHiddenAction->isChecked()
                  ? d->model->filter() & ~QDir::Hidden
                  : d->model->filter() | QDir::Hidden);
    d->setDirFilter(filter);
}

void QFileDialogPrivate::setup()
{
    q->setSizeGripEnabled(true);
    QGridLayout *grid = new QGridLayout(q, 4, 6, 11, 6);

    // model
    QDir dir("/");
    dir.setFilter(dir.filter() | QDir::AllDirs);
    dir.setSorting(QDir::DirsFirst);
    model = new QDirModel(dir, q);

    // views
    lview = new QGenericListView(model, q);
    lview->viewport()->setAcceptDrops(true);
    lview->setSpacing(2);
    lview->setWrapping(true);
    lview->setResizeMode(QGenericListView::Adjust);
    lview->setStartEditActions(QAbstractItemDelegate::EditKeyPressed);
    grid->addWidget(lview, 1, 0, 1, 6);

    QItemSelectionModel *selections = lview->selectionModel();

    tview = new QGenericTreeView(model, q);
    tview->viewport()->setAcceptDrops(true);
    tview->setSelectionModel(selections);
    tview->setShowRootDecoration(false);
    tview->header()->setResizeMode(QGenericHeader::Custom);
    tview->header()->setResizeMode(QGenericHeader::Stretch, 0);
    tview->header()->setSortIndicator(0, Qt::DescendingOrder);
    tview->hide();
    tview->setStartEditActions(QAbstractItemDelegate::EditKeyPressed);
    grid->addWidget(tview, 1, 0, 1, 6);

    // actions
    openAction = new QAction(tr("&Open"), q);
    renameAction = new QAction(tr("&Rename"), q);
    deleteAction = new QAction(tr("&Delete"), q);
    reloadAction = new QAction(tr("&Reload"), q);
    sortByNameAction = new QAction(tr("Sort by &Name"), q);
    sortByNameAction->setCheckable(true);
    sortByNameAction->setChecked(true);
    sortBySizeAction = new QAction(tr("Sort by &Size"), q);
    sortBySizeAction->setCheckable(true);
    sortByDateAction = new QAction(tr("Sort by &Date"), q);
    sortByDateAction->setCheckable(true);
    unsortedAction = new QAction(tr("&Unsorted"), q);
    unsortedAction->setCheckable(true);
    showHiddenAction = new QAction(tr("Show &hidden files"), q);
    showHiddenAction->setCheckable(true);


    QObject::connect(lview, SIGNAL(aboutToShowContextMenu(QMenu*, const QModelIndex&)),
                     q, SLOT(populateContextMenu(QMenu*, const QModelIndex&)));
    QObject::connect(tview, SIGNAL(aboutToShowContextMenu(QMenu*, const QModelIndex&)),
                     q, SLOT(populateContextMenu(QMenu*, const QModelIndex&)));
    QObject::connect(openAction, SIGNAL(triggered()), q, SLOT(accept()));
    QObject::connect(renameAction, SIGNAL(triggered()), q, SLOT(renameCurrent()));
    QObject::connect(deleteAction, SIGNAL(triggered()), q, SLOT(deleteCurrent()));
    QObject::connect(reloadAction, SIGNAL(triggered()), q, SLOT(reload()));
    QObject::connect(sortByNameAction, SIGNAL(triggered()), q, SLOT(sortByName()));
    QObject::connect(sortBySizeAction, SIGNAL(triggered()), q, SLOT(sortBySize()));
    QObject::connect(sortByDateAction, SIGNAL(triggered()), q, SLOT(sortByDate()));
    QObject::connect(unsortedAction, SIGNAL(triggered()), q, SLOT(setUnsorted()));
    QObject::connect(showHiddenAction, SIGNAL(triggered()), q, SLOT(showHidden()));

    // connect signals
    QObject::connect(lview, SIGNAL(doubleClicked(const QModelIndex&, int)),
                     q, SLOT(doubleClicked(const QModelIndex&)));
    QObject::connect(tview, SIGNAL(doubleClicked(const QModelIndex&, int)),
                     q, SLOT(doubleClicked(const QModelIndex&)));
    QObject::connect(selections, SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
                     q, SLOT(currentChanged(const QModelIndex &, const QModelIndex &)));
    QObject::connect(lview, SIGNAL(returnPressed(const QModelIndex&)),
                     q, SLOT(doubleClicked(const QModelIndex&)));
    QObject::connect(tview, SIGNAL(returnPressed(const QModelIndex&)),
                     q, SLOT(doubleClicked(const QModelIndex&)));
    QObject::connect(lview, SIGNAL(deletePressed(const QModelIndex&)),
                     q, SLOT(deletePressed(const QModelIndex&)));
    QObject::connect(tview, SIGNAL(deletePressed(const QModelIndex&)),
                     q, SLOT(deletePressed(const QModelIndex&)));
    QObject::connect(tview->header(), SIGNAL(sectionClicked(int, Qt::ButtonState)),
                     q, SLOT(headerClicked(int)));

    grid->addWidget(new QLabel(tr("Look in:"), q), 0, 0);
    grid->addWidget(new QLabel(tr("File name:"), q), 2, 0);
    grid->addWidget(new QLabel(tr("Files of type:"), q), 3, 0);

    // push buttons
    QPushButton *accept = new QPushButton(tr("Open"), q);
    QObject::connect(accept, SIGNAL(clicked()), q, SLOT(accept()));
    grid->addWidget(accept, 2, 5, Qt::AlignLeft);

    QPushButton *reject = new QPushButton(tr("Cancel"), q);
    QObject::connect(reject, SIGNAL(clicked()), q, SLOT(reject()));
    grid->addWidget(reject, 3, 5, Qt::AlignLeft);

    // conboboxes && lineedit
    lookIn = new QComboBox(q);
    lookIn->setInsertionPolicy(QComboBox::NoInsertion);
    lookIn->setDuplicatesEnabled(false);
    lookIn->setEditable(true);
    lookIn->insertItem(QDir::root().absolutePath());
    QObject::connect(lookIn, SIGNAL(activated(const QString&)),
                     q, SLOT(setCurrentDir(const QString&)));
    grid->addWidget(d->lookIn, 0, 1, 1, 3);
    lookInEdit = new QFileDialogLineEdit(lookIn);
    lookIn->setLineEdit(lookInEdit);
    QObject::connect(lookInEdit, SIGNAL(textChanged(const QString&)),
                     q, SLOT(lookInChanged(const QString&)));
    fileName = new QFileDialogLineEdit(q);
    QObject::connect(fileName, SIGNAL(textChanged(const QString&)),
                     q, SLOT(fileNameChanged(const QString&)));
    grid->addWidget(fileName, 2, 2, 1, 3);
    fileType = new QComboBox(q);
    fileType->setDuplicatesEnabled(false);
    fileType->insertStringList(QFileDialog::tr("All Files (*)"));
    QObject::connect(fileType, SIGNAL(activated(const QString&)),
                     q, SLOT(useFilter(const QString&)));
    grid->addWidget(fileType, 3, 2, 1, 3);

    // tool buttons
    QHBoxLayout *box = new QHBoxLayout(0, 3, 3);
    QSize tools(22, 22);

    back = new QToolButton(q);
    back->setIcon(QPixmap(back_xpm));
    back->setToolTip(tr("Back"));
    back->setAutoRaise(true);
    back->setEnabled(false);
    back->setFixedSize(tools);
    QObject::connect(back, SIGNAL(clicked()), q, SLOT(back()));
    box->addWidget(back);

    toParent = new QToolButton(q);
    toParent->setIcon(QPixmap(cdtoparent_xpm));
    toParent->setToolTip(tr("Parent Directory"));
    toParent->setAutoRaise(true);
    toParent->setEnabled(model->parent(root()).isValid());
    toParent->setFixedSize(tools);
    QObject::connect(toParent, SIGNAL(clicked()), q, SLOT(up()));
    box->addWidget(toParent);

    newFolder = new QToolButton(q);
    newFolder->setIcon(QPixmap(newfolder_xpm));
    newFolder->setToolTip(tr("Create New Folder"));
    newFolder->setAutoRaise(true);
    newFolder->setFixedSize(tools);
    QObject::connect(newFolder, SIGNAL(clicked()), q, SLOT(mkdir()));
    box->addWidget(newFolder);

    listMode = new QToolButton(q);
    listMode->setIcon(QPixmap(mclistview_xpm));
    listMode->setToolTip(tr("List View"));
    listMode->setAutoRaise(true);
    listMode->setDown(true);
    listMode->setFixedSize(tools);
    QObject::connect(listMode, SIGNAL(clicked()), q, SLOT(showList()));
    box->addWidget(listMode);

    detailMode = new QToolButton(q);
    detailMode->setIcon(QPixmap(detailedview_xpm));
    detailMode->setToolTip(tr("Detail View"));
    detailMode->setAutoRaise(true);
    detailMode->setFixedSize(tools);
    QObject::connect(detailMode, SIGNAL(clicked()), q, SLOT(showDetail()));
    box->addWidget(detailMode);

    grid->addLayout(box, 0, 4);

    q->resize(550, 320);
}

void QFileDialogPrivate::updateButtons(const QModelIndex &index)
{
    toParent->setEnabled(index.isValid());
    back->setEnabled(history.count() > 0);
    QString pth = d->model->path(index);
    int idx = qt_combo_insert_unique(lookIn, pth);
    lookIn->setCurrentItem(idx);
}

void QFileDialogPrivate::setRoot(const QModelIndex &index)
{
    QItemSelectionModel *selections = lview->selectionModel();
    bool block = selections->blockSignals(true);
    selections->clear();
    setCurrent(d->model->topLeft(index));
    lview->setRoot(index);
    tview->setRoot(index);
    selections->blockSignals(block);
}

QModelIndex QFileDialogPrivate::root() const
{
    return lview->root();
}

void QFileDialogPrivate::setCurrent(const QModelIndex &index)
{
    lview->setCurrentItem(index);
}

QModelIndex QFileDialogPrivate::current() const
{
    return lview->currentItem();
}

void QFileDialogPrivate::setDirSorting(int spec)
{
    int sortBy = spec & QDir::SortByMask;
    sortByNameAction->setChecked(sortBy == QDir::Name);
    sortBySizeAction->setChecked(sortBy == QDir::Size);
    sortByDateAction->setChecked(sortBy == QDir::Time);
    unsortedAction->setChecked(sortBy == QDir::Unsorted);
    model->setSorting(spec);
}

void QFileDialogPrivate::setDirFilter(int spec)
{
    showHiddenAction->setChecked(spec & QDir::Hidden);
    model->setFilter(spec);
}

void QFileDialogPrivate::setSelectionMode(int mode)
{
    lview->setSelectionMode(mode);
    tview->setSelectionMode(mode);
}

/******************************************************************
 *
 * Static functions for the native filedialogs
 *
 ******************************************************************/

#include <qapplication.h>
#include <qstyle.h>

extern Q_CORE_EXPORT bool qt_resolve_symlinks; // defined in qapplication.cpp
static QString qt_working_dir; // ### ugly! Used this to save cwd

#if defined(Q_WS_WIN)
extern QString qt_win_get_open_file_name(const QString &initialSelection,
                                         const QString &filter,
                                         QString *initialDirectory,
                                         QWidget *parent,
                                         const QString &caption,
                                         QString *selectedFilter);

extern QString qt_win_get_save_file_name(const QString &initialSelection,
                                         const QString &filter,
                                         QString *initialDirectory,
                                         QWidget *parent,
                                         const QString &caption,
                                         QString *selectedFilter);

extern QStringList qt_win_get_open_file_names(const QString &filter,
                                              QString *initialDirectory,
                                              QWidget *parent,
                                              const QString &caption,
                                              QString *selectedFilter);

extern QString qt_win_get_existing_directory(const QString &initialDirectory,
                                             QWidget *parent,
                                             const QString& caption);
#elif defined(Q_WS_MAC)
extern QStringList qt_mac_get_open_file_names(const QString &filter,
                                              QString *pwd,
                                              QWidget *parent,
                                              const QString &caption,
                                              QString *selectedFilter,
                                              bool multi, bool directory);

extern QString qt_mac_get_save_file_name(const QString &start,
                                         const QString &filter,
                                         QString *pwd,
                                         QWidget *parent,
                                         const QString &caption,
                                         QString *selectedFilter);
#endif

static QString qt_encode_file_name(const QString &filename)
{
    QString str;
    QByteArray name = filename.utf8();
    static const QByteArray illegal("<>#@\"&%$:,;?={}|^~[]\'`\\*");

    int len = name.length();
    if (!len)
        return QString::null;
    for (int i = 0; i < len ;++i) {
        uchar byte = static_cast<uchar>(name.at(i));
        if (byte >= 128 || illegal.contains(byte)) {
            str += QChar('%');
            ushort c = byte / 16;
            c += c > 9 ? 'A' - 10 : '0';
            str += static_cast<char>(c);
            c = byte % 16;
            c += c > 9 ? 'A' - 10 : '0';
            str += static_cast<char>(c);
        } else {
            str += static_cast<char>(byte);
        }
    }
    return str;
}

// Makes a list of filters from ;;-separated text.
QStringList qt_make_filter_list(const QString &filter)
{
    QString f(filter);

    if (f.isEmpty())
        return QStringList();

    QString sep(";;");
    int i = f.indexOf(sep, 0);
    if (i == -1) {
        if (f.indexOf("\n", 0) != -1) {
            sep = "\n";
            i = f.indexOf(sep, 0);
        }
    }

    return f.split(sep);
}

static QFileDialog *qt_create_file_dialog(QWidget *parent,
                                          const QString &caption,
                                          const QString &directory,
                                          const QString &initialSelection,
                                          const QString &filter,
                                          QString *selectedFilter)
{
    QFileDialog *dlg = new QFileDialog(parent);
    QDir dir(directory);
    dlg->setDirectory(dir);
    dlg->setModal(true);
    dlg->setWindowTitle(caption);
    if (filter != QString())
        dlg->setFilters(qt_make_filter_list(filter));
    if (selectedFilter)
        dlg->selectFilter(*selectedFilter);
    if (!initialSelection.isEmpty())
        dlg->selectFile(initialSelection);
    return dlg;
}

static void qt_get_dir_and_selection(const QString &path, QString *cwd, QString *sel)
{
    if (!path.isEmpty()) {
        QFileInfo info(qt_encode_file_name(path));
         if (info.exists()) {
             if (info.isDir()) {
                 if (cwd) *cwd = path;
                 if (sel) *sel = QString::null;
             } else {
                 if (cwd) *cwd = info.absolutePath();
                 if (sel) *sel = info.fileName();
             }
             return;
         }
    }
    if (cwd) *cwd = QDir::currentPath();
    if (sel) *sel = QString::null;
}

/*!
  This is a convenience static function that returns an existing file
  selected by the user. If the user pressed Cancel, it returns a null
  string.

  \code
    QString s = QFileDialog::getOpenFileName(
                    "/home",
                    "Images (*.png *.xpm *.jpg)",
                    this,
                    "Choose a file to open");
  \endcode

  The function creates a modal file dialog with parent, \a parent. If
  a parent is not 0, the dialog will be shown centered over the
  parent.

  The file dialog's working directory will be set to \a startWith. If \a
  startWith includes a file name, the file will be selected. The filter
  is set to \a filter so that only those files which match the filter
  are shown. The filter selected is set to \a selectedFilter. The parameters
  \a startWith, \a selectedFilter and \a filter may be QString::null.

  The dialog's caption is set to \a caption. If \a caption is not
  specified then a default caption will be used.

  Under Windows and Mac OS X, this static function will use the native
  file dialog and not a QFileDialog, unless the style of the application
  is set to something other than the native style (Note that on Windows the
  dialog will spin a blocking modal event loop that will not dispatch any
  QTimers and if parent is not 0 then it will position the dialog just under
  the parent's title bar).

  Under Unix/X11, the normal behavior of the file dialog is to resolve
  and follow symlinks. For example, if /usr/tmp is a symlink to /var/tmp,
  the file dialog will change to /var/tmp after entering /usr/tmp.
  If \a resolveSymlinks is false, the file dialog will treat
  symlinks as regular directories.

  \sa getOpenFileNames(), getSaveFileName(), getExistingDirectory()
*/

QString QFileDialog::getOpenFileName(QWidget *parent,
                                     const QString &caption,
                                     const QString &dir,
                                     const QString &filter,
                                     QString *selectedFilter,
                                     QFileDialog::Options options)
{
    bool save_qt_resolve_symlinks = qt_resolve_symlinks;
    qt_resolve_symlinks = !(options & DontResolveSymlinks);

    QString initialSelection;
    qt_get_dir_and_selection(dir, &qt_working_dir, &initialSelection);

    // create a native dialog

#if defined(Q_WS_WIN)
    if (qApp->style().styleHint(QStyle::SH_GUIStyle) == Qt::WindowsStyle)
        return qt_win_get_open_file_name(initialSelection, filter, &qt_working_dir,
                                         parent, caption, selectedFilter);
#elif defined(Q_WS_MAC)
    if (::qt_cast<QMacStyle*>(&qApp->style())) {
        QStringList files = qt_mac_get_open_file_names(filter, &qt_working_dir, parent,
                                                       caption, selectedFilter, false, false);
        return files.isEmpty() ? QString::null : files.first();
    }
#endif

    // create a qt dialog
    QFileDialog *dlg = qt_create_file_dialog(parent, caption == QString() ? "Open" : caption,
                                             qt_working_dir, initialSelection,
                                             filter, selectedFilter);
    dlg->setFileMode(QFileDialog::ExistingFile);

    QString result;
    if (dlg->exec() == QDialog::Accepted) {
        QStringList selected = dlg->selectedFiles();
        if (selected.count() > 0)
            result = selected.first();
//         if (selectedFilter)
//             *selectedFilter = dlg->selectedFilter();
        qt_working_dir = dlg->directory().absolutePath();
    }
    delete dlg;

    qt_resolve_symlinks = save_qt_resolve_symlinks;

    return result;
}

/*!
  This is a convenience static function that will return a file name
  selected by the user. The file does not have to exist.

  It creates a modal file dialog with parent, \a parent. If a parent
  is not 0, the dialog will be shown centered over the parent.

  \code
    QString s = QFileDialog::getSaveFileName(
                    "/home",
                    "Images (*.png *.xpm *.jpg)",
                    this,
                    "Choose a filename to save under");
  \endcode

  The file dialog's working directory will be set to \a startWith. If \a
  startWith includes a file name, the file will be selected. The filter
  is set to \a filter so that only those files which match the filter
  are shown. The filter selected is set to \a selectedFilter. The parameters
  \a startWith, \a selectedFilter and \a filter may be QString::null.

  The dialog's caption is set to \a caption. If \a caption is not
  specified then a default caption will be used.

  Under Windows and Mac OS X, this static function will use the native
  file dialog and not a QFileDialog, unless the style of the application
  is set to something other than the native style. (Note that on Windows the
  dialog will spin a blocking modal event loop that will not dispatch any
  QTimers and if parent is not 0 then it will position the dialog just under
  the parent's title bar.  And on the Mac the filter argument is ignored).

  Under Unix/X11, the normal behavior of the file dialog is to resolve
  and follow symlinks. For example, if /usr/tmp is a symlink to /var/tmp,
  the file dialog will change to /var/tmp after entering /usr/tmp.
  If \a resolveSymlinks is false, the file dialog will treat
  symlinks as regular directories.

  \sa getOpenFileName(), getOpenFileNames(), getExistingDirectory()
*/

QString QFileDialog::getSaveFileName(QWidget *parent,
                                     const QString &caption,
                                     const QString &dir,
                                     const QString &filter,
                                     QString *selectedFilter,
                                     Options options)
{
    bool save_qt_resolve_symlinks = qt_resolve_symlinks;
    qt_resolve_symlinks = !(options & DontResolveSymlinks);

    QString initialSelection;
    qt_get_dir_and_selection(dir, &qt_working_dir, &initialSelection);

#if defined(Q_WS_WIN)
    if (qApp->style().styleHint(QStyle::SH_GUIStyle) == Qt::WindowsStyle)
        return qt_win_get_save_file_name(initialSelection, filter, &qt_working_dir,
					 parent, caption, selectedFilter);
#elif defined(Q_WS_MAC)
    if (::qt_cast<QMacStyle*>(&qApp->style()))
        return qt_mac_get_save_file_name(initialSelection, filter, &qt_working_dir,
                                         parent, caption, selectedFilter);
#endif

    QFileDialog *dlg = qt_create_file_dialog(parent, caption == QString() ? "Save As" : caption,
                                             qt_working_dir, initialSelection,
                                             filter, selectedFilter);
    dlg->setFileMode(QFileDialog::AnyFile);

    QString result;
    if (dlg->exec() == QDialog::Accepted) {
        QStringList selected = dlg->selectedFiles();
        if (selected.count() > 0)
            result = selected.first();
//         if (selectedFilter)
//             *selectedFilter = dlg->selectedFilter();
        qt_working_dir = dlg->directory().absolutePath();
    }
    delete dlg;

    qt_resolve_symlinks = save_qt_resolve_symlinks;

    return result;
}

/*!
  This is a convenience static function that will return an existing directory
  selected by the user.

  \code
    QString s = QFileDialog::getExistingDirectory(
                    "/home",
                    this,
                    "Choose a directory",
                    true);
  \endcode

  This function creates a modal file dialog with parent, \a parent. If
  parent is not 0, the dialog will be shown centered over the parent.

  The dialog's working directory is set to \a dir, and the caption is
  set to \a caption. Either of these may be QString::null in which case
  the current directory and a default caption will be used respectively.

  If \a dirOnly is true, then only directories will be shown in
  the file dialog; otherwise both directories and files will be shown.

  Under Unix/X11, the normal behavior of the file dialog is to resolve
  and follow symlinks. For example, if /usr/tmp is a symlink to /var/tmp,
  the file dialog will change to /var/tmp after entering /usr/tmp.
  If \a resolveSymlinks is false, the file dialog will treat
  symlinks as regular directories.

  Note that on Windows the dialog will spin a blocking modal event loop
  that will not dispatch any QTimers and if parent is not 0 then it will
  position the dialog just under the parent's title bar.

  \sa getOpenFileName(), getOpenFileNames(), getSaveFileName()
*/

QString QFileDialog::getExistingDirectory(QWidget *parent,
                                          const QString &dir,
                                          const QString &caption,
                                          Options options)
{
    bool save_qt_resolve_symlinks = qt_resolve_symlinks;
    qt_resolve_symlinks = !(options & DontResolveSymlinks);

#if defined(Q_WS_WIN)
    QString initialDir;
    if (!dir.isEmpty() && QFileInfo(dir).isDir())
        initialDir = dir;
    if (qApp->style().styleHint(QStyle::SH_GUIStyle) == Qt::WindowsStyle && (options & ShowDirsOnly))
        return qt_win_get_existing_directory(initialDir, parent, caption);
#elif defined(Q_WS_MAC)
    if (::qt_cast<QMacStyle*>(&qApp->style()))
        return qt_mac_get_open_file_names("", 0, parent, caption, NULL, false, true).first();
#endif

    qt_get_dir_and_selection(dir, &qt_working_dir, 0);

    QFileDialog *dlg = qt_create_file_dialog(parent,
                                             caption == QString() ? "Find Directory" : caption,
                                             qt_working_dir, QString(), QString(), 0);
    dlg->setFileMode(options & ShowDirsOnly ? DirectoryOnly : Directory);
    dlg->d->fileType->clear();
    dlg->d->fileType->insertItem(QFileDialog::tr("Directories"));
    dlg->d->fileType->setEnabled(false);

    QString result;
    if (dlg->exec() == QDialog::Accepted) {
        result = dlg->selectedFiles().first();
        qt_working_dir = result;
    }
    delete dlg;

    if (!result.isEmpty() && result.right(1) != "/")
        result += "/";

    qt_resolve_symlinks = save_qt_resolve_symlinks;

    return result;
}

/*!
  This is a convenience static function that will return one or more
  existing files selected by the user.

  \code
    QStringList files = QFileDialog::getOpenFileNames(
                            "Images (*.png *.xpm *.jpg)",
                            "/home",
                            this,
                            "Select one or more files to open");
  \endcode

  This function creates a modal file dialog with parent \a parent. If
  \a parent is not 0, the dialog will be shown centered over the
  parent.

  The file dialog's working directory will be set to \a dir. If \a
  dir includes a file name, the file will be selected. The filter
  is set to \a filter so that only those files which match the filter
  are shown. The filter selected is set to \a selectedFilter. The parameters
  \a dir, \a selectedFilter and \a filter may be QString::null.

  The dialog's caption is set to \a caption. If \a caption is not
  specified then a default caption will be used.

  Under Windows and Mac OS X, this static function will use the native
  file dialog and not a QFileDialog, unless the style of the application
  is set to something other than the native style. (Note that on Windows the
  dialog will spin a blocking modal event loop that will not dispatch any
  QTimers and if parent is not 0 then it will position the dialog just under
  the parent's title bar).

  Under Unix/X11, the normal behavior of the file dialog is to resolve
  and follow symlinks. For example, if /usr/tmp is a symlink to /var/tmp,
  the file dialog will change to /var/tmp after entering /usr/tmp.
  If \a resolveSymlinks is false, the file dialog will treat
  symlinks as regular directories.

  Note that if you want to iterate over the list of files, you should
  iterate over a copy, e.g.
    \code
    QStringList list = files;
    QStringList::Iterator it = list.begin();
    while(it != list.end()) {
        myProcessing(*it);
        ++it;
    }
    \endcode

  \sa getOpenFileName(), getSaveFileName(), getExistingDirectory()
*/

QStringList QFileDialog::getOpenFileNames(QWidget *parent,
                                          const QString &caption,
                                          const QString &dir,
                                          const QString &filter,
                                          QString *selectedFilter,
                                          QFileDialog::Options options)
{
    bool save_qt_resolve_symlinks = qt_resolve_symlinks;
    qt_resolve_symlinks = !(options & DontResolveSymlinks);
    qt_get_dir_and_selection(dir, &qt_working_dir, 0);

#if defined(Q_WS_WIN)
    if (qApp->style().styleHint(QStyle::SH_GUIStyle) == Qt::WindowsStyle)
        return qt_win_get_open_file_names(filter, &qt_working_dir, parent, caption, selectedFilter);
#elif defined(Q_WS_MAC)
    if (::qt_cast<QMacStyle*>(&qApp->style()))
        return qt_mac_get_open_file_names(filter, &qt_working_dir, parent, caption, selectedFilter, true, false);
#endif

    QFileDialog *dlg = qt_create_file_dialog(parent, caption.isNull() ? "Open" : caption,
                                             qt_working_dir, QString(),
                                             filter, selectedFilter);
    dlg->setFileMode(QFileDialog::ExistingFiles);

    QStringList lst;
    if (dlg->exec() == QDialog::Accepted) {
//        lst = dlg->selectedFiles();
//         if (selectedFilter)
//             *selectedFilter = dlg->selectedFilter();
        qt_working_dir = dlg->directory().absolutePath();
    }
    delete dlg;

    qt_resolve_symlinks = save_qt_resolve_symlinks;

    return lst;
}
