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
    QString ofile = Option::output.name();
    if(ofile.findRev(Option::dir_sep) != -1)
	ofile = ofile.right(ofile.length() - ofile.findRev(Option::dir_sep) -1);
    bool do_incremental = (project->isActiveConfig("incremental") &&
			   !project->variables()["QMAKE_INCREMENTAL"].isEmpty() &&
			   (!project->variables()["QMAKE_APP_FLAG"].isEmpty() ||
			    !project->isActiveConfig("staticlib")));

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
    t << "CFLAGS   = " << var("QMAKE_CFLAGS") << " " << varGlue("DEFINES","-D"," -D","") << endl;
    t << "CXXFLAGS = " << var("QMAKE_CXXFLAGS") << " " << varGlue("DEFINES","-D"," -D","") << endl;
    t << "LEXFLAGS = " << var("QMAKE_LEXFLAGS") << endl;
    t << "YACCFLAGS= " << var("QMAKE_YACCFLAGS") << endl;
    t << "INCPATH  = " << varGlue("INCLUDEPATH","-I", " -I", "") << " -I" << Option::mkfile::qmakespec << endl;

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
    t << "DEL      = " << var("QMAKE_DEL") << endl;
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
	    do_incremental = FALSE;
	} else if(!incrs_out.count()) {
	    t << endl;
	    do_incremental = FALSE;
	} else {
	    t << endl;
	    t << "INCREMENTAL_OBJECTS = " << incrs_out.join(" \\\n\t\t") << endl;
	}
    } else {
	t << "OBJECTS = " << varList("OBJECTS") << endl;
    }
    t << "FORMS = " << varList("FORMS") << endl;
    QString uicDecls = varList("UICDECLS");
    QString uicImpls = varList("UICIMPLS");
    QString srcMoc = varList("SRCMOC");
    QString objMoc = varList("OBJMOC");
    t << "UICDECLS = " << uicDecls << endl;
    t << "UICIMPLS = " << uicImpls << endl;
    t << "SRCMOC   = " << srcMoc << endl;
    t << "OBJMOC = " << objMoc << endl;
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


    /* rules */
    t << "first: all" << endl;
    t << "####### Implicit rules" << endl << endl;
    t << ".SUFFIXES: .cpp .cxx .cc .C .c" << endl << endl;
    t << ".cpp.o:\n\t" << var("QMAKE_RUN_CXX_IMP") << endl << endl;
    t << ".cxx.o:\n\t" << var("QMAKE_RUN_CXX_IMP") << endl << endl;
    t << ".cc.o:\n\t" << var("QMAKE_RUN_CXX_IMP") << endl << endl;
    t << ".C.o:\n\t" << var("QMAKE_RUN_CXX_IMP") << endl << endl;
    t << ".c.o:\n\t" << var("QMAKE_RUN_CC_IMP") << endl << endl;

    if(include_deps) {
	QString cmd=var("QMAKE_CFLAGS_DEPS") + " ";
	cmd += varGlue("DEFINES","-D"," -D","");
	if(!project->isEmpty("QMAKE_ABSOLUTE_SOURCE_PATH"))
	    cmd += " -I" + project->first("QMAKE_ABSOLUTE_SOURCE_PATH") + " ";
	cmd += " $(INCPATH) " + varGlue("DEPENDPATH", "-I", " -I", "");
	QString odir;
	if(!project->variables()["OBJECTS_DIR"].isEmpty())
	    odir = project->first("OBJECTS_DIR");
	t << "###### Dependancies" << endl << endl;
	t << odir << ".deps/%.d: %.cpp\n\t"
	  << "@echo Creating depend for $<" << "\n\t"
	  << "@[ -d $(@D) ] || mkdir -p $(@D)" << "\n\t"
	  << "@$(CXX) " << cmd << " $< | sed \"s,^\\($(*F).o\\):," << odir << "\\1:,g\" >$@" << endl << endl;

	t << odir << ".deps/%.d: %.c\n\t"
	  << "@echo Creating depend for $<" << "\n\t"
	  << "@[ -d $(@D) ] || mkdir -p $(@D)" << "\n\t"
	  << "@$(CC) " << cmd << " $< | sed \"s,^\\($(*F).o\\):," << odir << "\\1:,g\" >$@" << endl << endl;


	QString src[] = { "SOURCES", "UICIMPLS", "SRCMOC", QString::null };
	for(int x = 0; !src[x].isNull(); x++) {
	    QStringList &l = project->variables()[src[x]];
	    for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
		if(!(*it).isEmpty()) {
		    QString d_file;
		    if((*it).right(2) == ".c")
			d_file = (*it).left((*it).length() - 2);
		    else if((*it).right(Option::cpp_ext.length()) == Option::cpp_ext)
			d_file = (*it).left((*it).length() - Option::cpp_ext.length());
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
	t << "SUBLIBS= ";
	QStringList &l = project->variables()["SUBLIBS"];
	for(QStringList::Iterator it = l.begin(); it != l.end(); ++it)
	    t << "tmp/lib" << (*it) << ".a ";
	t << endl << endl;
    }
    if(!project->variables()["QMAKE_APP_FLAG"].isEmpty()) {
	t << "all: " << ofile <<  " " << varGlue("ALL_DEPS",""," "," ") <<  "$(TARGET)" << endl << endl;

	QString destdir = project->first("DESTDIR");
	if(do_incremental) {
	    //utility variables
	    QString s_ext = project->variables()["QMAKE_EXTENSION_SHLIB"].first();
	    QString incr_target = var("TARGET") + "_incremental";
	    incr_target = incr_target.right(incr_target.length() - (incr_target.findRev(Option::dir_sep) + 1));
	    QString incr_target_dir = var("DESTDIR") + "lib" + incr_target + "." + s_ext;

	    //incremental target
	    t << incr_target_dir << ": $(OBJECTS) $(OBJMOC) ";
	    if(!destdir.isEmpty())
		t << "\n\t" << "test -d " << destdir << " || mkdir -p " << destdir;
	    if(!project->isEmpty("QMAKE_PRE_LINK"))
		t << "\n\t" << var("QMAKE_PRE_LINK");
	    QString incr_lflags = var("QMAKE_LFLAGS_SHLIB") + " ";
	    incr_lflags += var(project->isActiveConfig("debug") ? "QMAKE_LFLAGS_DEBUG" : "QMAKE_LFLAGS_RELEASE");
	    t << "\n\t"
	      << "$(LINK) " << incr_lflags << " -o "<< incr_target_dir << " $(OBJECTS) $(OBJMOC)" << endl;
	    //real target
	    QString objs = "$(INCREMENTAL_OBJECTS)";
	    t << var("TARGET") << ": " << incr_target_dir << " " << objs << var("TARGETDEPS") << "\n\t";
	    if(!destdir.isEmpty()) {
		t << "test -d " << destdir << " || mkdir -p " << destdir << "\n\t";
		objs += " -L" + destdir;
	    }
	    objs += " -l" + incr_target;
	    t << "$(LINK) $(LFLAGS) -o $(TARGET) " << objs << " $(LIBS)" << endl << endl;
	} else {
	    t << "$(TARGET): $(UICDECLS) $(OBJECTS) $(OBJMOC) " << var("TARGETDEPS") << "\n\t";
	    if(!destdir.isEmpty())
		t << "test -d " << destdir << " || mkdir -p " << destdir << "\n\t";
	    t << "$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJMOC) $(LIBS)";
	    if(!project->isEmpty("QMAKE_POST_LINK"))
		t << "\n\t" << var("QMAKE_POST_LINK");
	    t << endl << endl;
	}
    } else if(!project->isActiveConfig("staticlib")) {
	t << "all: " << ofile << " " << varGlue("ALL_DEPS",""," ","") << " " <<  var("DESTDIR_TARGET") << endl << endl;

	QString destdir = project->first("DESTDIR");
	if(do_incremental) {
	    //utility variables
	    QString s_ext = project->variables()["QMAKE_EXTENSION_SHLIB"].first();
	    QString incr_target = var("QMAKE_ORIG_TARGET").replace("\\." + s_ext, "").replace(QRegExp("^lib"), "") + "_incremental";
	    incr_target = incr_target.right(incr_target.length() - (incr_target.findRev(Option::dir_sep) + 1));
	    QString incr_target_dir = var("DESTDIR") + "lib" + incr_target + "." + s_ext;

	    //incremental target
	    t << incr_target_dir << ": $(OBJECTS) $(OBJMOC) ";
	    if(!destdir.isEmpty())
		t << "\n\t" << "test -d " << destdir << " || mkdir -p " << destdir;
	    if(!project->isEmpty("QMAKE_PRE_LINK"))
		t << "\n\t" << var("QMAKE_PRE_LINK");
	    QString incr_lflags = var("QMAKE_LFLAGS_SHLIB") + " ";
	    incr_lflags += var(project->isActiveConfig("debug") ? "QMAKE_LFLAGS_DEBUG" : "QMAKE_LFLAGS_RELEASE");
	    t << "\n\t"
	      << "$(LINK) " << incr_lflags << " -o " << incr_target_dir << " $(OBJECTS) $(OBJMOC)" << endl;
	    //real target
	    QString cmd = var("QMAKE_LINK_SHLIB_CMD").replace(QRegExp("\\$\\(OBJECTS\\) \\$\\(OBJMOC\\)"), "$(INCREMENTAL_OBJECTS) ");
	    if(!destdir.isEmpty())
		cmd += " -L" + destdir;
	    cmd += " -l" + incr_target;
	    project->variables()["QMAKE_LINK_SHLIB_CMD"].clear();
	    project->variables()["QMAKE_LINK_SHLIB_CMD"].append(cmd);
	    t << var("DESTDIR_TARGET") << ": " << incr_target_dir << " $(INCREMENTAL_OBJECTS) $(SUBLIBS) " << var("TARGETDEPS");
	} else {
	    t << var("DESTDIR_TARGET") << ": $(OBJECTS) $(OBJMOC) $(SUBLIBS) " << var("TARGETDEPS");
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
	    t << "$(TARGETA): $(UICDECLS) $(OBJECTS) $(OBJMOC)" <<
		var("TARGETDEPS") << "\n\t"
	      << "-rm -f $(TARGETA) " << "\n\t"
	      << var("QMAKE_AR_CMD") << varGlue("QMAKE_RANLIB","\n\t"," "," $(TARGETA)")
	      << endl << endl;
	}
    } else {
	t << "all: " << ofile << " " << varGlue("ALL_DEPS",""," "," ") << "$(TARGET)" << endl << endl;
	t << "staticlib: $(TARGET)" << endl << endl;
	t << "$(TARGET): $(UICDECLS) $(OBJECTS) $(OBJMOC) " << var("TARGETDEPS") << "\n\t";

	if(!project->isEmpty("DESTDIR")) {
	    QString destdir = project->first("DESTDIR");
	    t << "test -d " << destdir << " || mkdir -p " << destdir << "\n\t";
	}
	t << "-rm -f $(TARGET)" << "\n\t"
	  << var("QMAKE_AR_CMD") << "\n\t";
	if(!project->isEmpty("QMAKE_POST_LINK"))
	    t << var("QMAKE_POST_LINK") << "\n\t";

	t << varGlue("QMAKE_RANLIB",""," "," $(TARGET)")
	  << endl << endl;
    }

    t << "mocables: $(SRCMOC)" << endl << endl;

    //this is an implicity depend on moc, so it will be built if necesary, however
    //moc itself shouldn't have this dependancy - this is a little kludgy but it is
    //better than the alternative for now.
    QString moc = project->first("QMAKE_MOC"), target = project->first("TARGET");
    fixEnvVariables(target);
    fixEnvVariables(moc);
    if(target != moc) {
	t << "$(MOC): \n\t"
	  << "( cd $(QTDIR)/src/moc ; $(MAKE) )"  << endl << endl;
    }

    writeMakeQmake(t);

    if(!project->first("QMAKE_PKGINFO").isEmpty()) {
	QString pkginfo = project->first("QMAKE_PKGINFO");
	QString destdir = project->first("DESTDIR");
	t << "#Looks strange? Well I don't even know what it means! ###SAM" << endl;
	t << pkginfo << ": " << "\n\t";
	if(!destdir.isEmpty())
	    t << "test -d " << destdir << " || mkdir -p " << destdir << "\n\t";
	t << "rm -f " << pkginfo << "\n\t"
	  << "echo \"APPL????\" >" << pkginfo << endl;
    }

    t << "dist: " << "\n\t"
      << "cd ..\n\t"
      << "$(TAR) " << var("PROJECT") << ".tar " << " $(SOURCES) $(HEADERS) $(FORMS) $(DIST)" << "\n\t"
      << "$(GZIP) " << var("PROJECT") << ".tar" << endl << endl;

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
    t << varGlue("QMAKE_CLEAN","-rm -f "," ","\n\t")
      << "-rm -f *~ core *.core" << "\n"
      << varGlue("CLEAN_FILES","\t-rm -f "," ","") << endl << endl;
    t << "####### Sub-libraries" << endl << endl;
    if ( !project->variables()["SUBLIBS"].isEmpty() ) {
	QStringList &l = project->variables()["SUBLIBS"];
	for(QStringList::Iterator it = l.begin(); it != l.end(); ++it)
	    t << "tmp/lib" << (*it) << ".a" << ":\n\t"
	      << var(QString("MAKELIB") + (*it)) << endl << endl;
    }

    QString destdir = project->first("DESTDIR");
    if(!destdir.isEmpty() && destdir.right(1) != Option::dir_sep)
	destdir += Option::dir_sep;
    t << "distclean: " << "clean\n\t"
      << "-rm -f " << destdir << "$(TARGET)" << " " << "$(TARGET)" << "\n\t";
    if(!project->isActiveConfig("staticlib") && project->variables()["QMAKE_APP_FLAG"].isEmpty() &&
       !project->isActiveConfig("plugin"))
	t << "-rm -f " << destdir << "$(TARGET0) " << destdir << "$(TARGET1) " << destdir << "$(TARGET2) $(TARGETA)" << "\n\t";
    t << endl << endl;

    if ( !project->isEmpty("PRECOMPH") ) {
	QString outdir = project->first("MOC_DIR");
	QString qt_dot_h = Option::fixPathToLocalOS(project->first("PRECOMPH"));
	t << "###### Combined headers" << endl << endl;
	//XXX
	t << outdir << "allmoc.cpp: " << qt_dot_h << " "
	  << varList("HEADERS_ORIG") << "\n\t"
	  << "echo '#include \"" << qt_dot_h << "\"' >" << outdir << "allmoc.cpp" << "\n\t"
	  << "$(CXX) -E -DQT_MOC_CPP -DQT_NO_STL $(CXXFLAGS) $(INCPATH) >" << outdir << "allmoc.h " << outdir << "allmoc.cpp" << "\n\t"
	  << "$(MOC) -o " << outdir << "allmoc.cpp " << outdir << "allmoc.h" << "\n\t"
	  << "perl -pi -e 's{#include \"allmoc.h\"}{#define QT_H_CPP\\n#include \"" << qt_dot_h << "\"}' " << outdir << "allmoc.cpp" << "\n\t"
	  << "rm " << outdir << "allmoc.h" << endl << endl;
    }
}

void
UnixMakefileGenerator::writeSubdirs(QTextStream &t)
{
    QString ofile = Option::output.name();
    if(ofile.findRev(Option::dir_sep) != -1)
	ofile = ofile.right(ofile.length() - ofile.findRev(Option::dir_sep) -1);

    t << "MAKEFILE =	" << var("MAKEFILE") << endl;
    t << "QMAKE =	" << var("QMAKE") << endl;
    t << "SUBDIRS =	" << varList("SUBDIRS") << endl;

    // subdirectory targets are sub-directory
    t << "SUBTARGETS =	";
    QStringList subdirs = project->variables()["SUBDIRS"];
    QStringList::Iterator it = subdirs.begin();
    while (it != subdirs.end())
	t << " \\\n\t\tsub-" << *it++;
    t << endl << endl;

    t << "all: " << ofile << " $(SUBTARGETS)" << endl << endl;

    // generate target rules
    it = subdirs.begin();
    while (it != subdirs.end()) {
	t << "sub-" << *it << ": qmake_all FORCE" << "\n\t"
	  << "cd " << *it << " && $(MAKE)" << endl << endl;
	it++;
    }

    if (project->isActiveConfig("ordered")) {
	// generate dependencies
	QString tar, dep;
	it = subdirs.begin();
	while (it != subdirs.end()) {
	    tar = *it++;
	    if (it != subdirs.end()) {
		dep = *it;
		t << "sub-" << dep << ": sub-" << tar << endl;
	    }
	}
	t << endl;
    }

    writeMakeQmake(t);

    t << "qmake_all:" << "\n\t"
      << "for i in $(SUBDIRS); do ( if [ -d $$i ]; then cd $$i ; pro=`basename $$i`.pro ; "
      << "[ ! -f $(MAKEFILE) ] && $(QMAKE) $$pro -o $(MAKEFILE); "
      << "grep \"^qmake_all:$$\" $$pro 2>/dev/null >/dev/null && "
      << "$(MAKE) -f $(MAKEFILE) qmake_all || true; fi; ) ; done" << endl << endl;

    t <<"distclean install uiclean mocclean clean:" << "\n\t"
      << "for i in $(SUBDIRS); do ( if [ -d $$i ]; then cd $$i ; $(MAKE) $@; fi; ) ; done" << endl << endl;

    t <<"FORCE:" << endl << endl;
}
