/****************************************************************************
** $Id: //depot/qt/main/src/tools/qdir.cpp#3 $
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
#include "qregexp.h"

#include<stdlib.h>
#include<sys/types.h>
#include<dirent.h>
#include<unistd.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/tools/qdir.cpp#3 $";
#endif


static QString rootDirString()
{
    QString tmp = QDir::separator();
    return tmp;
}

static QString homeDirString()
{
    QString tmp( PATH_MAX );

    tmp = getenv("HOME");
    if ( tmp.isNull() )
        tmp = rootDirString();
    return tmp;
}

static QString currentDirString()
{
    QString tmp( PATH_MAX );

    if ( getcwd( tmp.data(), PATH_MAX ) )
        return tmp;
    else
        return QString();
}


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
    dPath = cleanPathName( currentDirString() );
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
    l        = 0;
    nameFilt = d.nameFilt;
    dirty    = TRUE;
    allDirs  = d.allDirs;
    filtS    = d.filtS;
    sortS    = d.sortS;
}

void QDir::init()
{
    l        = 0;
    nameFilt = "*";
    dirty    = TRUE;
    allDirs  = FALSE;
    filtS    = All;
    sortS    = Name;
}

QDir::~QDir()
{
    if ( l )
       delete l;
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
  Returns the path, this may contain symbolic links, but never contains
  ".", ".." or multiple separators.
   \sa  setPath(), absolutePath(), exists(), cleanPathName(), dirName(),
       fullPathName()
  */

QString QDir::path() const
{
    return dPath;
}

  /*!
  Sets the path of the directory. The path is cleaned of ".", ".."
  and multiple separators. No check is made to ensure that a directory
  with this path exists.

  \sa path(), absolutePath(), exists, cleanPathName(), dirName(), fullPathName()
  */
