/****************************************************************************
** $Id: //depot/qt/main/src/tools/qdir.cpp#7 $
**
** Implementation of QDir class
**
** Author  : Eirik Eng
** Created : 950427
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qdir.h"
#include "qfileinf.h"
#include "qfiledef.h"
#include "qregexp.h"

#include<stdlib.h>
#include<sys/types.h>
#include<dirent.h>
#include<unistd.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/tools/qdir.cpp#7 $";
#endif

  /*! \class QDir qdir.h

  \brief The QDir class provides system-independent directory access and
  related functions.

  \ingroup tools

  The QDir class can be used to traverse the directory structure and to
  read the contents of directories in a platform-independent way.

  A QDir can point to a file using either a relative or an absolute
  file path. Absolute file paths begin with the directory separator
  ('/' under UNIX) or a drive specification (not applicable to UNIX).
  Relative file names begin with a directory name or a file name and specify
  a path relative to the current directory. Under UNIX, an example of
  an absolute path is the string "/tmp/quartz", a relative path might look like
  "src/fatlib". You can use the function isRelative() to check if a QDir
  is using a relative or an absolute file path. You can call the function
  convertToAbsolute() to convert a relative QDir to an absolute one.

  The directory "example" under the current directory is checked for existence
  in the example below:

  \code
  QDir d("example");      // points to "example" under the home directory
  if ( !d.exists() )
      warning("Cannot find the example directory.");
  \endcode

  If you construct paths manually, the static function
  separator() or the macro Q_SEPARATOR can be used to ensure
  that your programs will be platform independent:

  \code
      QString myPath;
      myPath = "mystuff"           // "mystuff"
      myPath += QDir::separator(); // "mystuff/" on UNIX, "mystuff\" on MS-DOS
      myPath += "examples";        // "mystuff/examples" or "mystuff\examples"
      QDir d( myPath );
  \endcode

  The macro Q_SEPARATOR expands to a \e string containing the directory
  separator and can be used like this:

  \code
  QDir d( "mystuff" Q_SEPARATOR "examples" ); // "mystuff/examples" on UNIX.
  \endcode

  cd() and cdUp() can be used to navigate the directory tree. Note that the
  logical cd and cdUp operations are not performed if the new directory does
  not exist:

  \code
  QDir d = QDir::root();              // "/" on UNIX
  if ( !d.cd("tmp") ) {               // "/tmp" on UNIX
      warning("Cannot find the " Q_SEPARATOR "tmp directory." );
  } else {
      QFile f( d.pathName("ex1.txt") );
      if ( f.open(IO_ReadWrite) )
          warning("Cannot create the file %s.", f.name() );
  }
  \endcode

  To read the contents of a directory you can use the entries() and
  entryInfos() functions. The example below lists all files in the current
  directory sorted by size with the smallest files first:

  \code
  #include <stdlib.h>
  #include <qdir.h>

  main() {
      QDir d;                                      // current directory
                                                   // list all files that are
                                                   // not symbolic links:
      d.setFilter( QDir::Files | QDir::Hidden | QDir::NoSymLinks );
      d.setSorting( QDir::Size | QDir::Reversed ); // smallest files first
      const QFileInfoList *fil = d.entryInfos();   // get file infos
      QFileInfo *fi;                               // pointer for traversing
      QFileInfoIterator fiter( *fil );             // create iterator
      printf( "     BYTES FILENAME\n" );           // print header
      for( fi = fiter.toFirst() ; fi ; fi = fiter++ )  // loop for all files
          printf( "%10li %s\n",  fi->size(), fi->fileName().data() );
}

  \endcode
*/ // "


  /*!
  Constructs a QDir pointing to the current directory.
  */
QDir::QDir()
{
    dPath = ".";
    init();
}

  /*!
  Constructs a QDir pointing to the given directory. No check is made to ensure
  that the directory exists.

  \sa exists()
  */
