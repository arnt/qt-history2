/****************************************************************************
** $Id: $
**
** Definition of ________ class.
**
** Created : 970521
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "pbuilder_pbx.h"
#include "option.h"
#include <qdir.h>
#include <qdict.h>
#include <qregexp.h>
#include <stdlib.h>
#include <time.h>
#ifdef Q_OS_UNIX
#  include <sys/types.h>
#  include <sys/stat.h>
#endif

// Note: this is fairly hacky, but it does the job...


ProjectBuilderMakefileGenerator::ProjectBuilderMakefileGenerator(QMakeProject *p) : UnixMakefileGenerator(p)
{

}

bool
ProjectBuilderMakefileGenerator::writeMakefile(QTextStream &t)
{
    if(!project->variables()["QMAKE_FAILED_REQUIREMENTS"].isEmpty()) {
	/* for now just dump, I need to generated an empty xml or something.. */
	fprintf(stderr, "Project file not generated because all requirements not met:\n\t%s\n",
		var("QMAKE_FAILED_REQUIREMENTS").latin1());
	return TRUE;
    }

    project->variables()["MAKEFILE"].clear();
    project->variables()["MAKEFILE"].append("Makefile");
    if(project->first("TEMPLATE") == "app" || project->first("TEMPLATE") == "lib") {
	return writeMakeParts(t);
    } else if(project->first("TEMPLATE") == "subdirs") { 
	writeSubdirs(t, FALSE);
	return TRUE;
    }
    return FALSE;
}

