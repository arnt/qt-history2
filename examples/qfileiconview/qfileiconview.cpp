/****************************************************************************
** $Id: //depot/qt/main/examples/qfileiconview/qfileiconview.cpp#17 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "qfileiconview.h"

#include <qstringlist.h>
#include <qpixmap.h>
#include <qmime.h>
#include <qstrlist.h>
#include <qdragobject.h>
#include <qmessagebox.h>
#include <qevent.h>
#include <qpopupmenu.h>
#include <qcursor.h>
#include <qapplication.h>

static const char * file_icon[]={
    "32 32 17 1",
    "# c #000000",
    "a c #ffffff",
    "j c #808080",
    "n c #a0a0a4",
    "g c #c0c0c0",
    "m c #004000",
    "o c #000000",
    "l c #004040",
    "k c #404000",
    "i c #c0c000",
    "h c #ffff00",
    "b c #ffffc0",
    "e c #ff8000",
    "f c #c05800",
    "c c #ffa858",
    "d c #ffdca8",
    ". c None",
    "................................",
    "................................",
    "................................",
    "................................",
    ".............#....###...........",
    "...###......#a##.#aba##.........",
    "..#cdb#....#aaaa#aaaaaa##.......",
    "..#ecdb#..#aaaa#aaaaaaaba##.....",
    "..#fecdb##aaaa#aaaaaaaaaaab##...",
    "...#fecdb#aaa#aaaaaaabaabaaaa##.",
    "....#fecdb#a#baaaaa#baaaaaabaaa#",
    ".....#fecdb#aaaaab#a##baaaaaaa#.",
    ".....##fecdb#bbba#aaaa##baaab#..",
    "....#bb#fecdb#ba#aaaaaaa##aa#...",
    "...#bbbb#fecdb##aaabaaaaaa##....",
    "..#bbbb#b#fecdb#aaaaaaabaaaa##..",
    ".#bbbb#bbb#fecdg#aaaaaaaaaaaba#.",
    "#hhbb#bbbbb#fegg#iiaaaaaaaaaaaa#",
    "#jhhhklibbbk#ggj#aaiiaaaaaaaaa#j",
    ".#mjhhhkmikab####aaabiiaaaaaa#j.",
    "...##jhhhmaaibbaaiibaaaiiaab#n..",
    ".....##j#baaaiiabaaiibaabaa#n...",
    "......##baibaabiibaaaiiabb#j....",
    "......#bbbbiiaabbiiaaaaabon.....",
    ".....#bbbbbbbiiabbaiiaab#n......",
    ".....#jbbbbbbbbiibaabba#n.......",
    "......##jbbbbbbbbiiaabmj........",
    "........##jbbbbbbbbbb#j.........",
    "..........##nbbbbbbbmj..........",
    "............##jbbbb#j...........",
    "..............#mjj#n............",
    "................##n............."};

static const char * folder_icon[]={
    "32 32 11 1",
    "# c #000000",
    "b c #c0c000",
    "d c #585858",
    "a c #ffff00",
    "i c #400000",
    "h c #a0a0a4",
    "e c #000000",
    "c c #ffffff",
    "f c #303030",
    "g c #c0c0c0",
    ". c None",
    "...###..........................",
    "...#aa##........................",
    ".###baaa##......................",
    ".#cde#baaa##....................",
    ".#cccdeebaaa##..##f.............",
    ".#cccccdeebaaa##aaa##...........",
    ".#cccccccdeebaaaaaaaa##.........",
    ".#cccccccccdeebababaaa#.........",
    ".#cccccgcgghhebbbbbbbaa#........",
    ".#ccccccgcgggdebbbbbbba#........",
    ".#cccgcgcgcgghdeebiebbba#.......",
    ".#ccccgcggggggghdeddeeba#.......",
    ".#cgcgcgcggggggggghghdebb#......",
    ".#ccgcggggggggghghghghd#b#......",
    ".#cgcgcggggggggghghghhd#b#......",
    ".#gcggggggggghghghhhhhd#b#......",
    ".#cgcggggggggghghghhhhd#b#......",
    ".#ggggggggghghghhhhhhhdib#......",
    ".#gggggggggghghghhhhhhd#b#......",
    ".#hhggggghghghhhhhhhhhd#b#......",
    ".#ddhhgggghghghhhhhhhhd#b#......",
    "..##ddhhghghhhhhhhhhhhdeb#......",
    "....##ddhhhghhhhhhhhhhd#b#......",
    "......##ddhhhhhhhhhhhhd#b#......",
    "........##ddhhhhhhhhhhd#b#......",
    "..........##ddhhhhhhhhd#b#......",
    "............##ddhhhhhhd#b###....",
    "..............##ddhhhhd#b#####..",
    "................##ddhhd#b######.",
    "..................##dddeb#####..",
    "....................##d#b###....",
    "......................####......"};


static const char * link_icon[]={
    "32 32 12 1",
    "# c #000000",
    "h c #a0a0a4",
    "b c #c00000",
    "d c #585858",
    "i c #400000",
    "c c #ffffff",
    "e c #000000",
    "g c #c0c0c0",
    "a c #ff0000",
    "f c #303030",
    "n c white",
    ". c None",
    "...###..........................",
    "...#aa##........................",
    ".###baaa##......................",
    ".#cde#baaa##....................",
    ".#cccdeebaaa##..##f.............",
    ".#cccccdeebaaa##aaa##...........",
    ".#cccccccdeebaaaaaaaa##.........",
    ".#cccccccccdeebababaaa#.........",
    ".#cccccgcgghhebbbbbbbaa#........",
    ".#ccccccgcgggdebbbbbbba#........",
    ".#cccgcgcgcgghdeebiebbba#.......",
    ".#ccccgcggggggghdeddeeba#.......",
    ".#cgcgcgcggggggggghghdebb#......",
    ".#ccgcggggggggghghghghd#b#......",
    ".#cgcgcggggggggghghghhd#b#......",
    ".#gcggggggggghghghhhhhd#b#......",
    ".#cgcggggggggghghghhhhd#b#......",
    ".#ggggggggghghghhhhhhhdib#......",
    ".#gggggggggghghghhhhhhd#b#......",
    ".#hhggggghghghhhhhhhhhd#b#......",
    ".#ddhhgggghghghhhhhhhhd#b#......",
    "..##ddhhghghhhhhhhhhhhdeb#......",
    "############hhhhhhhhhhd#b#......",
    "#nnnnnnnnnn#hhhhhhhhhhd#b#......",
    "#nnnnnnnnnn#hhhhhhhhhhd#b#......",
    "#nn#nn#nnnn#ddhhhhhhhhd#b#......",
    "#nn##n##nnn###ddhhhhhhd#b###....",
    "#nnn#####nn#..##ddhhhhd#b#####..",
    "#nnnnn##nnn#....##ddhhd#b######.",
    "#nnnnn#nnnn#......##dddeb#####..",
    "#nnnnnnnnnn#........##d#b###....",
    "############..........####......"};

static const char * empty_icon[]={
    "1 1 1 1",
    ". c None",
    "."};

static const char * folder_locked_icon[]={
    "32 32 12 1",
    "# c #000000",
    "g c #808080",
    "h c #c0c0c0",
    "f c #c05800",
    "c c #ffffff",
    "d c #585858",
    "b c #ffa858",
    "a c #ffdca8",
    "e c #000000",
    "i c #a0a0a4",
    "j c #c0c0c0",
    ". c None",
    "...###..........................",
    "...#aa##........................",
    ".###baaa##......................",
    ".#cde#baaa##....................",
    ".#cccdeeba#######...............",
    ".#cccccde##fffff##..............",
    ".#cccccc##fffgggg#..............",
    ".#ccccccc#ffg####a##............",
    ".#ccccchc#ffg#eebbaa##..........",
    ".#ccccccc#ffg#ddeebbba##........",
    ".#ccchccc#ffg#ihddeebbba##......",
    ".#cccccaa#ffg#ihhhddeeba##......",
    ".#chchhbbaafg#ihhhihidebb#......",
    ".#cchccbbbbaa#ihhihihid#b#......",
    ".#chchhbb#bbbaaiihihiid#b#......",
    ".#hchhcbb#fbbbafhiiiiid#b#......",
    ".#chchhbb#ffgbbfihiiiid#b#......",
    ".#hhhhhbb#ffg#bfiiiiiid#b#......",
    ".#hhhhhbbaffg#bfiiiiiid#b#......",
    ".#iihhhjbbaab#bfiiiiiid#b#......",
    ".#ddiihhh#bbbabfiiiiiid#b#......",
    "..##ddiih#ffbbbfiiiiiid#b#......",
    "....##ddi#ffg#biiiiiiid#b#......",
    "......##d#ffg#iiiiiiiid#b#......",
    "........##ffg#iiiiiiiid#b#......",
    ".........#ffg#iiiiiiiid#b#......",
    ".........#ffg#ddiiiiiid#b###....",
    ".........##fg###ddiiiid#b#####..",
    "...........####.##ddiid#b######.",
    "..................##dddeb#####..",
    "....................##d#b###....",
    "......................####......"};


/*****************************************************************************
 *
 * Class QtFileIconViewItem
 *
 *****************************************************************************/

