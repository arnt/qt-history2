/****************************************************************************
** $Id: //depot/qt/main/examples/dirview/dirview.cpp#6 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "dirview.h"

#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qpixmap.h>
#include <qevent.h>
#include <qpoint.h>
#include <qmessagebox.h>
#include <qdragobject.h>
#include <qmime.h>
#include <qstrlist.h>
#include <qstringlist.h>

static const char* folder_closed_xpm[]={
    "16 16 9 1",
    "g c #808080",
    "b c #c0c000",
    "e c #c0c0c0",
    "# c #000000",
    "c c #ffff00",
    ". c None",
    "a c #585858",
    "f c #a0a0a4",
    "d c #ffffff",
    "..###...........",
    ".#abc##.........",
    ".#daabc#####....",
    ".#ddeaabbccc#...",
    ".#dedeeabbbba...",
    ".#edeeeeaaaab#..",
    ".#deeeeeeefe#ba.",
    ".#eeeeeeefef#ba.",
    ".#eeeeeefeff#ba.",
    ".#eeeeefefff#ba.",
    ".##geefeffff#ba.",
    "...##gefffff#ba.",
    ".....##fffff#ba.",
    ".......##fff#b##",
    ".........##f#b##",
    "...........####."};

static const char* folder_open_xpm[]={
    "16 16 11 1",
    "# c #000000",
    "g c #c0c0c0",
    "e c #303030",
    "a c #ffa858",
    "b c #808080",
    "d c #a0a0a4",
    "f c #585858",
    "c c #ffdca8",
    "h c #dcdcdc",
    "i c #ffffff",
    ". c None",
    "....###.........",
    "....#ab##.......",
    "....#acab####...",
    "###.#acccccca#..",
    "#ddefaaaccccca#.",
    "#bdddbaaaacccab#",
    ".eddddbbaaaacab#",
    ".#bddggdbbaaaab#",
    "..edgdggggbbaab#",
    "..#bgggghghdaab#",
    "...ebhggghicfab#",
    "....#edhhiiidab#",
    "......#egiiicfb#",
    "........#egiibb#",
    "..........#egib#",
    "............#ee#"};

static const char * folder_locked[]={
    "16 16 10 1",
    "h c #808080",
    "b c #ffa858",
    "f c #c0c0c0",
    "e c #c05800",
    "# c #000000",
    "c c #ffdca8",
    ". c None",
    "a c #585858",
    "g c #a0a0a4",
    "d c #ffffff",
    "..#a#...........",
    ".#abc####.......",
    ".#daa#eee#......",
    ".#ddf#e##b#.....",
    ".#dfd#e#bcb##...",
    ".#fdccc#daaab#..",
    ".#dfbbbccgfg#ba.",
    ".#ffb#ebbfgg#ba.",
    ".#ffbbe#bggg#ba.",
    ".#fffbbebggg#ba.",
    ".##hf#ebbggg#ba.",
    "...###e#gggg#ba.",
    ".....#e#gggg#ba.",
    "......###ggg#b##",
    ".........##g#b##",
    "...........####."};

static const char * pix_file []={
    "16 16 7 1",
    "# c #000000",
    "b c #ffffff",
    "e c #000000",
    "d c #404000",
    "c c #c0c000",
    "a c #ffffc0",
    ". c None",
    "................",
    ".........#......",
    "......#.#a##....",
    ".....#b#bbba##..",
    "....#b#bbbabbb#.",
    "...#b#bba##bb#..",
    "..#b#abb#bb##...",
    ".#a#aab#bbbab##.",
    "#a#aaa#bcbbbbbb#",
    "#ccdc#bcbbcbbb#.",
    ".##c#bcbbcabb#..",
    "...#acbacbbbe...",
    "..#aaaacaba#....",
    "...##aaaaa#.....",
    ".....##aa#......",
    ".......##......."};

/*****************************************************************************
 *
 * Class Directory
 *
 *****************************************************************************/

Directory::Directory( Directory * parent, const QString& filename )
    : QListViewItem( parent ), f(filename),
      showDirsOnly( parent->showDirsOnly )
{
    p = parent;
    readable = QDir( fullName() ).isReadable();

    if ( !readable )
        setPixmap( 0, QPixmap( folder_locked ) );
    else
        setPixmap( 0, QPixmap( folder_closed_xpm ) );
}


Directory::Directory( QListView * parent, const QString& filename )
    : QListViewItem( parent ), f(filename),
      showDirsOnly( ( (DirectoryView*)parent )->showDirsOnly() )
{
    p = 0;
    readable = QDir( fullName() ).isReadable();
}


