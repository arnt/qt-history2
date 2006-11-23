/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qfiledialog.h"
#include "qfiledialog_p.h"
#include <qaction.h>
#include <qheaderview.h>
#include <qshortcut.h>
#include <qgridlayout.h>
#include <qmenu.h>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include <stdlib.h>
#include <qsettings.h>
#include <qdebug.h>

#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
bool Q_GUI_EXPORT qt_use_native_dialogs = true; // for the benefit of testing tools, until we have a proper API
#endif

#ifdef Q_WS_WIN
#include <qwindowsstyle.h>
#endif
#include <qshortcut.h>
#ifdef Q_WS_MAC
#include <private/qunicodetables_p.h>
#include <qmacstyle_mac.h>
#endif

QFileDialog::QFileDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog(*new QFileDialogPrivate, parent, f)
{
    Q_D(QFileDialog);
    d->init();
}

QFileDialog::QFileDialog(QWidget *parent,
                     const QString &caption,
                     const QString &directory,
                     const QString &filter)
    : QDialog(*new QFileDialogPrivate, parent, 0)
{
    Q_D(QFileDialog);
    setWindowTitle(caption);
    d->init(directory, filter);
}

/*!
    \internal
*/
QFileDialog::QFileDialog(const QFileDialogArgs &args)
    : QDialog(*new QFileDialogPrivate, args.parent, 0)
{
    Q_D(QFileDialog);
    if (args.caption.isEmpty()) {
        if (args.mode == ExistingFiles || args.mode == ExistingFile)
            setWindowTitle(QFileDialog::tr("Open"));
        if (args.mode == AnyFile)
            setWindowTitle(QFileDialog::tr("Save As"));
        if (args.mode == DirectoryOnly || args.mode == Directory)
            setWindowTitle(QFileDialog::tr("Find Directory"));
    } else {
        setWindowTitle(args.caption);
    }
    d->init(args.directory, args.filter);
    selectFile(args.selection);
    setFileMode(args.mode);
    setConfirmOverwrite(!(args.options & DontConfirmOverwrite));
    setResolveSymlinks(!(args.options & DontResolveSymlinks));
    if (args.options & QFileDialog::StoreState) {
        d->saveState = true;
        QSettings settings;
        settings.beginGroup(QLatin1String("General"));
        restoreState(settings.value(QLatin1String("filedialog")).toByteArray());
    }
}

/*!
    \reimp
*/
QFileDialog::~QFileDialog()
{
    Q_D(QFileDialog);
    if (d->saveState) {
        QSettings settings;
        settings.beginGroup(QLatin1String("General"));
        settings.setValue(QLatin1String("filedialog"), saveState());
    }
}

/*!
    \since 4.3
    Sets the \a urls that are located in the sidebar
*/
void QFileDialog::setSidebarUrls(const QList<QUrl> &urls)
{
    Q_D(QFileDialog);
    d->sidebar->setUrls(urls);
}

/*!
    \since 4.3
    Returns a list of urls that are currently in the sidebar
*/
QList<QUrl> QFileDialog::sidebarUrls() const
{
    Q_D(const QFileDialog);
    return d->sidebar->urls();
}

static const qint32 QFileDialogMagic = 0xbe;

/*!
    \since 4.3
    Saves the state of the dialog's layout, history and current directory.

    Typically this is used in conjunction with QSettings to remember the size
    for a future session. A version number is stored as part of the data.
*/
QByteArray QFileDialog::saveState() const
{
    Q_D(const QFileDialog);
    int version = 1;
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << qint32(QFileDialogMagic);
    stream << qint32(version);
    stream << saveGeometry();
    stream << d->splitter->saveState();
    stream << d->sidebar->urls();
    stream << history();
    stream << directory().absolutePath();
    stream << isDetailsExpanded();

    // TODO Task #127430 use the headerView->saveState() function when it is added
    // which will also save hidden sections
    stream << d->treeView->header()->sectionSize(0);
    stream << d->treeView->header()->sectionSize(1);
    stream << d->treeView->header()->sectionSize(2);
    stream << d->treeView->header()->sectionSize(3);
    return data;
}

/*!
    \since 4.3
    Restores the dialogs's layout, history and current directory to the \a state specified.

    Typically this is used in conjunction with QSettings to restore the size
    from a past session.
*/
bool QFileDialog::restoreState(const QByteArray &state)
{
    Q_D(QFileDialog);
    int version = 1;
    QByteArray sd = state;
    QDataStream stream(&sd, QIODevice::ReadOnly);
    if (stream.atEnd())
        return true;
    QByteArray geometry;
    QByteArray splitterState;
    QList<QUrl> bookmarks;
    QStringList history;
    QString currentDirectory;
    bool expanded;
    qint32 marker;
    qint32 v;
    stream >> marker;
    stream >> v;
    if (marker != QFileDialogMagic || v != version)
        return false;

    stream >> geometry
           >> splitterState
           >> bookmarks
           >> history
           >> currentDirectory
           >> expanded;
    if (d->acceptMode == AcceptSave)
        setDetailsExpanded(!expanded);
    if (!restoreGeometry(geometry))
        return false;

    if (!expanded && !d->splitter->restoreState(splitterState))
        return false;
    d->sidebar->setUrls(bookmarks);
    setHistory(history);
    setDirectory(currentDirectory);

    // TODO use the headerView->restoreState() function when it is added
    // which will also have hidden sections
    int size0, size1, size2, size3;
    stream >> size0 >> size1 >> size2 >> size3;
    d->treeView->header()->resizeSection(0, size0);
    d->treeView->header()->resizeSection(1, size0);
    d->treeView->header()->resizeSection(2, size0);
    d->treeView->header()->resizeSection(3, size0);
    return true;
}

/*!
    \internal
    set the directory to url
*/
void QFileDialogPrivate::_q_goToUrl(const QUrl &url)
{
    Q_Q(QFileDialog);
    q->setDirectory(url.path());
}

/*!
    Sets the file dialog's current \a directory.
*/
void QFileDialog::setDirectory(const QString &directory)
{
    Q_D(QFileDialog);

    QString path = d->model->filePath(d->rootIndex());
    if (!(
       (!d->forwardHistory.isEmpty() && d->forwardHistory.last() == path)
     ||(!d->backHistory.isEmpty()    && d->backHistory.last() == path)
         ) && isVisible()) {
        d->backHistory.push_back(d->model->filePath(d->rootIndex()));
    }

    QModelIndex root = d->model->setRootPath(directory);
    if (d->model->canFetchMore(root))
        d->model->fetchMore(root);
    d->listView->setRootIndex(root);
    d->treeView->setRootIndex(root);
    d->listView->selectionModel()->clear();
    d->newFolderButton->setEnabled(d->model->flags(root) & Qt::ItemIsDropEnabled );
}

/*!
    Returns the directory currently being displayed in the dialog.
*/
QDir QFileDialog::directory() const
{
    Q_D(const QFileDialog);
    return d->model->rootDirectory();
}

/*!
    Selects the given \a filename in both the file dialog.

    \sa selectedFiles();
*/
void QFileDialog::selectFile(const QString &filename)
{
    Q_D(QFileDialog);
    if (filename.isEmpty())
        return;

    QModelIndex index;
    QString text = filename;
    if (QFileInfo(filename).isAbsolute()) {
        QString current = d->model->filePath(d->listView->rootIndex());
        text.remove(current);
    }
    index = d->model->index(text);


    if (index.isValid()) {
        d->listView->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    } else {
        d->listView->selectionModel()->clear();
        if (!d->lineEdit()->hasFocus())
            d->lineEdit()->setText(text);
    }
}

/*!
    Returns a list of strings containing the absolute paths of the
    selected files in the dialog. If no files are selected, or
    the mode is not ExistingFiles, selectedFiles() is an empty string list.

    \sa selectedFilter(), selectFile()
*/
QStringList QFileDialog::selectedFiles() const
{
    Q_D(const QFileDialog);
    QModelIndexList indexes = d->listView->selectionModel()->selectedRows();
    QStringList files;
    for (int i = 0; i < indexes.count(); ++i)
        files.append(d->model->filePath(indexes.at(i)));

    // if we have no selected items, use the name(s) in the lineedit
    if (files.isEmpty() && !d->lineEdit()->text().isEmpty()) {
        QString editText = d->lineEdit()->text();
        if (editText.contains(QLatin1Char('"'))) {
            // " is used to separate files like so: "file1" "file2" "file3" ...
            // ### need escape character for filenames with quotes (")
            QStringList tokens = editText.split(QLatin1Char('\"'));
            for (int i = 0; i < tokens.size(); ++i) {
                if ((i % 2) == 0)
                    continue; // Every even token is a separator
                QString name = d->toInternal(tokens.at(i));
                QFileInfo info(name);
                // if the filename has no suffix, add the default suffix
                if (!d->defaultSuffix.isEmpty() && !info.isDir() && name.lastIndexOf(QLatin1Char('.')) == -1)
                    name += QLatin1Char('.') + d->defaultSuffix;
                // a new filename
                if ((d->fileMode == ExistingFiles) || files.isEmpty()) {
                    if (info.isAbsolute())
                        files.append(name);
                    else
                        files.append(d->toInternal(d->lookInCombo->currentText()
                                                   + QDir::separator() + name));
                }
            }
        } else {
            QString name = editText;
            QFileInfo info(name);
            // if the filename has no suffix, add the default suffix
            if (!d->defaultSuffix.isEmpty() && !info.isDir() && name.lastIndexOf(QLatin1Char('.')) == -1)
                name += QLatin1Char('.') + d->defaultSuffix;
            if (info.isAbsolute())
                files.append(name);
            else
                files.append(d->toInternal(d->model->filePath(d->rootIndex()) + QDir::separator() + name));
        }
    }

    // accept the current directory when in DirectoryOnly mode
    if (files.isEmpty() && d->lineEdit()->text().isEmpty() && d->fileMode == DirectoryOnly)
        files.append(d->model->filePath(d->rootIndex()));

    return files;
}

