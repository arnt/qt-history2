/****************************************************************************
**
** Implementation of UnixMakefileGenerator class.
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

#include "unixmake.h"
#include "option.h"
#include <qregexp.h>
#include <qfile.h>
#include <qhash.h>
#include <qdir.h>
#include <time.h>


void
UnixMakefileGenerator::init()
{
    if(init_flag)
	return;
    init_flag = TRUE;

    if(!project->isEmpty("QMAKE_FAILED_REQUIREMENTS")) /* no point */
	return;

    QStringList &configs = project->variables()["CONFIG"];
    /* this should probably not be here, but I'm using it to wrap the .t files */
    if(project->first("TEMPLATE") == "app")
	project->variables()["QMAKE_APP_FLAG"].append("1");
    else if(project->first("TEMPLATE") == "lib")
	project->variables()["QMAKE_LIB_FLAG"].append("1");
    else if(project->first("TEMPLATE") == "subdirs") {
	MakefileGenerator::init();
	if(project->isEmpty("MAKEFILE"))
	    project->variables()["MAKEFILE"].append("Makefile");
	if(project->isEmpty("QMAKE"))
	    project->variables()["QMAKE"].append("qmake");
	if(project->variables()["QMAKE_INTERNAL_QMAKE_DEPS"].indexOf("qmake_all") == -1)
	    project->variables()["QMAKE_INTERNAL_QMAKE_DEPS"].append("qmake_all");
	return; /* subdirs is done */
    }

    if(project->isEmpty("QMAKE_EXTENSION_SHLIB")) {
	if(project->isEmpty("QMAKE_CYGWIN_SHLIB")) {
	    project->variables()["QMAKE_EXTENSION_SHLIB"].append("so");
	} else {
	    project->variables()["QMAKE_EXTENSION_SHLIB"].append("dll");
	}
    }
    if(project->isEmpty("QMAKE_CFLAGS_PRECOMPILE"))
	project->variables()["QMAKE_CFLAGS_PRECOMPILE"].append("-x c-header -c");
    if(project->isEmpty("QMAKE_CXXFLAGS_PRECOMPILE"))
	project->variables()["QMAKE_CXXFLAGS_PRECOMPILE"].append("-x c++-header -c");
    if(project->isEmpty("QMAKE_CFLAGS_USE_PRECOMPILE"))
	project->variables()["QMAKE_CFLAGS_USE_PRECOMPILE"].append("-include");
    if(project->isEmpty("QMAKE_EXTENSION_PLUGIN"))
	project->variables()["QMAKE_EXTENSION_PLUGIN"].append(project->first("QMAKE_EXTENSION_SHLIB"));
    if(project->isEmpty("QMAKE_COPY_FILE"))
	project->variables()["QMAKE_COPY_FILE"].append("$(COPY)");
    if(project->isEmpty("QMAKE_COPY_DIR"))
	project->variables()["QMAKE_COPY_DIR"].append("$(COPY) -R");
    if(project->isEmpty("QMAKE_LIBTOOL"))
	project->variables()["QMAKE_LIBTOOL"].append("libtool --silent");
    //If the TARGET looks like a path split it into DESTDIR and the resulting TARGET
    if(!project->isEmpty("TARGET")) {
	QString targ = project->first("TARGET");
	int slsh = qMax(targ.lastIndexOf('/'), targ.lastIndexOf(Option::dir_sep));
	if(slsh != -1) {
	    if(project->isEmpty("DESTDIR"))
		project->values("DESTDIR").append("");
	    else if(project->first("DESTDIR").right(1) != Option::dir_sep)
		project->variables()["DESTDIR"] = project->first("DESTDIR") + Option::dir_sep;
	    project->variables()["DESTDIR"] = project->first("DESTDIR") + targ.left(slsh+1);
	    project->variables()["TARGET"] = targ.mid(slsh+1);
	}
    }

    project->variables()["QMAKE_ORIG_TARGET"] = project->variables()["TARGET"];
    project->variables()["QMAKE_LIBS"] += project->variables()["LIBS"];
    if((!project->isEmpty("QMAKE_LIB_FLAG") && !project->isActiveConfig("staticlib")) ||
	 (project->isActiveConfig("qt") &&  project->isActiveConfig("plugin"))) {
	if(configs.indexOf("dll") == -1) configs.append("dll");
    } else if(!project->isEmpty("QMAKE_APP_FLAG") || project->isActiveConfig("dll")) {
	configs.remove("staticlib");
    }
    if(!project->isEmpty("QMAKE_INCREMENTAL"))
	project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_INCREMENTAL"];
    else if(!project->isEmpty("QMAKE_LFLAGS_PREBIND") &&
	    !project->variables()["QMAKE_LIB_FLAG"].isEmpty() &&
	    project->isActiveConfig("dll"))
	project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_PREBIND"];
    if(!project->isEmpty("QMAKE_INCDIR"))
	project->variables()["INCLUDEPATH"] += project->variables()["QMAKE_INCDIR"];
    if(!project->isEmpty("QMAKE_LIBDIR")) {
	const QStringList &libdirs = project->values("QMAKE_LIBDIR");
	for(QStringList::ConstIterator it = libdirs.begin(); it != libdirs.end(); ++it) {
	    if(!project->isEmpty("QMAKE_RPATH"))
		project->variables()["QMAKE_LFLAGS"] += var("QMAKE_RPATH") + (*it);
	    project->variables()["QMAKE_LIBDIR_FLAGS"] += "-L" + (*it);
	}
    }
    if(project->isActiveConfig("moc"))
	setMocAware(TRUE);
    QString compile_flag = var("QMAKE_COMPILE_FLAG");
    if(compile_flag.isEmpty())
	compile_flag = "-c";
    if(doPrecompiledHeaders() && !project->isEmpty("PRECOMPILED_HEADER")) {
	QString prefix_flags = project->first("QMAKE_CFLAGS_PREFIX_INCLUDE");
	if(prefix_flags.isEmpty())
	    prefix_flags = "-include";
	compile_flag += " " + prefix_flags + " " + project->first("QMAKE_ORIG_TARGET");
    }
    if(!project->isEmpty("ALLMOC_HEADER")) {
	initOutPaths(); 	// Need to fix outdirs since we do this before init() (because we could add to SOURCES et al)
	QString allmoc = fileFixify(project->first("MOC_DIR") + "/allmoc.cpp", QDir::currentDirPath(), Option::output_dir);
	project->variables()["SOURCES"].prepend(allmoc);
	project->variables()["HEADERS_ORIG"] = project->variables()["HEADERS"];
	project->variables()["HEADERS"].clear();
    }
    if(project->isEmpty("QMAKE_RUN_CC"))
	project->variables()["QMAKE_RUN_CC"].append("$(CC) " + compile_flag + " $(CFLAGS) $(INCPATH) -o $obj $src");
    if(project->isEmpty("QMAKE_RUN_CC_IMP"))
	project->variables()["QMAKE_RUN_CC_IMP"].append("$(CC) " + compile_flag + " $(CFLAGS) $(INCPATH) -o $@ $<");
    if(project->isEmpty("QMAKE_RUN_CXX"))
	project->variables()["QMAKE_RUN_CXX"].append("$(CXX) " + compile_flag + " $(CXXFLAGS) $(INCPATH) -o $obj $src");
    if(project->isEmpty("QMAKE_RUN_CXX_IMP"))
	project->variables()["QMAKE_RUN_CXX_IMP"].append("$(CXX) " + compile_flag + " $(CXXFLAGS) $(INCPATH) -o $@ $<");

    char *filetags[] = { "HEADERS", "SOURCES", "TARGET", "DESTDIR", NULL };
    for(int i = 0; filetags[i]; i++)
	project->variables()["QMAKE_FILETAGS"] << filetags[i];

    if(project->isActiveConfig("GNUmake") && !project->isEmpty("QMAKE_CFLAGS_DEPS"))
	include_deps = TRUE; //do not generate deps
    if(project->isActiveConfig("compile_libtool"))
	Option::obj_ext = ".lo"; //override the .o

    MakefileGenerator::init();
    if(project->isActiveConfig("resource_fork") && !project->isActiveConfig("console")) {
	if(!project->isEmpty("QMAKE_APP_FLAG")) {
	    if(project->isEmpty("DESTDIR"))
		project->values("DESTDIR").append("");
	    project->variables()["DESTDIR"].first() += project->variables()["TARGET"].first() +
						       ".app/Contents/MacOS/";
	    project->variables()["QMAKE_PKGINFO"].append(project->first("DESTDIR") + "../PkgInfo");
	    project->variables()["ALL_DEPS"] += project->first("QMAKE_PKGINFO");

	    QString plist = specdir() + QDir::separator() + "Info.plist." +
			    project->first("TEMPLATE");
	    if(QFile::exists(Option::fixPathToLocalOS(plist))) {
		project->variables()["QMAKE_INFO_PLIST"].append(plist);
		project->variables()["QMAKE_INFO_PLIST_OUT"].append(project->first("DESTDIR") +
								    "../Info.plist");
		project->variables()["ALL_DEPS"] += project->first("QMAKE_INFO_PLIST_OUT");
		if(!project->isEmpty("RC_FILE"))
		    project->variables()["ALL_DEPS"] += project->first("DESTDIR") +
							"../Resources/application.icns";
	    }
	}
    }

    if(!project->isEmpty("QMAKE_INTERNAL_INCLUDED_FILES"))
	project->variables()["DISTFILES"] += project->variables()["QMAKE_INTERNAL_INCLUDED_FILES"];
    project->variables()["DISTFILES"] += Option::mkfile::project_files;

    init2();
    project->variables()["QMAKE_INTERNAL_PRL_LIBS"] << "QMAKE_LIBDIR_FLAGS" << "QMAKE_LIBS";
    if(!project->isEmpty("QMAKE_MAX_FILES_PER_AR")) {
	bool ok;
	int max_files = project->first("QMAKE_MAX_FILES_PER_AR").toInt(&ok);
	QStringList ar_sublibs, objs = project->variables()["OBJECTS"] + project->variables()["OBJMOC"];
	if(ok && max_files > 5 && max_files < (int)objs.count()) {
	    int obj_cnt = 0, lib_cnt = 0;
	    QString lib;
	    for(QStringList::Iterator objit = objs.begin(); objit != objs.end(); ++objit) {
		if((++obj_cnt) >= max_files) {
		    if(lib_cnt) {
			lib.sprintf("lib%s-tmp%d.a", project->first("QMAKE_ORIG_TARGET").latin1(), lib_cnt);
			ar_sublibs << lib;
			obj_cnt = 0;
		    }
		    lib_cnt++;
		}
	    }
	}
	if(!ar_sublibs.isEmpty()) {
	    project->variables()["QMAKE_AR_SUBLIBS"] = ar_sublibs;
	    project->variables()["QMAKE_INTERNAL_PRL_LIBS"] << "QMAKE_AR_SUBLIBS";
	}
    }

    if(project->isActiveConfig("compile_libtool")) {
	const QString libtoolify[] = { "QMAKE_RUN_CC", "QMAKE_RUN_CC_IMP",
				       "QMAKE_RUN_CXX", "QMAKE_RUN_CXX_IMP",
				       "QMAKE_LINK_THREAD", "QMAKE_LINK", "QMAKE_AR_CMD", "QMAKE_LINK_SHLIB_CMD",
				       QString::null };
	for(int i = 0; !libtoolify[i].isNull(); i++) {
	    QStringList &l = project->variables()[libtoolify[i]];
	    if(!l.isEmpty()) {
		QString libtool_flags, comp_flags;
		if(libtoolify[i].startsWith("QMAKE_LINK") || libtoolify[i] == "QMAKE_AR_CMD") {
		    libtool_flags += " --mode=link";
		    if(project->isActiveConfig("staticlib")) {
			libtool_flags += " -static";
		    } else {
			if(!project->isEmpty("QMAKE_LIB_FLAG")) {
			    int maj = project->first("VER_MAJ").toInt();
			    int min = project->first("VER_MIN").toInt();
			    int pat = project->first("VER_PAT").toInt();
			    comp_flags += " -version-info " + QString::number(10*maj + min) +
					  ":" + QString::number(pat) + ":0";
			    if(libtoolify[i] != "QMAKE_AR_CMD") {
				QString rpath = Option::output_dir;
				if(!project->isEmpty("DESTDIR")) {
				    rpath = project->first("DESTDIR");
				    if(QDir::isRelativePath(rpath))
					rpath.prepend(Option::output_dir + Option::dir_sep);
				}
				comp_flags += " -rpath " + Option::fixPathToTargetOS(rpath, FALSE);
			    }
			}
		    }
		    if(project->isActiveConfig("plugin"))
			libtool_flags += " -module";
		} else {
		    libtool_flags += " --mode=compile";
		}
		l.first().prepend("$(LIBTOOL)" + libtool_flags + " ");
		if(!comp_flags.isEmpty())
		    l.first() += comp_flags;
	    }
	}
    }
}