QDir::QDir( const char *pathName )
{
    dPath = cleanPathName( pathName );
    init();
}

  /*!
  Constructs a QDir that is a copy of the given directory.
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
    nameFilt = "*";
    dirty    = TRUE;
    allDirs  = FALSE;
    filtS    = All;
    sortS    = Name | IgnoreCase;
}

  /*
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
  Sets the path of the directory. The path is cleaned of ".", ".."
  and multiple separators. No check is made to ensure that a directory
  with this path exists.

  The path can be either absolute or relative. Absolute paths begin with the
  directory separator ('/' under UNIX) or a drive specification (not
  applicable to UNIX).
  Relative file names begin with a directory name or a file name and specify
  a path relative to the current directory. Under UNIX, an example of
  an absolute path is the string "/tmp/quartz", a relative path might look like
  "src/fatlib". You can use the function isRelative() to check if a QDir
  is using a relative or an absolute file path. You can call the function
  convertToAbsolute() to convert a relative QDir to an absolute one.

  \sa path(), absolutePath(), exists, cleanPathName(), dirName(),
      fullPathName(), isRelative(), convertToAbsolute()
  */
void QDir::setPath( const char *relativeOrAbsolutePath )
{
    dPath = cleanPathName( relativeOrAbsolutePath );
    dirty = TRUE;
}

  /*!
  \fn  const char *QDir::path() const
  Returns the path, this may contain symbolic links, but never contains
  redundant ".", ".." or multiple separators. The returned path can be
  either absolute or relative (see setPath()).
   \sa  setPath(), absolutePath(), exists(), cleanPathName(), dirName(),
       fullPathName()
  */

  /*!
  Returns the full path, i.e. this may contain symbolic links, but never
  contains redundant ".", ".." or multiple separators. The returned path
  can be either absolute or relative (see setPath()).
   \sa  setPath(), absolutePath(), exists(), cleanPathName(), dirName(),
       fullPathName()
  */

QString QDir::absolutePath() const
{
    if ( QDir::isRelativePath(dPath) ) {
        QString tmp = currentDirString();
        if ( tmp.right(1) != Q_SEPARATOR )
            tmp += separator();
        tmp += dPath;
        return tmp;
    } else {
        return dPath.copy();
    }
}

  /*!
  Returns the canonical path, i.e. a path without symbolic links.
  On systems that do not have symbolic links this function will
  always return the same string that absolutePath returns.
  If the canonical path does not exist a null string is returned.
  
  \sa path(), fullPath(), exists(), cleanPathName(), dirName(),
      fullPathName(), QString::isNull()
  */

QString QDir::canonicalPath() const
{
    QString cur( PATH_MAX );
    QString tmp( PATH_MAX );

    getcwd( cur.data(), PATH_MAX );   
    if ( chdir( dPath ) >= 0 )
        getcwd( tmp.data(), PATH_MAX );
    chdir( cur );

    return tmp;
}

  /*!
  Returns the name of the directry, this is NOT the same as the path, e.g.
  a directory with the name "mail", might have the path "/var/spool/mail".
  If the directory has no name (e.g. the root directory) a null string is
  returned.
  No check is made to ensure that a directory with this name actually exists.
  \sa path(), absolutePath(), fullPathName(), exists(), QString::isNull()
  */

QString QDir::dirName() const
{
    int pos = dPath.findRev( separator() );
    if ( pos == -1  )
        return dPath;
    return dPath.right( dPath.length() - pos - 1 );
}

  /*!
  Returns the path name of a file in the directory. Does NOT check if
  the file actually exists in the directory. If the QDir is relative
  the returned pathname will also be relative. Redundant multiple separators
  or "." and ".." directories in \e fileName will NOT be removed (see
  cleanPathName()).

  If \e acceptAbsolutePath is TRUE a \e fileName starting with a separator
  ('/' under UNIX) will return the path of a file in the absolute directory,
  if \e acceptAbsolutePath is FALSE an absolute path will be appended to
  the directory path.

  \sa fullPathName(), isRelative()
  */
