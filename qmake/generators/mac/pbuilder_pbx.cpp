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


ProjectBuilderMakefileGenerator::ProjectBuilderMakefileGenerator(QMakeProject *p) : UnixMakefileGenerator(p), init_flag(FALSE)
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

    if(project->first("TEMPLATE") == "app" ||
       project->first("TEMPLATE") == "lib") {
	return writeMakeParts(t);
    }
    else if(project->first("TEMPLATE") == "subdirs") {
	qDebug("Not supported!");
	return TRUE;
    }
    return FALSE;
}

bool
ProjectBuilderMakefileGenerator::writeMakeParts(QTextStream &t)
{
    QStringList tmp, tmp_out;
    int i;
    //HEADER
    t << "// !$*UTF8*$!" << "\n" 
      << "{" << "\n"
      << "\t" << "archiveVersion = 1;" << "\n"
      << "\t" << "classes = {" << "\n" << "\t" << "};" << "\n"
      << "\t" << "objectVersion = 32;" << "\n"
      << "\t" << "objects = {" << endl;

    //DUMP SOURCES
    QString srcs[] = { "SOURCES", "SRCMOC", "UICIMPLS", QString::null };
    for(i = 0; !srcs[i].isNull(); i++) {
	tmp = project->variables()[srcs[i]];
	for(QStringList::Iterator it = tmp.begin(); it != tmp.end(); ++it) {
	    QString file = (*it);
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
	    QString obj_key = keyFor(file + ".o");
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
	    QString grp, script;
	    if(srcs[i] == "SOURCES") {
		grp = "Sources";
	    } else if(srcs[i] == "SRCMOC") {
		script = Option::output_dir + Option::dir_sep + "mocit.sh";
		grp = "Mocables";
	    } else if(srcs[i] == "UICIMPLS") {
		script = Option::output_dir + Option::dir_sep + "uicit.sh";
		grp = "UICables";
	    }
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

	    if(!script.isEmpty()) {
		QString phase_key = keyFor(srcs[i] + "_BUILDPHASE");
		project->variables()["QMAKE_PBX_BUILDPHASES"].append(phase_key);
		t << "\t\t" << phase_key << " = {" << "\n"
		  << "\t\t\t" << "buildActionMask = 2147483647;" << "\n"
		  << "\t\t\t" << "files = (" << "\n"
		  << "\t\t\t" << ");" << "\n"
		  << "\t\t\t" << "generatedFileNames = (" << "\n"
		  << "\t\t\t" << ");" << "\n"
		  << "\t\t\t" << "isa = PBXShellScriptBuildPhase;" << "\n"
		  << "\t\t\t" << "name = \"Creating " << grp << "\";" << "\n"
		  << "\t\t\t" << "neededFileNames = (" << "\n"
		  << "\t\t\t" << ");" << "\n"
		  << "\t\t\t" << "shellPath = /bin/sh;" << "\n"
		  << "\t\t\t" << "shellScript = \"" << script << "\";" << "\n"
		  << "\t\t" << "};" << "\n";
	    } else if(srcs[i] == "SOURCES") {
	    }
	}
    }
    {
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
    QString libs[] = { "SUBLIBS", "QMAKE_LIBDIR_FLAGS", "QMAKE_LIBS", QString::null };
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
		    for(int i = 0; !extns[i].isNull(); i++) {
			QString tmp =  (*lit) + Option::dir_sep + lib + extns[i];
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
		    if(QFile::exists((*fit) + Option::dir_sep + (*it) + ".framework")) {
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

		QString obj_key = keyFor(library + ".o");
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
      << "\t\t\t" << "isa = PBXApplicationReference;" << "\n"
      << "\t\t\t" << "path = " << project->first("QMAKE_ORIG_TARGET") << ".app;" << "\n"
      << "\t\t\t" << "refType = 3;" << "\n"
      << "\t\t" << "};" << "\n";
    //TARGET
    t << "\t\t" << keyFor("QMAKE_PBX_TARGET") << " = {" << "\n"
      << "\t\t\t" << "buildPhases = (" << "\n"
      << varGlue("QMAKE_PBX_BUILDPHASES", "\t\t\t\t", ",\n\t\t\t\t", "\n")
      << "\t\t\t" << ");" << "\n"
      << "\t\t\t" << "buildSettings = {" << "\n"
      << "\t\t\t\t" << "FRAMEWORK_SEARCH_PATHS = \"\";" << "\n"
      << "\t\t\t\t" << "HEADER_SEARCH_PATHS = \"" << fixEnvs("INCLUDEPATH") << " " << fixEnvs(specdir()) << "\";" << "\n"
      << "\t\t\t\t" << "INSTALL_PATH = \"" << 
	(project->isEmpty("DESTDIR") ? QString("") : project->first("DESTDIR")) << "\";" << "\n"
      << "\t\t\t\t" << "LIBRARY_SEARCH_PATHS = \"" << var("QMAKE_PBX_LIBPATHS") << "\";" << "\n"
      << "\t\t\t\t" << "OPTIMIZATION_CFLAGS = \"\";" << "\n"
      << "\t\t\t\t" << "OTHER_CFLAGS = \"" << 
	fixEnvs("QMAKE_CFLAGS") << varGlue("PRL_EXPORT_DEFINES","-D"," -D","") << varGlue("DEFINES","-D"," -D","") << "\";" << "\n"
      << "\t\t\t\t" << "OTHER_LDFLAGS = \"" << fixEnvs("QMAKE_LFLAGS") << " " << fixEnvs("QMAKE_LIBDIR_FLAGS") << 
	" " << fixEnvs("QMAKE_LIBS") << "\";" << "\n"
      << "\t\t\t\t" << "OTHER_REZFLAGS = \"\";" << "\n"
      << "\t\t\t\t" << "PRODUCT_NAME = " << project->first("QMAKE_ORIG_TARGET") << ";" << "\n"
      << "\t\t\t\t" << "SECTORDER_FLAGS = \"\";" << "\n"
      << "\t\t\t\t" << "WARNING_CFLAGS = \"\";" << "\n"
      << "\t\t\t\t" << "WRAPPER_EXTENSION = app;" << "\n"
      << "\t\t\t" << "};" << "\n"
      << "\t\t\t" << "conditionalBuildSettings = {" << "\n"
      << "\t\t\t" << "};" << "\n"
      << "\t\t\t" << "dependencies = (" << "\n"
      << "\t\t\t" << ");" << "\n"
      << "\t\t\t" << "isa = PBXApplicationTarget;" << "\n"
      << "\t\t\t" << "name = " << project->first("QMAKE_ORIG_TARGET") << ";" << "\n"
      << "\t\t\t" << "productInstallPath = \"" << 
	(project->isEmpty("DESTDIR") ? QString("") : project->first("DESTDIR")) << "\";" << "\n"
      << "\t\t\t" << "productName = " << project->first("QMAKE_ORIG_TARGET") << ";" << "\n"
      << "\t\t\t" << "productReference = " << keyFor("QMAKE_PBX_REFERENCE") << ";" << "\n"
      << "\t\t\t" << "shouldUseHeadermap = 1;" << "\n"
      << "\t\t" << "};" << "\n";
    //DEBUG-RELEASE
    QString builds[] = { "DEBUG", "RELEASE", QString::null };
    for(i = 0; !builds[i].isNull(); i++) {
	QString key = keyFor("QMAKE_PBX_" + builds[i]);
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
    return TRUE;
}

void
ProjectBuilderMakefileGenerator::init()
{
    if(init_flag)
	return;
    QStringList::Iterator it;
    init_flag = TRUE;
    UnixMakefileGenerator::init();
}

QString
ProjectBuilderMakefileGenerator::fixEnvs(QString where)
{
    QString ret;
    const QStringList &l = project->variables()[where];
    for(QStringList::ConstIterator it = l.begin(); it != l.end(); ++it) {
	QString p = (*it);
	fixEnvVariables(p);
	if(!ret.isEmpty())
	    ret += " ";
	ret += p;
    }
    return ret;
}

QString
ProjectBuilderMakefileGenerator::keyFor(QString file)
{
    QString ret;
    if(!keys.contains(file)) {
	static int r = 1;
	ret.sprintf("%024x", r++);
	ret = ret.upper();
	keys.insert(file, ret);
    } else {
	ret = keys[file];
    }
    return ret;
}
