/****************************************************************************
** $Id: //depot/qt/main/src/tools/qdir.cpp#13 $
**
** Implementation of QDir class
**
** Author  : Eirik Eng
** Created : 950427
**
** Copyright (C) 1995 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#include "qdir.h"
#include "qfileinf.h"
#include "qfiledef.h"
#include "qregexp.h"
#include <stdlib.h>

RCSTAG("$Id: //depot/qt/main/src/tools/qdir.cpp#13 $")


#if !defined(PATH_MAX)
#define PATH_MAX 300
#endif


#if defined(_OS_MSDOS_) || defined(_OS_WIN32_) || defined(_OS_OS2_)

static void convertSeparators( char *n )
{
    if ( !n )
	return;
    while ( *n ) {
	if ( *n ==  '\\' )
	    *n = '/';
	n++;
    }
}

#elif defined(UNIX)

static void convertSeparators( char * )
{
    return;
}

#endif


/*----------------------------------------------------------------------------
  \class QDir qdir.h
  \brief QDir can traverse the directory structure and read the contents of
	 directories in a platform-independent way.

  \ingroup tools

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
	warning( "Cannot find the example directory." );
  \endcode

  If you always use '/' as a directory separator, Qt will translate your
  paths to conform to the underlying operating system.

  cd() and cdUp() can be used to navigate the directory tree. Note that the
  logical cd and cdUp operations are not performed if the new directory does
  not exist.

  Example:
  \code
    QDir d = QDir::root();			// UNIX: "/"
    if ( !d.cd("tmp") ) {			// UNIX: "/tmp"
	warning( "Cannot find the \"/tmp\" directory" );
    } else {
	QFile f( d.filePath("ex1.txt") );	// UNIX: "/tmp/ex1.txt"
	if ( !f.open(IO_ReadWrite) )
	    warning( "Cannot create the file %s.", f.name() );
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
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Constructs a QDir pointing to the current directory.
  \sa currentDirPath()
 ----------------------------------------------------------------------------*/

QDir::QDir()
{
    dPath = ".";
    init();
}

/*----------------------------------------------------------------------------
  Constructs a QDir pointing to the given directory.

  No check is made to ensure that the directory exists.

  \sa exists(), setPath(), operator=()
 ----------------------------------------------------------------------------*/

QDir::QDir( const char *path )
{
    dPath = cleanDirPath( path );
    init();
}

/*----------------------------------------------------------------------------
  Constructs a QDir pointing to the given directory, setting the name filter
  to \e nameF. No check is made to ensure that the directory exists.

  \sa exists(), setPath(), setNameFilter(), setFilter(), setSorting()
 ----------------------------------------------------------------------------*/

QDir::QDir( const char *path, const char *nameF, int sortSpec, int filterSpec )
{
    dPath    = cleanDirPath( path );
    nameFilt = nameF;
    filtS    = (FilterSpec)filterSpec;
    sortS    = (SortSpec)sortSpec;

    fList    = 0;
    fiList   = 0;
    dirty    = TRUE;
    allDirs  = FALSE;
}

/*----------------------------------------------------------------------------
  Constructs a QDir that is a copy of the given directory.
  \sa operator=()
 ----------------------------------------------------------------------------*/

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
    nameFilt = "*";
    dirty    = TRUE;
    allDirs  = FALSE;
    filtS    = All;
    sortS    = SortSpec(Name | IgnoreCase);
}

/*----------------------------------------------------------------------------
  Destroys the QDir and cleans up.
 ----------------------------------------------------------------------------*/

QDir::~QDir()
{
    if ( fList )
       delete fList;
    if ( fiList )
       delete fiList;
}


/*----------------------------------------------------------------------------
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
 ----------------------------------------------------------------------------*/

void QDir::setPath( const char *path )
{
    dPath = cleanDirPath( path );
    dirty = TRUE;
}

/*----------------------------------------------------------------------------
  \fn  const char *QDir::path() const
  Returns the path, this may contain symbolic links, but never contains
  redundant ".", ".." or multiple separators.

  The returned path can be either absolute or relative (see setPath()).

  \sa setPath(), absPath(), exists(), cleanDirPath(), dirName(), absFilePath()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Returns the absolute (a path that starts with '/') path, which may
  contain symbolic links, but never contains redundant ".", ".." or
  multiple separators.

  \sa setPath(), canonicalPath(), exists(),  cleanDirPath(), dirName(),
  absFilePath()
 ----------------------------------------------------------------------------*/

QString QDir::absPath() const
{
    if ( QDir::isRelativePath(dPath) ) {
	QString tmp = currentDirPath();
	if ( tmp.right(1) != "/" )
	    tmp += '/';
	tmp += dPath;
	return cleanDirPath( tmp.data() );
    } else {
	return dPath.copy();
    }
}

