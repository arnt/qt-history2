/****************************************************************************
** $Id: //depot/qt/main/src/tools/qdir.cpp#1 $
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
#include "qfile.h"
#include "qregexp.h"

#include<stdlib.h>
#include<sys/types.h>
#include<dirent.h>
#include<unistd.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/tools/qdir.cpp#1 $";
#endif

static QString currentDirString()
{
    QString tmp( PATH_MAX );

    if ( getcwd( tmp.data(), PATH_MAX ) )
        return tmp;
    else
        return QString();
}

static QString homeDirString()
{
    QString tmp( PATH_MAX );

    tmp = getenv("HOME");
    if ( tmp.isNull() )
        tmp = QDir::separator();
    return tmp;
}

static QString rootDirString()
{
    QString tmp = QDir::separator();
    return tmp;
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
  QFile f( d.fullName("ex1.txt") );
  if ( f.open(IO_ReadWrite) )
      warning("Cannot create the file %s.", f.name() );
  \endcode
  */

QDir::QDir()
{
    dPath = cleanPathName( homeDirString() );
}

QDir::QDir( const char *pathName )
{
    dPath = cleanPathName( pathName );
}

QDir::QDir( const QDir &d )
{
    dPath = d.dPath;
}

QDir::~QDir()
{
}

  /*!
  Returns the path, this may contain symbolic links, but never contains
  ".", ".." or multiple separators.
   \sa  setPath(), absolutePath(), exists(), cleanPathName(), dirName(),
       fullName()
  */

QString QDir::path() const
{
    return dPath;
}

  /*!
  Returns the absolute path, i.e. a path without symbolic links.
  If the absolute path does not exist a null string is returned.
  
  \sa path(), exists(), cleanPathName(), dirName(), fullName(), 
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
  Sets the path of the directory. The path is cleaned of ".", ".."
  and multiple separators. No check is made to ensure that a directory
  with this path exists.

  \sa path(), absolutePath(), exists, cleanPathName(), dirName(), fullName()
  */
void QDir::setPath( const char *pathName )
{
    dPath = cleanPathName( pathName );
}

  /*!
  Returns the name of the directry, this is NOT the same as the path, e.g.
  a directory with the name "mail", might have the path "/var/spool/mail".
  No check is made to ensure that a directory with this name actually exists.
  \sa path(), absolutePath(), fullName(), exists()
  */

QString QDir::dirName() const
{
    int pos = dPath.findRev( separator() );
    if ( pos == -1  )
        return dPath;
    return dPath.right( dPath.length() - pos - 1 );
}

  /*!
   Returns the full path to a file in the directory. Does NOT check if
   the file actually exists in the directory.
  */

QString QDir::fullName( const char *fileName ) const
{
    QString tmp = dPath.copy();
    tmp += separator();
    tmp += fileName;
    return tmp;
}

  /*!
  Appends, in alphabetical order, the names of files in the directory to the
  QStrList. If filter is 0 (default) all files are appended, if not, only
  the ones matching the filter are appended. If includeSymLinks is TRUE
  (default) symbolic links to files are also appended.

  Returns FALSE if the directory is unreadable or does not exist.  \sa
  readDirs(), readAll(), match(), QRegExp::setWildCard(),
  QFile::isSymLink(), QFile::isRegular()
  */

bool QDir::readFiles( QStrList *l, const char *filter,
                           bool includeSymLinks ) const
{
    return readDirEntries( l, TRUE, FALSE, filter,includeSymLinks );
}

  /*!
   Appends, in alphabetical order,  the names of directories in the
   directory to the QStrList. If filter is 0 (default) all directories
   are appended, 
   if not, only the ones matching the filter are appended. If includeSymLinks
   is TRUE (default) symbolic links to directories are also appended.

   Returns FALSE if the directory is  unreadable or does not exist.
   \sa readFiles(), readAll(),  match(), QRegExp::setWildCard(),
   QFile::isSymLink(), QFile::isDirectory()
  */