QtFileIconViewItem::QtFileIconViewItem( QtFileIconView *parent, QFileInfo *fi )
// set parent 0 => don't align in grid yet, as aour metrics is not correct yet
    : QIconViewItem( 0, fi->fileName(), QIconSet( QPixmap( empty_icon ) ) ), itemFileName( fi->filePath() ),
      itemFileInfo( *fi ), checkSetText( FALSE ), timer( this )
{
    view = parent;

    if ( itemFileInfo.isDir() )
	itemType = Dir;
    else if ( itemFileInfo.isFile() )
	itemType = File;
    else if ( itemFileInfo.isSymLink() )
	itemType = Link;

    setAllowDrop( FALSE );

    switch ( itemType )
    {
    case Dir:
	if ( !QDir( itemFileName ).isReadable() )
	    setIcon( QPixmap( folder_locked_icon ) );
	else
	    setIcon( QPixmap( folder_icon ) );
	setAllowDrop( QDir( itemFileName ).isReadable()	 );
	break;
    case File:
	setIcon( QPixmap( file_icon ) );
	break;
    case Link:
	setIcon( QPixmap( link_icon ) );
	break;
    }

    if ( itemFileInfo.fileName() == "." ||
	 itemFileInfo.fileName() == ".." )
	setAllowRename( FALSE );

    checkSetText = TRUE;
    calcRect();

    connect( &timer, SIGNAL( timeout() ),
	     this, SLOT( openFolder() ) );


    // now do init stuff, to align in grid and so on
    init();
}