/*----------------------------------------------------------------------------
  Returns the canonical path, i.e. a path without symbolic links.

  On systems that do not have symbolic links this function will
  always return the same string that absPath returns.
  If the canonical path does not exist a null string is returned.

  \sa path(), absPath(), exists(), cleanDirPath(), dirName(),
      absFilePath(), QString::isNull()
 ----------------------------------------------------------------------------*/

QString QDir::canonicalPath() const
{
    QString cur( PATH_MAX );
    QString tmp( PATH_MAX );

    GETCWD( cur.data(), PATH_MAX );
    if ( chdir( dPath ) >= 0 )
	GETCWD( tmp.data(), PATH_MAX );
    chdir( cur );

    return tmp;
}

/*----------------------------------------------------------------------------
  Returns the name of the directory, this is NOT the same as the path, e.g.
  a directory with the name "mail", might have the path "/var/spool/mail".
  If the directory has no name (e.g. the root directory) a null string is
  returned.

  No check is made to ensure that a directory with this name actually exists.

  \sa path(), absPath(), absFilePath(), exists(), QString::isNull()
 ----------------------------------------------------------------------------*/

QString QDir::dirName() const
{
    int pos = dPath.findRev( '/' );
    if ( pos == -1  )
	return dPath;
    return dPath.right( dPath.length() - pos - 1 );
}

/*----------------------------------------------------------------------------
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
 ----------------------------------------------------------------------------*/

QString QDir::filePath( const char *fileName,
			bool acceptAbsPath ) const
{
    if ( acceptAbsPath && !isRelativePath( fileName ) )
	return QString( fileName );

    QString tmp = dPath.copy();
    if ( tmp.isEmpty() || (tmp[(int)tmp.length()-1] != '/' && fileName &&
			   fileName[0] != '/') )
	tmp += '/';
    tmp += fileName;
    return tmp;
}

/*----------------------------------------------------------------------------
  Returns the absolute path name of a file in the directory. Does NOT check if
  the file actually exists in the directory. Redundant multiple separators
  or "." and ".." directories in \e fileName will NOT be removed (see
  cleanDirPath()).

  If \e acceptAbsPath is TRUE a \e fileName starting with a separator
  ('/') will be returned without change.
  if \e acceptAbsPath is FALSE an absolute path will be appended to
  the directory path.

  \sa filePath()
 ----------------------------------------------------------------------------*/

QString QDir::absFilePath( const char *fileName,
			   bool acceptAbsPath ) const
{
    if ( acceptAbsPath && !isRelativePath( fileName ) )
	return fileName;

    QString tmp = absPath();
    if ( tmp.isEmpty() || (tmp[(int)tmp.length()-1] != '/' && fileName &&
			   fileName[0] != '/') )
	tmp += '/';
    tmp += fileName;
    return tmp;
}

/*----------------------------------------------------------------------------
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
	      warning( "Cannot cd into \"%s\".", (char*)d.absFilePath("c++") );
	  else
	      warning( "Cannot make directory \"%s\"\n"
		       "A file named \"c++\" already exists in \"%s\".",
		       (char*) d.absFilePath("c++"), (char*) d.path() );
	  return;
      } else {
	  warning("Making directory \"%s\".", (char*) d.absFilePath("c++") );
	  if ( !d.mkdir( "c++" ) ) {
	      warning("Could not make directory \"%s\".",
		      (char*) d.absFilePath("c++") );
	      return;
	  }
      }
  }
  \endcode

  Calling cd( ".." ) is equivalent to calling cdUp.

  \sa cdUp(), isReadable(), exists(), path()
 ----------------------------------------------------------------------------*/

bool QDir::cd( const char *dirName, bool acceptAbsPath )
{
    if ( strcmp( dirName, "." ) == 0 ) {
	return TRUE;
    }
    QString old = dPath;
    dPath.detach();		// dPath can be shared by several QDirs
    if ( acceptAbsPath && !isRelativePath(dirName) ) {
	dPath = cleanDirPath( dirName );
    } else {
	if ( !isRoot() )
	    dPath += '/';
	dPath += dirName;
	if ( strchr( dirName, '/' ) || old == "." ||
	     strcmp( dirName, "..") == 0 )
	    dPath = cleanDirPath( dPath.data() );
    }
    if ( !exists() ) {
	dPath = old;
	return FALSE;
    }
    dirty = TRUE;
    return TRUE;
}

/*----------------------------------------------------------------------------
  Changes directory by moving one directory up the path followed to arrive
  at the current directory.

  Returns TRUE if the new directory exists and is readable. Note that the
  logical cdUp() operation is not performed if the new directory does not
  exist.

  \sa cd(), isReadable(), exists(), path()
 ----------------------------------------------------------------------------*/

bool QDir::cdUp()
{
    return cd( ".." );
}

