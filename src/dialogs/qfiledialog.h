/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qfiledialog.h#70 $
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
#include "qurl.h"
#include "qurlinfo.h"
#endif // QT_H


class Q_EXPORT QFileIconProvider : public QObject
{
    Q_OBJECT
public:
    QFileIconProvider( QObject * parent = 0, const char* name = 0 );

    virtual const QPixmap * pixmap( const QFileInfo & );
    virtual const QPixmap * pixmap( const QUrlInfo & );
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
    void viewportMousePressEvent( QMouseEvent *e );
    void viewportMouseReleaseEvent( QMouseEvent *e );
    void viewportMouseDoubleClickEvent( QMouseEvent *e );
    void viewportMouseMoveEvent( QMouseEvent *e );
    void viewportDragEnterEvent( QDragEnterEvent *e );
    void viewportDragMoveEvent( QDragMoveEvent *e );
    void viewportDragLeaveEvent( QDragLeaveEvent *e );
    void viewportDropEvent( QDropEvent *e );
    bool acceptDrop( const QPoint &pnt, QWidget *source );
    void setCurrentDropItem( const QPoint &pnt );
    void keyPressEvent( QKeyEvent *e );

private slots:
    void rename();
    void cancelRename();

private slots:
    void doubleClickTimeout();
    void changeDirDuringDrag();
    void doDragScroll();
    void dragObjDestroyed();

private:
    QRenameEdit *lined;
    QFileDialog *filedialog;
    bool renaming;
    QTimer* renameTimer;
    QListBoxItem *renameItem;
    QPoint pressPos, oldDragPos;
    bool mousePressed, eraseDragShape;
    int urls;
    QString startDragDir;
    QListBoxItem *currDropItem;
    QTimer *changeDirTimer;
    QTimer *dragScrollTimer;
    bool firstMousePressEvent;
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
    void viewportMouseReleaseEvent( QMouseEvent *e );
    void viewportMouseMoveEvent( QMouseEvent *e );
    void viewportDragEnterEvent( QDragEnterEvent *e );
    void viewportDragMoveEvent( QDragMoveEvent *e );
    void viewportDragLeaveEvent( QDragLeaveEvent *e );
    void viewportDropEvent( QDropEvent *e );
    bool acceptDrop( const QPoint &pnt, QWidget *source );
    void setCurrentDropItem( const QPoint &pnt );

private slots:
    void rename();
    void cancelRename();

private slots:
    void doubleClickTimeout();
    void changeDirDuringDrag();
    void doDragScroll();
    void dragObjDestroyed();

private:
    QRenameEdit *lined;
    QFileDialog *filedialog;
    bool renaming;
    QTimer* renameTimer;
    QListViewItem *renameItem;
    QPoint pressPos, oldDragPos;
    bool mousePressed, eraseDragShape;
    int urls;
    QString startDragDir;
    QListViewItem *currDropItem;
    QTimer *changeDirTimer;
    QTimer *dragScrollTimer;
    bool firstMousePressEvent;
};

class Q_EXPORT QFileDialog : public QDialog
{
    friend class QFileListBox;
    friend class QFileListView;

    Q_OBJECT
public:
    QFileDialog( const QString& dirName, const QString& filter = QString::null,
                 QWidget *parent=0, const char *name = 0, bool modal = FALSE );
    QFileDialog( QWidget *parent=0, const char *name = 0, bool modal = FALSE );
    ~QFileDialog();

    // recommended static functions

    static QString getOpenFileName( const QString &initially = QString::null,
                                    const QString &filter = QString::null,
                                    QWidget *parent = 0, const char* name = 0);
    static QString getSaveFileName( const QString &initially = QString::null,
                                    const QString &filter = QString::null,
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

    void setShowHiddenFiles( bool s );
    bool showHiddenFiles() const;

    void rereadDir();

    enum Mode { AnyFile, ExistingFile, Directory, ExistingFiles };
    void setMode( Mode );
    Mode mode() const;

    bool eventFilter( QObject *, QEvent * );

    void setPreviewMode( bool info, bool contents );
    void setInfoPreviewWidget( QWidget *w );
    void setContentsPreviewWidget( QWidget *w );

    QUrl url() const;
    void setUrl( const QUrl &url );

public slots:
    void setDir( const QString& );
    void setFilter( const QString& );
    void setFilters( const char ** );
    void setFilters( const QStringList& );

signals:
    void fileHighlighted( const QString& );
    void fileSelected( const QString& );
    void dirEntered( const QString& );
    void showPreview( const QUrl & );

protected slots:
    void detailViewSelectionChanged();
    void listBoxSelectionChanged();
    void changeMode( int );
    void slotIsDir();
    void slotIsFile();
    
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

    void drawDragShapes( const QPoint &pnt, bool multRow, int num );

private slots:
    void updateGeometries();
    void modeButtonsDestroyed();
    void clearView();
    void polishDirectory();
    void insertEntry( const QUrlInfo &fi );
    void removeEntry( const QString &fi );
    void createdDirectory( const QUrlInfo &info );
    void error( int ecode, const QString &msg );
    void itemChanged( const QString &oldname, const QString &newname );

private:
    enum PopupAction {
        PA_Open = 0,
        PA_Delete,
        PA_Rename,
        PA_SortName,
        PA_SortSize,
        PA_SortType,
        PA_SortDate,
        PA_SortUnsorted,
        PA_Cancel,
        PA_Reload,
        PA_Hidden
    };

    void init();
    bool trySetSelection( bool isDir, const QUrl &, bool );
    void deleteFile( const QString &filename );
    void popupContextMenu( const QString &filename, bool withSort,
                           PopupAction &action, const QPoint &p );

    QDir reserved; // was cwd
    QString fileName;

    QFileDialogPrivate *d;
    QFileListView  *files;

    QLineEdit  *nameEdit; // also filter
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
