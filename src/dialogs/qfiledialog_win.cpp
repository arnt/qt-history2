/****************************************************************************
** $Id$
**
** Implementation of QFileDialog Windows-specific functionality
**
** Created : 990601
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the dialogs module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qfiledialog.h"

#ifndef QT_NO_FILEDIALOG

#include "qapplication.h"
#include "private/qapplication_p.h"
#include "qt_windows.h"
#include "qregexp.h"
#include "qbuffer.h"
#include "qdir.h"
#include "qstringlist.h"
#include "qlibrary.h"

#include "shlobj.h"

#ifdef Q_OS_TEMP
#include "commdlg.h"
#endif

extern const char qt_file_dialog_filter_reg_exp[]; // defined in qfiledialog.cpp

const int maxNameLen = 255;
const int maxMultiLen = 16383;

typedef LPITEMIDLIST (WINAPI *PtrSHBrowseForFolder)(BROWSEINFO*);
static PtrSHBrowseForFolder ptrSHBrowseForFolder = 0;
typedef BOOL (WINAPI *PtrSHGetPathFromIDList)(LPITEMIDLIST,LPSTR);
static PtrSHGetPathFromIDList ptrSHGetPathFromIDList = 0;

static void resolveLibs()
{
#ifndef Q_OS_TEMP
    QLibrary* lib = new QLibrary("shell32");
    lib->setAutoUnload( FALSE );
    static bool triedResolve = FALSE;
    if ( !ptrSHBrowseForFolder && !ptrSHGetPathFromIDList && !triedResolve ) {
	triedResolve = TRUE;
	ptrSHBrowseForFolder = (PtrSHBrowseForFolder) lib->resolve( "SHBrowseForFolderW" );
	ptrSHGetPathFromIDList = (PtrSHGetPathFromIDList) lib->resolve( "SHGetPathFromIDListW" );
    }
    delete lib;
#endif
}

// Returns the wildcard part of a filter.
static QString extractFilter( const QString& rawFilter )
{
    QString result = rawFilter;
    QRegExp r( QString::fromLatin1(qt_file_dialog_filter_reg_exp) );
    int index = r.search( result );
    if ( index >= 0 )
	result = r.cap( 1 );
    return result.replace( QRegExp(QString::fromLatin1(" ")), QChar(';') );
}

// Makes a list of filters from ;;-separated text.
static QStringList makeFiltersList( const QString &filter )
{
    QString f( filter );

    if ( f.isEmpty( ) )
	f = QFileDialog::tr( "All Files (*.*)" );

    if ( f.isEmpty() )
	return QStringList();

    int i = f.find( ";;", 0 );
    QString sep( ";;" );
    if ( i == -1 ) {
	if ( f.find( "\n", 0 ) != -1 ) {
	    sep = "\n";
	    i = f.find( sep, 0 );
	}
    }

    return QStringList::split( sep, f  );
}

// Makes a NUL-oriented Windows filter from a Qt filter.
static QString winFilter( const QString& filter )
{
    QStringList filterLst = makeFiltersList( filter );
    QStringList::Iterator it = filterLst.begin();
    QString winfilters;
    for ( ; it != filterLst.end(); ++it ) {
        winfilters += *it;
        winfilters += QChar::null;
        winfilters += extractFilter(*it);
        winfilters += QChar::null;
    }
    winfilters += QChar::null;
    return winfilters;
}

static QString selFilter( const QString& filter, DWORD idx )
{
    QStringList filterLst = makeFiltersList( filter );
    return filterLst[(int)idx - 1];
}

#ifndef Q_OS_TEMP
// Static vars for OFNA funcs:
static QCString aInitDir;
static QCString aInitSel;
static QCString aTitle;
static QCString aFilter;
// Use ANSI strings and API

static
OPENFILENAMEA* makeOFNA( QWidget* parent,
			 const QString& initialSelection,
			 const QString& initialDirectory,
			 const QString& title,
			 const QString& filters,
			 QFileDialog::Mode mode )
{
    if ( parent )
	parent = parent->topLevelWidget();
    else
	parent = qApp->mainWidget();

    aTitle = title.local8Bit();
    aInitDir = QDir::convertSeparators( initialDirectory ).local8Bit();
    if ( initialSelection.isEmpty() )
	aInitSel = "";
    else
	aInitSel = QDir::convertSeparators( initialSelection ).local8Bit();
    int maxLen = mode == QFileDialog::ExistingFiles ? maxMultiLen : maxNameLen;
    aInitSel.resize( maxLen + 1 );		// make room for return value
    aFilter = filters.local8Bit();

    OPENFILENAMEA* ofn = new OPENFILENAMEA;
    memset( ofn, 0, sizeof(OPENFILENAMEA) );

    ofn->lStructSize	= sizeof(OPENFILENAMEA);
    ofn->hwndOwner	= parent ? parent->winId() : 0;
    ofn->lpstrFilter	= aFilter.data();
    ofn->lpstrFile	= aInitSel.data();
    ofn->nMaxFile	= maxLen;
    ofn->lpstrInitialDir = aInitDir.data();
    ofn->lpstrTitle	= aTitle.data();
    ofn->Flags		= ( OFN_NOCHANGEDIR | OFN_HIDEREADONLY );

    if ( mode == QFileDialog::ExistingFile ||
	 mode == QFileDialog::ExistingFiles )
	ofn->Flags |= ( OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST );
    if ( mode == QFileDialog::ExistingFiles )
	ofn->Flags |= ( OFN_ALLOWMULTISELECT | OFN_EXPLORER );

    return ofn;
}

static void cleanUpOFNA( OPENFILENAMEA** ofn )
{
    delete *ofn;
    *ofn = 0;
}
#endif


// Static vars for OFN funcs:
static TCHAR* tTitle;
static TCHAR* tInitDir;
static TCHAR* tInitSel;
static TCHAR* tFilter;

static
OPENFILENAME* makeOFN( QWidget* parent,
		       const QString& initialSelection,
		       const QString& initialDirectory,
		       const QString& title,
		       const QString& filters,
		       QFileDialog::Mode mode )
{
    if ( parent )
	parent = parent->topLevelWidget();
    else
	parent = qApp->mainWidget();

    QString initDir = QDir::convertSeparators( initialDirectory );
    QString initSel = QDir::convertSeparators( initialSelection );

    tTitle = (TCHAR*)qt_winTchar_new( title );
    tInitDir = (TCHAR*)qt_winTchar_new( initDir );
    tFilter = (TCHAR*)qt_winTchar_new( filters );
    int maxLen = mode == QFileDialog::ExistingFiles ? maxMultiLen : maxNameLen;
    tInitSel = new TCHAR[maxLen+1];
    tInitSel[0] = 0;
    if ( initSel.length() > 0 && initSel.length() <= (uint)maxLen )
	memcpy( tInitSel, qt_winTchar( initSel, TRUE ),
		(initSel.length()+1) * sizeof(TCHAR) );

    OPENFILENAME* ofn = new OPENFILENAME;
    memset( ofn, 0, sizeof(OPENFILENAME) );

    ofn->lStructSize	= sizeof(OPENFILENAME);
    ofn->hwndOwner	= parent ? parent->winId() : 0;
    ofn->lpstrFilter	= tFilter;
    ofn->lpstrFile	= tInitSel;
    ofn->nMaxFile	= maxLen;
    ofn->lpstrInitialDir = tInitDir;
    ofn->lpstrTitle	= tTitle;
    ofn->Flags		= ( OFN_NOCHANGEDIR | OFN_HIDEREADONLY );

    if ( mode == QFileDialog::ExistingFile ||
	 mode == QFileDialog::ExistingFiles )
	ofn->Flags |= ( OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST );
    if ( mode == QFileDialog::ExistingFiles )
	ofn->Flags |= ( OFN_ALLOWMULTISELECT | OFN_EXPLORER );

    return ofn;
}


static void cleanUpOFN( OPENFILENAME** ofn )
{
    delete *ofn;
    *ofn = 0;
    delete tFilter;
    tFilter = 0;
    delete tTitle;
    tTitle = 0;
    delete tInitDir;
    tInitDir = 0;
    delete tInitSel;
    tInitSel = 0;
}

extern void qt_enter_modal( QWidget* );
extern void qt_leave_modal( QWidget* );

QString QFileDialog::winGetOpenFileName( const QString &initialSelection,
					 const QString &filter,
					 QString* initialDirectory,
					 QWidget *parent, const char* /*name*/,
					 const QString& caption,
					 QString* selectedFilter )
{
    QString result;

    QString isel = initialSelection;
    if ( initialDirectory && initialDirectory->left( 5 ) == "file:" )
	initialDirectory->remove( 0, 5 );
    QFileInfo fi( *initialDirectory );

    if ( initialDirectory && !fi.isDir() ) {
	*initialDirectory = fi.dirPath( TRUE );
	isel = fi.fileName();
    }

    if ( !fi.dir().exists() )
	*initialDirectory = QDir::homeDirPath();

    QString title = caption;
    if ( title.isNull() )
	title = tr("Open");

    DWORD selFilIdx;        

    if ( parent )
	qt_enter_modal( parent );
#ifndef Q_OS_TEMP
    if ( qt_winver & WV_DOS_based ) {
	// Use ANSI strings and API
	OPENFILENAMEA* ofn = makeOFNA( parent, isel,
				       *initialDirectory, title,
				       winFilter(filter), ExistingFile );
	if ( GetOpenFileNameA( ofn ) ) {
	    result = QString::fromLocal8Bit( ofn->lpstrFile );
	    selFilIdx = ofn->nFilterIndex;
	}
	cleanUpOFNA( &ofn );
    } else
#endif
    {
	// Use Unicode or ANSI strings and API
	OPENFILENAME* ofn = makeOFN( parent, isel,
				     *initialDirectory, title,
				     winFilter(filter), ExistingFile );
	if ( GetOpenFileName( ofn ) ) {
	    result = qt_winQString( ofn->lpstrFile );
	    selFilIdx = ofn->nFilterIndex;
	}
	cleanUpOFN( &ofn );
    }
    if ( parent )
	qt_leave_modal( parent );

    if ( result.isEmpty() ) {
	return result;
    }
    else {
	QFileInfo fi( result );
	*initialDirectory = fi.dirPath();
	if ( selectedFilter )
	    *selectedFilter = selFilter( filter, selFilIdx );	
	return fi.absFilePath();
    }
}


