/****************************************************************************
** $Id: //depot/qt/main/src/tools/qfileinfo.cpp#6 $
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
static char ident[] = "$Id: //depot/qt/main/src/tools/qfileinfo.cpp#6 $";
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
  refresh(). If you would rather like a QFileInfo to access the file system
  every time you request information from it, you can call the function
  setCaching( FALSE ).
  
  A QFileInfo can point to a file using either a relative or an absolute
  file path. Absolute file paths begin with the directory separator
  (e.g. '/' under UNIX) or a drive specification (not applicable to UNIX).
  Relative file names begin with a directory name or a file name and specify
  a path relative to the current directory. Under UNIX, an example of
  a relative path is the string "/tmp/quartz", a relative path might look like
  "src/fatlib". You can use the function isRelative() to check if a QFileInfo
  is using a relative or an absolute file path. You can call the function
  convertToAbsolute() to convert a relative QFileInfo to an absolute one.

  If you need to read and traverse directories, see the QDir class.
  */

  /*!
  \fn bool QFileInfo::caching() const

  Returns TRUE if caching is turned on.

  \sa setCaching(), refresh()
  */

  /*!
  Constructs a new empty QFileInfo.
  */
QFileInfo::QFileInfo()
{
    fic   = 0;
    cache = TRUE;
}

  /*!
  Constructs a new QFileInfo that is a copy of \e fi.
  */
QFileInfo::QFileInfo( const QFileInfo &fi )
{
    fn = fi.fn;
    if ( fi.fic ) {
        fic = new QFileInfoCache;
        *fic = *fi.fic;
    } else {
        fic = 0;
    }
    cache = fi.cache;
}

  /*!
  Makes a copy of fi and assigns it to this QFileInfo.
  */
QFileInfo &QFileInfo::operator=( const QFileInfo &fi )
{
    fn = fi.fn;
    if ( !fi.fic ) {
        delete fic;
        fic = 0;
    } else {
        if ( !fic )
            fic = new QFileInfoCache;
        *fic = *fi.fic;    
    }
    cache = fi.cache;
    return *this;
}

  /*!
  Constructs a new QFileInfo that gives information about \e file.
  If the file has a relative path, the QFileInfo will also have one.

  \sa isRelative()
  */
QFileInfo::QFileInfo( const QFile &file )
{
    fn    = file.name();
    fic   = 0;
    cache = TRUE;
}

  /*!
  Constructs a new QFileInfo that gives information about the file
  named \e fileName in the directory \e d.
  If the directory has a relative path, the QFileInfo will also have one.
  */
QFileInfo::QFileInfo( const  QDir &d, const char *fileName )
{
    fn    = d.pathName( fileName );
    fic   = 0;    
    cache = TRUE;
}

  /*!
  Constructs a new QFileInfo that gives information about the given file.
  The string given can be an absolute or a relative file path. 
  \sa bool setFile(const char*), isRelative(),
  QDir::setCurrent, QDir::isRelativePath
  */
QFileInfo::QFileInfo( const char *relativeOrAbsoluteFileName )
{
    fn    = relativeOrAbsoluteFileName;
    fic   = 0;
    cache = TRUE;
}

  /*!
  Destroys the QFileInfo and cleans up.
  */
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
  file system the next time a cached property is fetched.

  \sa setCaching()
  */
void QFileInfo::refresh() const
{
    QFileInfo *This = (QFileInfo*)this;       // Mutable function
    delete This->fic;
    This->fic = 0;
}

  /*!
  Sets caching of file information on or off, the default is on.

  \sa refresh(), caching()
  */
void QFileInfo::setCaching( bool on )
{
    if ( cache == on )
        return;
    cache = on;
    if ( cache ) {
        delete fic;
        fic = 0;
    }
}

  /*!
  Sets the file to get information about.
  If the file has a relative path, the QFileInfo will also have one.

  \sa isRelative()
  */
void QFileInfo::setFile( const QFile &file )
{
    fn  = file.name();
    delete fic;
    fic = 0;
}

  /*!
  Sets the file to get information about to \e fileName in the directory \e d.
  If the directory has a relative path, the QFileInfo will also have one.

  \sa isRelative()
  */
