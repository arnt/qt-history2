#include "folderdlgimpl.h"
#include "shell.h"
#include <qlineedit.h>
#include <qlistview.h>
#include <qpixmap.h>
#include <qdir.h>

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
    "...........####."
};

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
    "....#ab##.......",
    "....###.........",
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
    "............#ee#"
};

static QPixmap* closedImage = NULL;
static QPixmap* openImage = NULL;

FolderDlgImpl::FolderDlgImpl( QWidget* parent, const char* name, bool modal, WFlags f ) :
    FolderDlg( parent, name, modal, f )
{
    closedImage = new QPixmap( folder_closed_xpm );
    openImage = new QPixmap( folder_open_xpm );
}

void FolderDlgImpl::setup( QString programsFolder, QString folder )
{
    folderName->setText( folder );
    if( programsFolder.length() ) {
	QString topLevel = programsFolder.mid( programsFolder.findRev( '\\' ) + 1 );
	QListViewItem* topItem = new QListViewItem( folderTree, topLevel );
	topItem->setOpen( true );
	topItem->setPixmap( 0, *openImage );

	ScanFolder( programsFolder, topItem );

    }
}

void FolderDlgImpl::ScanFolder( QString folderPath, QListViewItem* parent )
{
    QDir folderDir( folderPath );
    folderDir.setFilter( QDir::Dirs );
    folderDir.setSorting( QDir::Name | QDir::IgnoreCase );
    const QFileInfoList* fiList = folderDir.entryInfoList();
    QFileInfoListIterator it( *fiList );
    QFileInfo* fi;

    while( ( fi = it.current() ) ) {
	if( fi->fileName()[0] != '.' ) { // Exclude dot-dirs
	    QListViewItem* item = new QListViewItem( parent, fi->fileName() );
	    item->setOpen( false );
	    item->setPixmap( 0, *closedImage );
	    ScanFolder( fi->absFilePath(), item );
	}
	++it;
    }
}

void FolderDlgImpl::expandedDir( QListViewItem* item )
{
    item->setPixmap( 0, *openImage );
}

void FolderDlgImpl::collapsedDir( QListViewItem* item )
{
    item->setPixmap( 0, *closedImage );
}

QString FolderDlgImpl::getFolderName()
{
    return folderName->text();
}

/*
** This will replace the contents of the folderName lineedit widget.
**
** The algoritm will traverse the item tree until it gets to the toplevel
** item, prepending each name to the folder name as it goes
*/
void FolderDlgImpl::selectedDir( QListViewItem* item )
{
    QListViewItem* currentItem = item;
    QString newFolder;

    while( currentItem->parent() ) {
	newFolder = currentItem->text( 0 ) + QString( "\\" ) + newFolder;
	currentItem = currentItem->parent();
    }
    newFolder.truncate( newFolder.length() - 1 );
    folderName->setText( newFolder );
}
