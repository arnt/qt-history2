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
#include <private/qunicodetables_p.h>
#include <qmacstyle_mac.h>
#endif
#include <qdebug.h>

#include <private/qdialog_p.h>

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
        // FIXME: this is a hack to avoid propagating key press events
        // to the dialog and from there to the "Ok" button
        if (key != Qt::Key_Escape)
            e->accept();
    }
private:
    int key;
};

class QFileDialogPrivate : public QDialogPrivate
{
    Q_DECLARE_PUBLIC(QFileDialog)
public:
    QFileDialogPrivate();

    // private slots
    void backClicked();
    void upClicked();
    void mkdirClicked();
    void showListClicked();
    void showDetailClicked();
    void enterSubdir(const QModelIndex &index);
    void keyPressed(const QModelIndex &index, Qt::Key key, Qt::KeyboardModifiers modifiers);
    void deletePressed(const QModelIndex &index);
    void selectionChanged(const QItemSelection &selection);
    void fileNameChanged(const QString &text);
    void lookInChanged(const QString &text);
    void useFilter(const QString &filter);
    void setCurrentDir(const QString &path);
    void populateContextMenu(QMenu *menu, const QModelIndex &index);
    void renameCurrent();
    void deleteCurrent();
    void reload();
    void lookInReturnPressed();
    void sortByName();
    void sortBySize();
    void sortByDate();
    void setUnsorted();
    void showHidden();

    // setup
    void setup(const QString &directory, const QStringList &nameFilter);
    void setupActions();
    void setupListView(const QModelIndex &index, QGridLayout *grid);
    void setupTreeView(const QModelIndex &index, QGridLayout *grid);
    void setupToolButtons(const QModelIndex &index, QGridLayout *grid);
    void setupWidgets(QGridLayout *grid);

    // other
    void updateButtons(const QModelIndex &index);
    void setRoot(const QModelIndex &index);
    QModelIndex root() const;
    void setDirSorting(QDir::SortFlags sort);
    void setDirFilter(QDir::Filters filter);
    QDir::Filters filterForMode(QFileDialog::FileMode mode);
    QAbstractItemView::SelectionMode selectionMode(QFileDialog::FileMode mode);
    QModelIndex matchDir(const QString &text, const QModelIndex &first) const;
    QModelIndex matchName(const QString &name, const QModelIndex &first) const;

    // inlined stuff
    inline QString tr(const char *text) const { return QObject::tr(text); }
    inline QString toNative(const QString  &path) const
        { return QDir::convertSeparators(path); }
    inline QString toInternal(const QString &path) const
        {
#if defined(Q_FS_FAT) || defined(Q_OS_OS2EMX)
            QString n(path);
            for (int i = 0; i < (int)n.length(); ++i)
                if (n[i] == '\\') n[i] = '/';
            return n;
#else // the compile should optimize away this
            return path;
#endif
        }

    // data
    QDirModel *model;
    QItemSelectionModel *selections;
    QListView *listView;
    QTreeView *treeView;
    QFileDialog::ViewMode viewMode;
    QFileDialog::FileMode fileMode;
    QFileDialog::AcceptMode acceptMode;

    QList<QPersistentModelIndex> history;

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

