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

#include "qfiledialog.h"
#include <qcombobox.h>
#include <qdirmodel.h>
#include <qframe.h>
#include <qheaderview.h>
#include <qlistview.h>
#include <qtreeview.h>
#include <qitemselectionmodel.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmenu.h>
#include <qevent.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qsignal.h>
#include <qtoolbutton.h>
#include <qmessagebox.h>
#include <qwindowsstyle.h>
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

// Makes a list of filters from a normal filter string "Image Files (*.png *.jpg)"
static QStringList qt_clean_filter_list(const QString &filter)
{
    QRegExp regexp(QString::fromLatin1(qt_file_dialog_filter_reg_exp));
    QString f = filter;
    int i = regexp.indexIn(f);
    if (i >= 0)
        f = regexp.cap(2);
    return f.split(' ', QString::SkipEmptyParts);
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
    Q_DECLARE_PUBLIC(QFileDialog)
public:
    QFileDialogPrivate()
        : QDialogPrivate(),
          model(0), lview(0), tview(0),
          viewMode(QFileDialog::Detail),
          fileMode(QFileDialog::AnyFile),
          acceptMode(QFileDialog::AcceptOpen),
          frame(0), lookIn(0), fileName(0), fileType(0),
          openAction(0), renameAction(0), deleteAction(0),
          reloadAction(0), sortByNameAction(0), sortBySizeAction(0),
          sortByDateAction(0), unsortedAction(0), showHiddenAction(0),
          acceptButton(0), cancelButton(0),
          back(0), toParent(0), newFolder(0), detailMode(0), listMode(0)
        {}

    void setup(const QString &directory, const QStringList &nameFilter);

    void updateButtons(const QModelIndex &index);

    void setRoot(const QModelIndex &index);
    QModelIndex root() const;

    void setDirSorting(QDir::SortFlags sort);
    void setDirFilter(QDir::Filters filter);

    QDir::Filters filterForMode(QFileDialog::FileMode mode);
    QAbstractItemView::SelectionMode selectionMode(QFileDialog::FileMode mode);

    inline QString tr(const char *text) const { return QObject::tr(text); }

    QDirModel *model;
    QItemSelectionModel *selections;
    QListView *lview;
    QTreeView *tview;
    QFileDialog::ViewMode viewMode;
    QFileDialog::FileMode fileMode;
    QFileDialog::AcceptMode acceptMode;

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

    QPushButton *acceptButton;
    QPushButton *cancelButton;

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
                    this,
                    "Choose a file",
                    "/home",
                    "Images (*.png *.xpm *.jpg)");
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
    \enum QFileDialog::AcceptMode

    \value Open
    \value Save
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
    \fn void QFileDialog::filesSelected(const QStringList &selected)

    When the selection changes this signal is emitted with the
    (possibly empty) list of \a selected files.
*/

/*!
    \fn QFileDialog::QFileDialog(QWidget *parent, Qt::WFlags flags)

    Constructs a file dialog with the given \a parent and widget \a flags.
*/

QFileDialog::QFileDialog(QWidget *parent, Qt::WFlags f)
    : QDialog(*new QFileDialogPrivate, parent, f)
{
    QDir dir = QDir::current();
    d->setup(dir.absolutePath(), dir.nameFilters());
}

/*!
    Constructs a file dialog with the given \a parent. The dialog has
    the given \a caption, starts in directory \a dir, has the filters
    specified by \a filter, with the filter in \a selectedFilter
    selected, and with the \a selectedFile selected. The files that
    can be chosen are determined by the \a fileMode.
*/
QFileDialog::QFileDialog(QWidget *parent,
                         const QString &caption,
                         const QString &dir,
                         const QString &filter,
                         const QString &selectedFilter,
                         const QString &selectedFile,
                         FileMode fileMode)
    : QDialog(*new QFileDialogPrivate, parent, 0)
{
    setWindowTitle(caption);
    QStringList nameFilter = qt_make_filter_list(filter);
    d->fileMode = fileMode;
    d->setup(dir, nameFilter.isEmpty() ? QStringList(tr("All Files (*)")) : nameFilter);
    if (!selectedFilter.isEmpty())
        selectFilter(selectedFilter); // slow
    if (!selectedFile.isEmpty())
        selectFile(selectedFile);
}

/*!
*/

QFileDialog::~QFileDialog()
{

}

/*!
    \fn void QFileDialog::setDirectory(const QDir &directory)

    \overload
*/

/*!
    Sets the file dialog's current \a directory.
*/

void QFileDialog::setDirectory(const QString &directory)
{
    QModelIndex index = d->model->index(directory);
    d->setRoot(index);
    d->updateButtons(index);
}