void QDir::setPath( const char *pathName )
{
    dPath = cleanPathName( pathName );
    dirty = TRUE;
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

  /*!
   Returns the full path to a file in the directory. Does NOT check if
   the file actually exists in the directory.
  */

QString QDir::fullPathName( const char *fileName ) const
{
    QString tmp = dPath.copy();
    if ( tmp[ tmp.length() - 1 ] != separator() )
        tmp += separator();
    tmp += fileName;
    return tmp;
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

bool QDir::cd( const char *dirName )
{
    dirty = TRUE;
    if ( strcmp( dirName, ".." ) == 0 ) {
        dPath = absolutePath();
        return cdUp();
    }
    if ( strcmp( dirName, "." ) == 0 ) {
        return TRUE;
    }
    QString old = dPath.copy();
    dPath += separator();
    dPath += dirName;
    if ( !exists() ) {
        dPath = old;
        dirty = FALSE;
        return FALSE;
    }
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
    dirty = TRUE;
    int i = dPath.length();
    while ( i-- && dPath.at(i) != separator() )
        ;
    if ( i == -1 ) {
        dPath = ".";
        return TRUE;
    }
    if ( i == 0 ) {
        dPath = separator();
        return TRUE;
    }
    dPath = dPath.left( i );
    return exists();
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
        return l;
    return entries( nameFilt, filterSpec, sortSpec );
}

const QStrList *QDir::entries( const char *nameFilter,
                               int filterSpec, int sortSpec ) const
{
    if ( filterSpec == (int)DefaultFilter )
        filterSpec = filtS;
    if ( sortSpec == (int)DefaultSort )
        sortSpec = sortS;
    QDir *This = (QDir*) this;         // mutable function
    if ( l )
        delete This->l;
    This->l = readDirEntries( nameFilter, filterSpec, sortSpec );
    This->dirty = FALSE;
    return l;
}

bool QDir::mkdir( const char *dirName )
{
    dirName = dirName;
    return TRUE;
    //###
}

bool QDir::rmdir( const char *dirName )
{
    dirName = dirName;
    return TRUE;
    //###
}

  /*!
  Returns TRUE if the directory is readable AND we can open files by
  name. This function will return FALSE if only one of these is present.
  \warning A FALSE value from this function is not a guarantee that files
  in the directory are not accessible.
  */
bool QDir::isReadable() const
{
    QFileInfo fi( dPath );
    return fi.isReadable();
//    return access( dPath, R_OK | X_OK ) == 0; // should be in QFile ###
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
   Returns TRUE if the directory is the root directory.
  */
bool QDir::isRoot() const
{
    QString tmp = absolutePath();
    return tmp == Q_SEPARATOR;
}

QDir &QDir::operator=( const QDir &d )
{
    dPath    = d.dPath;
    l        = 0;
    nameFilt = d.nameFilt;
    dirty    = TRUE;
    allDirs  = d.allDirs;
    filtS    = d.filtS;
    sortS    = d.sortS;
    return *this;    
}

QDir &QDir::operator=( const char *s )
{
    dPath = cleanPathName( s );
    dirty = TRUE;
    return *this;    
}

bool QDir::operator==( const QDir &d )
{
    return dPath == d.dPath;
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
  Returns the current directory.
  */
QDir QDir::current()
{
    return QDir( currentDirString() );
}

  /*!
  Sets the the current directory to be this directory. 
  Returns TRUE if successful.
  */
void QDir::setToCurrent() const
{
    setCurrent( dPath.data() );
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

typedef declare(QListM,QFileInfo) QFileInfoList;

static void dirInSort( QStrList *l, QFileInfoList *il, const char *fileName,
                       const QFileInfo &fi, int sortSpec )
{
    int sortBy = sortSpec & QDir::SortByMask;
    if ( sortBy == QDir::Unsorted ) {
        if ( sortSpec & QDir::Reversed )
            l->insert( fileName );
        else
            l->append( fileName );
        return;
    }

    char      *tmp1;
    QFileInfo *tmp2;
    tmp1 = ( sortSpec & QDir::Reversed ) ? l->last() : l->first();
    if ( sortBy != QDir::Name )
        tmp2 = ( sortSpec & QDir::Reversed ) ? il->last() : il->first();
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
        tmp1 = ( sortSpec & QDir::Reversed ) ? l->prev() : l->next();
        if ( sortBy != QDir::Name )
            tmp2 = ( sortSpec & QDir::Reversed ) ? il->prev() : il->next();
    }
    int pos;
    if ( stop )
        pos = l->at() + ( ( sortSpec & QDir::Reversed ) ? 1 : 0 );
    else
        pos = ( sortSpec & QDir::Reversed ) ? 0 : l->count();
    l->insert( pos, fileName );
    if ( sortBy != QDir::Name )
        il->insert( pos, new QFileInfo( fi ) );
}

QStrList *QDir::readDirEntries( const QString &nameFilter,
                                int filterSpec, int sortSpec ) const
{
    QStrList *fList = new QStrList;
    QStrList *dList = 0;
    if ( sortSpec & DirsFirst )
        dList = new QStrList;
    QFileInfoList *fil, *dil = 0;
    if ( (sortSpec & SortByMask) != Name && 
         (sortSpec & SortByMask) != Unsorted ) {
        fil = new QFileInfoList;
        fil->setAutoDelete( TRUE );
        if ( sortSpec & DirsFirst ) {
            dil = new QFileInfoList;
            dil->setAutoDelete( TRUE );
	}
    } else {
        fil = 0;
    }
    CHECK_PTR( fList );

    DIR    *dir;
    dirent *file;

    dir = opendir( dPath );
    if ( !dir ) {
        warning("QDir::readDirEntries: Cannot read the directory:\n\t%s",
                (char*) dPath);
        return 0;
    }
    QFileInfo fi;

    QRegExp tmp( nameFilter, TRUE, TRUE ); // case sens. and wildcard mode on
    if ( !nameFilter.isEmpty() && nameFilter[0] == '.' )
        filterSpec |= Hidden;

    while(( file = readdir( dir ) )) {
        fi.setFile( *this, file->d_name );
        if ( tmp.match( file->d_name ) == -1 && ( !allDirs || !fi.isDir() ) )
            continue;
        if  ( ( (filterSpec  & Dirs) && fi.isDir() ) ||
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
                dirInSort( dList, dil, file->d_name, fi, sortSpec );
            else
                dirInSort( fList, fil, file->d_name, fi, sortSpec );
	}
    }
    if ( closedir( dir ) != 0 )
        warning("QDir::readDirEntries: Cannot close the directory:\n\t%s",
                (char*) dPath);
    if ( (sortSpec & SortByMask) != Name && 
         (sortSpec & SortByMask) != Unsorted ) {
        delete fil;
        if ( sortSpec & DirsFirst )
            delete dil;
    }
    if ( sortSpec & DirsFirst ) {
        char *tmp = dList->last();
        while( tmp ) {
            fList->insert( tmp );
            tmp = dList->prev();
	}
        delete dList;
    }
    return fList;
}



