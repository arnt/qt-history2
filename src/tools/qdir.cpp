/****************************************************************************
** $Id: //depot/qt/main/src/tools/qdir.cpp#83 $
**
** Implementation of QDir class
**
** Created : 950427
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

#include "qdir.h"
#include "qfileinfo.h"
#include "qfiledefs.h"
#include "qregexp.h"
#include "qstringlist.h"
#include <stdlib.h>
#include <ctype.h>
#if defined(_OS_WIN32_)
#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE
#endif
#endif
#if defined(_CC_BOOL_DEF_)
#undef	bool
#include <windows.h>
#define bool int
#else
#include <windows.h>
#endif
#include <direct.h>
#include <tchar.h>
#endif
#if defined(_OS_OS2EMX_)
extern Q_UINT32 DosQueryCurrentDisk(Q_UINT32*,Q_UINT32*);
#define NO_ERROR 0
#endif

#if defined(_OS_FATFS_) || defined(_OS_OS2EMX_)


static void slashify( QString& n )
{
    if ( n.isNull() )
	return;
    for ( int i=0; i<(int)n.length(); i++ ) {
	if ( n[i] ==  '\\' )
	    n[i] = '/';
    }
}

#elif defined(UNIX)

static void slashify( QString& )
{
    return;
}

#endif




/*!
  \class QDir qdir.h
  \brief Traverses directory structures and contents in a
	    platform-independent way.

  \ingroup io

  A QDir can point to a file using either a relative or an absolute file
  path. Absolute file paths begin with the directory separator ('/') or a
  drive specification (not applicable to UNIX).	 Relative file names begin
  with a directory name or a file name and specify a path relative to the
  current directory.

  An example of an absolute path is the string "/tmp/quartz", a relative
  path might look like "src/fatlib". You can use the function isRelative()
  to check if a QDir is using a relative or an absolute file path. You can
  call the function convertToAbs() to convert a relative QDir to an
  absolute one.

  The directory "example" under the current directory is checked for existence
  in the example below:

  \code
    QDir d( "example" );			// "./example"
    if ( !d.exists() )
	qWarning( "Cannot find the example directory" );
  \endcode

  If you always use '/' as a directory separator, Qt will translate your
  paths to conform to the underlying operating system.

  cd() and cdUp() can be used to navigate the directory tree. Note that the
  logical cd and cdUp operations are not performed if the new directory does
  not exist.

  Example:
  \code
    QDir d = QDir::root();			// "/"
    if ( !d.cd("tmp") ) {			// "/tmp"
	qWarning( "Cannot find the \"/tmp\" directory" );
    } else {
	QFile f( d.filePath("ex1.txt") );	// "/tmp/ex1.txt"
	if ( !f.open(IO_ReadWrite) )
	    qWarning( "Cannot create the file %s", f.name() );
    }
  \endcode

  To read the contents of a directory you can use the entryList() and
  entryInfoList() functions.

  Example:
  \code
    #include <stdio.h>
    #include <qdir.h>

    //
    // This program scans the current directory and lists all files
    // that are not symbolic links, sorted by size with the smallest files
    // first.
    //

    int main( int argc, char **argv )
    {
	QDir d;
	d.setFilter( QDir::Files | QDir::Hidden | QDir::NoSymLinks );
	d.setSorting( QDir::Size | QDir::Reversed );

	const QFileInfoList *list = d.entryInfoList();
	QFileInfoListIterator it( *list );	// create list iterator
	QFileInfo *fi;				// pointer for traversing

	printf( "     BYTES FILENAME\n" );	// print header
	while ( (fi=it.current()) ) {		// for each file...
	    printf( "%10li %s\n", fi->size(), fi->fileName().data() );
	    ++it;				// goto next list element
	}
    }
  \endcode
*/


/*!
  Constructs a QDir pointing to the current directory.
  \sa currentDirPath()
*/

QDir::QDir()
{
    dPath = QString::fromLatin1(".");
    init();
}

/*!
  Constructs a QDir.

  \arg \e path is the directory.
  \arg \e nameFilter is the file name filter.
  \arg \e sortSpec is the sort specification, which describes how to
  sort the files in the directory.
  \arg \e filterSpec is the filter specification, which describes how
  to filter the files in the directory.

  Most of these arguments (except \e path) have optional values.

  Example:
  \code
    // lists all files in /tmp

    QDir d( "/tmp" );
    for ( int i=0; i<d.count(); i++ )
	printf( "%s\n", d[i] );
  \endcode

  If \e path is "" or null, the directory is set to "." (the current
  directory).  If \e nameFilter is "" or null, it is set to "*" (all
  files).

  No check is made to ensure that the directory exists.

  \sa exists(), setPath(), setNameFilter(), setFilter(), setSorting()
*/

QDir::QDir( const QString &path, const QString &nameFilter,
	    int sortSpec, int filterSpec )
{
    init();
    dPath = cleanDirPath( path );
    if ( dPath.isEmpty() )
	dPath = QString::fromLatin1(".");
    nameFilt = nameFilter;
    if ( nameFilt.isEmpty() )
	nameFilt = QString::fromLatin1("*");
    filtS = (FilterSpec)filterSpec;
    sortS = (SortSpec)sortSpec;
}

/*!
  Constructs a QDir that is a copy of the given directory.
  \sa operator=()
*/

QDir::QDir( const QDir &d )
{
    dPath    = d.dPath;
    fList    = 0;
    fiList   = 0;
    nameFilt = d.nameFilt;
    dirty    = TRUE;
    allDirs  = d.allDirs;
    filtS    = d.filtS;
    sortS    = d.sortS;
}

void QDir::init()
{
    fList     = 0;
    fiList    = 0;
    nameFilt = QString::fromLatin1("*");
    dirty    = TRUE;
    allDirs  = FALSE;
    filtS    = All;
    sortS    = SortSpec(Name | IgnoreCase);
}

/*!
  Destroys the QDir and cleans up.
*/

QDir::~QDir()
{
    if ( fList )
       delete fList;
    if ( fiList )
       delete fiList;
}


