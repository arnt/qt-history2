/****************************************************************************
** $Id: //depot/qt/main/src/tools/qdir.cpp#5 $
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
static char ident[] = "$Id: //depot/qt/main/src/tools/qdir.cpp#5 $";
#endif

  /*! \class QDir qdir.h

  \brief The QDir class provides system-independent directory access and
  related functions.

  \ingroup tools

  The QDir class can be used to traverse the directory structure and to
  read the contents of directories in a platform-independent way.

  When a QDir is created with a relative path, it is assumed that the
  starting directory is the home directory, NOT the current directory.
  The directory "example" under the home directory is checked for existence
  in the example below:

  \code
  QDir d("mystuff");      // points to "mystuff" under the home directory
  if ( !d.exists() )
      warning("Cannot find the example directory.");
  \endcode

  If you construct paths manually (not recommended), the static function
  separator() can be used to ensure that your programs will be
  platform-independent:

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

  cd() and cdUp() can be used to navigate the directory tree:

  \code
  QDir d = QDir::root();              // "/" on UNIX
  if ( !d.cd("tmp") )                 // "/tmp" on UNIX
      warning("Cannot find the %s directory.", (char *) d.path());
  if ( !d.cd("example") )             // "/tmp/example" on UNIX
      warning("Cannot find the %s directory.", (char *) d.path());
  QFile f( d, "ex1.txt" );
  if ( f.open(IO_ReadWrite) )
      warning("Cannot create the file %s.", f.name() );
  \endcode
  */ // "

QDir::QDir()
{
    dPath = ".";
    init();
}

QDir::QDir( const char *pathName )
{
    dPath = cleanPathName( pathName );
    init();
}

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

  \sa path(), absolutePath(), exists, cleanPathName(), dirName(), fullPathName()
  */
void QDir::setPath( const char *relativeOrAbsolutePath )
{
    dPath = cleanPathName( relativeOrAbsolutePath );
    dirty = TRUE;
}

  /*!
  \fn  QDir::path()
  Returns the path, this may contain symbolic links, but never contains
  redundant ".", ".." or multiple separators.
   \sa  setPath(), absolutePath(), exists(), cleanPathName(), dirName(),
       fullPathName()
  */

QString QDir::fullPath() const
{
    if ( QDir::isRelativePath(dPath) ) {
        QString tmp = currentDirString();
        tmp += separator();
        tmp += dPath;
        return cleanPathName( tmp.data() );
    } else {
        return dPath;
    }
}

  /*!
  Returns the absolute path, i.e. a path without symbolic links.
  If the absolute path does not exist a null string is returned.
  
  \sa path(), exists(), cleanPathName(), dirName(), fullPathName(), 
      QString::isNull()
  */

QString QDir::absolutePath() const
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
        return QString();
    return dPath.right( dPath.length() - pos - 1 );
}

QString QDir::pathName( const char *fileName,
                        bool acceptAbsolutePath ) const
{
    if ( acceptAbsolutePath && !isRelativePath( fileName ) )
        return cleanPathName( fileName );

    QString tmp = dPath.copy();
    if ( tmp[ tmp.length() - 1 ] != separator() )
        tmp += separator();
    tmp += fileName;
    return cleanPathName( tmp.data() );
}

  /*!
   Returns the full path name of a file in the directory. Does NOT check if
   the file actually exists in the directory.
  */

QString QDir::fullPathName( const char *fileName,
                            bool acceptAbsolutePath ) const
{
    if ( acceptAbsolutePath && !isRelativePath( fileName ) )
        return cleanPathName( fileName );

    QString tmp = fullPath();
    if ( tmp[ tmp.length() - 1 ] != separator() )
        tmp += separator();
    tmp += fileName;
    return cleanPathName( tmp.data() );
}

  /*!
  Changes directory by descending into the given directory. Returns
  TRUE if the new directory exists and is readable. Note that the logical
  cd operation is NOT performed if the new directory does not exist. 

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

   Calling cd( ".." ) will move one directory up the \e absolute path of the
   current directory (see cdUp()). 

  \sa cdUp(), isReadable(), exists(), path()
  */

bool QDir::cd( const char *dirName, bool acceptAbsolutePath )
{
    if ( strcmp( dirName, ".." ) == 0 ) {
        dPath = absolutePath();
        return cdUp();
    }
    if ( strcmp( dirName, "." ) == 0 ) {
        return TRUE;
    }
    QString old = dPath;
    dPath.detach();
    if ( !acceptAbsolutePath || isRelativePath( dirName ) ) {
        dPath += separator();
        dPath += dirName;
    } else {
        dPath = dirName;
    }
    dPath = cleanPathName( dPath.data() );
    if ( !exists() ) {
        dPath = old;
        return FALSE;
    }
    dirty = TRUE;
    return TRUE;
}

  /*!
  Changes directory by moving one directory up the path followed to arrive
  at the current directory. Note that this is NOT always the same as "cd.."
  under UNIX since the directory could have been reached through a symbolic
  link:
  \code
  QDir d1 = QDir::root();
  d1.cd("tmp");
  d1.cd("kidney");   // actually a symbolic link to "/usr/local/kidney"
  QDir d2 = d;
  d1.cdUp();         // d1 now points to "/tmp"
  d2.cd("..");       // d2 now points to "/usr/local"
  \endcode

  Returns TRUE if the new directory exists and is readable. Note that the
  logical cdUp() operation is NOT performed if the new directory does not
  exist. 
  */

bool QDir::cdUp()
{
    QString old = dPath;
    dPath.detach();
    dPath += Q_SEPARATOR "..";
    dPath = cleanPathName( dPath.data() );
    if ( !exists() ) {
        dPath = old;
        return FALSE;
    }
    dirty = TRUE;
    return TRUE;
}

void QDir::setNameFilter( const char *nameFilter )
{
    nameFilt = nameFilter;
    dirty    = TRUE;
}

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

bool QDir::mkdir( const char *dirName, bool acceptAbsolutePath ) const
{
    if ( ::mkdir( pathName( dirName, acceptAbsolutePath ) , 0777 ) == 0 )
        return TRUE;
    else
        return FALSE;
}

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
    return dPath == d.dPath;
}

  /*!
  Sets the the current directory to be this directory. 
  Returns TRUE if successful.
  */
bool QDir::setToCurrent() const
{
    return setCurrent( dPath.data() );
}

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
    return ( pathName && pathName[0] == separator() );   // ### UNIX only!
}

static void dirInSort( QStrList *fl, QFileInfoList *fil, const char *fileName,
                       const QFileInfo &fi, int sortSpec )
{
    QFileInfo *newFi = new QFileInfo( fi );
    CHECK_PTR( newFi );
    int sortBy = sortSpec & QDir::SortByMask;
    if ( sortBy == QDir::Unsorted ) {
        if ( sortSpec & QDir::Reversed ) {
            fl ->insert( fileName );
            fil->insert( newFi );
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
            fList->insert( tmp );
            tmp = dList->prev();
            fiList->insert( fiTmp );
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




