/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qfiledialog_win.cpp#6 $
**
** Implementation of QFileDialog Windows-specific functionality
**
** Created : 990601
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qfiledialog.h"
#include "qapplication.h"
#include "qt_windows.h"
#include "qregexp.h"
#include "qbuffer.h"
#include "qstringlist.h"

extern Qt::WindowsVersion qt_winver;	// defined in qapplication_win.cpp

const int maxNameLen = 255;
const int maxMultiLen = 16383;

static
void splitFilter( const QString& rawFilter, QString* filterName,
		  QString* filterExp )
{
    if ( !filterName || !filterExp )
	return;
    QRegExp r( QString::fromLatin1("([a-zA-Z0-9;\\.\\*\\?]*)$") );
    int len;
    int index = r.match( rawFilter, 0, &len );
    if ( index >= 0 ) {
	*filterExp = rawFilter.mid( index+1, len-2 );
	*filterName = qApp->translate( "QFileDialog",
				       rawFilter.left( index-1 ) );
    }
    else {
	*filterExp = rawFilter;
	*filterName = rawFilter;
	filterName->prepend( QString::fromLatin1( "(" ) );
	filterName->append( QString::fromLatin1( ")" ) );	
    }
}


static
void addFilterA( QBuffer* buf, const QString& rawFilter )
{
    if ( !rawFilter.isEmpty() ) {
	QString filterName;
	QString filterExp;
	splitFilter( rawFilter, &filterName, &filterExp );
	QCString fName = filterName.local8Bit();
	buf->writeBlock( fName.data(), fName.length() + 1 );
	QCString fExp = filterExp.local8Bit();
	buf->writeBlock( fExp.data(), fExp.length() + 1 );
    }
}


static
void addFilter( QBuffer* buf, const QString& rawFilter )
{
    if ( !rawFilter.isEmpty() ) {
	QString fName;
	QString fExp;
	splitFilter( rawFilter, &fName, &fExp );
	buf->writeBlock( (const char*)qt_winTchar( fName, TRUE ),
			 ( fName.length() + 1 ) * sizeof(TCHAR) );
	buf->writeBlock( (const char*)qt_winTchar( fExp, TRUE ),
			 ( fExp.length() + 1 ) * sizeof(TCHAR) );
    }
}

// Static vars for OFNA funcs:
QCString aInitDir;
QCString aInitSel;
QCString aTitle;
QByteArray aFilter;
// Use ANSI strings and API

static QStringList makeFiltersList( const QString &filter )
{
    if ( filter.isEmpty() )
        return QStringList();

    int i = filter.find( ";;", 0 );
    QString sep( ";;" );
    if ( i == -1 ) {
        if ( filter.find( "\n", 0 ) != -1 ) {
            sep = "\n";
            i = filter.find( sep, 0 );
        }
    }

    return QStringList::split( filter, sep );
}