/*
    Makes a list of filters from ;;-separated text.
    Used by the mac and windows implementations
*/
QStringList qt_make_filter_list(const QString &filter)
{
    QString f(filter);

    if (f.isEmpty())
        return QStringList();

    QString sep(QLatin1String(";;"));
    int i = f.indexOf(sep, 0);
    if (i == -1) {
        if (f.indexOf(QLatin1Char('\n'), 0) != -1) {
            sep = QLatin1Char('\n');
            i = f.indexOf(sep, 0);
        }
    }

    return f.split(sep);
}

/*!
    Sets the filter used in the file dialog to the given \a filter.

    If \a filter contains a pair of parentheses containing one or more
    of \bold{anything*something}, separated by spaces or by
    semicolons then only the text contained in the parentheses is used as
    the filter. This means that these calls are all equivalent:

    \code
       dialog.setFilter("All C++ files (*.cpp *.cc *.C *.cxx *.c++)");
       dialog.setFilter("*.cpp *.cc *.C *.cxx *.c++");
       dialog.setFilter("All C++ files (*.cpp;*.cc;*.C;*.cxx;*.c++)");
       dialog.setFilter("*.cpp;*.cc;*.C;*.cxx;*.c++");
    \endcode

    \sa setFilters()
*/
void QFileDialog::setFilter(const QString &filter)
{
    setFilters(qt_make_filter_list(filter));
}

/*!
    Sets the \a filters used in the file dialog.

    \code
      QStringList filters;
      filters << "Image files (*.png *.xpm *.jpg)"
              << "Text files (*.txt)"
              << "Any files (*)";

      QFileDialog dialog(this);
      dialog.setFilters(filters);
      dialog.exec();
    \endcode
*/
void QFileDialog::setFilters(const QStringList &filters)
{
    Q_D(QFileDialog);
    d->fileTypeCombo->clear();
    d->fileTypeCombo->addItems(filters);
    d->updateFileTypeVisibility();
    d->_q_useNameFilter(filters.first());
}

/*!
    Returns the file type filters that are in operation on this file
    dialog.
*/
QStringList QFileDialog::filters() const
{
    Q_D(const QFileDialog);
    QStringList items;
    for (int i = 0; i < d->fileTypeCombo->count(); ++i)
        items.append(d->fileTypeCombo->itemText(i));
    return items;
}

/*!
    Sets the current file type \a filter. Multiple filters can be
    passed in \a filter by separating them with semicolons or spaces.

    \sa setFilter(), setFilters(), selectedFilter()
*/
void QFileDialog::selectFilter(const QString &filter)
{
    Q_D(QFileDialog);
    int i = d->fileTypeCombo->findText(filter);
    if (i >= 0) {
        d->fileTypeCombo->setCurrentIndex(i);
        d->_q_useNameFilter(d->fileTypeCombo->currentText());
    }
}

/*!
    Returns the filter that the user selected in the file dialog.

    \sa selectedFiles()
*/
QString QFileDialog::selectedFilter() const
{
    Q_D(const QFileDialog);
    return d->fileTypeCombo->currentText();
}

/*!
    \property QFileDialog::viewMode
    \brief the way files and directories are displayed in the dialog

    By default, the \c Detail mode is used to display information about
    files and directories.

    \sa ViewMode
*/
void QFileDialog::setViewMode(QFileDialog::ViewMode mode)
{
    Q_D(QFileDialog);
    if (mode == Detail)
        d->_q_showDetailsView();
    else
        d->_q_showListView();
}

QFileDialog::ViewMode QFileDialog::viewMode() const
{
    Q_D(const QFileDialog);
    return (d->stackedWidget->currentWidget() == d->listView ? QFileDialog::List : QFileDialog::Detail);
}

/*!
    \property QFileDialog::fileMode
    \brief the file mode of the dialog

    The file mode defines the number and type of items that the user is
    expected to select in the dialog.

    \sa FileMode
*/
void QFileDialog::setFileMode(QFileDialog::FileMode mode)
{
    Q_D(QFileDialog);
    d->fileMode = mode;
    // set selection mode and behavior
    QAbstractItemView::SelectionMode selectionMode;
    if (mode == QFileDialog::ExistingFiles)
        selectionMode = QAbstractItemView::ExtendedSelection;
    else
        selectionMode = QAbstractItemView::SingleSelection;
    d->listView->setSelectionMode(selectionMode);
    d->treeView->setSelectionMode(selectionMode);
    // set filter
    QDir::Filters filters = d->filterForMode(mode);
    d->model->setFilter(filters);
    // setup file type for directory
    if (mode == DirectoryOnly || mode == Directory) {
        d->newFolderButton->setVisible(true);
        d->fileTypeCombo->clear();
        d->fileTypeCombo->addItem(tr("Directories"));
        d->fileTypeCombo->setEnabled(false);
        setLabelText(FileName, tr("Directory:"));
    } else {
        setLabelText(FileName, tr("Save &as:"));
    }
    d->fileTypeCombo->setEnabled(mode != DirectoryOnly);
}

QFileDialog::FileMode QFileDialog::fileMode() const
{
    Q_D(const QFileDialog);
    return d->fileMode;
}

/*!
    \property QFileDialog::acceptMode
    \brief the accept mode of the dialog

    The action mode defines whether the dialog is for opening or saving files.

    \sa AcceptMode
*/
void QFileDialog::setAcceptMode(QFileDialog::AcceptMode mode)
{
    Q_D(QFileDialog);
    d->acceptMode = mode;
    bool directoryMode = (d->fileMode == Directory || d->fileMode == DirectoryOnly);
    if (mode == AcceptOpen && directoryMode)
        d->openAction->setText(tr("Choose"));
    else
        d->openAction->setText(mode == AcceptOpen ? tr("&Open") : tr("&Save"));

    QDialogButtonBox::StandardButton button = (mode == AcceptOpen ? QDialogButtonBox::Open : QDialogButtonBox::Save);
    d->buttonBox->setStandardButtons(button | QDialogButtonBox::Cancel);
    d->buttonBox->button(button)->setEnabled(false);
    d->newFolderButton->setVisible(mode == AcceptSave || directoryMode);
    d->updateFileTypeVisibility();
    d->fileNameEdit->setVisible(mode == AcceptSave);
    d->expandButton->setVisible(mode == AcceptSave);
    d->line->setVisible(mode == AcceptSave);
    d->fileNameLabel->setVisible(mode == AcceptSave);

    if (mode != AcceptSave) {
        d->lookInCombo->setEditable(true);
        d->lookInCombo->setLineEdit(new ComboLineEdit(d->lookInCombo));
        d->lookInCombo->setCompleter(d->comboCompleter);
        connect(d->lookInCombo->lineEdit(), SIGNAL(textChanged(QString)),
                     this, SLOT(_q_updateOkButton()));
        connect(d->lookInCombo->lineEdit(), SIGNAL(textChanged(QString)),
                     this, SLOT(_q_autoCompleteFileName(QString)));
        connect(d->lookInCombo->lineEdit(), SIGNAL(returnPressed()),
                     this, SLOT(accept()));
    } else {
        d->lookInCombo->setEditable(false);
    }
}

void QFileDialogPrivate::updateFileTypeVisibility()
{
    bool showFilterGUI = true;
    if (fileTypeCombo->count() == 1
        && fileTypeCombo->itemText(0) == QFileDialog::tr("AllFiles (*)"))
            showFilterGUI = false;
    fileTypeCombo->setVisible(showFilterGUI);
    fileTypeLabel->setVisible(showFilterGUI);
}

QFileDialog::AcceptMode QFileDialog::acceptMode() const
{
    Q_D(const QFileDialog);
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
    Q_D(QFileDialog);
    d->model->setReadOnly(enabled);
    d->newFolderButton->setEnabled(!enabled);
    d->renameAction->setEnabled(!enabled);
    d->deleteAction->setEnabled(!enabled);
}

bool QFileDialog::isReadOnly() const
{
    Q_D(const QFileDialog);
    return d->model->isReadOnly();
}

void QFileDialog::setDetailsExpanded(bool expanded)
{
    Q_D(QFileDialog);

    // The order matters because otherwise it widgets will jump around
    if (expanded)
        d->lookInLabel->setVisible(!expanded);
    d->updateFileTypeVisibility();
    d->sidebar->setVisible(expanded);
    d->backButton->setVisible(expanded);
    d->forwardButton->setVisible(expanded);
    d->toParentButton->setVisible(expanded);
    d->stackedWidget->setVisible(expanded);
    d->listModeButton->setVisible(expanded);
    d->detailModeButton->setVisible(expanded);
    d->newFolderButton->setVisible(expanded);
    d->line->setVisible(expanded);
    d->bottomRightSpacer->setVisible(expanded);
    if (!expanded)
        d->lookInLabel->setVisible(!expanded);

    if (expanded) {
        d->expandButton->setArrowType(Qt::UpArrow);
    } else {
        d->fileNameEdit->setFocus();
        // ### auto select the file name
        d->expandButton->setArrowType(Qt::DownArrow);
    }
}

bool QFileDialog::isDetailsExpanded() const
{
    Q_D(const QFileDialog);
    return (d->expandButton->arrowType() == Qt::DownArrow);
}

/*!
    \property QFileDialog::resolveSymlinks
    \brief whether the filedialog should resolve symbolic links

    If this property is set to true, the file dialog will resolve
    symbolic links.
*/
void QFileDialog::setResolveSymlinks(bool enabled)
{
    Q_D(QFileDialog);
    d->model->setResolveSymlinks(enabled);
}