QString QDir::pathName( const char *fileName,
                        bool acceptAbsolutePath ) const
{
    if ( acceptAbsolutePath && !isRelativePath( fileName ) )
        return QString( fileName );

    QString tmp = dPath.copy();
    if ( tmp.right(1) != Q_SEPARATOR && fileName && fileName[0] != separator())
        tmp += separator();
    tmp += fileName;
    return tmp;
}

  /*!
  Returns the full path name of a file in the directory. Does NOT check if
  the file actually exists in the directory. Redundant multiple separators
  or "." and ".." directories in \e fileName will NOT be removed (see
  cleanPathName()).

  If \e acceptAbsolutePath is TRUE a \e fileName starting with a separator
  ('/' under UNIX) will return the path of a file in the absolute directory,
  if \e acceptAbsolutePath is FALSE an absolute path will be appended to
  the directory path.

  \sa pathName()
  */

QString QDir::fullPathName( const char *fileName,
                            bool acceptAbsolutePath ) const
{
    if ( acceptAbsolutePath && !isRelativePath( fileName ) )
        return fileName;

    QString tmp = absolutePath();
    if ( tmp.right(1) != Q_SEPARATOR && fileName && fileName[0] != separator())
        tmp += separator();
    tmp += fileName;
    return tmp;
}

  /*!
  Changes directory by descending into the given directory. Returns
  TRUE if the new directory exists and is readable. Note that the logical
  cd operation is NOT performed if the new directory does not exist. 

  If \e acceptAbsolutePath is TRUE a path starting with a separator ('/' under
  UNIX) will cd to the absolute directory, if \e acceptAbsolutePath is FALSE
  any number of separators at the beginning of \e dirName will be removed.

  Example: \code

  QDir d = QDir::home();  // now points to home directory
  if ( !d.cd("c++") ) {   // now points to "c++" under home directory if OK
      QFileInfo fi( d, "c++" );
      if ( fi.exists() ) {
          if ( fi.isDir() )
              warning( "Cannot cd into \"%s\".", (char*) d.fullPathName("c++") );
          else
              warning( "Cannot make directory \"%s\"\n"
                       "A file named \"c++\" already exists in \"%s\".", 
                       (char*) d.fullPathName("c++"), (char*) d.path() );
          return;
      } else {
          warning("Making directory \"%s\".", (char*) d.fullPathName("c++") );
          if ( !d.mkdir( "c++" ) ) {
              warning("Could not make directory \"%s\".",
                      (char*) d.fullPathName("c++") );
              return;
          }
      }
  }
  \endcode

   Calling cd( ".." ) is equivalent to calling cdUp. 

  \sa cdUp(), isReadable(), exists(), path()
  */

bool QDir::cd( const char *dirName, bool acceptAbsolutePath )
{
    if ( strcmp( dirName, "." ) == 0 ) {
        return TRUE;
    }
    QString old = dPath;
    dPath.detach();                   // dPath can be shared by several QDirs
    if ( acceptAbsolutePath && !isRelativePath(dirName) ) {
        dPath = cleanPathName( dirName );
    } else {
        if ( !isRoot() )
            dPath += separator();
        dPath += dirName;
        if ( strchr( dirName, separator() ) || old == "." || 
             strcmp( dirName, "..") == 0 )
            dPath = cleanPathName( dPath.data() );
    }
    if ( !exists() ) {
        dPath = old;
        return FALSE;
    }
    dirty = TRUE;
    return TRUE;
}

  /*!
  Changes directory by moving one directory up the path followed to arrive
  at the current directory. Returns TRUE if the new directory exists and is
  readable. Note that the logical cdUp() operation is NOT performed if the
  new directory does not exist. 
  */