QtFileIconViewItem::~QtFileIconViewItem()
{
}

void QtFileIconViewItem::setText( const QString &text )
{
    QIconViewItem::setText( text );

    if ( checkSetText ) {
	QDir dir( itemFileInfo.dir() );
	dir.rename( itemFileInfo.fileName(), text );
	itemFileName = itemFileInfo.dirPath( TRUE ) + "/" + text;
	itemFileInfo = QFileInfo( itemFileName );
    }
}

bool QtFileIconViewItem::acceptDrop( QMimeSource *e )
{
    if ( type() == Dir && e->provides( "text/uri-list" ) &&
	 allowDrop() )
	return TRUE;

    return FALSE;
}

void QtFileIconViewItem::dropped( QDropEvent *e )
{
    timer.stop();

    if ( !QUriDrag::canDecode( e ) ) {
	e->ignore();
	return;
    }

    QStringList lst;
    QUriDrag::decodeLocalFiles( e, lst );

    QString str;
    if ( e->action() == QDropEvent::Copy )
	str = "Copy\n\n";
    else
	str = "Move\n\n";
    QStringList::Iterator it = lst.begin();
    for ( ; it != lst.end(); ++it )
	str += QString( "   %1\n" ).arg( *it );
    str += QString( "\n"
		    "To\n\n"
		    "	%1" ).arg( filename() );

    QMessageBox::information( iconView(), e->action() == QDropEvent::Copy ? "Copy" : "Move" , str, "Not Implemented" );
    if ( e->action() == QDropEvent::Move )
	QMessageBox::information( iconView(), "Remove" , str, "Not Implemented" );
    e->acceptAction();
}