/*!
  Sets the path of the directory. The path is cleaned of redundant ".", ".."
  and multiple separators. No check is made to ensure that a directory
  with this path exists.

  The path can be either absolute or relative. Absolute paths begin with the
  directory separator ('/') or a drive specification (not
  applicable to UNIX).
  Relative file names begin with a directory name or a file name and specify
  a path relative to the current directory. An example of
  an absolute path is the string "/tmp/quartz", a relative path might look like
  "src/fatlib". You can use the function isRelative() to check if a QDir
  is using a relative or an absolute file path. You can call the function
  convertToAbs() to convert a relative QDir to an absolute one.

  \sa path(), absPath(), exists, cleanDirPath(), dirName(),
      absFilePath(), isRelative(), convertToAbs()
*/

void QDir::setPath( const QString &path )
{
    dPath = cleanDirPath( path );
    if ( dPath.isEmpty() )
	dPath = QString::fromLatin1(".");
    dirty = TRUE;
}

/*!
  \fn  QString QDir::path() const
  Returns the path, this may contain symbolic links, but never contains
  redundant ".", ".." or multiple separators.

  The returned path can be either absolute or relative (see setPath()).

  \sa setPath(), absPath(), exists(), cleanDirPath(), dirName(),
  absFilePath(), convertSeparators()
*/

/*!
  Returns the absolute (a path that starts with '/') path, which may
  contain symbolic links, but never contains redundant ".", ".." or
  multiple separators.

  \sa setPath(), canonicalPath(), exists(),  cleanDirPath(), dirName(),
  absFilePath()
*/

QString QDir::absPath() const
{
    if ( QDir::isRelativePath(dPath) ) {
	QString tmp = currentDirPath();
	if ( tmp.right(1) != QString::fromLatin1("/") )
	    tmp += '/';
	tmp += dPath;
	return cleanDirPath( tmp );
    } else {
	return cleanDirPath( dPath );
    }
}

/*!
  Returns the canonical path, i.e. a path without symbolic links.

  On systems that do not have symbolic links this function will
  always return the same string that absPath returns.
  If the canonical path does not exist a null string is returned.

  \sa path(), absPath(), exists(), cleanDirPath(), dirName(),
      absFilePath(), QString::isNull()
*/

QString QDir::canonicalPath() const
{
    QString r;

#ifdef _WS_WIN_
    char cur[PATH_MAX];
    GETCWD( cur, PATH_MAX );
    if ( qt_winunicode ) {
	if ( _tchdir((TCHAR*)qt_winTchar(dPath,TRUE)) >= 0 ) {
	    TCHAR tmp[PATH_MAX];
	    if ( _tgetcwd( tmp, PATH_MAX ) )
		r = qt_winQString(tmp);
	}
    } else {
	if ( _chdir(qt_winQString2MB(convertSeparators(dPath))) >= 0 ) {
	    char tmp[PATH_MAX];
	    if ( _getcwd( tmp, PATH_MAX ) )
		r = tmp;
	}
    }
    CHDIR( cur );
    slashify( r );
#else
    char cur[PATH_MAX];
    char tmp[PATH_MAX];

    GETCWD( cur, PATH_MAX );
    if ( CHDIR(QFile::encodeName(dPath)) >= 0 ) {
	GETCWD( tmp, PATH_MAX );
	r = QFile::decodeName(tmp);
    }
    CHDIR( cur );

#endif

    return r;
}

/*!
  Returns the name of the directory, this is NOT the same as the path, e.g.
  a directory with the name "mail", might have the path "/var/spool/mail".
  If the directory has no name (e.g. the root directory) a null string is
  returned.

  No check is made to ensure that a directory with this name actually exists.

  \sa path(), absPath(), absFilePath(), exists(), QString::isNull()
*/

QString QDir::dirName() const
{
    int pos = dPath.findRev( '/' );
    if ( pos == -1  )
	return dPath;
    return dPath.right( dPath.length() - pos - 1 );
}

/*!
  Returns the path name of a file in the directory. Does NOT check if
  the file actually exists in the directory. If the QDir is relative
  the returned path name will also be relative. Redundant multiple separators
  or "." and ".." directories in \e fileName will not be removed (see
  cleanDirPath()).

  If \e acceptAbsPath is TRUE a \e fileName starting with a separator
  ('/') will be returned without change.
  If \e acceptAbsPath is FALSE an absolute path will be appended to
  the directory path.

  \sa absFilePath(), isRelative(), canonicalPath()
*/

QString QDir::filePath( const QString &fileName,
			bool acceptAbsPath ) const
{
    if ( acceptAbsPath && !isRelativePath(fileName) )
	return QString(fileName);

    QString tmp = dPath.copy();
    if ( tmp.isEmpty() || (tmp[(int)tmp.length()-1] != '/' && !!fileName &&
			   fileName[0] != '/') )
	tmp += '/';
    tmp += fileName;
    return tmp;
}

/*!
  Returns the absolute path name of a file in the directory. Does NOT check if
  the file actually exists in the directory. Redundant multiple separators
  or "." and ".." directories in \e fileName will NOT be removed (see
  cleanDirPath()).

  If \e acceptAbsPath is TRUE a \e fileName starting with a separator
  ('/') will be returned without change.
  if \e acceptAbsPath is FALSE an absolute path will be appended to
  the directory path.

  \sa filePath()
*/

QString QDir::absFilePath( const QString &fileName,
			   bool acceptAbsPath ) const
{
    if ( acceptAbsPath && !isRelativePath( fileName ) )
	return fileName;

    QString tmp = absPath();
    if ( tmp.isEmpty() || (tmp[(int)tmp.length()-1] != '/' && !!fileName &&
			   fileName[0] != '/') )
	tmp += '/';
    tmp += fileName;
    return tmp;
}


/*!
  Converts the '/' separators in \a pathName to system native
  separators.  Returns the translated string.

  On Windows, convertSeparators("c:/winnt/system32") returns
  "c:\winnt\system32".

  No conversion is done on UNIX.
*/

QString QDir::convertSeparators( const QString &pathName )
{
    QString n( pathName );
#if defined(_OS_FATFS_) || defined(_OS_OS2EMX_)
    for ( int i=0; i<(int)n.length(); i++ ) {
	if ( n[i] == '/' )
	    n[i] = '\\';
    }
#endif
    return n;
}