/*----------------------------------------------------------------------------
  \fn const char *QDir::nameFilter() const
  Returns the string set by setNameFilter()
  \sa setNameFilter()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the name filter used by entryList() and entryInfoList().
  The name filter is
  a wildcarding filter that understands "*" and "?" wildcards, if you want
  entryList() and entryInfoList() to list all files ending with ".cpp", you
  simply call dir.setNameFilter("*.cpp");

  \sa nameFilter()
 ----------------------------------------------------------------------------*/

void QDir::setNameFilter( const char *nameFilter )
{
    nameFilt = nameFilter;
    dirty    = TRUE;
}

/*----------------------------------------------------------------------------
  \fn QDir::FilterSpec QDir::filter() const

  Returns the value set by setFilter()

  \sa setFilter()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the filter used by entryList() and entryInfoList(). The filter is used
  to specify the kind of files that should be returned by entryList() and
  entryInfoList(). The filter is specified by or-ing values from the enum
  FilterSpec. The different values are:


  <dl compact>
  <dt> Dirs <dd> List directories only.
  <dt> Files <dd> List files only.
  <dt> Drives <dd> List drives.
  <dt> NoSymLinks <dd> Do not list symbolic links.

  <dt> Readable <dd> List files with read permission.
  <dt> Writable <dd> List files with write permission.
  <dt> Executable <dd> List files with execute permission.

  Setting none of the three flags above is equivalent to setting all of them.

  <dt> Modified <dd> Only list files that have been modified (does nothing
			  under UNIX).
  <dt> Hidden <dd> List hidden files also (.* under UNIX).
  <dt> System <dd> List system files (does nothing under UNIX).

  </dl>

  \sa nameFilter()
 ----------------------------------------------------------------------------*/

void QDir::setFilter( int filterSpec )
{
    if ( filtS == (FilterSpec) filterSpec )
	return;
    filtS = (FilterSpec) filterSpec;
    dirty = TRUE;
}

/*----------------------------------------------------------------------------
  \fn QDir::SortSpec QDir::sorting() const

  Returns the value set by setSorting()

  \sa setSorting()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the sorting order used by entryList() and entryInfoList().

  The \e sortSpec is specified by or-ing values from the enum
  SortSpec. The different values are:

  One of these:
  <dl compact>
  <dt> Name <dd> Sort by name (alphabetical order).
  <dt> Time <dd> Sort by time (most recent first).
  <dt> Size <dd> Sort by size (largest first).
  <dt> Unsorted <dd> Use the operating system order (UNIX does NOT sort
  alphabetically).

  ORed with zero or more of these:

  <dt> DirsFirst <dd> Always put directory names first.
  <dt> Reversed <dd> Reverse sort order.
  <dt>IgnoreCase  <dd> Ignore case when sorting by name.
  </dl>
 ----------------------------------------------------------------------------*/

void QDir::setSorting( int sortSpec )
{
    if ( sortS == (SortSpec) sortSpec )
	return;
    sortS = (SortSpec) sortSpec;
    dirty = TRUE;
}

/*----------------------------------------------------------------------------
  \fn bool QDir::matchAllDirs() const
  Returns the value set by setMatchAllDirs()

  \sa setMatchAllDirs()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  If \e enable is TRUE, all directories will be listed (even if they do not
  match the filter or the name filter), otherwise only matched directories
  will be listed.

  \bug Currently, directories that do not match the filter will not be
  included (the name filter will be ignored as expected).

  \sa matchAllDirs()
 ----------------------------------------------------------------------------*/

void QDir::setMatchAllDirs( bool enable )
{
    if ( (bool)allDirs == enable )
	return;
    allDirs = enable;
    dirty = TRUE;
}


/*----------------------------------------------------------------------------
  Returns a list of the names of all files and directories in the directory
  pointed to using the setSorting(), setFilter() and setNameFilter()
  specifications.

  The the filter and sorting specifications can be overridden using the
  \e filterSpec and \e sortSpec arguments.

  Returns 0 if the directory is unreadable or does not exist.

  \sa entryInfoList(), setNameFilter(), setSorting(), setFilter()
 ----------------------------------------------------------------------------*/

const QStrList *QDir::entryList( int filterSpec, int sortSpec ) const
{
    if ( !dirty && filterSpec == (int)DefaultFilter &&
		   sortSpec   == (int)DefaultSort )
	return fList;
    return entryList( nameFilt, filterSpec, sortSpec );
}

/*----------------------------------------------------------------------------
  Returns a list of the names of all files and directories in the directory
  pointed to using the setSorting(), setFilter() and setNameFilter()
  specifications.

  The the filter and sorting specifications can be overridden using the
  \e nameFilter, \e filterSpec and \e sortSpec arguments.

  Returns 0 if the directory is unreadable or does not exist.

  \sa entryInfoList(), setNameFilter(), setSorting(), setFilter()
 ----------------------------------------------------------------------------*/

