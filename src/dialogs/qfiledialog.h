/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qfiledialog.h#54 $
**
** Definition of QFileDialog class
**
** Created : 950428
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QFILEDIALOG_H
#define QFILEDIALOG_H

struct QFileDialogPrivate;
class QPushButton;
class QLabel;
class QWidget;
class QFileDialog;
class QTimer;

#ifndef QT_H
#include "qdir.h"
#include "qdialog.h"
#include "qlistbox.h"
#include "qlineedit.h"
#include "qlistview.h"
#endif // QT_H


class Q_EXPORT QFileIconProvider : public QObject
{
    Q_OBJECT
public:
    QFileIconProvider( QObject * parent = 0, const char* name = 0 );

    virtual const QPixmap * pixmap( const QFileInfo & );
};

class QRenameEdit : public QLineEdit
{
    Q_OBJECT

public:
    QRenameEdit( QWidget *parent )
        : QLineEdit( parent )
    {}

protected:
    void keyPressEvent( QKeyEvent *e );
    void focusOutEvent( QFocusEvent *e );

signals:
    void escapePressed();

};


class QFileListBox : public QListBox
{
    friend class QFileDialog;

    Q_OBJECT

private:
    QFileListBox( QWidget *parent, QFileDialog *d );

    void clear();
    void show();
    void startRename( bool check = TRUE );
    void setSelected( QListBoxItem *i, bool s );
    void setSelected( int i, bool s );

private:
    void viewportMousePressEvent( QMouseEvent *e );
    void viewportMouseDoubleClickEvent( QMouseEvent *e );
    void keyPressEvent( QKeyEvent *e );

private slots:
    void rename();
    void cancelRename();

private slots:
    void doubleClickTimeout();

private:
    QRenameEdit *lined;
    QFileDialog *filedialog;
    bool renaming;
    QTimer* renameTimer;
    QListBoxItem *renameItem;

};


class QFileListView : public QListView
{
    friend class QFileDialog;

    Q_OBJECT

private:
    QFileListView( QWidget *parent, QFileDialog *d );

    void clear();
    void startRename( bool check = TRUE );

private:
    void viewportMousePressEvent( QMouseEvent *e );
    void viewportMouseDoubleClickEvent( QMouseEvent *e );
    void keyPressEvent( QKeyEvent *e );

private slots:
    void rename();
    void cancelRename();

private slots:
    void doubleClickTimeout();

private:
    QRenameEdit *lined;
    QFileDialog *filedialog;
    bool renaming;
    QTimer* renameTimer;
    QListViewItem *renameItem;

};


class Q_EXPORT QFileDialog : public QDialog
{
    friend class QFileListBox;

    Q_OBJECT
public:
    QFileDialog( const QString& dirName, const QString& filter = QString::null,
                 QWidget *parent=0, const char *name=0, bool modal=FALSE );
    QFileDialog( QWidget *parent=0, const char *name=0, bool modal=FALSE );
    ~QFileDialog();

    // recommended static functions

    static QString getOpenFileName( const QString &initially = QString::null,
                                    const QString &filter= QString::null,
                                    QWidget *parent = 0, const char* name = 0);
    static QString getSaveFileName( const QString &initially = QString::null,
                                    const QString &filter= QString::null,
                                    QWidget *parent = 0, const char* name = 0);
    static QString getExistingDirectory( const QString &dir = QString::null,
                                         QWidget *parent = 0,
                                         const char* name = 0 );
    static QStringList getOpenFileNames( const QString &filter= QString::null,
                                         const QString &dir = QString::null,
                                         QWidget *parent = 0,
                                         const char* name = 0);

    // other static functions

    static void setIconProvider( QFileIconProvider * );
    static QFileIconProvider * iconProvider();

    // non-static function for special needs

    QString selectedFile() const;
    void setSelection( const QString &);

    QString dirPath() const;

    void setDir( const QDir & );
    const QDir *dir() const;

    void rereadDir();

    enum Mode { AnyFile, ExistingFile, Directory, ExistingFiles };
    void setMode( Mode );
    Mode mode() const;

    bool eventFilter( QObject *, QEvent * );

public slots:
    void setDir( const QString& );
    void setFilter( const QString& );
    void setFilters( const char ** );
    void setFilters( const QStringList& );

signals:
    void fileHighlighted( const QString& );
    void fileSelected( const QString& );
    void dirEntered( const QString& );

private slots:
    void fileSelected( int );
    void fileHighlighted( int );
    void dirSelected( int );
    void pathSelected( int );

    void updateFileNameEdit( QListViewItem *);
    void selectDirectoryOrFile( QListViewItem * );
    void popupContextMenu( QListViewItem *, const QPoint &, int );
    void popupContextMenu( QListBoxItem *, const QPoint & );
    void updateFileNameEdit( QListBoxItem *);
    void selectDirectoryOrFile( QListBoxItem * );
    void fileNameEditDone();

    void okClicked();
    void filterClicked(); // not used
    void cancelClicked();

    void cdUpClicked();
    void newFolderClicked();

    void fixupNameEdit();

protected:
    void resizeEvent( QResizeEvent * );
    void keyPressEvent( QKeyEvent * );

    void addWidgets( QLabel *, QWidget *, QPushButton * );

private slots:
    void updateGeometries();
    void modeButtonsDestroyed();

private:
    enum PopupAction {
        PA_Open = 0,
        PA_Delete,
        PA_Rename,
        PA_SortAscent,
        PA_SortDescent,
        PA_Cancel
    };

    void init();
    bool trySetSelection( const QFileInfo&, bool );
    void deleteFile( const QString &filename );
    void popupContextMenu( const QString &filename, bool withSort,
                           PopupAction &action, const QPoint &p );

    QDir cwd;
    QString fileName;

    QFileDialogPrivate *d;
    QFileListView  *files;

    QLineEdit  *nameEdit; // also filter
    void *unused1;
    void *unused2;
    void *unused3;
    void *unused4;
    void *unused5;
    void *unused6;
    void *unused7;
    QPushButton *okB;
    QPushButton *cancelB;

#if defined(_WS_WIN_)
    static QString winGetOpenFileName( const QString &initialSelection,
                                       const QString &filter,
                                       QString* workingDirectory,
                                       QWidget *parent = 0,
                                       const char* name = 0 );
    static QString winGetSaveFileName( const QString &initialSelection,
                                       const QString &filter,
                                       QString* workingDirectory,
                                       QWidget *parent = 0,
                                       const char* name = 0 );
    static QStringList winGetOpenFileNames( const QString &filter,
                                            QString* workingDirectory,
                                            QWidget *parent = 0,
                                            const char* name = 0 );
#endif

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QFileDialog( const QFileDialog & );
    QFileDialog &operator=( const QFileDialog & );
#endif
};


#endif // QFILEDIALOG_H
