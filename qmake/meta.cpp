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
    if(meta_file.isNull())
	return FALSE;

    if(meta_file.endsWith(".pc")) {
	return FALSE;
    } else if(meta_file.endsWith(".la")) {
	return FALSE;
    } else if(meta_file.endsWith(Option::prl_ext)) {
	QMakeProject proj;
	if(!proj.read(meta_file, QDir::currentDirPath(), QMakeProject::ReadProFile))
	    return FALSE;
	vars = proj.variables();
	return TRUE;
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
    QString extns[] = { Option::prl_ext, QString(".pc"), QString(".la"), QString::null };
    for(int extn = 0; !extns[extn].isNull(); extn++) {
	if(lib.endsWith(extns[extn]))
	    return QFile::exists(lib) ? lib : QString::null;
    }
    if(QFile::exists(lib + Option::prl_ext)) 
	return lib + Option::prl_ext;
    if(QFile::exists(lib + ".la")) 
	return lib + ".la";
    if(QFile::exists(lib + ".pc")) 
	return lib + ".pc";
    return QString::null;
}
