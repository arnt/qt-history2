/****************************************************************************
** $Id: //depot/qt/main/src/tools/qfileinf.cpp#3 $
**
** Implementation of QFileInfo class
**
** Author  : Eirik Eng
** Created : 950628
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qfileinf.h"
#include "qfiledef.h"
#include "qdatetm.h"
#include "qdir.h"

#include <pwd.h>
#include <grp.h>


#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/tools/qfileinf.cpp#3 $";
#endif

struct QFileInfoCache 
{
    struct STATBUF st;
};

  /*! \class QFileInfo qfileinf.h

  \brief The QFile class provides system-independent file information.

  \ingroup tools

  QFileInfo provides information about a files name and position (path) in
  the filesystem, its access rights and whether it is a directory or a
  symbolic link. Its size and last modified/read times are also available.

  To speed up performance QFileInfo caches information about the file. Since
  files can be changed by other users or programs, or even by other parts of
  the same program there is a function that refreshes the file information; 
  refresh(). If you would rather like QFileInfo to access the file system
  every time you request information from it, you can call the function
  setCaching( FALSE ).
  
  A QFileInfo can point to a file using either a relative or an absolute
  file path. Absolute file paths begin with the directory separator
  (e.g. '/' under UNIX) or a drive specification (not applicable to UNIX).
  Relative file names begin with a directory name or a file name and specify
  a path relative to the current directory. You can use the function
  isRelative() to check if a QFileInfo is using a relative or an absolute
  file path. You can call the function useAbsolutePath() to fix the file
  pointer to 

  \code
  #include <qdir.h>
  #include <qfileinf.h>

  void test() {
                                  // expands to "/liver/aorta" under UNIX:
      const char *absolute = Q_SEPARATOR "liver" Q_SEPARATOR "aorta";
      const char *relative = "liver" Q_SEPARATOR "aorta";
      QFileInfo fi1( absolute );
      QFileInfo fi2( relative );
  
      QDir::setCurrent( QDir::rootDirString() );
                              // fi1 and fi2 now point to the same file
      QDir::setCurrent( Q_SEPARATOR "tmp" );
                              // fi1 now points to "/liver/aorta",
                              // while fi2 points "/tmp/liver/aorta"
  }
  \endcode

  If you need to read and traverse directories, see QDir.
  */


  /*!
  Constructs a new empty QFileInfo.
  */
QFileInfo::QFileInfo()
{
    fic = 0;
}

  /*!
  Constructs a new QFileInfo that is a copy of \e fi.
  */
QFileInfo::QFileInfo( const QFileInfo &fi )
{
    fn  = fi.fn;
    fic = fi.fic;
}

  /*!
  Constructs a new QFileInfo that gives information about \e file.
  */
QFileInfo::QFileInfo( const QFile &file )
{
    fn  = file.name();
    fic = 0;
}

  /*!
  Constructs a new QFileInfo that  gives information about the file
  named \e fileName in the directory \e d.
  */
QFileInfo::QFileInfo( const  QDir &d, const char *fileName )
{
    fn  = QDir::cleanPathName( d.fullPathName( fileName ) );
    fic = 0;    
}

  /*!
  Constructs a new QFileInfo that gives information about the given file.
  The string given can be an absolute or a relative file path. Absolute file
  paths begin with the directory separator (e.g. '/' under UNIX) or a drive
  specification (not applicable to UNIX). Relative file names begin with a
  directory name or a file name and specify a path relative to the current
  directory. You can use QDir::isRelativePath() to check if a string is a
  relative or an absolute file path.

  \code
  #include <qdir.h>
  #include <qfileinf.h>

  void test() {
                                  // expands to "/liver/aorta" under UNIX:
      const char *absolute = Q_SEPARATOR "liver" Q_SEPARATOR "aorta";
      const char *relative = "liver" Q_SEPARATOR "aorta";
      QFileInfo fi1( absolute );
      QFileInfo fi2( relative );
  
      QDir::setCurrent( QDir::rootDirString() );
                              // fi1 and fi2 now point to the same file
      QDir::setCurrent( Q_SEPARATOR "tmp" );
                              // fi1 now points to "/liver/aorta",
                              // while fi2 points "/tmp/liver/aorta"
  }
  \endcode
  \sa isRelative(), QDir::setCurrent, QDir::isRelativePath
  */
