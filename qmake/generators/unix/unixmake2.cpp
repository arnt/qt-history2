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

#include "unixmake.h"
#include "option.h"
#include <qregexp.h>
#include <qfile.h>
#include <qdir.h>
#include <time.h>


UnixMakefileGenerator::UnixMakefileGenerator(QMakeProject *p) : MakefileGenerator(p), init_flag(FALSE), include_deps(FALSE)
{

}

bool
UnixMakefileGenerator::writeMakefile(QTextStream &t)
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

    if (project->variables()["TEMPLATE"].first() == "app" ||
	project->variables()["TEMPLATE"].first() == "lib") {
	writeMakeParts(t);
	return MakefileGenerator::writeMakefile(t);
    } else if(project->variables()["TEMPLATE"].first() == "subdirs") {
	writeSubdirs(t);
	return TRUE;
    }
    return FALSE;
}

void
UnixMakefileGenerator::writeMakeParts(QTextStream &t)
{
    QString deps = Option::output.name(), prl;
    fileFixify(deps, QDir::currentDirPath());
    bool do_incremental = (project->isActiveConfig("incremental") &&
			   !project->variables()["QMAKE_INCREMENTAL"].isEmpty() &&
			   (!project->variables()["QMAKE_APP_FLAG"].isEmpty() ||
			    !project->isActiveConfig("staticlib"))), 
	 src_incremental=FALSE, moc_incremental=FALSE;

    t << "####### Compiler, tools and options" << endl << endl;
    t << "CC       = ";
    if (project->isActiveConfig("thread") &&
	! project->variables()["QMAKE_CC_THREAD"].isEmpty())
	t << var("QMAKE_CC_THREAD") << endl;
    else
	t << var("QMAKE_CC") << endl;

    t << "CXX      = ";
    if (project->isActiveConfig("thread") &&
	! project->variables()["QMAKE_CXX_THREAD"].isEmpty())
	t << var("QMAKE_CXX_THREAD") << endl;
    else
	t << var("QMAKE_CXX") << endl;

    t << "LEX      = " << var("QMAKE_LEX") << endl;
    t << "YACC     = " << var("QMAKE_YACC") << endl;
    t << "CFLAGS   = " << var("QMAKE_CFLAGS") << " "
      << varGlue("PRL_EXPORT_DEFINES","-D"," -D","") << " "
      << varGlue("DEFINES","-D"," -D","") << endl;
    t << "CXXFLAGS = " << var("QMAKE_CXXFLAGS") << " "
      << varGlue("PRL_EXPORT_DEFINES","-D"," -D","") << " "
      << varGlue("DEFINES","-D"," -D","") << endl;
    t << "LEXFLAGS = " << var("QMAKE_LEXFLAGS") << endl;
    t << "YACCFLAGS= " << var("QMAKE_YACCFLAGS") << endl;
    t << "INCPATH  = " << varGlue("INCLUDEPATH","-I", " -I", "") << " -I" << specdir() << endl;

    if(!project->isActiveConfig("staticlib")) {
	t << "LINK     = ";
	if (project->isActiveConfig("thread") &&
	    ! project->variables()["QMAKE_LINK_THREAD"].isEmpty())
	    t << var("QMAKE_LINK_THREAD") << endl;
	else
	    t << var("QMAKE_LINK") << endl;

	t << "LFLAGS   = " << var("QMAKE_LFLAGS") << endl;
	t << "LIBS     = " << "$(SUBLIBS) " << var("QMAKE_LIBDIR_FLAGS") << " " << var("QMAKE_LIBS") << endl;
    }

    t << "AR       = " << var("QMAKE_AR") << endl;
    t << "RANLIB   = " << var("QMAKE_RANLIB") << endl;
    t << "MOC      = " << var("QMAKE_MOC") << endl;
    t << "UIC      = "	<< var("QMAKE_UIC") << endl;
    t << "QMAKE    = "	<< (project->isEmpty("QMAKE_QMAKE") ? QString("qmake") : var("QMAKE_QMAKE")) << endl;
    t << "TAR      = "	<< var("QMAKE_TAR") << endl;
    t << "GZIP     = " << var("QMAKE_GZIP") << endl;
    t << "COPY     = " << var("QMAKE_COPY") << endl;
    t << "COPY_FILE= " << var("QMAKE_COPY_FILE") << endl;
    t << "COPY_DIR = " << var("QMAKE_COPY_DIR") << endl;
    t << "DEL_FILE = " << var("QMAKE_DEL_FILE") << endl;
    t << "DEL_DIR  = " << var("QMAKE_DEL_DIR") << endl;
    t << "MOVE     = " << var("QMAKE_MOVE") << endl;
    t << endl;

    t << "####### Output directory" << endl << endl;
    if (! project->variables()["OBJECTS_DIR"].isEmpty())
	t << "OBJECTS_DIR = " << var("OBJECTS_DIR") << endl;
    else
	t << "OBJECTS_DIR = ./" << endl;
    t << endl;

    /* files */
    t << "####### Files" << endl << endl;
    t << "HEADERS = " << varList("HEADERS") << endl;
    t << "SOURCES = " << varList("SOURCES") << endl;
    if(do_incremental) {
	QStringList &objs = project->variables()["OBJECTS"], &incrs = project->variables()["QMAKE_INCREMENTAL"], incrs_out;
	t << "OBJECTS = ";
	for(QStringList::Iterator objit = objs.begin(); objit != objs.end(); ++objit) {
	    bool increment = FALSE;
	    for(QStringList::Iterator incrit = incrs.begin(); incrit != incrs.end(); ++incrit) {
		if((*objit).find(QRegExp((*incrit), TRUE, TRUE)) != -1) {
		    increment = TRUE;
		    incrs_out.append((*objit));
		    break;
		}
	    }
	    if(!increment)
		t << "\\\n\t\t" << (*objit);
	}
	if(incrs_out.count() == objs.count()) { //we just switched places, no real incrementals to be done!
	    t << incrs_out.join(" \\\n\t\t") << endl;
	} else if(!incrs_out.count()) {
	    t << endl;
	} else {
	    src_incremental = TRUE;
	    t << endl;
	    t << "INCREMENTAL_OBJECTS = " << incrs_out.join(" \\\n\t\t") << endl;
	}
    } else {
	t << "OBJECTS = " << varList("OBJECTS") << endl;
    }
    t << "FORMS = " << varList("FORMS") << endl;
    t << "UICDECLS = " << varList("UICDECLS") << endl;
    t << "UICIMPLS = " << varList("UICIMPLS") << endl;
    QString srcMoc = varList("SRCMOC"), objMoc = varList("OBJMOC");
    t << "SRCMOC   = " << srcMoc << endl;
    if(do_incremental) {
	QStringList &objs = project->variables()["OBJMOC"],
		   &incrs = project->variables()["QMAKE_INCREMENTAL"], incrs_out;
	t << "OBJMOC = ";
	for(QStringList::Iterator objit = objs.begin(); objit != objs.end(); ++objit) {
	    bool increment = FALSE;
	    for(QStringList::Iterator incrit = incrs.begin(); incrit != incrs.end(); ++incrit) {
		if((*objit).find(QRegExp((*incrit), TRUE, TRUE)) != -1) {
		    increment = TRUE;
		    incrs_out.append((*objit));
		    break;
		}
	    }
	    if(!increment)
		t << "\\\n\t\t" << (*objit);
	}
	if(incrs_out.count() == objs.count()) { //we just switched places, no real incrementals to be done!
	    t << incrs_out.join(" \\\n\t\t") << endl;
	} else if(!incrs_out.count()) {
	    t << endl;
	} else {
	    moc_incremental = TRUE;
	    t << endl;
	    t << "INCREMENTAL_OBJMOC = " << incrs_out.join(" \\\n\t\t") << endl;
	}
    } else {
	t << "OBJMOC = " << objMoc << endl;
    }
    if(do_incremental && !moc_incremental && !src_incremental)
	do_incremental = FALSE;
    t << "DIST	   = " << varList("DISTFILES") << endl;
    t << "QMAKE_TARGET = " << var("QMAKE_ORIG_TARGET") << endl;
    t << "DESTDIR  = " << var("DESTDIR") << endl;
    t << "TARGET   = " << var("TARGET") << endl;
    if(project->isActiveConfig("plugin") ) {
	t << "TARGETD   = " << var("TARGET") << endl;
    } else if (!project->isActiveConfig("staticlib") && project->variables()["QMAKE_APP_FLAG"].isEmpty()) {
	t << "TARGETA	= " << var("TARGETA") << endl;
	if(project->variables()["QMAKE_HPUX_SHLIBS"].isEmpty()) {
	    t << "TARGETD	= " << var("TARGET_x.y.z") << endl;
	    t << "TARGET0	= " << var("TARGET_") << endl;
	    t << "TARGET1	= " << var("TARGET_x") << endl;
	    t << "TARGET2	= " << var("TARGET_x.y") << endl;
	}
	else {
	    t << "TARGETD	= " << var("TARGET_x") << endl;
	    t << "TARGET0	= " << var("TARGET_") << endl;
	}
    }
    t << endl;

    // blasted incldues
    QStringList &qeui = project->variables()["QMAKE_EXTRA_UNIX_INCLUDES"];
    QStringList::Iterator it;
    for( it = qeui.begin(); it != qeui.end(); ++it)
	t << "include " << (*it) << endl;

    /* rules */
    t << "first: all" << endl;
    t << "####### Implicit rules" << endl << endl;
    t << ".SUFFIXES: .c";
    QStringList::Iterator cppit;
    for(cppit = Option::cpp_ext.begin(); cppit != Option::cpp_ext.end(); ++cppit)
	t << " " << (*cppit);
    t << endl << endl;
    for(cppit = Option::cpp_ext.begin(); cppit != Option::cpp_ext.end(); ++cppit)
	t << (*cppit) << ".o:\n\t" << var("QMAKE_RUN_CXX_IMP") << endl << endl;
    t << ".c.o:\n\t" << var("QMAKE_RUN_CC_IMP") << endl << endl;

    if(include_deps) {
	QString cmd=var("QMAKE_CFLAGS_DEPS") + " ";
	cmd += varGlue("DEFINES","-D"," -D","") + varGlue("PRL_EXPORT_DEFINES"," -D"," -D","");
	if(!project->isEmpty("QMAKE_ABSOLUTE_SOURCE_PATH"))
	    cmd += " -I" + project->first("QMAKE_ABSOLUTE_SOURCE_PATH") + " ";
	cmd += " $(INCPATH) " + varGlue("DEPENDPATH", "-I", " -I", "");
	QString odir;
	if(!project->variables()["OBJECTS_DIR"].isEmpty())
	    odir = project->first("OBJECTS_DIR");
	t << "###### Dependancies" << endl << endl;
	t << odir << ".deps/%.d: %.cpp\n\t"
	  << "@echo Creating depend for $<" << "\n\t"
	  << "@test -d $(@D) || mkdir -p $(@D)" << "\n\t"
	  << "@$(CXX) " << cmd << " $< | sed \"s,^\\($(*F).o\\):," << odir << "\\1:,g\" >$@" << endl << endl;

	t << odir << ".deps/%.d: %.c\n\t"
	  << "@echo Creating depend for $<" << "\n\t"
	  << "@test -d $(@D) || mkdir -p $(@D)" << "\n\t"
	  << "@$(CC) " << cmd << " $< | sed \"s,^\\($(*F).o\\):," << odir << "\\1:,g\" >$@" << endl << endl;


	QString src[] = { "SOURCES", "UICIMPLS", "SRCMOC", QString::null };
	for(int x = 0; !src[x].isNull(); x++) {
	    QStringList &l = project->variables()[src[x]];
	    for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
		if(!(*it).isEmpty()) {
		    QString d_file;
		    if((*it).right(2) == ".c") {
			d_file = (*it).left((*it).length() - 2);
		    } else {
			for(QStringList::Iterator cppit = Option::cpp_ext.begin();
			    cppit != Option::cpp_ext.end(); ++cppit) {
			    if((*it).right((*cppit).length()) == (*cppit)) {
				d_file = (*it).left((*it).length() - (*cppit).length());
				break;
			    }
			}
		    }
		    if(!d_file.isEmpty()) {
			d_file = odir + ".deps/" + d_file + ".d";
			QStringList deps = findDependencies((*it)).grep(QRegExp(Option::moc_ext + "$"));
			if(!deps.isEmpty())
			    t << d_file << ": " << deps.join(" ") << endl;;
			t << "-include " << d_file << endl;
		    }
		}
	    }
	}
    }

    t << "####### Build rules" << endl << endl;
    if(!project->variables()["SUBLIBS"].isEmpty()) {
	QString libdir = "tmp/";
	if(!project->isEmpty("SUBLIBS_DIR"))
	    libdir = project->first("SUBLIBS_DIR");
	t << "SUBLIBS= ";
	QStringList &l = project->variables()["SUBLIBS"];
	for(QStringList::Iterator it = l.begin(); it != l.end(); ++it)
	    t << libdir << "lib" << (*it) << ".a ";
	t << endl << endl;
    }
    if(project->isActiveConfig("depend_prl") && !project->isEmpty("QMAKE_PRL_INTERNAL_FILES")) {
	QStringList &l = project->variables()["QMAKE_PRL_INTERNAL_FILES"];
	QStringList::Iterator it;
	for(it = l.begin(); it != l.end(); ++it) {
	    QMakeProject proj;
	    if(proj.read((*it), QDir::currentDirPath()) && !proj.isEmpty("QMAKE_PRL_BUILD_DIR")) {
		QString dir;
		int slsh = (*it).findRev(Option::dir_sep);
		if(slsh != -1)
		    dir = (*it).left(slsh + 1);
		QString targ = dir + proj.first("QMAKE_PRL_TARGET");
		deps += " " + targ;
		t << targ << ":" << "\n\t"
		  << "@echo \"Creating '" << targ << "'\"" << "\n\t"
		  << "(cd " << proj.first("QMAKE_PRL_BUILD_DIR") << ";"
		  << "$(MAKE) )" << endl;
	    }
	}
    }
    if(!project->variables()["QMAKE_APP_FLAG"].isEmpty()) {
	QString destdir = project->first("DESTDIR");
	if(do_incremental) {
	    //incremental target
	    QString incr_target = var("TARGET") + "_incremental";
	    if(incr_target.find(Option::dir_sep) != -1)
		incr_target = incr_target.right(incr_target.length() -
						(incr_target.findRev(Option::dir_sep) + 1));
	    QString incr_deps, incr_objs;
	    if(project->first("QMAKE_INCREMENTAL_STYLE") == "ld") {
		QString incr_target_dir = var("OBJECTS_DIR") + incr_target + Option::obj_ext;
		//actual target
		t << incr_target_dir << ": $(OBJECTS)" << "\n\t"
		  << "ld -r  -o "<< incr_target_dir << " $(OBJECTS)" << endl;
		//communicated below
		deps.prepend(incr_target_dir + " ");
		incr_deps = "$(UICDECLS) $(INCREMENTAL_OBJECTS) $(INCREMENTAL_OBJMOC) $(OBJMOC)";
		if(!incr_objs.isEmpty())
		    incr_objs += " ";
		incr_objs += incr_target_dir;
	    } else {
		//actual target
		QString incr_target_dir = var("DESTDIR") + "lib" + incr_target + "." +
					  project->variables()["QMAKE_EXTENSION_SHLIB"].first();
		QString incr_lflags = var("QMAKE_LFLAGS_SHLIB") + " ";
		if(project->isActiveConfig("debug"))
		    incr_lflags += var("QMAKE_LFLAGS_DEBUG");
		else
		    incr_lflags += var("QMAKE_LFLAGS_RELEASE");
		t << incr_target_dir << ": $(INCREMENTAL_OBJECTS) $(INCREMENTAL_OBJMOC)" << "\n\t";
		if(!destdir.isEmpty())
		    t << "\n\t" << "test -d " << destdir << " || mkdir -p " << destdir << "\n\t";
		t << "$(LINK) " << incr_lflags << " -o "<< incr_target_dir <<
		    " $(INCREMENTAL_OBJECTS) $(INCREMENTAL_OBJMOC)" << endl;
		//communicated below
		if(!destdir.isEmpty()) {
		    if(!incr_objs.isEmpty())
			incr_objs += " ";
		    incr_objs += "-L" + destdir;
		} else {
		    if(!incr_objs.isEmpty())
			incr_objs += " ";
		    incr_objs += "-L" + QDir::currentDirPath();
		}
		if(!incr_objs.isEmpty())
		    incr_objs += " ";
		incr_objs += " -l" + incr_target;
		deps.prepend(incr_target_dir + " ");
		incr_deps = "$(UICDECLS) $(OBJECTS) $(OBJMOC)";
	    }
	    t << "all: " << deps <<  " " << varGlue("ALL_DEPS",""," "," ") <<  "$(TARGET)"
	      << endl << endl;

	    //real target
	    t << var("TARGET") << ": " << " " << incr_deps << " " << var("TARGETDEPS") << "\n\t";
	    if(!destdir.isEmpty())
		t << "\n\t" << "test -d " << destdir << " || mkdir -p " << destdir << "\n\t";
	    if(!project->isEmpty("QMAKE_PRE_LINK"))
		t << var("QMAKE_PRE_LINK") << "\n\t";
	    t << "$(LINK) $(LFLAGS) -o $(TARGET) " << incr_deps << " " << incr_objs << " $(LIBS)";
	    if(!project->isEmpty("QMAKE_POST_LINK"))
		t << "\n\t" << var("QMAKE_POST_LINK");
	    t << endl << endl;
	} else {
	    t << "all: " << deps <<  " " << varGlue("ALL_DEPS",""," "," ") <<  "$(TARGET)"
	      << endl << endl;

	    t << "$(TARGET): $(UICDECLS) $(OBJECTS) $(OBJMOC) " << var("TARGETDEPS") << "\n\t";
	    if(!destdir.isEmpty())
		t << "test -d " << destdir << " || mkdir -p " << destdir << "\n\t";
	    if(!project->isEmpty("QMAKE_PRE_LINK"))
		t << var("QMAKE_PRE_LINK") << "\n\t";
	    t << "$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJMOC) $(LIBS)";
	    if(!project->isEmpty("QMAKE_POST_LINK"))
		t << "\n\t" << var("QMAKE_POST_LINK");
	    t << endl << endl;
	}
    } else if(!project->isActiveConfig("staticlib")) {
	QString destdir = project->first("DESTDIR"), incr_deps;
	if(do_incremental) {
	    QString s_ext = project->variables()["QMAKE_EXTENSION_SHLIB"].first();
	    QString incr_target = var("QMAKE_ORIG_TARGET").replace(
		QRegExp("\\." + s_ext), "").replace(QRegExp("^lib"), "") + "_incremental";
	    if(incr_target.find(Option::dir_sep) != -1)
		incr_target = incr_target.right(incr_target.length() -
						(incr_target.findRev(Option::dir_sep) + 1));

	    if(project->first("QMAKE_INCREMENTAL_STYLE") == "ld") {
		QString incr_target_dir = var("OBJECTS_DIR") + incr_target + Option::obj_ext;
		//actual target
		const QString link_deps = "$(UICDECLS) $(OBJECTS) $(OBJMOC)";
		t << incr_target_dir << ": " << link_deps << "\n\t"
		  << "ld -r  -o " << incr_target_dir << " " << link_deps << endl;
		//communicated below
		QStringList &cmd = project->variables()["QMAKE_LINK_SHLIB_CMD"];
		cmd.first().replace(QRegExp("\\$\\(OBJECTS\\) \\$\\(OBJMOC\\)"),
				    "$(INCREMENTAL_OBJECTS) $(INCREMENTAL_OBJMOC)"); //ick
		cmd.append(incr_target_dir);
		deps.prepend(incr_target_dir + " ");
		incr_deps = "$(INCREMENTAL_OBJECTS) $(INCREMENTAL_OBJMOC)";
	    } else {
		//actual target
		QString incr_target_dir = var("DESTDIR") + "lib" + incr_target + "." + s_ext;
		QString incr_lflags = var("QMAKE_LFLAGS_SHLIB") + " ";
		if(!project->isEmpty("QMAKE_LFLAGS_INCREMENTAL"))
		    incr_lflags += var("QMAKE_LFLAGS_INCREMENTAL") + " ";
		if(project->isActiveConfig("debug"))
		    incr_lflags += var("QMAKE_LFLAGS_DEBUG");
		else
		    incr_lflags += var("QMAKE_LFLAGS_RELEASE");
		t << incr_target_dir << ": $(INCREMENTAL_OBJECTS) $(INCREMENTAL_OBJMOC)" << "\n\t";
		if(!destdir.isEmpty())
		    t << "test -d " << destdir << " || mkdir -p " << destdir << "\n\t";
		t << "$(LINK) " << incr_lflags << " -o "<< incr_target_dir <<
		    " $(INCREMENTAL_OBJECTS) $(INCREMENTAL_OBJMOC)" << endl;
		//communicated below
		QStringList &cmd = project->variables()["QMAKE_LINK_SHLIB_CMD"];
		if(!destdir.isEmpty())
		    cmd.append(" -L" + destdir);
		cmd.append(" -l" + incr_target);
		deps.prepend(incr_target_dir + " ");
		incr_deps = "$(UICDECLS) $(OBJECTS) $(OBJMOC)";
	    }

	    t << "all: " << " " << deps << " " << varGlue("ALL_DEPS",""," ","")
	      << " " <<  var("DESTDIR_TARGET") << endl << endl;

	    //real target
	    t << var("DESTDIR_TARGET") << ": " << incr_deps << " $(SUBLIBS) " <<
		var("TARGETDEPS");
	} else {
	    t << "all: " << deps << " " << varGlue("ALL_DEPS",""," ","") << " " <<
		var("DESTDIR_TARGET") << endl << endl;
	    t << var("DESTDIR_TARGET") << ":  $(UICDECLS) $(OBJECTS) $(OBJMOC) $(SUBLIBS) " <<
		var("TARGETDEPS");
	}
	if(!destdir.isEmpty())
	    t << "\n\t" << "test -d " << destdir << " || mkdir -p " << destdir;
	if(!project->isEmpty("QMAKE_PRE_LINK"))
	    t << "\n\t" << var("QMAKE_PRE_LINK");

	if(project->isActiveConfig("plugin")) {
	    t << "\n\t"
	      << "-rm -f $(TARGET)" << "\n\t"
	      << var("QMAKE_LINK_SHLIB_CMD");
	    if(!destdir.isEmpty())
		t << "\n\t"
		  << "-mv $(TARGET) " << var("DESTDIR");
	    if(!project->isEmpty("QMAKE_POST_LINK"))
		t << "\n\t" << var("QMAKE_POST_LINK") << "\n\t";
	    t << endl << endl;
	} else if(project->variables()["QMAKE_HPUX_SHLIB"].isEmpty()) {
	    t << "\n\t"
	      << "-rm -f $(TARGET) $(TARGET0) $(TARGET1) $(TARGET2)" << "\n\t"
	      << var("QMAKE_LINK_SHLIB_CMD") << "\n\t";
	    t << varGlue("QMAKE_LN_SHLIB",""," "," $(TARGET) $(TARGET0)")  << "\n\t"
	      << varGlue("QMAKE_LN_SHLIB",""," "," $(TARGET) $(TARGET1)") << "\n\t"
	      << varGlue("QMAKE_LN_SHLIB",""," "," $(TARGET) $(TARGET2)");
	    if(!destdir.isEmpty())
		t << "\n\t"
		  << "-rm -f " << var("DESTDIR") << "$(TARGET)\n\t"
		  << "-rm -f " << var("DESTDIR") << "$(TARGET0)\n\t"
		  << "-rm -f " << var("DESTDIR") << "$(TARGET1)\n\t"
		  << "-rm -f " << var("DESTDIR") << "$(TARGET2)\n\t"
		  << "-mv $(TARGET) $(TARGET0) $(TARGET1) $(TARGET2) " << var("DESTDIR");
	    if(!project->isEmpty("QMAKE_POST_LINK"))
		t << "\n\t" << var("QMAKE_POST_LINK");
	    t << endl << endl;
	} else {
	    t << "\n\t"
	      << "-rm -f $(TARGET) $(TARGET0)" << "\n\t"
	      << var("QMAKE_LINK_SHLIB_CMD") << "\n\t";
	    t << varGlue("QMAKE_LN_SHLIB",""," "," $(TARGET) $(TARGET0)");
	    if(!destdir.isEmpty())
		t  << "\n\t"
		   << "-rm -f " << var("DESTDIR") << "$(TARGET)\n\t"
		   << "-rm -f " << var("DESTDIR") << "$(TARGET0)\n\t"
		   << "-mv $(TARGET) $(TARGET0) " << var("DESTDIR");
	    if(!project->isEmpty("QMAKE_POST_LINK"))
		t << "\n\t" << var("QMAKE_POST_LINK");
	    t << endl << endl;
	}
	t << endl << endl;

	if (! project->isActiveConfig("plugin")) {
	    t << "staticlib: $(TARGETA)" << endl << endl;
	    t << "$(TARGETA): $(UICDECLS) $(OBJECTS) $(OBJMOC)";
	    if(do_incremental)
		t << " $(INCREMENTAL_OBJECTS) $(INCREMENTAL_OBJMOC)";
	    t << var("TARGETDEPS") << "\n\t"
	      << "-rm -f $(TARGETA) " << "\n\t"
	      << var("QMAKE_AR_CMD");
	    if(do_incremental)
		t << " $(INCREMENTAL_OBJECTS) $(INCREMENTAL_OBJMOC)";
	    if(!project->isEmpty("QMAKE_RANLIB"))
		t << "\n\t" << "$(RANLIB) $(TARGETA)";
	    t << endl << endl;
	}
    } else {
	t << "all: " << deps << " " << varGlue("ALL_DEPS",""," "," ") << "$(TARGET)" << "\n\n"
	  << "staticlib: $(TARGET)" << "\n\n"
	  << "$(TARGET): $(UICDECLS) $(OBJECTS) $(OBJMOC) " << var("TARGETDEPS") << "\n\t";

	if(!project->isEmpty("DESTDIR")) {
	    QString destdir = project->first("DESTDIR");
	    t << "test -d " << destdir << " || mkdir -p " << destdir << "\n\t";
	}
	t << "-rm -f $(TARGET)" << "\n\t"
	  << var("QMAKE_AR_CMD") << "\n\t";
	if(!project->isEmpty("QMAKE_POST_LINK"))
	    t << var("QMAKE_POST_LINK") << "\n\t";

	if(!project->isEmpty("QMAKE_RANLIB"))
	    t << "$(RANLIB) $(TARGET)";
	t << endl << endl;
    }

    t << "mocables: $(SRCMOC)" << endl << endl;

    if(!project->isActiveConfig("no_mocdepend")) {
	//this is an implicity depend on moc, so it will be built if necesary, however
	//moc itself shouldn't have this dependancy - this is a little kludgy but it is
	//better than the alternative for now.
	QString moc = project->first("QMAKE_MOC"), target = project->first("TARGET");
	fixEnvVariables(target);
	fixEnvVariables(moc);
	if(target != moc) 
	    t << "$(MOC): \n\t"
	      << "( cd $(QTDIR)/src/moc ; $(MAKE) )"  << endl << endl;
    }

    writeMakeQmake(t);

    if(!project->first("QMAKE_PKGINFO").isEmpty()) {
	QString pkginfo = project->first("QMAKE_PKGINFO");
	QString destdir = project->first("DESTDIR");
	t << pkginfo << ": " << "\n\t";
	if(!destdir.isEmpty())
	    t << "@test -d " << destdir << " || mkdir -p " << destdir << "\n\t";
	t << "@rm -f " << pkginfo << "\n\t"
	  << "@echo \"APPL????\" >" << pkginfo << endl;
    }
    if(!project->first("QMAKE_INFO_PLIST").isEmpty()) {
	QString info_plist = project->first("QMAKE_INFO_PLIST"),
	       info_plist_out = project->first("QMAKE_INFO_PLIST_OUT");
	QString destdir = project->first("DESTDIR");
	t << info_plist_out << ": " << "\n\t";
	if(!destdir.isEmpty())
	    t << "@test -d " << destdir << " || mkdir -p " << destdir << "\n\t";
	t << "@rm -f " << info_plist_out << "\n\t"
	  << "@cp \"" << info_plist << "\" \"" << info_plist_out << "\"" << endl;
	if(!project->first("RC_FILE").isEmpty()) {
	    QString dir = destdir + "../Resources/";
	    t << dir << "application.icns:" << "\n\t"
	      << "@test -d " << dir << " || mkdir -p " << dir << "\n\t"
	      << "@cp " << var("RC_FILE") << " " << dir << "application.icns" << endl;
	}
    }

    QString ddir = project->isEmpty("QMAKE_DISTDIR") ? project->first("QMAKE_ORIG_TARGET") :
		   project->first("QMAKE_DISTDIR");
    QString ddir_c = (project->isEmpty("OBJECTS_DIR") ? QString(".tmp/") :
		    project->first("OBJECTS_DIR")) + ddir;
    fileFixify(ddir_c);
    t << "dist: " << "\n\t"
      << "@mkdir -p " << ddir_c << " && "
      << "$(COPY_FILE) --parents $(SOURCES) $(HEADERS) $(FORMS) $(DIST) " << ddir_c << Option::dir_sep << " && "
      << "( cd `dirname " << ddir_c << "` && "
      << "$(TAR) " << var("QMAKE_ORIG_TARGET") << ".tar " << ddir << " && "
      << "$(GZIP) " << var("QMAKE_ORIG_TARGET") << ".tar ) && "
      << "mv `dirname " << ddir_c << "`" << Option::dir_sep << var("QMAKE_ORIG_TARGET") << ".tar.gz . && "
      << "rm -rf " << ddir_c
      << endl << endl;

    QString clean_targets;
    if(mocAware()) {
	t << "mocclean:" << "\n";
	if(!objMoc.isEmpty() || !srcMoc.isEmpty()) {
	    if(!objMoc.isEmpty())
		t << "\t-rm -f $(OBJMOC)" << '\n';
	    if(!srcMoc.isEmpty())
		t << "\t-rm -f $(SRCMOC)" << '\n';
	    clean_targets += " mocclean";
	}
	t << endl;
    }
    t << "uiclean:" << "\n";
    if (!var("UICIMPLS").isEmpty() || !var("UICDECLS").isEmpty()) {
	t << "\t-rm -f $(UICIMPLS) $(UICDECLS)" << "\n";
	clean_targets += " uiclean";
    }
    t << endl;

    t << "clean:" << clean_targets << "\n\t"
      << "-rm -f $(OBJECTS) " << "\n\t";
    if(!project->isEmpty("IMAGES"))
	t << varGlue("QMAKE_IMAGE_COLLECTION", "\t-rm -f ", " ", "") << "\n\t";
    if(do_incremental)
	t << "-rm -f $(INCREMENTAL_OBJECTS) $(INCREMENTAL_OBJMOC)" << "\n\t";
    t << varGlue("QMAKE_CLEAN","-rm -f "," ","\n\t")
      << "-rm -f *~ core *.core" << "\n"
      << varGlue("CLEAN_FILES","\t-rm -f "," ","") << endl << endl;
    t << "####### Sub-libraries" << endl << endl;
    if ( !project->variables()["SUBLIBS"].isEmpty() ) {
	QString libdir = "tmp/";
	if(!project->isEmpty("SUBLIBS_DIR"))
	    libdir = project->first("SUBLIBS_DIR");
	QStringList &l = project->variables()["SUBLIBS"];
	for(it = l.begin(); it != l.end(); ++it)
	    t << libdir << "lib" << (*it) << ".a" << ":\n\t"
	      << var(QString("MAKELIB") + (*it)) << endl << endl;
    }

    QString destdir = project->first("DESTDIR");
    if(!destdir.isEmpty() && destdir.right(1) != Option::dir_sep)
	destdir += Option::dir_sep;
    t << "distclean: " << "clean\n\t"
      << "-rm -f " << destdir << "$(TARGET)" << " " << "$(TARGET)" << "\n\t";
    if(!project->isActiveConfig("staticlib") && project->variables()["QMAKE_APP_FLAG"].isEmpty() &&
       !project->isActiveConfig("plugin"))
	t << "-rm -f " << destdir << "$(TARGET0) " << destdir << "$(TARGET1) "
	  << destdir << "$(TARGET2) $(TARGETA)" << "\n\t";
    t << endl << endl;

    if ( !project->isEmpty("PRECOMPH") ) {
	QString outdir = project->first("MOC_DIR");
	QString qt_dot_h = Option::fixPathToLocalOS(project->first("PRECOMPH"));
	t << "###### Combined headers" << endl << endl;
	//XXX
	t << outdir << "allmoc.cpp: " << qt_dot_h << " "
	  << varList("HEADERS_ORIG") << "\n\t"
	  << "echo '#include \"" << qt_dot_h << "\"' >" << outdir << "allmoc.cpp" << "\n\t"
	  << "$(CXX) -E -DQT_MOC_CPP -DQT_NO_STL $(CXXFLAGS) $(INCPATH) >" << outdir << "allmoc.h "
	     << outdir << "allmoc.cpp" << "\n\t"
	  << "$(MOC) -o " << outdir << "allmoc.cpp " << outdir << "allmoc.h" << "\n\t"
	  << "perl -pi -e 's{#include \"allmoc.h\"}{#define QT_H_CPP\\n#include \""
	     << qt_dot_h << "\"}' " << outdir << "allmoc.cpp" << "\n\t"
	  << "rm " << outdir << "allmoc.h" << endl << endl;
    }

    // blasted user defined targets
    QStringList &qut = project->variables()["QMAKE_EXTRA_UNIX_TARGETS"];
    for(it = qut.begin(); it != qut.end(); ++it) {
	QString targ = var((*it) + ".target"),
		 cmd = var((*it) + ".commands"), deps;
	if(targ.isEmpty())
	    targ = (*it);
	QStringList &depends = project->variables()[(*it) + ".depends"];
	for(QStringList::Iterator dep_it = depends.begin(); dep_it != depends.end(); ++dep_it) {
	    QString dep = var((*dep_it) + ".target");
	    if(dep.isEmpty())
		dep = (*dep_it);
	    deps += " " + dep;
	}
	t << targ << ":" << deps << "\n\t"
	  << cmd << endl << endl;
    }
    t <<"FORCE:" << endl << endl;
}

