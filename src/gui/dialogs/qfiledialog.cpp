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

/*!
  \class QFileDialog
  \brief The QFileDialog class provides a dialog that allow users to select files or directories.
  \ingroup dialogs
  \mainclass

  The QFileDialog class enables a user to traverse the file system in
  order to select one or many files or a directory.

  The easiest way to create a QFileDialog is to use the static
  functions. On Windows, these static functions will call the native
  Windows file dialog, and on Mac OS X these static function will call
  the native Mac OS X file dialog.

  \code
    QString s = QFileDialog::getOpenFileName(
            "/home",
            "Images (*.png *.xpm *.jpg)",
            this,
            "open file dialog",
            "Choose a file");
  \endcode

  In the above example, a modal QFileDialog is created using a static
  function. The dialog initially displays the contents of the "/home"
  directory, and displays files matching the patterns given in the
  string "Images (*.png *.xpm *.jpg)". The parent of the file dialog
  is set to \e this, and the dialog is named "open file dialog".
  The caption at the top of file dialog is set to "Choose a file"
  instead of the default.

  If you want to use multiple filters, separate each one with
  \e two semicolons. For example:
  \code
  "Images (*.png *.xpm *.jpg);;Text files (*.txt);;XML files (*.xml)"
  \endcode

  You can create your own QFileDialog without using the static
  functions. By calling setMode(), you can specify what the user must
  select in the dialog:

  \code
    QFileDialog *fd = new QFileDialog(this);
    fd->setMode(QFileDialog::AnyFile);
  \endcode

  In the above example, the mode of the file dialog is set to \c
  AnyFile, meaning that the user can select any file, or even specify a
  file that doesn't exist. This mode is useful for creating a
  "Save As" file dialog. Use \c ExistingFile if the user must select an
  existing file, or \c Directory if only a directory may be selected.
  See the \l QFileDialog::FileMode enum for the complete list of modes.

  You can retrieve the dialog's mode with mode(). Use setFilter() to set
  the dialog's file filter. For example:

  \code
    fd->setFilter( "Images (*.png *.xpm *.jpg)" );
  \endcode

  In the above example, the filter is set to "Images (*.png *.xpm
  *.jpg)", this means that only files with the extension \c png, \c xpm,
  or \c jpg will be shown in the QFileDialog. You can apply
  several filters by using setFilters(), and add additional filters with
  addFilter(). Use setSelectedFilter() to select one of the filters
  you've given as the file dialog's default filter. Whenever the user
  changes the filter the filterSelected() signal is emitted.

  The file dialog has two view modes: QFileDialog::List and
  QFileDialog::Detail.
  QFileDialog::List presents the contents of the current directory as a
  list of file and directory names. QFileDialog::Detail also displays a
  list of file and directory names, but provides additional information
  alongside each name, such as the file size and modification date.
  Set the mode with setViewMode().

  \code
    fd->setViewMode(QFileDialog::Detail);
  \endcode

  The last important function you will need to use when creating your
  own file dialog is selectedFile().

  \code
    QString fileName;
    if (fd->exec() == QDialog::Accepted)
        fileName = fd->selectedFile();
  \endcode

  In the above example, a modal file dialog is created and shown. If
  the user clicked OK, the file they selected is put in \c fileName.

  If you are using the \c ExistingFiles mode then you will need to use
  selectedFiles() which returns the selected files in a QStringList.

  The dialog's working directory can be set with setDirectory().
  The display of hidden files is controlled with showHidden().
  The dialog can be forced to refresh the directory display with reload()
  and sort the directory with sortByDate(), sortByName(), and
  sortBySize(). Each file in the current directory can be selected using
  the selectFile() function.

  \section1 Creating and using preview widgets

  A preview widget is a widget that is placed inside a QFileDialog so
  that the user can see either the contents of the file, or information
  about the file.

  There are two kinds of preview widgets that can be used with
  QFileDialogs: \e content preview widgets and \e information preview
  widgets. They are created and used with the setContentsPreview() and
  setInfoPreview() functions.

  \code
    class Preview : public QLabel, public QFilePreview
    {
    public:
        Preview(QWidget *parent=0) : QLabel(parent) {}

        void previewUrl(const QUrl &u) {
            QString path = u.path();
            QPixmap pix(path);
            if (pix.isNull())
                setText("This is not a pixmap");
            else
                setPixmap(pix);
        }
    };
  \endcode

  In the above snippet, we create a preview widget which inherits from
  QLabel and QFilePreview. File preview widgets \e must inherit from
  QFilePreview.

  Inside the class we reimplement QFilePreview::previewUrl(); this is
  where we determine what happens when a file is selected. In the
  above example we only show a preview of the file if it is a valid
  pixmap. Here's how to make a file dialog use a preview widget:

  \code
    Preview* preview = new Preview;

    QFileDialog* dialog = new QFileDialog(this);
    dialog->setContentsPreviewEnabled(true);
    dialog->setContentsPreview(preview, preview);
    dialog->setPreviewMode(QFileDialog::Contents);
    dialog->show();
  \endcode

  The first line creates an instance of our preview widget. We create
  our file dialog, and call setContentsPreviewEnabled(true) to preview
  the contents of the currently selected file. We then call
  setContentsPreview() -- note that we pass the same preview widget
  twice. Finally, before showing the file dialog, we call
  setPreviewMode(), setting the mode to \e Contents. As a result, the
  file dialog will show the contents preview of the file that the user
  has selected.

  If you create another preview widget that is used for displaying
  information about a file, create it in the same way as the contents
  preview widget then call setInfoPreviewEnabled() and setInfoPreview().
  The user will be able to switch between the two preview modes.

  For more information about creating a QFilePreview widget see
  \l{QFilePreview}.

  <img src=qfiledlg-m.png> <img src=qfiledlg-w.png>
*/