QFileInfo::QFileInfo( const char *relativeOrAbsoluteFileName )
{
    fn  = QDir::cleanPathName( relativeOrAbsoluteFileName );
    fic = 0;
}

QFileInfo::~QFileInfo()
{
    delete fic;
}

  /*!
  Returns TRUE if the file pointed to exists.
  */
bool QFileInfo::exists() const
{
    if ( !fn.isNull() )
        return ( access( fn.data(), F_OK ) == 0 );
    else
        return FALSE;
}

  /*!
  Refresh the information about the file, i.e. read in information from the
  file system.
  */
void QFileInfo::refresh() const
{
    QFileInfo *This = ((QFileInfo*)this);
    if ( !fic )
        This->fic = new QFileInfoCache;
    if ( STAT( fn.data(), &This->fic->st ) != 0 ) {
        delete This->fic;
        This->fic = 0;
    }
}

void QFileInfo::setFile( const QFile &file )
{
    fn  = file.name();
    delete fic;
    fic = 0;
}

void QFileInfo::setFile( const  QDir &d, const char *fileName )
{
    fn  = QDir::cleanPathName( d.fullPathName( fileName ) );
    delete fic;
    fic = 0;    
}

void QFileInfo::setFile( const char *relativeOrAbsoluteFileName )
{
    fn  = QDir::cleanPathName( relativeOrAbsoluteFileName );
    delete fic;
    fic = 0;
}

QString QFileInfo::name() const
{
    return fn;
}

QString QFileInfo::fileName() const
{
    int pos = fn.findRev( QDir::separator() );
    if ( pos == -1 )
        return fn;
    else
        return fn.right( fn.length() - pos - 1 );
}

QString QFileInfo::fullPathName() const
{
    if ( QDir::isRelativePath(fn) ) {
        QString tmp = QDir::currentDirString();
        tmp.detach();
        tmp += QDir::separator();
        tmp += fn;
        return QDir::cleanPathName( tmp.data() );
    } else {
        return fn;
    }
}

QString QFileInfo::baseName() const
{
    QString tmp = fileName();
    int pos = tmp.find( '.' );
    if ( pos == -1 )
        return tmp;
    else
        return tmp.left( pos );
}

QString QFileInfo::extension() const
{
    QString tmp = fileName();
    int pos = tmp.find( '.' );
    if ( pos == -1 )
        return QString("");
    else
        return tmp.right( tmp.length() - pos - 1 );
}

QString QFileInfo::dirName( bool fullPath ) const
{
    QString tmp = fullPath ? fullPathName() : fn;
    int pos = tmp.findRev( QDir::separator() );
    if ( pos == -1 )
        return ".";
    else
        return tmp.left( pos );
}

QDir QFileInfo::dir( bool fullPath ) const
{
    return QDir( dirName(fullPath) );
}

bool QFileInfo::isReadable() const
{
    if ( !fn.isNull() )
        return ( access( fn.data(), R_OK ) == 0 );
    else
        return FALSE;
}

bool QFileInfo::isWritable() const
{
    if ( !fn.isNull() )
        return ( access( fn.data(), W_OK ) == 0 );
    else
        return FALSE;
}

bool QFileInfo::isExecutable() const
{
    if ( !fn.isNull() )
        return ( access( fn.data(), X_OK ) == 0 );
    else
        return FALSE;
}

  /*!
  Returns TRUE if the file path name is relative to the current directory,
  FALSE if the path is absolute (e.g. under UNIX a path is relative if it
  does not start with a '/').

  According to Einstein this function should always return TRUE.
  */
bool QFileInfo::isRelative() const
{
    return QDir::isRelativePath( fn.data() );    
}

