#include "qglobal.h"
#include "qdir.h"
#include "qfileinfo.h"
#include "qfiledefs.h"
#include "qregexp.h"
#include "qstringlist.h"

void QDir::slashify(QString& n)
{
  if(n.isNull())
      return;
  for(int i=0;i<(int)n.length();i++) {
      if(n[i]==':') {
	  n[i]='/';
      }
  }
}

QString QDir::canonicalPath() const
{
    return QString();
}

bool QDir::mkdir(const QString &dirname,bool acceptAbsPath) const
{
    return true;
}

bool QDir::rmdir(const QString &dirname,bool acceptAbsPath) const
{
    return true;
}

bool QDir::isReadable() const
{
    return true;
}

bool QDir::isRoot() const
{
    return false;
}

bool QDir::rename(const QString& name,const QString& newName,
		  bool acceptAbsPaths)
{
    return true;
}

bool QDir::setCurrent(const QString& path)
{
    return true;
}

QString QDir::currentDirPath()
{
    return QString();
}

QString QDir::rootDirPath()
{
    return QString();
}

bool QDir::isRelativePath(const QString& path)
{
    return true;
}

bool QDir::readDirEntries(const QString& nameFilter,int filterSpec,
			  int sortSpec)
{
    return true;
}

const QFileInfoList * QDir::drives()
{
    return 0;
}