bool QFileDialog::resolveSymlinks() const
{
    Q_D(const QFileDialog);
    return d->model->resolveSymlinks();
}

/*!
    \property QFileDialog::confirmOverwrite
    \brief whether the filedialog should ask before accepting a selected file,
    when the accept mode is AcceptSave

    If this property is set to true and the accept mode is
    AcceptSave, the filedialog will ask whether the user wants to
    overwrite the fike before accepting the file.
*/
void QFileDialog::setConfirmOverwrite(bool enabled)
{
    Q_D(QFileDialog);
    d->confirmOverwrite = enabled;
}

bool QFileDialog::confirmOverwrite() const
{
    Q_D(const QFileDialog);
    return d->confirmOverwrite;
}

/*!
    \property QFileDialog::defaultSuffix
    \brief suffix added to the filename if no other suffix was specified

    This property specifies a string that will be added to the
    filename if it has no suffix already. The suffix is typically
    used to indicate the file type (e.g. "txt" indicates a text
    file).
*/
void QFileDialog::setDefaultSuffix(const QString &suffix)
{
    Q_D(QFileDialog);
    d->defaultSuffix = suffix;
}

QString QFileDialog::defaultSuffix() const
{
    Q_D(const QFileDialog);
    return d->defaultSuffix;
}

/*!
    Sets the browsing history of the filedialog to contain the given
    \a paths.
*/
void QFileDialog::setHistory(const QStringList &paths)
{
    Q_D(QFileDialog);
    d->history = paths;

    d->lookInCombo->clear();
    //d->lookInComob->model()->insertCOlumns(0, 1);
    QList<QUrl> list;
    QModelIndex idx = d->model->index(d->model->rootPath());
    while (idx.isValid()) {
        QUrl url = QUrl::fromLocalFile(d->model->filePath(idx));
        if (url.isValid())
            list.append(url);
        idx = idx.parent();
    }
    // add "my computer"
    list.append(QUrl::fromLocalFile(QLatin1String("")));
    d->addUrls(list, 0);
    idx = d->lookInCombo->model()->index(d->lookInCombo->model()->rowCount()-1, 0);

    // ### append history
    QList<QUrl> urls;
    for (int i = 0; i < d->history.count(); ++i) {
        QUrl path = QUrl::fromLocalFile(d->history.at(i));
        if (!urls.contains(path))
            urls += path;
    }
    if (urls.count() > 0) {
        d->lookInCombo->model()->insertRow(d->lookInCombo->model()->rowCount());
        idx = d->lookInCombo->model()->index(d->lookInCombo->model()->rowCount()-1, 0);
        // ### TODO disable this item and maybe add a horizontal line before it
        d->lookInCombo->model()->setData(idx, QLatin1String("Recent Places"));
        QStandardItemModel *m = qobject_cast<QStandardItemModel*>(d->lookInCombo->model());
        if (m) {
            Qt::ItemFlags flags = m->flags(idx);
            flags &= ~Qt::ItemIsEnabled;
            m->item(idx.row(), idx.column())->setFlags(flags);
        }
        d->addUrls(urls, -1);
    }
    d->lookInCombo->setCurrentIndex(0);
}

// Copied from QSidebar NMC
void QFileDialogPrivate::addUrls(const QList<QUrl> &list, int row) {
    if (row < 0)
        row = lookInCombo->model()->rowCount();
    for (int i = list.count() - 1; i >= 0; --i) {
        QUrl url = list.at(i);
        if (!url.isValid())
            continue;
        QModelIndex idx = model->index(url.path());
        lookInCombo->model()->insertRows(row, 1);
        setUrl(lookInCombo->model()->index(row, 0), url);
        watching.append(url.toString());
    }
}

// Copied from QSidebar NMC
void QFileDialogPrivate::setUrl(const QModelIndex &index, const QUrl &url)
{
    QModelIndex dirIndex = model->index(url.path());
    lookInCombo->model()->setData(index, url, UrlRole);
    if (url.path().isEmpty()) {
        lookInCombo->model()->setData(index, model->myComputer());
        lookInCombo->model()->setData(index, model->myComputer(Qt::DecorationRole), Qt::DecorationRole);
    } else {
        if (index.data() != dirIndex.data()) {
            lookInCombo->model()->setData(index, model->data(dirIndex).toString());
        }
        if (index.data(Qt::DecorationRole).value<QIcon>().serialNumber() != dirIndex.data(Qt::DecorationRole).value<QIcon>().serialNumber()) {
            lookInCombo->model()->setData(index, model->data(dirIndex, Qt::DecorationRole), Qt::DecorationRole);
        }
    }
}

// Copied from QSidebar NMC
void QFileDialogPrivate::_q_layoutChanged()
{
    QStringList paths;
    for (int i = 0; i < watching.count(); ++i)
        paths.append(watching.at(i));
    watching.clear();
    QMultiHash<QString, QModelIndex> lt;
    for (int i = 0; i < lookInCombo->model()->rowCount(); ++i) {
        QModelIndex idx = lookInCombo->model()->index(i, 0);
        lt.insert(idx.data(UrlRole).toUrl().toString(), idx);
    }

    for (int i = 0; i < paths.count(); ++i) {
        QString path = paths.at(i);
        QModelIndex newIndex = model->index(QUrl(path).path());
        watching.append(path);
        if (!newIndex.isValid())
            continue;
        QList<QModelIndex> values = lt.values(path);
        for (int i = 0; i < values.size(); ++i) {
            QModelIndex idx = values.at(i);
            setUrl(idx, path);
        }
    }
    newFolderButton->setEnabled(model->flags(listView->rootIndex()) & Qt::ItemIsDropEnabled);
}

/*!
    \brief returns the browsing history of the filedialog as a list of paths.
*/
QStringList QFileDialog::history() const
{
    Q_D(const QFileDialog);
    QStringList currentHistory = d->history;
    QString newHistory = d->model->filePath(d->rootIndex());
    if (!currentHistory.contains(newHistory))
        currentHistory << newHistory;
    return currentHistory;
}

/*!
    \brief set the item delegate used to render the items in the views in the
    filedialog to the specified \a delegate
*/
void QFileDialog::setItemDelegate(QAbstractItemDelegate *delegate)
{
    Q_D(QFileDialog);
    d->listView->setItemDelegate(delegate);
    d->treeView->setItemDelegate(delegate);
}

/*!
    \brief returns the item delegate used to render the items in the views in
  the filedialog
*/
QAbstractItemDelegate *QFileDialog::itemDelegate() const
{
    Q_D(const QFileDialog);
    return d->listView->itemDelegate();
}

/*!
    \brief set the icon provider used by the filedialog to the specified
    \a provider
*/
void QFileDialog::setIconProvider(QFileIconProvider *provider)
{
    Q_D(QFileDialog);
    d->model->setIconProvider(provider);
}

/*!
    \brief returns the icon provider used by the filedialog.
*/
QFileIconProvider *QFileDialog::iconProvider() const
{
    Q_D(const QFileDialog);
    return d->model->iconProvider();
}

/*!
    \brief set the \a text shown in the filedialog in the specified \a label
*/
void QFileDialog::setLabelText(DialogLabel label, const QString &text)
{
    Q_D(QFileDialog);
    QPushButton *button;
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
        if (acceptMode() == AcceptOpen)
            button = d->buttonBox->button(QDialogButtonBox::Open);
        else
            button = d->buttonBox->button(QDialogButtonBox::Save);
        if (button)
            button->setText(text);
        d->openAction->setText(text);
        break;
    case Reject:
        button = d->buttonBox->button(QDialogButtonBox::Cancel);
        if (button)
            button->setText(text);
        break;
    }
}

/*!
    \brief returns the text shown in the filedialog in the specified \a label
*/
QString QFileDialog::labelText(DialogLabel label) const
{
    QPushButton *button;
    Q_D(const QFileDialog);
    switch (label) {
    case LookIn:
        return d->lookInLabel->text();
    case FileName:
        return d->fileNameLabel->text();
    case FileType:
        return d->fileTypeLabel->text();
    case Accept:
        if (acceptMode() == AcceptOpen)
            button = d->buttonBox->button(QDialogButtonBox::Open);
        else
            button = d->buttonBox->button(QDialogButtonBox::Save);
        if (button)
            return button->text();
    case Reject:
        button = d->buttonBox->button(QDialogButtonBox::Cancel);
        if (button)
            return button->text();
    }
    return QString();
}

/*
    For the native file dialogs
*/

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

