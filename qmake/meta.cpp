#include "meta.h"
#include "project.h"
#include "option.h"
#include <qdir.h>

QMakeMetaInfo::QMakeMetaInfo()
{
    
}


bool
QMakeMetaInfo::readLib(const QString &lib)
{
    clear();
    QString meta_file = findLib(lib);
    if(!meta_file.isNull()) {
	if(meta_file.endsWith(".pc")) {
	    fprintf(stderr, "Must implement reading in pkg-config files (%s)!!!\n", meta_file.latin1());
	} else if(meta_file.endsWith(".la")) {
	    fprintf(stderr, "Must implement reading in libtool files (%s)!!!\n", meta_file.latin1());
	} else if(meta_file.endsWith(Option::prl_ext)) {
	    QMakeProject proj;
	    if(!proj.read(meta_file, QDir::currentDirPath(), QMakeProject::ReadProFile))
		return FALSE;
	    vars = proj.variables();
	    return TRUE;
	} else {
	    warn_msg(WarnLogic, "QMakeMetaInfo: unknown file format for %s", meta_file.latin1());
	}
    }
    return FALSE;
}


void
QMakeMetaInfo::clear()
{
    vars.clear();
}


QString
QMakeMetaInfo::findLib(const QString &lib)
{
    QString ret = QString::null;
    QString extns[] = { Option::prl_ext, QString(".pc"), QString(".la"), QString::null };
    for(int extn = 0; !extns[extn].isNull(); extn++) {
	if(lib.endsWith(extns[extn]))
	    ret = QFile::exists(lib) ? lib : QString::null;
    }
    if(ret.isNull()) {
	if(QFile::exists(lib + Option::prl_ext)) 
	    ret = lib + Option::prl_ext;
	if(QFile::exists(lib + ".la")) 
	    ret = lib + ".la";
	if(QFile::exists(lib + ".pc")) 
	    ret = lib + ".pc";
    }
    if(ret.isNull())
	debug_msg(2, "QMakeMetaInfo: Cannot find info file for %s", lib.latin1());
    else
	debug_msg(2, "QMakeMetaInfo: Found info file %s for %s", ret.latin1(), lib.latin1());
    return ret;
}