/*!
  Changes directory by descending into the given directory. Returns
  TRUE if the new directory exists and is readable. Note that the logical
  cd operation is NOT performed if the new directory does not exist.

  If \e acceptAbsPath is TRUE a path starting with a separator ('/')
  will cd to the absolute directory, if \e acceptAbsPath is FALSE
  any number of separators at the beginning of \e dirName will be removed.

  Example:
  \code
  QDir d = QDir::home();  // now points to home directory
  if ( !d.cd("c++") ) {	  // now points to "c++" under home directory if OK
      QFileInfo fi( d, "c++" );
      if ( fi.exists() ) {
	  if ( fi.isDir() )
	      qWarning( "Cannot cd into \"%s\".", (char*)d.absFilePath("c++") );
	  else
	      qWarning( "Cannot create directory \"%s\"\n"
		       "A file named \"c++\" already exists in \"%s\"",
		       (const char *)d.absFilePath("c++"),
		       (const char *)d.path() );
	  return;
      } else {
	  qWarning( "Creating directory \"%s\"",
		   (const char *) d.absFilePath("c++") );
	  if ( !d.mkdir( "c++" ) ) {
	      qWarning("Could not create directory \"%s\"",
		      (const char *)d.absFilePath("c++") );
	      return;
	  }
      }
  }
  \endcode

  Calling cd( ".." ) is equivalent to calling cdUp().

  \sa cdUp(), isReadable(), exists(), path()
*/

bool QDir::cd( const QString &dirName, bool acceptAbsPath )
{
    if ( dirName.isEmpty() || dirName==QString::fromLatin1(".") )
	return TRUE;
    QString old = dPath;
    if ( acceptAbsPath && !isRelativePath(dirName) ) {
	dPath = cleanDirPath( dirName );
    } else {
	if ( !isRoot() )
	    dPath += '/';
	dPath += dirName;
	if ( dirName.find('/') >= 0
		|| old == QString::fromLatin1(".")
		|| dirName == QString::fromLatin1("..") )
	    dPath = cleanDirPath( dPath );
    }
    if ( !exists() ) {
	dPath = old;			// regret
	return FALSE;
    }
    dirty = TRUE;
    return TRUE;
}

/*!
  Changes directory by moving one directory up the path followed to arrive
  at the current directory.

  Returns TRUE if the new directory exists and is readable. Note that the
  logical cdUp() operation is not performed if the new directory does not
  exist.

  \sa cd(), isReadable(), exists(), path()
*/

bool QDir::cdUp()
{
    return cd( QString::fromLatin1("..") );
}

/*!
  \fn QString QDir::nameFilter() const
  Returns the string set by setNameFilter()
  \sa setNameFilter()
*/

/*!
  Sets the name filter used by entryList() and entryInfoList().

  The name filter is a wildcarding filter that understands "*" and "?"
  wildcards, if you want entryList() and entryInfoList() to list all files
  ending with ".cpp", you simply call dir.setNameFilter("*.cpp");

  \sa nameFilter()
*/

void QDir::setNameFilter( const QString &nameFilter )
{
    nameFilt = nameFilter;
    if ( nameFilt.isEmpty() )
	nameFilt = QString::fromLatin1("*");
    dirty = TRUE;
}

/*!
  \fn QDir::FilterSpec QDir::filter() const
  Returns the value set by setFilter()
  \sa setFilter()
*/

/*!
  Sets the filter used by entryList() and entryInfoList(). The filter is used
  to specify the kind of files that should be returned by entryList() and
  entryInfoList(). The filter is specified by or-ing values from the enum
  FilterSpec. The different values are:


  <dl compact>
  <dt>Dirs<dd> List directories only.
  <dt>Files<dd> List files only.
  <dt>Drives<dd> List drives.
  <dt>NoSymLinks<dd> Do not list symbolic links.

  <dt>Readable<dd> List files with read permission.
  <dt>Writable<dd> List files with write permission.
  <dt>Executable<dd> List files with execute permission.

  Setting none of the three flags above is equivalent to setting all of them.

  <dt>Modified<dd> Only list files that have been modified (does nothing
			  under UNIX).
  <dt>Hidden<dd> List hidden files also (.* under UNIX).
  <dt>System<dd> List system files (does nothing under UNIX).

  </dl>

  \sa nameFilter()
*/

void QDir::setFilter( int filterSpec )
{
    if ( filtS == (FilterSpec) filterSpec )
	return;
    filtS = (FilterSpec) filterSpec;
    dirty = TRUE;
}

/*!
  \fn QDir::SortSpec QDir::sorting() const

  Returns the value set by setSorting()

  \sa setSorting()
*/

/*!
  Sets the sorting order used by entryList() and entryInfoList().

  The \e sortSpec is specified by or-ing values from the enum
  SortSpec. The different values are:

  One of these:
  <dl compact>
  <dt>Name<dd> Sort by name (alphabetical order).
  <dt>Time<dd> Sort by time (most recent first).
  <dt>Size<dd> Sort by size (largest first).
  <dt>Unsorted<dd> Use the operating system order (UNIX does NOT sort
  alphabetically).

  ORed with zero or more of these:

  <dt>DirsFirst<dd> Always put directory names first.
  <dt>Reversed<dd> Reverse sort order.
  <dt>IgnoreCase<dd> Ignore case when sorting by name.
  </dl>
*/

void QDir::setSorting( int sortSpec )
{
    if ( sortS == (SortSpec) sortSpec )
	return;
    sortS = (SortSpec) sortSpec;
    dirty = TRUE;
}

/*!
  \fn bool QDir::matchAllDirs() const
  Returns the value set by setMatchAllDirs()

  \sa setMatchAllDirs()
*/

/*!
  If \e enable is TRUE, all directories will be listed (even if they do not
  match the filter or the name filter), otherwise only matched directories
  will be listed.

  \bug Currently, directories that do not match the filter will not be
  included (the name filter will be ignored as expected).

  \sa matchAllDirs()
*/

void QDir::setMatchAllDirs( bool enable )
{
    if ( (bool)allDirs == enable )
	return;
    allDirs = enable;
    dirty = TRUE;
}