void
UnixMakefileGenerator::writeSubdirs(QTextStream &t, bool direct)
{
    QString ofile = Option::output.name();
    if(ofile.findRev(Option::dir_sep) != -1)
	ofile = ofile.right(ofile.length() - ofile.findRev(Option::dir_sep) -1);

    t << "MAKEFILE =	" << var("MAKEFILE") << endl;
    t << "QMAKE =	" << var("QMAKE") << endl;
    t << "SUBDIRS =	" << varList("SUBDIRS") << endl;

    // subdirectory targets are sub-directory
    QStringList::Iterator it;
    QStringList subdirs = project->variables()["SUBDIRS"];
    t << "SUBTARGETS =	";
    for(it = subdirs.begin(); it != subdirs.end(); ++it) {
	QString sr = (*it);
	sr.replace(QRegExp("/"), "-");
	t << " \\\n\t\tsub-" << sr;
    }
    t << endl << endl;

    t << "first: all\n\nall: " << ofile << " $(SUBTARGETS)" << endl << endl;

    // generate target rules
    for(it = subdirs.begin(); it != subdirs.end(); ++it) {
	QString sr = (*it), mkfile = (*it) + Option::dir_sep + "$(MAKEFILE)", out;
	if(direct)
	    out = " -o $(MAKEFILE)";
	sr.replace(QRegExp("/"), "-");
	//qmake it
	t << mkfile << ": " << "\n\t"
	  << "cd " << (*it) << " && $(QMAKE)" << buildArgs() << out << endl;
	//actually compile
	t << "sub-" << sr << ": " << mkfile << " FORCE" << "\n\t"
	  << "cd " << *(it) << " && $(MAKE) -f $(MAKEFILE)" << endl << endl;
    }

    if (project->isActiveConfig("ordered")) {
	// generate dependencies
	QString tar, dep;
	it = subdirs.begin();
	while (it != subdirs.end()) {
	    tar = *it++;
            tar.replace(QRegExp("/"), "-");
	    if (it != subdirs.end()) {
		dep = *it;
                dep.replace(QRegExp("/"), "-");
		t << "sub-" << dep << ": sub-" << tar << endl;
	    }
	}
	t << endl;
    }

    writeMakeQmake(t);

    if(project->isEmpty("SUBDIRS")) {
	t << "all qmake_all distclean install uiclean mocclean clean: FORCE" << endl;
    } else {
	t << "qmake_all: " << varGlue("SUBDIRS", "", Option::dir_sep + "$(MAKEFILE) ",
				      Option::dir_sep + "$(MAKEFILE)") << "\n\t"
	  << "for i in $(SUBDIRS); do ( if [ -d $$i ]; then cd $$i ; "
	  << "grep \"^qmake_all:\" $(MAKEFILE) 2>/dev/null >/dev/null && "
	  << "$(MAKE) -f $(MAKEFILE) qmake_all || true; fi; ) ; done" << endl << endl;

	t << "clean: qmake_all FORCE" << "\n\t"
	  << varGlue("QMAKE_CLEAN","-rm -f "," ","\n\t")
	  << "for i in $(SUBDIRS); do ( if [ -d $$i ]; then cd $$i ; $(MAKE) "
	    "-f $(MAKEFILE) clean; fi; ) ; done" << endl;
	t <<"uninstall install uiclean mocclean: qmake_all FORCE" << "\n\t"
	  << "for i in $(SUBDIRS); do ( if [ -d $$i ]; then cd $$i ; $(MAKE) "
	    "-f $(MAKEFILE) $@; fi; ) ; done" << endl;
	t <<"distclean: qmake_all" << " FORCE\n\t"
	  << "for i in $(SUBDIRS); do ( if [ -d $$i ]; then cd $$i ; $(MAKE) "
	    "-f $(MAKEFILE) $@ ; rm -f $(MAKEFILE) ; fi; ) ; done"
	  << endl << endl;
    }
    t <<"FORCE:" << endl << endl;
}

