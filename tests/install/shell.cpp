#include "shell.h"
#include "environment.h"
#include "folderdlgimpl.h"
#include <qnamespace.h>
#include <qdir.h>
#include <windows.h>
#include <shlobj.h>

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

static const char* file_xpm []={
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
    ".......##......."
};

static const char* info_xpm[] = { 
    "16 16 6 1",
    "# c #0000ff",
    "a c #6868ff",
    "b c #d0d0ff",
    "c c #ffffff",
    "- c #000000",
    ". c none",
    ".....------.....",
    "...--######--...",
    "..-###acca###-..",
    ".-####cccc####-.",
    ".-####acca####-.",
    "-##############-",
    "-######bcc#####-",
    "-####ccccc#####-",
    "-#####cccc#####-",
    "-#####cccc#####-",
    "-#####cccc#####-",
    ".-####cccc####-.",
    ".-###cccccc###-.",
    "..-##########-..",
    "...--#######-...",
    ".....------....."
};

static QPixmap* closedImage = NULL;
static QPixmap* openImage = NULL;
static QPixmap* fileImage = NULL;
static QPixmap* infoImage = NULL;

WinShell::WinShell()
{
    QByteArray buffer( MAX_PATH );
    HRESULT hr;
    IEnumIDList* enumerator = NULL;
    LPITEMIDLIST item;

    localProgramsFolderName = QString::null;
    commonProgramsFolderName = QString::null;
    windowsFolderName = QString::null;

    if( SUCCEEDED( hr = SHGetSpecialFolderLocation( NULL, CSIDL_PROGRAMS, &item ) ) ) {
	if( SHGetPathFromIDList( item, buffer.data() ) ) {
	    localProgramsFolderName = buffer.data();
	    if( int( qWinVersion ) & int( Qt::WV_NT_based ) ) { // On NT we also have a common folder
		if( SUCCEEDED( hr = SHGetSpecialFolderLocation( NULL, CSIDL_COMMON_PROGRAMS, &item ) ) ) {
		    if( SHGetPathFromIDList( item, buffer.data() ) )
			commonProgramsFolderName = buffer.data();
		    else
			qDebug( "Could not get name of common programs folder" );
		}
		else
		    qDebug( "Could not get common programs folder location" );
		if( GetWindowsDirectory( buffer.data(), buffer.size() ) )
		    windowsFolderName = buffer.data();
		else
		    qDebug( "Could not get Windows directory" );
	    }
	}
	else
	    qDebug( "Could not get name of programs folder" );
    }
    else
	qDebug( "Could not get programs folder location" );

    closedImage = new QPixmap( folder_closed_xpm );
    openImage = new QPixmap( folder_open_xpm );
    fileImage = new QPixmap( file_xpm );
    infoImage = new QPixmap( info_xpm );
} 

WinShell::~WinShell()
{
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

QPixmap* WinShell::getClosedFolderImage()
{
    return closedImage;
}

QPixmap* WinShell::getOpenFolderImage()
{
    return openImage;
}

QPixmap* WinShell::getFileImage()
{
    return fileImage;
}

QPixmap* WinShell::getInfoImage()
{
    return infoImage;
}