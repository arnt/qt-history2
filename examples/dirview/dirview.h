/****************************************************************************
** $Id: //depot/qt/main/examples/dirview/dirview.h#4 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef DIRVIEW_H
#define DIRVIEW_H

#include <qlistview.h>
#include <qstring.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtimer.h>

class QWidget;
class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;

class Directory : public QListViewItem
{
public:
    Directory( QListView * parent, const QString& filename );
    Directory( Directory * parent, const QString& filename );

    QString text( int column ) const;

    QString fullName();

    void setOpen( bool );
    void setup();

private:
    QFile f;
    Directory * p;
    bool readable;
    bool showDirsOnly;

};

class DirectoryView : public QListView
{
    Q_OBJECT

public:
    DirectoryView( QWidget *parent = 0, const char *name = 0, bool sdo = FALSE );
    bool showDirsOnly() { return dirsOnly; }

signals:
    void folderSelected( const QString & );

protected slots:
    void slotFolderSelected( QListViewItem * );
    void openFolder();

protected:
    void contentsDragEnterEvent( QDragEnterEvent *e );
    void contentsDragMoveEvent( QDragMoveEvent *e );
    void contentsDragLeaveEvent( QDragLeaveEvent *e );
    void contentsDropEvent( QDropEvent *e );
    void contentsMouseMoveEvent( QMouseEvent *e );
    void contentsMousePressEvent( QMouseEvent *e );
    void contentsMouseReleaseEvent( QMouseEvent *e );

private:
    QString fullPath(QListViewItem* item);
    bool dirsOnly;
    QListViewItem *oldCurrent;
    QListViewItem *dropItem;
    QTimer autoopen_timer;
    QPoint presspos;
    bool mousePressed;
    
    // ############# Move this to QScrollView
private:
    QTimer autoscroll_timer;
    int autoscroll_time;
    int autoscroll_accel;
public slots:
    void startAutoScroll();
    void stopAutoScroll();
protected slots:
    void autoScroll();
};

#endif
