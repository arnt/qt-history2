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
    // Rather simple-minded but should work with an absolute path
    // not containing the equivalent of '/../'
    if(dPath.contains(':')) {
	return false;
    }
    return true;
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
    QFileInfoList * qfl=new QFileInfoList();
    QElemPtr drivep;
    QHdrPtr headerp;
    headerp=GetDrvQHdr();
    drivep=headerp->qHead;
    char somebuf[257];
    int refnum;
    int freebytes;
    while(drivep) {
	DrvQEl * el=(DrvQEl *)drivep;
	drivep=el->qLink;
	int drivenum=el->dQDrive;
	int driveref=el->dQRefNum;
	int driveid=el->dQFSID;
	refnum=driveref;
	OSErr ret=GetVInfo(drivenum,somebuf,&refnum,&freebytes);
	if(ret!=noErr) {
	    if(ret==nsvErr) {
		qWarning("QDir::drives, no such volume");
	    } else {
		qWarning("QDir::drives unknown error");
	    }
	}
	somebuf[somebuf[0]+1]=0;
	QString * widg=new QString(&somebuf[1]);
	qfl->append(widg);
    }
    return 0;
}