void QFileInfo::setFile( const  QDir &d, const char *fileName )
{
    fn  = d.pathName( fileName );
    delete fic;
    fic = 0;    
}

  /*!
  Sets the file to get information about.
  The string given can be an absolute or a relative file path. Absolute file
  paths begin with the directory separator (e.g. '/' under UNIX) or a drive
  specification (not applicable to UNIX). Relative file names begin with a
  directory name or a file name and specify a path relative to the current
  directory.

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
                              // while fi2 points to "/tmp/liver/aorta"
  }
  \endcode
  \sa isRelative(), QDir::setCurrent, QDir::isRelativePath
  */
void QFileInfo::setFile( const char *relativeOrAbsoluteFileName )
{
    fn = relativeOrAbsoluteFileName;
    delete fic;
    fic = 0;
}

  /*!
  Returns the name, i.e. the file name including the path (which can be
  absolute or relative). 
 
  \sa isRelative(), fullPathName()
  */
const char * QFileInfo::name() const
{
    return fn.data();
}

  /*!
  Returns the name of the file, the file path is NOT included.

  \sa isRelative(), name(), baseName(), extension()
  */
QString QFileInfo::fileName() const
{
    int pos = fn.findRev( QDir::separator() );
    if ( pos == -1 )
        return fn.copy();
    else
        return fn.right( fn.length() - pos - 1 );
}

  /*!
  Returns the full path name, i.e. the file name including the full
  absolute path. If the QFileInfo is absolute (i.e. not relative) this
  function will return the same string as name(). Note that this
  function can be time-consuming under UNIX. (in the order of milliseconds
  on a 486 DX2/66 running Linux).
 
  \sa isRelative(), name()
  */
QString QFileInfo::fullPathName() const
{
    if ( QDir::isRelativePath(fn) ) {
        QString tmp = QDir::currentDirString();
        tmp.detach();
        tmp += QDir::separator();
        tmp += fn;
        return tmp;
    } else {
        return fn.copy();
    }
}

  /*!
  Returns the base name of the file, i.e. all characters in the file name
  up to (but not including) the first '.' character. The path is NOT included.

  \sa fileName(), extension()
  */
QString QFileInfo::baseName() const
{
    QString tmp = fileName();
    int pos = tmp.find( '.' );
    if ( pos == -1 )
        return tmp;
    else
        return tmp.left( pos );
}

  /*!
  Returns the extension name of the file, i.e. all characters in the file name
  after (but not including) the first '.' character.

  For a file named "yo.tar.gz" this function will return "tar.gz".

  \sa fileName(), baseName()
  */
QString QFileInfo::extension() const
{
    QString tmp = fileName();
    int pos = tmp.find( '.' );
    if ( pos == -1 )
        return QString("");
    else
        return tmp.right( tmp.length() - pos - 1 );
}

  /*!
  Returns the name of the directory containing the file, i.e. the path of
  the file (which can be absolute or relative). If fullPath is TRUE an
  absolute path is always returned.

  \sa name(), fileName(), isRelative()
  */
QString QFileInfo::dirName( bool fullPath ) const
{
    QString tmp = fullPath ? fullPathName() : fn;
    int pos = tmp.findRev( QDir::separator() );
    if ( pos == -1 )
        return ".";
    else
        return tmp.left( pos );
}

  /*!
  Returns the directory that contains the file. If the QFileInfo is relative
  and fullPath is FALSE, the QDir will be relative, otherwise it will be
  absolute.

  \sa name(), fileName(), dirName(), isRelative()
  */
QDir QFileInfo::dir( bool fullPath ) const
{
    return QDir( dirName(fullPath) );
}

  /*!
  Returns TRUE if the file is readable.

  \sa isWritable(), isExecutable(), permission()
  */
bool QFileInfo::isReadable() const
{
    if ( !fn.isNull() )
        return ( access( fn.data(), R_OK ) == 0 );
    else
        return FALSE;
}

  /*!
  Returns TRUE if the file is writable.

  \sa isReadable(), isExecutable(), permission()
  */
bool QFileInfo::isWritable() const
{
    if ( !fn.isNull() )
        return ( access( fn.data(), W_OK ) == 0 );
    else
        return FALSE;
}

  /*!
  Returns TRUE if the file is executable.

  \sa isReadable(), isWritable(), permission()
  */
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

  /*!
  Converts the file path name to an absolute path. If it is already
  absolute nothing is done.

  \sa name(), isRelative()
  */
bool QFileInfo::convertToAbsolute()
{
    if ( isRelative )
        fn = fullPathName();
    return QDir::isRelativePath( fn.data() );    
}

  /*!
  Returns TRUE if we are pointing to a file or a symbolic link to a file.

  \sa isDir(), isSymLink()
  */