bool QDir::cdUp()
{
    return cd( ".." );
}

  /*!
  \fn const char *QDir::nameFilter() const

  Returns the string set by setNameFilter()

  \sa setNameFilter
  */

  /*!
  Sets the name filter used by entries() and entryInfos(). The name filter is
  a wildcarding filter that understands "*" and "?" wildcards, if you want
  entries() and entryInfos() to list all files ending with ".cpp", you
  simply call dir.setNameFilter("*.cpp");

  \sa nameFilter()
  */
void QDir::setNameFilter( const char *nameFilter )
{
    nameFilt = nameFilter;
    dirty    = TRUE;
}

  /*!
  \fn QDir::FilterSpec QDir::filter() const

  Returns the value set by setFilter()

  \sa setFilter
  */

  /*!
  Sets the filter used by entries() and entryInfos(). The filter is used
  to specify the kind of files that should be returned by entries() and
  entryInfos(). The filter is specified by or-ing values from the enum
  FilterSpec. The different values are:


  <dl compact>
  <dt> Dirs <dd>
  <dt> Files <dd>
  <dt> Drives <dd>
  <dt> NoSymLinks <dd>
  <dt> All <dd>

  <dt> Readable <dd>
  <dt> Writable <dd>
  <dt> Executable <dd>

  <dt> Modified <dd>
  <dt> Hidden <dd>
  <dt> System <dd>

  <dt> DefaultFilter <dd>
  </dl>

  <dl>
  <dt> Name <dd>
  <dt> Time <dd>
  <dt> Size <dd>
  <dt> Unsorted <dd>

  <dt> DirsFirst <dd>
  <dt> Reversed <dd>
  <dt>IgnoreCase  <dd>
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

void QDir::setSorting( int sortSpec )
{
    if ( sortS == (SortSpec) sortSpec )
        return;
    sortS = (SortSpec) sortSpec;
    dirty = TRUE;
}

void QDir::setMatchAllDirs( bool on )
{
    if ( allDirs == on )
        return;
    allDirs = on;
    dirty = TRUE;
}


  /*!          76093020
  Appends, in alphabetical order, the names of files in the directory to the
  QStrList. If filter is 0 (default) all files are appended, if not, only
  the ones matching the filter are appended. If includeSymLinks is TRUE
  (default) symbolic links to files are also appended.

  Returns FALSE if the directory is unreadable or does not exist.  \sa
  readDirs(), readAll(), match(), QRegExp::setWildCard(),
  QFile::isSymLink(), QFile::isRegular()
  */

const QStrList *QDir::entries( int filterSpec, int sortSpec ) const
{
    if ( !dirty && filterSpec == (int)DefaultFilter && 
                   sortSpec   == (int)DefaultSort )
        return fList;
    return entries( nameFilt, filterSpec, sortSpec );
}