void UnixMakefileGenerator::init2()
{
    //version handling
    if(project->variables()["VERSION"].isEmpty())
	project->variables()["VERSION"].append("1.0." +
					       (project->isEmpty("VER_PAT") ? QString("0") :
						project->first("VER_PAT")) );
    QStringList l = QStringList::split('.', project->first("VERSION")) << "0" << "0"; //make sure there are three
    project->variables()["VER_MAJ"].append(l[0]);
    project->variables()["VER_MIN"].append(l[1]);
    project->variables()["VER_PAT"].append(l[2]);

    if ( !project->variables()["QMAKE_APP_FLAG"].isEmpty() ) {
#if 0
	if ( project->isActiveConfig("dll") ) {
	    project->variables()["TARGET"] += project->variables()["TARGET.so"];
	    if(project->variables()["QMAKE_LFLAGS_SHAPP"].isEmpty())
		project->variables()["QMAKE_LFLAGS_SHAPP"] += project->variables()["QMAKE_LFLAGS_SHLIB"];
	    if(!project->variables()["QMAKE_LFLAGS_SONAME"].isEmpty())
		project->variables()["QMAKE_LFLAGS_SONAME"].first() += project->first("TARGET");
	}
#endif
	project->variables()["TARGET"].first().prepend(project->first("DESTDIR"));
    } else if ( project->isActiveConfig("staticlib") ) {
	project->variables()["TARGET"].first().prepend(project->first("DESTDIR") + "lib");
	project->variables()["TARGET"].first() += ".a";
	if(project->variables()["QMAKE_AR_CMD"].isEmpty())
	    project->variables()["QMAKE_AR_CMD"].append("$(AR) $(TARGET) $(OBJECTS) $(OBJMOC)");
    } else {
	project->variables()["TARGETA"].append(project->first("DESTDIR") + "lib" + project->first("TARGET") + ".a");
	if ( !project->variables()["QMAKE_AR_CMD"].isEmpty() )
	    project->variables()["QMAKE_AR_CMD"].first().replace(QRegExp("\\(TARGET\\)"),"(TARGETA)");
	else
	    project->variables()["QMAKE_AR_CMD"].append("$(AR) $(TARGETA) $(OBJECTS) $(OBJMOC)");
	if( project->isActiveConfig("plugin") ) {
	    project->variables()["TARGET_x.y.z"].append("lib" +
							project->first("TARGET") + "." + project->first("QMAKE_EXTENSION_SHLIB"));
	    if(project->isActiveConfig("lib_version_first"))
		project->variables()["TARGET_x"].append("lib" + project->first("TARGET") + "." +
							project->first("VER_MAJ") + "." + 
							project->first("QMAKE_EXTENSION_SHLIB"));
	    else
		project->variables()["TARGET_x"].append("lib" + project->first("TARGET") + "." +
							project->first("QMAKE_EXTENSION_SHLIB") +
							"." + project->first("VER_MAJ"));

	    project->variables()["TARGET"] = project->variables()["TARGET_x.y.z"];
	    if(project->isActiveConfig("qt"))
		project->variables()["DEFINES"].append("QT_PLUGIN");
	} else if ( !project->variables()["QMAKE_HPUX_SHLIB"].isEmpty() ) {
	    project->variables()["TARGET_"].append("lib" + project->first("TARGET") + ".sl");
	    if(project->isActiveConfig("lib_version_first"))
		project->variables()["TARGET_x"].append("lib" + project->first("VER_MAJ") + "." + 
							project->first("TARGET"));
	    else
		project->variables()["TARGET_x"].append("lib" + project->first("TARGET") + "." + 
							project->first("VER_MAJ"));
	    project->variables()["TARGET"] = project->variables()["TARGET_x"];
	} else if ( !project->variables()["QMAKE_AIX_SHLIB"].isEmpty() ) {
	    project->variables()["TARGET_"].append("lib" + project->first("TARGET") + ".a");
	    if(project->isActiveConfig("lib_version_first")) {
		project->variables()["TARGET_x"].append("lib" + project->first("TARGET") + "." +
							project->first("VER_MAJ") + "." +
							project->first("QMAKE_EXTENSION_SHLIB"));
		project->variables()["TARGET_x.y"].append("lib" + project->first("TARGET") + "." +
							  project->first("VER_MAJ") +
							  "." + project->first("VER_MIN") + "." +
							  project->first("QMAKE_EXTENSION_SHLIB"));
		project->variables()["TARGET_x.y.z"].append("lib" + project->first("TARGET") + "." +
							    project->first("VER_MAJ") + "." +
							    project->first("VER_MIN") + "." +
							    project->first("VER_PAT") + "." +
							    project->first("QMAKE_EXTENSION_SHLIB"));
	    } else {
		project->variables()["TARGET_x"].append("lib" + project->first("TARGET") + "." +
							project->first("QMAKE_EXTENSION_SHLIB") +
							"." + project->first("VER_MAJ"));
		project->variables()["TARGET_x.y"].append("lib" + project->first("TARGET") + "." +
							  project->first("QMAKE_EXTENSION_SHLIB") +
							  "." + project->first("VER_MAJ") +
							  "." + project->first("VER_MIN"));
		project->variables()["TARGET_x.y.z"].append("lib" + project->first("TARGET") + "." +
							    project->first("QMAKE_EXTENSION_SHLIB") + "." +
							    project->first("VER_MAJ") + "." +
							    project->first("VER_MIN") + "." +
							    project->first("VER_PAT"));
	    }
	    project->variables()["TARGET"] = project->variables()["TARGET_x.y.z"];
	} else {
	    project->variables()["TARGET_"].append("lib" + project->first("TARGET") + "." +
						   project->first("QMAKE_EXTENSION_SHLIB"));
	    if(project->isActiveConfig("lib_version_first")) {
		project->variables()["TARGET_x"].append("lib" + project->first("TARGET") + "." +
							project->first("VER_MAJ") + "." +
							project->first("QMAKE_EXTENSION_SHLIB"));
		project->variables()["TARGET_x.y"].append("lib" + project->first("TARGET") + "." +
							  project->first("VER_MAJ") +
							  "." + project->first("VER_MIN") + "." +
							  project->first("QMAKE_EXTENSION_SHLIB"));
		project->variables()["TARGET_x.y.z"].append("lib" + project->first("TARGET") + "." +
							    project->first("VER_MAJ") + "." +
							    project->first("VER_MIN") +  "." +
							    project->first("VER_PAT") + "." +
							    project->variables()["QMAKE_EXTENSION_SHLIB"].first());
	    } else {
		project->variables()["TARGET_x"].append("lib" + project->first("TARGET") + "." +
							project->first("QMAKE_EXTENSION_SHLIB") +
							"." + project->first("VER_MAJ"));
		project->variables()["TARGET_x.y"].append("lib" + project->first("TARGET") + "." +
						      project->first("QMAKE_EXTENSION_SHLIB")
						      + "." + project->first("VER_MAJ") +
						      "." + project->first("VER_MIN"));
		project->variables()["TARGET_x.y.z"].append("lib" + project->first("TARGET") +
							    "." +
							    project->variables()[
								"QMAKE_EXTENSION_SHLIB"].first() + "." +
							    project->first("VER_MAJ") + "." +
							    project->first("VER_MIN") +  "." +
							    project->first("VER_PAT"));
	    }
	    project->variables()["TARGET"] = project->variables()["TARGET_x.y.z"];
	}
	project->variables()["QMAKE_LN_SHLIB"].append("-ln -s");
	project->variables()["DESTDIR_TARGET"].append("$(TARGET)");
	if ( !project->variables()["DESTDIR"].isEmpty() )
	    project->variables()["DESTDIR_TARGET"].first().prepend(project->first("DESTDIR"));
	if ( !project->variables()["QMAKE_LFLAGS_SONAME"].isEmpty() && !project->variables()["TARGET_x"].isEmpty() )
	    project->variables()["QMAKE_LFLAGS_SONAME"].first() += project->first("TARGET_x");
	if ( project->variables()["QMAKE_LINK_SHLIB_CMD"].isEmpty() )
	    project->variables()["QMAKE_LINK_SHLIB_CMD"].append(
		"$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJMOC) $(LIBS)");
    }
    if ( !project->variables()["QMAKE_APP_FLAG"].isEmpty() ) {
	project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_SHAPP"];
    } else if ( project->isActiveConfig("dll") ) {
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_SHLIB"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_SHLIB"];
	if ( project->isActiveConfig("plugin") ) {
	    project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_PLUGIN"];
	    project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_SONAME"];
	} else {
	    project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_SHLIB"];
	    project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_SONAME"];
	}
	QString destdir = project->first("DESTDIR");
	if ( !destdir.isEmpty() && !project->variables()["QMAKE_RPATH"].isEmpty() )
	    project->variables()["QMAKE_LFLAGS"] += project->first("QMAKE_RPATH") + destdir;
    }
}
