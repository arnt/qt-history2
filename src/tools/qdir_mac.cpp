#include "qdir.h"
#include "qdir_p.h"
#include "qfileinfo.h"
#include "qfiledefs_p.h"
#include "qregexp.h"
#include "qstringlist.h"
#include "qt_mac.h"

static QString qt_cwd;

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

QString QDir::homeDirPath()
{
    QString d;
    d = QFile::decodeName(getenv("HOME"));
    slashify( d );
    if ( d.isNull() )
	d = rootDirPath();
    return d;
}

QString QDir::canonicalPath() const
{
    return QString();
}

bool QDir::mkdir(const QString &dirname,bool acceptAbsPath) const
{
    FSSpec myspec;
    char bigbuf[257];
    const char * wingle=
           (const char *)QFile::encodeName(filePath(dirname,
						    acceptAbsPath));
    qstrcpy(bigbuf+1,wingle);
    bigbuf[0]=qstrlen(wingle);
    OSErr ret;
    ret=FSMakeFSSpec((short)0,(long)0,(const unsigned char *)bigbuf,&myspec);
    if(ret!=noErr) {
	qWarning("Make FS spec in mkdir error %d",ret);
	return false;
    }
    long int dummy;
    ret=DirCreate(myspec.vRefNum,myspec.parID,myspec.name,&dummy);
    if(ret!=noErr) {
	qWarning("DirCreate error %d",ret);
	return false;
    }
    return true;
}

bool QDir::rmdir(const QString &dirname,bool acceptAbsPath) const
{
    FSSpec myspec;
    char bigbuf[257];
    const char * wingle=
           (const char *)QFile::encodeName(filePath(dirname,
						    acceptAbsPath));
    qstrcpy(bigbuf+1,wingle);
    bigbuf[0]=qstrlen(wingle);
    OSErr ret;
    ret=FSMakeFSSpec((short)0,(long)0,(const unsigned char *)bigbuf,&myspec);
    if(ret!=noErr) {
	qWarning("Make FS spec in rmdir error %d",ret);
	return false;
    }
    ret=HDelete(myspec.vRefNum,myspec.parID,myspec.name);
    if(ret!=noErr) {
	qWarning("Directory delete error %d",ret);
	return false;
    }
    return true;
}

bool QDir::isReadable() const
{
    FSSpec myspec;
    char bigbuf[257];
    const char * wingle=
           (const char *)QFile::encodeName(filePath(dPath,
						    true));
    qstrcpy(bigbuf+1,wingle);
    bigbuf[0]=qstrlen(wingle);
    OSErr ret;
    ret=FSMakeFSSpec((short)0,(long)0,(const unsigned char *)bigbuf,&myspec);
    if(ret!=noErr) {
	qWarning("Make FS spec in QDir::isReadable error %d",ret);
	return false;
    }
    // If we can get to it, it must be readable. MacOS isn't big
    // on file permissions
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
    if ( name.isEmpty() || newName.isEmpty() ) {
#if defined(QT_CHECK_NULL)
        qWarning( "QDir::rename: Empty or null file name(s)" );
#endif
        return FALSE;
    }
    QString fn1 = filePath( name, acceptAbsPaths );
    QString fn2 = filePath( newName, acceptAbsPaths );
    return ::rename(fn1.ascii(), fn2.ascii()) == 0;
}

bool QDir::setCurrent(const QString& path)
{
    qt_cwd=path;
    FSSpec myspec;
    char bigbuf[257];
    const char * wingle=
           (const char *)QFile::encodeName(path);
    qstrcpy(bigbuf+1,wingle);
    bigbuf[0]=qstrlen(wingle);
    OSErr ret;
    ret=FSMakeFSSpec((short)0,(long)0,(const unsigned char *)bigbuf,&myspec);
    if(ret!=noErr) {
	qWarning("Make FS spec in setCurrent error %d",ret);
	return false;
    }
    ret=HSetVol(0,myspec.vRefNum,myspec.parID);
    if(ret!=noErr) {
	qWarning("HSetVol error in setCurrent %d\n",ret);
	return false;
    }
    return true;
}

QString QDir::currentDirPath()
{
    return qt_cwd;
}

QString QDir::rootDirPath()
{
    return QString("");
}

bool QDir::isRelativePath(const QString& path)
{
    if(path[0]==':')
	return true;
    if(path.find(':')>-1)
	return true;
    return false;
}