void QtFileIconViewItem::dragEntered()
{
    if ( type() != Dir ||
	 type() == Dir && !QDir( itemFileName ).isReadable() )
	return;

    timer.start( 1500 );
}

void QtFileIconViewItem::dragLeft()
{
    if ( type() != Dir ||
	 type() == Dir && !QDir( itemFileName ).isReadable() )
	return;

    timer.stop();
}

void QtFileIconViewItem::openFolder()
{
    if ( type() != Dir ||
	 type() == Dir && !QDir( itemFileName ).isReadable() )
	return;

    timer.stop();
    ((QtFileIconView*)iconView())->setDirectory( itemFileName );
}

/*****************************************************************************
 *
 * Class QtFileIconView
 *
 *****************************************************************************/

QtFileIconView::QtFileIconView( const QString &dir, bool isdesktop,
				QWidget *parent, const char *name )
    : QIconView( parent, name ), viewDir( dir ), newFolderNum( 0 ),
      isDesktop( isdesktop ), makeNewGradient( TRUE )
{
    setRastX( 100 );
    setRastY( 75 );
    setResizeMode( Adjust );

    connect( this, SIGNAL( doubleClicked( QIconViewItem * ) ), this, SLOT( itemDoubleClicked( QIconViewItem * ) ) );
    connect( this, SIGNAL( dropped( QDropEvent * ) ), this, SLOT( slotDropped( QDropEvent * ) ) );
    connect( this, SIGNAL( itemRightClicked( QIconViewItem * ) ), this, SLOT( slotItemRightClicked( QIconViewItem * ) ) );
    connect( this, SIGNAL( viewportRightClicked() ), this, SLOT( slotViewportRightClicked() ) );
}

void QtFileIconView::setDirectory( const QString &dir )
{
    viewDir = QDir( dir );
    readDir( viewDir );
}

void QtFileIconView::setDirectory( const QDir &dir )
{
    viewDir = dir;
    readDir( viewDir );
}

void QtFileIconView::newDirectory()
{
    if ( viewDir.mkdir( QString( "New Folder %1" ).arg( ++newFolderNum ) ) ) {
	QFileInfo *fi = new QFileInfo( viewDir, QString( "New Folder %1" ).arg( newFolderNum ) );
	QtFileIconViewItem *item = new QtFileIconViewItem( this, fi );
	delete fi;
	repaintContents( contentsX(), contentsY(), contentsWidth(), contentsHeight() );
	ensureItemVisible( item );
	item->setSelected( TRUE, TRUE );
	setCurrentItem( item );
	repaintItem( item );
	item->rename();
    }
}

QDir QtFileIconView::currentDir()
{
    return viewDir;
}