const QStrList *QDir::entryList( const char *nameFilter,
				 int filterSpec, int sortSpec ) const
{
    if ( filterSpec == (int)DefaultFilter )
	filterSpec = filtS;
    if ( sortSpec == (int)DefaultSort )
	sortSpec = sortS;
    QDir *This = (QDir*) this;		// Mutable function
    if ( This->readDirEntries( nameFilter, filterSpec, sortSpec ) )
	return fList;
    else
	return 0;
}

/*----------------------------------------------------------------------------
  Returns a list of QFileInfo objects for all files and directories in
  the directory pointed to using the setSorting(), setFilter() and
  setNameFilter() specifications.

  The the filter and sorting specifications can be overridden using the
  \e filterSpec and \e sortSpec arguments.

  Returns 0 if the directory is unreadable or does not exist.

  \sa entryList(), setNameFilter(), setSorting(), setFilter()
 ----------------------------------------------------------------------------*/

const QFileInfoList *QDir::entryInfoList( int filterSpec, int sortSpec ) const
{
    if ( !dirty && filterSpec == (int)DefaultFilter &&
		   sortSpec   == (int)DefaultSort )
	return fiList;
    return entryInfoList( nameFilt, filterSpec, sortSpec );
}

/*----------------------------------------------------------------------------
  Returns a list of QFileInfo objects for all files and directories in
  the directory pointed to using the setSorting(), setFilter() and
  setNameFilter() specifications.

  The the filter and sorting specifications can be overridden using the
  \e nameFilter, \e filterSpec and \e sortSpec arguments.

  Returns 0 if the directory is unreadable or does not exist.

  \sa entryList(), setNameFilter(), setSorting(), setFilter()
 ----------------------------------------------------------------------------*/

const QFileInfoList *QDir::entryInfoList( const char *nameFilter,
					  int filterSpec, int sortSpec ) const
{
    if ( filterSpec == (int)DefaultFilter )
	filterSpec = filtS;
    if ( sortSpec == (int)DefaultSort )
	sortSpec = sortS;
    QDir *This = (QDir*) this;		// Mutable function
    if ( This->readDirEntries( nameFilter, filterSpec, sortSpec ) )
	return fiList;
    else
	return 0;
}

/*----------------------------------------------------------------------------
  Creates a directory.

  If \e acceptAbsPath is TRUE a path starting with a separator ('/')
  will create the absolute directory, if \e acceptAbsPath is FALSE
  any number of separators at the beginning of \e dirName will be removed.

  Returns TRUE if successful, otherwise FALSE.

  \sa rmdir()
 ----------------------------------------------------------------------------*/

bool QDir::mkdir( const char *dirName, bool acceptAbsPath ) const
{
#if defined (UNIX)
    return MKDIR( filePath(dirName,acceptAbsPath), 0777 ) == 0;
#else
    return MKDIR( filePath(dirName,acceptAbsPath) ) == 0;
#endif
}

/*----------------------------------------------------------------------------
  Removes a directory.

  If \e acceptAbsPath is TRUE a path starting with a separator ('/')
  will remove the absolute directory, if \e acceptAbsPath is FALSE
  any number of separators at the beginning of \e dirName will be removed.

  The directory must be empty for rmdir() to succeed.

  Returns TRUE if successful, otherwise FALSE.

  \sa mkdir()
 ----------------------------------------------------------------------------*/

bool QDir::rmdir( const char *dirName, bool acceptAbsPath ) const
{
    return RMDIR( filePath(dirName,acceptAbsPath) ) == 0;
}

/*----------------------------------------------------------------------------
  Returns TRUE if the directory is readable AND we can open files by
  name. This function will return FALSE if only one of these is present.
  \warning A FALSE value from this function is not a guarantee that files
  in the directory are not accessible.

  \sa QFileInfo::isReadable()
 ----------------------------------------------------------------------------*/

bool QDir::isReadable() const
{
#if defined(UNIX)
    return ACCESS( dPath.data(), R_OK | X_OK ) == 0;
#else
    return ACCESS( dPath.data(), R_OK ) == 0;
#endif
}

/*----------------------------------------------------------------------------
   Returns TRUE if the directory exists. (If a file with the same
   name is found this function will of course return FALSE).

   \sa QFileInfo::exists(), QFile::exists()
 ----------------------------------------------------------------------------*/

bool QDir::exists() const
{
    QFileInfo fi( dPath );
    return fi.exists() && fi.isDir();
}