bool QFileInfo::isFile() const
{
#if defined(_OS_MAC_)
    return FALSE;
#else
    if ( !fic )
        refresh();
    if ( fic )    
        return ( fic->st.st_mode & STAT_REG) == STAT_REG;
    else
        return FALSE;
#endif
}

bool QFileInfo::isDir() const
{
#if defined(_OS_MAC_)
    return FALSE;
#else
    if ( !fic )
        refresh();
    if ( fic )    
        return ( fic->st.st_mode & STAT_DIR) == STAT_DIR;
    else
        return FALSE;
#endif
}

bool QFileInfo::isSymLink() const
{
#if defined(_OS_MAC_)
    return FALSE;
#else
    if ( !fic )
        refresh();
    if ( fic )    
        return ( fic->st.st_mode & STAT_LNK) == STAT_LNK;
    else
        return FALSE;
#endif
}

  /*!
  Returns the owner of the file. On systems where files do not have owners
  this function returns 0. Note that this function can be time-consuming
  under UNIX. (in the order of milliseconds on a 486 DX266
  running Linux).
  */
const char *QFileInfo::owner() const
{
#if defined(UNIX)
    passwd *tmp = getpwuid( ownerId() );
    return tmp ? tmp->pw_name : 0;
#else
    return 0;
#endif
}

// Advanced programming in the UNIX environment, page 146: 65534
// Slackware: -1 
// Arnt: -2

static const uint nobodyID = (uint) -2;   

uint QFileInfo:: ownerId() const
{
#if defined(UNIX)
    if ( !fic )
        refresh();
    if ( fic )    
        return fic->st.st_uid;
#endif
    return nobodyID;
}

  /*!
  Returns the group the file belongs to. On systems where files do not have
  groups this function returns 0. Note that this function can be
  time-consuming under UNIX (in the order of milliseconds on a 486 DX266
  running Linux).
  */
const char *QFileInfo::group() const
{
#if defined(UNIX)
    struct group *gr = getgrgid( groupId() );
    return gr ? gr->gr_name : 0;
#else
    return 0;
#endif
}

uint QFileInfo:: groupId() const
{
#if defined(UNIX)
    if ( !fic )
        refresh();
    if ( fic )    
        return fic->st.st_gid;
#endif
    return nobodyID;
}

bool QFileInfo::permission( int permissionSpec ) const
{
#if defined(UNIX)
    if ( !fic )
        refresh();
    if ( fic ) {
        ulong mask = 0;
        if ( permissionSpec & ReadUser)
            mask |= S_IRUSR;
        if ( permissionSpec & WriteUser)
            mask |= S_IWUSR;
        if ( permissionSpec & ExeUser)
            mask |= S_IXUSR;
        if ( permissionSpec & ReadGroup)
            mask |= S_IRGRP;
        if ( permissionSpec & WriteGroup)
            mask |= S_IWGRP;
        if ( permissionSpec & ExeGroup)
            mask |= S_IXGRP;
        if ( permissionSpec & ReadOther)
            mask |= S_IROTH;
        if ( permissionSpec & WriteOther)
            mask |= S_IWOTH;
        if ( permissionSpec & ExeOther)
            mask |= S_IXOTH;
	if ( mask ) {
           return ( fic->st.st_mode & mask) == mask;
        } else {
#if defined(CHECK_NULL)
           warning("QFileInfo::permission: permissionSpec is 0.");
#endif
           return TRUE;
	}
    } else {
        return FALSE;
    }
#else
    return TRUE;
#endif
}

long QFileInfo::size() const
{
    if ( !fic )
        refresh();
    if ( fic )
        return fic->st.st_size;
    else
        return -1;
}

QDateTime QFileInfo::lastModified() const
{
    QDateTime dt;

    if ( !fic )
        refresh();
    if ( fic )
        dt.setTime_t( fic->st.st_mtime );
    return dt;
}

QDateTime QFileInfo::lastRead() const
{
    QDateTime dt;

    if ( !fic )
        refresh();
    if ( fic )
        dt.setTime_t( fic->st.st_atime );
    return dt;
}