void QtFileIconView::readDir( const QDir &dir )
{
    if ( !dir.isReadable() )
	return;

    makeNewGradient = FALSE;
    QSize cs( contentsWidth(), contentsHeight() );

    clear();

    emit directoryChanged( dir.absPath() );

    const QFileInfoList *filist = dir.entryInfoList( QDir::DefaultFilter, QDir::DirsFirst | QDir::Name );

    emit startReadDir( filist->count() );

    QFileInfoListIterator it( *filist );
    QFileInfo *fi;
    bool allowRename = FALSE, allowRenameSet = FALSE;
    while ( ( fi = it.current() ) != 0 ) {
	emit readNextDir();
	++it;
	QtFileIconViewItem *item = new QtFileIconViewItem( this, fi );
	if ( !allowRenameSet ) {
	    if ( !QFileInfo( fi->absFilePath() ).isWritable() ||
		 item->text() == "." || item->text() == ".." )
		allowRename = FALSE;
	    else
		allowRename = TRUE;
	    if ( item->text() == "." || item->text() == ".." )
		allowRenameSet = FALSE;
	    else
		allowRenameSet = TRUE;
	}
	item->setAllowRename( allowRename );
	qApp->processEvents();
    }
    emit readDirDone();
    makeNewGradient = TRUE;
    if ( isDesktop && cs != QSize( contentsWidth(), contentsHeight() ) ) {
	int w = QMAX( contentsWidth(), viewport()->width() );
	int h = QMAX( contentsHeight(), viewport()->height() );
	if ( makeNewGradient ) {
	    QSize s = pix.size();
	    makeGradient( pix, Qt::blue, Qt::yellow, w, h );
	    if ( s != pix.size() )
		viewport()->repaint( FALSE );
	}
    }
}

void QtFileIconView::itemDoubleClicked( QIconViewItem *i )
{
    QtFileIconViewItem *item = ( QtFileIconViewItem* )i;

    if ( item->type() == QtFileIconViewItem::Dir ) {
	viewDir = QDir( item->filename() );
	readDir( viewDir );
    } else if ( item->type() == QtFileIconViewItem::Link &&
		QFileInfo( QFileInfo( item->filename() ).readLink() ).isDir() ) {
	viewDir = QDir( QFileInfo( item->filename() ).readLink() );
	readDir( viewDir );
    }
}

QDragObject *QtFileIconView::dragObject()
{
    if ( !currentItem() )
	return 0;

    QStrList lst;
    lst.setAutoDelete( FALSE );

    QtFileIconViewItem *item = ( QtFileIconViewItem* )firstItem();
    for ( ; item; item = ( QtFileIconViewItem* )item->nextItem() )
	if ( item->isSelected() )
	    lst.append( QString( "file://" + item->filename() ).latin1() );

    QUrlDrag *drag = new QUrlDrag( lst, viewport() );
    drag->setPixmap( QPixmap( currentItem()->icon().pixmap( viewMode(), QIconSet::Normal ) ),
		     QPoint( currentItem()->iconRect().width() / 2, currentItem()->iconRect().height() / 2 ) );

    return drag;
}

void QtFileIconView::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Key_N &&
	 ( e->state() & ControlButton ) )
	newDirectory();
    else
	QIconView::keyPressEvent( e );
}

void QtFileIconView::slotDropped( QDropEvent *e )
{
    if ( !QUriDrag::canDecode( e ) ) {
	e->ignore();
	return;
    }

    QStringList lst;
    QUriDrag::decodeLocalFiles( e, lst );

    QString str;
    if ( e->action() == QDropEvent::Copy )
	str = "Copy\n\n";
    else
	str = "Move\n\n";
    QStringList::Iterator it = lst.begin();
    for ( ; it != lst.end(); ++it )
	str += QString( "   %1\n" ).arg( *it );
    str += QString( "\n"
		    "To\n\n"
		    "	%1" ).arg( viewDir.absPath() );

    QMessageBox::information( this, e->action() == QDropEvent::Copy ? "Copy" : "Move" , str, "Not Implemented" );
    if ( e->action() == QDropEvent::Move )
	QMessageBox::information( this, "Remove" , str, "Not Implemented" );
    e->acceptAction();
}

void QtFileIconView::viewLarge()
{
    setViewMode( QIconSet::Large );
    alignInGrid();
}