QStringList
UnixMakefileGenerator::combineSetLFlags(const QStringList &list1, const QStringList &list2)
{
    if(project->isActiveConfig("no_smart_library_merge"))
	return list1 + list2;

    QStringList ret;
    for(int i = 0; i < 2; i++) {
	const QStringList *lst = i ? &list2 : &list1;
	for(QStringList::ConstIterator it = lst->begin(); it != lst->end(); ++it) {
	    if((*it).startsWith("-")) {
		if((*it).startsWith("-L")) {
		    if(ret.indexOf((*it)) == -1)
			ret.append((*it));
		} else if((*it).startsWith("-l")) {
		    ret.remove(*it);
		    ret.append(*it);
		} else if(project->isActiveConfig("macx") && (*it).startsWith("-framework")) {
		    int as_one = TRUE;
		    QString framework_in;
		    if((*it).length() > 11) {
			framework_in = (*it).mid(11);
		    } else {
			if(it != lst->end()) {
			    ++it;
			    as_one = FALSE;
			    framework_in = (*it);
			}
		    }
		    if(!framework_in.isEmpty()) {
			for(QStringList::Iterator outit = ret.begin(); outit != ret.end(); ++outit) {
			    if((*outit).startsWith("-framework")) {
				int found = 0;
				if((*outit).length() > 11) {
				    if(framework_in == (*outit).mid(11))
					found = 1;
				} else {
				    if(it != lst->end()) {
					++outit;
					if(framework_in == (*outit)) {
					    --outit;
					    found = 2;
					}
				    }
				}
				for(int i = 0; i < found; i++)
				    outit = ret.erase(outit);
			    }
			}
			if(as_one) {
			    ret.append("-framework " + framework_in);
			} else {
			    ret.append("-framework");
			    ret.append(framework_in);
			}
		    }
		} else {
#if 1
		    remove((*it));
#endif
		    ret.append((*it));
		}
	    } else /*if(QFile::exists((*it)))*/ {
		ret.remove(*it);
		ret.append(*it);
	    }
	}
    }
    return ret;
}