bool
ProjectBuilderMakefileGenerator::writeMakeParts(QTextStream &t)
{
    QStringList tmp;
    int i;
    
    //HEADER
    t << "// !$*UTF8*$!" << "\n"
      << "{" << "\n"
      << "\t" << "archiveVersion = 1;" << "\n"
      << "\t" << "classes = {" << "\n" << "\t" << "};" << "\n"
      << "\t" << "objectVersion = 34;" << "\n"
      << "\t" << "objects = {" << endl;

    //MAKE QMAKE equivlant
    if(!project->isActiveConfig("no_autoqmake") && project->projectFile() != "(stdin)") {
	QString mkfile = Option::output_dir + Option::dir_sep + "qt_makeqmake.mak";
	QFile mkf(mkfile);
	if(mkf.open(IO_WriteOnly | IO_Translate)) {
	    debug_msg(0, "Creating file: %s", mkfile.latin1());
	    QTextStream mkt(&mkf);
	    writeHeader(mkt);
	    mkt << "QMAKE    = "	<<
		(project->isEmpty("QMAKE_QMAKE") ? QString("$(QTDIR)/bin/qmake") :
		 var("QMAKE_QMAKE")) << endl;
	    writeMakeQmake(mkt);
	    mkf.close();
	}
	QString phase_key = keyFor("QMAKE_PBX_MAKEQMAKE_BUILDPHASE");
	fileFixify(mkfile, QDir::currentDirPath());
	project->variables()["QMAKE_PBX_BUILDPHASES"].append(phase_key);
	t << "\t\t" << phase_key << " = {" << "\n"
	  << "\t\t\t" << "buildActionMask = 2147483647;" << "\n"
	  << "\t\t\t" << "files = (" << "\n"
	  << "\t\t\t" << ");" << "\n"
	  << "\t\t\t" << "generatedFileNames = (" << "\n"
	  << "\t\t\t" << ");" << "\n"
	  << "\t\t\t" << "isa = PBXShellScriptBuildPhase;" << "\n"
	  << "\t\t\t" << "name = \"Qt Qmake\";" << "\n"
	  << "\t\t\t" << "neededFileNames = (" << "\n"
	  << "\t\t\t" << ");" << "\n"
	  << "\t\t\t" << "shellPath = /bin/sh;" << "\n"
	  << "\t\t\t" << "shellScript = \"make -C " << QDir::currentDirPath() <<
	    " -f " << mkfile << "\";" << "\n"
	  << "\t\t" << "};" << "\n";
    }

    //DUMP SOURCES
    QString srcs[] = { "SOURCES", "SRCMOC", "UICIMPLS", QString::null };
    for(i = 0; !srcs[i].isNull(); i++) {
	tmp = project->variables()[srcs[i]];
	for(QStringList::Iterator it = tmp.begin(); it != tmp.end(); ++it) {
	    QString file = (*it);
	    if(file.right(Option::moc_ext.length()) == Option::moc_ext) {
		continue;
	    } else if(!project->isEmpty("YACCIMPLS")) {
		QStringList yaccs = project->variables()["YACCIMPLS"];
		for(QStringList::Iterator yit = yaccs.begin(); yit != yaccs.end(); yit++) {
		    if((*yit) == (*it)) {
			file = project->first("YACCSOURCES");
			break;
		    }
		}
	    } else if(!project->isEmpty("LEXIMPLS") && !project->isActiveConfig("lex_included")) {
		QStringList lexs = project->variables()["LEXIMPLS"];
		for(QStringList::Iterator lit = lexs.begin(); lit != lexs.end(); lit++) {
		    if((*lit) == (*it)) {
			file = project->first("LEXSOURCES");
			break;
		    }
		}
	    }
	    fileFixify(file);
	    QString key = keyFor(file);
	    project->variables()["QMAKE_PBX_" + srcs[i]].append(key);
	    //source reference
	    t << "\t\t" << key << " = {" << "\n"
	      << "\t\t\t" << "isa = PBXFileReference;" << "\n"
	      << "\t\t\t" << "path = \"" << file << "\";" << "\n"
	      << "\t\t\t" << "refType = 4;" << "\n"
	      << "\t\t" << "};" << "\n";
	    //build reference
	    QString obj_key = file + ".o";
	    obj_key = keyFor(obj_key);
	    t << "\t\t" << obj_key << " = {" << "\n"
	      << "\t\t\t" << "fileRef = " << key << ";" << "\n"
	      << "\t\t\t" << "isa = PBXBuildFile;" << "\n"
	      << "\t\t\t" << "settings = {" << "\n"
	      << "\t\t\t\t" << "ATTRIBUTES = (" << "\n"
	      << "\t\t\t\t" << ");" << "\n"
	      << "\t\t\t" << "};" << "\n"
	      << "\t\t" << "};" << "\n";
	    project->variables()["QMAKE_PBX_OBJ"].append(obj_key);
	}
	tmp = project->variables()["QMAKE_PBX_" + srcs[i]];
	if(!tmp.isEmpty()) {
	    QString grp;
	    if(srcs[i] == "SOURCES")
		grp = "Sources";
	    else if(srcs[i] == "SRCMOC")
		grp = "Mocables";
	    else if(srcs[i] == "UICIMPLS")
		grp = "UICables";
	    QString grp_key = keyFor(grp);
	    project->variables()["QMAKE_PBX_GROUPS"].append(grp_key);
	    t << "\t\t" << grp_key << " = {" << "\n"
	      << "\t\t\t" << "children = (" << "\n"
	      << varGlue("QMAKE_PBX_" + srcs[i], "\t\t\t\t", ",\n\t\t\t\t", "\n")
	      << "\t\t\t" << ");" << "\n"
	      << "\t\t\t" << "isa = PBXGroup;" << "\n"
	      << "\t\t\t" << "name = " << grp << ";" << "\n"
	      << "\t\t\t" << "refType = 4;" << "\n"
	      << "\t\t" << "};" << "\n";
	}
    }
    //PREPROCESS BUILDPHASE (just a makefile)
    if(!project->isEmpty("UICIMPLS") || !project->isEmpty("SRCMOC")) {
	QString mkfile = Option::output_dir + Option::dir_sep + "qt_preprocess.mak";
	QFile mkf(mkfile);
	if(mkf.open(IO_WriteOnly | IO_Translate)) {
	    debug_msg(0, "Creating file: %s", mkfile.latin1());
	    QTextStream mkt(&mkf);
	    writeHeader(mkt);
	    mkt << "MOC = " << var("QMAKE_MOC") << endl;
	    mkt << "UIC = " << var("QMAKE_UIC") << endl << endl;
	    mkt << "FORMS = " << varList("UICIMPLS") << endl;
	    mkt << "MOCS = " << varList("SRCMOC") << endl;
	    mkt << "preprocess: $(FORMS) $(MOCS)" << endl << endl;
	    writeUicSrc(mkt, "FORMS");
	    writeMocSrc(mkt, "HEADERS");
	    writeMocSrc(mkt, "SOURCES");
	    writeMocSrc(mkt, "UICDECLS");
	    mkf.close();
	}
	QString phase_key = keyFor("QMAKE_PBX_PREPROCESST_BUILDPHASE");
	fileFixify(mkfile, QDir::currentDirPath());
	project->variables()["QMAKE_PBX_BUILDPHASES"].append(phase_key);
	t << "\t\t" << phase_key << " = {" << "\n"
	  << "\t\t\t" << "buildActionMask = 2147483647;" << "\n"
	  << "\t\t\t" << "files = (" << "\n"
	  << "\t\t\t" << ");" << "\n"
	  << "\t\t\t" << "generatedFileNames = (" << "\n"
//	  << varGlue("QMAKE_PBX_UICIMPLS", "\t\t\t\t", ",\n\t\t\t\t", "")
//	  << varGlue("QMAKE_PBX_SRCMOC", ",\n\t\t\t\t", ",\n\t\t\t\t", "\n")
	  << "\t\t\t" << ");" << "\n"
	  << "\t\t\t" << "isa = PBXShellScriptBuildPhase;" << "\n"
	  << "\t\t\t" << "name = \"Qt Preprocessors\";" << "\n"
	  << "\t\t\t" << "neededFileNames = (" << "\n"
	  << "\t\t\t" << ");" << "\n"
	  << "\t\t\t" << "shellPath = /bin/sh;" << "\n"
	  << "\t\t\t" << "shellScript = \"make -C " << QDir::currentDirPath() <<
	    " -f " << mkfile << "\";" << "\n"
	  << "\t\t" << "};" << "\n";
    }
    //SOURCE BUILDPHASE
    if(!project->isEmpty("QMAKE_PBX_OBJ")) {
	QString grp = "Build Sources", key = keyFor(grp);
	project->variables()["QMAKE_PBX_BUILDPHASES"].append(key);
	t << "\t\t" << key << " = {" << "\n"
	  << "\t\t\t" << "buildActionMask = 2147483647;" << "\n"
	  << "\t\t\t" << "files = (" << "\n"
	  << varGlue("QMAKE_PBX_OBJ", "\t\t\t\t", ",\n\t\t\t\t", "\n")
	  << "\t\t\t" << ");" << "\n"
	  << "\t\t\t" << "isa = PBXSourcesBuildPhase;" << "\n"
	  << "\t\t\t" << "name = \"" << grp << "\";" << "\n"
	  << "\t\t" << "};" << "\n";
    }

    //DUMP LIBRARIES
    QStringList &libdirs = project->variables()["QMAKE_PBX_LIBPATHS"];
    QString libs[] = { "QMAKE_LIBDIR_FLAGS", "QMAKE_LIBS", QString::null };
    for(i = 0; !libs[i].isNull(); i++) {
	tmp = project->variables()[libs[i]];
	for(QStringList::Iterator it = tmp.begin(); it != tmp.end();) {
	    bool remove = FALSE;
	    QString library, name;
	    if((*it).left(2) == "-L") {
		QString r = (*it).right((*it).length() - 2);
		fixEnvVariables(r);
		libdirs.append(r);
	    } else if((*it).left(2) == "-l") {
		name = (*it).right((*it).length() - 2);
		QString lib("lib" + name  + ".");
		for(QStringList::Iterator lit = libdirs.begin(); lit != libdirs.end(); ++lit) {
		    QString extns[] = { "dylib", "so", "a", QString::null };
		    for(int n = 0; !extns[n].isNull(); n++) {
			QString tmp =  (*lit) + Option::dir_sep + lib + extns[n];
			if(QFile::exists(tmp)) {
			    library = tmp;
			    //don't remove, so it gets into LDFLAGS for now -- FIXME
			    //remove = TRUE
			    break;
			}
		    }
		}
	    } else if((*it) == "-framework") {
		++it;
		if(it == tmp.end())
		    break;
		QStringList &fdirs = project->variables()["QMAKE_FRAMEWORKDIR"];
		if(fdirs.isEmpty())
		    fdirs.append("/System/Library/Frameworks/");
		for(QStringList::Iterator fit = fdirs.begin(); fit != fdirs.end(); ++fit) {
		    if(QFile::exists((*fit) + QDir::separator() + (*it) + ".framework")) {
			--it;
			it = tmp.remove(it);
			remove = TRUE;
			library = (*fit) + Option::dir_sep + (*it) + ".framework";
			break;
		    }
		}
	    } else if((*it).left(1) != "-") {
		remove = TRUE;
		library = (*it);
	    }
	    if(!library.isEmpty()) {
		if(name.isEmpty()) {
		    int slsh = library.findRev(Option::dir_sep);
		    if(slsh != -1)
			name = library.right(library.length() - slsh - 1);
		}
		fileFixify(library);
		QString key = keyFor(library);
		bool is_frmwrk = (library.right(10) == ".framework");
		t << "\t\t" << key << " = {" << "\n"
		  << "\t\t\t" << "isa = " << (is_frmwrk ? "PBXFrameworkReference" : "PBXFileReference") << ";" << "\n"
		  << "\t\t\t" << "name = \"" << name << "\";" << "\n"
		  << "\t\t\t" << "path = \"" << library << "\";" << "\n"
		  << "\t\t\t" << "refType = 0;" << "\n"
		  << "\t\t" << "};" << "\n";
		project->variables()["QMAKE_PBX_LIBRARIES"].append(key);
		QString obj_key = library + ".o";
		obj_key = keyFor(obj_key);
		t << "\t\t" << obj_key << " = {" << "\n"
		  << "\t\t\t" << "fileRef = " << key << ";" << "\n"
		  << "\t\t\t" << "isa = PBXBuildFile;" << "\n"
		  << "\t\t\t" << "settings = {" << "\n"
		  << "\t\t\t" << "};" << "\n"
		  << "\t\t" << "};" << "\n";
		project->variables()["QMAKE_PBX_BUILD_LIBRARIES"].append(obj_key);
	    }
	    if(remove)
		it = tmp.remove(it);
	    else
		++it;
	}
	project->variables()[libs[i]] = tmp;
    }
    //SUBLIBS BUILDPHASE (just another makefile)
    if(!project->isEmpty("SUBLIBS")) {
	QString mkfile = Option::output_dir + Option::dir_sep + "qt_sublibs.mak";
	QFile mkf(mkfile);
	if(mkf.open(IO_WriteOnly | IO_Translate)) {
	    debug_msg(0, "Creating file: %s", mkfile.latin1());
	    QTextStream mkt(&mkf);
	    writeHeader(mkt);
	    mkt << "SUBLIBS= ";
	    tmp = project->variables()["SUBLIBS"];
	    QStringList::Iterator it;
	    for(it = tmp.begin(); it != tmp.end(); ++it)
		t << "tmp/lib" << (*it) << ".a ";
	    t << endl << endl;
	    mkt << "sublibs: $(SUBLIBS)" << endl << endl;
	    tmp = project->variables()["SUBLIBS"];
	    for(it = tmp.begin(); it != tmp.end(); ++it)
		t << "tmp/lib" << (*it) << ".a" << ":\n\t"
		  << var(QString("MAKELIB") + (*it)) << endl << endl;
	    mkf.close();
	}
	QString phase_key = keyFor("QMAKE_PBX_SUBLIBS_BUILDPHASE");
	fileFixify(mkfile, QDir::currentDirPath());
	project->variables()["QMAKE_PBX_BUILDPHASES"].append(phase_key);
	t << "\t\t" << phase_key << " = {" << "\n"
	  << "\t\t\t" << "buildActionMask = 2147483647;" << "\n"
	  << "\t\t\t" << "files = (" << "\n"
	  << "\t\t\t" << ");" << "\n"
	  << "\t\t\t" << "generatedFileNames = (" << "\n"
	  << "\t\t\t" << ");" << "\n"
	  << "\t\t\t" << "isa = PBXShellScriptBuildPhase;" << "\n"
	  << "\t\t\t" << "name = \"Qt Sublibs\";" << "\n"
	  << "\t\t\t" << "neededFileNames = (" << "\n"
	  << "\t\t\t" << ");" << "\n"
	  << "\t\t\t" << "shellPath = /bin/sh;" << "\n"
	  << "\t\t\t" << "shellScript = \"make -C " << QDir::currentDirPath() <<
	    " -f " << mkfile << "\";" << "\n"
	  << "\t\t" << "};" << "\n";
    }
    //LIBRARY BUILDPHASE
    if(!project->isEmpty("QMAKE_PBX_LIBRARIES")) {
	tmp = project->variables()["QMAKE_PBX_LIBRARIES"];
	if(!tmp.isEmpty()) {
	    QString grp("External Frameworks and Libraries"), key = keyFor(grp);
	    project->variables()["QMAKE_PBX_GROUPS"].append(key);
	    t << "\t\t" << key << " = {" << "\n"
	      << "\t\t\t" << "children = (" << "\n"
	      << varGlue("QMAKE_PBX_LIBRARIES", "\t\t\t\t", ",\n\t\t\t\t", "\n")
	      << "\t\t\t" << ");" << "\n"
	      << "\t\t\t" << "isa = PBXGroup;" << "\n"
	      << "\t\t\t" << "name = \"" << grp << "\"" << ";" << "\n"
	      << "\t\t\t" << "path = \"\";" << "\n"
	      << "\t\t\t" << "refType = 4;" << "\n"
	      << "\t\t" << "};" << "\n";
	}
	tmp = project->variables()["QMAKE_PBX_BUILD_LIBRARIES"];
	if(!tmp.isEmpty()) {
	    QString grp("Frameworks & Libraries"), key = keyFor(grp);
	    project->variables()["QMAKE_PBX_BUILDPHASES"].append(key);
	    t << "\t\t" << key << " = {" << "\n"
	      << "\t\t\t" << "buildActionMask = 2147483647;" << "\n"
	      << "\t\t\t" << "files = (" << "\n"
	      << varGlue("QMAKE_PBX_BUILD_LIBRARIES", "\t\t\t\t", ",\n\t\t\t\t", "\n")
	      << "\t\t\t" << ");" << "\n"
	      << "\t\t\t" << "isa = PBXFrameworksBuildPhase;" << "\n"
	      << "\t\t\t" << "name = \"" << grp << "\";" << "\n"
	      << "\t\t" << "};" << "\n";
	}
    }

    //DUMP EVERYTHING THAT TIES THE ABOVE TOGETHER
    //PRODUCTS
    {
	QString grp("Products"), key = keyFor(grp);
	project->variables()["QMAKE_PBX_GROUPS"].append(key);
	t << "\t\t" << key << " = {" << "\n"
	  << "\t\t\t" << "children = (" << "\n"
	  << "\t\t\t\t" << keyFor("QMAKE_PBX_REFERENCE") << "\n"
	  << "\t\t\t" << ");" << "\n"
	  << "\t\t\t" << "isa = PBXGroup;" << "\n"
	  << "\t\t\t" << "name = Products;" << "\n"
	  << "\t\t\t" << "refType = 4;" << "\n"
	  << "\t\t" << "};" << "\n";
    }
    //INSTALL BUILDPHASE (sh script)
    if(!project->isEmpty("DESTDIR")) {
	QString targ = project->first("TARGET");
	if(project->first("TEMPLATE") == "lib" && !project->isActiveConfig("staticlib") &&
	   project->isActiveConfig("frameworklib"))
	    targ = project->first("QMAKE_ORIG_TARGET");
	int slsh = targ.findRev(Option::dir_sep);
	if(slsh != -1)
	    targ = targ.right(targ.length() - slsh - 1);
	fixEnvVariables(targ);
	QStringList links;
	if(project->first("TEMPLATE") == "app") {
	    targ += ".app";
	} else if(!project->isActiveConfig("staticlib") &&
	   !project->isActiveConfig("frameworklib")) {
	    QString li[] = { "TARGET_", "TARGET_x", "TARGET_x.y", QString::null };
	    for(int n = 0; !li[n].isNull(); n++) {
		QString t = project->first(li[n]);
		slsh = t.findRev(Option::dir_sep);
		if(slsh != -1)
		    t = t.right(t.length() - slsh);
		fixEnvVariables(t);
		links << t;
	    }
	}
	QString script = Option::output_dir + Option::dir_sep + "qt_install.sh";
	QFile shf(script);
	if(shf.open(IO_WriteOnly | IO_Translate)) {
	    debug_msg(0, "Creating file: %s", script.latin1());
	    QString lib = project->first("QMAKE_ORIG_TARGET");
	    if(project->first("TEMPLATE") == "app") {
		lib += ".app";
	    } else if(!project->isActiveConfig("staticlib") &&
		      !project->isActiveConfig("frameworklib")) {
		lib = project->first("TARGET_");
		int slsh = lib.findRev(Option::dir_sep);
		if(slsh != -1)
		    lib = lib.right(lib.length() - slsh - 1);
	    }
	    QTextStream sht(&shf);
	    QString dstdir = project->first("DESTDIR");
	    fixEnvVariables(dstdir);
	    sht << "#!/bin/sh" << endl;
	    sht << "OUT_TARG=\"" << lib << "\"\n" 
		<< "[ -z \"$BUILD_ROOT\" ] || OUT_TARG=\"${BUILD_ROOT}/${OUT_TARG}\"" << endl;
	    sht << "cp -r \"$OUT_TARG\" " << "\"" <<
		dstdir << targ << "\"" << endl;
	    if(project->first("TEMPLATE") == "lib" && project->isActiveConfig("frameworklib"))
		sht << "ln -sf \"" << targ <<  "\" " << "\"" << dstdir << lib << "\"" << endl;
	    for(QStringList::Iterator it = links.begin(); it != links.end(); ++it)
		sht << "ln -sf \"" << targ <<  "\" " <<
		    "\"" << dstdir << (*it) << "\"" << endl;
	    shf.close();
#ifdef Q_OS_UNIX
	    chmod(script.latin1(), S_IRWXU | S_IRWXG);
#endif
	    QString phase_key = keyFor("QMAKE_PBX_INSTALL_BUILDPHASE");
	    fileFixify(script, QDir::currentDirPath());
	    project->variables()["QMAKE_PBX_BUILDPHASES"].append(phase_key);
	    t << "\t\t" << phase_key << " = {" << "\n"
	      << "\t\t\t" << "buildActionMask = 2147483647;" << "\n"
	      << "\t\t\t" << "files = (" << "\n"
	      << "\t\t\t" << ");" << "\n"
	      << "\t\t\t" << "generatedFileNames = (" << "\n"
	      << "\t\t\t" << ");" << "\n"
	      << "\t\t\t" << "isa = PBXShellScriptBuildPhase;" << "\n"
	      << "\t\t\t" << "name = \"Qt Install\";" << "\n"
	      << "\t\t\t" << "neededFileNames = (" << "\n"
	      << "\t\t\t" << ");" << "\n"
	      << "\t\t\t" << "shellPath = /bin/sh;" << "\n"
	      << "\t\t\t" << "shellScript = \"" << script << "\";" << "\n"
	      << "\t\t" << "};" << "\n";
	}
    }
    //ROOT_GROUP
    t << "\t\t" << keyFor("QMAKE_PBX_ROOT_GROUP") << " = {" << "\n"
      << "\t\t\t" << "children = (" << "\n"
      << varGlue("QMAKE_PBX_GROUPS", "\t\t\t\t", ",\n\t\t\t\t", "\n")
      << "\t\t\t" << ");" << "\n"
      << "\t\t\t" << "isa = PBXGroup;" << "\n"
      << "\t\t\t" << "name = " << project->first("QMAKE_ORIG_TARGET") << ";" << "\n"
      << "\t\t\t" << "path = \"\";" << "\n"
      << "\t\t\t" << "refType = 4;" << "\n"
      << "\t\t" << "};" << "\n";
    //REFERENCE
    t << "\t\t" << keyFor("QMAKE_PBX_REFERENCE") << " = {" << "\n"
      << "\t\t\t" << "refType = 3;" << "\n";
    if(project->first("TEMPLATE") == "app") {
	t << "\t\t\t" << "isa = PBXApplicationReference;" << "\n"
	<< "\t\t\t" << "path = " << project->first("QMAKE_ORIG_TARGET") << ".app;" << "\n";
    } else {
	QString lib = project->first("QMAKE_ORIG_TARGET");
	if(!project->isActiveConfig("staticlib") && !project->isActiveConfig("frameworklib")) {
	    if(project->isActiveConfig("plugin"))
	       lib = project->first("TARGET");
	    else
		lib = project->first("TARGET_");
	    int slsh = lib.findRev(Option::dir_sep);
	    if(slsh != -1)
		lib = lib.right(lib.length() - slsh - 1);
	}
	t << "\t\t\t" << "isa = PBXLibraryReference;" << "\n"
	  << "\t\t\t" << "path = " << lib << ";\n";
    }
    t << "\t\t" << "};" << "\n";
    //TARGET
    t << "\t\t" << keyFor("QMAKE_PBX_TARGET") << " = {" << "\n"
      << "\t\t\t" << "buildPhases = (" << "\n"
      << varGlue("QMAKE_PBX_BUILDPHASES", "\t\t\t\t", ",\n\t\t\t\t", "\n")
      << "\t\t\t" << ");" << "\n"
      << "\t\t\t" << "buildSettings = {" << "\n"
      << "\t\t\t\t" << "FRAMEWORK_SEARCH_PATHS = \"\";" << "\n"
      << "\t\t\t\t" << "HEADER_SEARCH_PATHS = \"" << fixEnvsList("INCLUDEPATH") << " " << fixEnvs(specdir()) << "\";" << "\n"
      << "\t\t\t\t" << "LIBRARY_SEARCH_PATHS = \"" << var("QMAKE_PBX_LIBPATHS") << "\";" << "\n"
      << "\t\t\t\t" << "OPTIMIZATION_CFLAGS = \"\";" << "\n"
      << "\t\t\t\t" << "OTHER_CFLAGS = \"" <<
	fixEnvsList("QMAKE_CFLAGS") << varGlue("PRL_EXPORT_DEFINES"," -D"," -D","") <<
	varGlue("DEFINES"," -D"," -D","") << "\";" << "\n"
      << "\t\t\t\t" << "LEXFLAGS = \"" << var("QMAKE_LEXFLAGS") << "\";" << "\n"
      << "\t\t\t\t" << "YACCFLAGS = \"" << var("QMAKE_YACCFLAGS") << "\";" << "\n"
      << "\t\t\t\t" << "OTHER_CPLUSPLUSFLAGS = \"" <<
	fixEnvsList("QMAKE_CXXFLAGS") << varGlue("PRL_EXPORT_DEFINES"," -D"," -D","") <<
	varGlue("DEFINES"," -D"," -D","") << "\";" << "\n"
      << "\t\t\t\t" << "OTHER_LDFLAGS = \"" << fixEnvsList("SUBLIBS") << " " <<
	fixEnvsList("QMAKE_LFLAGS") << " " << fixEnvsList("QMAKE_LIBDIR_FLAGS") <<
	" " << fixEnvsList("QMAKE_LIBS") << "\";" << "\n"
      << "\t\t\t\t" << "OTHER_REZFLAGS = \"\";" << "\n"
      << "\t\t\t\t" << "SECTORDER_FLAGS = \"\";" << "\n"
      << "\t\t\t\t" << "WARNING_CFLAGS = \"\";" << "\n";
#if 1
    t << "\t\t\t\t" << "BUILD_ROOT = \"" << QDir::currentDirPath() << "\";" << "\n";
#endif
    if(!project->isEmpty("DESTDIR"))
	t << "\t\t\t\t" << "INSTALL_PATH = \"" << project->first("DESTDIR") << "\";" << "\n";
    if(!project->isEmpty("VERSION") && project->first("VERSION") != "0.0.0")
	t << "\t\t\t\t" << "DYLIB_CURRENT_VERSION = \"" << project->first("VERSION") << "\";" << "\n";
    if(!project->isEmpty("OBJECTS_DIR"))
	t << "\t\t\t\t" << "OBJECT_FILE_DIR = \"" << project->first("OBJECTS_DIR") << "\";" << "\n";
    if(project->first("TEMPLATE") == "app") {
	t << "\t\t\t\t" << "WRAPPER_EXTENSION = app;" << "\n"
	  << "\t\t\t\t" << "PRODUCT_NAME = " << project->first("QMAKE_ORIG_TARGET") << ";" << "\n";
    } else {
	QString lib = project->first("QMAKE_ORIG_TARGET");
	if(!project->isActiveConfig("plugin") && project->isActiveConfig("staticlib")) {
	    t << "\t\t\t\t" << "LIBRARY_STYLE = STATIC;" << "\n";
	} else {
	    t << "\t\t\t\t" << "LIBRARY_STYLE = DYNAMIC;" << "\n";
	    if(!project->isActiveConfig("frameworklib")) {
		if(project->isActiveConfig("plugin"))
		    lib = project->first("TARGET");
		else
		    lib = project->first("TARGET_");
		int slsh = lib.findRev(Option::dir_sep);
		if(slsh != -1)
		    lib = lib.right(lib.length() - slsh - 1);
	    }
	}
	t << "\t\t\t\t" << "PRODUCT_NAME = " << lib << ";" << "\n";
    }
    tmp = project->variables()["QMAKE_PBX_VARS"];
    for(QStringList::Iterator it = tmp.begin(); it != tmp.end(); ++it)
	t << "\t\t\t\t" << (*it) << " = " << getenv((*it)) << ";" << "\n";
    t << "\t\t\t" << "};" << "\n"
      << "\t\t\t" << "conditionalBuildSettings = {" << "\n"
      << "\t\t\t" << "};" << "\n"
      << "\t\t\t" << "dependencies = (" << "\n"
      << "\t\t\t" << ");" << "\n"
      << "\t\t\t" << "productReference = " << keyFor("QMAKE_PBX_REFERENCE") << ";" << "\n"
      << "\t\t\t" << "shouldUseHeadermap = 1;" << "\n";
    if(project->first("TEMPLATE") == "app") {
	t << "\t\t\t" << "isa = PBXApplicationTarget;" << "\n"
	  << "\t\t\t" << "name = \"" << project->first("QMAKE_ORIG_TARGET") << "\";" << "\n"
	  << "\t\t\t" << "productName = " << project->first("QMAKE_ORIG_TARGET") << ";" << "\n";
	t << "\t\t\t" << "productSettingsXML = " << "\"" << "<?xml version=" 
	  << "\\\"1.0\\\" encoding=" << "\\\"UTF-8\\\"" << "?>" << "\n"
	  << "\t\t\t\t" << "<!DOCTYPE plist SYSTEM \\\"file://localhost/System/" 
	  << "Library/DTDs/PropertyList.dtd\\\">" << "\n"
	  << "\t\t\t\t" << "<plist version=\\\"0.9\\\">" << "\n"
	  << "\t\t\t\t" << "<dict>" << "\n"
	  << "\t\t\t\t\t" << "<key>CFBundleDevelopmentRegion</key>" << "\n"
	  << "\t\t\t\t\t" << "<string>English</string>" << "\n"
	  << "\t\t\t\t\t" << "<key>CFBundleExecutable</key>" << "\n"
	  << "\t\t\t\t\t" << "<string>" << project->first("QMAKE_ORIG_TARGET") << "</string>" << "\n"
	  << "\t\t\t\t\t" << "<key>CFBundleIconFile</key>" << "\n"
	  << "\t\t\t\t\t" << "<string></string>" << "\n"
	  << "\t\t\t\t\t" << "<key>CFBundleInfoDictionaryVersion</key>"  << "\n"
	  << "\t\t\t\t\t" << "<string>6.0</string>" << "\n"
	  << "\t\t\t\t\t" << "<key>CFBundlePackageType</key>" << "\n"
	  << "\t\t\t\t\t" << "<string>APPL</string>" << "\n"
	  << "\t\t\t\t\t" << "<key>CFBundleSignature</key>" << "\n"
	  << "\t\t\t\t\t" << "<string>????</string>" << "\n"
	  << "\t\t\t\t\t" << "<key>CFBundleVersion</key>" << "\n"
	  << "\t\t\t\t\t" << "<string>0.1</string>" << "\n"
	  << "\t\t\t\t\t" << "<key>CSResourcesFileMapped</key>" << "\n"
	  << "\t\t\t\t\t" << "<true/>" << "\n"
	  << "\t\t\t\t" << "</dict>" << "\n"
	  << "\t\t\t\t" << "</plist>" << "\";" << "\n";
    } else {
	QString lib = project->first("QMAKE_ORIG_TARGET");
	if(!project->isActiveConfig("frameworklib"))
	   lib.prepend("lib");
	t << "\t\t\t" << "isa = PBXLibraryTarget;" << "\n"
	  << "\t\t\t" << "name = \"" << lib << "\";" << "\n"
	  << "\t\t\t" << "productName = " << lib << ";" << "\n";
    }
    if(!project->isEmpty("DESTDIR"))
	t << "\t\t\t" << "productInstallPath = \"" << project->first("DESTDIR") << "\";" << "\n";
    t << "\t\t" << "};" << "\n";
    //DEBUG-RELEASE
    QString builds[] = { "DEBUG", "RELEASE", QString::null };
    for(i = 0; !builds[i].isNull(); i++) {
	QString key = "QMAKE_PBX_" + builds[i];
	key = keyFor(key);
	project->variables()["QMAKE_PBX_BUILDSTYLES"].append(key);
	bool as_debug = (builds[i] == "DEBUG");
	t << "\t\t" << key << " = {" << "\n"
	  << "\t\t\t" << "buildRules = (" << "\n"
	  << "\t\t\t" << ");" << "\n"
	  << "\t\t\t" << "buildSettings = {" << "\n"
	  << "\t\t\t\t" << "COPY_PHASE_STRIP = " << (as_debug ? "NO" : "YES") << ";" << "\n"
	  << "\t\t\t" << "};" << "\n"
	  << "\t\t\t" << "isa = PBXBuildStyle;" << "\n"
	  << "\t\t\t" << "name = " << (as_debug ? "Development" : "Deployment") << ";" << "\n"
	  << "\t\t" << "};" << "\n";
    }
    //ROOT
    t << "\t\t" << keyFor("QMAKE_PBX_ROOT") << " = {" << "\n"
      << "\t\t\t" << "buildStyles = (" << "\n"
      << varGlue("QMAKE_PBX_BUILDSTYLES", "\t\t\t\t", ",\n\t\t\t\t", "\n")
      << "\t\t\t" << ");" << "\n"
      << "\t\t\t" << "isa = PBXProject;" << "\n"
      << "\t\t\t" << "mainGroup = " << keyFor("QMAKE_PBX_ROOT_GROUP") << ";" << "\n"
      << "\t\t\t" << "targets = (" << "\n"
      << "\t\t\t\t" << keyFor("QMAKE_PBX_TARGET") << "\n"
      << "\t\t\t" << ");" << "\n"
      << "\t\t" << "};" << "\n";

    //FOOTER
    t << "\t" << "};" << "\n"
      << "\t" << "rootObject = " << keyFor("QMAKE_PBX_ROOT") << ";" << "\n"
      << "}" << endl;

    QString mkwrap = Option::output_dir + Option::dir_sep + ".." + Option::dir_sep + project->first("MAKEFILE");
    QFile mkwrapf(mkwrap);
    if(mkwrapf.open(IO_WriteOnly | IO_Translate)) {
	debug_msg(0, "Creating file: %s", mkwrap.latin1());
	QTextStream mkwrapt(&mkwrapf);
	writeHeader(mkwrapt);
	mkwrapt << "#This is a makefile wrapper for PROJECT BUILDER\n"
		<< "all:" << "\n\t" 
		<< "cd " << project->first("QMAKE_ORIG_TARGET") + ".pbproj/ && pbxbuild" << "\n"
		<< "distclean clean:" << "\n\t" 
		<< "cd " << project->first("QMAKE_ORIG_TARGET") + ".pbproj/ && pbxbuild clean" << "\n"
		<< "install:" 
		<< endl;
    }
    return TRUE;
}