void Directory::setOpen( bool o )
{
    if ( o )
        setPixmap( 0, QPixmap( folder_open_xpm ) );
    else
        setPixmap( 0, QPixmap( folder_closed_xpm ) );

    if ( o && !childCount() ) {
        QString s( fullName() );
        QDir thisDir( s );
        if ( !thisDir.isReadable() ) {
            readable = FALSE;
            setExpandable( FALSE );
            return;
        }

	listView()->setUpdatesEnabled( FALSE );
        const QFileInfoList * files = thisDir.entryInfoList();
        if ( files ) {
            QFileInfoListIterator it( *files );
            QFileInfo * f;
            while( (f=it.current()) != 0 ) {
                ++it;
                if ( f->fileName() == "." || f->fileName() == ".." )
                    ; // nothing
                else if ( f->isSymLink() && !showDirsOnly ) {
                    QListViewItem *item = new QListViewItem( this, f->fileName(),
                                                             "Symbolic Link" );
                    item->setPixmap( 0, QPixmap( pix_file ) );
                }
                else if ( f->isDir() )
                    (void)new Directory( this, f->fileName() );
                else if ( !showDirsOnly ) {
                    QListViewItem *item
			= new QListViewItem( this, f->fileName(),
					     f->isFile()?"File":"Special" );
                    item->setPixmap( 0, QPixmap( pix_file ) );
                }
            }
        }
	listView()->setUpdatesEnabled( TRUE );
    }
    QListViewItem::setOpen( o );
}


void Directory::setup()
{
    setExpandable( TRUE );
    QListViewItem::setup();
}


QString Directory::fullName()
{
    QString s;
    if ( p ) {
        s = p->fullName();
        s.append( f.name() );
        s.append( "/" );
    } else {
        s = f.name();
    }
    return s;
}


QString Directory::text( int column ) const
{
    if ( column == 0 )
        return f.name();
    else if ( readable )
        return "Directory";
    else
        return "Unreadable Directory";
}

/*****************************************************************************
 *
 * Class DirectoryView
 *
 *****************************************************************************/

DirectoryView::DirectoryView( QWidget *parent, const char *name, bool sdo )
    : QListView( parent, name ), dirsOnly( sdo ), oldCurrent( 0 ),
      dropItem( 0 ), autoopen_timer( this ), mousePressed( FALSE ), autoscroll_timer( this )
{
    connect( this, SIGNAL( doubleClicked( QListViewItem * ) ),
             this, SLOT( slotFolderSelected( QListViewItem * ) ) );
    connect( this, SIGNAL( returnPressed( QListViewItem * ) ),
             this, SLOT( slotFolderSelected( QListViewItem * ) ) );

    setAcceptDrops( TRUE );
    viewport()->setAcceptDrops( TRUE );

    connect( &autoopen_timer, SIGNAL( timeout() ),
             this, SLOT( openFolder() ) );

    connect( &autoscroll_timer, SIGNAL( timeout() ),
             this, SLOT( autoScroll() ) );
}

void DirectoryView::slotFolderSelected( QListViewItem *i )
{
    if ( !i || !showDirsOnly() )
        return;

    Directory *dir = (Directory*)i;
    emit folderSelected( dir->fullName() );
}

void DirectoryView::openFolder()
{
    autoopen_timer.stop();
    if ( dropItem && !dropItem->isOpen() ) {
        dropItem->setOpen( TRUE );
        dropItem->repaint();
    }
}

static const int autoopenTime = 750;

void DirectoryView::contentsDragEnterEvent( QDragEnterEvent *e )
{
    if ( !QUriDrag::canDecode(e) ) {
        e->ignore();
        return;
    }

    oldCurrent = currentItem();

    QListViewItem *i = itemAt( contentsToViewport(e->pos()) );
    if ( i ) {
        dropItem = i;
        autoopen_timer.start( autoopenTime );
    }
}

static const int autoscroll_margin = 16;
static const int initialScrollTime = 30;
static const int initialScrollAccel = 5;

void DirectoryView::startAutoScroll()
{
    if ( !autoscroll_timer.isActive() ) {
        autoscroll_time = initialScrollTime;
        autoscroll_accel = initialScrollAccel;
        autoscroll_timer.start( autoscroll_time );
    }
}

void DirectoryView::stopAutoScroll()
{
    autoscroll_timer.stop();
}

void DirectoryView::autoScroll()
{
    QPoint p = viewport()->mapFromGlobal( QCursor::pos() );

    if ( autoscroll_accel-- <= 0 && autoscroll_time ) {
        autoscroll_accel = initialScrollAccel;
        autoscroll_time--;
        autoscroll_timer.start( autoscroll_time );
    }
    int l = QMAX(1,(initialScrollTime-autoscroll_time));

    int dx=0,dy=0;
    if ( p.y() < autoscroll_margin ) {
        dy = -l;
    } else if ( p.y() > visibleHeight()-autoscroll_margin ) {
        dy = +l;
    }
    if ( p.x() < autoscroll_margin ) {
        dx = -l;
    } else if ( p.x() > visibleWidth()-autoscroll_margin ) {
        dx = +l;
    }
    if ( dx || dy ) {
        scrollBy(dx,dy);
    } else {
        stopAutoScroll();
    }
}