/*!
    Returns the directory currently being displayed in the dialog.
*/

QDir QFileDialog::directory() const
{
    QFileInfo info = d->model->fileInfo(d->root());
    QDir dir = info.dir();
    dir.setNameFilters(d->model->nameFilters());
    dir.setSorting(d->model->sorting());
    dir.setFilter(d->model->filter());
    return dir;
}

/*!
    Selects the given \a filename in both the file dialog.
*/

void QFileDialog::selectFile(const QString &filename)
{
    QStringList entries = directory().entryList(d->model->filter(), d->model->sorting());
    int r = entries.indexOf(filename);
    if (r < 0)
        return;
    QModelIndex index = d->model->index(r, 0, d->root());
    if (index.isValid())
        d->selections->select(index, QItemSelectionModel::Select);
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
    QModelIndexList indexes = d->selections->selectedIndexes();
    QStringList files;
    int r = -1;
    for (int i = 0; i < indexes.count(); ++i) {
        if (indexes.at(i).row() != r) {
            files.append(d->model->path(indexes.at(i)));
            r = indexes.at(i).row();
        }
    }
    if (d->fileMode == AnyFile && files.count() <= 0) { // a new filename
        QString file = d->fileName->text();
        if (QDir::isAbsolutePath(file))
            files.append(file);
        else
            files.append(d->lookIn->currentText() + QDir::separator() + file);
    }
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
  Sets the \a filters used in the file dialog.

  \code
    QStringList types;
    types << "Image files (*.png *.xpm *.jpg)"
              << "Text files (*.txt)"
              << "Any files (*)";
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
    int i = d->fileType->findItem(filter, QAbstractItemModel::MatchExactly);
    if (i >= 0)
        d->fileType->setCurrentItem(i);
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
    QAbstractItemView::SelectionMode selectionMode = d->selectionMode(mode);
    d->lview->setSelectionMode(selectionMode);
    d->tview->setSelectionMode(selectionMode);
    d->model->setFilter(d->filterForMode(mode));
    if (mode == DirectoryOnly) {
        d->fileType->clear();
        d->fileType->insertItem(QFileDialog::tr("Directories"));
        d->fileType->setEnabled(false);
    }
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

void QFileDialog::setAcceptMode(AcceptMode mode)
{
    d->acceptMode = mode;
    d->openAction->setText(mode == AcceptOpen ? tr("&Open") : tr("&Save"));
    d->acceptButton->setText(mode == AcceptOpen ? tr("Open") : tr("Save"));
}

/*!
    \property QFileDialog::acceptMode
    \brief the accept mode of the dialog

    The action mode defines whether the dialog is for opening or saving files.

    \sa AcceptMode acceptMode() setAcceptMode()
*/

QFileDialog::AcceptMode QFileDialog::acceptMode() const
{
    return d->acceptMode;
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
    QString fn = d->fileName->text();

    if (files.count() < 1)
        files.append(fn);

    switch (d->fileMode) {
    case DirectoryOnly:
    case Directory:
        if (QFileInfo(files.first()).isDir()) {
            emit filesSelected(files);
            QDialog::accept();
        }
        return;
    case AnyFile: {
        QString fn = files.first();
        QFileInfo info(fn);
        if (info.isDir())
            return;
        if (!info.exists())
            QDialog::accept();
        else if (QMessageBox::warning(this, windowTitle(),
                                      fn + tr(" already exists.\nDo you want to replace it?"),
                                      QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
            QDialog::accept();
        return;}
    case ExistingFile:
    case ExistingFiles:
        for (int i = 0; i < files.count(); ++i) {
            QFileInfo info(files.at(i));
            if (!info.exists()) {
                QString message = tr("\nFile not found.\nPlease verify the correct file name was given");
                QMessageBox::warning(this, d->acceptButton->text(), info.fileName() + message);
                return;
            }
            if (info.isDir()) {
                setDirectory(info.absoluteFilePath());
                return;
            }
        }
        emit filesSelected(files);
        QDialog::accept();
        return;
    }
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
    d->lview->clearSelection(); // FIXME: the selection model doesn't use persistent indices yet

    QModelIndex index = d->model->mkdir(parent, "New Folder");
    if (!index.isValid())
        return;
    if (!index.isValid()) {
        d->selections->setCurrentIndex(d->model->index(0, 0, d->root()),
                                       QItemSelectionModel::SelectCurrent);
        return;
    }
    d->selections->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
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
    if (d->model->hasChildren(index)) {
        d->history.push_back(d->root());
        d->setRoot(index);
        d->updateButtons(index);
    } else {
        accept();
    }
}

/*!
    \internal

    This is called when the user presses a \a key with the
    \a modifiers down, when the current item is \a index.
*/

void QFileDialog::keyPressed(const QModelIndex &index, Qt::Key key, Qt::KeyboardModifiers)
{
    switch (key) {
    case Qt::Key_Delete:
        deletePressed(index);
        return;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        doubleClicked(index);
        return;
    default:
        return;
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

void QFileDialog::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    if (!d->fileName->hasFocus() && current.isValid()) {
        QString text = d->model->data(current, QAbstractItemModel::DisplayRole).toString();
        d->fileName->setText(text);
    }
}

/*!
    \internal

    This is called when the text in the filename lineedit is changed; the new text
    is passed as \a text. This function provides autocompletion for filenames.
*/

void QFileDialog::fileNameChanged(const QString &text)
{
    if (d->fileName->hasFocus() && !text.isEmpty()) {
        QModelIndex current = d->selections->currentIndex();
        if (!current.isValid())
            current = d->model->index(0, 0, d->root());
        QModelIndexList indexes = d->model->match(current, QAbstractItemModel::DisplayRole, text);
        int key = d->fileName->lastKeyPressed();
        if (indexes.count() <= 0) { // no matches
            d->selections->clear();
        } else if (key != Qt::Key_Delete && key != Qt::Key_Backspace) {
            d->selections->setCurrentIndex(indexes.first(), QItemSelectionModel::SelectCurrent);
            QString completed = d->model->data(indexes.first(),
                                               QAbstractItemModel::DisplayRole).toString();
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
    combobox; the new text is passed in \a text. The file dialog accordingly.
*/

void QFileDialog::lookInChanged(const QString &text)
{
    if (d->lookInEdit->hasFocus()) {

        int key = d->lookInEdit->lastKeyPressed();
        if (key == QDir::separator())
            return;

        QModelIndex result;
        if (!text.isEmpty()) {
            int s = text.lastIndexOf(QDir::separator());
            QString pth = (s == 0 ? QDir::rootPath() : text.left(s));
            QModelIndex dirIndex = d->model->index(pth);
            QString searchText = text.section(QDir::separator(), -1);
            int rowCount = d->model->rowCount(dirIndex);
            QModelIndexList indices = d->model->match(d->model->index(0, 0, dirIndex),
                                                      QAbstractItemModel::DisplayRole,
                                                      searchText, rowCount);
            for (int i = 0; i < indices.count(); ++i) {
                if (d->model->isDir(indices.at(i))) {
                    result = indices.at(i);
                    break;
                }
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
    QStringList filters = qt_clean_filter_list(filter);
    d->model->setNameFilters(filters);

    // FIXME: workaroud for problem in rowsRemoved()/rowsInserted()
    d->lview->doItemsLayout();
    d->tview->doItemsLayout();
}

/*!
    \internal

    Changes the file dialog's current directory to the one specified
    by \a path.
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
    underlying model in \a index.
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
        QMenu *sort = new QMenu(tr("Sort"));
        menu->addMenu(sort);
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

void QFileDialog::headerPressed(int section)
{
    QDir::SortFlags sort = QDir::NoSort;
    switch (section) {
    case 0:
        sort = QDir::Name;
        break;
    case 1:
        sort = QDir::Size;
        break;
    case 2:
        qDebug("sorting on type is not implemented yet");
        return;
    case 3:
        sort = QDir::Time;
        break;
    default:
        return;
    }
    QHeaderView *header = d->tview->header();
    Qt::SortOrder order = (header->sortIndicatorSection() == section
                       && header->sortIndicatorOrder() == Qt::DescendingOrder)
                      ? Qt::AscendingOrder : Qt::DescendingOrder;
    bool sortByName = (sort & QDir::SortByMask) == QDir::Name;
    bool reverse = (order == Qt::AscendingOrder ? sortByName : !sortByName);
    if (reverse) {
        sort |= QDir::Reversed;
        sort |= QDir::DirsLast;
        sort &= ~QDir::DirsFirst;
    } else {
        sort &= ~QDir::Reversed;
        sort &= ~QDir::DirsLast;
        sort |= QDir::DirsFirst;
    }

    d->setDirSorting(sort);
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
    view->edit(d->selections->currentIndex());
}

/*!
    \internal

    Deletes the currently selected item in the dialog.
*/

void QFileDialog::deleteCurrent()
{
    deletePressed(d->selections->currentIndex());
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

    Sets the current directory to the path showed in the "look in" combobox.
    Called when the enterPressed() signal is emitted.
*/

void QFileDialog::lookIn()
{
    QString path = d->lookIn->currentText();
    QModelIndex index = d->model->index(path);
    d->setRoot(index);
    d->updateButtons(index);
}

/*!
    \internal

    Sorts the items in the dialog by name order.
*/

void QFileDialog::sortByName()
{
    QDir::SortFlags sort = QDir::SortFlags(QDir::Name|QDir::DirsFirst);
    if(d->model->filter() & QDir::Reversed)
        sort |= QDir::Reversed;
    d->setDirSorting(sort);
}

/*!
    \internal

    Sorts the items in the dialog by size order.
*/

void QFileDialog::sortBySize()
{
    QDir::SortFlags sort = QDir::SortFlags(QDir::Size|QDir::DirsFirst);
    if(d->model->filter() & QDir::Reversed)
        sort |= QDir::Reversed;
    d->setDirSorting(sort);
}

/*!
    \internal

    Sorts the items in the dialog by date order.
*/

void QFileDialog::sortByDate()
{
    QDir::SortFlags sort = QDir::SortFlags(QDir::Time|QDir::DirsFirst);
    if(d->model->filter() & QDir::Reversed)
        sort |= QDir::Reversed;
    d->setDirSorting(sort);
}

/*!
    \internal

    Displays the contents of the current directory in an arbitrary order.

    \sa sortByDate() sortByName() sortBySize()
*/

void QFileDialog::setUnsorted()
{
    QDir::SortFlags sort = QDir::SortFlags(QDir::Unsorted|QDir::DirsFirst);
    if(d->model->filter() & QDir::Reversed)
        sort |= QDir::Reversed;
    d->setDirSorting(sort);
}

/*!
   \internal

   Includes hidden files and directories in the items displayed in the dialog.

*/

void QFileDialog::showHidden()
{
    QDir::Filters filters = d->model->filter();
    if(d->showHiddenAction->isChecked())
        filters &= ~(int)QDir::Hidden;
    else
        filters |= QDir::Hidden;
    d->setDirFilter(filters);
}

void QFileDialogPrivate::setup(const QString &directory,
                               const QStringList &nameFilter)
{
    q->setSizeGripEnabled(true);
    QGridLayout *grid = new QGridLayout(q);
    grid->setMargin(11);
    grid->setSpacing(6);

    // Model
    QDir::Filters filters = filterForMode(fileMode);
    QDir::SortFlags sort = QDir::SortFlags(QDir::Name|QDir::IgnoreCase|QDir::DirsFirst);
    QAbstractItemView::SelectionMode selMode = selectionMode(fileMode);
    QStringList cleanedFilter = qt_clean_filter_list(nameFilter.first());
    model = new QDirModel(QString::null, cleanedFilter, filters, sort, q);
    selections = new QItemSelectionModel(model, model);
    QModelIndex current = directory.isEmpty() ? QModelIndex::Null : model->index(directory);

    // views
    lview = new QListView(q);

    lview->setModel(model);
    lview->setSelectionModel(selections);
    lview->setSelectionMode(selMode);
    lview->setRoot(current);
    lview->setKeyTracking(true);

    lview->viewport()->setAcceptDrops(true);
    lview->setSpacing(2);
    lview->setWrapping(true);
    lview->setResizeMode(QListView::Adjust);
    lview->setLayoutMode(QListView::Batched);
    lview->setEditTriggers(QAbstractItemView::EditKeyPressed);
    grid->addWidget(lview, 1, 0, 1, 6);

    tview = new QTreeView(q);

    tview->setModel(model);
    tview->setSelectionModel(selections);
    tview->setSelectionMode(selMode);
    tview->setRoot(current);
    tview->setKeyTracking(true);

    tview->viewport()->setAcceptDrops(true);
    tview->setRootIsDecorated(false);
    tview->header()->setResizeMode(QHeaderView::Custom);
    tview->header()->setResizeMode(QHeaderView::Stretch, 0);
    tview->header()->setSortIndicator(0, Qt::DescendingOrder);
    tview->header()->setSortIndicatorShown(true);
    tview->hide();
    tview->setEditTriggers(QAbstractItemView::EditKeyPressed);
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

    // connect signals
    QObject::connect(lview, SIGNAL(aboutToShowContextMenu(QMenu*,QModelIndex)),
                     q, SLOT(populateContextMenu(QMenu*,QModelIndex)));
    QObject::connect(tview, SIGNAL(aboutToShowContextMenu(QMenu*,QModelIndex)),
                     q, SLOT(populateContextMenu(QMenu*,QModelIndex)));
    QObject::connect(openAction, SIGNAL(triggered()), q, SLOT(accept()));
    QObject::connect(renameAction, SIGNAL(triggered()), q, SLOT(renameCurrent()));
    QObject::connect(deleteAction, SIGNAL(triggered()), q, SLOT(deleteCurrent()));
    QObject::connect(reloadAction, SIGNAL(triggered()), q, SLOT(reload()));
    QObject::connect(sortByNameAction, SIGNAL(triggered()), q, SLOT(sortByName()));
    QObject::connect(sortBySizeAction, SIGNAL(triggered()), q, SLOT(sortBySize()));
    QObject::connect(sortByDateAction, SIGNAL(triggered()), q, SLOT(sortByDate()));
    QObject::connect(unsortedAction, SIGNAL(triggered()), q, SLOT(setUnsorted()));
    QObject::connect(showHiddenAction, SIGNAL(triggered()), q, SLOT(showHidden()));

    QObject::connect(lview, SIGNAL(doubleClicked(QModelIndex,Qt::MouseButton,Qt::KeyboardModifiers)),
                     q, SLOT(doubleClicked(QModelIndex)));
    QObject::connect(tview, SIGNAL(doubleClicked(QModelIndex,Qt::MouseButton,Qt::KeyboardModifiers)),
                     q, SLOT(doubleClicked(QModelIndex)));
    QObject::connect(selections, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                     q, SLOT(currentChanged(QModelIndex,QModelIndex)));
    QObject::connect(lview, SIGNAL(keyPressed(QModelIndex,Qt::Key,Qt::KeyboardModifiers)),
                     q, SLOT(keyPressed(QModelIndex,Qt::Key,Qt::KeyboardModifiers)));
    QObject::connect(tview, SIGNAL(keyPressed(QModelIndex,Qt::Key,Qt::KeyboardModifiers)),
                     q, SLOT(keyPressed(QModelIndex,Qt::Key,Qt::KeyboardModifiers)));
    QObject::connect(tview->header(), SIGNAL(sectionPressed(int)),
                     q, SLOT(headerPressed(int)));

    // labels
    grid->addWidget(new QLabel(tr("Look in:"), q), 0, 0);
    grid->addWidget(new QLabel(tr("File name:"), q), 2, 0);
    grid->addWidget(new QLabel(tr("Files of type:"), q), 3, 0);

    // push buttons
    acceptButton = new QPushButton(tr("Open"), q);
    QObject::connect(acceptButton, SIGNAL(clicked()), q, SLOT(accept()));
    grid->addWidget(acceptButton, 2, 5, Qt::AlignLeft);

    cancelButton = new QPushButton(tr("Cancel"), q);
    QObject::connect(cancelButton, SIGNAL(clicked()), q, SLOT(reject()));
    grid->addWidget(cancelButton, 3, 5, Qt::AlignLeft);

    // conboboxes && lineedits
    lookIn = new QComboBox(q);
    lookIn->setInsertionPolicy(QComboBox::NoInsertion);
    lookIn->setDuplicatesEnabled(false);
    lookIn->setEditable(true);
    lookIn->setAutoCompletion(false);

    // insert the paths
    lookIn->insertItem(model->icon(QModelIndex::Null), model->path(QModelIndex::Null)); // root
    for (int r = 0; r < model->rowCount(QModelIndex::Null); ++r) { // drives
        QModelIndex index = model->index(r, 0, QModelIndex::Null);
        QString path = model->path(index);
        QIcon icons = model->icon(index);
        lookIn->insertItem(icons, path);
    }
    QModelIndex home = model->index(QDir::homePath());
    lookIn->insertItem(model->icon(home), QDir::homePath());
    lookIn->insertItem(model->icon(current), directory);
    int c = lookIn->findItem(directory, QAbstractItemModel::MatchExactly);
    lookIn->setCurrentItem(c >= 0 ? c : 0);
    QObject::connect(lookIn, SIGNAL(activated(QString)),
                     q, SLOT(setCurrentDir(QString)));
    grid->addWidget(d->lookIn, 0, 1, 1, 3);

    lookInEdit = new QFileDialogLineEdit(lookIn);
    QObject::connect(lookInEdit, SIGNAL(textChanged(QString)),
                     q, SLOT(lookInChanged(QString)));
    QObject::connect(lookInEdit, SIGNAL(returnPressed()), q, SLOT(lookIn()));
    lookIn->setLineEdit(lookInEdit); // FIXME: this will crash if done before the connects above!!!

    fileName = new QFileDialogLineEdit(q);
    QObject::connect(fileName, SIGNAL(textChanged(QString)),
                     q, SLOT(fileNameChanged(QString)));
    grid->addWidget(fileName, 2, 2, 1, 3);

    fileType = new QComboBox(q);
    fileType->setDuplicatesEnabled(false);
    if (fileMode == QFileDialog::DirectoryOnly) {
        fileType->insertItem(QFileDialog::tr("Directories"));
        fileType->setEnabled(false);
    } else {
        fileType->insertStringList(nameFilter);
    }
    QObject::connect(fileType, SIGNAL(activated(QString)),
                     q, SLOT(useFilter(QString)));
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
    toParent->setEnabled(model->parent(current).isValid());
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

    // tab order
    QWidget::setTabOrder(lookIn, lview);
    QWidget::setTabOrder(lview, tview);
    QWidget::setTabOrder(tview, fileName);
    QWidget::setTabOrder(fileName, fileType);
    QWidget::setTabOrder(fileType, acceptButton);
    QWidget::setTabOrder(acceptButton, cancelButton);
    QWidget::setTabOrder(cancelButton, lookIn);

    // last init
    grid->addLayout(box, 0, 4);
    q->resize(550, 320);
    fileName->setFocus();
}

void QFileDialogPrivate::updateButtons(const QModelIndex &index)
{
    toParent->setEnabled(index.isValid());
    back->setEnabled(history.count() > 0);
    QString pth = d->model->path(index);
    QIcon icn = d->model->icon(index);
    int i = lookIn->findItem(pth, QAbstractItemModel::MatchExactly);
    bool block = lookIn->blockSignals(true);
    if (i > -1) {
        lookIn->setCurrentItem(i);
    } else {
        lookIn->insertItem(icn, pth);
        lookIn->setCurrentItem(lookIn->count() - 1);
    }
    lookIn->blockSignals(block);
}

void QFileDialogPrivate::setRoot(const QModelIndex &index)
{
    bool block = d->selections->blockSignals(true);
    d->selections->clear();
    d->fileName->clear();
    lview->setRoot(index);
    tview->setRoot(index);
    d->selections->blockSignals(block);
    d->selections->setCurrentIndex(d->model->index(0, 0, index), QItemSelectionModel::SelectCurrent);
}

QModelIndex QFileDialogPrivate::root() const
{
    return lview->root();
}

void QFileDialogPrivate::setDirSorting(QDir::SortFlags sort)
{
    int sortBy = sort & QDir::SortByMask;
    sortByNameAction->setChecked(sortBy == QDir::Name);
    sortBySizeAction->setChecked(sortBy == QDir::Size);
    sortByDateAction->setChecked(sortBy == QDir::Time);
    unsortedAction->setChecked(sortBy == QDir::Unsorted);
    model->setSorting(sort);
}

void QFileDialogPrivate::setDirFilter(QDir::Filters filters)
{
    showHiddenAction->setChecked(filters & QDir::Hidden);
    model->setFilter(filters);
}

QDir::Filters QFileDialogPrivate::filterForMode(QFileDialog::FileMode mode)
{
    if (mode == QFileDialog::DirectoryOnly)
        return QDir::Filters(QDir::AllDirs|QDir::Drives|QDir::Dirs);
    return QDir::Filters(QDir::AllDirs|QDir::Drives|QDir::All);
}

QAbstractItemView::SelectionMode QFileDialogPrivate::selectionMode(QFileDialog::FileMode mode)
{
    if (mode == QFileDialog::ExistingFiles)
        return QAbstractItemView::ExtendedSelection;
    return QAbstractItemView::SingleSelection;
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
                    this,
                    "Choose a file to open",
                    "/home",
                    "Images (*.png *.xpm *.jpg)");
  \endcode

  The function creates a modal file dialog with the given \a parent widget.
  If the parent is not 0, the dialog will be shown centered over the
  parent widget.

  The file dialog's working directory will be set to \a dir.
  If \a dir includes a file name, the file will be selected. Only files
  that match the given \a filter are shown. The filter selected is set to
  \a selectedFilter. The parameters \a dir, \a selectedFilter, and
  \a filter may be empty strings. The \a options argument can be used
  to tell the dialog not to resolve symbolic links
  (\c{DontResolveSymlinks}), and not to show files (\c{ShowDirsOnly}).

  The dialog's caption is set to \a caption. If \a caption is not
  specified then a default caption will be used.

  Under Windows and Mac OS X, this static function will use the native
  file dialog and not a QFileDialog, unless the style of the application
  is set to something other than the native style.

  Note that on Windows the dialog will spin a blocking modal event loop
  that will not dispatch any QTimers, and if parent is not 0 then it will
  position the dialog just under the parent's title bar.

  Under Unix/X11, the normal behavior of the file dialog is to resolve
  and follow symlinks. For example, if \c{/usr/tmp} is a symlink to
  \c{/var/tmp}, the file dialog will change to \c{/var/tmp} after
  entering \c{/usr/tmp}. If \a options includes DontResolveSymlinks,
  the file dialog will treat symlinks as regular directories.

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
    if (::qt_cast<QWindowsStyle*>(qApp->style()))
        return qt_win_get_open_file_name(initialSelection, filter, &qt_working_dir,
                                         parent, caption, selectedFilter);
#elif defined(Q_WS_MAC)
    if (::qt_cast<QMacStyle*>(qApp->style())) {
        QStringList files = qt_mac_get_open_file_names(filter, &qt_working_dir, parent,
                                                       caption, selectedFilter, false, false);
        return files.isEmpty() ? QString::null : files.first();
    }
#endif

    // create a qt dialog
    QFileDialog *dlg = new QFileDialog(parent,
                                       caption.isEmpty() ? "Open" : caption,
                                       qt_working_dir,
                                       filter,
                                       selectedFilter ? *selectedFilter : QString::null,
                                       initialSelection,
                                       QFileDialog::ExistingFile);
    dlg->setModal(true);

    QString result;
    if (dlg->exec() == QDialog::Accepted) {
        QStringList selected = dlg->selectedFiles();
        if (selected.count() > 0)
            result = selected.first();
        if (selectedFilter)
            *selectedFilter = dlg->selectedFilter();
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
                    this,
                    "Choose a filename to save under",
                    "/home",
                    "Images (*.png *.xpm *.jpg)");
  \endcode

  The file dialog's working directory will be set to \a dir. If \a
  dir includes a file name, the file will be selected. Only files that
  match the \a filter are shown. The filter selected is set to
  \a selectedFilter. The parameters \a dir, \a selectedFilter, and
  \a filter may be empty strings. The \a options argument can be used
  to tell the dialog not to resolve symbolic links
  (\c{DontResolveSymlinks}), and not to show files (\c{ShowDirsOnly}).

  The dialog's caption is set to \a caption. If \a caption is not
  specified then a default caption will be used.

  Under Windows and Mac OS X, this static function will use the native
  file dialog and not a QFileDialog, unless the style of the application
  is set to something other than the native style.

  Note that on Windows the dialog will spin a blocking modal event loop
  that will not dispatch any QTimers, and if parent is not 0 then it will
  position the dialog just under the parent's title bar.
  On Mac OS X, the filter argument is ignored.

  Under Unix/X11, the normal behavior of the file dialog is to resolve
  and follow symlinks. For example, if \c{/usr/tmp} is a symlink to
  \c{/var/tmp}, the file dialog will change to \c{/var/tmp} after
  entering \c{/usr/tmp}. If \a options includes DontResolveSymlinks,
  the file dialog will treat symlinks as regular directories.


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
    if (::qt_cast<QWindowsStyle*>(qApp->style()))
        return qt_win_get_save_file_name(initialSelection, filter, &qt_working_dir,
					 parent, caption, selectedFilter);
#elif defined(Q_WS_MAC)
    if (::qt_cast<QMacStyle*>(qApp->style()))
        return qt_mac_get_save_file_name(initialSelection, filter, &qt_working_dir,
                                         parent, caption, selectedFilter);
#endif

    QFileDialog *dlg = new QFileDialog(parent,
                                       caption.isEmpty() ? "Save As" : caption,
                                       qt_working_dir,
                                       filter,
                                       selectedFilter ? *selectedFilter : QString::null,
                                       initialSelection,
                                       QFileDialog::AnyFile);
    dlg->setModal(true);
    dlg->setAcceptMode(QFileDialog::AcceptSave);

    QString result;
    if (dlg->exec() == QDialog::Accepted) {
        QStringList selected = dlg->selectedFiles();
        if (selected.count() > 0)
            result = selected.first();
        if (selectedFilter)
            *selectedFilter = dlg->selectedFilter();
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
                    this,
                    "Choose a directory",
                    "/home",
                    DontResolveSymlinks);
  \endcode

  This function creates a modal file dialog with the given \a parent
  widget. If the parent is not 0, the dialog will be shown centered over
  the parent widget.

  The dialog's working directory is set to \a dir, and the caption is
  set to \a caption. Either of these may be an empty string in which case
  the current directory and a default caption will be used
  respectively. The \a options argument can be used to tell the dialog
  not to resolve symbolic links (\c{DontResolveSymlinks}), and not to
  show files (\c{ShowDirsOnly}).

  Under Unix/X11, the normal behavior of the file dialog is to resolve
  and follow symlinks. For example, if \c{/usr/tmp} is a symlink to
  \c{/var/tmp}, the file dialog will change to \c{/var/tmp} after
  entering \c{/usr/tmp}. If \a options includes DontResolveSymlinks,
  the file dialog will treat symlinks as regular directories.

  Note that on Windows the dialog will spin a blocking modal event loop
  that will not dispatch any QTimers, and if parent is not 0 then it will
  position the dialog just under the parent's title bar.

  \sa getOpenFileName(), getOpenFileNames(), getSaveFileName()
*/

QString QFileDialog::getExistingDirectory(QWidget *parent,
                                          const QString &caption,
                                          const QString &dir,
                                          Options options)
{
    bool save_qt_resolve_symlinks = qt_resolve_symlinks;
    qt_resolve_symlinks = !(options & DontResolveSymlinks);

#if defined(Q_WS_WIN)
    QString initialDir;
    if (!dir.isEmpty() && QFileInfo(dir).isDir())
        initialDir = dir;
    if (qt_cast<QWindowsStyle *>(qApp->style()) && (options & ShowDirsOnly))
        return qt_win_get_existing_directory(initialDir, parent, caption);
#elif defined(Q_WS_MAC)
    if (::qt_cast<QMacStyle*>(qApp->style())) {
        QStringList files = qt_mac_get_open_file_names("", 0, parent, caption, 0, false, true);
        return files.isEmpty() ? QString::null : files.first();
    }
#endif

    qt_get_dir_and_selection(dir, &qt_working_dir, 0);

    QFileDialog *dlg = new QFileDialog(parent,
                                       caption.isEmpty() ? "Find Directory" : caption,
                                       qt_working_dir,
                                       QString::null,
                                       QString::null,
                                       QString::null,
                                       options & ShowDirsOnly ? DirectoryOnly : Directory);
    dlg->setModal(true);
//     dlg->d->fileType->clear();
//     dlg->d->fileType->insertItem(QFileDialog::tr("Directories"));
//     dlg->d->fileType->setEnabled(false);

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
                            this,
                            "Select one or more files to open",
                            "/home",
                            "Images (*.png *.xpm *.jpg)");
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

  Under Unix/X11, the normal behavior of the file dialog is to resolve
  and follow symlinks. For example, if \c{/usr/tmp} is a symlink to
  \c{/var/tmp}, the file dialog will change to \c{/var/tmp} after
  entering \c{/usr/tmp}. If \a options includes DontResolveSymlinks,
  the file dialog will treat symlinks as regular directories.

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
    if (::qt_cast<QWindowsStyle*>(qApp->style()))
        return qt_win_get_open_file_names(filter, &qt_working_dir, parent, caption, selectedFilter);
#elif defined(Q_WS_MAC)
    if (::qt_cast<QMacStyle*>(qApp->style()))
        return qt_mac_get_open_file_names(filter, &qt_working_dir, parent, caption, selectedFilter, true, false);
#endif

    QFileDialog *dlg = new QFileDialog(parent,
                                       caption.isEmpty() ? "Open" : caption,
                                       qt_working_dir,
                                       filter,
                                       selectedFilter ? *selectedFilter : QString::null,
                                       QString::null,
                                       QFileDialog::ExistingFiles);
    dlg->setModal(true);

    QStringList lst;
    if (dlg->exec() == QDialog::Accepted) {
        lst = dlg->selectedFiles();
        if (selectedFilter)
            *selectedFilter = dlg->selectedFilter();
        qt_working_dir = dlg->directory().absolutePath();
    }
    delete dlg;

    qt_resolve_symlinks = save_qt_resolve_symlinks;

    return lst;
}


#ifdef QT_COMPAT
/*!
    Use selectedFiles() instead.

    \oldcode
    QString selected = dialog->selectedFile();
    \newcode
    QStringList files = dialog->selectedFiles();
    QString selected;
    if (files.count())
        selected = files[0];
    \endcode
*/
QString QFileDialog::selectedFile() const
{
    QStringList files = selectedFiles();
    return files.size() ? files.at(0) : QString();
}
#endif

/*!
    \fn void QFileDialog::setMode(FileMode m)

    Use setFileMode() instead.
*/

/*!
    \fn FileMode QFileDialog::mode() const

    Use fileMode() instead.
*/

/*!
    \fn void QFileDialog::setDir(const QString &directory)

    Use setDirectory() instead.
*/

/*!
    \fn void QFileDialog::setDir( const QDir &directory )

    Use setDirectory() instead.
*/