/*!
  Returns the number of files that was found.
  Equivalent to entryList().count().
  \sa operator[], entryList()
*/

uint QDir::count() const
{
    return entryList().count();
}

/*!
  Returns the file name at position \e index in the list of found file
  names.
  Equivalent to entryList().at(index).

  Returns null if the \e index is out of range or if the entryList()
  function failed.

  \sa count(), entryList()
*/

QString QDir::operator[]( int index ) const
{
    entryList();
    return fList && index >= 0 && index < (int)fList->count() ?
	(*fList)[index] : QString::null;
}


/*!
  This function is included to easy porting from Qt 1.x to Qt 2.0,
  it is the same as entryList(), but encodes the filenames as 8-bit
  strings using QFile::encodedName().

  It is more efficient to use entryList().
*/
QStrList QDir::encodedEntryList( int filterSpec, int sortSpec ) const
{
    QStrList r;
    QStringList l = entryList(filterSpec,sortSpec);
    for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it ) {
	r.append( QFile::encodeName(*it) );
    }
    return r;
}

/*!
  This function is included to easy porting from Qt 1.x to Qt 2.0,
  it is the same as entryList(), but encodes the filenames as 8-bit
  strings using QFile::encodedName().

  It is more efficient to use entryList().
*/
QStrList QDir::encodedEntryList( const QString &nameFilter,
			   int filterSpec,
			   int sortSpec ) const
{
    QStrList r;
    QStringList l = entryList(nameFilter,filterSpec,sortSpec);
    for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it ) {
	r.append( QFile::encodeName(*it) );
    }
    return r;
}



/*!
  Returns a list of the names of all files and directories in the directory
  indicated by the setSorting(), setFilter() and setNameFilter()
  specifications.

  The the filter and sorting specifications can be overridden using the
  \e filterSpec and \e sortSpec arguments.

  Returns an empty list if the directory is unreadable or does not exist.

  \sa entryInfoList(), setNameFilter(), setSorting(), setFilter(),
	encodedEntryList()
*/

QStringList QDir::entryList( int filterSpec, int sortSpec ) const
{
    if ( !dirty && filterSpec == (int)DefaultFilter &&
		   sortSpec   == (int)DefaultSort )
	return *fList;
    return entryList( nameFilt, filterSpec, sortSpec );
}

/*!
  Returns a list of the names of all files and directories in the directory
  indicated by the setSorting(), setFilter() and setNameFilter()
  specifications.

  The the filter and sorting specifications can be overridden using the
  \e nameFilter, \e filterSpec and \e sortSpec arguments.

  Returns and empty list if the directory is unreadable or does not exist.

  \sa entryInfoList(), setNameFilter(), setSorting(), setFilter(),
	encodedEntryList()
*/

QStringList QDir::entryList( const QString &nameFilter,
				 int filterSpec, int sortSpec ) const
{
    if ( filterSpec == (int)DefaultFilter )
	filterSpec = filtS;
    if ( sortSpec == (int)DefaultSort )
	sortSpec = sortS;
    QDir *that = (QDir*)this;			// mutable function
    if ( that->readDirEntries(nameFilter, filterSpec, sortSpec) )
	return *that->fList;
    else
	return QStringList();
}

/*!
  Returns a list of QFileInfo objects for all files and directories in
  the directory pointed to using the setSorting(), setFilter() and
  setNameFilter() specifications.

  The the filter and sorting specifications can be overridden using the
  \e filterSpec and \e sortSpec arguments.

  Returns 0 if the directory is unreadable or does not exist.

  The returned pointer is a const pointer to a QFileInfoList. The list is
  owned by the QDir object and will be reused on the next call to
  entryInfoList() for the same QDir instance. If you want to keep the
  entries of the list after a subsequent call to this function you will
  need to copy them.

  \sa entryList(), setNameFilter(), setSorting(), setFilter()
*/

const QFileInfoList *QDir::entryInfoList( int filterSpec, int sortSpec ) const
{
    if ( !dirty && filterSpec == (int)DefaultFilter &&
		   sortSpec   == (int)DefaultSort )
	return fiList;
    return entryInfoList( nameFilt, filterSpec, sortSpec );
}

/*!
  Returns a list of QFileInfo objects for all files and directories in
  the directory pointed to using the setSorting(), setFilter() and
  setNameFilter() specifications.

  The the filter and sorting specifications can be overridden using the
  \e nameFilter, \e filterSpec and \e sortSpec arguments.

  Returns 0 if the directory is unreadable or does not exist.

  The returned pointer is a const pointer to a QFileInfoList. The list is
  owned by the QDir object and will be reused on the next call to
  entryInfoList() for the same QDir instance. If you want to keep the
  entries of the list after a subsequent call to this function you will
  need to copy them.

  \sa entryList(), setNameFilter(), setSorting(), setFilter()
*/

const QFileInfoList *QDir::entryInfoList( const QString &nameFilter,
					  int filterSpec, int sortSpec ) const
{
    if ( filterSpec == (int)DefaultFilter )
	filterSpec = filtS;
    if ( sortSpec == (int)DefaultSort )
	sortSpec = sortS;
    QDir *that = (QDir*)this;			// mutable function
    if ( that->readDirEntries(nameFilter, filterSpec, sortSpec) )
	return that->fiList;
    else
	return 0;
}

/*!
  Creates a directory.

  If \e acceptAbsPath is TRUE a path starting with a separator ('/')
  will create the absolute directory, if \e acceptAbsPath is FALSE
  any number of separators at the beginning of \e dirName will be removed.

  Returns TRUE if successful, otherwise FALSE.

  \sa rmdir()
*/

bool QDir::mkdir( const QString &dirName, bool acceptAbsPath ) const
{
#if defined (UNIX) || defined(__CYGWIN32__)
    return MKDIR( QFile::encodeName(filePath(dirName,acceptAbsPath)), 0777 ) == 0;
#else
    if ( qt_winunicode ) {
	return _tmkdir((const TCHAR*)qt_winTchar(filePath(dirName,acceptAbsPath),TRUE)) == 0;
    } else {
	return _mkdir(qt_winQString2MB(filePath(dirName,acceptAbsPath))) == 0;
    }
#endif
}