static
OPENFILENAMEA* makeOFNA( QWidget* parent,
			 const QString& initialSelection,
			 const QString& initialDirectory,
			 const char* rawTitle,
			 const QString& rawFilter,
			 QFileDialog::Mode mode )
{
    if ( parent )
	parent = parent->topLevelWidget();
    else
	parent = qApp->mainWidget();

    aTitle = qApp->translate( "QFileDialog", rawTitle ).local8Bit();
    aInitDir = QDir::convertSeparators( initialDirectory ).local8Bit();
    if ( initialSelection.isEmpty() )
	aInitSel = "";
    else
	aInitSel = QDir::convertSeparators( initialSelection ).local8Bit();
    int maxLen = mode == QFileDialog::ExistingFiles ? maxMultiLen : maxNameLen;
    aInitSel.resize( maxLen + 1 );		// make room for return value

//      aFilter.resize( 0 );	// > 0 for opt?
//      QBuffer buf( aFilter );
//      buf.open( IO_WriteOnly );
//      //addFilterA( &buf, rawFilter );
//      //addFilterA( &buf, QString::fromLatin1( "All Files (*.*)" ) );
//      buf.putch( 0 );				// Termination
//      buf.close();

    QStringList filterLst = makeFiltersList( rawFilter );
    QDir tmpDir;
    QStringList::Iterator it = filterLst.begin();
    QString filterBuffer;
    for ( ; it != filterLst.end(); ++it ) {
        tmpDir.setNameFilter( *it );
        QString pattern = tmpDir.nameFilter();
        filterBuffer += *it;
        filterBuffer += QChar::null; 
        filterBuffer += pattern;
        filterBuffer += QChar::null;
    }
    filterBuffer += QChar::null;
    
    OPENFILENAMEA* ofn = new OPENFILENAMEA;
    memset( ofn, 0, sizeof(OPENFILENAMEA) );

    ofn->lStructSize	= sizeof(OPENFILENAMEA);
    ofn->hwndOwner	= parent ? parent->winId() : 0;
    ofn->lpstrFilter	= filterBuffer.latin1();
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


static
void cleanUpOFNA( OPENFILENAMEA** ofn )
{
    delete *ofn;
    *ofn = 0;
}


// Static vars for OFN funcs:
TCHAR* tTitle;
TCHAR* tInitDir;
TCHAR* tInitSel;
QByteArray tFilter;

static
OPENFILENAME* makeOFN( QWidget* parent,
		       const QString& initialSelection,
		       const QString& initialDirectory,
		       const char* rawTitle,
		       const QString& rawFilter,
		       QFileDialog::Mode mode )
{
    if ( parent )
	parent = parent->topLevelWidget();
    else
	parent = qApp->mainWidget();

    QString title = qApp->translate( "QFileDialog", rawTitle );
    QString initDir = QDir::convertSeparators( initialDirectory );
    QString initSel = QDir::convertSeparators( initialSelection );

    tTitle = (TCHAR*)qt_winTchar_new( title );
    tInitDir = (TCHAR*)qt_winTchar_new( initDir );
    int maxLen = mode == QFileDialog::ExistingFiles ? maxMultiLen : maxNameLen;
    tInitSel = new TCHAR[maxLen+1];
    tInitSel[0] = 0;
    if ( initSel.length() <= (uint)maxLen )
	memcpy( tInitSel, qt_winTchar( initSel, TRUE ),
		(initSel.length()+1) * sizeof(TCHAR) );

    tFilter.resize( 0 );	// > 0 for opt?
    QBuffer buf( tFilter );
    buf.open( IO_WriteOnly );
    addFilter( &buf, rawFilter );
    addFilter( &buf, QString::fromLatin1( "All Files (*.*)" ) );
    TCHAR nullChar = 0;
    buf.writeBlock( (const char*)&nullChar, sizeof(TCHAR) );  // Termination
    buf.close();

    OPENFILENAME* ofn = new OPENFILENAME;
    memset( ofn, 0, sizeof(OPENFILENAME) );

    ofn->lStructSize	= sizeof(OPENFILENAME);
    ofn->hwndOwner	= parent ? parent->winId() : 0;
    ofn->lpstrFilter	= (TCHAR*)tFilter.data();
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


static
void cleanUpOFN( OPENFILENAME** ofn )
{
    delete *ofn;
    *ofn = 0;
    delete tTitle;
    tTitle = 0;
    delete tInitDir;
    tInitDir = 0;
    delete tInitSel;
    tInitSel = 0;
}

QString QFileDialog::winGetOpenFileName( const QString &initialSelection,
					 const QString &filter,
					 QString* initialDirectory,
					 QWidget *parent, const char* /*name*/ )
{
    QString result;

    if ( qt_winver != WV_NT ) {
	// Use ANSI strings and API
	OPENFILENAMEA* ofn = makeOFNA( parent, initialSelection,
				       *initialDirectory, "Open",
				       filter, ExistingFile );
	if ( GetOpenFileNameA( ofn ) )
	    result = QString::fromLocal8Bit( ofn->lpstrFile );
	cleanUpOFNA( &ofn );
    }
    else {
	// Use Unicode or ANSI strings and API
	OPENFILENAME* ofn = makeOFN( parent, initialSelection,
				     *initialDirectory, "Open",
				     filter, ExistingFile );
	if ( GetOpenFileName( ofn ) )
	    result = qt_winQString( ofn->lpstrFile );
	cleanUpOFN( &ofn );
    }

    if ( result.isEmpty() ) {
	return result;
    }
    else {
	QFileInfo fi( result );
	*initialDirectory = fi.dirPath();
	return fi.absFilePath();
    }
}


QString QFileDialog::winGetSaveFileName( const QString &initialSelection,
					 const QString &filter,
					 QString* initialDirectory,
					 QWidget *parent, const char* /*name*/ )
{
    QString result;

    if ( qt_winver != WV_NT ) {
	// Use ANSI strings and API
	OPENFILENAMEA* ofn = makeOFNA( parent, initialSelection,
				       *initialDirectory, "Save As",
				       filter, AnyFile );
	if ( GetSaveFileNameA( ofn ) )
	    result = QString::fromLocal8Bit( ofn->lpstrFile );
	cleanUpOFNA( &ofn );
    }
    else {
	// Use Unicode or ANSI strings and API
	OPENFILENAME* ofn = makeOFN( parent, initialSelection,
				     *initialDirectory, "Save As",
				     filter, AnyFile );
	if ( GetSaveFileName( ofn ) )
	    result = qt_winQString( ofn->lpstrFile );
	cleanUpOFN( &ofn );
    }

    if ( result.isEmpty() ) {
	return result;
    }
    else {
	QFileInfo fi( result );
	*initialDirectory = fi.dirPath();
	return fi.absFilePath();
    }
}



QStringList QFileDialog::winGetOpenFileNames( const QString &filter,
					      QString* initialDirectory,
					      QWidget *parent,
					      const char* /*name*/ )
{
    QStringList result;
    QFileInfo fi;
    QDir dir;

    if ( qt_winver != WV_NT ) {
	// Use ANSI strings and API
	OPENFILENAMEA* ofn = makeOFNA( parent, QString::null,
				       *initialDirectory, "Open",
				       filter, ExistingFiles );
	if ( GetOpenFileNameA( ofn ) ) {
	    QCString fileOrDir = ofn->lpstrFile;
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
    }
    else {
	// Use Unicode or ANSI strings and API
	OPENFILENAME* ofn = makeOFN( parent, QString::null,
				     *initialDirectory, "Open",
				     filter, ExistingFiles );
	if ( GetOpenFileName( ofn ) ) {
	    QString fileOrDir = qt_winQString( ofn->lpstrFile );
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
    *initialDirectory = fi.dirPath();
    return result;
}