/*!
    \enum QFileDialog::ViewMode

    This enum describes the view mode of the file dialog; i.e. what
    information about each file will be displayed.

    \value Detail Displays an icon, a name, and details for each item in
                  the directory.
    \value List   Displays only an icon and a name for each item in the
                  directory.

    \sa setViewMode()
*/

/*!
    \enum QFileDialog::FileMode

    This enum is used to indicate what the user may select in the file
    dialog; i.e. what the dialog will return if the user clicks OK.

    \value AnyFile        The name of a file, whether it exists or not.
    \value ExistingFile   The name of a single existing file.
    \value Directory      The name of a directory. Both files and
                          directories are displayed.
    \value DirectoryOnly  The name of a directory. The file dialog will only display directories.
    \value ExistingFiles  The names of zero or more existing files.

    \sa setFileMode()
*/

/*!
    \enum QFileDialog::Option

    \value DontResolveSymlinks
    \value ShowDirsOnly
*/

/*!
    \fn QFileDialog::QFileDialog(QWidget *parent, Qt::WFlags flags)

    Constructs a file dialog with the given \a parent and widget \a flags.
*/

QFileDialog::QFileDialog(QWidget *parent, Qt::WFlags f)
    : QDialog(*new QFileDialogPrivate, parent, f)
{
    d->setup();
    setDirectory(QDir::home());
}

/*!
*/

QFileDialog::~QFileDialog()
{

}

/*!
    Sets the file dialog's current \a directory. (This is also applied to the
    underlying model.)
*/

void QFileDialog::setDirectory(const QDir &directory)
{
    QModelIndex index = d->model->index(directory.absolutePath());
    if (index.isValid()) {
        d->setRoot(index);
        d->updateButtons(index);
    }
}

/*!
    Returns the directory currently being displayed in the dialog.
*/

QDir QFileDialog::directory() const
{
    QFileInfo info = d->model->fileInfo(d->root());
    QDir dir(info.filePath());
    dir.setNameFilters(d->model->nameFilters());
    dir.setSorting(d->model->sorting());
    dir.setFilter(d->model->filter());
    return dir;
}

/*!
    Selects the given \a filename in both the file dialog and the
    underlying model.
*/