/*!
  This is a convenience static function that returns an existing file
  selected by the user. If the user presses Cancel, it returns a null
  string.

  \code
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    "/home",
                                                    tr("Images (*.png *.xpm *.jpg)"));
  \endcode

  The function creates a modal file dialog with the given \a parent widget.
  If the parent is not 0, the dialog will be shown centered over the
  parent widget.

  The file dialog's working directory will be set to \a dir.
  If \a dir includes a file name, the file will be selected. Only files
  that match the given \a filter are shown. The filter selected is set to
  \a selectedFilter. The parameters \a dir, \a selectedFilter, and
  \a filter may be empty strings. The \a options argument holds various
  options about how to run the dialog, see the QFileDialog::Option enum for
  more information on the flags you can pass.

  The dialog's caption is set to \a caption. If \a caption is not
  specified then a default caption will be used.

  Under Windows and Mac OS X, this static function will use the native
  file dialog and not a QFileDialog. On Mac OS X, the \a dir argument
  is ignored, the native dialog always displays the last visited directory.

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
    QFileDialogArgs args;
    args.parent = parent;
    args.caption = caption;
    args.directory = QFileDialogPrivate::workingDirectory(dir);
    args.selection = QFileDialogPrivate::initialSelection(dir);
    args.filter = filter;
    args.mode = ExistingFile;
    args.options = options;
#if defined(Q_WS_WIN)
    if (qt_use_native_dialogs && !(args.options & DontUseNativeDialog)) {
        return qt_win_get_open_file_name(args, &(args.directory), selectedFilter);
    }
#elif defined(Q_WS_MAC)
    if (qt_use_native_dialogs && !(args.options & DontUseNativeDialog)) {
        QStringList files = qt_mac_get_open_file_names(args, &(args.directory), selectedFilter);
        if (!files.isEmpty())
            return files.first().normalized(QString::NormalizationForm_C);
        return QString();
    }
#endif

    // create a qt dialog
    QFileDialog dialog(args);
    dialog.setModal(true);
    if (selectedFilter)
        dialog.selectFilter(*selectedFilter);
    if (dialog.exec() == QDialog::Accepted) {
        if (selectedFilter)
            *selectedFilter = dialog.selectedFilter();
        return dialog.selectedFiles().value(0);
    }
    return QString();
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
  file dialog and not a QFileDialog. On Mac OS X, the \a dir argument
  is ignored, the native dialog always displays the last visited directory.

  Note that on Windows the dialog will spin a blocking modal event loop
  that will not dispatch any QTimers, and if parent is not 0 then it will
  position the dialog just under the parent's title bar.

  Under Unix/X11, the normal behavior of the file dialog is to resolve
  and follow symlinks. For example, if \c{/usr/tmp} is a symlink to
  \c{/var/tmp}, the file dialog will change to \c{/var/tmp} after
  entering \c{/usr/tmp}. The \a options argument holds various
  options about how to run the dialog, see the QFileDialog::Option enum for
  more information on the flags you can pass.

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
    QFileDialogArgs args;
    args.parent = parent;
    args.caption = caption;
    args.directory = QFileDialogPrivate::workingDirectory(dir);
    args.selection = QFileDialogPrivate::initialSelection(dir);
    args.filter = filter;
    args.mode = ExistingFiles;
    args.options = options;

#if defined(Q_WS_WIN)
    if (qt_use_native_dialogs && !(args.options & DontUseNativeDialog)) {
        return qt_win_get_open_file_names(args, &(args.directory), selectedFilter);
    }
#elif defined(Q_WS_MAC)
    if (qt_use_native_dialogs && !(args.options & DontUseNativeDialog)) {
        QStringList files = qt_mac_get_open_file_names(args, &(args.directory), selectedFilter);
        for (int i = 0; i < files.count(); ++i)
            files.replace(i, files.at(i).normalized(QString::NormalizationForm_C));
        return files;
    }
#endif

    // create a qt dialog
    QFileDialog dialog(args);
    dialog.setModal(true);
    if (selectedFilter)
        dialog.selectFilter(*selectedFilter);
    if (dialog.exec() == QDialog::Accepted) {
        if (selectedFilter)
            *selectedFilter = dialog.selectedFilter();
        return dialog.selectedFiles();
    }
    return QStringList();
}

/*!
  This is a convenience static function that will return a file name
  selected by the user. The file does not have to exist.

  It creates a modal file dialog with the given \a parent widget. If the
  parent is not 0, the dialog will be shown centered over the parent
  widget.

  \code
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                               "/home/jana/untitled.png",
                               tr("Images (*.png *.xpm *.jpg)"));
  \endcode

  The file dialog's working directory will be set to \a dir. If \a
  dir includes a file name, the file will be selected. Only files that
  match the \a filter are shown. The filter selected is set to
  \a selectedFilter. The parameters \a dir, \a selectedFilter, and
  \a filter may be empty strings. The \a options argument holds various
  options about how to run the dialog, see the QFileDialog::Option enum for
  more information on the flags you can pass.

  The dialog's caption is set to \a caption. If \a caption is not
  specified then a default caption will be used.

  Under Windows and Mac OS X, this static function will use the native
  file dialog and not a QFileDialog.

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
    QFileDialogArgs args;
    args.parent = parent;
    args.caption = caption;
    args.directory = QFileDialogPrivate::workingDirectory(dir);
    args.selection = QFileDialogPrivate::initialSelection(dir);
    args.filter = filter;
    args.mode = AnyFile;
    args.options = options;

#if defined(Q_WS_WIN)
    if (qt_use_native_dialogs && !(args.options & DontUseNativeDialog)) {
        return qt_win_get_save_file_name(args, &(args.directory), selectedFilter);
    }
#elif defined(Q_WS_MAC)
    if (qt_use_native_dialogs && !(args.options & DontUseNativeDialog)) {
        QString result = qt_mac_get_save_file_name(args, &(args.directory), selectedFilter);
        return result.normalized(QString::NormalizationForm_C);
    }
#endif

    // create a qt dialog
    QFileDialog dialog(args);
    dialog.setModal(true);
    dialog.setAcceptMode(AcceptSave);
    dialog.setDetailsExpanded(false);
    dialog.resize(dialog.width(), dialog.sizeHint().height());
    if (selectedFilter)
        dialog.selectFilter(*selectedFilter);
    if (dialog.exec() == QDialog::Accepted) {
        if (selectedFilter)
            *selectedFilter = dialog.selectedFilter();
        return dialog.selectedFiles().value(0);
    }

    return QString();
}

/*!
  This is a convenience static function that will return an existing
  directory selected by the user.

  \code
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    "/home",
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
  \endcode

  This function creates a modal file dialog with the given \a parent
  widget. If the parent is not 0, the dialog will be shown centered over
  the parent widget.

  The dialog's working directory is set to \a dir, and the caption is
  set to \a caption. Either of these may be an empty string in which case
  the current directory and a default caption will be used
  respectively. The \a options argument holds various
  options about how to run the dialog, see the QFileDialog::Option enum for
  more information on the flags you can pass.

  Under Windows and Mac OS X, this static function will use the native
  file dialog and not a QFileDialog. On Mac OS X, the \a dir argument
  is ignored, the native dialog always displays the last visited directory.

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
    QFileDialogArgs args;
    args.parent = parent;
    args.caption = caption;
    args.directory = QFileDialogPrivate::workingDirectory(dir);
    args.mode = (options & ShowDirsOnly ? DirectoryOnly : Directory);
    args.options = options;

#if defined(Q_WS_WIN)
    if (qt_use_native_dialogs && !(args.options & DontUseNativeDialog) && (options & ShowDirsOnly)) {
        return qt_win_get_existing_directory(args);
    }
#elif defined(Q_WS_MAC)
    if (qt_use_native_dialogs && !(args.options & DontUseNativeDialog)) {
        QStringList files = qt_mac_get_open_file_names(args, 0, 0);
        if (!files.isEmpty())
            return files.first().normalized(QString::NormalizationForm_C);
        return QString();
    }
#endif

    // create a qt dialog
    QFileDialog dialog(args);
    dialog.setModal(true);
    if (dialog.exec() == QDialog::Accepted) {
        QString result = dialog.selectedFiles().value(0);
        if (!result.isEmpty() && result.right(1) != QLatin1String("/"))
            result += QLatin1Char('/');
        return result;
    }
    return QString();
}

/*
    Get the initial directory path

    \sa initialSelection()
 */
QString QFileDialogPrivate::workingDirectory(const QString &path)
{
    if (!path.isEmpty()) {
        QFileInfo info(path);
        if (info.exists() && info.isDir())
            return info.absoluteFilePath();
        return info.absolutePath();
    }
    return QDir::currentPath();
}

/*
    Get the initial selection given a path.  The initial directory
    can contain both the initial directory and initial selection
    /home/user/foo.txt

    \sa workingDirectory()
 */