/*----------------------------------------------------------------------------
  Returns TRUE if the directory is the root directory, otherwise FALSE.

  Note: If the directory is a symbolic link to the root directory this
  function returns FALSE. If you want to test for this you can use
  canonicalPath():

  Example:
  \code
    QDir d( "/yo/root_link" );
    d = d.canonicalPath();
    if ( d.isRoot() )
	warning( "It IS a root link!" );
  \endcode

  \sa root(), rootDirPath()
 ----------------------------------------------------------------------------*/

bool QDir::isRoot() const
{
    return dPath == "/";		// UNIX test only ###
}

/*----------------------------------------------------------------------------
  Returns TRUE if the directory path is relative to the current directory,
  FALSE if the path is absolute (e.g. under UNIX a path is relative if it
  does not start with a '/').

  According to Einstein this function should always return TRUE.

  \sa convertToAbs()
 ----------------------------------------------------------------------------*/

bool QDir::isRelative() const
{
    return isRelativePath( dPath.data() );
}

/*----------------------------------------------------------------------------
  Converts the directory path to an absolute path. If it is already
  absolute nothing is done.

  \sa isRelative()
 ----------------------------------------------------------------------------*/

void QDir::convertToAbs()
{
    dPath = absPath();
}

/*----------------------------------------------------------------------------
  Makes a copy of d and assigns it to this QDir.
 ----------------------------------------------------------------------------*/

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

/*----------------------------------------------------------------------------
  Sets the directory path to be the given path.
 ----------------------------------------------------------------------------*/

QDir &QDir::operator=( const char *path )
{
    dPath = cleanDirPath( path );
    dirty = TRUE;
    return *this;
}


/*----------------------------------------------------------------------------
  \fn bool QDir::operator!=( const QDir &d ) const
  Returns TRUE if the \e d and this dir havee different path or
  different sort/filter settings, otherwise FALSE.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Returns TRUE if the \e d and this dir have the same path and all sort
  and filter settings are equal, otherwise FALSE.
 ----------------------------------------------------------------------------*/

bool QDir::operator==( const QDir &d ) const
{
    return dPath    == d.dPath &&
	   nameFilt == d.nameFilt &&
	   allDirs  == d.allDirs &&
	   filtS    == d.filtS &&
	   sortS    == d.sortS;
}


/*----------------------------------------------------------------------------
  Removes a file.

  If \e acceptAbsPath is TRUE a path starting with a separator ('/')
  will remove the file with the absolute path, if \e acceptAbsPath is FALSE
  any number of separators at the beginning of \e fileName will be removed.

  Returns TRUE if successful, otherwise FALSE.
 ----------------------------------------------------------------------------*/

bool QDir::remove( const char *fileName, bool acceptAbsPath )
{
    if ( fileName == 0 || fileName[0] == '\0' ) {
#if defined(CHECK_NULL)
	warning( "QDir::remove: Empty or NULL file name." );
#endif
	return FALSE;
    }
    QString tmp = filePath( fileName, acceptAbsPath );
#if defined(UNIX)
    return unlink( tmp ) == 0;			// unlink more common in UNIX
#else
    return ::remove( tmp ) == 0;		// use standard ANSI remove
#endif
}

/*----------------------------------------------------------------------------
  Renames a file.

  If \e acceptAbsPaths is TRUE a path starting with a separator ('/')
  will rename the file with the absolute path, if \e acceptAbsPath is FALSE
  any number of separators at the beginning of \e name will be removed.

  Returns TRUE if successful, otherwise FALSE.
 ----------------------------------------------------------------------------*/

bool QDir::rename( const char *name, const char *newName,
		   bool acceptAbsPaths	)
{
    if ( name == 0 || name[0] == '\0' || newName == 0 || newName[0] == '\0' ) {
#if defined(CHECK_NULL)
	warning( "QDir::rename: Empty or NULL file name(s)." );
#endif
	return FALSE;
    }
    QString tmp1 = filePath( name, acceptAbsPaths );
    QString tmp2 = filePath( newName, acceptAbsPaths );
    return rename( tmp1, tmp2) == 0;
}

/*----------------------------------------------------------------------------
  Checks for existence of a file.

  If \e acceptAbsPaths is TRUE a path starting with a separator ('/')
  will check the file with the absolute path, if \e acceptAbsPath is FALSE
  any number of separators at the beginning of \e name will be removed.

  Returns TRUE if the file exists, otherwise FALSE.

  \sa QFileInfo::exists(), QFile::exists()
 ----------------------------------------------------------------------------*/

bool QDir::exists( const char *name, bool acceptAbsPath )
{
    if ( name == 0 || name[0] == '\0' ) {
#if defined(CHECK_NULL)
	warning( "QDir::exists: Empty or NULL file name." );
#endif
	return FALSE;
    }
    QString tmp = filePath( name, acceptAbsPath );
    return QFile::exists( tmp.data() );
}

