/****************************************************************************
**
** Implementation of Win32MakefileGenerator class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of qmake.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "winmakefile.h"
#include "option.h"
#include "project.h"
#include "meta.h"
#include <qtextstream.h>
#include <qstring.h>
#include <qhash.h>
#include <qregexp.h>
#include <qstringlist.h>
#include <qdir.h>
#include <stdlib.h>

Win32MakefileGenerator::Win32MakefileGenerator(QMakeProject *p) : MakefileGenerator(p)
{

}


struct SubDir
{
    QString directory, profile, target, makefile;
};

void
Win32MakefileGenerator::writeSubDirs(QTextStream &t)
{
    QList<SubDir*> subdirs;
    {
	QStringList subdirs_in = project->variables()["SUBDIRS"];
	for(QStringList::Iterator it = subdirs_in.begin(); it != subdirs_in.end(); ++it) {
	    QString file = (*it);
	    file = fileFixify(file);
	    SubDir *sd = new SubDir;
	    subdirs.append(sd);
	    sd->makefile = "$(MAKEFILE)";
	    if((*it).right(4) == ".pro") {
		int slsh = file.lastIndexOf(Option::dir_sep);
		if(slsh != -1) {
		    sd->directory = file.left(slsh+1);
		    sd->profile = file.mid(slsh+1);
		} else {
		    sd->profile = file;
		}
	    } else {
		if(!file.isEmpty() && !project->isActiveConfig("subdir_first_pro"))
		    sd->profile = file.section(Option::dir_sep, -1) + ".pro";
		sd->directory = file;
	    }
	    while(sd->directory.right(1) == Option::dir_sep)
		sd->directory = sd->directory.left(sd->directory.length() - 1);
	    if(!sd->profile.isEmpty()) {
		QString basename = sd->directory;
		int new_slsh = basename.lastIndexOf(Option::dir_sep);
		if(new_slsh != -1)
		    basename = basename.mid(new_slsh+1);
		if(sd->profile != basename + ".pro")
		    sd->makefile += "." + sd->profile.left(sd->profile.length() - 4); //no need for the .pro
	    }
	    sd->target = "sub-" + (*it);
	    sd->target.replace('/', '-');
	    sd->target.replace('.', '_');
	}
    }

    t << "MAKEFILE = " << (project->isEmpty("MAKEFILE") ? QString("Makefile") : var("MAKEFILE")) << endl;
    t << "QMAKE =	" << (project->isEmpty("QMAKE_QMAKE") ? QString("qmake") : var("QMAKE_QMAKE")) << endl;
    t << "SUBTARGETS	= ";
    for(QList<SubDir*>::Iterator it = subdirs.begin(); it != subdirs.end(); ++it)
	t << " \\\n\t\t" << (*it)->target;
    t << endl << endl;
    t << "all: $(MAKEFILE) $(SUBTARGETS)" << endl << endl;

    for(QList<SubDir*>::Iterator it = subdirs.begin(); it != subdirs.end(); ++it) {
	bool have_dir = !(*it)->directory.isEmpty();

	//make the makefile
	QString mkfile = (*it)->makefile;
	if(have_dir)
	    mkfile.prepend((*it)->directory + Option::dir_sep);
	t << mkfile << ":";
	if(have_dir)
	    t << "\n\t" << "cd " << (*it)->directory;
	t << "\n\t" << "$(QMAKE) " << (*it)->profile << " " << buildArgs();
	t << " -o " << (*it)->makefile;
	if(have_dir) {
	    int subLevels = (*it)->directory.count(Option::dir_sep) + 1;
	    t << "\n\t" << "@cd ..";
	    for(int i = 1; i < subLevels; i++)
		t << Option::dir_sep << "..";
	}
	t << endl;

	//now actually build
	t << (*it)->target << ": " << mkfile;
	if(project->variables()["QMAKE_NOFORCE"].isEmpty())
	    t << " FORCE";
	if(have_dir)
	    t << "\n\t" << "cd " << (*it)->directory;
	t << "\n\t" << "$(MAKE)";
        t << " -f " << (*it)->makefile;
	if(have_dir) {
	    int subLevels = (*it)->directory.count(Option::dir_sep) + 1;
	    t << "\n\t" << "@cd ..";
	    for(int i = 1; i < subLevels; i++)
		t << Option::dir_sep << "..";
	}
	t << endl << endl;
    }

    if(project->variables()["QMAKE_INTERNAL_QMAKE_DEPS"].indexOf("qmake_all") == -1)
	project->variables()["QMAKE_INTERNAL_QMAKE_DEPS"].append("qmake_all");
    writeMakeQmake(t);

    t << "qmake_all:";
    if(!subdirs.isEmpty()) {
	for(QList<SubDir*>::Iterator it = subdirs.begin(); it != subdirs.end(); ++it) {
	    QString subdir = (*it)->directory;
	    QString profile = (*it)->profile;
	    int subLevels = subdir.count(Option::dir_sep) + 1;
	    t << "\n\t";
	    if(!subdir.isEmpty())
		t << "cd " << subdir << "\n\t";
	    int lastSlash = subdir.lastIndexOf(Option::dir_sep);
	    if(lastSlash != -1)
		subdir = subdir.mid(lastSlash + 1);
	    t << "$(QMAKE) "
	      << (!profile.isEmpty() ? profile : subdir + ".pro")
	      << " -o " << (*it)->makefile
	      << " " << buildArgs() << "\n\t";
	    if(!subdir.isEmpty())
		t << "@cd ..";
	    for(int i = 1; i < subLevels; i++)
		t << Option::dir_sep << "..";
	}
    } else {
	// Borland make does not like empty an empty command section, so insert
	// a dummy command.
	t << "\n\t" << "@cd .";
    }
    t << endl << endl;

    QStringList targs;
    targs << "clean" << "install_subdirs" << "mocables" << "uicables" << "uiclean" << "mocclean";
    targs += project->values("SUBDIR_TARGETS");
    for(QStringList::Iterator targ_it = targs.begin(); targ_it != targs.end(); ++targ_it) {
        t << (*targ_it) << ": qmake_all";
	QString targ = (*targ_it);
	if(targ == "install_subdirs")
	    targ = "install";
	else if(targ == "uninstall_subdirs")
	    targ = "uninstall";
	if(targ == "clean")
	    t << varGlue("QMAKE_CLEAN","\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ", "");
	if(!subdirs.isEmpty()) {
	    for(QList<SubDir*>::Iterator it = subdirs.begin(); it != subdirs.end(); ++it) {
		int subLevels = (*it)->directory.count(Option::dir_sep) + 1;
		bool have_dir = !(*it)->directory.isEmpty();
		if(have_dir)
		    t << "\n\t" << "cd " << (*it)->directory;
		QString in_file = " -f " + (*it)->makefile;
		t << "\n\t" << "$(MAKE) " << in_file << " " << targ;
		if(have_dir) {
		    t << "\n\t" << "@cd ..";
		    for(int i = 1; i < subLevels; i++)
			t << Option::dir_sep << "..";
		}
	    }
	} else {
	    // Borland make does not like empty an empty command section, so
	    // insert a dummy command.
	    t << "\n\t" << "@cd .";
	}
	t << endl << endl;
    }

    //installations
    project->variables()["INSTALLDEPS"]   += "install_subdirs";
    project->variables()["UNINSTALLDEPS"] += "uninstall_subdirs";
    writeInstalls(t, "INSTALLS");

    // user defined targets
    QStringList &qut = project->variables()["QMAKE_EXTRA_TARGETS"];
    for(QStringList::Iterator sit = qut.begin(); sit != qut.end(); ++sit) {
	QString targ = var((*sit) + ".target"),
		 cmd = var((*sit) + ".commands"), deps;
	if(targ.isEmpty())
	    targ = (*sit);
	QStringList &deplist = project->variables()[(*sit) + ".depends"];
	for(QStringList::Iterator dep_it = deplist.begin(); dep_it != deplist.end(); ++dep_it) {
	    QString dep = var((*dep_it) + ".target");
	    if(dep.isEmpty())
		dep = (*dep_it);
	    deps += " " + dep;
	}
	if(!project->variables()["QMAKE_NOFORCE"].isEmpty() &&
	   project->variables()[(*sit) + ".CONFIG"].indexOf("phony") != -1)
	    deps += QString(" ") + "FORCE";
	t << "\n\n" << targ << ":" << deps << "\n\t"
	  << cmd;
    }

    if(project->variables()["QMAKE_NOFORCE"].isEmpty())
	t << "FORCE:" << endl << endl;
}


int
Win32MakefileGenerator::findHighestVersion(const QString &d, const QString &stem)
{
    QString bd = Option::fixPathToLocalOS(d, TRUE);
    if(!QFile::exists(bd))
	return -1;
    if(!project->variables()["QMAKE_" + stem.toUpper() + "_VERSION_OVERRIDE"].isEmpty())
	return project->variables()["QMAKE_" + stem.toUpper() + "_VERSION_OVERRIDE"].first().toInt();

    QDir dir(bd);
    int biggest=-1;
    QStringList entries = dir.entryList();
    QString dllStem = stem + QTDLL_POSTFIX;
    QRegExp regx("(" + dllStem + "([0-9]*)).lib", QString::CaseInsensitive);
    for(QStringList::Iterator it = entries.begin(); it != entries.end(); ++it) {
	if(regx.exactMatch((*it)))
	    biggest = qMax(biggest, (regx.cap(1) == dllStem ||
				     regx.cap(2).isEmpty()) ? -1 : regx.cap(2).toInt());
    }
    QMakeMetaInfo libinfo;
    if(libinfo.readLib(bd + dllStem)) {
	if(!libinfo.isEmpty("QMAKE_PRL_VERSION"))
	    biggest = qMax(biggest, libinfo.first("QMAKE_PRL_VERSION").replace(".", "").toInt());
    }
    return biggest;
}

QString
Win32MakefileGenerator::findDependency(const QString &dep)
{
    {
	QStringList &qut = project->variables()["QMAKE_EXTRA_TARGETS"];
	for(QStringList::Iterator it = qut.begin(); it != qut.end(); ++it) {
	    QString targ = var((*it) + ".target");
	    if(targ.isEmpty())
		targ = (*it);
	    if(targ.endsWith(dep))
		return targ;
	}
    }
    {
	QStringList &quc = project->variables()["QMAKE_EXTRA_COMPILERS"];
	for(QStringList::Iterator it = quc.begin(); it != quc.end(); ++it) {
	    QString tmp_out = project->variables()[(*it) + ".output"].first();
	    QString tmp_cmd = project->variables()[(*it) + ".commands"].join(" ");
	    if(tmp_out.isEmpty() || tmp_cmd.isEmpty())
		continue;
	    QStringList &tmp = project->variables()[(*it) + ".input"];
	    for(QStringList::Iterator it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
		QStringList &inputs = project->variables()[(*it2)];
		for(QStringList::Iterator input = inputs.begin(); input != inputs.end(); ++input) {
		    QString out = tmp_out;
		    QFileInfo fi(Option::fixPathToLocalOS((*input)));
		    out.replace("${QMAKE_FILE_BASE}", fi.baseName());
		    out.replace("${QMAKE_FILE_NAME}", fi.fileName());
		    if(out.endsWith(dep)) 
			return out;
		}
	    }
	}
    }
    return MakefileGenerator::findDependency(dep);
}

bool
Win32MakefileGenerator::findLibraries(const QString &where)
{
    QStringList &l = project->variables()[where];
    QList<QMakeLocalFileName> dirs;
    {
	QStringList &libpaths = project->variables()["QMAKE_LIBDIR"];
	for(QStringList::Iterator libpathit = libpaths.begin(); libpathit != libpaths.end(); ++libpathit) 
	    dirs.append(QMakeLocalFileName((*libpathit)));
    }
    for(QStringList::Iterator it = l.begin(); it != l.end();) {
	QChar quote;
	bool modified_opt = FALSE, remove = FALSE;
	QString opt = (*it).trimmed();
	if((opt[0] == '\'' || opt[0] == '"') && opt[(int)opt.length()-1] == opt[0]) {
	    quote = opt[0];
	    opt = opt.mid(1, opt.length()-2);
	}
	if(opt.startsWith("/LIBPATH:")) {
            dirs.append(QMakeLocalFileName(opt.mid(9)));
        } else if(opt.startsWith("-L") || opt.startsWith("/L")) {
            dirs.append(QMakeLocalFileName(opt.mid(2)));
            remove = TRUE; //we eat this switch
        } else if(opt.startsWith("-l") || opt.startsWith("/l")) {
            QString lib = opt.right(opt.length() - 2), out;
            if(!lib.isEmpty()) {
		for(QList<QMakeLocalFileName>::Iterator it = dirs.begin(); it != dirs.end(); ++it) {
		    QString extension;
                    int ver = findHighestVersion((*it).local(), lib);
		    if(ver > 0)
			extension += QString::number(ver);
		    extension += ".lib";
		    if(QMakeMetaInfo::libExists((*it).local() + Option::dir_sep + lib) ||
		       QFile::exists((*it).local() + Option::dir_sep + lib + extension)) {
			out = (*it).real() + Option::dir_sep + lib + extension;
			break;
		    }
                }
            }
            if(out.isEmpty()) {
                remove = TRUE; //just eat it since we cannot find one..
            } else {
		modified_opt = TRUE;
                (*it) = out;
	    }
        } else if(!QFile::exists(Option::fixPathToLocalOS(opt))) {
	    QList<QMakeLocalFileName> lib_dirs;
	    QString file = opt;
            int slsh = file.lastIndexOf(Option::dir_sep);
            if(slsh != -1) {
		lib_dirs.append(QMakeLocalFileName(file.left(slsh+1)));
                file = file.right(file.length() - slsh - 1);
            } else {
		lib_dirs = dirs;
	    }
	    if(!project->variables()["QMAKE_QT_DLL"].isEmpty()) {
		if(file.endsWith(".lib")) {
		    file = file.left(file.length() - 4);
		    if(!file.at(file.length()-1).isNumber()) {
			for(QList<QMakeLocalFileName>::Iterator dep_it = lib_dirs.begin(); dep_it != lib_dirs.end(); ++dep_it) {
			    QString lib_tmpl(file + "%1" + ".lib");
			    int ver = findHighestVersion((*dep_it).local(), file);
			    if(ver != -1) {
				if(ver)
				    lib_tmpl = lib_tmpl.arg(ver);
				else
				     lib_tmpl = lib_tmpl.arg("");
				if(slsh != -1) {
				    QString dir = (*dep_it).real();
				    if(!dir.endsWith(Option::dir_sep))
					dir += Option::dir_sep;
				    lib_tmpl.prepend(dir);
				}
				modified_opt = TRUE;
				(*it) = lib_tmpl;
				break;
			    }
			}
		    }
		}
	    }
        }
        if(remove) {
            it = l.erase(it);
        } else {
	    if(!quote.isNull() && modified_opt)
		(*it) = quote + (*it) + quote;
            ++it;
	}
    }
    return TRUE;
}

void
Win32MakefileGenerator::processPrlFiles()
{
    QHash<QString, bool> processed;
    QList<QMakeLocalFileName> libdirs;
    {
	QStringList &libpaths = project->variables()["QMAKE_LIBDIR"];
	for(QStringList::Iterator libpathit = libpaths.begin(); libpathit != libpaths.end(); ++libpathit) 
	    libdirs.append(QMakeLocalFileName((*libpathit)));
    }
    for(bool ret = FALSE; TRUE; ret = FALSE) {
	//read in any prl files included..
	QStringList l_out;
	QString where = "QMAKE_LIBS";
	if(!project->isEmpty("QMAKE_INTERNAL_PRL_LIBS"))
	    where = project->first("QMAKE_INTERNAL_PRL_LIBS");
	QStringList l = project->variables()[where];
	for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
	    QString opt = (*it);
	    if(opt.startsWith("/")) {
		if(opt.startsWith("/LIBPATH:")) 
		    libdirs.append(QMakeLocalFileName(opt.mid(9)));
	    } else {
		if(!processed[opt]) {
		    if(processPrlFile(opt)) {
			processed.insert(opt, true);
			ret = TRUE;
		    } else {
			for(QList<QMakeLocalFileName>::Iterator it = libdirs.begin(); it != libdirs.end(); ++it) {
			    QString prl = (*it).local() + Option::dir_sep + opt;
			    if(processed[prl]) {
				break;
			    } else if(processPrlFile(prl)) {
				processed.insert(prl, true);
				ret = TRUE;
				break;
			    }
			}
		    }
		}
	    }
	    if(!opt.isEmpty())
		l_out.append(opt);
	}
	if(ret)
	    l = l_out;
	else
	    break;
    }
}


void Win32MakefileGenerator::processVars()
{
    project->variables()["QMAKE_ORIG_TARGET"] = project->variables()["TARGET"];
    if ( !project->variables()["QMAKE_INCDIR"].isEmpty()) 
	project->variables()["INCLUDEPATH"] += project->variables()["QMAKE_INCDIR"];
    if ( !project->variables()["VERSION"].isEmpty()) {
	QStringList l = project->first("VERSION").split('.');
	project->variables()["VER_MAJ"].append(l[0]);
	project->variables()["VER_MIN"].append(l[1]);
    }
    fixTargetExt();
    processLibsVar();
    processRcFileVar();
    processExtraWinCompilersVar();
    processFileTagsVar();
    processQtConfig();
    processMocConfig();
    processDllConfig();
}

void Win32MakefileGenerator::processLibsVar()
{
    project->variables()["QMAKE_LIBS"] += project->variables()["LIBS"];
    QStringList &libList = project->variables()["QMAKE_LIBS"];
    for (QStringList::Iterator stIt = libList.begin(); stIt != libList.end() ;) {
	QString s = *stIt;
	if (s.startsWith("-l")) {
	    stIt = libList.erase(stIt);
	    stIt = libList.insert(stIt, s.mid(2) + ".lib");
        } else if (s.startsWith("-L")) {
	    stIt = libList.erase(stIt);
	    project->variables()["QMAKE_LIBDIR"].append(QDir::convertSeparators(s.mid(2)));
	} else {
	    stIt++;
	}
    }
}

void Win32MakefileGenerator::fixTargetExt()
{
    if ( project->isActiveConfig("dll") ) {
	if ( !project->variables()["QMAKE_LIB_FLAG"].isEmpty()) {
	    project->variables()["TARGET_EXT"].append(project->first("VERSION").replace(".", "") + ".dll");
	} else {
	    project->variables()["TARGET_EXT"].append(".dll");
	}
    } else {
	if ( !project->variables()["QMAKE_APP_FLAG"].isEmpty()) {
	    project->variables()["TARGET_EXT"].append(".exe");
	} else {
	    project->variables()["TARGET_EXT"].append(".lib");
	}
    }
}

void Win32MakefileGenerator::processRttiConfig()
{
    if(project->isActiveConfig("rtti")) {
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_RTTI_ON"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_RTTI_ON"];
    } else {
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_RTTI_OFF"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_RTTI_OFF"];
    }
}

void Win32MakefileGenerator::processMocConfig()
{
    if(project->isActiveConfig("moc"))
	setMocAware(TRUE);
}

void Win32MakefileGenerator::processRcFileVar()
{
    bool mingw = (Option::mkfile::qmakespec == "win32-g++");
    if ( !project->variables()["RC_FILE"].isEmpty()) {
	if ( !project->variables()["RES_FILE"].isEmpty()) {
	    fprintf(stderr, "Both .rc and .res file specified.\n");
	    fprintf(stderr, "Please specify one of them, not both.");
	    exit(666);
	}
	project->variables()["RES_FILE"] = project->variables()["RC_FILE"];
	project->variables()["RES_FILE"].first().replace(".rc", mingw ? ".o" : ".res");
	project->variables()["POST_TARGETDEPS"] += project->variables()["RES_FILE"];
	project->variables()["CLEAN_FILES"] += project->variables()["RES_FILE"];
    }
    if(!project->variables()["RES_FILE"].isEmpty())
	project->variables()["QMAKE_LIBS"] += project->variables()["RES_FILE"];
}

void Win32MakefileGenerator::processExtraWinCompilersVar()
{
    QStringList &quc = project->variables()["QMAKE_EXTRA_COMPILERS"];
    for(QStringList::Iterator it = quc.begin(); it != quc.end(); ++it) {
	QString tmp_out = project->variables()[(*it) + ".output"].first();
	if(tmp_out.isEmpty())
	    continue;
	QStringList &tmp = project->variables()[(*it) + ".input"];
	for(QStringList::Iterator it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
	    QStringList &inputs = project->variables()[(*it2)];
	    for(QStringList::Iterator input = inputs.begin(); input != inputs.end(); ++input) {
		QFileInfo fi(Option::fixPathToLocalOS((*input)));
		QString in = Option::fixPathToTargetOS((*input), FALSE),
		    out = tmp_out;
		out.replace("${QMAKE_FILE_BASE}", fi.baseName());
		out.replace("${QMAKE_FILE_NAME}", fi.fileName());
		if(project->variables()[(*it) + ".CONFIG"].indexOf("no_link") == -1)
		    project->variables()["OBJCOMP"] += out;
	    }
	}
    }
}

void Win32MakefileGenerator::processQtConfig()
{
    if (project->isActiveConfig("qt")) {
	if (project->isActiveConfig("target_qt") && !project->variables()["QMAKE_LIB_FLAG"].isEmpty()) {
	} else {
	    if (!project->variables()["QMAKE_QT_DLL"].isEmpty()) {
		int hver = findHighestVersion(project->first("QMAKE_LIBDIR_QT"), "qt");
		if(hver != -1) {
		    QString ver;
		    ver.sprintf("qt" QTDLL_POSTFIX "%d.lib", hver);
		    QStringList &libs = project->variables()["QMAKE_LIBS"];
		    for(QStringList::Iterator libit = libs.begin(); libit != libs.end(); ++libit)
			(*libit).replace(QRegExp("qt\\.lib"), ver);
		}
	    }
	    if (!project->isActiveConfig("dll") && !project->isActiveConfig("plugin")) 
		project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_QT_ENTRY"];
	}
    }
}

void Win32MakefileGenerator::processDllConfig()
{
    if(project->isActiveConfig("dll") || !project->variables()["QMAKE_APP_FLAG"].isEmpty()) {
	project->variables()["CONFIG"].remove("staticlib");
	project->variables()["QMAKE_APP_OR_DLL"].append("1");
    } else {
	project->variables()["CONFIG"].append("staticlib");
    }
}

void Win32MakefileGenerator::processFileTagsVar()
{
    char *filetags[] = { "HEADERS", "SOURCES", "DEF_FILE", "RC_FILE", "TARGET", "QMAKE_LIBS", "DESTDIR", "DLLDESTDIR", "INCLUDEPATH", NULL };
    for(int i = 0; filetags[i]; i++) {
	project->variables()["QMAKE_FILETAGS"] << filetags[i];
	//clean path
	QStringList &gdmf = project->variables()[filetags[i]];
	for(QStringList::Iterator it = gdmf.begin(); it != gdmf.end(); ++it)
	    (*it) = Option::fixPathToTargetOS((*it), FALSE);
    }
}

void Win32MakefileGenerator::writeExtraCompilerParts(QTextStream &t)
{
    QStringList::Iterator it;
    QStringList &quc = project->variables()["QMAKE_EXTRA_COMPILERS"];
    for(it = quc.begin(); it != quc.end(); ++it) {
	QString tmp_out = project->variables()[(*it) + ".output"].first();
	QString tmp_cmd = project->variables()[(*it) + ".commands"].join(" ");
	QString tmp_dep = project->variables()[(*it) + ".depends"].join(" ");
	QStringList &vars = project->variables()[(*it) + ".variables"];
	if(tmp_out.isEmpty() || tmp_cmd.isEmpty())
	    continue;
	QStringList &tmp = project->variables()[(*it) + ".input"];
	for(QStringList::Iterator it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
	    QStringList &inputs = project->variables()[(*it2)];
	    for(QStringList::Iterator input = inputs.begin(); input != inputs.end(); ++input) {
		QFileInfo fi(Option::fixPathToLocalOS((*input)));
		QString in = Option::fixPathToTargetOS((*input), FALSE),
		       out = tmp_out, cmd = tmp_cmd, deps;
		out.replace("${QMAKE_FILE_BASE}", fi.baseName());
		out.replace("${QMAKE_FILE_NAME}", fi.fileName());
		cmd.replace("${QMAKE_FILE_BASE}", fi.baseName());
		cmd.replace("${QMAKE_FILE_OUT}", out);
		cmd.replace("${QMAKE_FILE_NAME}", fi.fileName());
		for(QStringList::Iterator it3 = vars.begin(); it3 != vars.end(); ++it3)
		    cmd.replace("$(" + (*it3) + ")", "$(QMAKE_COMP_" + (*it3)+")");
		if(!tmp_dep.isEmpty()) {
		    char buff[256];
		    QString dep_cmd = tmp_dep;
		    dep_cmd.replace("${QMAKE_FILE_NAME}", fi.fileName());
		    if(FILE *proc = QT_POPEN(dep_cmd.latin1(), "r")) {
			while(!feof(proc)) {
			    int read_in = fread(buff, 1, 255, proc);
			    if(!read_in)
				break;
			    int l = 0;
			    for(int i = 0; i < read_in; i++) {
				if(buff[i] == '\n' || buff[i] == ' ') {
				    deps += " " + QString::fromLatin1(buff+l, (i - l) + 1);
				    l = i;
				}
			    }
			}
			fclose(proc);
		    }
		}
		t << out << ": " << in << deps << "\n\t"
		  << cmd << endl << endl;
	    }
	}
    }
    t << endl;
}

void Win32MakefileGenerator::writeExtraTargetParts(QTextStream &t)
{
    QStringList::Iterator it;
    QStringList &qut = project->variables()["QMAKE_EXTRA_TARGETS"];
    for(it = qut.begin(); it != qut.end(); ++it) {
	QString targ = var((*it) + ".target"),
		 cmd = var((*it) + ".commands"), deps;
	if(targ.isEmpty())
	    targ = (*it);
	QStringList &deplist = project->variables()[(*it) + ".depends"];
	for(QStringList::Iterator dep_it = deplist.begin(); dep_it != deplist.end(); ++dep_it) {
	    QString dep = var((*dep_it) + ".target");
	    if(dep.isEmpty())
		dep = (*dep_it);
	    deps += " " + dep;
	}
	if(!project->variables()["QMAKE_NOFORCE"].isEmpty() &&
	   project->variables()[(*it) + ".CONFIG"].indexOf("phony") != -1)
	    deps += QString(" ") + "FORCE";
	t << "\n\n" << targ << ":" << deps << "\n\t"
	  << cmd;
    }
    t << endl << endl;
}

void Win32MakefileGenerator::writeCleanParts(QTextStream &t)
{
    t << "uiclean:"
      << varGlue("UICDECLS" ,"\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ","")
      << varGlue("UICIMPLS" ,"\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ","") << endl;
    
    t << "mocclean:"
      << varGlue("SRCMOC" ,"\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ","")
      << varGlue("OBJMOC" ,"\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ","") << endl;
    
    t << "clean: uiclean mocclean"
      << varGlue("OBJECTS","\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ","")
      << varGlue("QMAKE_CLEAN","\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ","\n")
      << varGlue("CLEAN_FILES","\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ","\n");

    if(project->isActiveConfig("activeqt")) {
	t << ("\n\t-$(DEL_FILE) " + var("OBJECTS_DIR") + project->variables()["TARGET"].first() + ".idl");
	t << ("\n\t-$(DEL_FILE) " + var("OBJECTS_DIR") + project->variables()["TARGET"].first() + ".tlb");
    }
    
    if(!project->isEmpty("IMAGES"))
	t << varGlue("QMAKE_IMAGE_COLLECTION", "\n\t-$(DEL_FILE) ", "\n\t-$(DEL_FILE) ", "");
    t << endl;

    t << "distclean: clean"
      << "\n\t-$(DEL_FILE) $(TARGET)"
      << endl << endl;
}
