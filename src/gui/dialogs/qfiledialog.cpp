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

#ifdef Q_WS_WIN
#include <qwindowsstyle.h>
#endif
#include <qshortcut.h>
#ifdef Q_WS_MAC
#include <private/qunicodetables_p.h>
#include <qmacstyle_mac.h>
#endif

#include <qdebug.h>
#include <private/qfiledialog_p.h>

#define d d_func()
#define q q_func()

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
    void keyPressEvent(QKeyEvent *e);
private:
    int key;
};

void QFileDialogLineEdit::keyPressEvent(QKeyEvent *e)
{
    key = e->key();
    QLineEdit::keyPressEvent(e);
    // FIXME: this is a hack to avoid propagating key press events
    // to the dialog and from there to the "Ok" button
    if (key != Qt::Key_Escape)
        e->accept();
}

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
  own file dialog is selectedFiles().

  \code
    QStringList fileNames;
    if (fileDialog->exec())
        fileNames = fileNames->selectedFiles();
  \endcode

  In the above example, a modal file dialog is created and shown. If
  the user clicked OK, the file they selected is put in \c fileName.

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

    \value ShowDirsOnly
    \value DontResolveSymlinks
    \value DontConfirmOverwrite
*/

/*!
  \enum QFileDialog::DialogLabel

  \value LookIn
  \value FileName
  \value FileType
  \value Accept
  \value Reject
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
    Constructs a file dialog with the given \a parent and \a caption that
    initially displays the contents of the specified \a directory.
    The contents of the directory are filtered before being shown in the
    dialog, using a semicolon-separated list of filters specified by
    \a filter.
*/
QFileDialog::QFileDialog(QWidget *parent,
                         const QString &caption,
                         const QString &directory,
                         const QString &filter)
    : QDialog(*new QFileDialogPrivate, parent, 0)
{
    setWindowTitle(caption);
    QStringList nameFilter = qt_make_filter_list(filter);
    if (nameFilter.isEmpty())
        d->setup(directory, QStringList(tr("All Files (*)")));
    else
        d->setup(directory, nameFilter);
}

/*!
    \internal
*/
QFileDialog::QFileDialog(const QFileDialogArgs &args)
    : QDialog(*new QFileDialogPrivate, args.parent, 0)
{
    d->fileMode = args.mode;
    d->confirmOverwrite = !(args.options & DontConfirmOverwrite);
    setWindowTitle(args.caption);
    QStringList nameFilter = qt_make_filter_list(args.filter);
    if (nameFilter.isEmpty())
        d->setup(args.directory, QStringList(tr("All Files (*)")));
    else
        d->setup(args.directory, nameFilter);
    setResolveSymlinks(!(args.options & DontResolveSymlinks));
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
    d->setRootIndex(index);
    d->updateButtons(index);
}

/*!
    Returns the directory currently being displayed in the dialog.
*/

QDir QFileDialog::directory() const
{
    QDir dir(d->model->filePath(d->rootIndex()));
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
        QString current = d->model->filePath(d->rootIndex());
        text.remove(current);
    } else { // faster than asking for model()->index(currentPath + filename)
        QStringList entries = directory().entryList(d->model->filter(), d->model->sorting());
        int r = entries.indexOf(filename);
        index = (r >= 0 ? d->model->index(r, 0, d->rootIndex()) : QModelIndex());
    }
    if (index.isValid()) {
        d->selections->select(index, QItemSelectionModel::Select);
    } else {
        d->selections->clear();
        d->fileNameEdit->setText(text);
    }
}

/*!
  Returns a list of strings containing the absolute paths of the
  selected files in the dialog. If no files are selected, or
  the mode is not ExistingFiles, selectedFiles() is an empty list.

  \sa selectedFilter, QValueList::empty()
*/