/*!
  Removes a directory.

  If \e acceptAbsPath is TRUE a path starting with a separator ('/')
  will remove the absolute directory, if \e acceptAbsPath is FALSE
  any number of separators at the beginning of \e dirName will be removed.

  The directory must be empty for rmdir() to succeed.

  Returns TRUE if successful, otherwise FALSE.

  \sa mkdir()
*/

bool QDir::rmdir( const QString &dirName, bool acceptAbsPath ) const
{
#if defined (UNIX) || defined(__CYGWIN32__)
    return RMDIR( QFile::encodeName(filePath(dirName,acceptAbsPath)) ) == 0;
#else
    if ( qt_winunicode ) {
	return _trmdir((const TCHAR*)qt_winTchar(filePath(dirName,acceptAbsPath),TRUE)) == 0;
    } else {
	return _rmdir(qt_winQString2MB(filePath(dirName,acceptAbsPath))) == 0;
    }
#endif
}

/*!
  Returns TRUE if the directory is readable AND we can open files by
  name. This function will return FALSE if only one of these is present.
  \warning A FALSE value from this function is not a guarantee that files
  in the directory are not accessible.

  \sa QFileInfo::isReadable()
*/

bool QDir::isReadable() const
{
#if defined (UNIX) || defined(__CYGWIN32__)
    return ACCESS( QFile::encodeName(dPath), R_OK | X_OK ) == 0;
#else
    if ( qt_winunicode ) {
	return _taccess((const TCHAR*)qt_winTchar(dPath,TRUE), R_OK) == 0;
    } else {
	return _access(qt_winQString2MB(convertSeparators(dPath)), R_OK) == 0;
    }
#endif
}

/*!
  Returns TRUE if the directory exists. (If a file with the same
  name is found this function will of course return FALSE).

  \sa QFileInfo::exists(), QFile::exists()
*/

bool QDir::exists() const
{
    QFileInfo fi( dPath );
    return fi.exists() && fi.isDir();
}

/*!
  Returns TRUE if the directory is the root directory, otherwise FALSE.

  Note: If the directory is a symbolic link to the root directory this
  function returns FALSE. If you want to test for this you can use
  canonicalPath():

  Example:
  \code
    QDir d( "/tmp/root_link" );
    d = d.canonicalPath();
    if ( d.isRoot() )
	qWarning( "It IS a root link!" );
  \endcode

  \sa root(), rootDirPath()
*/

bool QDir::isRoot() const
{
#if defined(_OS_FATFS_) || defined(_OS_OS2EMX_)
    return dPath == "/" || dPath == "//" ||
	(isalpha(dPath[0]) && dPath.mid(1,dPath.length()) == ":/");
#else
    return dPath == QString::fromLatin1("/");
#endif
}

/*!
  Returns TRUE if the directory path is relative to the current directory,
  FALSE if the path is absolute (e.g. under UNIX a path is relative if it
  does not start with a '/').

  According to Einstein this function should always return TRUE.

  \sa convertToAbs()
*/

bool QDir::isRelative() const
{
    return isRelativePath( dPath );
}

/*!
  Converts the directory path to an absolute path. If it is already
  absolute nothing is done.

  \sa isRelative()
*/

void QDir::convertToAbs()
{
    dPath = absPath();
}

/*!
  Makes a copy of d and assigns it to this QDir.
*/

QDir &QDir::operator=( const QDir &d )
{
    dPath    = d.dPath;
    fList    = 0;
    fiList   = 0;
    nameFilt = d.nameFilt;
    dirty    = TRUE;
    allDirs  = d.allDirs;
    filtS    = d.filtS;
    sortS    = d.sortS;
    return *this;
}

/*!
  Sets the directory path to be the given path.
*/

QDir &QDir::operator=( const QString &path )
{
    dPath = cleanDirPath( path );
    dirty = TRUE;
    return *this;
}


/*!
  \fn bool QDir::operator!=( const QDir &d ) const
  Returns TRUE if the \e d and this dir have different path or
  different sort/filter settings, otherwise FALSE.
*/

/*!
  Returns TRUE if the \e d and this dir have the same path and all sort
  and filter settings are equal, otherwise FALSE.
*/

bool QDir::operator==( const QDir &d ) const
{
    return dPath    == d.dPath &&
	   nameFilt == d.nameFilt &&
	   allDirs  == d.allDirs &&
	   filtS    == d.filtS &&
	   sortS    == d.sortS;
}


/*!
  Removes a file.

  If \e acceptAbsPath is TRUE a path starting with a separator ('/')
  will remove the file with the absolute path, if \e acceptAbsPath is FALSE
  any number of separators at the beginning of \e fileName will be removed.

  Returns TRUE if successful, otherwise FALSE.
*/

bool QDir::remove( const QString &fileName, bool acceptAbsPath )
{
    if ( fileName.isEmpty() ) {
#if defined(CHECK_NULL)
	qWarning( "QDir::remove: Empty or null file name" );
#endif
	return FALSE;
    }
    QString p = filePath( fileName, acceptAbsPath );
    return QFile::remove( p );
}

/*!
  Renames a file.

  If \e acceptAbsPaths is TRUE a path starting with a separator ('/')
  will rename the file with the absolute path, if \e acceptAbsPath is FALSE
  any number of separators at the beginning of \e name will be removed.

  Returns TRUE if successful, otherwise FALSE.

  On most file systems, rename() fails only if oldName does not exist
  or if \a newName and \a oldName are not on the same partition, but
  there are also other reasons why rename() can fail.  For example, on
  at least one file system rename() fails if newName points to an open
  file
*/

bool QDir::rename( const QString &name, const QString &newName,
		   bool acceptAbsPaths	)
{
    if ( name.isEmpty() || newName.isEmpty() ) {
#if defined(CHECK_NULL)
	qWarning( "QDir::rename: Empty or null file name(s)" );
#endif
	return FALSE;
    }
    QString fn1 = filePath( name, acceptAbsPaths );
    QString fn2 = filePath( newName, acceptAbsPaths );
#if defined (UNIX) || defined(__CYGWIN32__)
    return ::rename( QFile::encodeName(fn1),
		     QFile::encodeName(fn2) ) == 0;
#else
    if ( qt_winunicode ) {
	TCHAR* t2 = (TCHAR*)qt_winTchar_new(fn1);
	bool r = _trename((const TCHAR*)qt_winTchar(fn1,TRUE), t2) == 0;
	delete [] t2;
	return r;
    } else {
	return rename(qt_winQString2MB(fn1), qt_winQString2MB(fn2)) == 0;
    }
#endif
}