bool QDir::readDirs( QStrList *l, const char *filter,
                           bool includeSymLinks ) const
{
    return readDirEntries( l, FALSE, TRUE, filter,includeSymLinks );
}

  /*!
   Appends, in alphabetical order,  the names of files and directories
   in the directory
   to the QStrList. If filter is 0 (default) all files and directories
   are appended, 
   if not, only the ones matching the filter are appended. If includeSymLinks
   is TRUE (default) symbolic links to files and directories are also appended.

   Returns FALSE if the directory is  unreadable or does not exist.
   \sa readDirs(), readFiles(),  match(), QRegExp::setWildCard(),
   QFile::isSymLink(), QFile::isRegular(), QFile::isRegular()
  */

bool QDir::readAll( QStrList *l, const char *filter,
                           bool includeSymLinks ) const
{
    return readDirEntries( l, TRUE, TRUE, filter,includeSymLinks );
}

bool QDir::readDirEntries( QStrList *l, bool files, bool dirs, 
                                const char *filter, bool includeSymLinks) const
{

    CHECK_PTR( l );
    if ( !l || !readable() )
        return FALSE;

    l->clear();

    DIR    *dir;
    dirent *file;

    dir = opendir( dPath );
    if ( !dir ) {
        warning("QDir::readDirEntries: Cannot read the directory:\n%s",
                (char*) dPath);
        return FALSE;
    }
    QFile qFile;
    while(( file = readdir( dir ) )) {
        if ( filter && !match( filter, file->d_name ) )
            continue;
        if ( includeSymLinks && files && dirs ) {
            l->inSort( file->d_name );        
            continue;
        }
        qFile.setFileName( fullName( file->d_name ) );
        if ( !includeSymLinks && qFile.isSymLink() )
            continue;
        if ( dirs && qFile.isDirectory() ) {
            l->inSort( file->d_name );
            continue;
        }
        if ( files && qFile.isRegular() )
            l->inSort( file->d_name );
    }
    if ( !closedir( dir ) )
        warning("QDir::readDirEntries: Cannot close the directory:\n%s",
                (char*) dPath);
    return TRUE;
}


  /*!
  changes directory by descending into the given directory. Returns
  TRUE if the new directory exists and is readable. Note that the logical
  cd operation is performed even if the new directory does not actually
  exist. \code

  QDir d;              // now points to home directory
  if ( !d.cd("c++") )  // now points to "c++" under home directory
      warning("Danger, could not find directory \"c++\".")
  d.cdUp();            // now points to home directory
  if ( !d.cd("news") ) // now points to "news" under home directory
      warning("Danger, could not find directory \"news\".")
  d.cdUp();            // now points to home directory
  \endcode

  \sa cdUp(), readable(), exists(), path()
  */

bool QDir::cd( const char *dirName )
{
    if ( strcmp( dirName, ".." ) == 0 ) {
        cdUp();
        return TRUE;
    }
    if ( strcmp( dirName, "." ) == 0 ) {
        return TRUE;
    }
    QString old = dPath.copy();
    dPath += separator();
    dPath += dirName;
    if ( !exists() ) {
        dPath = old;
        return FALSE;
    }
    return TRUE;
}

bool QDir::cdUp()
{
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

  /*!
  Returns TRUE if the directory is readable AND we can open files by
  name. This function will return FALSE if only one of these is present.
  \warning A FALSE value from this function is not a guarantee that files
  in the directory are not accessible.
  */
bool QDir::readable() const
{
    return access( dPath, R_OK | X_OK ) == 0; // should be in QFile ###
}

  /*!
   Returns TRUE if the directory exists. (If a file with the same 
   name is found this function will of course return FALSE).
  */

bool QDir::exists() const
{
    QFile qFile( dPath );
    return qFile.exists() && qFile.isDirectory();
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
    dPath = d.dPath;
    return *this;    
}

QDir &QDir::operator=( const char *s )
{
    dPath = cleanPathName( s );
    return *this;    
}

bool QDir::operator==( const QDir &d )
{
    return dPath == d.dPath;
}

  /*!
  Returns the directory separator in paths on the current platform, e.g.
  "/" under UNIX and "\" under MS-DOS. 
  */
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

    bool addedSlash;
    if ( name.at( 0 ) != separator() ) {
        addedSlash = TRUE;
        name.insert( 0, Q_SEPARATOR );
    } else {
        addedSlash = FALSE;
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
    if ( addedSlash ) {
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

