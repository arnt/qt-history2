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

#ifndef DIRVIEW_H
#define DIRVIEW_H

#include <q3listview.h>
#include <qstring.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtimer.h>

class QWidget;
class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;

class FileItem : public Q3ListViewItem
{
public:
    FileItem( Q3ListViewItem *parent, const QString &s1, const QString &s2 )
	: Q3ListViewItem( parent, s1, s2 ), pix( 0 ) {}

    const QPixmap *pixmap( int i ) const;
#if !defined(Q_NO_USING_KEYWORD)
    using Q3ListViewItem::setPixmap;
#endif
    void setPixmap( QPixmap *p );

private:
    QPixmap *pix;

};

class Directory : public Q3ListViewItem
{
public:
    Directory( Q3ListView * parent, const QString& filename );
    Directory( Directory * parent, const QString& filename, const QString &col2 )
	: Q3ListViewItem( parent, filename, col2 ), pix( 0 ) {}
    Directory( Directory * parent, const QString& filename );

    QString text( int column ) const;

    QString fullName();

    void setOpen( bool );
    void setup();

    const QPixmap *pixmap( int i ) const;
#if !defined(Q_NO_USING_KEYWORD)
    using Q3ListViewItem::setPixmap;
#endif
    void setPixmap( QPixmap *p );

private:
    QFile f;
    Directory * p;
    bool readable;
    bool showDirsOnly;
    QPixmap *pix;

};

class DirectoryView : public Q3ListView
{
    Q_OBJECT

public:
    DirectoryView( QWidget *parent = 0, const char *name = 0, bool sdo = FALSE );
    bool showDirsOnly() { return dirsOnly; }

public slots:
    void setDir( const QString & );

signals:
    void folderSelected( const QString & );

protected slots:
    void slotFolderSelected( Q3ListViewItem * );
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
    QString fullPath(Q3ListViewItem* item);
    bool dirsOnly;
    Q3ListViewItem *oldCurrent;
    Q3ListViewItem *dropItem;
    QTimer* autoopen_timer;
    QPoint presspos;
    bool mousePressed;

};

#endif