bool QFileInfo::isFile() const
{
#if defined(_OS_MAC_)
    return FALSE;
#else
    if ( !fic || !cache )
        doStat();
    if ( fic )    
        return ( fic->st.st_mode & STAT_MASK) == STAT_REG;
    else
        return FALSE;
#endif
}

  /*!
  Returns TRUE if we are pointing to a directory or a symbolic link to
  a directory.

  \sa isFile(), isSymLink()
  */
bool QFileInfo::isDir() const
{
#if defined(_OS_MAC_)
    return FALSE;
#else
    if ( !fic || !cache )
        doStat();
    if ( fic )    
        return ( fic->st.st_mode & STAT_MASK) == STAT_DIR;
    else
        return FALSE;
#endif
}

  /*!
  Returns TRUE if we are pointing to a symbolic link.

  \sa isFile(), isDir()
  */
bool QFileInfo::isSymLink() const
{
#if defined(_OS_MAC_)
    return FALSE;
#else
    if ( !fic || !cache )
        doStat();
    if ( fic )    
        return ( fic->st.st_mode & STAT_MASK) == STAT_LNK;
    else
        return FALSE;
#endif
}

  /*!
  Returns the owner of the file. On systems where files do not have owners
  this function returns 0. Note that this function can be time-consuming
  under UNIX. (in the order of milliseconds on a 486 DX2/66
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

  /*!
  Returns the id of the owner of the file. On systems where files do not
  have owners this function returns ((uint) -2).
  */
uint QFileInfo::ownerId() const
{
#if defined(UNIX)
    if ( !fic || !cache )
        doStat();
    if ( fic )    
        return fic->st.st_uid;
#endif
    return nobodyID;
}

  /*!
  Returns the group the file belongs to. On systems where files do not have
  groups this function returns 0. Note that this function can be
  time-consuming under UNIX (in the order of milliseconds on a 486 DX2/66
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

  /*!
  Returns the id of the group the file belongs to. On systems where files do
  not have groups this function returns ((uint) -2).
  */
uint QFileInfo::groupId() const
{
#if defined(UNIX)
    if ( !fic || !cache )
        doStat();
    if ( fic )    
        return fic->st.st_gid;
#endif
    return nobodyID;
}

  /*!
  Tests for file permissions, the \e permissionSpec argument can be several
  flags of type PermissionSpec or-ed together to check for permission
  combinations. On systems where files do not have permissions this function
  always returns TRUE.

  \code
  QFileInfo fi( Q_SEPARATOR "tmp" Q_SEPARATOR "tonsils" );
  if ( fi.permission( QFileInfo::WriteUser | QFileInfo::ReadGroup ) )
      warning( "Tonsils can be changed by me, and the group can read them." );
  if ( fi.permission( QFileInfo::WriteGroup | QFileInfo::WriteOther ) )
      warning( "Danger! Tonsils can be changed by the group or others!" );
  \endcode
  \sa isReadable(), isWritable(), isExecutable()
  */
bool QFileInfo::permission( int permissionSpec ) const
{
#if defined(UNIX)
    if ( !fic || !cache )
        doStat();
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

  /*!
  Returns the file size in bytes, if the file does not exist or the size
  cannot be fetched this function returns -1.
  */
long QFileInfo::size() const
{
    if ( !fic || !cache )
        doStat();
    if ( fic )
        return fic->st.st_size;
    else
        return -1;
}

  /*!
  Returns the time and date when the file was last modified.

  \sa lastRead()
  */
QDateTime QFileInfo::lastModified() const
{
    QDateTime dt;

    if ( !fic || !cache )
        doStat();
    if ( fic )
        dt.setTime_t( fic->st.st_mtime );
    return dt;
}

  /*!
  Returns the time and date when the file was last read (accessed). On systems
  that do not support last read times the modification time is returned.

  \sa lastModified()
  */
QDateTime QFileInfo::lastRead() const
{
    QDateTime dt;

    if ( !fic || !cache )
        doStat();
    if ( fic )
        dt.setTime_t( fic->st.st_atime );
    return dt;
}

void QFileInfo::doStat() const
{
    QFileInfo *This = ((QFileInfo*)this);            // Mutable function
    if ( !fic )
        This->fic = new QFileInfoCache;
    if ( STAT( fn.data(), &This->fic->st ) != 0 ) {
        delete This->fic;
        This->fic = 0;
    }
}




