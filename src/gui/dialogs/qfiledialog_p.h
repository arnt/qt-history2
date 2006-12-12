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
#include "qsidebar_p.h"
#include <qurl.h>
#include <qstackedwidget.h>
#include <qsplitter.h>
#include <qdialogbuttonbox.h>
#include <qcompleter.h>
#include <qtimeline.h>
#include <qstandarditemmodel.h>

class QFileDialogListView;
class QFileDialogTreeView;
class QFileDialogLineEdit;
class QGridLayout;
class QCompleter;
class QHBoxLayout;

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
    fileNameLabel(0),
    fileNameEdit(0),
    expandButton(0),
    line(0),
    backButton(0),
    forwardButton(0),
    toParentButton(0),
    detailModeButton(0),
    listModeButton(0),
    lookInLabel(0),
    lookInCombo(0),
    bottomRightSpacer(0),
    splitter(0),
    sidebar(0),
    stackedWidget(0),
    listView(0),
    treeView(0),
    fileTypeLabel(0),
    fileTypeCombo(0),
    newFolderButton(0),
    buttonBox(0),
    model(0),
    fileMode(QFileDialog::AnyFile),
    acceptMode(QFileDialog::AcceptOpen),
    confirmOverwrite(true),
    renameAction(0),
    deleteAction(0),
    showHiddenAction(0),
    saveState(false)
    {};

    void createToolButtons();
    void createMenuActions();
    void createWidgets();
    void layout();

    void init(const QString &directory = QString(), const QString &nameFilter = QString());
    bool itemViewKeyboardEvent(QKeyEvent *event);
    QString getEnvironmentVariable(const QString &string);
    static QString workingDirectory(const QString &path);
    static QString initialSelection(const QString &path);
    void updateFileTypeVisibility();
    QStringList typedFiles() const;

    inline QModelIndex rootIndex() const;

    QLineEdit *lineEdit() const {
        if (acceptMode == QFileDialog::AcceptSave)
            return (QLineEdit*)fileNameEdit;
        return lookInCombo->lineEdit();
    }

    // return true if the combo line edit text is not the currently selected item
    // (i.e. no user input is contained there)
    bool comboLineEditChanged() const {
        return (lookInCombo->itemText(lookInCombo->currentIndex()) != lookInCombo->currentText());
    }

    static inline QDir::Filters filterForMode(QFileDialog::FileMode mode)
    {
        if (mode == QFileDialog::DirectoryOnly)
            return QDir::Drives | QDir::AllDirs | QDir::NoDotAndDotDot;
        return QDir::Drives | QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot;
    }

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

    void _q_chooseLocation();
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
    void _q_animateDialog();
    void _q_animateDialogV(int);
    void _q_animateDialogH(int);
    void _q_autoCompleteFileName(const QString &);

    void addUrls(const QList<QUrl> &list, int row);
    void setUrl(const QModelIndex &row, const QUrl & url);
    void _q_layoutChanged();

    // layout
    QLabel *fileNameLabel;
    QFileDialogLineEdit *fileNameEdit;
    QToolButton *expandButton;
    QFrame *line;
    QToolButton *backButton;
    QToolButton *forwardButton;
    QToolButton *toParentButton;
    QToolButton *detailModeButton;
    QToolButton *listModeButton;
    QLabel *lookInLabel;
    QComboBox *lookInCombo;
    QWidget *bottomRightSpacer;
    QSplitter *splitter;
    QSidebar *sidebar;
    QStackedWidget *stackedWidget;
    QFileDialogListView *listView;
    QFileDialogTreeView *treeView;
    QLabel *fileTypeLabel;
    QComboBox *fileTypeCombo;
    QPushButton *newFolderButton;
    QDialogButtonBox *buttonBox;

    // data
    QStringList watching;
    QFileSystemModel *model;
    QCompleter *completer;
    QCompleter *comboCompleter;

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

    QTimeLine *vTimeLine;
    QTimeLine *hTimeLine;
    QSize oldSize;
    bool saveState;
};

class QFileDialogListView : public QListView
{
public:
    QFileDialogListView(QFileDialogPrivate *d_pointer);
protected:
    void keyPressEvent(QKeyEvent *e)
    {
        if (d_ptr->itemViewKeyboardEvent(e)) {
            e->accept();
        } else {
            QListView::keyPressEvent(e);
        }
    }
private:
    QFileDialogPrivate *d_ptr;
};

inline QModelIndex QFileDialogPrivate::rootIndex() const { return listView->rootIndex(); }

class QFileDialogTreeView : public QTreeView
{
public:
    QFileDialogTreeView(QFileDialogPrivate *d_pointer);
protected:
    void keyPressEvent(QKeyEvent *e)
    {
        if (d_ptr->itemViewKeyboardEvent(e)) {
            e->accept();
        } else {
            QTreeView::keyPressEvent(e);
        }
    }
    QSize sizeHint() const;
private:
    QFileDialogPrivate *d_ptr;
};

class ComboLineEdit : public QLineEdit
{
public:
    ComboLineEdit(QWidget *parent)
        : QLineEdit(parent){}
protected:
    void mousePressEvent ( QMouseEvent * event ) {
        QLineEdit::mousePressEvent(event);
        clear();
    }
};


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