QString QFileDialog::winGetSaveFileName( const QString &initialSelection,
					 const QString &filter,
					 QString* initialDirectory,
					 QWidget *parent, const char* /*name*/,
					 const QString& caption,
					 QString* selectedFilter )
{
    QString result;

    QString isel = initialSelection;
    if ( initialDirectory && initialDirectory->left( 5 ) == "file:" )
	initialDirectory->remove( 0, 5 );
    QFileInfo fi( *initialDirectory );

    if ( initialDirectory && !fi.isDir() ) {
	*initialDirectory = fi.dirPath( TRUE );
	isel = fi.fileName();
    }

    if ( !fi.dir().exists() )
	*initialDirectory = QDir::homeDirPath();

    QString title = caption;
    if ( title.isNull() )
	title = tr("Save As");
    
    DWORD selFilIdx;

    if ( parent )
	qt_enter_modal( parent );
#ifndef Q_OS_TEMP
    if ( qt_winver & WV_DOS_based ) {
	// Use ANSI strings and API
	OPENFILENAMEA* ofn = makeOFNA( parent, isel,
				       *initialDirectory, title,
				       winFilter(filter), AnyFile );
	if ( GetSaveFileNameA( ofn ) ) {
	    result = QString::fromLocal8Bit( ofn->lpstrFile );
	    selFilIdx = ofn->nFilterIndex;
	}
	cleanUpOFNA( &ofn );
    } else
#endif
    {
	// Use Unicode or ANSI strings and API
	OPENFILENAME* ofn = makeOFN( parent, isel,
				     *initialDirectory, title,
				     winFilter(filter), AnyFile );
	if ( GetSaveFileName( ofn ) ) {
	    result = qt_winQString( ofn->lpstrFile );
	    selFilIdx = ofn->nFilterIndex;
	}
	cleanUpOFN( &ofn );
    }
    if ( parent )
	qt_leave_modal( parent );

    if ( result.isEmpty() ) {
	return result;
    }
    else {
	QFileInfo fi( result );
	*initialDirectory = fi.dirPath();
	if ( selectedFilter )
	    *selectedFilter = selFilter( filter, selFilIdx );	
	return fi.absFilePath();
    }
}



