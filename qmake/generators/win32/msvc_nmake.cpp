/****************************************************************************
**
** Implementation of NmakeMakefileGenerator class.
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

#include "msvc_nmake.h"
#include "option.h"
#include <qregexp.h>
#include <qhash.h>
#include <qdir.h>
#include <time.h>

NmakeMakefileGenerator::NmakeMakefileGenerator(QMakeProject *p) : Win32MakefileGenerator(p), init_flag(FALSE)
{

}

bool
NmakeMakefileGenerator::writeMakefile(QTextStream &t)
{
    writeHeader(t);
    if(!project->variables()["QMAKE_FAILED_REQUIREMENTS"].isEmpty()) {
	t << "all clean:" << "\n\t"
	  << "@echo \"Some of the required modules ("
	  << var("QMAKE_FAILED_REQUIREMENTS") << ") are not available.\"" << "\n\t"
	  << "@echo \"Skipped.\"" << endl << endl;
	writeMakeQmake(t);
	return TRUE;
    }

    if(project->first("TEMPLATE") == "app" ||
       project->first("TEMPLATE") == "lib") {
	writeNmakeParts(t);
	return MakefileGenerator::writeMakefile(t);
    }
    else if(project->first("TEMPLATE") == "subdirs") {
	writeSubDirs(t);
	return TRUE;
    }
    return FALSE;
}

QStringList
&NmakeMakefileGenerator::findDependencies(const QString &file)
{
    QStringList &aList = MakefileGenerator::findDependencies(file);
    // Note: The QMAKE_IMAGE_COLLECTION file have all images
    // as dependency, so don't add precompiled header then
    if (file == project->first("QMAKE_IMAGE_COLLECTION"))
	return aList;
    for(QStringList::Iterator it = Option::cpp_ext.begin(); it != Option::cpp_ext.end(); ++it) {
	if(file.endsWith(*it)) {
	    if(!aList.contains(precompH))
		aList += precompH;
	    break;
	}
    }
    return aList;
}

void
NmakeMakefileGenerator::writeNmakeParts(QTextStream &t)
{
    t << "####### Compiler, tools and options" << endl << endl;
    t << "CC		=	" << var("QMAKE_CC") << endl;
    t << "CXX		=	" << var("QMAKE_CXX") << endl;
    t << "LEX		= " << var("QMAKE_LEX") << endl;
    t << "YACC		= " << var("QMAKE_YACC") << endl;
    t << "CFLAGS	=	" << var("QMAKE_CFLAGS") << " "
      << varGlue("PRL_EXPORT_DEFINES","-D"," -D","") << " "
      <<  varGlue("DEFINES","-D"," -D","") << endl;
    t << "CXXFLAGS	=	" << var("QMAKE_CXXFLAGS") << " "
      << varGlue("PRL_EXPORT_DEFINES","-D"," -D","") << " "
      << varGlue("DEFINES","-D"," -D","") << endl;
    t << "LEXFLAGS	=" << var("QMAKE_LEXFLAGS") << endl;
    t << "YACCFLAGS	=" << var("QMAKE_YACCFLAGS") << endl;

    t << "INCPATH	=	";
    QStringList &incs = project->variables()["INCLUDEPATH"];
    for(QStringList::Iterator incit = incs.begin(); incit != incs.end(); ++incit) {
	QString inc = (*incit);
	if (inc.endsWith("\\"))
	    inc.truncate(inc.length()-1);
	t << " -I\"" << inc << "\"";
    }
    t << " -I\"" << specdir() << "\""
      << endl;
    if(!project->variables()["QMAKE_APP_OR_DLL"].isEmpty()) {
	t << "LINK	=	" << var("QMAKE_LINK") << endl;
	t << "LFLAGS	=	" << var("QMAKE_LFLAGS");
	if(!project->variables()["QMAKE_LIBDIR"].isEmpty())
	  t << " " << varGlue("QMAKE_LIBDIR","/LIBPATH:\"","\" /LIBPATH:\"","\"");
	t << endl;
	t << "LIBS	=	";
	QStringList &libs = project->variables()["QMAKE_LIBS"];
	for(QStringList::Iterator libit = libs.begin(); libit != libs.end(); ++libit) {
	    QString lib = (*libit);
	    if (lib.endsWith("\\"))
		lib.truncate(lib.length()-1);
	    t << " \"" << lib << "\"";
	}
	t << endl;
    }
    else {
	t << "LIB	=	" << var("QMAKE_LIB") << endl;
    }
    t << "MOC		=	" << (project->isEmpty("QMAKE_MOC") ? QString("moc") :
			      Option::fixPathToTargetOS(var("QMAKE_MOC"), FALSE)) << endl;
    t << "UIC		=	" << (project->isEmpty("QMAKE_UIC") ? QString("uic") :
			      Option::fixPathToTargetOS(var("QMAKE_UIC"), FALSE)) << endl;
    t << "QMAKE		=	" << (project->isEmpty("QMAKE_QMAKE") ? QString("qmake") :
			      Option::fixPathToTargetOS(var("QMAKE_QMAKE"), FALSE)) << endl;
    t << "IDC		=	" << (project->isEmpty("QMAKE_IDC") ? QString("idc") :
			      Option::fixPathToTargetOS(var("QMAKE_IDC"), FALSE)) << endl;
    t << "IDL		=	" << (project->isEmpty("QMAKE_IDL") ? QString("midl") :
			      Option::fixPathToTargetOS(var("QMAKE_IDL"), FALSE)) << endl;
    t << "ZIP		=	" << var("QMAKE_ZIP") << endl;
    t << "COPY_FILE	=       " << var("QMAKE_COPY") << endl;
    t << "COPY_DIR	=       " << var("QMAKE_COPY") << endl;
    t << "DEL_FILE	=       " << var("QMAKE_DEL_FILE") << endl;
    t << "DEL_DIR	=       " << var("QMAKE_DEL_DIR") << endl;
    t << "MOVE		=       " << var("QMAKE_MOVE") << endl;
    t << "CHK_DIR_EXISTS =	" << var("QMAKE_CHK_DIR_EXISTS") << endl;
    t << "MKDIR		=	" << var("QMAKE_MKDIR") << endl;
    t << endl;

    t << "####### Files" << endl << endl;
    t << "HEADERS =	" << varList("HEADERS") << endl;
    t << "SOURCES =	" << varList("SOURCES") << endl;
    t << "OBJECTS =	" << varList("OBJECTS") << endl;
    t << "FORMS =	" << varList("FORMS") << endl;
    t << "UICDECLS =	" << varList("UICDECLS") << endl;
    t << "UICIMPLS =	" << varList("UICIMPLS") << endl;
    t << "SRCMOC	=	" << varList("SRCMOC") << endl;
    t << "OBJMOC	=	" << varList("OBJMOC") << endl;

    QString extraCompilerDeps;
    if(!project->isEmpty("QMAKE_EXTRA_COMPILERS")) {
	t << "OBJCOMP = " << varList("OBJCOMP") << endl;
	extraCompilerDeps += " $(OBJCOMP) ";

	QStringList &comps = project->variables()["QMAKE_EXTRA_COMPILERS"];
	for(QStringList::Iterator compit = comps.begin(); compit != comps.end(); ++compit) {
	    QStringList &vars = project->variables()[(*compit) + ".variables"];
	    for(QStringList::Iterator varit = vars.begin(); varit != vars.end(); ++varit) {
		QStringList vals = project->variables()[(*varit)];
		if(!vals.isEmpty())
		    t << "QMAKE_COMP_" << (*varit) << " = " << valList(vals) << endl;
	    }
	}
    }

    t << "DIST	=	" << varList("DISTFILES") << endl;
    t << "TARGET	=	";
    if(!project->variables()[ "DESTDIR" ].isEmpty())
	t << varGlue("TARGET",project->first("DESTDIR"),"",project->first("TARGET_EXT"));
    else
	t << project->variables()[ "TARGET" ].first() << project->variables()[ "TARGET_EXT" ].first();
    t << endl;
    t << endl;

    t << "####### Implicit rules" << endl << endl;
    t << ".SUFFIXES: .c";
    QStringList::Iterator cppit;
    for(cppit = Option::cpp_ext.begin(); cppit != Option::cpp_ext.end(); ++cppit)
	t << " " << (*cppit);
    t << endl << endl;

    if(!project->isActiveConfig("no_batch")) {
	// Batchmode doesn't use the non implicit rules QMAKE_RUN_CXX & QMAKE_RUN_CC
	project->variables().remove("QMAKE_RUN_CXX");
	project->variables().remove("QMAKE_RUN_CC");

	QHash<QString, void*> source_directories;
	source_directories.insertMulti(".", (void*)1);
	QString directories[] = { QString("MOC_DIR"), QString("UI_SOURCES_DIR"), QString("UI_DIR"), QString::null };
	for(int y = 0; !directories[y].isNull(); y++) {
	    QString dirTemp = project->first(directories[y]);
	    if (dirTemp.endsWith("\\")) 
		dirTemp.truncate(dirTemp.length()-1);
	    if(!dirTemp.isEmpty())
		source_directories.insertMulti(dirTemp, (void*)1);
	}
	QString srcs[] = { QString("SOURCES"), QString("UICIMPLS"), QString("SRCMOC"), QString::null };
	for(int x = 0; !srcs[x].isNull(); x++) {
	    QStringList &l = project->variables()[srcs[x]];
	    for(QStringList::Iterator sit = l.begin(); sit != l.end(); ++sit) {
		QString sep = "\\";
		if((*sit).indexOf(sep) == -1)
		    sep = "/";
		QString dir = (*sit).section(sep, 0, -2);
		if(!dir.isEmpty() && !source_directories[dir])
		    source_directories.insertMulti(dir, (void*)1);
	    }
	}

	for(QHash<QString, void*>::Iterator it(source_directories.begin()); it != source_directories.end(); ++it) {
	    if(it.key().isEmpty())
		continue;
	    for(cppit = Option::cpp_ext.begin(); cppit != Option::cpp_ext.end(); ++cppit)
		t << "{" << it.key() << "}" << (*cppit) << "{" << var("OBJECTS_DIR") << "}" << Option::obj_ext << "::\n\t"
		  << var("QMAKE_RUN_CXX_IMP_BATCH").replace( QRegExp( "\\$@" ), var("OBJECTS_DIR") ) << endl << "\t$<" << endl << "<<" << endl << endl;
	    t << "{" << it.key() << "}" << ".c{" << var("OBJECTS_DIR") << "}" << Option::obj_ext << "::\n\t"
	      << var("QMAKE_RUN_CC_IMP_BATCH").replace( QRegExp( "\\$@" ), var("OBJECTS_DIR") ) << endl << "\t$<" << endl << "<<" << endl << endl;
	}
    } else {
	for(cppit = Option::cpp_ext.begin(); cppit != Option::cpp_ext.end(); ++cppit)
	    t << (*cppit) << Option::obj_ext << ":\n\t" << var("QMAKE_RUN_CXX_IMP") << endl << endl;
	t << ".c" << Option::obj_ext << ":\n\t" << var("QMAKE_RUN_CC_IMP") << endl << endl;
    }

    t << "####### Build rules" << endl << endl;
    t << "all: " << fileFixify(Option::output.name()) << " " << varGlue("ALL_DEPS"," "," "," ") << "$(TARGET)" << endl << endl;
    t << "$(TARGET): " << var("PRE_TARGETDEPS") << " $(UICDECLS) $(OBJECTS) $(OBJMOC) "
      << extraCompilerDeps << var("POST_TARGETDEPS");
    if(!project->variables()["QMAKE_APP_OR_DLL"].isEmpty()) {
	t << "\n\t" << "$(LINK) $(LFLAGS) /OUT:$(TARGET) @<< " << "\n\t  "
	  << "$(OBJECTS) $(OBJMOC) $(LIBS)";
    } else {
	t << "\n\t" << "$(LIB) /OUT:$(TARGET) @<<" << "\n\t  "
	  << "$(OBJECTS) $(OBJMOC)";
    }
    t << extraCompilerDeps;
    t << endl << "<<" << endl;
    if(project->isActiveConfig("dll") && !project->variables()["DLLDESTDIR"].isEmpty()) {
	QStringList dlldirs = project->variables()["DLLDESTDIR"];
	for(QStringList::Iterator dlldir = dlldirs.begin(); dlldir != dlldirs.end(); ++dlldir)
	    t << "\n\t" << "-$(COPY_FILE) $(TARGET) " << *dlldir;
    }
    if(!project->variables()["QMAKE_POST_LINK"].isEmpty())
	t << "\t" << var("QMAKE_POST_LINK") << endl;
    t << endl << endl;

    if(!project->variables()["RC_FILE"].isEmpty()) {
	t << var("RES_FILE") << ": " << var("RC_FILE") << "\n\t"
	  << var("QMAKE_RC") << " " << var("RC_FILE") << endl << endl;
    }

    t << "mocables: $(SRCMOC)" << endl
      << "uicables: $(UICIMPLS) $(UICDECLS)" << endl << endl;

    writeMakeQmake(t);

    QStringList dist_files = Option::mkfile::project_files;
    if(!project->isEmpty("QMAKE_INTERNAL_INCLUDED_FILES"))
	dist_files += project->variables()["QMAKE_INTERNAL_INCLUDED_FILES"];
    if(!project->isEmpty("TRANSLATIONS"))
	dist_files << var("TRANSLATIONS");
    if(!project->isEmpty("FORMS")) {
	QStringList &forms = project->variables()["FORMS"];
	for(QStringList::Iterator formit = forms.begin(); formit != forms.end(); ++formit) {
	    QString ui_h = fileFixify((*formit) + Option::h_ext.first());
	    if(QFile::exists(ui_h))
		dist_files << ui_h;
	}
    }
    t << "dist:" << "\n\t"
      << "$(ZIP) " << var("QMAKE_ORIG_TARGET") << ".zip " << "$(SOURCES) $(HEADERS) $(DIST) $(FORMS) "
      << dist_files.join(" ") << " " << var("TRANSLATIONS") << " " << var("IMAGES") << endl << endl;

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
    if(!project->isEmpty("IMAGES"))
	t << varGlue("QMAKE_IMAGE_COLLECTION", "\n\t-$(DEL_FILE) ", "\n\t-$(DEL_FILE) ", "");
    t << endl;

    // user defined targets

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

    if(project->variables()["QMAKE_NOFORCE"].isEmpty())
	t << "FORCE:" << endl << endl;

    t << "distclean: clean"
      << "\n\t-$(DEL_FILE) $(TARGET)"
      << endl << endl;

    // precompiled header
    if(usePCH) {
	QString precompRule = QString("-c -Yc -Fp%1 -Fo%2").arg(precompPch).arg(precompObj);
	t << precompObj << ": " << precompH << " " << findDependencies(precompH).join(" \\\n\t\t")
	  << "\n\t" << "$(CXX) " + precompRule +" $(CXXFLAGS) $(INCPATH) -TP " << precompH << endl << endl;
    }
}

QString
NmakeMakefileGenerator::var(const QString &value)
{
    if (usePCH) {
    	if ((value == "QMAKE_RUN_CXX_IMP_BATCH"
	    || value == "QMAKE_RUN_CXX_IMP"
	    || value == "QMAKE_RUN_CXX")) {
	    QFileInfo precompHInfo(precompH);
	    QString precompRule = QString("-c -FI%1 -Yu%2 -Fp%3")
		.arg(precompHInfo.fileName())
		.arg(precompHInfo.fileName())
		.arg(precompPch);
	    QString p = MakefileGenerator::var(value);
	    p.replace("-c", precompRule);
	    // Cannot use -Gm with -FI & -Yu, as this gives an 
	    // internal compiler error, on the newer compilers
	    p.remove("-Gm");
	    return p;
	} else if (value == "QMAKE_CXXFLAGS") {
	    // Remove internal compiler error option
	    return MakefileGenerator::var(value).remove("-Gm");
	}
    }

    // Normal val    
    return MakefileGenerator::var(value);
}

void
NmakeMakefileGenerator::init()
{
    if(init_flag)
	return;
    init_flag = TRUE;

    /* this should probably not be here, but I'm using it to wrap the .t files */
    if(project->first("TEMPLATE") == "app")
	project->variables()["QMAKE_APP_FLAG"].append("1");
    else if(project->first("TEMPLATE") == "lib")
	project->variables()["QMAKE_LIB_FLAG"].append("1");
    else if(project->first("TEMPLATE") == "subdirs") {
	MakefileGenerator::init();
	if(project->variables()["MAKEFILE"].isEmpty())
	    project->variables()["MAKEFILE"].append("Makefile");
	if(project->variables()["QMAKE"].isEmpty())
	    project->variables()["QMAKE"].append("qmake");
	return;
    }

    processVars();
    
    if(!project->isActiveConfig("debug"))
	project->variables()["DEFINES"] += "QT_NO_DEBUG";

    if(!project->variables()["DEF_FILE"].isEmpty())
	project->variables()["QMAKE_LFLAGS"].append(QString("/DEF:") + project->first("DEF_FILE"));

    if(!project->variables()["VERSION"].isEmpty()) {
	QString version = project->variables()["VERSION"][0];
	int firstDot = version.indexOf(".");
	QString major = version.left(firstDot);
	QString minor = version.right(version.length() - firstDot - 1);
	minor.replace(".", "");
	project->variables()["QMAKE_LFLAGS"].append("/VERSION:" + major + "." + minor);
    }

    // Base class init!
    MakefileGenerator::init();

    // Setup PCH variables
    precompH = project->first("PRECOMPILED_HEADER");
    usePCH = !precompH.isEmpty() && project->isActiveConfig("precompile_header");
    if (usePCH) {
	// Created files
	precompObj = var("OBJECTS_DIR") + project->first("TARGET") + "_pch" + Option::obj_ext;
	precompPch = var("OBJECTS_DIR") + project->first("TARGET") + "_pch.pch";
	// Add linking of precompObj (required for whole precompiled classes)
	project->variables()["OBJECTS"]		  += precompObj;
	// Add pch file to cleanup
	project->variables()["QMAKE_CLEAN"]	  += precompPch;
	// Return to variable pool
	project->variables()["PRECOMPILED_OBJECT"] = precompObj;
	project->variables()["PRECOMPILED_PCH"]    = precompPch;
    }

    QString version = project->first("VERSION").replace(".", "");
    if(project->isActiveConfig("dll")) {
	project->variables()["QMAKE_CLEAN"].append(project->first("DESTDIR") + project->first("TARGET") + version + ".exp");
    }
    if(project->isActiveConfig("debug")) {
	project->variables()["QMAKE_CLEAN"].append(project->first("DESTDIR") + project->first("TARGET") + version + ".pdb");
	project->variables()["QMAKE_CLEAN"].append(project->first("DESTDIR") + project->first("TARGET") + version + ".ilk");
	project->variables()["QMAKE_CLEAN"].append("vc*.pdb");
	project->variables()["QMAKE_CLEAN"].append("vc*.idb");
    }
}