const QStrList *QDir::entries( const char *nameFilter,
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

const QFileInfoList *QDir::entryInfos( int filterSpec, int sortSpec ) const
{
    if ( !dirty && filterSpec == (int)DefaultFilter && 
                   sortSpec   == (int)DefaultSort )
        return fiList;
    return entryInfos( nameFilt, filterSpec, sortSpec );
}

const QFileInfoList *QDir::entryInfos( const char *nameFilter,
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

/*
  If \e acceptAbsolutePath is TRUE a path starting with a separator ('/' under
  UNIX) will cd to the absolute directory, if \e acceptAbsolutePath is FALSE
  any number of separators at the beginning of \e dirName will be removed.
*/

bool QDir::mkdir( const char *dirName, bool acceptAbsolutePath ) const
{
    if ( ::mkdir( pathName( dirName, acceptAbsolutePath ) , 0777 ) == 0 )
        return TRUE;
    else
        return FALSE;
}

/*
  If \e acceptAbsolutePath is TRUE a path starting with a separator ('/' under
  UNIX) will cd to the absolute directory, if \e acceptAbsolutePath is FALSE
  any number of separators at the beginning of \e dirName will be removed.
*/

bool QDir::rmdir( const char *dirName, bool acceptAbsolutePath ) const
{
    if ( rmdir( pathName( dirName, acceptAbsolutePath  ) ) == 0 )
        return TRUE;
    else
        return FALSE;
}

  /*!
  Returns TRUE if the directory is readable AND we can open files by
  name. This function will return FALSE if only one of these is present.
  \warning A FALSE value from this function is not a guarantee that files
  in the directory are not accessible.
  */
bool QDir::isReadable() const
{
    return access( dPath.data(), R_OK | X_OK ) == 0;
}

  /*!
   Returns TRUE if the directory exists. (If a file with the same 
   name is found this function will of course return FALSE).
  */

bool QDir::exists() const
{
    QFileInfo fi( dPath );
    return fi.exists() && fi.isDir();
}

  /*!
  Returns TRUE if the directory is the root directory, FALSE otherwise. 
  Note: If the directory is a symbolic link to the root directory this
  function returns FALSE. If you want to test for this you can use
  absolutePath():
  \code
  QDir d( Q_SEPARATOR "yo" Q_SEPARATOR "root_link" );
  if ( d.absolutePath().isRoot() )
    warning( "It IS a root link!" );
  \endcode
  */
bool QDir::isRoot() const
{
    return dPath == Q_SEPARATOR;		// UNIX test only ###
}

bool QDir::isRelative() const
{
    return isRelativePath( dPath.data() );
}

void QDir::convertToAbsolute()
{
    dPath = absolutePath();
}


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

QDir &QDir::operator=( const char *relativeOrAbsolutePath )
{
    dPath = cleanPathName( relativeOrAbsolutePath );
    dirty = TRUE;
    return *this;    
}

bool QDir::operator==( const QDir &d )
{
    return dPath    == d.dPath &&
           nameFilt == d.nameFilt &&
           allDirs  == d.allDirs &&
           filtS    == d.filtS &&
           sortS    == d.sortS;
}

  /*!
  Sets the the current directory to be this directory. 
  Returns TRUE if successful.
  */
bool QDir::setToCurrent() const
{
    return setCurrent( dPath.data() );
}

/*
  If \e acceptAbsolutePath is TRUE a path starting with a separator ('/' under
  UNIX) will cd to the absolute directory, if \e acceptAbsolutePath is FALSE
  any number of separators at the beginning of \e dirName will be removed.
*/

bool QDir::remove( const char *fileName, bool acceptAbsolutePath )
{
    if ( fileName == 0 || fileName[0] == '\0' ) {
#if defined(CHECK_NULL)
	warning( "QDir::remove: Empty or NULL file name." );
#endif
	return FALSE;
    }
    QString tmp = pathName( fileName, acceptAbsolutePath );
#if defined(UNIX)
    return unlink( tmp ) == 0;			// unlink more common in UNIX
#else
    return ::remove( tmp ) == 0;		// use standard ANSI remove
#endif
}

/*
  If \e acceptAbsolutePath is TRUE a path starting with a separator ('/' under
  UNIX) will cd to the absolute directory, if \e acceptAbsolutePath is FALSE
  any number of separators at the beginning of \e dirName will be removed.
*/

bool QDir::rename( const char *name, const char *newName,
                   bool acceptAbsolutePaths  )
{
    if ( name == 0 || name[0] == '\0' || newName == 0 || newName[0] == '\0' ) {
#if defined(CHECK_NULL)
	warning( "QDir::rename: Empty or NULL file name(s)." );
#endif
	return FALSE;
    }
    QString tmp1 = pathName( name, acceptAbsolutePaths );
    QString tmp2 = pathName( newName, acceptAbsolutePaths );
    return rename( tmp1, tmp2) == 0;
}

/*
  If \e acceptAbsolutePath is TRUE a path starting with a separator ('/' under
  UNIX) will cd to the absolute directory, if \e acceptAbsolutePath is FALSE
  any number of separators at the beginning of \e dirName will be removed.
*/

bool QDir::exists( const char *name, bool acceptAbsolutePath )
{
    if ( name == 0 || name[0] == '\0' ) {
#if defined(CHECK_NULL)
	warning( "QDir::exists: Empty or NULL file name." );
#endif
	return FALSE;
    }
    QString tmp = pathName( name, acceptAbsolutePath );
    return QFile::exists( tmp.data() );
}

  /*!
  Returns the directory separator in paths on the current platform, e.g.
  "/" under UNIX and "\" under MS-DOS. 
  */ // "
char QDir::separator()
{
#if defined( UNIX )
    return '/';
#elif defined (_OS_MSDOS_) || defined(_OS_OS2_) || defined(_OS_WINNT_)
    return '\\';
#elif defined (_OS_MAC_)
    return ':';
#endif
    return '/';
}

  /*!
  Sets the the current directory. Returns TRUE if successful.
  */
bool QDir::setCurrent( const char *path )
{
    if ( chdir( path ) >= 0 )
        return TRUE;
    else
        return FALSE;
}

  /*!
  Returns the current directory.
  */
QDir QDir::current()
{
    return QDir( currentDirString() );
}

  /*!
  Returns the home directory.
  */
QDir QDir::home()
{
    return QDir( homeDirString() );
}

  /*!
  Returns the root directory.
  */
QDir QDir::root()
{
    return QDir( rootDirString() );
}

QString QDir::currentDirString()
{
    static bool forcecwd = TRUE;
    static ino_t cINode;
    static dev_t cDevice;
    static QString currentName( PATH_MAX );

    struct stat st;

    if ( STAT( ".", &st ) == 0 ) {
        if ( forcecwd || cINode != st.st_ino || cDevice != st.st_dev ) {
            if ( getcwd( currentName.data(), PATH_MAX ) == 0 ) {
                currentName.resize( strlen( currentName.data() ) + 1 );
                cINode   = st.st_ino;
                cDevice  = st.st_dev;
                // forcecwd = FALSE;   ###
            } else {
                warning("QDir::currentDirString: getcwd() failed!");
                currentName = 0;
                forcecwd    = TRUE;
	    }
	}
    } else {
        warning("QDir::currentDirString: stat(\".\") failed!");
        currentName = 0;
        forcecwd    = TRUE;
    }
    return currentName;
}

QString QDir::homeDirString()
{
    QString tmp( PATH_MAX );

    tmp = getenv("HOME");
    if ( tmp.isNull() )
        tmp = rootDirString();
    return tmp;
}

QString QDir::rootDirString()
{
    QString tmp = QDir::separator();
    return tmp;
}

bool QDir::match( const char * filter, const char *fileName )
{
    QRegExp tmp( filter, TRUE, TRUE ); // case sensitive and wildcard mode on
    return tmp.match( fileName ) != -1;
}


  /*!
  Removes all multiple directory separators ("/" under UNIX) and resolves
  any "." or ".." found in the path. Symbolic links are kept (i.e. this
  function does not return the absoulte path).
  */

QString QDir::cleanPathName( const char *pathName )
{
    QString name = pathName;
    QString newPath;

    if ( name.isEmpty() )
        return name;

    bool addedSeparator;
    if ( name.at( 0 ) != separator() ) {
        addedSeparator = TRUE;
        name.insert( 0, Q_SEPARATOR );
    } else {
        addedSeparator = FALSE;
    }

    int ePos, pos, upLevel;

    pos = ePos = name.size() - 1;
    upLevel = 0;
    int len;

    while( pos && (pos = name.findRev( separator(), --pos)) != -1 ) {
        len = ePos - pos - 1;
        if ( len == 2 && name.at( pos + 1 ) == '.'
                      && name.at( pos + 2 ) == '.' ) {
            upLevel++;
        } else {
            if ( len != 0 && ( len != 1 || name.at( pos + 1 ) != '.' ) ) {
                if ( !upLevel )
                    newPath = Q_SEPARATOR + name.mid( pos + 1, len) + newPath;
                else
                    upLevel--;
            }
        }
        ePos = pos;
    }
    if ( addedSeparator ) {
        while( upLevel-- )
            newPath.insert( 0, Q_SEPARATOR ".." );
        if ( !newPath.isEmpty() )
            newPath.remove( 0, 1 );
        else
            newPath = ".";
    } else {
        if ( newPath.isEmpty() )
            newPath = Q_SEPARATOR;
    }
    return newPath;
}

bool QDir::isRelativePath( const char *pathName )
{
    return ( !pathName || pathName[0] != separator() );   // ### UNIX only!
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
        pos = fl->at() + ( ( sortSpec & QDir::Reversed ) ? 1 : 0 );
    else
        pos = ( sortSpec & QDir::Reversed ) ? 0 : fl->count();
    fl ->insert( pos, fileName );
    fil->insert( pos, newFi );
}

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
    QStrList      *dList  = 0;
    QFileInfoList *diList = 0;
    
    if ( sortSpec & DirsFirst ) {
        dList = new QStrList;
        CHECK_PTR( dList );
        diList   = new QFileInfoList;  // NOT autodelete, will be emptied here!
        CHECK_PTR( dList );
    }

    DIR    *dir;
    dirent *file;

    dir = opendir( dPath );
    if ( !dir ) {
        warning("QDir::readDirEntries: Cannot read the directory:\n\t%s",
                (char*) dPath);
        return FALSE;
    }
    QFileInfo fi;

    QRegExp tmp( nameFilter, TRUE, TRUE ); // case sens. and wildcard mode on
    if ( !nameFilter.isEmpty() && nameFilter[0] == '.' )
        filterSpec |= Hidden;

    while(( file = readdir( dir ) )) {
        fi.setFile( *this, file->d_name );
        if ( tmp.match( file->d_name ) == -1 && ( !allDirs || !fi.isDir() ) )
            continue;
        if  ( ( (filterSpec & Dirs) && fi.isDir() ) ||
              ( (filterSpec & Files) && fi.isFile() ) ) {
            if ( (filterSpec & NoSymLinks) && fi.isSymLink() )
                continue;
            if ( filterSpec & RWEMask != 0 )
                if ( ( filterSpec & Readable   && !fi.isReadable() ) ||
                     ( filterSpec & Writable   && !fi.isWritable() ) ||
                     ( filterSpec & Executable && !fi.isExecutable() ) )
                    continue;
            if ( !(filterSpec & Hidden) && file->d_name[0] == '.' && 
                 ( file->d_name[1] != '\0' ) && 
                 ( file->d_name[1] != '.' || file->d_name[2] != '\0' ) )
                continue;
            if ( (sortSpec & DirsFirst) && fi.isDir() )
                dirInSort( dList, diList , file->d_name, fi, sortSpec );
            else
                dirInSort( fList, fiList, file->d_name, fi, sortSpec );
	}
    }
    if ( closedir( dir ) != 0 )
        warning("QDir::readDirEntries: Cannot close the directory:\n\t%s",
                (char*) dPath);
    if ( sortSpec & DirsFirst ) {
        char      *tmp   = dList ->last();
        QFileInfo *fiTmp = diList->last();
        while( tmp ) {
            fList->insert( 0, tmp );
            tmp = dList->prev();
            fiList->insert( 0, fiTmp );
            fiTmp = diList->prev();
	}
        delete dList;
        delete diList;
    }
    if ( filterSpec == (FilterSpec) filtS &&
         sortSpec   == (SortSpec)   sortS &&
         nameFilter == nameFilt )
        dirty = FALSE;
    else
        dirty = TRUE;
    return TRUE;
}




