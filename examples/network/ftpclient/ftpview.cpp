/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "ftpview.h"

#include <qpixmap.h>
#include <qvaluelist.h>

/* XPM */
static const char* closed_xpm[]={
    "15 15 6 1",
    ". c None",
    "b c #ffff00",
    "d c #000000",
    "* c #999999",
    "a c #cccccc",
    "c c #ffffff",
    "...............",
    "..*****........",
    ".*ababa*.......",
    "*abababa******.",
    "*cccccccccccc*d",
    "*cbababababab*d",
    "*cabababababa*d",
    "*cbababababab*d",
    "*cabababababa*d",
    "*cbababababab*d",
    "*cabababababa*d",
    "*cbababababab*d",
    "**************d",
    ".dddddddddddddd",
    "..............."};

/* XPM */
static const char* file_xpm[]={
    "13 15 5 1",
    ". c #7f7f7f",
    "# c None",
    "c c #000000",
    "b c #bfbfbf",
    "a c #ffffff",
    "..........###",
    ".aaaaaaaab.##",
    ".aaaaaaaaba.#",
    ".aaaaaaaacccc",
    ".aaaaaaaaaabc",
    ".aaaaaaaaaabc",
    ".aaaaaaaaaabc",
    ".aaaaaaaaaabc",
    ".aaaaaaaaaabc",
    ".aaaaaaaaaabc",
    ".aaaaaaaaaabc",
    ".aaaaaaaaaabc",
    ".aaaaaaaaaabc",
    ".bbbbbbbbbbbc",
    "ccccccccccccc"};

QPixmap *folderIcon = 0;
QPixmap *fileIcon = 0;

FtpViewItem::FtpViewItem( QListView *parent, const QUrlInfo &i )
    : QListViewItem( parent, i.name() ), info( i )
{
}

int FtpViewItem::compare( QListViewItem * i, int col, bool ascending ) const
{
    FtpViewItem *other = (FtpViewItem*)i;
    switch ( col ) {
    case 1:
	if ( info.size() == other->info.size() )
	    return 0;
	else
	    return info.size() < other->info.size() ? -1 : 1;
    case 2:
	if ( info.lastModified() == other->info.lastModified() )
	    return 0;
	else
	    return info.lastModified() < other->info.lastModified() ? -1 : 1;
	break;
    default:
	// use default method for colum 0 and others added in the future
	return QListViewItem::compare( i, col, ascending );
    };
}

QString FtpViewItem::text( int c ) const
{
    switch ( c ) {
    case 0:
	return info.name();
    case 1:
	return QString::number( info.size() );
    case 2:
	return info.lastModified().toString();
    }

    return "????";
}

const QPixmap *FtpViewItem::pixmap( int c ) const
{
    if ( !folderIcon )
	folderIcon = new QPixmap( closed_xpm );
    if ( !fileIcon )
	fileIcon = new QPixmap( file_xpm );
    if ( info.isDir() && c == 0 )
	return folderIcon;
    else if ( info.isFile() && c == 0 )
	return fileIcon;
    return 0;
}


FtpView::FtpView( QWidget *parent )
    : QListView( parent )
{
    addColumn( tr( "Name" ) );
    addColumn( tr( "Size" ) );
    addColumn( tr( "Last Modified" ) );
    setColumnAlignment( 1, Qt::AlignRight );
    setShowSortIndicator( TRUE );
    setAllColumnsShowFocus( TRUE );
    setSelectionMode( Extended );

    connect( this, SIGNAL( doubleClicked( QListViewItem * ) ),
	     this, SLOT( slotSelected( QListViewItem * ) ) );
    connect( this, SIGNAL( returnPressed( QListViewItem * ) ),
	     this, SLOT( slotSelected( QListViewItem * ) ) );
}

void FtpView::slotInsertEntries( const QValueList<QUrlInfo> &info )
{
    QValueList<QUrlInfo>::ConstIterator it;
    for( it = info.begin(); it != info.end(); ++it ) {
	if ( (*it).name() != ".." && (*it).name() != "." && (*it).name()[ 0 ] == '.' )
	    continue;
	FtpViewItem *item = new FtpViewItem( this, (*it) );
	if ( (*it).isDir() )
	    item->setSelectable( FALSE );
    }
}

void FtpView::slotSelected( QListViewItem *item )
{
    if ( !item )
	return;

    FtpViewItem *i = (FtpViewItem*)item;
    if ( i->entryInfo().isDir() )
	emit itemSelected( i->entryInfo() );
}

QValueList<QUrlInfo> FtpView::selectedItems() const
{
    QValueList<QUrlInfo> lst;
    QListViewItemIterator it( (QListView*)this );
    for ( ; it.current(); ++it ) {
	if ( it.current()->isSelected() ) {
	    lst << ( (FtpViewItem*)it.current() )->entryInfo();
	}
    }

    return lst;
}