QString QFileDialogPrivate::initialSelection(const QString &path)
{
    if (!path.isEmpty()) {
        QFileInfo info(path);
        if (!info.isDir())
            return info.fileName();
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
    Q_D(QFileDialog);
    QStringList files = selectedFiles();
    if (files.isEmpty())
        return;
    QString lineEditText = d->lineEdit()->text();
    // "hidden feature" type .. and then enter, and it will move up a dir
    // special case for ".."
    if (lineEditText == QLatin1String("..")) {
        d->_q_navigateToParent();
        bool block = d->fileNameEdit->blockSignals(true);
        d->lineEdit()->selectAll();
        d->lineEdit()->selectAll();
        d->fileNameEdit->blockSignals(block);
        return;
    }

    switch (d->fileMode) {
    case DirectoryOnly:
    case Directory: {
        QString fn = files.first();
        QFileInfo info(fn);
        if (!info.exists())
            info = QFileInfo(d->getEnvironmentVariable(fn));
        if (!info.exists()) {
#ifndef QT_NO_MESSAGEBOX
            QString message = tr("\nFile not found.\nPlease verify the "
                                 "correct file name was given");

            QPushButton *button = d->buttonBox->button(acceptMode() == AcceptOpen ?
                                                       QDialogButtonBox::Open : QDialogButtonBox::Save);
            QMessageBox::warning(this, button->text(), info.fileName() + message);
#endif // QT_NO_MESSAGEBOX
            return;
        }
        if (info.isDir()) {
            emit filesSelected(files);
            QDialog::accept();
        }
        return;
    }

    case AnyFile: {
        QString fn = files.first();
        QFileInfo info(fn);
        if (info.isDir()) {
            setDirectory(info.absoluteFilePath());
            d->lineEdit()->selectAll();
            return;
        }
        // check if we have to ask for permission to overwrite the file
        if (!info.exists() || !confirmOverwrite() || acceptMode() == AcceptOpen) {
            QDialog::accept();
#ifndef QT_NO_MESSAGEBOX
        } else {
            QPushButton *button = d->buttonBox->button(acceptMode() == AcceptOpen ?
                                                       QDialogButtonBox::Open : QDialogButtonBox::Save);
            if (QMessageBox::warning(this, button->text(),
                                     tr("%1 already exists.\nDo you want to replace it?")
                                     .arg(info.fileName()),
                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
                == QMessageBox::Yes)
                QDialog::accept();
#endif
        }
        return;
    }

    case ExistingFile:
    case ExistingFiles:
        for (int i = 0; i < files.count(); ++i) {
            QFileInfo info(files.at(i));
            if (!info.exists())
                info = QFileInfo(d->getEnvironmentVariable(files.at(i)));
            if (!info.exists()) {
#ifndef QT_NO_MESSAGEBOX
                QString message = tr("%1\nFile not found.\nPlease verify the "
                                     "correct file name was given.");
                QPushButton *button = d->buttonBox->button(acceptMode() == AcceptOpen ?
                                                            QDialogButtonBox::Open : QDialogButtonBox::Save);

                QMessageBox::warning(this, button->text(), message.arg(info.fileName()));
#endif // QT_NO_MESSAGEBOX
                return;
            }
            if (info.isDir()) {
                setDirectory(info.absoluteFilePath());
                d->lineEdit()->selectAll();
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

    Create widgets, layout and set default values
*/
void QFileDialogPrivate::init(const QString &directory, const QString &nameFilter)
{
    Q_Q(QFileDialog);
    createWidgets();
    createMenuActions();
    createToolButtons();
    layout();
    oldSize = q->sizeHint();

    // Default case
    q->setFilter(nameFilter.isEmpty() ? QFileDialog::tr("AllFiles (*)") : nameFilter);
    q->setFileMode(fileMode);
    q->setAcceptMode(QFileDialog::AcceptOpen);
    q->setDirectory(workingDirectory(directory));
    q->selectFile(initialSelection(directory));
}

/*!
    \internal

    layout all the widgets in a grid
*/
void QFileDialogPrivate::layout()
{
    Q_Q(QFileDialog);
    q->setSizeGripEnabled(true);
    QGridLayout *grid = new QGridLayout(q);

    // First row
    topGrid = new QGridLayout();
    QHBoxLayout *topLeftLayout = new QHBoxLayout();
    topLeftLayout->addStretch();
    topLeftLayout->addWidget(fileNameLabel, 0, Qt::AlignHCenter);

    topGrid->addLayout(topLeftLayout, 0, 0);
    topGrid->addWidget(fileNameEdit, 0, 1);
    QHBoxLayout *topRightLayout = new QHBoxLayout();
    topRightLayout->addWidget(expandButton);
    topRightLayout->addStretch();
    topGrid->addLayout(topRightLayout, 0, 2);

    // line
    topGrid->addWidget(line, 1, 0, 1, 3);

    // second line
    bottomLeftLayout = new QHBoxLayout();
    bottomLeftLayout->addWidget(backButton);
    bottomLeftLayout->addWidget(forwardButton);
    bottomLeftLayout->addWidget(toParentButton);
    bottomLeftLayout->addWidget(listModeButton);
    bottomLeftLayout->addWidget(detailModeButton);
    bottomLeftLayout->addStretch();
    bottomLeftLayout->addWidget(lookInLabel);

    topGrid->addLayout(bottomLeftLayout, 2, 0);
    topGrid->addWidget(lookInCombo, 2, 1);
    QHBoxLayout *bottomRightLayout = new QHBoxLayout();
    bottomRightSpacer = new QWidget(q);
    bottomRightSpacer->setMinimumWidth(backButton->sizeHint().width() * 5);
    bottomRightLayout->addWidget(bottomRightSpacer);
    bottomRightLayout->addStretch();
    topGrid->addLayout(bottomRightLayout, 1, 2);

    grid->addLayout(topGrid, 0, 0, 1, 3);

    // main box
    grid->addWidget(splitter, 2, 0, 1, 3);

    // bottom
    grid->addWidget(fileTypeLabel, 3, 1);
    grid->addWidget(fileTypeCombo, 3, 2, 1, 1);

    grid->addWidget(newFolderButton, 4, 0, 1, 1);
    grid->addWidget(buttonBox,       4, 2);
    grid->setRowStretch(2, 1);

    // tab order
    QWidget::setTabOrder(fileNameEdit, expandButton);
    QWidget::setTabOrder(expandButton, backButton);
    QWidget::setTabOrder(backButton, forwardButton);
    QWidget::setTabOrder(forwardButton, toParentButton);
    QWidget::setTabOrder(toParentButton, listModeButton);
    QWidget::setTabOrder(listModeButton, detailModeButton);
    QWidget::setTabOrder(detailModeButton, lookInCombo);
    QWidget::setTabOrder(lookInCombo, sidebar);
    QWidget::setTabOrder(sidebar, listView);
    QWidget::setTabOrder(listView, treeView);
    QWidget::setTabOrder(treeView, fileTypeCombo);
    QWidget::setTabOrder(fileTypeCombo, newFolderButton);
    QWidget::setTabOrder(newFolderButton, buttonBox);
    QWidget::setTabOrder(buttonBox, fileNameEdit);

    q->resize(q->sizeHint());

    if (acceptMode == QFileDialog::AcceptSave)
        fileNameEdit->setFocus();
    else
        stackedWidget->currentWidget()->setFocus();
}

/*!
    \internal

    Create the widgets, set properties and connections
*/
void QFileDialogPrivate::createWidgets()
{
    Q_Q(QFileDialog);
    model = new QFileSystemModel(q);
    model->setObjectName(QLatin1String("qt_filesystem_model"));
    QFileDialog::connect(model, SIGNAL(layoutChanged()), q, SLOT(_q_layoutChanged()), Qt::QueuedConnection);
    QFileDialog::connect(model, SIGNAL(rootPathChanged(const QString &)),
            q, SLOT(_q_pathChanged(const QString &)));
    model->setReadOnly(false);

    QList<QUrl> initialBookmarks;
    initialBookmarks << QUrl::fromLocalFile(QLatin1String(""))
                     << QUrl::fromLocalFile(QDir::homePath());
    sidebar = new QSidebar(model, initialBookmarks, q);
    sidebar->setObjectName(QLatin1String("qt_sidebar"));
    QFileDialog::connect(sidebar, SIGNAL(goToUrl(const QUrl &)), q, SLOT(_q_goToUrl(const QUrl &)));

    // labels
    lookInLabel = new QLabel(QFileDialog::tr("Where:"), q);
    lookInLabel->setObjectName(QLatin1String("qt_look_in_label"));
    lookInLabel->hide();
    fileNameLabel = new QLabel(q);
    fileTypeLabel = new QLabel(QFileDialog::tr("Files of type:"), q);

    // push buttons
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Open | QDialogButtonBox::Cancel, Qt::Horizontal, q);
    buttonBox->setObjectName(QLatin1String("qt_button_box"));
    QObject::connect(buttonBox, SIGNAL(accepted()), q, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), q, SLOT(reject()));

    // "lookin" combobox
    lookInCombo = new QComboBox(q);
    lookInCombo->setObjectName(QLatin1String("qt_look_in_combo"));
    lookInCombo->setMinimumWidth(lookInCombo->sizeHint().width() * 3);
    lookInCombo->setInsertPolicy(QComboBox::NoInsert);
    lookInCombo->setDuplicatesEnabled(false);
    QObject::connect(lookInCombo, SIGNAL(activated(QString)), q, SLOT(_q_goToDirectory(QString)));
    lookInCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    comboCompleter = new QFSCompletor(q);
    comboCompleter->setModel(model);
    comboCompleter->setCompletionMode(QCompleter::InlineCompletion);

    // filename
    fileNameEdit = new QFileDialogLineEdit(q);
    fileNameEdit->setObjectName(QLatin1String("qt_file_name_edit"));
    fileNameLabel->setBuddy(fileNameEdit);
    fileNameEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    completer = new QFSCompletor(q);
    completer->setModel(model);
    fileNameEdit->setCompleter(completer);
    QObject::connect(fileNameEdit, SIGNAL(textChanged(QString)),
                     q, SLOT(_q_updateOkButton()));
    QObject::connect(fileNameEdit, SIGNAL(textChanged(QString)),
                     q, SLOT(_q_autoCompleteFileName(QString)));
    QObject::connect(fileNameEdit, SIGNAL(returnPressed()), q, SLOT(accept()));

    // filetype
    fileTypeCombo = new QComboBox(q);
    fileTypeCombo->setObjectName(QLatin1String("qt_file_type_combo"));
    fileTypeCombo->setDuplicatesEnabled(false);
    fileTypeCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    fileTypeCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QObject::connect(fileTypeCombo, SIGNAL(activated(const QString &)), q, SLOT(_q_useNameFilter(const QString &)));

    listView = new QFileDialogListView(this);
    listView->setObjectName(QLatin1String("qt_list_view"));
    listView->setModel(model);
    QObject::connect(listView, SIGNAL(activated(QModelIndex)), q, SLOT(_q_enterDirectory(QModelIndex)));
    QObject::connect(listView, SIGNAL(customContextMenuRequested(QPoint)),
                    q, SLOT(_q_showContextMenu(QPoint)));
#ifndef QT_NO_SHORTCUT
    QShortcut *shortcut = new QShortcut(listView);
    shortcut->setKey(QKeySequence(QLatin1String("Delete")));
    QObject::connect(shortcut, SIGNAL(activated()), q, SLOT(_q_deleteCurrent()));
#endif

    treeView = new QFileDialogTreeView(this);
    treeView->setObjectName(QLatin1String("qt_tree_view"));
    treeView->setModel(model);
    QHeaderView *treeHeader = treeView->header();
    treeHeader->setSectionHidden(1, true);
    treeHeader->setSectionHidden(2, true);
    treeHeader->setStretchLastSection(false);
    treeHeader->setResizeMode(0, QHeaderView::Interactive);
    treeHeader->resizeSection(0, 2 * treeHeader->sectionSizeHint(3));
    treeHeader->resizeSection(3, (int)(1.5 * treeHeader->sectionSizeHint(3)));
    treeHeader->setContextMenuPolicy(Qt::ActionsContextMenu);

    QActionGroup *showActionGroup = new QActionGroup(q);
    showActionGroup->setExclusive(false);
    q->connect(showActionGroup, SIGNAL(triggered(QAction *)), q, SLOT(_q_showHeader(QAction *)));;
    for (int i = 1; i < model->columnCount(QModelIndex()); ++i) {
        QAction *showHeader = new QAction(QFileDialog::tr("Show ") + model->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString(), showActionGroup);
        showHeader->setCheckable(true);
        if (i != 1 && i != 2)
            showHeader->setChecked(true);
        treeHeader->addAction(showHeader);
    }

    treeView->setSelectionModel(listView->selectionModel());
    QObject::connect(treeView, SIGNAL(activated(QModelIndex)), q, SLOT(_q_enterDirectory(QModelIndex)));
    QObject::connect(treeView, SIGNAL(customContextMenuRequested(QPoint)),
                     q, SLOT(_q_showContextMenu(QPoint)));
#ifndef QT_NO_SHORTCUT
    shortcut = new QShortcut(treeView);
    shortcut->setKey(QKeySequence(QLatin1String("Delete")));
    QObject::connect(shortcut, SIGNAL(activated()), q, SLOT(_q_deleteCurrent()));
#endif

    // Selections
    QItemSelectionModel *selections = listView->selectionModel();
    QObject::connect(selections, SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                     q, SLOT(_q_selectionChanged()));
    QObject::connect(selections, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                     q, SLOT(_q_currentChanged(QModelIndex)));

    stackedWidget = new QStackedWidget;
    stackedWidget->addWidget(treeView);
    stackedWidget->addWidget(listView);

    splitter = new QSplitter(q);
    splitter->addWidget(sidebar);
    splitter->addWidget(stackedWidget);

    line = new QFrame(q);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->hide();

    vTimeLine = new QTimeLine(100, q);
    hTimeLine = new QTimeLine(100, q);
    q->connect(vTimeLine, SIGNAL(frameChanged(int)), q, SLOT(_q_animateDialogV(int)));
    q->connect(hTimeLine, SIGNAL(frameChanged(int)), q, SLOT(_q_animateDialogH(int)));
}

void QFileDialogPrivate::_q_showHeader(QAction *action)
{
    Q_Q(QFileDialog);
    QActionGroup *actionGroup = qobject_cast<QActionGroup*>(q->sender());
    treeView->header()->setSectionHidden(actionGroup->actions().indexOf(action) + 1, !action->isChecked());
}

void QFileDialogPrivate::_q_animateDialogV(int x)
{
    Q_Q(QFileDialog);
    if (x != -1)
        q->resize(q->width(), x);

    // You have to update after the fact because the hide/show events haven't
    // been processed the first time sizeHint().height() was called.
    if (vTimeLine->direction() == QTimeLine::Backward)
        vTimeLine->setFrameRange(q->sizeHint().height(), vTimeLine->endFrame());

    if ((vTimeLine->endFrame() == x && vTimeLine->direction() == QTimeLine::Forward)
       || x == -1){
        bool expanded = (vTimeLine->direction() == QTimeLine::Forward);
        q->setDetailsExpanded(expanded);
    }
}

void QFileDialogPrivate::_q_animateDialogH(int x)
{
    Q_Q(QFileDialog);
    q->resize(x, q->height());
}

void QFileDialogPrivate::_q_animateDialog()
{
    Q_Q(QFileDialog);
    bool expanded = true;
    if (expandButton->arrowType() == Qt::DownArrow) {
        expandButton->setArrowType(Qt::UpArrow);
    } else {
        expandButton->setArrowType(Qt::DownArrow);
        expanded = false;
    }
    int newHeight;
    int newWidth;
    if (!expanded) {
        q->setDetailsExpanded(false);
        newHeight = q->sizeHint().height();
        newWidth = q->sizeHint().height();
    } else {
        newHeight = oldSize.height();
        newWidth = oldSize.width();
    }

    oldSize = q->size();

    if (newHeight < q->height()) {
        vTimeLine->setFrameRange(newHeight, q->height());
        hTimeLine->setFrameRange(newWidth, q->height());
        vTimeLine->setDirection(QTimeLine::Backward);
        hTimeLine->setDirection(QTimeLine::Backward);
    } else {
        vTimeLine->setFrameRange(q->height(), newHeight);
        hTimeLine->setFrameRange(q->height(), newWidth);
        vTimeLine->setDirection(QTimeLine::Forward);
        hTimeLine->setDirection(QTimeLine::Forward);
    }

    if (vTimeLine->state() == QTimeLine::NotRunning)
        vTimeLine->start();

#ifdef Q_WS_MAC
    if (hTimeLine->state() == QTimeLine::NotRunning)
        hTimeLine->start();
#endif
}

/*!
    \internal

    Create tool buttons, set properties and connections
*/
void QFileDialogPrivate::createToolButtons()
{
    Q_Q(QFileDialog);

    expandButton = new QToolButton(q);
    expandButton->setObjectName(QLatin1String("qt_expand_button"));
    expandButton->setArrowType(Qt::UpArrow);
    QObject::connect(expandButton, SIGNAL(clicked()), q, SLOT(_q_animateDialog()));
    expandButton->setVisible(false);

    backButton = new QToolButton(q);
    backButton->setObjectName(QLatin1String("qt_back_button"));
    backButton->setIcon(q->style()->standardPixmap(QStyle::SP_ArrowBack));
#ifndef QT_NO_TOOLTIP
    backButton->setToolTip(QFileDialog::tr("Back"));
#endif
    backButton->setAutoRaise(true);
    backButton->setEnabled(false);
    QObject::connect(backButton, SIGNAL(clicked()), q, SLOT(_q_navigateBackward()));

    forwardButton = new QToolButton(q);
    forwardButton->setObjectName(QLatin1String("qt_forward_button"));
    forwardButton->setIcon(q->style()->standardPixmap(QStyle::SP_ArrowForward));
#ifndef QT_NO_TOOLTIP
    forwardButton->setToolTip(QFileDialog::tr("Forward"));
#endif
    forwardButton->setAutoRaise(true);
    forwardButton->setEnabled(false);
    QObject::connect(forwardButton, SIGNAL(clicked()), q, SLOT(_q_navigateForward()));

    toParentButton = new QToolButton(q);
    toParentButton->setObjectName(QLatin1String("qt_toParent_button"));
    toParentButton->setIcon(q->style()->standardPixmap(QStyle::SP_FileDialogToParent));
#ifndef QT_NO_TOOLTIP
    toParentButton->setToolTip(QFileDialog::tr("Parent Directory"));
#endif
    toParentButton->setAutoRaise(true);
    toParentButton->setEnabled(false);
    QObject::connect(toParentButton, SIGNAL(clicked()), q, SLOT(_q_navigateToParent()));

    newFolderButton = new QPushButton(q);
    newFolderButton->setObjectName(QLatin1String("qt_new_folder_button"));
    newFolderButton->setText(QFileDialog::tr("New Folder"));
    newFolderButton->setEnabled(false);
    QObject::connect(newFolderButton, SIGNAL(clicked()), q, SLOT(_q_createDirectory()));

    listModeButton = new QToolButton(q);
    listModeButton->setObjectName(QLatin1String("qt_list_mode_button"));
    listModeButton->setIcon(q->style()->standardPixmap(QStyle::SP_FileDialogListView));
#ifndef QT_NO_TOOLTIP
    listModeButton->setToolTip(QFileDialog::tr("List View"));
#endif
    listModeButton->setAutoRaise(true);
    QObject::connect(listModeButton, SIGNAL(clicked()), q, SLOT(_q_showListView()));

    detailModeButton = new QToolButton(q);
    detailModeButton->setObjectName(QLatin1String("qt_detail_mode_button"));
    detailModeButton->setDown(true);
    detailModeButton->setIcon(q->style()->standardPixmap(QStyle::SP_FileDialogDetailedView));
#ifndef QT_NO_TOOLTIP
    detailModeButton->setToolTip(QFileDialog::tr("Detail View"));
#endif
    detailModeButton->setAutoRaise(true);
    QObject::connect(detailModeButton, SIGNAL(clicked()), q, SLOT(_q_showDetailsView()));

    QSize toolSize(fileNameEdit->sizeHint().height(), fileNameEdit->sizeHint().height());
    expandButton->setFixedSize(toolSize);
    backButton->setFixedSize(toolSize);
    listModeButton->setFixedSize(toolSize);
    detailModeButton->setFixedSize(toolSize);
    forwardButton->setFixedSize(toolSize);
    toParentButton->setFixedSize(toolSize);
}


/*!
    \internal

    Create actions which will be used in the right click.
*/
void QFileDialogPrivate::createMenuActions()
{
    Q_Q(QFileDialog);

    QAction *locationAction = new QAction(QFileDialog::tr("Go To The Folder"), q);
    QList<QKeySequence> shortcuts;
    shortcuts.append(Qt::CTRL + Qt::Key_L);
    shortcuts.append(Qt::CTRL + Qt::Key_G + Qt::SHIFT);
    locationAction->setShortcuts(shortcuts);
    QObject::connect(locationAction, SIGNAL(triggered()), q, SLOT(_q_chooseLocation()));
    q->addAction(locationAction);

    QAction *goHomeAction =  new QAction(QFileDialog::tr("Go To The Home Folder"), q);
    goHomeAction->setShortcut(Qt::CTRL + Qt::Key_H + Qt::SHIFT);
    QObject::connect(goHomeAction, SIGNAL(triggered()), q, SLOT(_q_goHome()));
    q->addAction(goHomeAction);

    // ### TODO add Desktop & Computer actions

    QAction *goToParent =  new QAction(QFileDialog::tr("Go To Parent Folder"), q);
    goToParent->setObjectName(QLatin1String("qt_goto_parent_action"));
    goToParent->setShortcut(Qt::CTRL + Qt::UpArrow);
    QObject::connect(goToParent, SIGNAL(triggered()), q, SLOT(_q_navigateToParent()));
    q->addAction(goToParent);

    openAction = new QAction(QFileDialog::tr("&Open"), q);
    openAction->setObjectName(QLatin1String("qt_open_action"));
    QObject::connect(openAction, SIGNAL(triggered()), q, SLOT(accept()));

    renameAction = new QAction(QFileDialog::tr("&Rename"), q);
    renameAction->setObjectName(QLatin1String("qt_rename_action"));
    QObject::connect(renameAction, SIGNAL(triggered()), q, SLOT(_q_renameCurrent()));

    deleteAction = new QAction(QFileDialog::tr("&Delete"), q);
    deleteAction->setObjectName(QLatin1String("qt_delete_action"));
    QObject::connect(deleteAction, SIGNAL(triggered()), q, SLOT(_q_deleteCurrent()));

    showHiddenAction = new QAction(QFileDialog::tr("Show &hidden files"), q);
    showHiddenAction->setObjectName(QLatin1String("qt_show_hidden_action"));
    showHiddenAction->setCheckable(true);
    QObject::connect(showHiddenAction, SIGNAL(triggered()), q, SLOT(_q_showHidden()));
}

void QFileDialogPrivate::_q_goHome()
{
    Q_Q(QFileDialog);
    q->setDirectory(QDir::homePath());
}

void QFileDialogPrivate::_q_chooseLocation()
{
    Q_Q(QFileDialog);
    bool ok;
    QString text = QInputDialog::getText(q, QFileDialog::tr("Open Location"),
                                         QFileDialog::tr("Go to the folder:"), QLineEdit::Normal,
                                         QDir::toNativeSeparators(q->directory().path()), &ok);
    if (ok && !text.isEmpty())
        q->setDirectory(text);
}

/*!
    \internal

    Update history with new path, buttons, and combo
*/
void QFileDialogPrivate::_q_pathChanged(const QString &newPath)
{
    Q_Q(QFileDialog);
    QDir dir(model->rootDirectory());
    toParentButton->setEnabled(dir.exists());
    backButton->setEnabled(!backHistory.isEmpty());
    forwardButton->setEnabled(!forwardHistory.isEmpty());
    newFolderButton->setEnabled(!model->isReadOnly());
    sidebar->selectUrl(QUrl::fromLocalFile(newPath));
    q->setHistory(history);

    // Ugly hack to workaround headerview
    if (treeView->header()->sectionSize(0) == 0) {
        QHeaderView::ResizeMode oldMode = treeView->header()->resizeMode(0);
        treeView->header()->setResizeMode(0, QHeaderView::ResizeToContents);
        treeView->header()->setResizeMode(0, oldMode);
    }
}

/*!
    \internal

    Navigates to the last directory viewed in the dialog.
*/
void QFileDialogPrivate::_q_navigateBackward()
{
    Q_Q(QFileDialog);
    if (!backHistory.isEmpty()) {
        QString lastDirectory = backHistory.takeLast();
        forwardHistory.append(model->filePath(rootIndex()));
        q->setDirectory(lastDirectory);
    }
}

/*!
    \internal

    Navigates to the last directory viewed in the dialog.
*/
void QFileDialogPrivate::_q_navigateForward()
{
    Q_Q(QFileDialog);
    if (!forwardHistory.isEmpty())
        q->setDirectory(forwardHistory.takeLast());
}

/*!
    \internal

    Navigates to the parent directory of the currently displayed directory
    in the dialog.
*/
void QFileDialogPrivate::_q_navigateToParent()
{
    Q_Q(QFileDialog);
    QDir dir(model->rootDirectory());
    if (dir.isRoot()) {
        q->setDirectory(QFileSystemModel::tr("My Computer"));
    } else {
        dir.cdUp();
        q->setDirectory(dir.absolutePath());
    }
}

/*!
    \internal

    Creates a new directory, first asking the user for a suitable name.
*/
void QFileDialogPrivate::_q_createDirectory()
{
    Q_Q(QFileDialog);
    listView->clearSelection();

    QString newFolderString = QFileDialog::tr("New Folder");
    QString folderName = newFolderString;
    QString prefix = q->directory().absolutePath() + QDir::separator();
    if (QFile::exists(prefix + folderName)) {
        qlonglong suffix = 2;
        while (QFile::exists(prefix + folderName)) {
            folderName = newFolderString + QString::number(suffix++);
        }
    }

    QModelIndex parent = rootIndex();
    QModelIndex index = model->mkdir(parent, folderName);
    if (!index.isValid())
        return;
    listView->setCurrentIndex(index);
    treeView->setFocus();
    if (q->viewMode() == QFileDialog::List)
        listView->edit(index);
    else
        treeView->edit(index);
}

void QFileDialogPrivate::_q_showListView()
{
    listModeButton->setDown(true);
    detailModeButton->setDown(false);
    listView->doItemsLayout();
    stackedWidget->setCurrentWidget(listView);
}

void QFileDialogPrivate::_q_showDetailsView()
{
    listModeButton->setDown(false);
    detailModeButton->setDown(true);
    treeView->doItemsLayout();
    stackedWidget->setCurrentWidget(treeView);
}

/*!
    \internal

    Show the context menu for the file/dir under position
*/
void QFileDialogPrivate::_q_showContextMenu(const QPoint &position)
{
#ifdef QT_NO_MENU
    Q_UNUSED(position);
#else
    Q_Q(QFileDialog);
    QAbstractItemView *view = 0;
    if (q->viewMode() == QFileDialog::Detail)
        view = treeView;
    else
        view = listView;
    QModelIndex index = view->indexAt(position);
    index = index.sibling(index.row(), 0);

    QMenu menu(view);
    if (index.isValid()) {
        // file context menu
        menu.addAction(openAction);
        menu.addSeparator();
        menu.addAction(renameAction);
        menu.addAction(deleteAction);
        menu.addSeparator();
    }
    menu.addAction(showHiddenAction);
    menu.exec(view->viewport()->mapToGlobal(position));
#endif // QT_NO_MENU
}

/*!
    \internal
*/
void QFileDialogPrivate::_q_renameCurrent()
{
    Q_Q(QFileDialog);
    QModelIndex index = listView->currentIndex();
    index = index.sibling(index.row(), 0);
    if (q->viewMode() == QFileDialog::List)
        listView->edit(index);
    else
        treeView->edit(index);
}

/*!
    \internal

    Deletes the currently selected item in the dialog.
*/
void QFileDialogPrivate::_q_deleteCurrent()
{
    Q_Q(QFileDialog);
    QModelIndex index = listView->currentIndex();
    index = index.sibling(index.row(), 0);
    if (!index.isValid() || model->isReadOnly())
        return;

    QString fileName = model->fileName(index);
#ifndef QT_NO_MESSAGEBOX
    if (!model->fileInfo(index).isWritable()
        && (QMessageBox::warning(q_func(), q_func()->windowTitle(),
                                QFileDialog::tr("'%1' is write protected.\nDo you want to delete it anyway?")
                                .arg(fileName),
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No))
        return;
    else if (QMessageBox::warning(q_func(), q_func()->windowTitle(),
                                  QFileDialog::tr("Are sure you want to delete '%1'?")
                                  .arg(fileName),
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
        return;
#else
    if (!model->fileInfo(index).isWritable())
        return;
#endif // QT_NO_MESSAGEBOX

    if (model->isDir(index) && !model->rmdir(index)) {
#ifndef QT_NO_MESSAGEBOX
        QMessageBox::warning(q, q->windowTitle(),
                            QFileDialog::tr("Could not delete directory."));
#endif
    } else {
        model->remove(index);
    }
}

void QFileDialogPrivate::_q_autoCompleteFileName(const QString &text) {
    QModelIndex idx = model->index(text);
    if (!idx.isValid())
        idx = model->index(model->rootPath() + QDir::separator() + text);
    if (listView->selectionModel()->isSelected(idx) || listView->currentIndex() == idx)
        return;
    if (!idx.isValid())
        listView->selectionModel()->clear();
    else
        listView->setCurrentIndex(idx);
}

/*!
    \internal
*/
void QFileDialogPrivate::_q_updateOkButton() {
    Q_Q(QFileDialog);
    QPushButton *button =  buttonBox->button((acceptMode == QFileDialog::AcceptOpen)
                    ? QDialogButtonBox::Open : QDialogButtonBox::Save);
    if (!button)
        return;

    QModelIndex index = listView->selectionModel()->currentIndex();
    if (q->selectedFiles().isEmpty() || fileMode != QFileDialog::AnyFile && !index.isValid()) {
        button->setEnabled(false);
        return;
    }

    switch (fileMode) {
    case QFileDialog::DirectoryOnly:
    case QFileDialog::Directory:
        button->setEnabled(model->isDir(index));
        break;
    case QFileDialog::ExistingFile:
    case QFileDialog::ExistingFiles:
        button->setEnabled(!model->isDir(index));
    case QFileDialog::AnyFile:
        button->setEnabled(!model->isDir(index) || !lineEdit()->text().isEmpty());
    }
}

/*!
    \internal
*/
void QFileDialogPrivate::_q_currentChanged(const QModelIndex &index)
{
    _q_updateOkButton();
    emit q_func()->currentChanged(model->filePath(index));
}

/*!
    \internal

    This is called when the user double clicks on a file with the corresponding
    model item \a index.
*/
void QFileDialogPrivate::_q_enterDirectory(const QModelIndex &index)
{
    Q_Q(QFileDialog);
    // My Computer or a directory
    QString path = model->filePath(index);
    if (path.isEmpty() || model->isDir(index)) {
        q->setDirectory(path);
    } else {
        q->accept();
    }
}

/*!
    \internal

    Changes the file dialog's current directory to the one specified
    by \a path.
*/
void QFileDialogPrivate::_q_goToDirectory(const QString &path)
{
 #ifndef QT_NO_MESSAGEBOX
    Q_Q(QFileDialog);
#endif
    QModelIndex index = lookInCombo->model()->index(lookInCombo->currentIndex(),
                                                    lookInCombo->modelColumn(),
                                                    lookInCombo->rootModelIndex());
    QString path2 = path;
    if (!index.isValid())
        index = model->index(getEnvironmentVariable(path));
    else {
        path2 = index.data(UrlRole).toUrl().path();
        index = model->index(path2);
    }
    QDir dir(path2);
    if (!dir.exists())
        dir = getEnvironmentVariable(path2);

    if (dir.exists() || path2.isEmpty() || path2 == QFileDialog::tr("My Computer")) {
        _q_enterDirectory(index);
#ifndef QT_NO_MESSAGEBOX
    } else {
        QString message = QFileDialog::tr("%1\nDirectory not found.\nPlease verify the "
                                          "correct directory name was given.");
        QMessageBox::warning(q, q->windowTitle(), message.arg(path2));
#endif // QT_NO_MESSAGEBOX
    }
}

const char *qt_file_dialog_filter_reg_exp =
    "([a-zA-Z0-9]*)\\(([a-zA-Z0-9_.*? +;#\\-\\[\\]@\\{\\}/!<>\\$%&=^~:\\|]*)\\)$";

// Makes a list of filters from a normal filter string "Image Files (*.png *.jpg)"
static QStringList qt_clean_filter_list(const QString &filter)
{
    QRegExp regexp(QString::fromLatin1(qt_file_dialog_filter_reg_exp));
    QString f = filter;
    int i = regexp.indexIn(f);
    if (i >= 0)
        f = regexp.cap(2);
    return f.split(QLatin1Char(' '), QString::SkipEmptyParts);
}

/*!
    \internal

    Sets the current name filter to be nameFilter and
    update the fileNameEdit when in AcceptSave mode with the new extension.
*/
void QFileDialogPrivate::_q_useNameFilter(const QString &nameFilter)
{
    QStringList newNameFilters = qt_clean_filter_list(nameFilter);
    if (acceptMode == QFileDialog::AcceptSave) {
        QString newNameFilterExtension;
        if (newNameFilters.count() > 0)
            newNameFilterExtension = QFileInfo(newNameFilters.at(0)).suffix();

        QString fileName = lineEdit()->text();
        const QString fileNameExtension = QFileInfo(fileName).suffix();
        if (!fileNameExtension.isEmpty() && !newNameFilterExtension.isEmpty()) {
            const int fileNameExtensionLength = fileNameExtension.count();
            fileName.replace(fileName.count() - fileNameExtensionLength,
                             fileNameExtensionLength, newNameFilterExtension);
            lineEdit()->setText(fileName);
        }
    }

    model->setNameFilters(newNameFilters);
}

/*!
    \internal

    This is called when the model index corresponding to the current file is changed
    from \a index to \a current.
*/
void QFileDialogPrivate::_q_selectionChanged()
{
    if (fileNameEdit->hasFocus())
        return; // the selection changed because of autocompletion

    QModelIndexList indexes = listView->selectionModel()->selectedRows();
    bool stripDirs = (fileMode != QFileDialog::DirectoryOnly && fileMode != QFileDialog::Directory);
    bool addQuotes = indexes.count() > 1;
    QString allFiles;
    for (int i = 0; i < indexes.count(); ++i) {
        if (stripDirs && model->isDir(indexes.at(i)))
            continue;
        QString fileName = model->data(indexes.at(i)).toString();
        if (addQuotes)
            fileName = (QLatin1Char('"')) + fileName + QLatin1String("\" ");
        allFiles.append(fileName);
    }
    if (!allFiles.isEmpty() && !lineEdit()->hasFocus())
        lineEdit()->setText(allFiles);
}

/*!
    \internal

    Includes hidden files and directories in the items displayed in the dialog.
*/
void QFileDialogPrivate::_q_showHidden()
{
    QDir::Filters dirFilters = model->filter();
    if (showHiddenAction->isChecked())
        dirFilters |= QDir::Hidden;
    else
        dirFilters &= ~(int)QDir::Hidden;
    model->setFilter(dirFilters);
}

/*!
    \internal

    For the list and tree view watch keys to goto parent and back in the history

    returns true if handled
*/
bool QFileDialogPrivate::itemViewKeyboardEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Backspace:
        _q_navigateToParent();
        return true;
    case Qt::Key_Back:
#ifdef QT_KEYPAD_NAVIGATION
        if (QApplication::keypadNavigationEnabled())
            return false;
#endif
    case Qt::Key_Left:
        if (event->key() == Qt::Key_Back || event->modifiers() == Qt::AltModifier) {
            _q_navigateBackward();
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}

QString QFileDialogPrivate::getEnvironmentVariable(const QString &string)
{
#ifdef Q_OS_UNIX
    if (string.size() > 1 && string.startsWith(QLatin1Char('$'))) {
        return QString::fromLocal8Bit(getenv(string.mid(1).toLatin1().constData()));
    }
#else
    if (string.size() > 2 && string.startsWith(QLatin1Char('%')) && string.endsWith(QLatin1Char('%'))) {
        return QString::fromLocal8Bit(qgetenv(string.mid(1, string.size() - 2).toLatin1().constData()));
    }
#endif
    return string;
}

QFileDialogListView::QFileDialogListView(QFileDialogPrivate *d_pointer)
    : QListView(qobject_cast<QWidget*>(d_pointer->q_ptr)), d_ptr(d_pointer)
{
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setWrapping(true);
    setResizeMode(QListView::Adjust);
    setEditTriggers(QAbstractItemView::EditKeyPressed);
    setContextMenuPolicy(Qt::CustomContextMenu);
#ifndef QT_NO_DRAGANDDROP
    setDragDropMode(QAbstractItemView::DragOnly);
#endif
}

QFileDialogTreeView::QFileDialogTreeView(QFileDialogPrivate *d_pointer)
    : QTreeView(qobject_cast<QWidget*>(d_pointer->q_ptr)), d_ptr(d_pointer)
{
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setRootIsDecorated(false);
    setItemsExpandable(false);
    setSortingEnabled(true);
    header()->setSortIndicator(0, Qt::AscendingOrder);
    header()->setStretchLastSection(true);
    header()->resizeSection(0, 0);
    setEditTriggers(QAbstractItemView::EditKeyPressed);
    setContextMenuPolicy(Qt::CustomContextMenu);
#ifndef QT_NO_DRAGANDDROP
    setDragDropMode(QAbstractItemView::DragOnly);
#endif
}

QSize QFileDialogTreeView::sizeHint() const
{
    int height = sizeHintForRow(0);
    QSize sizeHint = QTreeView::sizeHint();
    if (height == -1)
        return sizeHint;
    return QSize(sizeHint.width(), height * 10);
}

/*!
    // FIXME: this is a hack to avoid propagating key press events
    // to the dialog and from there to the "Ok" button

*/
void QFileDialogLineEdit::keyPressEvent(QKeyEvent *e)
{
    key = e->key();
    QLineEdit::keyPressEvent(e);
    if (key != Qt::Key_Escape)
        e->accept();
}

QString QFSCompletor::pathFromIndex(const QModelIndex &index) const
{
    const QFileSystemModel *dirModel = static_cast<const QFileSystemModel *>(model());
    if (dirModel->filePath(index.parent()) == dirModel->rootPath())
        return dirModel->fileName(index);
    return dirModel->filePath(index);
}

QStringList QFSCompletor::splitPath(const QString &path) const
{
    if (path.isEmpty())
        return QStringList(completionPrefix());

    QString pathCopy = QDir::toNativeSeparators(path);
    QString sep = QDir::separator();
#ifdef Q_OS_WIN
    if (pathCopy == QLatin1String("\\") || pathCopy == QLatin1String("\\\\"))
        return QStringList(pathCopy);
    QString doubleSlash(QLatin1String("\\\\"));
    if (pathCopy.startsWith(doubleSlash))
        pathCopy = pathCopy.mid(2);
    else
        doubleSlash.clear();
#endif

    QRegExp re(QLatin1String("[") + QRegExp::escape(sep) + QLatin1String("]"));
    QStringList parts = pathCopy.split(re);

#ifdef Q_OS_WIN
    if (!doubleSlash.isEmpty())
        parts[0].prepend(doubleSlash);
#else
    if (path[0] == sep[0]) // read the "/" at the beginning as the split removed it
        parts[0] = sep[0];
#endif

    if (parts.count() == 1 || (parts.count() > 1 && path[0] != sep[0])) {
        const QFileSystemModel *dirModel = static_cast<const QFileSystemModel *>(model());
        QString currentLocation = dirModel->rootPath();
        if (currentLocation.contains(sep))
            return splitPath(currentLocation + sep + path);
    }
    return parts;
}

#include "moc_qfiledialog.cpp"