QString
ProjectBuilderMakefileGenerator::fixEnvs(QString file)
{
    QRegExp reg_var("\\$\\((.*)\\)");
    for(int rep = 0; (rep = reg_var.search(file, rep)) != -1; ) {
	if(project->variables()["QMAKE_PBX_VARS"].findIndex(reg_var.cap(1)) == -1)
	    project->variables()["QMAKE_PBX_VARS"].append(reg_var.cap(1));
	rep += reg_var.matchedLength();
    }
    return file;
}

QString
ProjectBuilderMakefileGenerator::fixEnvsList(QString where)
{
    QString ret;
    const QStringList &l = project->variables()[where];
    for(QStringList::ConstIterator it = l.begin(); it != l.end(); ++it) {
	fixEnvs((*it));
	if(!ret.isEmpty())
	    ret += " ";
	ret += (*it);
    }
    return ret;
}

QString
ProjectBuilderMakefileGenerator::keyFor(QString block)
{
#if 0 //This make this code much easier to debug..
    return block;
#endif

    QString ret;
    if(!keys.contains(block)) {
#if 0
	static unsigned int r = 0;
	ret.sprintf("%024x", ++r);
#else //not really necesary, but makes it look more interesting..
	static struct { unsigned int a1, a2, a3; } r = { 0, 0, 0 };
	if(!r.a1 && !r.a2 && !r.a3) {
	    r.a1 = rand();
	    r.a2 = rand();
	    r.a3 = rand();
	}
	switch(rand() % 3) {
	case 0: ++r.a1; break;
	case 1: ++r.a2; break;
	case 2: ++r.a3; break;
	}
	ret.sprintf("%08x%08x%08x", r.a1, r.a2, r.a3);
#endif
	ret = ret.upper();
	keys.insert(block, ret);
    } else {
	ret = keys[block];
    }
    return ret;
}

QString
ProjectBuilderMakefileGenerator::defaultMakefile() const
{
    if(project->first("TEMPLATE") == "subdirs")
	return UnixMakefileGenerator::defaultMakefile();
    return project->first("TARGET") + ".pbproj/project.pbxproj";
}