void QFileDialog::selectFile(const QString &filename)
{
    QStringList entries = directory().entryList(d->model->filter(), d->model->sorting());
    QModelIndex index = d->model->index(entries.indexOf(filename), 0, d->root());
    d->lview->selectionModel()->select(index, QItemSelectionModel::Select);
}

/*!
  Returns a list of strings containing the absolute paths of the
  selected files in the dialog. If no files are selected, or
  the mode is not ExistingFiles, selectedFiles is an empty list.

  It is more convenient to use selectedFile() if the mode is
  \c ExistingFile, \c Directory or \c DirectoryOnly.

  Note that if you want to iterate over the list, you should
  iterate over a copy, e.g.
    \code
    QStringList list = myFileDialog.selectedFiles();
    QStringList::Iterator it = list.begin();
    while( it != list.end() ) {
        myProcessing( *it );
        ++it;
    }
    \endcode

  \sa selectedFile, selectedFilter, QValueList::empty()
*/

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

/*!
  Sets the filter used in the file dialog to the given \a filter.

  If \a filter contains a pair of parentheses containing one or more
  of \bold{anything*something}, separated by spaces or by
  semicolons then only the text contained in the parentheses is used as
  the filter. This means that these calls are all equivalent:

  \code
     fd->setFilter("All C++ files (*.cpp *.cc *.C *.cxx *.c++)");
     fd->setFilter("*.cpp *.cc *.C *.cxx *.c++");
     fd->setFilter("All C++ files (*.cpp;*.cc;*.C;*.cxx;*.c++)");
     fd->setFilter("*.cpp;*.cc;*.C;*.cxx;*.c++");
  \endcode

  \sa setFilters()
*/

void QFileDialog::setFilter(const QString &filter)
{
    d->fileType->clear();
    d->fileType->insertItem(filter);
    useFilter(filter);
}

/*!
  Sets the \a filters used in the file dialog. Each group
  of filters must be separated by \c{;;} (\e two semicolons).

  \code
    QString types("Image files (*.png *.xpm *.jpg);;"
                  "Text files (*.txt);;"
                  "Any files (*)");
    QFileDialog fd = new QFileDialog( this );
    fd->setFilters( types );
    fd->show();
  \endcode
*/

void QFileDialog::setFilters(const QStringList &filters)
{
    d->fileType->clear();
    d->fileType->insertStringList(filters);
    useFilter(filters.first());
}

/*!
    Returns the file type filters that are in operation on this file
    dialog.
*/

QStringList QFileDialog::filters() const
{
    QStringList items;
    for (int i = 0; i < d->fileType->count(); ++i)
        items.append(d->fileType->text(i));
    return items;
}

/*!
    Sets the current file type \a filter. Multiple filters can be
    passed in \a filter by separating them with semicolons or spaces.

    \sa setFilter() setFilters()
*/

void QFileDialog::selectFilter(const QString &filter)
{
    d->fileType->setCurrentText(filter);
    setFilter(filter);
}

/*!
  Returns the filter that the user selected in the file dialog.

  \sa filterSelected(), selectedFiles, selectedFile
*/

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

/*!
    \property QFileDialog::viewMode
    \brief the way files and directories are displayed in the dialog

    By default, the \c Detail mode is used to display information about
    files and directories.

    \sa ViewMode viewMode() setViewMode()
*/

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

/*!
    \property QFileDialog::fileMode
    \brief the file mode of the dialog

    The file mode defines the number and type of items that the user is
    expected to select in the dialog.

    By default,

    \sa FileMode fileMode() setFileMode()
*/

QFileDialog::FileMode QFileDialog::fileMode() const
{
    return d->fileMode;
}

/*!
 \reimp*/

void QFileDialog::done(int result)
{
    QDialog::done(result);
}

/*!
    \reimp
*/

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

/*!
    \reimp
*/

void QFileDialog::reject()
{
    QDialog::reject();
}

/*!
    \internal

    Navigates to the last directory viewed in the dialog.
*/

void QFileDialog::back()
{
    QModelIndex root = d->history.back();
    d->history.pop_back();
    d->setRoot(root);
    d->updateButtons(root);
}