/*!
  Checks for existence of a file.

  If \e acceptAbsPaths is TRUE a path starting with a separator ('/')
  will check the file with the absolute path, if \e acceptAbsPath is FALSE
  any number of separators at the beginning of \e name will be removed.

  Returns TRUE if the file exists, otherwise FALSE.

  \sa QFileInfo::exists(), QFile::exists()
*/

bool QDir::exists( const QString &name, bool acceptAbsPath )
{
    if ( name.isEmpty() ) {
#if defined(CHECK_NULL)
	qWarning( "QDir::exists: Empty or null file name" );
#endif
	return FALSE;
    }
    QString tmp = filePath( name, acceptAbsPath );
    return QFile::exists( tmp );
}

/*!
  Returns the native directory separator; '/' under UNIX and '\' under
  MS-DOS, Windows NT and OS/2.

  You do not need to use this function to build file paths. If you always
  use '/', Qt will translate your paths to conform to the underlying
  operating system.
*/

char QDir::separator()
{
#if defined( UNIX )
    return '/';
#elif defined (_OS_FATFS_)
    return '\\';
#elif defined (_OS_MAC_)
    return ':';
#else
    return '/';
#endif
}


/*!
  Sets the the current directory. Returns TRUE if successful.
*/

bool QDir::setCurrent( const QString &path )
{
    int r;

#ifdef _WS_WIN_
    if ( qt_winunicode ) {
	r = _tchdir((const TCHAR*)qt_winTchar(path,TRUE));
    } else {
	r = _chdir(qt_winQString2MB(path));
    }
#else
    r = CHDIR( QFile::encodeName(path) );
#endif

    return r >= 0;
}

/*!
  Returns the current directory.
  \sa currentDirPath(), QDir::QDir()
*/

QDir QDir::current()
{
    return QDir( currentDirPath() );
}

/*!
  Returns the home directory.
  \sa homeDirPath()
*/

QDir QDir::home()
{
    return QDir( homeDirPath() );
}

/*!
  Returns the root directory.
  \sa rootDirPath() drives()
*/

QDir QDir::root()
{
    return QDir( rootDirPath() );
}


/*!
  Returns the absolute path of the current directory.
  \sa current()
*/

QString QDir::currentDirPath()
{
    static bool forcecwd = TRUE;
    static ino_t cINode;
    static dev_t cDevice;
    QString result;

    STATBUF st;

    if ( STAT( ".", &st ) == 0 ) {
	if ( forcecwd || cINode != st.st_ino || cDevice != st.st_dev ) {
#ifdef _WS_WIN_
	    if ( qt_winunicode ) {
		TCHAR currentName[PATH_MAX];
		if ( _tgetcwd(currentName,PATH_MAX) >= 0 ) {
		    result = qt_winQString(currentName);
		}
	    } else {
		char currentName[PATH_MAX];
		if ( _getcwd(currentName,PATH_MAX) >= 0 ) {
		    result = QString::fromLatin1(currentName);
		}
	    }
#else
	    {
		char currentName[PATH_MAX];
		if ( GETCWD( currentName, PATH_MAX ) != 0 ) {
		    result = QFile::decodeName(currentName);
		}
	    }
#endif
	    if ( !result.isNull() ) {
		cINode	 = st.st_ino;
		cDevice	 = st.st_dev;
		slashify( result );
		// forcecwd = FALSE;   ### caching removed, not safe
	    } else {
		qWarning( "QDir::currentDirPath: getcwd() failed" );
		forcecwd    = TRUE;
	    }
	}
    } else {
#if defined(DEBUG)
	qWarning( "QDir::currentDirPath: stat(\".\") failed" );
#endif
	forcecwd    = TRUE;
    }
    return result;
}

/*!
  Returns the absolute path for the user's home directory,
  \sa home()
*/

QString QDir::homeDirPath()
{
    QString d;
    d = QFile::decodeName(getenv("HOME"));
    slashify( d );
    if ( d.isNull() )
	d = rootDirPath();
    return d;
}

/*!
  Returns the absolute path for the root directory ("/" under UNIX).

  \sa root() drives()
*/

QString QDir::rootDirPath()
{
#if defined(_OS_FATFS_)
    QString d( "c:/" );
#elif defined(_OS_OS2EMX_)
    char dir[4];
    _abspath( dir, "/", _MAX_PATH );
    QString d( dir );
#elif defined(UNIX)
    QString d = QString::fromLatin1( "/" );
#else
# error Not implemented
#endif
    return d;
}

/*!
  Returns TRUE if the \e fileName matches the wildcard \e filter.
  \sa QRegExp
*/

bool QDir::match( const QString &filter, const QString &fileName )
{
    QRegExp tmp( filter, TRUE, TRUE ); // case sensitive and wildcard mode on
    return tmp.match( fileName ) != -1;
}


/*!
  Removes all multiple directory separators ('/') and resolves
  any "." or ".." found in the path.

  Symbolic links are kept.  This function does not return the
  canonical path.
*/

