#include "shell.h"
#include "environment.h"
#include "folderdlgimpl.h"
#include <qnamespace.h>
#include <qdir.h>
#if defined(Q_OS_WIN32)
#include <windows.h>
#include <shlobj.h>
#else
#include <sys/param.h>
#include <sys/mount.h>
#endif

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
#if defined(Q_OS_WIN32)
    QByteArray buffer( MAX_PATH * 2 );
    HRESULT hr;
    IEnumIDList* enumerator = NULL;
    LPITEMIDLIST item;
#endif

    localProgramsFolderName = QString::null;
    commonProgramsFolderName = QString::null;
    windowsFolderName = QString::null;

#if defined(Q_OS_WIN32)
    if( int( qWinVersion() ) & int( Qt::WV_NT_based ) ) {
	if( SUCCEEDED( hr = SHGetSpecialFolderLocation( NULL, CSIDL_PROGRAMS, &item ) ) ) {
	    if( SHGetPathFromIDListA( item, buffer.data() ) ) {
		localProgramsFolderName = buffer.data();
		if( SUCCEEDED( hr = SHGetSpecialFolderLocation( NULL, CSIDL_COMMON_PROGRAMS, &item ) ) ) {
		    if( SHGetPathFromIDListA( item, buffer.data() ) )
			commonProgramsFolderName = buffer.data();
		    else
			qDebug( "Could not get name of common programs folder" );
		}
		else
		    qDebug( "Could not get common programs folder location" );

		if( GetWindowsDirectoryA( buffer.data(), buffer.size() ) )
		    windowsFolderName = buffer.data();
		else
		    qDebug( "Could not get Windows directory" );
	    }
	    else
		qDebug( "Could not get name of programs folder" );
	}
	else
	    qDebug( "Could not get programs folder location" );
    }
    else {
	if( SUCCEEDED( hr = SHGetSpecialFolderLocation( NULL, CSIDL_PROGRAMS, &item ) ) ) {
	    if( SHGetPathFromIDListA( item, buffer.data() ) ) {
		localProgramsFolderName = buffer.data();
		commonProgramsFolderName = buffer.data();
	    }
	    else
		qDebug( "Could not get name of programs folder" );
	}
	else
	    qDebug( "Could not get programs folder location" );
    }
#endif

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


#if defined(Q_OS_WIN32)
HRESULT WinShell::createShortcut( QString folderName, bool common, QString shortcutName, QString target, QString description, QString arguments, QString wrkDir )
{
    IPersistFile* linkFile;
    HRESULT hr;

    // Add .lnk to shortcut name if needed
    if( shortcutName.right( 4 ) != ".lnk" )
	shortcutName += ".lnk";

    folderName = QEnvironment::getFSFileName( folderName );
    if( int( qWinVersion() ) & int( Qt::WV_NT_based ) ) {
        IShellLinkW* link;
	if( SUCCEEDED( hr = CoCreateInstance( CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (void**)&link ) ) ) {
	    if( SUCCEEDED( hr = link->QueryInterface( IID_IPersistFile, (void**)&linkFile ) ) ) {
		link->SetPath( (LPOLESTR)qt_winTchar( target, true ) );
		QString _wrkDir = wrkDir;
		if( !_wrkDir.length() ) {
		    _wrkDir = QDir::convertSeparators( target );
		    // remove the filename
		    int pos = _wrkDir.findRev( '\\' );
		    if ( pos > 0 )
			_wrkDir = _wrkDir.left( pos );
		    else
			_wrkDir = "";
		}

		link->SetWorkingDirectory( (LPOLESTR)qt_winTchar( _wrkDir, true ) );
		if( description.length() )
		    link->SetDescription( (LPOLESTR)qt_winTchar( description, true ) );
		if( arguments.length() )
		    link->SetArguments( (LPOLESTR)qt_winTchar( arguments, true ) );

		hr = linkFile->Save( (LPCOLESTR)qt_winTchar( folderName + QString( "\\" ) + shortcutName, true ), false );
		
		linkFile->Release();
	    }
	    else
		qDebug( "Could not get link file interface" );

	    link->Release();
	}
	else
	    qDebug( "Could not instantiate link object" );
    }
    else {
        IShellLinkA* link;
	if( SUCCEEDED( hr = CoCreateInstance( CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkA, (void**)&link ) ) ) {
	    if( SUCCEEDED( hr = link->QueryInterface( IID_IPersistFile, (void**)&linkFile ) ) ) {
		link->SetPath( QString( target.latin1() ) );
		QString wrkDir = QDir::convertSeparators( target );

		// remove the filename
		int pos = wrkDir.findRev( '\\' );
		if ( pos > 0 )
		    wrkDir = wrkDir.left( pos );
		else
		    wrkDir = "";

		link->SetWorkingDirectory( wrkDir );
		if( description.length() )
		    link->SetDescription( description.latin1() );
		if( arguments.length() )
		    link->SetArguments( arguments.latin1() );

		hr = linkFile->Save( (LPCOLESTR)qt_winTchar( folderName + QString( "\\" ) + shortcutName, true ), false );
		
		linkFile->Release();
	    }
	    else
		qDebug( "Could not get link file interface" );

	    link->Release();
	}
	else
	    qDebug( "Could not instantiate link object" );
    }

    return hr;
}
#endif

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

#if defined(Q_OS_WIN32)
QString WinShell::OLESTR2QString( LPOLESTR str )
{
    QString tmp;
    
    for( int i = 0; str[ i ]; i++ )
	tmp += QChar( str[ i ] );

    return tmp;
}
#endif

/*!
  Returns the free space for the directory.  The space is returned in bytes,
  and should only be considered valid for this particular directory.
*/
#if defined(Q_OS_WIN32)
ULARGE_INTEGER WinShell::dirFreeSpace( QString dirPath )
{
    QString drive = dirPath.left( dirPath.find( '\\' ) );
    ULARGE_INTEGER freeSpace;

    freeSpace.QuadPart = 0;

    if( GetProcAddress( GetModuleHandleA( "kernel32.dll" ), "GetDiskFreeSpaceExA" ) ) {
	ULARGE_INTEGER ulBytesAvailable, ulBytesTotal;
	if( GetDiskFreeSpaceExA( drive.latin1(), &ulBytesAvailable, &ulBytesTotal, NULL ) )
	    freeSpace = ulBytesAvailable;
    }
    else if( GetProcAddress( GetModuleHandleA( "kernel32.dll" ), "GetDiskFreeSpaceA" ) ) {
	DWORD dwSPC, dwBPS, dwClusters, dwTotalClusters;
	if( GetDiskFreeSpaceA( drive.latin1(), &dwSPC, &dwBPS, &dwClusters, &dwTotalClusters ) )
	    freeSpace.QuadPart = dwSPC * dwBPS * dwClusters;
    }
    return freeSpace;
}
#elif defined(Q_OS_MACX)
long  WinShell::dirFreeSpace( QString dirPath )
{
    struct statfs buf;
    if (statfs( dirPath.local8Bit(), &buf ) != -1)
	return buf.f_bavail * buf.f_bsize;
    return 0;
}
#endif