/*----------------------------------------------------------------------------
  Returns the native directory separator, e.g.
  "/" under UNIX and "\" under MS-DOS.

  You do not need to use this function to build file paths. If you always
  use '/', Qt will  translate your
  paths to conform to the underlying operating system.
*/ // "

char QDir::separator()
{
#if defined( UNIX )
    return '/';
#elif defined (_OS_MSDOS_) || defined(_OS_OS2_) || defined(_OS_WIN32_)
    return '\\';
#elif defined (_OS_MAC_)
    return ':';
#endif
    return '/';
}

/*----------------------------------------------------------------------------
  Sets the the current directory. Returns TRUE if successful.
 ----------------------------------------------------------------------------*/

bool QDir::setCurrent( const char *path )
{
    if ( chdir( path ) >= 0 )
	return TRUE;
    else
	return FALSE;
}

/*----------------------------------------------------------------------------
  Returns the current directory.
  \sa currentDirPath(), QDir::QDir()
 ----------------------------------------------------------------------------*/

QDir QDir::current()
{
    return QDir( currentDirPath() );
}

/*----------------------------------------------------------------------------
  Returns the home directory.
  \sa homeDirPath()
 ----------------------------------------------------------------------------*/

QDir QDir::home()
{
    return QDir( homeDirPath() );
}

/*----------------------------------------------------------------------------
  Returns the root directory.
  \sa rootDirPath()
 ----------------------------------------------------------------------------*/

QDir QDir::root()
{
    return QDir( rootDirPath() );
}

/*----------------------------------------------------------------------------
  Returns the absolute path of the current directory.
  \sa current()
 ----------------------------------------------------------------------------*/

QString QDir::currentDirPath()
{
    static bool forcecwd = TRUE;
    static ino_t cINode;
    static dev_t cDevice;
    static QString currentName( PATH_MAX );

    STATBUF st;

    if ( STAT( ".", &st ) == 0 ) {
	if ( forcecwd || cINode != st.st_ino || cDevice != st.st_dev ) {
	    if ( GETCWD( currentName.data(), PATH_MAX ) != 0 ) {
		cINode	 = st.st_ino;
		cDevice	 = st.st_dev;
		convertSeparators( currentName.data() );
		// forcecwd = FALSE;   ###
	    } else {
		warning("QDir::currentDirPath: getcwd() failed!");
		currentName = 0;
		forcecwd    = TRUE;
	    }
	}
    } else {
	warning("QDir::currentDirPath: stat(\".\") failed!");
	currentName = 0;
	forcecwd    = TRUE;
    }
    return currentName.copy();
}

/*----------------------------------------------------------------------------
  Returns the absolute path for the user's home directory,
  \sa home()
 ----------------------------------------------------------------------------*/

QString QDir::homeDirPath()
{
    QString d( PATH_MAX );
    d = getenv("HOME");
    if ( d.isNull() )
	d = rootDirPath();
    return d;
}

/*----------------------------------------------------------------------------
  Returns the absolute path for the root directory ("/" under UNIX).
  \sa root()
 ----------------------------------------------------------------------------*/

QString QDir::rootDirPath()
{
#if defined(_OS_MSDOS_) || defined(_OS_WIN32_) || defined(_OS_OS2_)
    QString d( "c:/" );
#elif defined(UNIX)
    QString d( "/" );
#else
# error Not implemented
#endif
    return d;
}

/*----------------------------------------------------------------------------
  Returns TRUE if the \e fileName matches the wildcard \e filter.
  \sa QRegExp
 ----------------------------------------------------------------------------*/

bool QDir::match( const char *filter, const char *fileName )
{
    QRegExp tmp( filter, TRUE, TRUE ); // case sensitive and wildcard mode on
    return tmp.match( fileName ) != -1;
}


/*----------------------------------------------------------------------------
  Removes all multiple directory separators ('/') and resolves
  any "." or ".." found in the path.

  Symbolic links are kept.  This function does not return the
  canonical path.
 ----------------------------------------------------------------------------*/

QString QDir::cleanDirPath( const char *filePath )
{
    QString name = filePath;
    QString newPath;

    if ( name.isEmpty() )
	return name;

    convertSeparators( name.data() );

    bool addedSeparator;
    if ( name.at( 0 ) != '/' ) {
	addedSeparator = TRUE;
	name.insert( 0, '/' );
    } else {
	addedSeparator = FALSE;
    }

    int ePos, pos, upLevel;

    pos = ePos = name.size() - 1;
    upLevel = 0;
    int len;

    while( pos && (pos = name.findRev( '/', --pos)) != -1 ) {
	len = ePos - pos - 1;
	if ( len == 2 && name.at( pos + 1 ) == '.'
		      && name.at( pos + 2 ) == '.' ) {
	    upLevel++;
	} else {
	    if ( len != 0 && ( len != 1 || name.at( pos + 1 ) != '.' ) ) {
		if ( !upLevel )
		    newPath = "/" + name.mid( pos + 1, len) + newPath;
		else
		    upLevel--;
	    }
	}
	ePos = pos;
    }
    if ( addedSeparator ) {
	while( upLevel-- )
	    newPath.insert( 0, "/.." );
	if ( !newPath.isEmpty() )
	    newPath.remove( 0, 1 );
	else
	    newPath = ".";
    } else {
	if ( newPath.isEmpty() )
	    newPath = "/";
    }
    return newPath;
}