QStringList QFileDialog::winGetOpenFileNames( const QString &filter,
					      QString* initialDirectory,
					      QWidget *parent,
					      const char* /*name*/,
					      const QString& caption,
					      QString* selectedFilter )
{
    QStringList result;
    QFileInfo fi;
    QDir dir;

    if ( initialDirectory && initialDirectory->left( 5 ) == "file:" )
	initialDirectory->remove( 0, 5 );
    fi = QFileInfo( *initialDirectory );

    if ( initialDirectory && !fi.isDir() ) {
	*initialDirectory = fi.dirPath( TRUE );
    }

    if ( !fi.exists() )
	*initialDirectory = QDir::homeDirPath();

    QString title = caption;
    if ( title.isNull() )
	title = tr( "Open ");

    DWORD selFilIdx;

    if ( parent )
	qt_enter_modal( parent );
#ifndef Q_OS_TEMP
    if ( qt_winver & WV_DOS_based ) {
	// Use ANSI strings and API
	OPENFILENAMEA* ofn = makeOFNA( parent, QString::null,
				       *initialDirectory, title,
				       winFilter( filter ), ExistingFiles );
	if ( GetOpenFileNameA( ofn ) ) {
	    QCString fileOrDir = ofn->lpstrFile;
	    selFilIdx = ofn->nFilterIndex;
	    int offset = fileOrDir.length() + 1;
	    if ( ofn->lpstrFile[offset] == '\0' ) {
		// Only one file selected; has full path
		fi.setFile( QString::fromLocal8Bit( fileOrDir ) );
		QString res = fi.absFilePath();
		if ( !res.isEmpty() )
		    result.append( res );
	    }
	    else {
		// Several files selected; first string is path
		dir.setPath( QString::fromLocal8Bit( fileOrDir ) );
		QCString f;
		while( !( f = ofn->lpstrFile + offset).isEmpty() ) {
		    fi.setFile( dir, QString::fromLocal8Bit( f ) );
		    QString res = fi.absFilePath();
		    if ( !res.isEmpty() )
			result.append( res );
		    offset += f.length() + 1;
		}
	    }
	    cleanUpOFNA( &ofn );
	}
    } else 
#endif
    {
	// Use Unicode or ANSI strings and API
	OPENFILENAME* ofn = makeOFN( parent, QString::null,
				     *initialDirectory, title,
				     winFilter( filter ), ExistingFiles );
	if ( GetOpenFileName( ofn ) ) {
	    QString fileOrDir = qt_winQString( ofn->lpstrFile );
	    selFilIdx = ofn->nFilterIndex;
	    int offset = fileOrDir.length() + 1;
	    if ( ofn->lpstrFile[offset] == 0 ) {
		// Only one file selected; has full path
		fi.setFile( fileOrDir );
		QString res = fi.absFilePath();
		if ( !res.isEmpty() )
		    result.append( res );
	    }
	    else {
		// Several files selected; first string is path
		dir.setPath( fileOrDir );
		QString f;
		while( !(f=qt_winQString(ofn->lpstrFile+offset)).isEmpty() ) {
		    fi.setFile( dir, f );
		    QString res = fi.absFilePath();
		    if ( !res.isEmpty() )
			result.append( res );
		    offset += f.length() + 1;
		}
	    }
	}
	cleanUpOFN( &ofn );
    }
    if ( parent )
	qt_leave_modal( parent );

    *initialDirectory = fi.dirPath();
    if ( !result.isEmpty() && selectedFilter )
	*selectedFilter = selFilter( filter, selFilIdx );	
    return result;
}