QStringList QFileDialog::selectedFiles() const
{
    QModelIndexList indexes = d->selections->selectedIndexes();
    QStringList files;
    int r = -1;
    for (int i = 0; i < indexes.count(); ++i) {
        QModelIndex index = indexes.at(i);
        if (index.row() != r && index.column() == 0) {
            files.append(d->model->filePath(index));
            r = index.row();
        }
    }

    QStringList fileNames = d->fileNameEdit->text().split(' ', QString::SkipEmptyParts);
    for (int j = 0; j < fileNames.count(); ++j) {
        QString name = d->toInternal(fileNames.at(j));
        QFileInfo info(name);
        // if the filename has no suffix, add the default suffix
        if (!d->defaultSuffix.isEmpty() && !info.isDir() && name.lastIndexOf('.') == -1)
            name += "." + d->defaultSuffix;
        // a new filename
        if ((d->fileMode == AnyFile && files.isEmpty())
            || (d->fileMode == ExistingFile && info.exists())) {
            if (info.isAbsolute())
                files.append(name);
            else
                files.append(d->toInternal(d->lookInCombo->currentText()
                                           + QDir::separator() + name));
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
    d->fileTypeCombo->clear();
    d->fileTypeCombo->addItem(filter);
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
    d->fileTypeCombo->clear();
    d->fileTypeCombo->addItems(filters);
    d->useFilter(filters.first());
}

/*!
    Returns the file type filters that are in operation on this file
    dialog.
*/

QStringList QFileDialog::filters() const
{
    QStringList items;
    for (int i = 0; i < d->fileTypeCombo->count(); ++i)
        items.append(d->fileTypeCombo->itemText(i));
    return items;
}

/*!
    Sets the current file type \a filter. Multiple filters can be
    passed in \a filter by separating them with semicolons or spaces.

    \sa setFilter() setFilters()
*/

void QFileDialog::selectFilter(const QString &filter)
{
    int i = d->fileTypeCombo->findText(filter);
    if (i >= 0)
        d->fileTypeCombo->setCurrentIndex(i);
}

/*!
  Returns the filter that the user selected in the file dialog.

  \sa filterSelected(), selectedFiles
*/

QString QFileDialog::selectedFilter() const
{
    return d->fileTypeCombo->currentText();
}

void QFileDialog::setViewMode(ViewMode mode)
{
    if (mode == Detail)
        d->showDetails();
    else
        d->showList();
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
    return d->viewMode();
}

void QFileDialog::setFileMode(FileMode mode)
{
    d->fileMode = mode;
    // set selection mode and behavior
    QAbstractItemView::SelectionMode selectionMode = d->selectionMode(mode);
    d->listView->setSelectionMode(selectionMode);
    d->listView->setSelectionBehavior(QAbstractItemView::SelectRows);
    d->treeView->setSelectionMode(selectionMode);
    d->treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    // set filter
    QDir::Filters filters = d->filterForMode(mode);
    d->model->setFilter(filters);
    // setup file type for directory
    if (mode == DirectoryOnly) {
        d->fileTypeCombo->clear();
        d->fileTypeCombo->addItem(QFileDialog::tr("Directories"));
        d->fileTypeCombo->setEnabled(false);
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

/*
  \property QFileDialog::confirmOverwrite
  \brief Whether the filedialog should ask before accepting a selected file,
  when the accept mode is AcceptSave.

  If this property is set to true and the accept mode is AcceptSave,
  the filedialog will ask whether the user wants to overwrite the fike before
  accepting the file.
*/
void QFileDialog::setConfirmOverwrite(bool enabled)
{
    d->confirmOverwrite = enabled;
}

bool QFileDialog::confirmOverwrite() const
{
    return d->confirmOverwrite;
}

/*
  \property QFileDialog::defaultSuffix
  \brief  Suffix added to the filename if no other suffix was specified.

  This property specifies a string that will be added to the filename if it has no suffix already.
  The suffix is typically used to indicate the file type (e.g. "txt" indicates a text file).
*/
void QFileDialog::setDefaultSuffix(const QString &suffix)
{
    d->defaultSuffix = suffix;
}

QString QFileDialog::defaultSuffix() const
{
    return d->defaultSuffix;
}

/*!
  \brief sets the browsing history of the filedialog to contain the given \a paths.
*/
void QFileDialog::setHistory(const QStringList &paths)
{
    QList<QPersistentModelIndex> history;
    QStringList::const_iterator it = paths.constBegin();
    for (; it != paths.constEnd(); ++it) {
        QModelIndex index = d->model->index(*it);
        if (index.isValid()) {
            history << QPersistentModelIndex(index);
            QIcon icn = d->model->fileIcon(index);
            d->lookInCombo->addItem(icn, *it);
        }
    }
    d->history = history;
    d->backButton->setEnabled(!history.isEmpty());
}

/*!
  \brief returns the browsing history of the filedialog as a list of paths.
*/
QStringList QFileDialog::history() const
{
    QStringList paths;
    QList<QPersistentModelIndex>::const_iterator it = d->history.constBegin();
    for (; it != d->history.constEnd(); ++it)
        paths << d->model->filePath(*it);
    return paths;
}

/*!
  \brief set the item delegate used to render the items in the views in the
  filedialog to the specified \a delegate
*/
void QFileDialog::setItemDelegate(QAbstractItemDelegate *delegate)
{
    d->listView->setItemDelegate(delegate);
    d->treeView->setItemDelegate(delegate);
}

/*!
  \brief returns the item delegate used to render the items in the views in
  the filedialog
*/
QAbstractItemDelegate *QFileDialog::itemDelegate() const
{
    return d->listView->itemDelegate();
}

/*!
  \brief set the icon provider used by the filedialog to the specified
  \a provider
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
  \brief set the \a text shown in the filedialog in the specified \a label
*/
void QFileDialog::setLabelText(DialogLabel label, const QString &text)
{
    switch (label) {
    case LookIn:
        d->lookInLabel->setText(text);
        break;
    case FileName:
        d->fileNameLabel->setText(text);
        break;
    case FileType:
        d->fileTypeLabel->setText(text);
        break;
    case Accept:
        d->acceptButton->setText(text);
        break;
    case Reject:
        d->rejectButton->setText(text);
        break;
    }
}

/*!
  \brief returns the text shown in the filedialog in the specified \a label
*/
QString QFileDialog::labelText(DialogLabel label) const
{
    switch (label) {
    case LookIn:
        return d->lookInLabel->text();
    case FileName:
        return d->fileNameLabel->text();
    case FileType:
        return d->fileTypeLabel->text();
    case Accept:
        return d->acceptButton->text();
    case Reject:
        return d->rejectButton->text();
    }
    return QString();
}

/*!
 \reimp
*/
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
    QString fn = d->fileNameEdit->text();

    // special case for ".."
    if (fn == "..") {
        d->navigateToParent();
        bool block = d->fileNameEdit->blockSignals(true);
        d->fileNameEdit->setText(fn);
        d->fileNameEdit->selectAll();
        d->fileNameEdit->blockSignals(block);
        return;
    }

    // if we have no selected items, use the name in the lineedit
    if (files.isEmpty())
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
        if (info.isDir()) {
            setDirectory(info.absoluteFilePath());
            return;
        }
        // check if we have to ask for permission to overwrite the file
        if (!info.exists() || !confirmOverwrite() || acceptMode() == AcceptOpen)
            QDialog::accept();
        else if (QMessageBox::warning(this, windowTitle(),
                                      info.fileName()
                                      + tr(" already exists.\nDo you want to replace it?"),
                                      QMessageBox::Yes, QMessageBox::No)
                 == QMessageBox::Yes)
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
      fileMode(QFileDialog::AnyFile),
      acceptMode(QFileDialog::AcceptOpen),
      confirmOverwrite(true),
      lookInCombo(0),
      fileNameEdit(0),
      fileTypeCombo(0),
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
      rejectButton(0),
      backButton(0),
      toParentButton(0),
      newFolderButton(0),
      detailModeButton(0),
      listModeButton(0),
      lookInLabel(0),
      fileNameLabel(0),
      fileTypeLabel(0)
{

}

/*!
    \internal

    Refreshes the display of the current directory in the dialog.
*/
void QFileDialogPrivate::reload()
{
    model->refresh(rootIndex());
}

/*!
    \internal

    Navigates to the last directory viewed in the dialog.
*/
void QFileDialogPrivate::navigateToPrevious()
{
    QModelIndex root = history.back();
    history.pop_back();
    setRootIndex(root);
    updateButtons(root);
}

/*!
    \internal

    Navigates to the parent directory of the currently displayed directory
    in the dialog.
*/

void QFileDialogPrivate::navigateToParent()
{
    QModelIndex index = rootIndex();
    history.push_back(index);
    QModelIndex parent = model->parent(index);
    setRootIndex(parent);
    updateButtons(parent);
}

/*!
    \internal

    This is called when the user double clicks on a file with the corresponding
    model item \a index.
*/

void QFileDialogPrivate::enterDirectory(const QModelIndex &index)
{
    // if it is "My Computer" or a directory, enter it
    if (!index.isValid() || d->model->isDir(index)) {
        history.push_back(rootIndex());
        setRootIndex(index);
        updateButtons(index);        
    } else {
        q->accept();
    }
}

/*!
    \internal

    Changes the file dialog's current directory to the one specified
    by \a path.
*/

void QFileDialogPrivate::enterDirectory(const QString &path)
{
    enterDirectory(model->index(path));
}

/*!
    \internal

    Sets the current directory to the path showed in the "look in" combobox.
    Called when the enterPressed() signal is emitted.
*/

void QFileDialogPrivate::enterDirectory()
{
    QString path = toInternal(lookInCombo->currentText());
    enterDirectory(path);
}

/*!
    \internal

    Displays the contents of the current directory in the form of a list of
    icons and names.

    \sa ViewMode
*/

void QFileDialogPrivate::showList()
{
    listModeButton->setDown(true);
    detailModeButton->setDown(false);
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

void QFileDialogPrivate::showDetails()
{
    listModeButton->setDown(false);
    detailModeButton->setDown(true);
    treeView->show();
    treeView->doItemsLayout();
    listView->hide();
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

    This is called when the model index corresponding to the current file is changed
    from \a index to \a current.
*/

void QFileDialogPrivate::updateFileName(const QItemSelection &selection)
{
    Q_UNUSED(selection);

    if (fileNameEdit->hasFocus())
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
    fileNameEdit->setText(text);
}

/*!
    \internal

    This is called when the text in the filename lineedit is changed; the new text
    is passed as \a text. This function provides autocompletion for filenames.
*/

void QFileDialogPrivate::autoCompleteFileName(const QString &text)
{
    QFileInfo info(toInternal(text));
    // the user is not typing  or the text is a valid file, there is no need for autocompletion
    if (!fileNameEdit->hasFocus() || info.exists())
        return;
    // if we hanve no filename or the last character is '/', then don't autocomplete
    if (text.isEmpty() || text[text.length() - 1] == QDir::separator())
        return;
    // if the user is removing text, don't autocomplete
    int key = fileNameEdit->lastKeyPressed();
    if (key == Qt::Key_Delete || key == Qt::Key_Backspace)
        return;
    // do autocompletion
    QModelIndex first;
    if (info.isAbsolute()) // if we have an absolute path, do completion in that directory
        first = model->index(0, 0, model->index(info.path()));
    else // otherwise, do completion from the currently selected file
        first = selections->currentIndex(); // optimization to we don't search from start
    if (!first.isValid())
        first = model->index(0, 0, d->rootIndex());
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
        bool block = d->fileNameEdit->blockSignals(true);
        fileNameEdit->setText(d->toNative(completed));
        fileNameEdit->setSelection(start, length);
        fileNameEdit->blockSignals(block);
    } else { // no matches
        selections->clear();
    }
}

/*!
    \internal

    This is called when the user changes the text in the "Look in"
    combobox; the new text is passed in \a text. The file dialog updates accordingly.
*/

void QFileDialogPrivate::autoCompleteDirectory(const QString &text)
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
        QString completed = toNative(d->model->filePath(result));
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

    This creates the default context menu for the file list.
*/

void QFileDialogPrivate::showContextMenu(const QPoint &pos)
{
    QAbstractItemView *view = 0;
    if (q->viewMode() == QFileDialog::Detail)
        view = treeView;
    else
        view = listView;
    QModelIndex index = view->indexAt(pos);
    QMenu menu(view);

    if (index.isValid()) {
        // file context menu
        menu.addAction(openAction);
        menu.addSeparator();
        menu.addAction(renameAction);
        menu.addAction(deleteAction);
        renameAction->setEnabled(!model->isReadOnly());
        deleteAction->setEnabled(!model->isReadOnly());
    } else {
        // view context menu
        menu.addAction(reloadAction);
        QMenu sort(tr("Sort"));
        menu.addMenu(&sort);
        sort.addAction(sortByNameAction);
        sort.addAction(sortBySizeAction);
        sort.addAction(sortByDateAction);
        sort.addSeparator();
        sort.addAction(unsortedAction);
        menu.addSeparator();
        menu.addAction(showHiddenAction);
    }

    menu.exec(view->mapToGlobal(pos));
}

/*!
    \internal

    Creates a new directory, first asking the user for a suitable name.
*/

void QFileDialogPrivate::createDirectory()
{
    QModelIndex parent = rootIndex();
    listView->clearSelection();

    QModelIndex index = model->mkdir(parent, "New Folder");
    if (!index.isValid())
        return;
    selections->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
    if (viewMode() == QFileDialog::List)
        listView->edit(index);
    else
        treeView->edit(index);
}

/*!
    Tells the dialog to rename the currently selected item using input from
    the user.
*/

void QFileDialogPrivate::renameCurrent()
{
    if (viewMode() == QFileDialog::List)
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
    // FIXME: should we delete all selected indexes ?
    QModelIndex index = selections->currentIndex();
    if(!index.isValid())
        return;
    if (model->isReadOnly())
        return;
    if (model->isDir(index))
        model->rmdir(index);
    else
        model->remove(index);
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

void QFileDialogPrivate::setup(const QString &directory, const QStringList &nameFilter)
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
    model->setLazyChildCount(true);

    // Selections
    selections = new QItemSelectionModel(model);
    QObject::connect(selections,
                     SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                     q, SLOT(updateFileName(const QItemSelection&)));


    QModelIndex current = directory.isEmpty() ? QModelIndex() : model->index(directory);

    setupActions();
    setupListView(current, grid);
    setupTreeView(current, grid);
    setupToolButtons(current, grid);
    setupWidgets(grid);

    // Insert paths in the "lookin" combobox
    lookInCombo->addItem(model->fileIcon(QModelIndex()), model->fileName(QModelIndex())); // root
    for (int r = 0; r < model->rowCount(QModelIndex()); ++r) { // drives
        QModelIndex index = model->index(r, 0, QModelIndex());
        QString path = model->filePath(index);
        Q_ASSERT(!path.isEmpty());
        QIcon icons = model->fileIcon(index);
        lookInCombo->addItem(icons, toNative(path));
    }

    // insert the home path
    QModelIndex home = model->index(QDir::homePath()); // home
    lookInCombo->addItem(model->fileIcon(home), toNative(QDir::homePath()));

    // if it is not already in the list, insert the current directory
    QString currentPath = toNative(model->filePath(current));
    int item = lookInCombo->findText(currentPath);
    if (item < 0 && currentPath.count()) {
        lookInCombo->addItem(model->fileIcon(current), currentPath);
        item = lookInCombo->findText(currentPath);
    }
    lookInCombo->setCurrentIndex(item);

    // Set filetypes or filter
    if (fileMode == QFileDialog::DirectoryOnly) {
        fileTypeCombo->addItem(QFileDialog::tr("Directories"));
        fileTypeCombo->setEnabled(false);
    } else {
        fileTypeCombo->addItems(nameFilter);
    }

    // tab order
    QWidget::setTabOrder(lookInCombo, listView);
    QWidget::setTabOrder(listView, treeView);
    QWidget::setTabOrder(treeView, fileNameEdit);
    QWidget::setTabOrder(fileNameEdit, fileTypeCombo);
    QWidget::setTabOrder(fileTypeCombo, acceptButton);
    QWidget::setTabOrder(acceptButton, rejectButton);
    QWidget::setTabOrder(rejectButton, lookInCombo);

    // last init
    q->resize(530, 340);
    fileNameEdit->setFocus();
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
    listView->setRootIndex(current);

    listView->setWrapping(true);
    listView->setResizeMode(QListView::Adjust);
    listView->setEditTriggers(QAbstractItemView::EditKeyPressed);
    listView->setContextMenuPolicy(Qt::CustomContextMenu);

    grid->addWidget(listView, 1, 0, 1, 6);

    QObject::connect(listView, SIGNAL(activated(QModelIndex)), q, SLOT(enterDirectory(QModelIndex)));
    QObject::connect(listView, SIGNAL(customContextMenuRequested(const QPoint&)),
                     q, SLOT(showContextMenu(const QPoint&)));

    QShortcut *shortcut = new QShortcut(listView);
    shortcut->setKey(QKeySequence("Delete"));

    QObject::connect(shortcut, SIGNAL(activated()), q, SLOT(deleteCurrent()));
}

void QFileDialogPrivate::setupTreeView(const QModelIndex &current, QGridLayout *grid)
{
    treeView = new QTreeView(q);

    treeView->setModel(model);
    treeView->setSelectionModel(selections);
    treeView->setSelectionMode(selectionMode(fileMode));
    treeView->setRootIndex(current);

    treeView->viewport()->setAcceptDrops(true);
    treeView->setRootIsDecorated(false);
    treeView->header()->setResizeMode(QHeaderView::Stretch, treeView->header()->count() - 1);
    treeView->header()->setSortIndicator(0, Qt::AscendingOrder);
    treeView->header()->setSortIndicatorShown(true);
    treeView->header()->setClickable(true);
    treeView->hide();
    treeView->setEditTriggers(QAbstractItemView::EditKeyPressed);
    treeView->resizeColumnToContents(0);
    treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    grid->addWidget(treeView, 1, 0, 1, 6);

    QObject::connect(treeView, SIGNAL(activated(QModelIndex)), q, SLOT(enterDirectory(QModelIndex)));
    QObject::connect(treeView, SIGNAL(customContextMenuRequested(const QPoint&)),
                     q, SLOT(showContextMenu(const QPoint&)));

    QShortcut *shortcut = new QShortcut(treeView);
    shortcut->setKey(QKeySequence("Delete"));

    QObject::connect(shortcut, SIGNAL(activated()), q, SLOT(deleteCurrent()));
}

void QFileDialogPrivate::setupToolButtons(const QModelIndex &current, QGridLayout *grid)
{
    QHBoxLayout *box = new QHBoxLayout;
    box->setMargin(3);
    box->setSpacing(3);
    QSize tools(22, 22);

    backButton = new QToolButton(q);
    backButton->setIcon(q->style()->standardPixmap(QStyle::SP_FileDialogBack));
    backButton->setToolTip(tr("Back"));
    backButton->setAutoRaise(true);
    backButton->setEnabled(false);
    backButton->setFixedSize(tools);
    QObject::connect(backButton, SIGNAL(clicked()), q, SLOT(navigateToPrevious()));
    box->addWidget(backButton);

    toParentButton = new QToolButton(q);
    toParentButton->setIcon(q->style()->standardPixmap(QStyle::SP_FileDialogToParent));
    toParentButton->setToolTip(tr("Parent Directory"));
    toParentButton->setAutoRaise(true);
    toParentButton->setEnabled(model->parent(current).isValid());
    toParentButton->setFixedSize(tools);
    QObject::connect(toParentButton, SIGNAL(clicked()), q, SLOT(navigateToParent()));
    box->addWidget(toParentButton);

    newFolderButton = new QToolButton(q);
    newFolderButton->setIcon(q->style()->standardPixmap(QStyle::SP_FileDialogNewFolder));
    newFolderButton->setToolTip(tr("Create New Folder"));
    newFolderButton->setAutoRaise(true);
    newFolderButton->setFixedSize(tools);
    QObject::connect(newFolderButton, SIGNAL(clicked()), q, SLOT(createDirectory()));
    box->addWidget(newFolderButton);

    listModeButton = new QToolButton(q);
    listModeButton->setIcon(q->style()->standardPixmap(QStyle::SP_FileDialogListView));
    listModeButton->setToolTip(tr("List View"));
    listModeButton->setAutoRaise(true);
    listModeButton->setDown(true);
    listModeButton->setFixedSize(tools);
    QObject::connect(listModeButton, SIGNAL(clicked()), q, SLOT(showList()));
    box->addWidget(listModeButton);

    detailModeButton = new QToolButton(q);
    detailModeButton->setIcon(q->style()->standardPixmap(QStyle::SP_FileDialogDetailedView));
    detailModeButton->setToolTip(tr("Detail View"));
    detailModeButton->setAutoRaise(true);
    detailModeButton->setFixedSize(tools);
    QObject::connect(detailModeButton, SIGNAL(clicked()), q, SLOT(showDetails()));
    box->addWidget(detailModeButton);
    box->setSizeConstraint(QLayout::SetFixedSize);

    grid->addLayout(box, 0, 4, 1, 2);
}

void QFileDialogPrivate::setupWidgets(QGridLayout *grid)
{
    // labels
    lookInLabel = new QLabel(tr("Look in:"), q);
    grid->addWidget(lookInLabel, 0, 0);
    fileNameLabel = new QLabel(tr("File name:"), q);
    grid->addWidget(fileNameLabel, 2, 0);
    fileTypeLabel = new QLabel(tr("Files of type:"), q);
    grid->addWidget(fileTypeLabel, 3, 0);

    // push buttons
    acceptButton = new QPushButton(tr("Open"), q);
    QObject::connect(acceptButton, SIGNAL(clicked()), q, SLOT(accept()));
    grid->addWidget(acceptButton, 2, 5, Qt::AlignLeft);

    rejectButton = new QPushButton(tr("Cancel"), q);
    QObject::connect(rejectButton, SIGNAL(clicked()), q, SLOT(reject()));
    grid->addWidget(rejectButton, 3, 5, Qt::AlignLeft);

    // "lookin" combobox
    lookInCombo = new QComboBox(q);
    lookInCombo->setInsertPolicy(QComboBox::NoInsert);
    lookInCombo->setDuplicatesEnabled(false);
    lookInCombo->setEditable(true);
    lookInCombo->setAutoCompletion(false);
    QObject::connect(lookInCombo, SIGNAL(activated(QString)), q, SLOT(enterDirectory(QString)));
    lookInEdit = new QFileDialogLineEdit(lookInCombo);
    QObject::connect(lookInEdit, SIGNAL(textChanged(QString)),
                     q, SLOT(autoCompleteDirectory(QString)));
    QObject::connect(lookInEdit, SIGNAL(returnPressed()), q, SLOT(enterDirectory()));
    lookInCombo->setLineEdit(lookInEdit);
    lookInCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    grid->addWidget(d->lookInCombo, 0, 1, 1, 3);

    // filename
    fileNameEdit = new QFileDialogLineEdit(q);
    fileNameEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QObject::connect(fileNameEdit, SIGNAL(textChanged(QString)),
                     q, SLOT(autoCompleteFileName(QString)));
    QObject::connect(fileNameEdit, SIGNAL(returnPressed()), q, SLOT(accept()));
    grid->addWidget(fileNameEdit, 2, 1, 1, 3);

    // filetype
    fileTypeCombo = new QComboBox(q);
    fileTypeCombo->setDuplicatesEnabled(false);
    fileTypeCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QObject::connect(fileTypeCombo, SIGNAL(activated(QString)), q, SLOT(useFilter(QString)));
    grid->addWidget(fileTypeCombo, 3, 1, 1, 3);
}

void QFileDialogPrivate::updateButtons(const QModelIndex &index)
{
    toParentButton->setEnabled(index.isValid());
    backButton->setEnabled(!history.isEmpty());
    newFolderButton->setEnabled(!model->isReadOnly());
    bool block = lookInCombo->blockSignals(true);
    if (index.isValid()) {
        QString pth = toNative(d->model->filePath(index));
        Q_ASSERT(!pth.isEmpty()); // this should be caught by the first if statement
        QIcon icn = d->model->fileIcon(index);
        int i = lookInCombo->findText(pth);
        if (i > -1) {
            lookInCombo->setCurrentIndex(i);
        } else {
            lookInCombo->addItem(icn, pth);
            lookInCombo->setCurrentIndex(lookInCombo->count() - 1);
        }
    } else { // "My Computer" is already in the list
        lookInCombo->setCurrentIndex(0);
    }
    lookInCombo->blockSignals(block);
}

void QFileDialogPrivate::setRootIndex(const QModelIndex &index)
{
    bool block = selections->blockSignals(true);
    selections->clear();
    fileNameEdit->clear();
    listView->setRootIndex(index);
    treeView->setRootIndex(index);
    treeView->resizeColumnToContents(0);
    selections->blockSignals(block);
    selections->setCurrentIndex(QModelIndex(), QItemSelectionModel::SelectCurrent);
}

QModelIndex QFileDialogPrivate::rootIndex() const
{
    return listView->rootIndex();
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
    QModelIndexList matches = model->match(first, Qt::DisplayRole,
                                           text, model->rowCount(first.parent()),
                                           QAbstractItemModel::MatchFromStart
                                           |QAbstractItemModel::MatchWrap
                                           |QAbstractItemModel::MatchCase);
    for (int i = 0; i < matches.count(); ++i)
        if (d->model->isDir(matches.at(i)))
            return matches.at(i);
    return QModelIndex();
}

QModelIndex QFileDialogPrivate::matchName(const QString &name, const QModelIndex &first) const
{
    QModelIndexList matches = model->match(first, Qt::DisplayRole, name, 1,
                                           QAbstractItemModel::MatchFromStart
                                           |QAbstractItemModel::MatchWrap
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

#if defined(Q_WS_WIN)
extern QString qt_win_get_open_file_name(const QFileDialogArgs &args,
                                         QString *initialDirectory,
                                         QString *selectedFilter);

extern QString qt_win_get_save_file_name(const QFileDialogArgs &args,
                                         QString *initialDirectory,
                                         QString *selectedFilter);

extern QStringList qt_win_get_open_file_names(const QFileDialogArgs &args,
                                              QString *initialDirectory,
                                              QString *selectedFilter);

extern QString qt_win_get_existing_directory(const QFileDialogArgs &args);

#elif defined(Q_WS_MAC)
extern QStringList qt_mac_get_open_file_names(const QFileDialogArgs &args,
                                              QString *pwd,
                                              QString *selectedFilter);

extern QString qt_mac_get_save_file_name(const QFileDialogArgs &args,
                                         QString *pwd,
                                         QString *selectedFilter);
#endif

QString QFileDialogPrivate::encodeFileName(const QString &filename)
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

QString QFileDialogPrivate::workingDirectory(const QString &path, bool encode)
{
    if (!path.isEmpty()) {
        QFileInfo info(encode ? encodeFileName(path) : path);
        if (info.exists() && info.isDir())
            return path;
        return info.absolutePath();
    }
    return QDir::currentPath();
}

QString QFileDialogPrivate::initialSelection(const QString &path, bool encode)
{
    if (!path.isEmpty()) {
        QFileInfo info(encode ? encodeFileName(path) : path);
        if (!info.isDir())
            return info.fileName();
    }
    return QString();
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
                                     Options options)
{
    QString directory = QFileDialogPrivate::workingDirectory(dir);
    QString selection = QFileDialogPrivate::initialSelection(dir);

    QFileDialogArgs args;
    args.parent = parent;
    args.caption = caption;
    args.directory = directory;
    args.selection = selection;
    args.filter = filter;
    args.mode = ExistingFile;
    args.options = options;

#if defined(Q_WS_WIN)
    if (::qobject_cast<QWindowsStyle*>(qApp->style())) {
        args.directory = QFileDialogPrivate::workingDirectory(dir, false);
        args.selection = QFileDialogPrivate::initialSelection(dir, false);
        return qt_win_get_open_file_name(args, &directory, selectedFilter);
    }
#elif defined(Q_WS_MAC)
    if (::qobject_cast<QMacStyle*>(qApp->style())) {
        QStringList files = qt_mac_get_open_file_names(args, &directory, selectedFilter);
        if (!files.isEmpty())
            return QUnicodeTables::normalize(files.first(), QString::NormalizationForm_C);
        return QString();
    }
#endif
    // create a qt dialog
    args.caption = caption.isEmpty() ? QString::fromLatin1("Open") : caption;
    QFileDialog *dlg = new QFileDialog(args);
    dlg->setModal(true);
    if (selectedFilter)
        dlg->selectFilter(*selectedFilter);
    if (!selection.isEmpty())
        dlg->selectFile(selection);

    QString result;
    if (dlg->exec() == QDialog::Accepted) {
        QStringList selected = dlg->selectedFiles();
        if (selected.count() > 0)
            result = selected.first();
        if (selectedFilter)
            *selectedFilter = dlg->selectedFilter();
        directory = dlg->directory().absolutePath();
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
    QString directory = QFileDialogPrivate::workingDirectory(dir);
    QString selection = QFileDialogPrivate::initialSelection(dir);

    QFileDialogArgs args;
    args.parent = parent;
    args.caption = caption;
    args.directory = directory;
    args.selection = selection;
    args.filter = filter;
    args.mode = AnyFile;
    args.options = options;

#if defined(Q_WS_WIN)
    if (::qobject_cast<QWindowsStyle*>(qApp->style())) {
        args.directory = QFileDialogPrivate::workingDirectory(dir, false);
        args.selection = QFileDialogPrivate::initialSelection(dir, false);
        return qt_win_get_save_file_name(args, &directory, selectedFilter);
    }
#elif defined(Q_WS_MAC)
    if (::qobject_cast<QMacStyle*>(qApp->style())) {
        QString result = qt_mac_get_save_file_name(args, &directory, selectedFilter);
        return QUnicodeTables::normalize(result, QString::NormalizationForm_C);
    }
#endif
    // create a qt dialog
    args.caption = caption.isEmpty() ? QString::fromLatin1("Save As") : caption;
    QFileDialog *dlg = new QFileDialog(args);
    dlg->setModal(true);
    dlg->setAcceptMode(AcceptSave);
    if (selectedFilter)
        dlg->selectFilter(*selectedFilter);
    if (!selection.isEmpty())
        dlg->selectFile(selection);

    QString result;
    if (dlg->exec() == QDialog::Accepted) {
        QStringList selected = dlg->selectedFiles();
        if (selected.count() > 0)
            result = selected.first();
        if (selectedFilter)
            *selectedFilter = dlg->selectedFilter();
        directory = dlg->directory().absolutePath();
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
    QString directory = QFileDialogPrivate::workingDirectory(dir);

    QFileDialogArgs args;
    args.parent = parent;
    args.caption = caption;
    args.directory = directory;
    args.mode = (options & ShowDirsOnly ? DirectoryOnly : Directory);
    args.options = options;

#if defined(Q_WS_WIN)
    if (qobject_cast<QWindowsStyle *>(qApp->style()) && (options & ShowDirsOnly)) {
        args.directory = QFileDialogPrivate::workingDirectory(dir, false);
        return qt_win_get_existing_directory(args);
    }
#elif defined(Q_WS_MAC)
    if (::qobject_cast<QMacStyle*>(qApp->style())) {
        QStringList files = qt_mac_get_open_file_names(args, 0, 0);
        if (!files.isEmpty())
            return QUnicodeTables::normalize(files.first(), QString::NormalizationForm_C);
        return QString();
    }
#endif
    // create a qt dialog
    args.caption = caption.isEmpty() ? QString::fromLatin1("Find Directory") : caption;
    QFileDialog *dlg = new QFileDialog(args);
    dlg->setModal(true);

    QString result;
    if (dlg->exec() == QDialog::Accepted) {
        QStringList files = dlg->selectedFiles();
        if (!files.isEmpty())
            result = files.first();
//        directory = result;
    }
    delete dlg;
//    qt_working_dir = directory;

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
                                          Options options)
{
    QString directory = QFileDialogPrivate::workingDirectory(dir);

    QFileDialogArgs args;
    args.parent = parent;
    args.caption = caption;
    args.directory = directory;
    args.filter = filter;
    args.mode = ExistingFiles;
    args.options = options;

#if defined(Q_WS_WIN)
    if (::qobject_cast<QWindowsStyle*>(qApp->style())) {
        args.directory = QFileDialogPrivate::workingDirectory(dir, false);
        return qt_win_get_open_file_names(args, &directory, selectedFilter);
    }
#elif defined(Q_WS_MAC)
    if (::qobject_cast<QMacStyle*>(qApp->style())) {
        QStringList result = qt_mac_get_open_file_names(args, &directory, selectedFilter);
        for (int i = 0; i < result.count(); ++i)
            result.replace(i, QUnicodeTables::normalize(result.at(i), QString::NormalizationForm_C));
        return result;
    }
#endif
    // create a qt dialog
    args.caption = caption.isEmpty() ? QString::fromLatin1("Open") : caption;
    QFileDialog *dlg = new QFileDialog(args);
    dlg->setModal(true);
    if (selectedFilter)
        dlg->selectFilter(*selectedFilter);

    QStringList result;
    if (dlg->exec() == QDialog::Accepted) {
        result = dlg->selectedFiles();
        if (selectedFilter)
            *selectedFilter = dlg->selectedFilter();
        directory = dlg->directory().absolutePath();
    }
    delete dlg;

    return result;
}


#ifdef QT3_SUPPORT
/*!
    Use selectedFiles() instead.

    \oldcode
        QString selected = dialog->selectedFile();
    \newcode
        QStringList files = dialog->selectedFiles();
        QString selected;
        if (!files.isEmpty())
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

/*!
    \fn QStringList QFileDialog::getOpenFileNames(const QString &filter,
        const QString &dir, QWidget *parent, const char* name,
        const QString &caption, QString *selectedFilter, bool resolveSymlinks)
    \compat
*/

/*!
    \fn QString QFileDialog::getOpenFileName(const QString &dir,
        const QString &filter, QWidget *parent = 0, const char *name,
        const QString &caption, QString *selectedFilter, bool resolveSymlinks)

    \compat
*/

/*!
    \fn QString QFileDialog::getSaveFileName(const QString &dir,
        const QString &filter, QWidget *parent, const char *name,
        const QString &caption, QString *selectedFilter, bool resolveSymlinks)

    \compat
*/

/*!
    \fn QString QFileDialog::getExistingDirectory(const QString &dir,
        QWidget *parent, const char *name, const QString &caption,
        bool dirOnly, bool resolveSymlinks)

    \compat
*/

#include "moc_qfiledialog.cpp"