/*----------------------------------------------------------------------------
  Returns TRUE if the path is relative, FALSE if it is absolute.
  \sa isRelative()
 ----------------------------------------------------------------------------*/

bool QDir::isRelativePath( const char *path )
{
    int len = strlen( path );
    if ( len == 0 )
	return FALSE;
#if defined(_OS_MSDOS_) || defined(_OS_WIN32_) || defined(_OS_OS2_)
    int i = 0;
    if ( len >= 2 && path[1] == ':' )		// drive
	i = 2;
    return path[i] != '/' && path[i] != '\\';
#elif defined(UNIX)
    return path[0] != '/';
#else
# error Not implemented for this operating system
#endif
}


static void dirInSort( QStrList *fl, QFileInfoList *fil, const char *fileName,
		       const QFileInfo &fi, int sortSpec )
{
    QFileInfo *newFi = new QFileInfo( fi );
    CHECK_PTR( newFi );
    int sortBy = sortSpec & QDir::SortByMask;
    if ( sortBy == QDir::Unsorted ) {
	if ( sortSpec & QDir::Reversed ) {
	    fl ->insert( 0, fileName );
	    fil->insert( 0, newFi );
	} else {
	    fl ->append( fileName );
	    fil->append( newFi );
	}
	return;
    }

    char      *tmp1;
    QFileInfo *tmp2;
    tmp1 = ( sortSpec & QDir::Reversed ) ? fl->last() : fl->first();
    if ( sortBy != QDir::Name )
	tmp2 = ( sortSpec & QDir::Reversed ) ? fil->last() : fil->first();
    bool stop = FALSE;
    while ( tmp1 ) {
	switch( sortBy ) {
	    case QDir::Name:
		if ( sortSpec & QDir::IgnoreCase ) {
		    if ( qstricmp( fileName , tmp1 ) < 0 )
			stop = TRUE;
		} else {
		    if ( strcmp( fileName , tmp1 ) < 0 )
			stop = TRUE;
		}
		break;
	    case QDir::Time:
		if ( fi.lastModified() > tmp2->lastModified() )
		    stop = TRUE;
		break;
	    case QDir::Size:
		if ( fi.size() > tmp2->size() )
		    stop = TRUE;
		break;
	}
	if (stop)
	    break;
	tmp1 = ( sortSpec & QDir::Reversed ) ? fl->prev() : fl->next();
	if ( sortBy != QDir::Name )
	    tmp2 = ( sortSpec & QDir::Reversed ) ? fil->prev() : fil->next();
    }
    int pos;
    if ( stop )
	pos = fl->at() + (( sortSpec & QDir::Reversed ) ? 1 : 0);
    else
	pos = ( sortSpec & QDir::Reversed ) ? 0 : fl->count();
    fl ->insert( pos, fileName );
    fil->insert( pos, newFi );
}


/*----------------------------------------------------------------------------
  \internal
  Reads directory entries.
 ----------------------------------------------------------------------------*/