/*!
    \internal

    Navigates to the parent directory of the currently displayed directory
    in the dialog.
*/

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

/*!
    \internal

    Creates a new directory, first asking the user for a suitable name.
*/

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

/*!
    \internal

    Displays the contents of the current directory in the form of a list of
    icons and names.

    \sa ViewMode
*/

void QFileDialog::showList()
{
    d->listMode->setDown(true);
    d->detailMode->setDown(false);
    d->lview->show();
    d->lview->doItemsLayout();
    d->tview->hide();
}

/*!
    \internal

    Displays the contents of the current directory in the form of a list of
    icons and names, alongside which are displayed further details of each
    item.

    \sa ViewMode*/

void QFileDialog::showDetail()
{
    d->listMode->setDown(false);
    d->detailMode->setDown(true);
    d->tview->show();
    d->tview->doItemsLayout();
    d->lview->hide();
}

/*!
    \internal

    This is called when the user double clicks on a file with the corresponding
    model item \a index.
*/

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

/*!
    \internal

    This is called when the user requests that a file be deleted; the
    corresponding model index is passed in \a index.
*/

void QFileDialog::deletePressed(const QModelIndex &index)
{
    if (d->model->isDir(index))
        d->model->rmdir(index);
    else
        d->model->remove(index);
}

/*!
    \fn void QFileDialog::currentChanged(const QModelIndex &index, const QModelIndex &current)

    \internal

    This is called when the model index corresponding to the current file is changed
    from \a index to \a current.
*/

void QFileDialog::currentChanged(const QModelIndex &, const QModelIndex &current)
{
    if (!d->fileName->hasFocus() && current.isValid()) {
        QString text = d->model->data(current, QAbstractItemModel::Role_Display).toString();
        d->fileName->setText(text);
    }
}

/*!
    \internal

    This is called when the filename is changed; the new name is
    passed in \a text.
*/

void QFileDialog::fileNameChanged(const QString &text)
{
    if (d->fileName->hasFocus() && !text.isEmpty()) {
        QModelIndex current = d->current();
        if (!current.isValid())
            current = d->model->topLeft(d->root());
        QModelIndexList indices = d->model->match(current, QAbstractItemModel::Role_Display, text);
        int key = d->fileName->lastKeyPressed();
        if (indices.count() <= 0) { // no matches
            d->lview->selectionModel()->clear();
        } else if (key != Qt::Key_Delete && key != Qt::Key_Backspace) {
            d->setCurrent(indices.first());
            QString completed = d->model->data(indices.first(),
                                               QAbstractItemModel::Role_Display).toString();
            int start = completed.length();
            int length = text.length() - start; // negative length
            bool block = d->fileName->blockSignals(true);
            d->fileName->setText(completed);
            d->fileName->setSelection(start, length);
            d->fileName->blockSignals(block);
        }
    }
}

/*!
    \internal

    This is called when the user changes the text in the "Look in"
    combobox; the new text is passed in \a text. The function updates
    the model and consequently the file dialog accordingly.
*/