void
UnixMakefileGenerator::processPrlVariable(const QString &var, const QStringList &l)
{
    if(var == "QMAKE_PRL_LIBS")
	project->variables()["QMAKE_CURRENT_PRL_LIBS"] = combineSetLFlags(project->variables()["QMAKE_CURRENT_PRL_LIBS"] +
									  project->variables()["QMAKE_LIBS"], l);
    else
	MakefileGenerator::processPrlVariable(var, l);
}

QString
UnixMakefileGenerator::findDependency(const QString &dep)
{
    QStringList::Iterator it;
    {
	QStringList &qut = project->variables()["QMAKE_EXTRA_UNIX_TARGETS"];
	for(it = qut.begin(); it != qut.end(); ++it) {
	    QString targ = var((*it) + ".target");
	    if(targ.isEmpty())
		targ = (*it);
	    if(targ.endsWith(dep))
		return targ;
	}
    }
    {
	QStringList &quc = project->variables()["QMAKE_EXTRA_UNIX_COMPILERS"];
	for(it = quc.begin(); it != quc.end(); ++it) {
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

QStringList
&UnixMakefileGenerator::findDependencies(const QString &file)
{
    QStringList &ret = MakefileGenerator::findDependencies(file);
    if(doPrecompiledHeaders() && !project->isEmpty("PRECOMPILED_HEADER")) {
	QString header_prefix = project->first("QMAKE_ORIG_TARGET") + ".gch" + Option::dir_sep;
	header_prefix += project->first("QMAKE_PRECOMP_PREFIX");
	if(file.endsWith(".c")) {
	    QString precomp_h = header_prefix + "c";
	    if(!ret.contains(precomp_h))
		ret += precomp_h;
	} else {
	    for(QStringList::Iterator it = Option::cpp_ext.begin(); it != Option::cpp_ext.end(); ++it) {
		if(file.endsWith(*it)) {
		    QString precomp_h = header_prefix + "c++";
		    if(!ret.contains(precomp_h))
			ret += precomp_h;
		    break;
		}
	    }
	}
    }
    return ret;
}

bool
UnixMakefileGenerator::findLibraries()
{
    QList<QMakeLocalFileName> libdirs;
    const QString lflags[] = { "QMAKE_LIBDIR_FLAGS", "QMAKE_LIBS", QString::null };
    for(int i = 0; !lflags[i].isNull(); i++) {
	QStringList &l = project->variables()[lflags[i]];
	for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
	    QString stub, dir, extn, opt = (*it).trimmed();
	    if(opt.startsWith("-")) {
		if(opt.startsWith("-L")) {
		    libdirs.append(QMakeLocalFileName(opt.right(opt.length()-2)));
		} else if(Option::target_mode == Option::TARG_MACX_MODE && opt.startsWith("-framework")) {
		    if(opt.length() > 11) {
			opt = opt.mid(11);
		    } else {
			++it;
			opt = (*it);
		    }
		    extn = "";
		    dir = "/System/Library/Frameworks/" + opt + ".framework/";
		    stub = opt;
		}
	    } else {
		extn = dir = "";
		stub = opt;
		int slsh = opt.lastIndexOf(Option::dir_sep);
		if(slsh != -1) {
		    dir = opt.left(slsh);
		    stub = opt.mid(slsh+1);
		}
		QRegExp stub_reg("^.*lib(" + stub + "[^./=]*)\\.(.*)$");
		if(stub_reg.exactMatch(stub)) {
		    stub = stub_reg.cap(1);
		    extn = stub_reg.cap(2);
		}
	    }
	    if(!stub.isEmpty()) {
		bool found = FALSE;
		QStringList extens;
		if(!extn.isNull())
		    extens << extn;
		else
		    extens << project->variables()["QMAKE_EXTENSION_SHLIB"].first() << "a";
		for(QStringList::Iterator extit = extens.begin(); extit != extens.end(); ++extit) {
		    if(dir.isNull()) {
			QString lib_stub;
			for(QList<QMakeLocalFileName>::Iterator dep_it = libdirs.begin(); dep_it != libdirs.end(); ++dep_it) {
			    if(QFile::exists((*dep_it).local() + Option::dir_sep + "lib" + stub +
					     "." + (*extit))) {
				lib_stub = stub;
				break;
			    }
			}
			if(!lib_stub.isNull()) {
			    (*it) = "-l" + lib_stub;
			    found = TRUE;
			    break;
			}
		    } else {
			if(QFile::exists("lib" + stub + "." + (*extit))) {
			    (*it) = "lib" + stub + "." + (*extit);
			    found = TRUE;
			    break;
			}
		    }
		}
		if(!found && project->isActiveConfig("compile_libtool")) {
		    for(QList<QMakeLocalFileName>::Iterator dep_it = libdirs.begin(); dep_it != libdirs.end(); ++dep_it) {
			if(QFile::exists((*dep_it).local() + Option::dir_sep + "lib" + stub + Option::libtool_ext)) {
			    (*it) = (*dep_it).real() + Option::dir_sep + "lib" + stub + Option::libtool_ext;
			    found = TRUE;
			    break;
			}
		    }
		}
		if(found)
		    break;
	    }
	}
    }
    return FALSE;
}

QString linkLib(const QString &file, const QString &libName) {
  QString ret;
  QRegExp reg("^.*lib(" + libName + "[^./=]*).*$");
  if(reg.exactMatch(file))
    ret = "-l" + reg.cap(1);
  return ret;
}

void
UnixMakefileGenerator::processPrlFiles()
{
    QHash<QString, void*> processed;
    QList<QMakeLocalFileName> libdirs;
    const QString lflags[] = { "QMAKE_LIBDIR_FLAGS", "QMAKE_LIBS", QString::null };
    for(int i = 0; !lflags[i].isNull(); i++) {
	for(bool ret = FALSE; TRUE; ret = FALSE) {
	    QStringList l_out;
	    QStringList &l = project->variables()[lflags[i]];
	    for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
		project->variables()["QMAKE_CURRENT_PRL_LIBS"].clear();
		QString opt = (*it).trimmed();
		if(opt.startsWith("-")) {
		    if(opt.startsWith("-L")) {
			libdirs.append(QMakeLocalFileName(opt.right(opt.length()-2)));
		    } else if(opt.startsWith("-l") && !processed[opt]) {
			QString lib = opt.right(opt.length() - 2);
			for(QList<QMakeLocalFileName>::Iterator dep_it = libdirs.begin(); dep_it != libdirs.end(); ++dep_it) {
 			    if(!project->isActiveConfig("compile_libtool")) { //give them the .libs..
 				QString la = (*dep_it).local() + Option::dir_sep + "lib" + lib + Option::libtool_ext;
 				if(QFile::exists(la) && QFile::exists((*dep_it).local() + Option::dir_sep + ".libs")) {
				    QString dot_libs = (*dep_it).real() + Option::dir_sep + ".libs";
 				    l_out.append("-L" + dot_libs);
				    libdirs.append(QMakeLocalFileName(dot_libs));
 				}
 			    }

			    QString prl = (*dep_it).local() + Option::dir_sep + "lib" + lib;
			    if(processPrlFile(prl)) {
				if(prl.startsWith((*dep_it).local()))
				    prl.replace(0, (*dep_it).local().length(), (*dep_it).real());
				opt = linkLib(prl, lib);
				if(!opt.isNull())
				    processed.insertMulti(opt, (void*)1);
				ret = TRUE;
				break;
			    }
			}
		    } else if(Option::target_mode == Option::TARG_MACX_MODE && opt.startsWith("-framework")) {
			if(opt.length() > 11) {
			    opt = opt.mid(11);
			} else {
			    ++it;
			    opt = (*it);
			}
			QString prl = "/System/Library/Frameworks/" + opt +
				      ".framework/" + opt;
			if(processPrlFile(prl))
			    ret = TRUE;
			l_out.append("-framework");
		    }
		    if(!opt.isEmpty())
			l_out.append(opt);
		    l_out = combineSetLFlags(l_out, project->variables()["QMAKE_CURRENT_PRL_LIBS"]);
		} else if(!opt.isNull()) {
		    QString lib = opt;
		    if(!processed[lib] && processPrlFile(lib)) {
		      processed.insertMulti(lib, (void*)1);
		      ret = TRUE;
		    }
#if 0
		    if(ret)
		      opt = linkLib(lib, "");
#endif
		    if(!opt.isEmpty())
		      l_out.append(opt);
		    l_out = combineSetLFlags(l_out, project->variables()["QMAKE_CURRENT_PRL_LIBS"]);
		}
	    }
	    if(ret && l != l_out)
		l = l_out;
	    else
		break;
	}
    }
}

QString
UnixMakefileGenerator::defaultInstall(const QString &t)
{
    if(t != "target" || project->first("TEMPLATE") == "subdirs")
	return QString();

    bool resource = FALSE;
    const QString root = "$(INSTALL_ROOT)";
    QStringList &uninst = project->variables()[t + ".uninstall"];
    QString ret, destdir=project->first("DESTDIR");
    QString targetdir = Option::fixPathToTargetOS(project->first("target.path"), FALSE);
    if(!destdir.isEmpty() && destdir.right(1) != Option::dir_sep)
	destdir += Option::dir_sep;
    targetdir = fileFixify(targetdir);
    if(targetdir.right(1) != Option::dir_sep)
	targetdir += Option::dir_sep;

    QStringList links;
    QString target="$(TARGET)";
    if(project->first("TEMPLATE") == "app") {
	target = "$(QMAKE_TARGET)";
	if(project->isActiveConfig("resource_fork") && !project->isActiveConfig("console")) {
	    destdir += "../../../";
	    target += ".app";
	    resource = TRUE;
	}
    } else if(project->first("TEMPLATE") == "lib") {
	if(project->isActiveConfig("create_prl") && !project->isActiveConfig("no_install_prl") &&
	   !project->isEmpty("QMAKE_INTERNAL_PRL_FILE")) {
	    QString dst_prl = project->first("QMAKE_INTERNAL_PRL_FILE");
	    int slsh = dst_prl.lastIndexOf('/');
	    if(slsh != -1)
		dst_prl = dst_prl.right(dst_prl.length() - slsh - 1);
	    dst_prl = root + targetdir + dst_prl;
	    ret += "-$(COPY) \"" + project->first("QMAKE_INTERNAL_PRL_FILE") + "\" \"" + dst_prl + "\"";
	    if(!uninst.isEmpty())
		uninst.append("\n\t");
	    uninst.append("-$(DEL_FILE) \"" + dst_prl + "\"");
	}
	if(project->isActiveConfig("create_libtool") && !project->isActiveConfig("compile_libtool")) {
	    QString src_lt = var("QMAKE_ORIG_TARGET");
	    int slsh = src_lt.lastIndexOf(Option::dir_sep);
	    if(slsh != -1)
		src_lt = src_lt.right(src_lt.length() - slsh);
	    int dot = src_lt.indexOf('.');
	    if(dot != -1)
		src_lt = src_lt.left(dot);
	    src_lt += Option::libtool_ext;
	    src_lt.prepend("lib");
	    QString dst_lt = root + targetdir + src_lt;
	    if(!project->isEmpty("DESTDIR")) {
		src_lt.prepend(var("DESTDIR"));
		src_lt = Option::fixPathToLocalOS(fileFixify(src_lt,
							     QDir::currentDirPath(), Option::output_dir));
	    }
	    if(!ret.isEmpty())
		ret += "\n\t";
	    ret += "-$(COPY) \"" + src_lt + "\" \"" + dst_lt + "\"";
	    if(!uninst.isEmpty())
		uninst.append("\n\t");
	    uninst.append("-$(DEL_FILE) \"" + dst_lt + "\"");
	}
	if(project->isActiveConfig("create_pc")) {
	    QString src_pc = var("QMAKE_ORIG_TARGET");
	    int slsh = src_pc.lastIndexOf(Option::dir_sep);
	    if(slsh != -1)
		src_pc = src_pc.right(src_pc.length() - slsh);
	    int dot = src_pc.indexOf('.');
	    if(dot != -1)
		src_pc = src_pc.left(dot);
	    src_pc += ".pc";
	    QString d = root + targetdir + "pkgconfig" + Option::dir_sep;
	    QString dst_pc = d + src_pc;
	    if(!project->isEmpty("DESTDIR")) {
		src_pc.prepend(var("DESTDIR"));
		src_pc = Option::fixPathToLocalOS(fileFixify(src_pc,
							     QDir::currentDirPath(), Option::output_dir));
	    }
	    if(!ret.isEmpty())
		ret += "\n\t";
	    ret += mkdir_p_asstring(d) + "\n\t";
	    ret += "-$(COPY) \"" + src_pc + "\" \"" + dst_pc + "\"";
	    if(!uninst.isEmpty())
		uninst.append("\n\t");
	    uninst.append("-$(DEL_FILE) \"" + dst_pc + "\"");
	}
	if(project->isEmpty("QMAKE_CYGWIN_SHLIB")) {
	    if(!project->isActiveConfig("staticlib") && !project->isActiveConfig("plugin")) {
		if(project->isEmpty("QMAKE_HPUX_SHLIB")) {
		    links << "$(TARGET0)" << "$(TARGET1)" << "$(TARGET2)";
		} else {
		    links << "$(TARGET0)";
	        }
	    }
	}
    }

    if(!resource && project->isActiveConfig("compile_libtool")) {
	QString src_targ = target;
	if(src_targ == "$(TARGET)")
	    src_targ = "$(TARGETL)";
	QString dst_dir = fileFixify(targetdir);
	if(QDir::isRelativePath(dst_dir))
	    dst_dir = Option::fixPathToTargetOS(Option::output_dir + Option::dir_sep + dst_dir);
	ret = "-$(LIBTOOL) --mode=install cp \"" + src_targ + "\" \"" + root + dst_dir + "\"";
	uninst.append("-$(LIBTOOL) --mode=uninstall \"" + src_targ + "\"");
    } else {
	QString src_targ = target;
	if(!destdir.isEmpty())
	    src_targ = Option::fixPathToTargetOS(destdir + target, FALSE);
	QString dst_targ = root + fileFixify(targetdir + target);
	if(!ret.isEmpty())
	    ret += "\n\t";
	if(resource)
	    ret += "$(DEL_FILE) -r \"" + dst_targ + "\"" + "\n\t";
	if(!ret.isEmpty())
	    ret += "\n\t";
	ret += QString(resource ? "-$(COPY_DIR)" : "-$(COPY)") + " \"" +
	       src_targ + "\" \"" + dst_targ + "\"";
	if(!project->isActiveConfig("debug") && !project->isEmpty("QMAKE_STRIP") &&
	   (project->first("TEMPLATE") != "lib" || !project->isActiveConfig("staticlib"))) {
	    ret += "\n\t-" + var("QMAKE_STRIP");
	    if(project->first("TEMPLATE") == "lib" && !project->isEmpty("QMAKE_STRIPFLAGS_LIB"))
		ret += " " + var("QMAKE_STRIPFLAGS_LIB");
	    else if(project->first("TEMPLATE") == "app" && !project->isEmpty("QMAKE_STRIPFLAGS_APP"))
		ret += " " + var("QMAKE_STRIPFLAGS_APP");
	    if(resource)
		ret = " \"" + dst_targ + "/Contents/MacOS/$(QMAKE_TARGET)\"";
	    else
		ret += " \"" + dst_targ + "\"";
	}
	if(!uninst.isEmpty())
	    uninst.append("\n\t");
	if(resource)
	    uninst.append("-$(DEL_FILE) -r \"" + dst_targ + "\"");
	else
	    uninst.append("-$(DEL_FILE) \"" + dst_targ + "\"");
	if(!links.isEmpty()) {
	    for(QStringList::Iterator it = links.begin(); it != links.end(); it++) {
		if(Option::target_mode == Option::TARG_WIN_MODE ||
		   Option::target_mode == Option::TARG_MAC9_MODE) {
		} else if(Option::target_mode == Option::TARG_UNIX_MODE ||
			  Option::target_mode == Option::TARG_MACX_MODE) {
		    QString link = Option::fixPathToTargetOS(destdir + (*it), FALSE);
		    int lslash = link.lastIndexOf(Option::dir_sep);
		    if(lslash != -1)
			link = link.right(link.length() - (lslash + 1));
		    QString dst_link = root + fileFixify(targetdir + link);
		    ret += "\n\t-$(SYMLINK) \"$(TARGET)\" \"" + dst_link + "\"";
		    if(!uninst.isEmpty())
			uninst.append("\n\t");
		    uninst.append("-$(DEL_FILE) \"" + dst_link + "\"");
		}
	    }
	}
    }
    return ret;
}