bool QDir::readDirEntries(const QString& nameFilter,int filterSpec,
			  int sortSpec)
{
    int i;
    if ( !fList ) {
        fList  = new QStringList;
        Q_CHECK_PTR( fList );
        fiList = new QFileInfoList;
        Q_CHECK_PTR( fiList );
        fiList->setAutoDelete( TRUE );
    } else {
        fList->clear();
        fiList->clear();
    }

    QStringList filters = qt_makeFilterList( nameFilter );

    bool doDirs     = (filterSpec & Dirs)       != 0;
    bool doFiles    = (filterSpec & Files)      != 0;
    bool noSymLinks = (filterSpec & NoSymLinks) != 0;
    bool doReadable = (filterSpec & Readable)   != 0;
    bool doWritable = (filterSpec & Writable)   != 0;
    bool doExecable = (filterSpec & Executable) != 0;
    bool doHidden   = (filterSpec & Hidden)     != 0;

    QFileInfo fi;

    OSErr ret;
    FSSpec * matches;
    matches=(FSSpec *)malloc(sizeof(FSSpec)*500);
    CInfoPBRec myspec1;
    CInfoPBRec myspec2;
    short myvrefnum;
    long mydirid;

    FSSpec myspec;
    char bigbuf[257];
    const char * wingle=
           (const char *)QFile::encodeName(filePath(dPath,
						    true));
    qstrcpy(bigbuf+1,wingle);
    bigbuf[0]=qstrlen(wingle);
    ret=FSMakeFSSpec((short)0,(long)0,(const unsigned char *)bigbuf,&myspec);
    if(ret!=noErr) {
	qWarning("Make FS spec in readDirEntries error %d",ret);
	return false;
    }
    myvrefnum=myspec.vRefNum;
    mydirid=myspec.parID;
    char mybuffer[4000];

    unsigned char * myfind=myspec.name;
    
    HParamBlockRec params;

    params.csParam.ioCompletion=0;
    params.csParam.ioNamePtr=0;
    params.csParam.ioVRefNum=myvrefnum;
    params.csParam.ioMatchPtr=(FSSpecArrayPtr)matches;
    params.csParam.ioReqMatchCount=500;
    params.csParam.ioSearchBits=fsSBDrParID;
    params.csParam.ioSearchInfo1=&myspec1;
    params.csParam.ioSearchInfo2=&myspec2;
    params.csParam.ioSearchTime=0;
    params.csParam.ioCatPosition.initialize=0;
    params.csParam.ioOptBuffer=(char *)&mybuffer;
    params.csParam.ioOptBufSize=4000;

    myspec1.dirInfo.ioNamePtr=myfind;
    myspec1.dirInfo.ioFlAttrib=0;
    myspec1.dirInfo.ioDrDirID=mydirid;
    myspec2.dirInfo.ioNamePtr=0;
    myspec2.dirInfo.ioFlAttrib=0x10;
    myspec2.dirInfo.ioDrDirID=mydirid;
    
    OSErr done;
    
    char namebuf[256];
    
    do {
	done=PBCatSearchSync(&params.csParam);
	if(done==noErr) {
	    int loopc;
	    for(loopc=0;loopc<params.csParam.ioActMatchCount;loopc++) {
		unsigned char * thename=matches[loopc].name;
		thename[thename[0]+1]=0;
		QString fn = (const char *)thename+1;
		fi.setFile( *this, fn );
		if ( !match( filters, fn ) && !(allDirs && fi.isDir()) )
		    continue;
		if  ( (doDirs && fi.isDir()) || (doFiles && fi.isFile()) ) {
		    if ( noSymLinks && fi.isSymLink() )
			continue;
		    if ( (filterSpec & RWEMask) != 0 )
			if ( (doReadable && !fi.isReadable()) ||
			     (doWritable && !fi.isWritable()) ||
			     (doExecable && !fi.isExecutable()) )
			    continue;
		    if ( !doHidden && fn[0] == '.' &&
			 fn != QString::fromLatin1(".")
			 && fn != QString::fromLatin1("..") )
			continue;
		    fiList->append( new QFileInfo( fi ) );
		}
	    }
	}
    } while(done==noErr);

        // Sort...
        QDirSortItem* si= new QDirSortItem[fiList->count()];
        QFileInfo* itm;
        i=0;
        for (itm = fiList->first(); itm; itm = fiList->next())
            si[i++].item = itm;
        qt_cmp_si_sortSpec = sortSpec;
        qsort( si, i, sizeof(si[0]), qt_cmp_si );
        // put them back in the list
        fiList->setAutoDelete( FALSE );
        fiList->clear();
        int j;
        for ( j=0; j<i; j++ ) {
            fiList->append( si[j].item );
            fList->append( si[j].item->fileName() );
        }
        delete [] si;
        fiList->setAutoDelete( TRUE );

        if ( filterSpec == (FilterSpec)filtS && sortSpec == (SortSpec)sortS &&
             nameFilter == nameFilt )
            dirty = FALSE;
        else
            dirty = TRUE;
        return TRUE;
}

const QFileInfoList * QDir::drives()
{
    static QFileInfoList * knownMemoryLeak=0;
    if(!knownMemoryLeak) {
		knownMemoryLeak=new QFileInfoList;
        	QElemPtr drivep;
#if 0
		QHdrPtr headerp;
		headerp=GetDrvQHdr();
		drivep=headerp->qHead;
		char somebuf[257];
		short int refnum;
		long int freebytes;
		while(drivep) {
	   		DrvQEl * el=(DrvQEl *)drivep;
	    	drivep=el->qLink;
	    	short int drivenum=el->dQDrive;
	    	int driveref=el->dQRefNum;
	    	int driveid=el->dQFSID;
	    	refnum=driveref;
	    	OSErr ret=GetVInfo(drivenum,(unsigned char *)somebuf,&refnum,
				       &freebytes);
	    	if(ret!=noErr) {
				if(ret==nsvErr) {
		    		qWarning("QDir::drives, no such volume");
				} else {
		    		qWarning("QDir::drives unknown error");
				}
	    	}
	    	somebuf[somebuf[0]+1]=0;
	    	knownMemoryLeak->append( new QFileInfo(
				       QString::fromLatin1 ( somebuf+1 ) ) );
		}
#else
	qDebug("oops..");
#endif
    }
    return knownMemoryLeak;
}
