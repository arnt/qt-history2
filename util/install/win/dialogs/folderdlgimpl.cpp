#include "folderdlgimpl.h"
#include "../shell.h"
#include <qlineedit.h>
#include <qlistview.h>
#include <qpixmap.h>
#include <qdir.h>

FolderDlgImpl::FolderDlgImpl( QWidget* parent, const char* name, bool modal, WFlags f ) :
    FolderDlg( parent, name, modal, f )
{
}

void FolderDlgImpl::setup( QString programsFolder, QString folder )
{
    folderName->setText( folder );
    if( programsFolder.length() ) {
	QString topLevel = programsFolder.mid( programsFolder.findRev( '\\' ) + 1 );
	QListViewItem* topItem = new QListViewItem( folderTree, topLevel );
	topItem->setOpen( true );
	topItem->setPixmap( 0, *WinShell::getOpenFolderImage() );

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
	    item->setPixmap( 0, *WinShell::getClosedFolderImage() );
	    ScanFolder( fi->absFilePath(), item );
	}
	++it;
    }
}

void FolderDlgImpl::expandedDir( QListViewItem* item )
{
    item->setPixmap( 0, *WinShell::getOpenFolderImage() );
}

void FolderDlgImpl::collapsedDir( QListViewItem* item )
{
    item->setPixmap( 0, *WinShell::getClosedFolderImage() );
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
