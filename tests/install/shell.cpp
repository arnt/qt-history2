#include "shell.h"
#include "environment.h"
#include "folderdlgimpl.h"
#include <qnamespace.h>
#include <qdir.h>
#include <windows.h>
#include <shlobj.h>

WinShell::WinShell()
{
    QByteArray buffer( MAX_PATH );
    HRESULT hr;
    IEnumIDList* enumerator = NULL;
    LPITEMIDLIST item;

    desktopFolder = NULL;
    localProgramsFolder = NULL;
    commonProgramsFolder = NULL;
    localProgramsFolderName = QString::null;
    commonProgramsFolderName = QString::null;

    if( SUCCEEDED( hr = SHGetDesktopFolder( &desktopFolder ) ) ) {
	if( SUCCEEDED( hr = SHGetSpecialFolderLocation( NULL, CSIDL_PROGRAMS, &item ) ) ) {
	    if( SHGetPathFromIDList( item, buffer.data() ) ) {
		localProgramsFolderName = buffer.data();
		if( SUCCEEDED( hr = desktopFolder->BindToObject( (LPCITEMIDLIST)item, NULL, IID_IShellFolder, (void**)&localProgramsFolder ) ) ) {
		    if( int( qWinVersion ) & int( Qt::WV_NT_based ) ) { // On NT we also have a common folder
			if( SUCCEEDED( hr = SHGetSpecialFolderLocation( NULL, CSIDL_COMMON_PROGRAMS, &item ) ) ) {
			    if( SHGetPathFromIDList( item, buffer.data() ) ) {
				commonProgramsFolderName = buffer.data();
				if( FAILED( hr = desktopFolder->BindToObject( (LPCITEMIDLIST)item, NULL, IID_IShellFolder, (void**)&commonProgramsFolder ) ) )
				    qDebug( "Could not get interface to programs folder" );
			    }
			    else
				qDebug( "Could not get name of common programs folder" );
			}
			else
			    qDebug( "Could not get common programs folder location" );
		    }
		}
		else
		    qDebug( "Could not get interface to programs folder" );
	    }
	    else
		qDebug( "Could not get name of programs folder" );
	}
	else
	    qDebug( "Could not get programs folder location" );
    }
    else
	qDebug( "Could not get desktop interface" );
} 

WinShell::~WinShell()
{
    if( localProgramsFolder )
	localProgramsFolder->Release();
    if( commonProgramsFolder )
	commonProgramsFolder->Release();
    if( desktopFolder )
	desktopFolder->Release();
}

HRESULT WinShell::EnumFolder( IShellFolder* folder, QStringList& entryList )
{
    HRESULT hr;
    IEnumIDList* enumerator;
    QString entries;

    if( SUCCEEDED( hr = folder->EnumObjects( NULL, SHCONTF_FOLDERS, &enumerator ) ) ) {
	LPITEMIDLIST item;
	while( ( hr = enumerator->Next( 1, &item, NULL ) ) == S_OK ) {
	    STRRET name;
	    if( SUCCEEDED( hr = folder->GetDisplayNameOf( item, SHGDN_INFOLDER, &name ) ) ) {
		switch( name.uType ) {
		case STRRET_CSTR:
		    entries += &name.cStr[ 0 ];
		    break;
		case STRRET_WSTR:
		    for( int i = 0; name.pOleStr[ i ] != 0; i++ )
			entries += QChar( name.pOleStr[ i ] );
		    break;
		}
		entries += "\\";
	    }
	    else {
		qDebug( "Could not get display name for entry" );
		break;
	    }
	}
        enumerator->Release();
    }
    else
	qDebug( "Could not get enumerator" );

    entryList = QStringList::split( '\\', entries );
    return hr;
}

QString WinShell::selectFolder( QString folderName, bool common )
{
    QStringList folders;
    FolderDlgImpl dlg;

    if( common )
	dlg.setup( commonProgramsFolderName, folderName );
    else
	dlg.setup( localProgramsFolderName, folderName );

    if( dlg.exec() ) {
	return dlg.getFolderName();
    }
    else
	return folderName;
}

QString WinShell::createFolder( QString folderName, bool common )
{
    QDir folderDir;
    QString folderPath;

    if( common )
	folderPath = commonProgramsFolderName + QString( "\\" ) + folderName;
    else
	folderPath = localProgramsFolderName + QString( "\\" ) + folderName;

    folderDir.setPath( folderPath );

    if( !folderDir.exists( folderPath ) )
	if( !createDir( folderPath ) )
	    return QString::null;

    return folderPath;
}


HRESULT WinShell::createShortcut( QString folderName, bool common, QString shortcutName, QString target, QString description, QString arguments )
{
    IShellLink* link;
    IPersistFile* linkFile;
    LPITEMIDLIST item;    
    HRESULT hr;

    // Add .lnk to shortcut name if needed
    if( shortcutName.right( 4 ) != ".lnk" )
	shortcutName += ".lnk";

    if( SUCCEEDED( hr = CoCreateInstance( CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&link ) ) ) {
	if( SUCCEEDED( hr = link->QueryInterface( IID_IPersistFile, (void**)&linkFile ) ) ) {
	    link->SetPath( QString( target.local8Bit() ) );
	    if( description.length() )
		link->SetDescription( description.local8Bit() );
	    if( arguments.length() )
		link->SetArguments( arguments.local8Bit() );

	    hr = linkFile->Save( (LPCOLESTR)QString2OLESTR( folderName + QString( "\\" ) + shortcutName ).data(), false );
	    
	    linkFile->Release();
	}
	else
	    qDebug( "Could not get link file interface" );

	link->Release();
    }
    else
	qDebug( "Could not instantiate link object" );

    return hr;
}

bool WinShell::createDir( QString fullPath )
{
    QStringList hierarchy = QStringList::split( QString( "\\" ), fullPath );
    QString pathComponent, tmpPath;
    QDir dirTmp;
    bool success;

    for( QStringList::Iterator it = hierarchy.begin(); it != hierarchy.end(); ++it ) {
	pathComponent = *it + "\\";
	tmpPath += pathComponent;
	success = dirTmp.mkdir( tmpPath );
    }
    return success;
}

QByteArray WinShell::QString2OLESTR( QString str )
{
    QByteArray buffer;

    buffer.resize( str.length() * 2 + 2 );
    const QChar *data = str.unicode();
    for ( int i = 0; i < (int)str.length(); ++i ) {
	buffer[ 2*i ] = data[ i ].cell();
	buffer[ (2*i)+1 ] = data[ i ].row();
    }
    buffer[ (2*i) ] = 0;
    buffer[ (2*i)+1 ] = 0;

    return buffer;
}