// MFC Directory Dialog. Contrib: Steve Williams (minor parts from Scott Powers)

static int __stdcall winGetExistDirCallbackProc(HWND hwnd,
						UINT uMsg,
						LPARAM lParam,
						LPARAM lpData)
{
#ifndef Q_OS_TEMP
    if (uMsg == BFFM_INITIALIZED && lpData != NULL) {
	QString *initDir = (QString *)(lpData);
	if (!initDir->isEmpty()) {
	    TCHAR *dispName = (TCHAR*)qt_winTchar_new(*initDir);
	    if (qt_winver != QFileDialog::WV_NT)
		SendMessageA(hwnd, BFFM_SETSELECTION, TRUE, Q_ULONG(dispName));
	    else
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, Q_ULONG(dispName));
	    delete dispName;
	    dispName = 0;
	}
    } else if (uMsg == BFFM_SELCHANGED) {
#if defined(UNICODE)
	resolveLibs();
	if ( ptrSHGetPathFromIDList && qt_winver & Qt::WV_NT_based ) {
	    TCHAR path[MAX_PATH];
	    ptrSHGetPathFromIDList(LPITEMIDLIST(lParam), (LPSTR)path);
	    QString tmpStr = qt_winQString(path);
	    if (!tmpStr.isEmpty())
		SendMessage(hwnd, BFFM_ENABLEOK, 1, 1);
	    else
		SendMessage(hwnd, BFFM_ENABLEOK, 0, 0);
	    SendMessage(hwnd, BFFM_SETSTATUSTEXT, 1, Q_ULONG(path));
	} else 
#endif
	{
	    char path[MAX_PATH];
	    SHGetPathFromIDListA(LPITEMIDLIST(lParam), path);
	    QString tmpStr = QString::fromLocal8Bit(path);
	    if (!tmpStr.isEmpty())
		SendMessageA(hwnd, BFFM_ENABLEOK, 1, 1);
	    else
		SendMessageA(hwnd, BFFM_ENABLEOK, 0, 0);
	    SendMessageA(hwnd, BFFM_SETSTATUSTEXT, 1, Q_ULONG(path));
	}
    }
