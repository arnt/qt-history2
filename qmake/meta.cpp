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
	if(meta_file.endsWith(Option::pkgcfg_ext)) {
	    return readPkgCfgFile(meta_file);
	} else if(meta_file.endsWith(Option::libtool_ext)) {
	    return readLibtoolFile(meta_file);
	} else if(meta_file.endsWith(Option::prl_ext)) {
	    QMakeProject proj;
	    if(!proj.read(Option::fixPathToLocalOS(meta_file), 
			  QDir::currentDirPath(), QMakeProject::ReadProFile))
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
    QString extns[] = { Option::prl_ext, Option::pkgcfg_ext, Option::libtool_ext, QString::null };
    for(int extn = 0; !extns[extn].isNull(); extn++) {
	if(lib.endsWith(extns[extn]))
	    ret = QFile::exists(lib) ? lib : QString::null;
    }
    if(ret.isNull()) {
	for(int extn = 0; !extns[extn].isNull(); extn++) {
	    if(QFile::exists(lib + extns[extn])) {
		ret = lib + extns[extn];
		break;
	    }
	}
    }
    if(ret.isNull())
	debug_msg(2, "QMakeMetaInfo: Cannot find info file for %s", lib.latin1());
    else
	debug_msg(2, "QMakeMetaInfo: Found info file %s for %s", ret.latin1(), lib.latin1());
    return ret;
}


bool
QMakeMetaInfo::readLibtoolFile(const QString &f)
{
    /* I can just run the .la through the .pro parser since they are compatible.. */
    QMakeProject proj;
    if(!proj.read(Option::fixPathToLocalOS(f), QDir::currentDirPath(), QMakeProject::ReadProFile))
	return FALSE;
    QString dirf = Option::fixPathToTargetOS(f).section(Option::dir_sep, 0, -1);
    if(dirf == f)
	dirf = "";
    else if(!dirf.isEmpty() && !dirf.endsWith(Option::output_dir))
	dirf += Option::dir_sep;
    QMap<QString, QStringList> &v = proj.variables();
    for(QMap<QString, QStringList>::Iterator it = v.begin(); it != v.end(); ++it) {
	QStringList lst = it.data();
	if(lst.count() == 1 && (lst.first().startsWith("'") || lst.first().startsWith("\"")) &&
	   lst.first().endsWith(QString(lst.first()[0])))
	    lst = lst.first().mid(1, lst.first().length() - 2);
	if(!vars.contains("QMAKE_PRL_TARGET") &&
	   (it.key() == "dlname" || it.key() == "library_names" || it.key() == "old_library")) {
	    QString dir = v["libdir"].first();
	    if(dir.startsWith("'") || dir.startsWith("\"") && dir.endsWith(QString(dir[0])))
		dir = dir.mid(1, dir.length() - 2);
	    if(!dir.isEmpty() && !dir.endsWith(Option::dir_sep))
		dir += Option::dir_sep;
	    if(lst.count() == 1)
		lst = QStringList::split(" ", lst.first());
	    for(QStringList::Iterator lst_it = lst.begin(); lst_it != lst.end(); ++lst_it) {
		if(QFile::exists(Option::fixPathToLocalOS(dir + (*lst_it)))) {
		    vars["QMAKE_PRL_TARGET"] << dir + (*lst_it);
		    break;
		} else if(QFile::exists(Option::fixPathToLocalOS(dirf + (*lst_it)))) {
		    vars["QMAKE_PRL_TARGET"] << dirf + (*lst_it);
		    break;
		} else if(QFile::exists(Option::fixPathToLocalOS(dirf + ".libs" + Option::dir_sep + 
								 (*lst_it)))) {
		    vars["QMAKE_PRL_TARGET"] << dirf + ".libs" + Option::dir_sep + (*lst_it);
		    break;
		}
	    }
	} else if(it.key() == "dependency_libs") {
	    vars["QMAKE_PRL_LIBS"] += lst;
	}
    }
    return TRUE;
}

bool
QMakeMetaInfo::readPkgCfgFile(const QString &f)
{
    fprintf(stderr, "Must implement reading in pkg-config files (%s)!!!\n", f.latin1());
    return FALSE;
}