bool QDir::readDirEntries( const QString &nameFilter,
			   int filterSpec, int sortSpec )
{
    if ( !fList ) {
	fList  = new QStrList;
	CHECK_PTR( fList );
	fiList = new QFileInfoList;
	CHECK_PTR( fiList );
	fiList->setAutoDelete( TRUE );
    }
    fList->clear();
    fiList->clear();

    bool doDirs	    = (filterSpec & Dirs)	!= 0;
    bool doFiles    = (filterSpec & Files)	!= 0;
    bool noSymLinks = (filterSpec & NoSymLinks) != 0;
    bool doReadable = (filterSpec & Readable)	!= 0;
    bool doWritable = (filterSpec & Writable)	!= 0;
    bool doExecable = (filterSpec & Executable) != 0;
    bool doHidden   = (filterSpec & Hidden)	!= 0;
#if !defined(UNIX)
    bool doModified = (filterSpec & Modified)	!= 0;
    bool doSystem   = (filterSpec & System)	!= 0;
#endif
    bool dirsFirst  = (sortSpec	  & DirsFirst)	!= 0;

    QStrList	  *dList  = 0;
    QFileInfoList *diList = 0;

    if ( dirsFirst ) {
	dList = new QStrList;
	CHECK_PTR( dList );
	diList	 = new QFileInfoList;
	CHECK_PTR( dList );
    }

#if defined(_OS_MSDOS_) || defined(_OS_WIN32_)

    QRegExp   wc( nameFilter, FALSE, TRUE );	// wild card, case insensitive
    bool      first = TRUE;
    QString   p = dPath.copy();
    int	      plen = p.length();
    long      ff;
    _finddata_t finfo;
    QFileInfo fi;

    if ( plen == 0 ) {
#if defined(CHECK_NULL)
	warning( "QDir::readDirEntries: No directory name specified" );
#endif
	return FALSE;
    }
    if ( p.at(plen-1) != '/' && p.at(plen-1) != '\\' )
	p += '/';
    p += "*.*";

    ff = _findfirst( p.data(), &finfo );
    if ( ff == -1 ) {
#if defined(CHECK_NULL)
	warning( "QDir::readDirEntries: Cannot read the directory: %s",
		 (const char *)dPath );
#endif
	return FALSE;
    }

    while ( TRUE ) {
	if ( first )
	    first = FALSE;
	else {
	    if ( _findnext(ff,&finfo) == -1 )
		break;
	}
	bool isDir	= (finfo.attrib & _A_SUBDIR) != 0;
	bool isFile	= !isDir;
	bool isSymLink	= FALSE;
	bool isReadable = TRUE;
	bool isWritable = (finfo.attrib & _A_RDONLY) == 0;
	bool isExecable = FALSE;
	bool isModified = (finfo.attrib & _A_ARCH)   != 0;
	bool isHidden	= (finfo.attrib & _A_HIDDEN) != 0;
	bool isSystem	= (finfo.attrib & _A_SYSTEM) != 0;

	if ( wc.match(finfo.name) == -1 && !(allDirs && isDir) )
	    continue;

	QString name = finfo.name;
	if ( doExecable ) {
	    QString ext = name.right(4).lower();
	    if ( ext == ".exe" || ext == ".com" || ext == ".bat" ||
		 ext == ".pif" || ext == ".cmd" )
		isExecable = TRUE;
	}

	if  ( (doDirs && isDir) || (doFiles && isFile) ) {
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
	    if ( dirsFirst && isDir )
		dirInSort( dList, diList, name, fi, sortSpec );
	    else
		dirInSort( fList, fiList, name, fi, sortSpec );
	}
    }
    _findclose( ff );

#elif defined(UNIX)

    QRegExp   wc( nameFilter, TRUE, TRUE );	// wild card, case sensitive
    QFileInfo fi;
    DIR	     *dir;
    dirent   *file;

    dir = opendir( dPath );
    if ( !dir ) {
#if defined(CHECK_NULL)
	warning( "QDir::readDirEntries: Cannot read the directory: %s",
		 (const char *)dPath );
#endif
	return FALSE;
    }

    if ( !nameFilter.isEmpty() && nameFilter[0] == '.' )
	filterSpec |= Hidden;

    while ( (file = readdir(dir)) ) {
	fi.setFile( *this, file->d_name );
	if ( wc.match(file->d_name) == -1 && !(allDirs && fi.isDir()) )
	    continue;
	if  ( (doDirs && fi.isDir()) || (doFiles && fi.isFile()) ) {
	    if ( noSymLinks && fi.isSymLink() )
		continue;
	    if ( (filterSpec & RWEMask) != 0 )
		if ( (doReadable && !fi.isReadable()) ||
		     (doWritable && !fi.isWritable()) ||
		     (doExecable && !fi.isExecutable()) )
		    continue;
	    if ( !doHidden && (file->d_name[0] == '.') &&
		 (file->d_name[1] != '\0') &&
		 (file->d_name[1] != '.' || file->d_name[2] != '\0') )
		continue;
	    if ( dirsFirst && fi.isDir() )
		dirInSort( dList, diList , file->d_name, fi, sortSpec );
	    else
		dirInSort( fList, fiList, file->d_name, fi, sortSpec );
	}
    }
    if ( closedir(dir) != 0 )
#if defined(CHECK_NULL)
	warning( "QDir::readDirEntries: Cannot close the directory: %s",
		 (const char *)dPath );
#endif

#endif

    if ( dirsFirst ) {
	char	  *tmp	 = dList ->last();
	QFileInfo *fiTmp = diList->last();
	while ( tmp ) {
	    fList->insert( 0, tmp );
	    tmp = dList->prev();
	    fiList->insert( 0, fiTmp );
	    fiTmp = diList->prev();
	}
	delete dList;
	delete diList;
    }
    if ( filterSpec == (FilterSpec)filtS && sortSpec == (SortSpec)sortS &&
	 nameFilter == nameFilt )
	dirty = FALSE;
    else
	dirty = TRUE;
    return TRUE;
}