QString QDir::cleanDirPath( const QString &filePath )
{
    QString name = filePath;
    QString newPath;

    if ( name.isEmpty() )
	return name;

    slashify( name );

    bool addedSeparator;
    if ( isRelativePath(name) ) {
	addedSeparator = TRUE;
	name.insert( 0, '/' );
    } else {
	addedSeparator = FALSE;
    }

    int ePos, pos, upLevel;

    pos = ePos = name.length();
    upLevel = 0;
    int len;

    while ( pos && (pos = name.findRev('/',--pos)) != -1 ) {
	len = ePos - pos - 1;
	if ( len == 2 && name.at(pos + 1) == '.'
		      && name.at(pos + 2) == '.' ) {
	    upLevel++;
	} else {
	    if ( len != 0 && (len != 1 || name.at(pos + 1) != '.') ) {
		if ( !upLevel )
		    newPath = QString::fromLatin1("/")
			+ name.mid(pos + 1, len) + newPath;
		else
		    upLevel--;
	    }
	}
	ePos = pos;
    }
    if ( addedSeparator ) {
	while ( upLevel-- )
	    newPath.insert( 0, QString::fromLatin1("/..") );
	if ( !newPath.isEmpty() )
	    newPath.remove( 0, 1 );
	else
	    newPath = QString::fromLatin1(".");
    } else {
	if ( newPath.isEmpty() )
	    newPath = QString::fromLatin1("/");
#if defined(_OS_FATFS_) || defined(_OS_OS2EMX_)
	if ( name[0] == '/' ) {
	    if ( name[1] == '/' )		// "\\machine\x\ ..."
		newPath.insert( 0, '/' );
	} else {
	    newPath = name.left(2) + newPath;
	}
#endif
    }
    return newPath;
}

/*!
  Returns TRUE if the path is relative, FALSE if it is absolute.
  \sa isRelative()
*/

bool QDir::isRelativePath( const QString &path )
{
    int len = path.length();
    if ( len == 0 )
	return TRUE;
#if defined(_OS_FATFS_) || defined(_OS_OS2EMX_)
    int i = 0;
    if ( isalpha(path[0]) && path[1] == ':' )		// drive, e.g. a:
	i = 2;
    return path[i] != '/' && path[i] != '\\';
#elif defined(UNIX)
    return path[0] != '/';
#else
# error Not implemented for this operating system
#endif
}

struct QDirSortItem {
    QString filename_cache;
    QFileInfo* item;
};
static int cmp_si_sortSpec;
static int cmp_si( const void *n1, const void *n2 )
{
    if ( !n1 || !n2 )
        return 0;
 
    QDirSortItem* f1 = (QDirSortItem*)n1;
    QDirSortItem* f2 = (QDirSortItem*)n2;

    if ( cmp_si_sortSpec & QDir::DirsFirst )
	if ( f1->item->isDir() != f2->item->isDir() )
	    return f1->item->isDir() ? -1 : 1;

    int r = 0;
    int sortBy = cmp_si_sortSpec & QDir::SortByMask;

    switch ( sortBy ) {
      case QDir::Time:
	r = f1->item->lastModified().secsTo(f2->item->lastModified());
	break;
      case QDir::Size:
	r = f2->item->size() - f1->item->size();
	break;
      default:
	;
    }

    if ( r == 0 && sortBy != QDir::Unsorted ) {
	// Still not sorted - sort by name
	bool ic = cmp_si_sortSpec & QDir::IgnoreCase;

	if ( f1->filename_cache.isNull() )
	    f1->filename_cache = ic ? f1->item->fileName().lower()
				    : f1->item->fileName();
	if ( f2->filename_cache.isNull() )
	    f2->filename_cache = ic ? f2->item->fileName().lower()
				    : f2->item->fileName();

	r = f1->filename_cache.compare(f2->filename_cache);
    }

    if ( r == 0 ) {
	// Enforce an order - the order the items appear in the array
	r = (char*)n1 - (char*)n2;
    }

    if ( cmp_si_sortSpec & QDir::Reversed )
	return -r;
    else
	return r;
}

/*!
  \internal
  Reads directory entries.
*/

