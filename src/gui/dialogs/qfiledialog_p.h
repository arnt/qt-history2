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

#ifndef QT_NO_FILEDIALOG

#include "qfiledialog.h"
#include "private/qdialog_p.h"


#include "qfilesystemmodel_p.h"
#include <qlistview.h>
#include <qtreeview.h>
#include <qcombobox.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <qevent.h>
#include <qlineedit.h>
#include <qurl.h>
#include <qstackedwidget.h>
#include <qdialogbuttonbox.h>
#include <qabstractproxymodel.h>
#include <qcompleter.h>
#include <qtimeline.h>
#include <qdebug.h>
#include "qsidebar_p.h"

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC) && !defined(Q_OS_OPENBSD)
#include <sys/statvfs.h>
#endif

class QFileDialogListView;
class QFileDialogTreeView;
class QFileDialogLineEdit;
class QGridLayout;
class QCompleter;
class QHBoxLayout;
class Ui_QFileDialog;

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

#define UrlRole (Qt::UserRole + 1)

class QFileDialogPrivate : public QDialogPrivate
{
    Q_DECLARE_PUBLIC(QFileDialog)

public:
    QFileDialogPrivate() :
    urlModel(0),
    proxyModel(0),
    model(0),
    fileMode(QFileDialog::AnyFile),
    acceptMode(QFileDialog::AcceptOpen),
    confirmOverwrite(true),
    renameAction(0),
    deleteAction(0),
    showHiddenAction(0),
    useDefaultCaption(true),
    defaultFileTypes(true),
    qFileDialogUi(0)
    {};

    void createToolButtons();
    void createMenuActions();
    void createWidgets();

    void init(const QString &directory = QString(), const QString &nameFilter = QString(),
              const QString &caption = QString());
    bool itemViewKeyboardEvent(QKeyEvent *event);
    QString getEnvironmentVariable(const QString &string);
    static QString workingDirectory(const QString &path);
    static QString initialSelection(const QString &path);
    QStringList typedFiles() const;

    inline QModelIndex mapToSource(const QModelIndex &index) const;
    inline QModelIndex mapFromSource(const QModelIndex &index) const;
    inline QModelIndex rootIndex() const;
    inline void setRootIndex(const QModelIndex &index) const;
    inline QModelIndex select(const QModelIndex &index) const;
    inline QString rootPath() const;

    QLineEdit *lineEdit() const;

    int maxNameLength(const QString &path) {
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC) && !defined(Q_OS_OPENBSD)
        struct statvfs vfs;
        if (statvfs(path.toLocal8Bit().constData(), &vfs) >= 0)
            return vfs.f_namemax;
#elif defined(Q_OS_WIN)
        DWORD maxLength;
        QString drive = path.left(3);
        if (QT_WA_INLINE(::GetVolumeInformationW(reinterpret_cast<const WCHAR *>(path.utf16()), NULL, 0, NULL, &maxLength, NULL, NULL, 0),
                         ::GetVolumeInformationA(path.toAscii().constData(), NULL, 0, NULL, &maxLength, NULL, NULL, 0)) == FALSE)
            return -1;
        return maxLength;
#endif
        return -1;
    }

    QString basename(const QString &path)
    {
        int separator = path.lastIndexOf(QDir::separator());
        if (separator != -1)
            return path.mid(separator + 1);
        return path;
    }

    static inline QDir::Filters filterForMode(QFileDialog::FileMode mode)
    {
        if (mode == QFileDialog::DirectoryOnly)
            return QDir::Drives | QDir::AllDirs | QDir::NoDotAndDotDot;
        return QDir::Drives | QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot;
    }

    QAbstractItemView *currentView() const;

    static inline QString toInternal(const QString &path)
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

    void retranslateWindowTitle();
    void retranslateStrings();

    void _q_goHome();
    void _q_pathChanged(const QString &);
    void _q_navigateBackward();
    void _q_navigateForward();
    void _q_navigateToParent();
    void _q_createDirectory();
    void _q_showListView();
    void _q_showDetailsView();
    void _q_showContextMenu(const QPoint &position);
    void _q_renameCurrent();
    void _q_deleteCurrent();
    void _q_showHidden();
    void _q_showHeader(QAction *);
    void _q_updateOkButton();
    void _q_currentChanged(const QModelIndex &index);
    void _q_enterDirectory(const QModelIndex &index);
    void _q_goToDirectory(const QString &);
    void _q_useNameFilter(const QString &nameFilter);
    void _q_selectionChanged();
    void _q_goToUrl(const QUrl &url);
    void _q_autoCompleteFileName(const QString &);
    void _q_rowsInserted(const QModelIndex & parent);

    // layout
    QUrlModel *urlModel;
    QAbstractProxyModel *proxyModel;

    // data
    QStringList watching;
    QFileSystemModel *model;
    QCompleter *completer;

    QFileDialog::FileMode fileMode;
    QFileDialog::AcceptMode acceptMode;
    bool confirmOverwrite;
    QString defaultSuffix;

    QStringList history;
    QStringList backHistory;
    QStringList forwardHistory;

    QAction *renameAction;
    QAction *deleteAction;
    QAction *showHiddenAction;
    QAction *newFolderAction;

    bool useDefaultCaption;
    bool defaultFileTypes;

    Ui_QFileDialog *qFileDialogUi;
};

class QFileDialogLineEdit : public QLineEdit
{
public:
    QFileDialogLineEdit(QWidget *parent = 0) : QLineEdit(parent), hideOnEsc(false), d_ptr(0){}
    void init(QFileDialogPrivate *d_pointer) {d_ptr = d_pointer; }
    void keyPressEvent(QKeyEvent *e);
    bool hideOnEsc;
private:
    QFileDialogPrivate *d_ptr;
};

class QFileDialogListView : public QListView
{
public:
    QFileDialogListView(QWidget *parent = 0);
    void init(QFileDialogPrivate *d_pointer);
    QSize sizeHint() const;
protected:
    void keyPressEvent(QKeyEvent *e);
private:
    QFileDialogPrivate *d_ptr;
};

class QFileDialogTreeView : public QTreeView
{
public:
    QFileDialogTreeView(QWidget *parent);
    void init(QFileDialogPrivate *d_pointer);
    QSize sizeHint() const;

    void selectAnyIndex() {
        QModelIndex idx = moveCursor(QAbstractItemView::MoveDown, Qt::NoModifier);
        if (!idx.isValid())
            idx = model()->index(0, 0, rootIndex());
        selectionModel()->select(idx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }

protected:
    void keyPressEvent(QKeyEvent *e);
private:
    QFileDialogPrivate *d_ptr;
};

inline QModelIndex QFileDialogPrivate::mapToSource(const QModelIndex &index) const {
    return proxyModel ? proxyModel->mapToSource(index) : index;
}
inline QModelIndex QFileDialogPrivate::mapFromSource(const QModelIndex &index) const {
    return proxyModel ? proxyModel->mapFromSource(index) : index;
}

inline QString QFileDialogPrivate::rootPath() const {
    return model->rootPath();
}

/*!
    QCompleter that can deal with QFileSystemModel
  */
class QFSCompletor :  public QCompleter {
public:
    QFSCompletor(QAbstractItemModel *model, QObject *parent = 0) : QCompleter(model, parent){}
    QString pathFromIndex(const QModelIndex &index) const;
    QStringList splitPath(const QString& path) const;
};

#endif

#endif
