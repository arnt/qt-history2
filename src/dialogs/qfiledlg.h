/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qfiledlg.h#16 $
**
** Definition of QFileDialog class
**
** Created : 950428
**
** Copyright (C) 1995-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QFILEDLG_H
#define QFILEDLG_H

struct QFileDialogPrivate;
class QLineEdit;
class QPushButton;
class QListView;
class QListViewItem;

#include "qdir.h"
#include "qdialog.h"


class QFileDialog : public QDialog
{
    Q_OBJECT
public:
    QFileDialog( const char *dirName, const char *filter = 0,
		 QWidget *parent=0, const char *name=0, bool modal=FALSE );
    QFileDialog( QWidget *parent=0, const char *name=0, bool modal=FALSE );
   ~QFileDialog();

    // recommended static functions

    static QString getOpenFileName( const char *dir = 0, const char *filter= 0,
				    QWidget *parent = 0, const char *name = 0);
    static QString getSaveFileName( const char *dir = 0, const char *filter= 0,
				    QWidget *parent = 0, const char *name = 0);

    // non-static function for special needs

    QString selectedFile() const;

    const char *dirPath() const;

    void setDir( const char * );
    void setDir( const QDir & );
    const QDir *dir() const;

    void rereadDir();

signals:
    void fileHighlighted( const char * );
    void fileSelected( const char * );
    void dirEntered( const char * );

private slots:
    void fileSelected( int );
    void fileHighlighted( int );
    void dirSelected( int );
    void pathSelected( int );

    void updateFileNameEdit( QListViewItem *);
    void selectDirectoryOrFile( QListViewItem * );
    void popupContextMenu( QListViewItem *, const QPoint &, int );
    void fileNameEditDone();

    void okClicked();
    void filterClicked(); // not used
    void cancelClicked();

    void cdUpClicked();
    void detailViewClicked();
    void mcViewClicked();
    
protected:
    void resizeEvent( QResizeEvent * );

private slots:
    void updateGeometry();

private:
    void init();
    void updatePathBox( const char * );

    QDir cwd;
    QString fileName;

    QFileDialogPrivate *d;
    QListView  *files;

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

private:	// Disabled copy constructor and operator=
    QFileDialog( const QFileDialog & );
    QFileDialog &operator=( const QFileDialog & );
};


#endif // QFILEDLG_H