bool QDir::readDirEntries( const QString &nameFilter,
			   int filterSpec, int sortSpec )
{
    int i;
    if ( !fList ) {
	fList  = new QStringList;
	CHECK_PTR( fList );
	fiList = new QFileInfoList;
	CHECK_PTR( fiList );
	fiList->setAutoDelete( TRUE );
    } else {
	fList->clear();
	fiList->clear();
    }

    bool doDirs	    = (filterSpec & Dirs)	!= 0;
    bool doFiles    = (filterSpec & Files)	!= 0;
    bool noSymLinks = (filterSpec & NoSymLinks) != 0;
    bool doReadable = (filterSpec & Readable)	!= 0;
    bool doWritable = (filterSpec & Writable)	!= 0;
    bool doExecable = (filterSpec & Executable) != 0;
    bool doHidden   = (filterSpec & Hidden)	!= 0;
#if !defined(UNIX)
    // show hidden files if the user asks explicitly for e.g. .*
    if ( !doHidden && !nameFilter.isEmpty() && nameFilter[0] == '.' )
	doHidden = TRUE;
    bool doModified = (filterSpec & Modified)	!= 0;
    bool doSystem   = (filterSpec & System)	!= 0;
#endif

#if defined(_OS_WIN32_)

    QRegExp   wc( nameFilter, FALSE, TRUE );	// wild card, case insensitive
    bool      first = TRUE;
    QString   p = dPath.copy();
    int	      plen = p.length();
    HANDLE    ff;
    WIN32_FIND_DATA finfo;
    QFileInfo fi;

#undef	IS_SUBDIR
#undef	IS_RDONLY
#undef	IS_ARCH
#undef	IS_HIDDEN
#undef	IS_SYSTEM
#undef	FF_ERROR

#define IS_SUBDIR   FILE_ATTRIBUTE_DIRECTORY
#define IS_RDONLY   FILE_ATTRIBUTE_READONLY
#define IS_ARCH	    FILE_ATTRIBUTE_ARCHIVE
#define IS_HIDDEN   FILE_ATTRIBUTE_HIDDEN
#define IS_SYSTEM   FILE_ATTRIBUTE_SYSTEM
#define FF_ERROR    INVALID_HANDLE_VALUE

    if ( plen == 0 ) {
#if defined(CHECK_NULL)
	qWarning( "QDir::readDirEntries: No directory name specified" );
#endif
	return FALSE;
    }
    if ( p.at(plen-1) != '/' && p.at(plen-1) != '\\' )
	p += '/';
    p += "*.*";
    for ( i=0; i<=plen; i++ )
	if ( p[i] == '/' )
	    p[i] = '\\';

    if ( qt_winunicode ) {
	ff = FindFirstFile((TCHAR*)qt_winTchar(p,TRUE),&finfo);
    } else {
	// Cast is safe, since char is at end of WIN32_FIND_DATA
	ff = FindFirstFileA(qt_winQString2MB(p),(WIN32_FIND_DATAA*)&finfo);
    }
    if ( ff == FF_ERROR ) {
#if defined(CHECK_RANGE)
	qWarning( "QDir::readDirEntries: Cannot read the directory: %s (UTF8)",
		 dPath.utf8().data() );
#endif
	return FALSE;
    }

    while ( TRUE ) {
	if ( first )
	    first = FALSE;
	else {
	    if ( qt_winunicode ) {
		if ( !FindNextFile(ff,&finfo) )
		    break;
	    } else {
		if ( !FindNextFileA(ff,(WIN32_FIND_DATAA*)&finfo) )
		    break;
	    }
	}
	int  attrib = finfo.dwFileAttributes;
	bool isDir	= (attrib & IS_SUBDIR) != 0;
	bool isFile	= !isDir;
	bool isSymLink	= FALSE;
	bool isReadable = TRUE;
	bool isWritable = (attrib & IS_RDONLY) == 0;
	bool isExecable = FALSE;
	bool isModified = (attrib & IS_ARCH)   != 0;
	bool isHidden	= (attrib & IS_HIDDEN) != 0;
	bool isSystem	= (attrib & IS_SYSTEM) != 0;

	QString fname;
	if ( qt_winunicode ) {
	    fname = qt_winQString(finfo.cFileName);
	} else {
	    fname = qt_winMB2QString((const char*)finfo.cFileName);
	}
	if ( wc.match(fname) == -1 && !(allDirs && isDir) )
	    continue;

	if  ( (doDirs && isDir) || (doFiles && isFile) ) {
	    QString name = fname;
	    slashify(name);
	    if ( doExecable ) {
		QString ext = name.right(4).lower();
		if ( ext == ".exe" || ext == ".com" || ext == ".bat" ||
		     ext == ".pif" || ext == ".cmd" )
		    isExecable = TRUE;
	    }

	    if ( noSymLinks && isSymLink )
		continue;
	    if ( (filterSpec & RWEMask) != 0 )
		if ( (doReadable && !isReadable) ||
		     (doWritable && !isWritable) ||
		     (doExecable && !isExecable) )
		    continue;
	    if ( doModified && !isModified )
		continue;
	    if ( !doHidden && isHidden )
		continue;
	    if ( !doSystem && isSystem )
		continue;
	    fi.setFile( *this, name );
	    fiList->append( new QFileInfo( fi ) );
	}
    }
    FindClose( ff );

#undef	IS_SUBDIR
#undef	IS_RDONLY
#undef	IS_ARCH
#undef	IS_HIDDEN
#undef	IS_SYSTEM
#undef	FF_ERROR

#else // UNIX

#if defined(_OS_OS2EMX_)
    QRegExp   wc( nameFilter, FALSE, TRUE );	// wild card, case insensitive
#else
    QRegExp   wc( nameFilter, TRUE, TRUE );	// wild card, case sensitive
#endif
    QFileInfo fi;
    DIR	     *dir;
    dirent   *file;

    dir = opendir( QFile::encodeName(dPath) );
    if ( !dir ) {
#if defined(CHECK_NULL)
	qWarning( "QDir::readDirEntries: Cannot read the directory: %s",
		 QFile::encodeName(dPath).data() );
#endif
	return FALSE;
    }

    while ( (file = readdir(dir)) ) {
	QString fn = QFile::decodeName(file->d_name);
	fi.setFile( *this, fn );
	if ( wc.match(fn) == -1 && !(allDirs && fi.isDir()) )
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
    if ( closedir(dir) != 0 ) {
#if defined(CHECK_NULL)
	qWarning( "QDir::readDirEntries: Cannot close the directory: %s (UTF8)",
		 dPath.utf8().data() );
#endif
    }

#endif // UNIX

    // Sort...
    QDirSortItem* si= new QDirSortItem[fiList->count()];
    QFileInfo* itm;
    i=0;
    for (itm = fiList->first(); itm; itm = fiList->next())
	si[i++].item = itm;
    cmp_si_sortSpec = sortSpec;
    qsort( si, i, sizeof(si[0]), cmp_si );
    // put them back in the list
    fiList->setAutoDelete( FALSE );
    fiList->clear();
    int j;
    for (j=0; j<i; j++) {
	fiList->append(si[j].item);
	fList->append(si[j].item->fileName());
    }
    delete [] si;
    fiList->setAutoDelete( TRUE );

    if ( filterSpec == (FilterSpec)filtS && sortSpec == (SortSpec)sortS &&
	 nameFilter == nameFilt )
	dirty = FALSE;
    else
	dirty = TRUE;
    return TRUE;
}


/*!  Returns a list if the root directories on this system.  On
  win32, this returns a number of QFileInfo objects containing "C:/",
  "D:/" etc.  On other operating systems, it returns a list containing
  just one root directory (e.g. "/").
*/

const QFileInfoList * QDir::drives()
{
    // at most one instance of QFileInfoList is leaked, and this variable
    // points to that list
    static QFileInfoList * knownMemoryLeak = 0;

    if ( !knownMemoryLeak ) {
	knownMemoryLeak = new QFileInfoList;

#if defined(_OS_WIN32_)
	Q_UINT32 driveBits = (Q_UINT32) GetLogicalDrives() & 0x3ffffff;
#elif defined(_OS_OS2EMX_)
	Q_UINT32 driveBits, cur;
	if (DosQueryCurrentDisk(&cur,&driveBits) != NO_ERROR)
	    exit(1);
	driveBits &= 0x3ffffff;
#endif
#if defined(_OS_WIN32_) || defined(_OS_OS2EMX_)
	char driveName[4];
	qstrcpy( driveName, "a:/" );
	while( driveBits ) {
	    if ( driveBits & 1 )
		knownMemoryLeak->append( new QFileInfo( driveName ) );
	    driveName[0]++;
	    driveBits = driveBits >> 1;
	}

#else

	// non-win32 versions both use just one root directory
	knownMemoryLeak->append( new QFileInfo( rootDirPath() ) );

#endif
    }

    return knownMemoryLeak;
}