void DirectoryView::contentsDragMoveEvent( QDragMoveEvent *e )
{
    if ( !QUriDrag::canDecode(e) ) {
        e->ignore();
        return;
    }

    QPoint vp = contentsToViewport(e->pos());
    QRect inside_margin(autoscroll_margin, autoscroll_margin,
                        visibleWidth()-autoscroll_margin*2,
                        visibleHeight()-autoscroll_margin*2);
    QListViewItem *i = itemAt( vp );
    if ( i ) {
        setSelected( i, TRUE );
        if ( !inside_margin.contains(vp) ) {
            startAutoScroll();
            e->accept(QRect(0,0,0,0)); // Keep sending move events
            autoopen_timer.stop();
        } else {
            e->accept();
            if ( i != dropItem ) {
                autoopen_timer.stop();
                dropItem = i;
                autoopen_timer.start( autoopenTime );
            }
        }
        switch ( e->action() ) {
	    case QDropEvent::Copy:
            break;
	    case QDropEvent::Move:
            e->acceptAction();
            break;
	    case QDropEvent::Link:
            e->acceptAction();
            break;
	    default:
            ;
        }
    } else {
        e->ignore();
        autoopen_timer.stop();
        dropItem = 0;
    }
}

void DirectoryView::contentsDragLeaveEvent( QDragLeaveEvent * )
{
    autoopen_timer.stop();
    stopAutoScroll();
    dropItem = 0;

    setCurrentItem( oldCurrent );
    setSelected( oldCurrent, TRUE );
}

void DirectoryView::contentsDropEvent( QDropEvent *e )
{
    autoopen_timer.stop();
    stopAutoScroll();

    if ( !QUriDrag::canDecode(e) ) {
        e->ignore();
        return;
    }

    QListViewItem *item = itemAt( contentsToViewport(e->pos()) );
    if ( item ) {

        QStrList lst;
        lst.setAutoDelete( FALSE );

        QUriDrag::decode( e, lst );

        QString str;

        switch ( e->action() ) {
	    case QDropEvent::Copy:
            str = "Copy";
            break;
	    case QDropEvent::Move:
            str = "Move";
            e->acceptAction();
            break;
	    case QDropEvent::Link:
            str = "Link";
            e->acceptAction();
            break;
	    default:
            str = "Unknown";
        }

        str += "\n\n";

        e->accept();

        for ( unsigned int i = 0; i < lst.count(); i++ ) {
            QString filename = QUriDrag::uriToLocalFile( lst.at( i ) );
            if ( filename.isNull() )
                str += QString( "   %1\n" ) // Full URI
                       .arg( QUriDrag::uriToUnicodeUri(lst.at( i )) );
            else
                str += QString( "   %1\n" ) // Local file
                       .arg( filename );
        }
        str += QString( "\nTo\n\n   %1" )
               .arg( fullPath(item) );

        QMessageBox::information( this, "Drop target", str, "Not implemented" );
    } else
        e->ignore();

}

QString DirectoryView::fullPath(QListViewItem* item)
{
    QString fullpath = item->text(0);
    while ( (item=item->parent()) ) {
        if ( item->parent() )
            fullpath = item->text(0) + "/" + fullpath;
        else
            fullpath = item->text(0) + fullpath;
    }
    return fullpath;
}

void DirectoryView::contentsMousePressEvent( QMouseEvent* e )
{
    QListView::contentsMousePressEvent(e);
    presspos = e->pos();
    mousePressed = TRUE;
}

void DirectoryView::contentsMouseMoveEvent( QMouseEvent* e )
{
    if ( mousePressed && (e->pos() - presspos).manhattanLength() > 4 ) {
	mousePressed = FALSE;
        QListViewItem *item = itemAt( contentsToViewport(presspos) );
        if ( item ) {
            QString source = fullPath(item);
            if ( QFile::exists(source) ) {
                QUriDrag* d = new QUriDrag(viewport());
                d->setFilenames(source);
                if ( d->drag() )
                    QMessageBox::information( this, "Drag source",
                                              QString("Delete ")+source, "Not implemented" );
            }
        }
    }
}

void DirectoryView::contentsMouseReleaseEvent( QMouseEvent * )
{
    mousePressed = FALSE;
}

void DirectoryView::setDir( const QString &s )
{
    QListViewItemIterator it( this );
    ++it;
    for ( ; it.current(); ++it ) {
	it.current()->setOpen( FALSE );
    }
    
    QStringList lst( QStringList::split( "/", s ) );
    QListViewItem *item = firstChild();
    QStringList::Iterator it2 = lst.begin();
    for ( ; it2 != lst.end(); ++it2 ) {
	while ( item ) {
	    if ( item->text( 0 ) == *it2 ) {
		item->setOpen( TRUE );
		break;
	    }
	    item = item->itemBelow();
	}
    }
    
    if ( item ) {
	setCurrentItem( item );
	ensureItemVisible( item );
    }
}
