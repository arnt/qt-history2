#include "qdir.h"

static void slashify(QString& n)
{
  if(n.isNull())
    return;
  for(int i=0;i<(int)n.length();i++) {
    if(n[i]==':')
      n[i]='/';
  }
}

QString QDir::canonicalPath() const
{
}

bool QDir::mkdir(const QString &dirname,bool acceptAbsPath) const
{
}

bool QDir::rmdir(const QString &dirname,bool acceptAbsPath) const
{
}

bool QDir::isReadable() const
{
}

bool QDir::isRoot() const
{
}

bool QDir::rename(const QString& name,const QString& newName,
		  bool acceptAbsPaths)
{
}

bool QDir::setCurrent(const QString& path)
{
}

QString QDir::currentDirPath()
{
}

QString QDir::rootDirPath()
{
}

bool QDir::isRelativePath(const QString& path)
{
}

bool QDir::readDirEntries(const QString& nameFilter,int filterSpec,
			  int sortSpec)
{
}

const QFileInfoList * QDir::drives()
{
}
