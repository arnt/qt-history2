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

#ifndef QFILEDIALOG_P_H
#define QFILEDIALOG_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <private/qdialog_p.h>
#include <qitemselectionmodel.h>
#include <qabstractitemview.h>
#include <qtoolbutton.h>

class QListView;
class QTreeView;
class QDirModel;
class QComboBox;
class QAction;
class QPushButton;
class QFileDialogLineEdit;
class QGridLayout;

class QFileDialogPrivate : public QDialogPrivate
{
    Q_DECLARE_PUBLIC(QFileDialog)
public:
    QFileDialogPrivate();

    // private slots
    void reload();
    void navigateToPrevious();
    void navigateToParent();
    void enterDirectory(const QModelIndex &index);
    void enterDirectory(const QString &path);
    void enterDirectory();
    void showList();
    void showDetails();
    void showHidden();
    void useFilter(const QString &filter);
    void updateFileName(const QItemSelection &selection);
    void autoCompleteFileName(const QString &text);
    void autoCompleteDirectory(const QString &text);
    void showContextMenu(const QPoint &pos);
    void createDirectory();
    void renameCurrent();
    void deleteCurrent();
    void sortByName();
    void sortBySize();
    void sortByDate();
    void setUnsorted();

    // setup
    void setup(const QString &directory, const QStringList &nameFilter);
    void setupActions();
    void setupListView(const QModelIndex &index, QGridLayout *grid);
    void setupTreeView(const QModelIndex &index, QGridLayout *grid);
    void setupToolButtons(const QModelIndex &index, QGridLayout *grid);
    void setupWidgets(QGridLayout *grid);

    // other
    void updateButtons(const QModelIndex &index);
    void setRootIndex(const QModelIndex &index);
    QModelIndex rootIndex() const;
    void setDirSorting(QDir::SortFlags sort);
    void setDirFilter(QDir::Filters filter);
    QDir::Filters filterForMode(QFileDialog::FileMode mode);
    QAbstractItemView::SelectionMode selectionMode(QFileDialog::FileMode mode);
    QModelIndex matchDir(const QString &text, const QModelIndex &first) const;
    QModelIndex matchName(const QString &name, const QModelIndex &first) const;

    // inlined stuff
    inline QString tr(const char *text) const { return QObject::tr(text); }
    inline QString toNative(const QString &path) const
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
    inline QFileDialog::ViewMode viewMode() const
        { return (listMode->isDown() ? QFileDialog::List : QFileDialog::Detail); }

    // static stuff
    static QString encodeFileName(const QString &filename);
    static QString workingDirectory(const QString &path, bool encode = true);
    static QString initialSelection(const QString &path, bool encode = true);

    // data
    QDirModel *model;
    QItemSelectionModel *selections;
    QListView *listView;
    QTreeView *treeView;
    
    QFileDialog::FileMode fileMode;
    QFileDialog::AcceptMode acceptMode;
    bool confirmOverwrite;
    QString defaultSuffix;

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

struct QFileDialogArgs
{
    QFileDialogArgs() : parent(0), mode(QFileDialog::AnyFile) {}

    QWidget *parent;
    QString caption;
    QString directory;
    QString selection;
    QString filter;
    QFileDialog::FileMode mode;
    QFileDialog::Options options;
};

#endif // QFILEDIALOG_P_H