void QFileDialog::lookInChanged(const QString &text)
{
    if (d->lookInEdit->hasFocus() && !text.isEmpty()) {
        QString pth = text.left(text.lastIndexOf('/'));
        QModelIndex dirIndex = d->model->index(pth);
        QString searchText = text.section('/', -1);
        int rowCount = d->model->rowCount(dirIndex);
        QModelIndexList indices = d->model->match(d->model->topLeft(dirIndex),
                                                  QAbstractItemModel::Role_Display,
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

/*!
    \internal

    Adds the given \a filter to the file dialog's name filter.
    (Actually the name filters are held in the underlying model and
    this is what is really updated.)
*/

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

/*!
    \internal

    Changes the file dialog's current directory to the one specified
    by \a path. This also sets the root of the underlying model to the
    item holding the \a path.
*/

void QFileDialog::setCurrentDir(const QString &path)
{
    QModelIndex index = d->model->index(path);
    d->setRoot(index);
    d->updateButtons(index);
}

/*!
    \internal

    This creates the default context menu for the file list. The
    context menu is passed in \a menu and the index into the
    underlying model-view in \a index.
*/

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

/*!
    \internal

    The \a{section}-th column header in the files list was clicked.
*/

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

/*!
    Tells the dialog to rename the currently selected item using input from
    the user.
*/

void QFileDialog::renameCurrent()
{
    QAbstractItemView *view = d->listMode->isDown()
                              ? static_cast<QAbstractItemView*>(d->lview)
                              : static_cast<QAbstractItemView*>(d->tview);
    view->edit(d->current());
}

/*!
    \internal

    Deletes the currently selected item in the dialog.
*/

void QFileDialog::deleteCurrent()
{
    deletePressed(d->current());
}

/*!
    \internal

    Refreshes the display of the current directory in the dialog.
*/

void QFileDialog::reload()
{
    d->model->refresh(d->root());
}

/*!
    \internal

    Sorts the items in the dialog by name order.
*/

void QFileDialog::sortByName()
{
    d->setDirSorting(QDir::Name|QDir::DirsFirst|(d->model->filter() & QDir::Reversed));
}

/*!
    \internal

    Sorts the items in the dialog by size order.
*/

void QFileDialog::sortBySize()
{
    d->setDirSorting(QDir::Size|QDir::DirsFirst|(d->model->filter() & QDir::Reversed));
}

/*!
    \internal

    Sorts the items in the dialog by date order.
*/

void QFileDialog::sortByDate()
{
    d->setDirSorting(QDir::Time|QDir::DirsFirst|(d->model->filter() & QDir::Reversed));
}

/*!
    \internal

    Displays the contents of the current directory in an arbitrary order.

    \sa sortByDate() sortByName() sortBySize()
*/

void QFileDialog::setUnsorted()
{
    d->setDirSorting(QDir::Unsorted|QDir::DirsFirst|(d->model->filter() & QDir::Reversed));
}

/*!
   \internal

   Includes hidden files and directories in the items displayed in the dialog.

*/

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
    QGridLayout *grid = new QGridLayout(q);
    grid->setMargin(11);
    grid->setSpacing(6);

    // model
    QDir dir("/");
    dir.setFilter(dir.filter() | QDir::AllDirs);
    dir.setSorting(QDir::DirsFirst);
    model = new QDirModel(dir, q);

    // views
    lview = new QGenericListView(q);
    lview->setModel(model);
    lview->viewport()->setAcceptDrops(true);
    lview->setSpacing(2);
    lview->setWrapping(true);
    lview->setResizeMode(QGenericListView::Adjust);
    lview->setBeginEditActions(QAbstractItemDelegate::EditKeyPressed);
    grid->addWidget(lview, 1, 0, 1, 6);

    QItemSelectionModel *selections = lview->selectionModel();

    tview = new QGenericTreeView(q);
    tview->setModel(model);
    tview->viewport()->setAcceptDrops(true);
    tview->setSelectionModel(selections);
    tview->showRootDecoration(false);
    tview->header()->setResizeMode(QGenericHeader::Custom);
    tview->header()->setResizeMode(QGenericHeader::Stretch, 0);
    tview->header()->setSortIndicator(0, Qt::DescendingOrder);
    tview->hide();
    tview->setBeginEditActions(QAbstractItemDelegate::EditKeyPressed);
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
    QHBoxLayout *box = new QHBoxLayout;
    box->setMargin(3);
    box->setSpacing(3);
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
    if (!lookIn->contains(pth)) {
        int insertRow = lookIn->count();
        lookIn->insertItem(pth, insertRow);
        lookIn->setCurrentItem(insertRow);
    }
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
  selected by the user. If the user presses Cancel, it returns a null
  string.

  \code
    QString s = QFileDialog::getOpenFileName(
                    "/home",
                    "Images (*.png *.xpm *.jpg)",
                    this,
                    "Choose a file to open");
  \endcode

  The function creates a modal file dialog with the given \a parent widget.
  If the parent is not 0, the dialog will be shown centered over the
  parent widget.

  The file dialog's working directory will be set to \a dir.
  If \a dir includes a file name, the file will be selected. Only files
  that match the given \a filter are shown. The filter selected is set to
  \a selectedFilter. The parameters \a dir, \a selectedFilter, and
  \a filter may be empty strings.

  The dialog's caption is set to \a caption. If \a caption is not
  specified then a default caption will be used.

  Under Windows and Mac OS X, this static function will use the native
  file dialog and not a QFileDialog, unless the style of the application
  is set to something other than the native style.

  Note that on Windows the dialog will spin a blocking modal event loop
  that will not dispatch any QTimers, and if parent is not 0 then it will
  position the dialog just under the parent's title bar.

  ### Under Unix/X11, the normal behavior of the file dialog is to resolve
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

  It creates a modal file dialog with the given \a parent widget. If the
  parent is not 0, the dialog will be shown centered over the parent
  widget.

  \code
    QString s = QFileDialog::getSaveFileName(
                    "/home",
                    "Images (*.png *.xpm *.jpg)",
                    this,
                    "Choose a filename to save under");
  \endcode

  The file dialog's working directory will be set to \a dir. If \a
  dir includes a file name, the file will be selected. Only files that
  match the \a filter are shown. The filter selected is set to
  \a selectedFilter. The parameters \a dir, \a selectedFilter, and
  \a filter may be empty strings.

  The dialog's caption is set to \a caption. If \a caption is not
  specified then a default caption will be used.

  Under Windows and Mac OS X, this static function will use the native
  file dialog and not a QFileDialog, unless the style of the application
  is set to something other than the native style.

  Note that on Windows the dialog will spin a blocking modal event loop
  that will not dispatch any QTimers, and if parent is not 0 then it will
  position the dialog just under the parent's title bar.
  On Mac OS X, the filter argument is ignored.

  ### Under Unix/X11, the normal behavior of the file dialog is to resolve
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
  This is a convenience static function that will return an existing
  directory selected by the user.

  \code
    QString s = QFileDialog::getExistingDirectory(
                    "/home",
                    this,
                    "Choose a directory",
                    true);
  \endcode

  This function creates a modal file dialog with the given \a parent
  widget. If the parent is not 0, the dialog will be shown centered over
  the parent widget.

  The dialog's working directory is set to \a dir, and the caption is
  set to \a caption. Either of these may be an empty string in which case
  the current directory and a default caption will be used respectively.

  ### Under Unix/X11, the normal behavior of the file dialog is to resolve
  and follow symlinks. For example, if /usr/tmp is a symlink to /var/tmp,
  the file dialog will change to /var/tmp after entering /usr/tmp.
  If \a resolveSymlinks is false, the file dialog will treat
  symlinks as regular directories.

  Note that on Windows the dialog will spin a blocking modal event loop
  that will not dispatch any QTimers, and if parent is not 0 then it will
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

  This function creates a modal file dialog with the given \a parent
  widget. If the parent is not 0, the dialog will be shown centered
  over the parent widget.

  The file dialog's working directory will be set to \a dir. If \a
  dir includes a file name, the file will be selected. The filter
  is set to \a filter so that only those files which match the filter
  are shown. The filter selected is set to \a selectedFilter. The parameters
  \a dir, \a selectedFilter and \a filter may be empty strings.

  The dialog's caption is set to \a caption. If \a caption is not
  specified then a default caption will be used.

  Under Windows and Mac OS X, this static function will use the native
  file dialog and not a QFileDialog, unless the style of the application
  is set to something other than the native style.

  Note that on Windows the dialog will spin a blocking modal event loop
  that will not dispatch any QTimers, and if parent is not 0 then it will
  position the dialog just under the parent's title bar.

  ### Under Unix/X11, the normal behavior of the file dialog is to resolve
  and follow symlinks. For example, if /usr/tmp is a symlink to /var/tmp,
  the file dialog will change to /var/tmp after entering /usr/tmp.
  If \a resolveSymlinks is false, the file dialog will treat
  symlinks as regular directories.

  Note that if you want to iterate over the list of files, you should
  iterate over a copy. For example:

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