void QtFileIconView::viewNormal()
{
    setViewMode( QIconSet::Automatic );
    alignInGrid();
}

void QtFileIconView::viewSmall()
{
    setViewMode( QIconSet::Small );
    alignInGrid();
}

void QtFileIconView::alignInGrid()
{
    orderItemsInGrid();
    repaintContents( contentsX(), contentsY(), viewport()->width(), viewport()->height() );
}

void QtFileIconView::slotItemRightClicked( QIconViewItem *item )
{
    if ( !item )
	return;

    setCurrentItem( item );
    item->setSelected( TRUE, TRUE );

    QPopupMenu *menu = new QPopupMenu( this );

    int RENAME_ITEM = menu->insertItem( "Rename Item" );
    int REMOVE_ITEM = menu->insertItem( "Remove Item" );

    menu->setMouseTracking( TRUE );
    int id = menu->exec( QCursor::pos() );

    if ( id == -1 )
	return;

    if ( id == RENAME_ITEM && item->allowRename() )
	item->rename();
    else if ( id == REMOVE_ITEM )
	QMessageBox::information( this, "Not implemented!", "Deleting files not implemented yet..." );
}

void QtFileIconView::slotViewportRightClicked()
{
    QPopupMenu *menu = new QPopupMenu( this );

    menu->insertItem( "&Large View", this, SLOT( viewLarge() ) );
    menu->insertItem( "&Normal View", this, SLOT( viewNormal() ) );
    menu->insertItem( "&Small View", this, SLOT( viewSmall() ) );
    menu->insertSeparator();
    menu->insertItem( "Align Items in &Grid", this, SLOT( alignInGrid() ) );

    menu->setMouseTracking( TRUE );
    menu->exec( QCursor::pos() );
}

int QtFileIconView::dragItems( QDropEvent *e )
{
    if ( QUriDrag::canDecode( e ) ) {
	QStringList l;
	QUriDrag::decodeLocalFiles( e, l );
	return l.count();
    }
    else
	return QIconView::dragItems( e );
}

void QtFileIconView::drawBackground( QPainter *p, const QRect &r )
{
    if ( !isDesktop ) {
	QIconView::drawBackground( p, r );
	return;
    } else {
	QRegion rg( r );
	p->setClipRegion( rg );

	p->drawTiledPixmap( 0, 0, viewport()->width(), viewport()->height(), pix,
			    contentsX(), contentsY() );
    }

}

void QtFileIconView::makeGradient( QPixmap &pmCrop, const QColor &_color1,
				   const QColor &_color2, int _xSize, int _ySize )
{
    QColor cRow;
    int rca, gca, bca;
    int rDiff, gDiff, bDiff;
    float rat;
    unsigned int *p;
    unsigned int rgbRow;

    pmCrop.resize( _xSize, _ySize );
    QImage image( 30, _ySize, 32 );

    rca = _color1.red();
    gca = _color1.green();
    bca = _color1.blue();
    rDiff = _color2.red() - _color1.red();
    gDiff = _color2.green() - _color1.green();
    bDiff = _color2.blue() - _color1.blue();

    for ( int y = _ySize; y > 0; y-- ) {
	p = ( unsigned int* )image.scanLine( _ySize - y );
	rat = 1.0 * y / _ySize;

	cRow.setRgb( rca + (int)( rDiff * rat ),
		     gca + (int)( gDiff * rat ),
		     bca + (int)( bDiff * rat ) );

	rgbRow = cRow.rgb();

	for( int x = 0; x < 30; x++ ) {
	    *p = rgbRow;
	    p++;
	}
    }

    pmCrop.convertFromImage( image );
}

void QtFileIconView::resizeContents( int w, int h )
{
    QIconView::resizeContents( w, h );
    w = QMAX( w, viewport()->width() );
    h = QMAX( h, viewport()->height() );
    if ( makeNewGradient )
	makeGradient( pix, Qt::blue, Qt::yellow, w, h );
}
