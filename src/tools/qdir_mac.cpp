/****************************************************************************
** $Id: $
**
** Implementation of QDir class
**
** Created : 950628
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Macintosh may use this file in accordance with the Qt Commercial
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

#include "qglobal.h"

#include "qdir.h"
#ifndef QT_NO_DIR

#include "qglobal.h"
#include "qdir_p.h"
#include "qfileinfo.h"
#include "qplatformdefs.h"
#include "qfiledefs_p.h"
#include "qregexp.h"
#include "qstringlist.h"
#include <stdlib.h>
#include <ctype.h>
#include "qt_mac.h"

static QString qt_cwd = "";
const unsigned char * p_str(const char * c, int len=-1); //qglobal_mac.cpp

FSSpec *QDir::make_spec(const QString &f)
{
	static FSSpec ret;
	const unsigned char *p = p_str(QFile::encodeName(QDir::convertSeparators(f)) + ":");
	if(FSMakeFSSpec(0, 0, p, &ret) != noErr)
		return NULL;
	return &ret;
}
	
void QDir::slashify( QString& n )
{
	if( n.isNull() )
		return;
	for ( int i = 0; i < (int)n.length(); i++) {
		if( n[i] == ':' )
			n[i] = '/';
	}
}

QString QDir::homeDirPath()
{
    QString d;
    d = QFile::decodeName(getenv("HOME"));
    slashify( d );
    if ( d.isNull() )
		d = rootDirPath();
    return d.isEmpty() ? QString::null : d;
}

QString QDir::canonicalPath() const
{
	return absPath();
}

bool QDir::mkdir( const QString &dirName, bool acceptAbsPath ) const
{
	FSSpec *spec = make_spec(filePath(dirName, acceptAbsPath));
	if(!spec) 
		return FALSE;
	long int d;
	if(DirCreate(spec->vRefNum, spec->parID, spec->name, &d) != noErr)
		return FALSE;
	return TRUE;
}

bool QDir::rmdir( const QString &dirName, bool acceptAbsPath ) const
{
	FSSpec *spec = make_spec(filePath(dirName, acceptAbsPath));
	if(!spec) 
		return FALSE;
	if(HDelete(spec->vRefNum, spec->parID, spec->name) != noErr)
		return FALSE;
	return TRUE;
}

bool QDir::isReadable() const
{
	if(!make_spec(filePath(dPath, TRUE)))
		return FALSE;
#ifdef Q_OS_MACX
    return ACCESS( QFile::encodeName(dPath), R_OK | X_OK ) == 0; //let macx do an additional check
#else
	return TRUE;
#endif
}

bool QDir::isRoot() const
{
    return convertSeparators(dPath).contains(':') == 1;
}

bool QDir::rename( const QString &name, const QString &newName,
		   bool acceptAbsPaths	)
{
    if ( name.isEmpty() || newName.isEmpty() ) {
#if defined(QT_CHECK_NULL)
	qWarning( "QDir::rename: Empty or null file name(s)" );
#endif
	return FALSE;
    }
    QString fn1 = filePath( name, acceptAbsPaths );
    QString fn2 = filePath( newName, acceptAbsPaths );
    return ::rename( QFile::encodeName(fn1),
		     QFile::encodeName(fn2) ) == 0;
}

bool QDir::setCurrent( const QString &path )
{
 	qt_cwd = path;
 	FSSpec *spec = make_spec(path);
 	if(!spec)
 		return FALSE;
 	if(HSetVol(0, spec->vRefNum, spec->parID) != noErr)
 		return FALSE;
 	return TRUE;
}

QString QDir::currentDirPath()
{
    if(qt_cwd.isEmpty())
        qt_cwd = drives()->getFirst()->filePath();
    const char *foo = qt_cwd.latin1();
	return qt_cwd;
}

QString QDir::rootDirPath()
{
	//FIXME
   return QString("");
}

bool QDir::isRelativePath( const QString &path )
{
	int len = path.length();
	if( len == 0)
		return TRUE;
	return path[0] != '/';
}

bool QDir::readDirEntries( const QString &nameFilter,
			   int filterSpec, int sortSpec )
{
    int i;
    if ( !fList ) {
	fList  = new QStringList;
	Q_CHECK_PTR( fList );
	fiList = new QFileInfoList;
	Q_CHECK_PTR( fiList );
	fiList->setAutoDelete( TRUE );
    } else {
	fList->clear();
	fiList->clear();
    }

    QStringList filters = qt_makeFilterList( nameFilter );

    bool doDirs	    = (filterSpec & Dirs)	!= 0;
    bool doFiles    = (filterSpec & Files)	!= 0;
    bool noSymLinks = (filterSpec & NoSymLinks) != 0;
    bool doReadable = (filterSpec & Readable)	!= 0;
    bool doWritable = (filterSpec & Writable)	!= 0;
    bool doExecable = (filterSpec & Executable) != 0;
    bool doHidden   = (filterSpec & Hidden)	!= 0;

    QFileInfo fi;
    FSIterator dir;
	FSRef ref;
	
	FSSpec *spec = make_spec(dPath);
	if( !spec )
		return FALSE;
 	if( FSpMakeFSRef(spec, &ref) != noErr)
 		return FALSE;	
	if( FSOpenIterator( &ref, kFSIterateFlat, &dir ) != noErr)
		return FALSE;

	char hacky[512];
	FSRefMakePath(&ref, (UInt8 *)hacky, 512);

	ItemCount specn = 512;
	FSSpec *specs = (FSSpec *)calloc(specn, sizeof(FSSpec));
	FSGetCatalogInfoBulk( dir, specn, &specn, NULL, kFSCatInfoNone, NULL, NULL, specs, NULL );
	Str255 tmp_n;
 for(ItemCount specx = 0; specx < specn; specx++) {
		memcpy(tmp_n, &(specs[specx].name[1]), specs[specx].name[0]);
		tmp_n[specs[specx].name[0]] = '\0';
		QString fn = QFile::decodeName((char *)tmp_n);
		fi.setFile( *this, fn );
		if ( !match( filters, fn ) && !(allDirs && fi.isDir()) )
	     	continue;
		if  ( (doDirs && fi.isDir()) || (doFiles && fi.isFile()) ) {
	   		if ( noSymLinks && fi.isSymLink() )
	        	continue;
	    	if ( (filterSpec & RWEMask) != 0 )
	        	if ( (doReadable && !fi.isReadable()) ||
	             	 (doWritable && !fi.isWritable()) ||
	             	(doExecable && !fi.isExecutable()) )
	            	continue;
	    		if ( !doHidden && fn[0] == '.' &&
	         		fn != QString::fromLatin1(".")
	         		&& fn != QString::fromLatin1("..") )
	        		continue;
	    		fiList->append( new QFileInfo( fi ) );
		}
    }
    FSCloseIterator(dir);
    free(specs);
    
    // Sort...
    if(fiList->count()) {
		QDirSortItem* si= new QDirSortItem[fiList->count()];
		QFileInfo* itm;
		i=0;
		for (itm = fiList->first(); itm; itm = fiList->next())
	    	si[i++].item = itm;
		qt_cmp_si_sortSpec = sortSpec;
		qsort( si, i, sizeof(si[0]), qt_cmp_si );
		// put them back in the list
		fiList->setAutoDelete( FALSE );
		fiList->clear();
		int j;
		for ( j=0; j<i; j++ ) {
	    	fiList->append( si[j].item );
	    	fList->append( si[j].item->fileName() );
		}
		delete [] si;
		fiList->setAutoDelete( TRUE );
    }

    if ( filterSpec == (FilterSpec)filtS && sortSpec == (SortSpec)sortS &&
	 		nameFilter == nameFilt )
			dirty = FALSE;
    else
			dirty = TRUE;
    return TRUE;
}

const QFileInfoList * QDir::drives()
{
    // FIXME
    static QFileInfoList * knownMemoryLeak = 0;

    if ( !knownMemoryLeak ) {
    		//FIXME
		knownMemoryLeak = new QFileInfoList;
		knownMemoryLeak->append( new QFileInfo( ":MacOS 9:" ) );
    }

    return knownMemoryLeak;
}
#endif //QT_NO_DIR