    \value AcceptOpen
    \value AcceptSave
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
    QModelIndex index;
    QString text = filename;
    if (QFileInfo(filename).isAbsolute()) {
        index = d->model->index(filename);
        QString current = d->model->path(d->root());
        text.remove(current);
    } else { // faster than asking for model()->index(currentPath + filename)
        QStringList entries = directory().entryList(d->model->filter(), d->model->sorting());
        int r = entries.indexOf(filename);
        index = (r >= 0 ? d->model->index(r, 0, d->root()) : QModelIndex());
    }
    if (index.isValid()) {
        d->selections->select(index, QItemSelectionModel::Select);
    } else {
        d->selections->clear();
        d->fileName->setText(text);
    }
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
        QModelIndex index = indexes.at(i);
        if (index.row() != r && index.column() == 0) {
            files.append(d->model->path(index));
            r = index.row();
        }
    }

    QStringList fileNames = d->fileName->text().split(' ', QString::SkipEmptyParts);
    for (int j = 0; j < fileNames.count(); ++j) {
        QString name = d->toInternal(fileNames.at(j));
        if ((d->fileMode == AnyFile && files.count() <= 0)
            ||(d->fileMode == ExistingFile && QFileInfo(name).exists())) { // a new filename
            if (QFileInfo(name).isAbsolute())
                files.append(name);
            else
                files.append(d->toInternal(d->lookIn->currentText() + QDir::separator() + name));
        }
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
    d->useFilter(filter);
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
    d->useFilter(filters.first());
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
        d->showDetailClicked();
    else
        d->showListClicked();
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
    d->listView->setSelectionMode(selectionMode);
    d->treeView->setSelectionMode(selectionMode);
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
  \property QFileDialog::readOnly
  \brief Wether the filedialog is readonly.

  If this property is set to false, the filedialog will allow creating, renaming, copying
  and deleting files and directories.
*/

void QFileDialog::setReadOnly(bool enabled)
{
    d->model->setReadOnly(enabled);
}

bool QFileDialog::isReadOnly() const
{
    return d->model->isReadOnly();
}

/*
  \property QFileDialog::resolveSymlinks
  \brief Whether the filedialog should resolve symbolic links.

  If this property is set to true, the filedialog will resolve symbolic
  links.
*/
void QFileDialog::setResolveSymlinks(bool enabled)
{
    d->model->setResolveSymlinks(enabled);
}

bool QFileDialog::resolveSymlinks() const
{
    return d->model->resolveSymlinks();
}

/*!
  \brief set the item delegate used to render the items in the views in the filedialog.
*/
void QFileDialog::setItemDelegate(QAbstractItemDelegate *delegate)
{
    d->listView->setItemDelegate(delegate);
    d->treeView->setItemDelegate(delegate);
}

/*!
  \brief returns the item delegate used to render the items in the views in the filedialog.
*/
QAbstractItemDelegate *QFileDialog::itemDelegate() const
{
    return d->listView->itemDelegate();
}

/*!
  \brief set the icon provider used by the filedialog.
*/
void QFileDialog::setIconProvider(QFileIconProvider *provider)
{
    d->model->setIconProvider(provider);
}

/*!
  \brief returns the icon provider used by the filedialog.
*/
QFileIconProvider *QFileDialog::iconProvider() const
{
    return d->model->iconProvider();
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

    // special case for ".."
    if (fn == "..") {
        d->upClicked();
        bool block = d->fileName->blockSignals(true);
        d->fileName->setText(fn);
        d->fileName->selectAll();
        d->fileName->blockSignals(block);
        return;
    }

    // if we have no selected items, use the name in the lineedit
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
        if (!info.exists() || acceptMode() == AcceptOpen)
            QDialog::accept();
        else if (QMessageBox::warning(this, windowTitle(),
                                      fn + tr(" already exists.\n"
                                              "Do you want to replace it?"),
                                      QMessageBox::Yes,
                                      QMessageBox::No) == QMessageBox::Yes)
            QDialog::accept();
        return;}
    case ExistingFile:
    case ExistingFiles:
        for (int i = 0; i < files.count(); ++i) {
            QFileInfo info(files.at(i));
            if (!info.exists()) {
                QString message = tr("\nFile not found.\nPlease verify the "
                                     "correct file name was given");
                QMessageBox::warning(this, d->acceptButton->text(),
                                     info.fileName() + message);
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

  Private object  constructor.
*/

QFileDialogPrivate::QFileDialogPrivate()
    : QDialogPrivate(),
      model(0),
      listView(0),
      treeView(0),
      viewMode(QFileDialog::Detail),
      fileMode(QFileDialog::AnyFile),
      acceptMode(QFileDialog::AcceptOpen),
      lookIn(0),
      fileName(0),
      fileType(0),
      openAction(0),
      renameAction(0),
      deleteAction(0),
      reloadAction(0),
      sortByNameAction(0),
      sortBySizeAction(0),
      sortByDateAction(0),
      unsortedAction(0),
      showHiddenAction(0),
      acceptButton(0),
      cancelButton(0),
      back(0),
      toParent(0),
      newFolder(0),
      detailMode(0),
      listMode(0)
{

}

/*!
    \internal

    Navigates to the last directory viewed in the dialog.
*/

void QFileDialogPrivate::backClicked()
{
    QModelIndex root = history.back();
    history.pop_back();
    setRoot(root);
    updateButtons(root);
}

/*!
    \internal

    Navigates to the parent directory of the currently displayed directory
    in the dialog.
*/

void QFileDialogPrivate::upClicked()
{
    QModelIndex index = root();
    if (!index.isValid())
        return;
    history.push_back(index);
    QModelIndex parent = model->parent(index);
    setRoot(parent);
    updateButtons(parent);
}

/*!
    \internal

    Creates a new directory, first asking the user for a suitable name.
*/

void QFileDialogPrivate::mkdirClicked()
{
    QModelIndex parent = root();
    listView->clearSelection();

    QModelIndex index = model->mkdir(parent, "New Folder");
    if (!index.isValid())
        return;
    if (!index.isValid()) {
        selections->setCurrentIndex(model->index(0, 0, root()),
                                    QItemSelectionModel::SelectCurrent);
        return;
    }
    selections->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
    if (listMode->isDown())
        listView->edit(index);
    else
        treeView->edit(index);
}

/*!
    \internal

    Displays the contents of the current directory in the form of a list of
    icons and names.

    \sa ViewMode
*/

void QFileDialogPrivate::showListClicked()
{
    listMode->setDown(true);
    detailMode->setDown(false);
    listView->show();
    listView->doItemsLayout();
    treeView->hide();
}

/*!
    \internal

    Displays the contents of the current directory in the form of a list of
    icons and names, alongside which are displayed further details of each
    item.

    \sa ViewMode*/

void QFileDialogPrivate::showDetailClicked()
{
    listMode->setDown(false);
    detailMode->setDown(true);
    treeView->show();
    treeView->doItemsLayout();
    listView->hide();
}

/*!
    \internal

    This is called when the user double clicks on a file with the corresponding
    model item \a index.
*/

void QFileDialogPrivate::enterSubdir(const QModelIndex &index)
{
    if (model->isDir(index)) {
        history.push_back(root());
        setRoot(index);
        updateButtons(index);
    } else {
        q->accept();
    }
}

/*!
    \internal

    This is called when the user presses a \a key with the
    \a modifiers down, when the current item is \a index.
*/

void QFileDialogPrivate::keyPressed(const QModelIndex &index,
                                    Qt::Key key,
                                    Qt::KeyboardModifiers)
{
    switch (key) {
    case Qt::Key_Delete:
        deletePressed(index);
        return;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        enterSubdir(index);
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

void QFileDialogPrivate::deletePressed(const QModelIndex &index)
{
    if (model->isReadOnly())
        return;
    if (model->isDir(index))
        model->rmdir(index);
    else
        model->remove(index);
}

/*!
    \internal

    This is called when the model index corresponding to the current file is changed
    from \a index to \a current.
*/

void QFileDialogPrivate::selectionChanged(const QItemSelection &selection)
{
    Q_UNUSED(selection);

    if (fileName->hasFocus())
        return; // the selection changed because of autocompletion

    QString text;
    QModelIndexList indexes = selections->selectedIndexes();
    if (indexes.count() && indexes.at(0).column() == 0)
        text.append(model->data(indexes.at(0)).toString());
    for (int i = 1; i < indexes.count(); ++i) {
        QModelIndex index = indexes.at(i);
        if (index.column() == 0) {
            text.append(" ");
            text.append(model->data(index).toString());
        }
    }
    fileName->setText(text);
}

/*!
    \internal

    This is called when the text in the filename lineedit is changed; the new text
    is passed as \a text. This function provides autocompletion for filenames.
*/

void QFileDialogPrivate::fileNameChanged(const QString &text)
{
    QFileInfo info(toInternal(text));
    // the user is not typing  or the text is a valid file, there is no need for autocompletion
    if (!fileName->hasFocus() || info.exists())
        return;
    // if we hanve no filename or the last character is '/', then don't autocomplete
    if (text.isEmpty() || text[text.length() - 1] == QDir::separator())
        return;
    // if the user is removing text, don't autocomplete
    int key = fileName->lastKeyPressed();
    if (key == Qt::Key_Delete || key == Qt::Key_Backspace)
        return;
    // do autocompletion
    QModelIndex first;
    if (info.isAbsolute()) // if we have an absolute path, do completion in that directory
        first = model->index(0, 0, model->index(info.path()));
    else // otherwise, do completion from the currently selected file
        first = selections->currentIndex(); // optimization to we don't search from start
    if (!first.isValid())
        first = model->index(0, 0, d->root());
    QModelIndex result = matchName(info.fileName(), first);
    // did we find a valid autocompletion ?
    if (result.isValid()) {
        selections->setCurrentIndex(result, QItemSelectionModel::SelectCurrent);
        QString completed = model->data(result).toString();
        if (info.isAbsolute()) { // if we are doing completion in another directory, add the path first
            if (info.path() == "/")
                completed = "/" + completed;
            else
                completed = info.path() + "/" + completed;
        }
        int start = completed.length();
        int length = text.length() - start; // negative length
        bool block = d->fileName->blockSignals(true);
        fileName->setText(d->toNative(completed));
        fileName->setSelection(start, length);
        fileName->blockSignals(block);
    } else { // no matches
        selections->clear();
    }
}

/*!
    \internal

    This is called when the user changes the text in the "Look in"
    combobox; the new text is passed in \a text. The file dialog updates accordingly.
*/

void QFileDialogPrivate::lookInChanged(const QString &text)
{
    // if the user is not typing or the text is a valid path, there is no need for autocompletion
    if (!lookInEdit->hasFocus() || QFileInfo(toInternal(text)).exists())
        return;
    // if we hanve no path or the last character is '/', then don't autocomplete
    if (text.isEmpty() || text[text.length() - 1] == QDir::separator())
        return;
    // if the user is removing text, don't autocomplete
    int key = lookInEdit->lastKeyPressed();
    if (key == Qt::Key_Delete || key == Qt::Key_Backspace)
        return;
    // do autocompletion; text is the local path format (on windows separator is '\\')
    QString path = text.left(text.lastIndexOf(QDir::separator()));
    QString name = text.section(QDir::separator(), -1);
    QModelIndex parent = model->index(toInternal(path));
    QModelIndex result = matchDir(name, model->index(0, 0, parent));
    // did we find a valid autocompletion ?
    if (result.isValid()) {
        QString completed = toNative(d->model->path(result));
        int start = completed.length();
        int length = text.length() - start; // negative length
        bool block = lookInEdit->blockSignals(true);
        lookInEdit->setText(completed);
        lookInEdit->setSelection(start, length);
        lookInEdit->blockSignals(block);
    }
}

/*!
    \internal

    Adds the given \a filter to the file dialog's name filter.
    (Actually the name filters are held in the underlying model and
    this is what is really updated.)
*/

void QFileDialogPrivate::useFilter(const QString &filter)
{
    QStringList filters = qt_clean_filter_list(filter);
    model->setNameFilters(filters);
    // FIXME: workaroud for problem in rowsRemoved()/rowsInserted()
    listView->doItemsLayout();
    treeView->doItemsLayout();
}

/*!
    \internal

    Changes the file dialog's current directory to the one specified
    by \a path.
*/

void QFileDialogPrivate::setCurrentDir(const QString &path)
{
    history.push_back(root());
    QModelIndex index = model->index(path);
    setRoot(index);
    updateButtons(index);
}

/*!
    \internal

    This creates the default context menu for the file list. The
    context menu is passed in \a menu and the index into the
    underlying model in \a index.
*/

void QFileDialogPrivate::populateContextMenu(QMenu *menu, const QModelIndex &index)
{
    if (index.isValid()) {
        // file context menu
        menu->addAction(openAction);
        menu->addSeparator();
        menu->addAction(renameAction);
        menu->addAction(deleteAction);
        renameAction->setEnabled(!model->isReadOnly());
        deleteAction->setEnabled(!model->isReadOnly());
    } else {
        // view context menu
        menu->addAction(reloadAction);
        QMenu *sort = new QMenu(tr("Sort"));
        menu->addMenu(sort);
        sort->addAction(sortByNameAction);
        sort->addAction(sortBySizeAction);
        sort->addAction(sortByDateAction);
        sort->addSeparator();
        sort->addAction(unsortedAction);
        menu->addSeparator();
        menu->addAction(showHiddenAction);
    }
}

/*!
    Tells the dialog to rename the currently selected item using input from
    the user.
*/

void QFileDialogPrivate::renameCurrent()
{
    if (listMode->isDown())
        listView->edit(selections->currentIndex());
    else
        treeView->edit(selections->currentIndex());
}

/*!
    \internal

    Deletes the currently selected item in the dialog.
*/

void QFileDialogPrivate::deleteCurrent()
{
    deletePressed(selections->currentIndex());
}

/*!
    \internal

    Refreshes the display of the current directory in the dialog.
*/

void QFileDialogPrivate::reload()
{
    model->refresh(root());
}

/*!
    \internal

    Sets the current directory to the path showed in the "look in" combobox.
    Called when the enterPressed() signal is emitted.
*/

void QFileDialogPrivate::lookInReturnPressed()
{
    QString path = toInternal(lookIn->currentText());
    QModelIndex index = model->index(path);
    setRoot(index);
    updateButtons(index);
}

/*!
    \internal

    Sorts the items in the dialog by name order.
*/

void QFileDialogPrivate::sortByName()
{
    QDir::SortFlags sort = QDir::SortFlags(QDir::Name|QDir::DirsFirst);
    if (model->filter() & QDir::Reversed)
        sort |= QDir::Reversed;
    setDirSorting(sort);
}

/*!
    \internal

    Sorts the items in the dialog by size order.
*/

void QFileDialogPrivate::sortBySize()
{
    QDir::SortFlags sort = QDir::SortFlags(QDir::Size|QDir::DirsFirst);
    if(model->filter() & QDir::Reversed)
        sort |= QDir::Reversed;
    setDirSorting(sort);
}

/*!
    \internal

    Sorts the items in the dialog by date order.
*/

void QFileDialogPrivate::sortByDate()
{
    QDir::SortFlags sort = QDir::SortFlags(QDir::Time|QDir::DirsFirst);
    if(model->filter() & QDir::Reversed)
        sort |= QDir::Reversed;
    setDirSorting(sort);
}

/*!
    \internal

    Displays the contents of the current directory in an arbitrary order.

    \sa sortByDate() sortByName() sortBySize()
*/

void QFileDialogPrivate::setUnsorted()
{
    QDir::SortFlags sort = QDir::SortFlags(QDir::Unsorted|QDir::DirsFirst);
    if(model->filter() & QDir::Reversed)
        sort |= QDir::Reversed;
    setDirSorting(sort);
}

/*!
   \internal

   Includes hidden files and directories in the items displayed in the dialog.
*/

void QFileDialogPrivate::showHidden()
{
    QDir::Filters filters = model->filter();
    if (showHiddenAction->isChecked())
        filters |= QDir::Hidden;
    else
        filters &= ~(int)QDir::Hidden;
    setDirFilter(filters);
}

void QFileDialogPrivate::setup(const QString &directory,
                               const QStringList &nameFilter)
{
    q->setSizeGripEnabled(true);
    QGridLayout *grid = new QGridLayout(q);
    grid->setMargin(11);
    grid->setSpacing(6);

    // QDirModel
    QDir::Filters filters = filterForMode(fileMode);
    QDir::SortFlags sort = QDir::SortFlags(QDir::Name|QDir::IgnoreCase|QDir::DirsFirst);
    QStringList cleanedFilter = qt_clean_filter_list(nameFilter.first());
    model = new QDirModel(cleanedFilter, filters, sort, q);
    model->setReadOnly(false);

    // Selections
    selections = new QItemSelectionModel(model);
    QObject::connect(selections,
                     SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                     q, SLOT(selectionChanged(const QItemSelection&)));


    QModelIndex current = directory.isEmpty() ? QModelIndex() : model->index(directory);

    setupActions();
    setupListView(current, grid);
    setupTreeView(current, grid);
    setupToolButtons(current, grid);
    setupWidgets(grid);

    // Insert paths in the "lookin" combobox
    lookIn->insertItem(model->icon(QModelIndex()), model->name(QModelIndex())); // root
    for (int r = 0; r < model->rowCount(QModelIndex()); ++r) { // drives
        QModelIndex index = model->index(r, 0, QModelIndex());
        QString path = model->path(index);
        QIcon icons = model->icon(index);
        lookIn->insertItem(icons, toNative(path));
    }

    // insert the home path
    QModelIndex home = model->index(QDir::homePath()); // home
    lookIn->insertItem(model->icon(home), toNative(QDir::homePath()));

    // if it is not already in the list, insert the current directory
    QString currentPath = toNative(model->path(current));
    int item = lookIn->findItem(currentPath, QAbstractItemModel::MatchExactly);
    if (item < 0) {
        lookIn->insertItem(model->icon(current), currentPath);
        item = lookIn->findItem(currentPath, QAbstractItemModel::MatchExactly);
    }
    lookIn->setCurrentItem(item);

    // Set filetypes or filter
    if (fileMode == QFileDialog::DirectoryOnly) {
        fileType->insertItem(QFileDialog::tr("Directories"));
        fileType->setEnabled(false);
    } else {
        fileType->insertStringList(nameFilter);
    }

    // tab order
    QWidget::setTabOrder(lookIn, listView);
    QWidget::setTabOrder(listView, treeView);
    QWidget::setTabOrder(treeView, fileName);
    QWidget::setTabOrder(fileName, fileType);
    QWidget::setTabOrder(fileType, acceptButton);
    QWidget::setTabOrder(acceptButton, cancelButton);
    QWidget::setTabOrder(cancelButton, lookIn);

    // last init
    q->resize(530, 340);
    fileName->setFocus();
}

void QFileDialogPrivate::setupActions()
{
    openAction = new QAction(tr("&Open"), q);
    QObject::connect(openAction, SIGNAL(triggered()), q, SLOT(accept()));

    renameAction = new QAction(tr("&Rename"), q);
    QObject::connect(renameAction, SIGNAL(triggered()), q, SLOT(renameCurrent()));

    deleteAction = new QAction(tr("&Delete"), q);
    QObject::connect(deleteAction, SIGNAL(triggered()), q, SLOT(deleteCurrent()));

    reloadAction = new QAction(tr("&Reload"), q);
    QObject::connect(reloadAction, SIGNAL(triggered()), q, SLOT(reload()));

    sortByNameAction = new QAction(tr("Sort by &Name"), q);
    sortByNameAction->setCheckable(true);
    sortByNameAction->setChecked(true);
    QObject::connect(sortByNameAction, SIGNAL(triggered()), q, SLOT(sortByName()));

    sortBySizeAction = new QAction(tr("Sort by &Size"), q);
    sortBySizeAction->setCheckable(true);
    QObject::connect(sortBySizeAction, SIGNAL(triggered()), q, SLOT(sortBySize()));

    sortByDateAction = new QAction(tr("Sort by &Date"), q);
    sortByDateAction->setCheckable(true);
    QObject::connect(sortByDateAction, SIGNAL(triggered()), q, SLOT(sortByDate()));

    unsortedAction = new QAction(tr("&Unsorted"), q);
    unsortedAction->setCheckable(true);
    QObject::connect(unsortedAction, SIGNAL(triggered()), q, SLOT(setUnsorted()));

    showHiddenAction = new QAction(tr("Show &hidden files"), q);
    showHiddenAction->setCheckable(true);
    QObject::connect(showHiddenAction, SIGNAL(triggered()), q, SLOT(showHidden()));
}

void QFileDialogPrivate::setupListView(const QModelIndex &current, QGridLayout *grid)
{
    listView = new QListView(q);

    listView->setModel(model);
    listView->setSelectionModel(selections);
    listView->setSelectionMode(selectionMode(fileMode));
    listView->setSelectionBehavior(QAbstractItemView::SelectRows);
    listView->setRoot(current);
    listView->setKeyTracking(true);

    listView->setWrapping(true);
    listView->setResizeMode(QListView::Adjust);
    listView->setEditTriggers(QAbstractItemView::EditKeyPressed);

    grid->addWidget(listView, 1, 0, 1, 6);

    QObject::connect(listView, SIGNAL(aboutToShowContextMenu(QMenu*,QModelIndex)),
                     q, SLOT(populateContextMenu(QMenu*,QModelIndex)));
    QObject::connect(listView,
                     SIGNAL(doubleClicked(QModelIndex,Qt::MouseButton,Qt::KeyboardModifiers)),
                     q, SLOT(enterSubdir(QModelIndex)));
    QObject::connect(listView, SIGNAL(keyPressed(QModelIndex,Qt::Key,Qt::KeyboardModifiers)),
                     q, SLOT(keyPressed(QModelIndex,Qt::Key,Qt::KeyboardModifiers)));
}

void QFileDialogPrivate::setupTreeView(const QModelIndex &current, QGridLayout *grid)
{
    treeView = new QTreeView(q);

    treeView->setModel(model);
    treeView->setSelectionModel(selections);
    treeView->setSelectionMode(selectionMode(fileMode));
    treeView->setRoot(current);
    treeView->setKeyTracking(true);

    treeView->viewport()->setAcceptDrops(true);
    treeView->setRootIsDecorated(false);
    treeView->header()->setResizeMode(QHeaderView::Stretch, treeView->header()->count() - 1);
    treeView->header()->setSortIndicator(0, Qt::AscendingOrder);
    treeView->header()->setSortIndicatorShown(true);
    treeView->header()->setClickable(true);
    treeView->hide();
    treeView->setEditTriggers(QAbstractItemView::EditKeyPressed);
    treeView->resizeColumnToContents(0);

    grid->addWidget(treeView, 1, 0, 1, 6);

    QObject::connect(treeView, SIGNAL(aboutToShowContextMenu(QMenu*,QModelIndex)),
                     q, SLOT(populateContextMenu(QMenu*,QModelIndex)));
    QObject::connect(treeView,
                     SIGNAL(doubleClicked(QModelIndex,Qt::MouseButton,Qt::KeyboardModifiers)),
                     q, SLOT(enterSubdir(QModelIndex)));
    QObject::connect(treeView, SIGNAL(keyPressed(QModelIndex,Qt::Key,Qt::KeyboardModifiers)),
                     q, SLOT(keyPressed(QModelIndex,Qt::Key,Qt::KeyboardModifiers)));
}

void QFileDialogPrivate::setupToolButtons(const QModelIndex &current, QGridLayout *grid)
{
    QHBoxLayout *box = new QHBoxLayout;
    box->setMargin(3);
    box->setSpacing(3);
    QSize tools(22, 22);

    back = new QToolButton(q);
    back->setIcon(q->style()->standardPixmap(QStyle::SP_FileDialogBack));
    back->setToolTip(tr("Back"));
    back->setAutoRaise(true);
    back->setEnabled(false);
    back->setFixedSize(tools);
    QObject::connect(back, SIGNAL(clicked()), q, SLOT(backClicked()));
    box->addWidget(back);

    toParent = new QToolButton(q);
    toParent->setIcon(q->style()->standardPixmap(QStyle::SP_FileDialogToParent));
    toParent->setToolTip(tr("Parent Directory"));
    toParent->setAutoRaise(true);
    toParent->setEnabled(model->parent(current).isValid());
    toParent->setFixedSize(tools);
    QObject::connect(toParent, SIGNAL(clicked()), q, SLOT(upClicked()));
    box->addWidget(toParent);

    newFolder = new QToolButton(q);
    newFolder->setIcon(q->style()->standardPixmap(QStyle::SP_FileDialogNewFolder));
    newFolder->setToolTip(tr("Create New Folder"));
    newFolder->setAutoRaise(true);
    newFolder->setFixedSize(tools);
    QObject::connect(newFolder, SIGNAL(clicked()), q, SLOT(mkdirClicked()));
    box->addWidget(newFolder);

    listMode = new QToolButton(q);
    listMode->setIcon(q->style()->standardPixmap(QStyle::SP_FileDialogListView));
    listMode->setToolTip(tr("List View"));
    listMode->setAutoRaise(true);
    listMode->setDown(true);
    listMode->setFixedSize(tools);
    QObject::connect(listMode, SIGNAL(clicked()), q, SLOT(showListClicked()));
    box->addWidget(listMode);

    detailMode = new QToolButton(q);
    detailMode->setIcon(q->style()->standardPixmap(QStyle::SP_FileDialogDetailedView));
    detailMode->setToolTip(tr("Detail View"));
    detailMode->setAutoRaise(true);
    detailMode->setFixedSize(tools);
    QObject::connect(detailMode, SIGNAL(clicked()), q, SLOT(showDetailClicked()));
    box->addWidget(detailMode);
    box->setResizeMode(QLayout::Fixed);

    grid->addLayout(box, 0, 4, 1, 2);
}

void QFileDialogPrivate::setupWidgets(QGridLayout *grid)
{
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

    // "lookin" combobox
    lookIn = new QComboBox(q);
    lookIn->setInsertionPolicy(QComboBox::NoInsertion);
    lookIn->setDuplicatesEnabled(false);
    lookIn->setEditable(true);
    lookIn->setAutoCompletion(false);
    QObject::connect(lookIn, SIGNAL(activated(QString)),
                     q, SLOT(setCurrentDir(QString)));
    lookInEdit = new QFileDialogLineEdit(lookIn);
    QObject::connect(lookInEdit, SIGNAL(textChanged(QString)),
                     q, SLOT(lookInChanged(QString)));
    QObject::connect(lookInEdit, SIGNAL(returnPressed()), q, SLOT(lookInReturnPressed()));
    lookIn->setLineEdit(lookInEdit);
    lookIn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    grid->addWidget(d->lookIn, 0, 1, 1, 3);

    // filename
    fileName = new QFileDialogLineEdit(q);
    fileName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QObject::connect(fileName, SIGNAL(textChanged(QString)),
                     q, SLOT(fileNameChanged(QString)));
    QObject::connect(fileName, SIGNAL(returnPressed()), q, SLOT(accept()));
    grid->addWidget(fileName, 2, 1, 1, 3);

    // filetype
    fileType = new QComboBox(q);
    fileType->setDuplicatesEnabled(false);
    fileType->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QObject::connect(fileType, SIGNAL(activated(QString)),
                     q, SLOT(useFilter(QString)));
    grid->addWidget(fileType, 3, 1, 1, 3);
}

void QFileDialogPrivate::updateButtons(const QModelIndex &index)
{
    if (!index.isValid()) // "My Computer" is already in the list
        return;
    toParent->setEnabled(index.isValid());
    back->setEnabled(history.count() > 0);
    newFolder->setEnabled(!model->isReadOnly());
    QString pth = toNative(d->model->path(index));
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
    bool block = selections->blockSignals(true);
    selections->clear();
    fileName->clear();
    listView->setRoot(index);
    treeView->setRoot(index);
    treeView->resizeColumnToContents(0);
    selections->blockSignals(block);
    selections->setCurrentIndex(QModelIndex(), QItemSelectionModel::SelectCurrent);
}

QModelIndex QFileDialogPrivate::root() const
{
    return listView->root();
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
        return QDir::Filters(QDir::AllDirs|QDir::Drives|QDir::Dirs) & ~QDir::NoSymLinks;
    return QDir::Filters(QDir::AllDirs|QDir::Drives|QDir::TypeMask) & ~QDir::NoSymLinks;
}

QAbstractItemView::SelectionMode QFileDialogPrivate::selectionMode(QFileDialog::FileMode mode)
{
    if (mode == QFileDialog::ExistingFiles)
        return QAbstractItemView::ExtendedSelection;
    return QAbstractItemView::SingleSelection;
}

QModelIndex QFileDialogPrivate::matchDir(const QString &text, const QModelIndex &first) const
{
    QModelIndexList matches = model->match(first,
                                           QAbstractItemModel::DisplayRole,
                                           text, model->rowCount(first.parent()),
                                           QAbstractItemModel::MatchDefault
                                           |QAbstractItemModel::MatchCase);
    for (int i = 0; i < matches.count(); ++i)
        if (d->model->isDir(matches.at(i)))
            return matches.at(i);
    return QModelIndex();
}

QModelIndex QFileDialogPrivate::matchName(const QString &name, const QModelIndex &first) const
{
    QModelIndexList matches = model->match(first,
                                           QAbstractItemModel::DisplayRole,
                                           name, 1,
                                           QAbstractItemModel::MatchDefault
                                           |QAbstractItemModel::MatchCase);
    if (matches.count() <= 0)
        return QModelIndex();
    return matches.first();
}

/******************************************************************
 *
 * Static functions for the native filedialogs
 *
 ******************************************************************/

#include <qapplication.h>
#include <qstyle.h>

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
    QByteArray name = filename.toUtf8();
    static const QByteArray illegal("<>#@\"&%$:,;?={}|^~[]\'`\\*");

    int len = name.length();
    if (!len)
        return QString();
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
                if (sel) *sel = QString();
            } else {
                if (cwd) *cwd = info.absolutePath();
                if (sel) *sel = info.fileName();
            }
            return;
        }
    }
    if (cwd) *cwd = QDir::currentPath();
    if (sel) *sel = QString();
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
        return files.isEmpty() ? QString() : QUnicodeTables::normalize(files.first(),
                                                                       QString::NormalizationForm_C);
    }
#endif

    // create a qt dialog
    QFileDialog *dlg = new QFileDialog(parent,
                                       caption.isEmpty() ? "Open" : caption,
                                       qt_working_dir,
                                       filter,
                                       selectedFilter ? *selectedFilter : QString(),
                                       initialSelection,
                                       QFileDialog::ExistingFile);
    dlg->setModal(true);
    dlg->setResolveSymlinks(!(options & DontResolveSymlinks));

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
    QString initialSelection;
    qt_get_dir_and_selection(dir, &qt_working_dir, &initialSelection);

#if defined(Q_WS_WIN)
    if (::qt_cast<QWindowsStyle*>(qApp->style()))
        return qt_win_get_save_file_name(initialSelection, filter, &qt_working_dir,
					 parent, caption, selectedFilter);
#elif defined(Q_WS_MAC)
    if (::qt_cast<QMacStyle*>(qApp->style()))
        return QUnicodeTables::normalize(qt_mac_get_save_file_name(initialSelection, filter,
                                                                   &qt_working_dir, parent, caption,
                                                                   selectedFilter),
                                         QString::NormalizationForm_C);
#endif

    QFileDialog *dlg = new QFileDialog(parent,
                                       caption.isEmpty() ? "Save As" : caption,
                                       qt_working_dir,
                                       filter,
                                       selectedFilter ? *selectedFilter : QString(),
                                       initialSelection.isEmpty() ? dir : QString(),
                                       QFileDialog::AnyFile);
    dlg->setModal(true);
    dlg->setResolveSymlinks(!(options & DontResolveSymlinks));
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
#if defined(Q_WS_WIN)
    QString initialDir;
    if (!dir.isEmpty() && QFileInfo(dir).isDir())
        initialDir = dir;
    if (qt_cast<QWindowsStyle *>(qApp->style()) && (options & ShowDirsOnly))
        return qt_win_get_existing_directory(initialDir, parent, caption);
#elif defined(Q_WS_MAC)
    if (::qt_cast<QMacStyle*>(qApp->style())) {
        QStringList files = qt_mac_get_open_file_names("", 0, parent, caption, 0, false, true);
        return files.isEmpty() ? QString() : QUnicodeTables::normalize(files.first(),
                                                                       QString::NormalizationForm_C);
    }
#endif

    qt_get_dir_and_selection(dir, &qt_working_dir, 0);

    QFileDialog *dlg = new QFileDialog(parent,
                                       caption.isEmpty() ? "Find Directory" : caption,
                                       qt_working_dir,
                                       QString(),
                                       QString(),
                                       QString(),
                                       options & ShowDirsOnly ? DirectoryOnly : Directory);
    dlg->setModal(true);
    dlg->setResolveSymlinks(!(options & DontResolveSymlinks));

    QString result;
    if (dlg->exec() == QDialog::Accepted) {
        result = dlg->selectedFiles().first();
        qt_working_dir = result;
    }
    delete dlg;

    if (!result.isEmpty() && result.right(1) != "/")
        result += "/";

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
    qt_get_dir_and_selection(dir, &qt_working_dir, 0);

#if defined(Q_WS_WIN)
    if (::qt_cast<QWindowsStyle*>(qApp->style()))
        return qt_win_get_open_file_names(filter, &qt_working_dir, parent, caption, selectedFilter);
#elif defined(Q_WS_MAC)
    if (::qt_cast<QMacStyle*>(qApp->style())) {
        QStringList sl = qt_mac_get_open_file_names(filter, &qt_working_dir, parent, caption,
                                                    selectedFilter, true, false);
        for (int i = 0; i < sl.count(); ++i)
            sl.replace(i, QUnicodeTables::normalize(sl.at(i), QString::NormalizationForm_C));
        return sl;
    }
#endif

    QFileDialog *dlg = new QFileDialog(parent,
                                       caption.isEmpty() ? "Open" : caption,
                                       qt_working_dir,
                                       filter,
                                       selectedFilter ? *selectedFilter : QString(),
                                       QString(),
                                       QFileDialog::ExistingFiles);
    dlg->setModal(true);
    dlg->setResolveSymlinks(!(options & DontResolveSymlinks));

    QStringList lst;
    if (dlg->exec() == QDialog::Accepted) {
        lst = dlg->selectedFiles();
        if (selectedFilter)
            *selectedFilter = dlg->selectedFilter();
        qt_working_dir = dlg->directory().absolutePath();
    }
    delete dlg;

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

#include "moc_qfiledialog.cpp"