#endif
    return 0;
}


QString QFileDialog::winGetExistingDirectory(const QString& initialDirectory,
					     QWidget *parent,
					     const char* /*name*/,
					     const QString& caption )
{
#ifndef Q_OS_TEMP
    QString result;
    if ( parent )
	parent = parent->topLevelWidget();
    else
	parent = qApp->mainWidget();
    QString title = caption;
    if ( title.isNull() )
	title = tr( "Select a Directory" );

    if ( parent )
	qt_enter_modal( parent );
#if defined(UNICODE)
    resolveLibs();
    if ( ptrSHBrowseForFolder && ptrSHGetPathFromIDList && qt_winver & Qt::WV_NT_based ) { 
	QString initDir = QDir::convertSeparators(initialDirectory);
	TCHAR path[MAX_PATH];
	TCHAR initPath[MAX_PATH];
	initPath[0] = 0;
	path[0] = 0;
	tTitle = (TCHAR*)qt_winTchar_new( title );
	BROWSEINFO bi;
	bi.hwndOwner = (parent ? parent->winId() : 0);
	bi.pidlRoot = NULL;
	bi.lpszTitle = tTitle;
	bi.pszDisplayName = initPath;
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;
	bi.lpfn = winGetExistDirCallbackProc;
	bi.lParam = Q_ULONG(&initDir);
	LPITEMIDLIST pItemIDList = ptrSHBrowseForFolder(&bi);
	if (pItemIDList) {
	    ptrSHGetPathFromIDList(pItemIDList, (LPSTR)path);
	    IMalloc *pMalloc;
	    if (SHGetMalloc(&pMalloc) != NOERROR)
		result = QString::null;
	    else {
		pMalloc->Free(pItemIDList);
		pMalloc->Release();
		result = qt_winQString(path);
	    }
	} else
	    result = QString::null;
	delete tTitle;
	tTitle = 0;
    } else 
#endif
    {
	QString initDir = QDir::convertSeparators(initialDirectory);
	char path[MAX_PATH];
	char initPath[MAX_PATH];
	initPath[0]=0;
	path[0]=0;
	BROWSEINFOA bi;
	bi.hwndOwner = (parent ? parent->winId() : 0);
	bi.pidlRoot = NULL;
	bi.lpszTitle = title.local8Bit();
	bi.pszDisplayName = initPath;
	bi.ulFlags = BIF_RETURNONLYFSDIRS;
	bi.lpfn = winGetExistDirCallbackProc;
	bi.lParam = Q_ULONG(&initDir);
	LPITEMIDLIST pItemIDList = SHBrowseForFolderA(&bi);
	if (pItemIDList) {
	    SHGetPathFromIDListA(pItemIDList, path);
	    IMalloc *pMalloc;
	    if (SHGetMalloc(&pMalloc) != NOERROR)
		result = QString::null;
	    else {
		pMalloc->Free(pItemIDList);
		pMalloc->Release();
		result = QString::fromLocal8Bit(path);
	    }
	} else
	    result = QString::null;
    }
    if ( parent )
	qt_leave_modal( parent );

    return result;
#else
    return QString::null;
#endif
}


#endif
