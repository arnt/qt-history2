/****************************************************************************
** $Id: //depot/qt/main/src/%s#3 $
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
#include <time.h>
#include <qregexp.h>
#include <qfile.h>

UnixMakefileGenerator::UnixMakefileGenerator(QMakeProject *p) : MakefileGenerator(p), init_flag(FALSE)
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

    if(project->variables()["TEMPLATE"].first() == "app" ||
       project->variables()["TEMPLATE"].first() == "lib") {
	writeMakeParts(t);
	return MakefileGenerator::writeMakefile(t);
    }
    else if(project->variables()["TEMPLATE"].first() == "subdirs") {
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

    t << "####### Compiler, tools and options" << endl << endl;
    t << "CC      = " << var("QMAKE_CC") << endl;
    t << "CXX     = " << var("QMAKE_CXX") << endl;
    t << "LEX     = " << var("QMAKE_LEX") << endl;
    t << "YACC    = " << var("QMAKE_YACC") << endl;
    t << "CFLAGS  = " << var("QMAKE_CFLAGS") << " " << varGlue("DEFINES","-D"," -D","") << endl;
    t << "CXXFLAGS= " << var("QMAKE_CXXFLAGS") << " " << varGlue("DEFINES","-D"," -D","") << endl;
    t << "LEXFLAGS=" << var("QMAKE_LEXFLAGS") << endl;
    t << "YACCFLAGS=" << var("QMAKE_YACCFLAGS") << endl;
    t << "INCPATH = " << varGlue("INCLUDEPATH","-I", " -I", "") << endl;
    if(!project->isActiveConfig("staticlib")) {
	t << "LINK  = " << var("QMAKE_LINK") << endl;
	t << "LFLAGS= " << var("QMAKE_LFLAGS") << endl;
	t << "LIBS  = " << "$(SUBLIBS) " << var("QMAKE_LIBDIR_FLAGS") << " " << var("QMAKE_LIBS") << endl;
    }
    if(!project->variables()["QMAKE_LIB_FLAG"].isEmpty()) {
	t << "AR      = " << var("QMAKE_AR") << endl;
	t << "RANLIB  = " << var("QMAKE_RANLIB") << endl;
    }
    t << "MOC     = " << var("QMAKE_MOC") << endl;
    t << "UIC     = "	<< var("QMAKE_UIC") << endl;
    t << "TAR     = "	<< var("QMAKE_TAR") << endl;
    t << "GZIP    = " << var("QMAKE_GZIP") << endl;
    t << "COPY  =       " << var("QMAKE_COPY") << endl;
    t << "DEL   =       " << var("QMAKE_DEL") << endl;
    t << "MOVE  =       " << var("QMAKE_MOVE") << endl;
    t << endl;

    /* files */
    t << "####### Files" << endl << endl;
    t << "HEADERS = " << varList("HEADERS") << endl;
    t << "SOURCES = " << varList("SOURCES") << endl;
    t << "OBJECTS = " << varList("OBJECTS") << endl;
    t << "INTERFACES = " << varList("INTERFACES") << endl;
    t << "UICDECLS = " << varList("UICDECLS") << endl;
    t << "UICIMPLS = " << varList("UICIMPLS") << endl;
    t << "SRCMOC   = " << varList("SRCMOC") << endl;
    t << "OBJMOC   = " << varList("OBJMOC") << endl;
    t << "DIST	   = " << varList("DISTFILES") << endl;
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
    t << "####### Implicit rules" << endl << endl;
    t << ".SUFFIXES: .cpp .cxx .cc .C .c" << endl << endl;
    t << ".cpp.o:\n\t" << var("QMAKE_RUN_CXX_IMP") << endl << endl;
    t << ".cxx.o:\n\t" << var("QMAKE_RUN_CXX_IMP") << endl << endl;
    t << ".cc.o:\n\t" << var("QMAKE_RUN_CXX_IMP") << endl << endl;
    t << ".C.o:\n\t" << var("QMAKE_RUN_CXX_IMP") << endl << endl;
    t << ".c.o:\n\t" << var("QMAKE_RUN_CC_IMP") << endl << endl;
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
	t << "$(TARGET): $(UICDECLS) $(OBJECTS) $(OBJMOC) " << var("TARGETDEPS") << "\n\t"
	  << "[ -d " << project->variables()["DESTDIR"].first()
	  << " ] || mkdir -p " << project->variables()["DESTDIR"].first() << "\n\t"
	  << "$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJMOC) $(LIBS)" << endl << endl;
    } else if(!project->isActiveConfig("staticlib")) {
	t << "all: " << ofile << " " << varGlue("ALL_DEPS",""," ","") << " " <<  var("DESTDIR_TARGET") << endl << endl;
	t << var("DESTDIR_TARGET") << ": $(OBJECTS) $(OBJMOC) $(SUBLIBS) " << var("TARGETDEPS") << "\n\t"
	  << "[ -d " << var("DESTDIR") << " ] || mkdir -p " << var("DESTDIR");

	if(project->isActiveConfig("plugin")) {
	    t << "\n\t"
	      << "-rm -f $(TARGETD)" << "\n\t"
	      << var("QMAKE_LINK_SHLIB_CMD") << "\n\t"
	      << "-mv $(TARGETD) " << var("DESTDIR") << endl << endl;
	} else if(project->variables()["QMAKE_HPUX_SHLIB"].isEmpty()) {
	    t << "\n\t"
	      << "-rm -f $(TARGET) $(TARGET0) $(TARGET1) $(TARGET2)" << "\n\t"
	      << var("QMAKE_LINK_SHLIB_CMD") << "\n\t"
	      << varGlue("QMAKE_LN_SHLIB",""," "," $(TARGET) $(TARGET0)")  << "\n\t"
	      << varGlue("QMAKE_LN_SHLIB",""," "," $(TARGET) $(TARGET1)") << "\n\t"
	      << varGlue("QMAKE_LN_SHLIB",""," "," $(TARGET) $(TARGET2)") << "\n\t"
	      << "-rm -f " << var("DESTDIR") << "$(TARGET)\n\t"
	      << "-rm -f " << var("DESTDIR") << "$(TARGET0)\n\t"
	      << "-rm -f " << var("DESTDIR") << "$(TARGET1)\n\t"
	      << "-rm -f " << var("DESTDIR") << "$(TARGET2)\n\t"
	      << "-mv $(TARGET) $(TARGET0) $(TARGET1) $(TARGET2) " << var("DESTDIR") << endl << endl;
	} else {
	    t << "\n\t"
	      << "-rm -f $(TARGET) $(TARGET0)" << "\n\t"
	      << var("QMAKE_LINK_SHLIB_CMD") << "\n\t"
	      << varGlue("QMAKE_LN_SHLIB",""," "," $(TARGET) $(TARGET0)") << "\n\t"
	      << "-rm -f " << var("DESTDIR") << "$(TARGET)\n\t"
	      << "-rm -f " << var("DESTDIR") << "$(TARGET0)\n\t"
	      << "-mv $(TARGET) $(TARGET0) " << var("DESTDIR") << endl << endl;
	}
	t << endl << endl;

	t << "staticlib: $(TARGETA)" << endl << endl;
	t << "$(TARGETA): $(UICDECLS) $(OBJECTS) $(OBJMOC)" << var("TARGETDEPS") << "\n\t"
	  << "-rm -f $(TARGETA) " << var("QMAKE_AR_CMD")
	  << varGlue("QMAKE_RANLIB","",""," $(TARGETA)") << endl << endl;
    } else {
	t << "all: " << ofile << " " << varGlue("ALL_DEPS",""," "," ") << "$(TARGET)" << endl << endl;
	t << "staticlib: $(TARGET)" << endl << endl;
	t << "$(TARGET): $(UICDECLS) $(OBJECTS) $(OBJMOC) " << var("TARGETDEPS") << "\n\t"
	  << "[ -d " << var("DESTDIR") << " ] || mkdir -p " << var("DESTDIR") << "\n\t"
	  << "-rm -f $(TARGET)" << "\n\t"
	  << var("QMAKE_AR_CMD") << "\n\t"
	  << varGlue("QMAKE_RANLIB","",""," $(TARGET)") << endl << endl;
    }

    t << "mocables: $(SRCMOC)" << endl << endl;

    //this is an implicity depend on moc, so it will be built if necesary, however
    //moc itself shouldn't have this dependancy - this is a little kludgy but it is
    //better than the alternative for now.
    QString moc = project->variables()["QMAKE_MOC"].first(), target = project->variables()["TARGET"].first();
    fixEnvVariables(target);
    fixEnvVariables(moc);
    if(target != moc) {
	t << "$(MOC): \n\t"
	  << "make -C $(QTDIR)/src/moc" << endl << endl;
    }

    writeMakeQmake(t);

    t << "dist: " << "\n\t"
      << "cd ..\n\t"
      << "$(TAR) " << var("PROJECT") << ".tar " << " $(SOURCES) $(HEADERS) $(INTERFACES) $(DIST)" << "\n\t"
      << "$(GZIP) " << var("PROJECT") << ".tar" << endl << endl;

#if 0
    t << "install: " << "\n\t"
      << "$(TAR) " << var("PROJECT") << ".tar " << " $(SOURCES) $(HEADERS) $(INTERFACES) $(DIST)" << "\n\t"
      << "$(GZIP) " << var("PROJECT") << ".tar" << endl << endl;
#endif

    t << "mocclean:" << "\n\t"
      << "-rm -f $(OBJMOC) $(SRCMOC)" << endl << endl;

    t << "clean:" << "\n\t"
      << "-rm -f $(OBJECTS) $(OBJMOC) $(SRCMOC) $(UICIMPLS) $(UICDECLS) $(TARGET)" << "\n\t";
    if(!project->isActiveConfig("staticlib") && project->variables()["QMAKE_APP_FLAG"].isEmpty()) {
	t << "-rm -f $(TARGET0) $(TARGET1) $(TARGET2) $(TARGETA)" << "\n\t";
    }
    t << varGlue("QMAKE_CLEAN","-rm -f "," ","") << "\n\t"
      << "-rm -f *~ core *.core" << "\n\t"
      << varGlue("CLEAN_FILES","-rm -f "," ","") << endl << endl;
    t << "####### Sub-libraries" << endl << endl;
    if ( !project->variables()["SUBLIBS"].isEmpty() ) {
	QStringList &l = project->variables()["SUBLIBS"];
	for(QStringList::Iterator it = l.begin(); it != l.end(); ++it)
	    t << "tmp/lib" << (*it) << ".a" << ":\n\t"
	      << var(QString("MAKELIB") + (*it)) << endl << endl;
    }

    if ( !project->variables()["PRECOMPH"].isEmpty() ) {
	QString outdir = project->variables()["MOC_DIR"].first();
	QString qt_dot_h = Option::fixPathToLocalOS(project->variables()["PRECOMPH"].first());
	t << "###### Combined headers" << endl << endl;
	//XXX
	t << outdir << "allmoc.cpp: " << qt_dot_h << " "
	  << varList("HEADERS_ORIG") << "\n\t"
	  << "echo '#include \"" << qt_dot_h << "\"' >" << outdir << "allmoc.cpp" << "\n\t"
	  << "$(CXX) -E -DQT_MOC_CPP $(CXXFLAGS) $(INCPATH) >" << outdir << "allmoc.h " << outdir << "allmoc.cpp" << "\n\t"
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
    t << "install: " << ofile << " qmake_all" << "\n\t"
      << "for i in $(SUBDIRS); do ( if [ -d $$i ]; then cd $$i ; "
      << "[ -f $(MAKEFILE) ] && $(MAKE) -f $(MAKEFILE) install; fi; ) ; done" << endl;

    // generate target rules
    it = subdirs.begin();
    while (it != subdirs.end()) {
	t << "sub-" << *it << ": qmake_all FORCE" << "\n\t"
	  << "cd " << *it << " && $(MAKE)" << endl << endl;
	it++;
    }

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

    writeMakeQmake(t);

    t << "qmake_all:" << "\n\t"
      << "for i in $(SUBDIRS); do ( if [ -d $$i ]; then cd $$i ; pro=`basename $$i`.pro ; "
      << "[ ! -f $(MAKEFILE) ] && $(QMAKE) $$pro -o $(MAKEFILE); "
      << "grep \"^qmake_all:$$\" $$pro 2>/dev/null >/dev/null && "
      << "$(MAKE) -f $(MAKEFILE) qmake_all || true; fi; ) ; done" << endl << endl;

    t <<"clean release debug:" << "\n\t"
      << "for i in $(SUBDIRS); do ( if [ -d $$i ]; then cd $$i ; $(MAKE) $@; fi; ) ; done" << endl << endl;

    t <<"FORCE:" << endl << endl;
}


void
UnixMakefileGenerator::init()
{
    if(init_flag)
	return;
    init_flag = TRUE;

    if(!project->variables()["QMAKE_FAILED_REQUIREMENTS"].isEmpty()) /* no point */
	return;

     QStringList &configs = project->variables()["CONFIG"];

    /* this should probably not be here, but I'm using it to wrap the .t files */
    if(project->variables()["TEMPLATE"].first() == "app")
	project->variables()["QMAKE_APP_FLAG"].append("1");
    else if(project->variables()["TEMPLATE"].first() == "lib")
	project->variables()["QMAKE_LIB_FLAG"].append("1");
    else if(project->variables()["TEMPLATE"].first() == "subdirs") {
	MakefileGenerator::init();
	if(project->variables()["MAKEFILE"].isEmpty())
	    project->variables()["MAKEFILE"].append("Makefile");
	if(project->variables()["QMAKE"].isEmpty())
	    project->variables()["QMAKE"].append("qmake");

	return; /* subdirs is done */
    }

    if( project->variables()["QMAKE_EXTENTION_SHLIB"].first().isEmpty() )
	project->variables()["QMAKE_EXTENTION_SHLIB"].append( "so" );

    bool extern_libs = !project->isActiveConfig("dll")  || (project->variables()["TARGET"].first() == "qt" ||
							    project->variables()["TARGET"].first() == "qte" ||
							    project->variables()["TARGET"].first() == "qt-mt");
    project->variables()["QMAKE_LIBS"] += project->variables()["LIBS"];
    if ( (!project->variables()["QMAKE_LIB_FLAG"].isEmpty() && !project->isActiveConfig("staticlib") ) ||
	 (project->isActiveConfig("qt") &&  project->isActiveConfig( "plugin" ) )) {
	if(configs.findIndex("dll") == -1) configs.append("dll");
    } else if ( !project->variables()["QMAKE_APP_FLAG"].isEmpty() || project->isActiveConfig("dll") ) {
	configs.remove("staticlib");
    }
    if ( project->isActiveConfig("warn_off") ) {
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_WARN_OFF"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_WARN_OFF"];
    } else if ( project->isActiveConfig("warn_on") ) {
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_WARN_ON"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_WARN_ON"];
    }
    if ( project->isActiveConfig("debug") ) {
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_DEBUG"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_DEBUG"];
	project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_DEBUG"];
    } else if ( project->isActiveConfig("release") ) {
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_RELEASE"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_RELEASE"];
	project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_RELEASE"];
    }
    if ( !project->variables()["QMAKE_INCDIR"].isEmpty() ) {
	project->variables()["INCLUDEPATH"] += project->variables()["QMAKE_INCDIR"];
    }
    if ( !project->variables()["QMAKE_LIBDIR"].isEmpty() ) {
	project->variables()["QMAKE_LIBDIR_FLAGS"].append("-L" + project->variables()["QMAKE_LIBDIR"].first());
    }
    if ( extern_libs && (project->isActiveConfig("qt") || project->isActiveConfig("opengl")) ) {
	if(configs.findIndex("x11lib") == -1) configs.append("x11lib");
	if ( project->isActiveConfig("opengl") ) {
	    if(configs.findIndex("x11inc") == -1) configs.append("x11inc");
	}
    }
    if ( project->isActiveConfig("x11") ) {
	if(configs.findIndex("x11lib") == -1) configs.append("x11lib");
	if(configs.findIndex("x11inc") == -1) configs.append("x11inc");
    }
    if ( project->isActiveConfig("thread") ) {
	project->variables()["DEFINES"].append("QT_THREAD_SUPPORT");
	if ( !project->variables()["QMAKE_CFLAGS_THREAD"].isEmpty())
	    project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_THREAD"];
        if( !project->variables()["QMAKE_CXXFLAGS_THREAD"].isEmpty())
	    project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_THREAD"];
	project->variables()["INCLUDEPATH"] += project->variables()["QMAKE_INCDIR_THREAD"];
	project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_THREAD"];
	if(!project->variables()["QMAKE_LFLAGS_THREAD"].isEmpty())
	    project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_THREAD"];
    }
    if ( project->isActiveConfig("qt") ) {
	if(configs.findIndex("moc")) configs.append("moc");
	project->variables()["INCLUDEPATH"] += project->variables()["QMAKE_INCDIR_QT"];
	if ( !project->isActiveConfig("debug") ) {
	    project->variables()["DEFINES"].append("QT_NO_DEBUG");
	}
	if ( !( (project->variables()["TARGET"].first() == "qt") ||
		(project->variables()["TARGET"].first() == "qte") ||
		(project->variables()["TARGET"].first() == "qt-mt") ) ) {
	    if(!project->variables()["QMAKE_LIBDIR_QT"].isEmpty())
		project->variables()["QMAKE_LIBDIR_FLAGS"].append("-L" +
							       project->variables()["QMAKE_LIBDIR_QT"].first());
	    if (project->isActiveConfig("thread") && !project->variables()["QMAKE_LIBS_QT_THREAD"].isEmpty()) {
	        project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_QT_THREAD"];
	    } else {
	        project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_QT"];
	    }
	}
    }
    if ( project->isActiveConfig("opengl") ) {
	project->variables()["INCLUDEPATH"] += project->variables()["QMAKE_INCDIR_OPENGL"];
	if(!project->variables()["QMAKE_LIBDIR_OPENGL"].isEmpty())
	    project->variables()["QMAKE_LIBDIR_FLAGS"].append("-L" +
							  project->variables()["QMAKE_LIBDIR_OPENGL"].first());
	if ( (project->variables()["TARGET"].first() == "qt") ||
	     (project->variables()["TARGET"].first() == "qte") ||
	     (project->variables()["TARGET"].first() == "qt-mt") ) {
	    	project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_OPENGL_QT"];
	} else {
	    project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_OPENGL"];
	}
    }
    if ( project->isActiveConfig("x11sm") ) {
	project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_X11SM"];
    }
    if ( project->isActiveConfig("x11inc") ) {
	project->variables()["INCLUDEPATH"] += project->variables()["QMAKE_INCDIR_X11"];
    }
    if ( project->isActiveConfig("x11lib") ) {
	if(!project->variables()["QMAKE_LIBDIR_X11"].isEmpty())
	    project->variables()["QMAKE_LIBDIR_FLAGS"].append("-L" +
							      project->variables()["QMAKE_LIBDIR_X11"].first());
	project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_X11"];
    }
    if ( project->isActiveConfig("moc") ) {
	setMocAware(TRUE);
    }
    if ( project->variables()["QMAKE_RUN_CC"].isEmpty() ) {
	project->variables()["QMAKE_RUN_CC"].append("$(CC) -c $(CFLAGS) $(INCPATH) -o $obj $src");
    }
    if ( project->variables()["QMAKE_RUN_CC_IMP"].isEmpty() ) {
	project->variables()["QMAKE_RUN_CC_IMP"].append("$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $<");
    }
    if ( project->variables()["QMAKE_RUN_CXX"].isEmpty() ) {
	project->variables()["QMAKE_RUN_CXX"].append("$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $obj $src");
    }
    if ( project->variables()["QMAKE_RUN_CXX_IMP"].isEmpty() ) {
	project->variables()["QMAKE_RUN_CXX_IMP"].append("$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<");
    }
    project->variables()["QMAKE_FILETAGS"] += QStringList::split("HEADERS SOURCES TARGET DESTDIR", " ");
    if ( !project->variables()["PRECOMPH"].isEmpty() ) {
	project->variables()["SOURCES"].append(project->variables()["MOC_DIR"].first() + "allmoc.cpp");
	project->variables()["HEADERS_ORIG"] = project->variables()["HEADERS"];
	project->variables()["HEADERS"].clear();
    }
    MakefileGenerator::init();
    if ( project->isActiveConfig("resource_fork") ) {
	if(!project->variables()["QMAKE_APP_FLAG"].isEmpty())
	    project->variables()["DESTDIR"].first() += project->variables()["TARGET"].first() + ".app/Contents/MacOS/";
    }
    if(project->variables()["VERSION"].isEmpty()) {
	project->variables()["VERSION"].append("1.0.0");
	project->variables()["VER_MAJ"].append("1");
	project->variables()["VER_MIN"].append("0");
	project->variables()["VER_PAT"].append("0");
    }
    if ( project->variables()["VER_PAT"].isEmpty() ) {
	QStringList l = QStringList::split('.', project->variables()["VERSION"].first());
	project->variables()["VER_MAJ"].append(l[0]);
	project->variables()["VER_MIN"].append(l[1]);
	project->variables()["VER_PAT"].append("0");
    }
    if ( project->variables()["VER_MIN"].isEmpty() ) {
	project->variables()["VER_MAJ"] = project->variables()["VERSION"];
	project->variables()["VER_MIN"].append("0");
	project->variables()["VER_PAT"].append("0");
    }
    //project->variables()["DESTDIR_TARGET"].append("$(TARGET)");
    if ( !project->variables()["QMAKE_APP_FLAG"].isEmpty() ) {
#if 0
	if ( project->isActiveConfig("dll") ) {
	    project->variables()["TARGET"] += project->variables()["TARGET.so"];
	    if(project->variables()["QMAKE_LFLAGS_SHAPP"].isEmpty())
		project->variables()["QMAKE_LFLAGS_SHAPP"] += project->variables()["QMAKE_LFLAGS_SHLIB"];
	    if(!project->variables()["QMAKE_LFLAGS_SONAME"].isEmpty())
		project->variables()["QMAKE_LFLAGS_SONAME"].first() += project->variables()["TARGET"].first();
	}
#endif
	project->variables()["TARGET"].first().prepend(project->variables()["DESTDIR"].first());
    } else if ( project->isActiveConfig("staticlib") ) {
	project->variables()["TARGET"].first().prepend(project->variables()["DESTDIR"].first() + "lib");
	project->variables()["TARGET"].first() += ".a";
	if(project->variables()["QMAKE_AR_CMD"].isEmpty())
	    project->variables()["QMAKE_AR_CMD"].append("$(AR) $(TARGET) $(OBJECTS) $(OBJMOC)");
    } else {
	project->variables()["TARGETA"].append(project->variables()["DESTDIR"].first() + "lib" +
					       project->variables()["TARGET"].first() + ".a");
	if ( !project->variables()["QMAKE_AR_CMD"].isEmpty() ) {
	    project->variables()["QMAKE_AR_CMD"].first().replace(QRegExp("\\(TARGET\\)"),"(TARGETA)");
	} else {
	    project->variables()["QMAKE_AR_CMD"].append("$(AR) $(TARGETA) $(OBJECTS) $(OBJMOC)");
	}
        if( project->isActiveConfig("plugin") ) {
	    project->variables()["TARGET_x.y.z"].append("lib" +
						   project->variables()["TARGET"].first() + "." +
							project->variables()[
							    "QMAKE_EXTENTION_SHLIB"].first());
	    project->variables()["TARGET"] = project->variables()["TARGET_x.y.z"];
	} else if ( !project->variables()["QMAKE_HPUX_SHLIB"].isEmpty() ) {
	    project->variables()["TARGET_"].append("lib" + project->variables()["TARGET"].first() + ".sl");
	    project->variables()["TARGET_x"].append("lib" + project->variables()["TARGET"].first() + "." +
						    project->variables()["VER_MAJ"].first());
	    project->variables()["TARGET"] = project->variables()["TARGET_x"];
	} else if ( !project->variables()["QMAKE_AIX_SHLIB"].isEmpty() ) {
	    project->variables()["TARGET_"].append("lib" + project->variables()["TARGET"].first() + ".a");
	    project->variables()["TARGET_x"].append("lib" + project->variables()["TARGET"].first() + "." +
						    project->variables()["QMAKE_EXTENTION_SHLIB"].first() +
						    "." + project->variables()["VER_MAJ"].first());
	    project->variables()["TARGET_x.y"].append("lib" + project->variables()["TARGET"].first() + "." +
						      project->variables()["QMAKE_EXTENTION_SHLIB"].first()
						      + "." + project->variables()["VER_MAJ"].first() +
						      "." + project->variables()["VER_MIN"].first());
	    project->variables()["TARGET_x.y.z"].append("lib" + project->variables()["TARGET"].first() +
							"." +
							project->variables()[
							    "QMAKE_EXTENTION_SHLIB"].first() + "." +
							project->variables()["VER_MAJ"].first() + "." +
							project->variables()["VER_MIN"].first() + "." +
							project->variables()["VER_PAT"].first());
	    project->variables()["TARGET"] = project->variables()["TARGET_x.y.z"];
	} else {
	    project->variables()["TARGET_"].append("lib" + project->variables()["TARGET"].first() + "." +
						   project->variables()["QMAKE_EXTENTION_SHLIB"].first());
	    project->variables()["TARGET_x"].append("lib" + project->variables()["TARGET"].first() + "." +
						    project->variables()["QMAKE_EXTENTION_SHLIB"].first() +
						    "." + project->variables()["VER_MAJ"].first());
	    project->variables()["TARGET_x.y"].append("lib" + project->variables()["TARGET"].first() + "." +
						      project->variables()["QMAKE_EXTENTION_SHLIB"].first()
						      + "." + project->variables()["VER_MAJ"].first() +
						      "." + project->variables()["VER_MIN"].first());
	    project->variables()["TARGET_x.y.z"].append("lib" + project->variables()["TARGET"].first() +
							"." +
							project->variables()[
							    "QMAKE_EXTENTION_SHLIB"].first() + "." +
							project->variables()["VER_MAJ"].first() + "." +
							project->variables()["VER_MIN"].first() +  "." +
							project->variables()["VER_PAT"].first());
	    project->variables()["TARGET"] = project->variables()["TARGET_x.y.z"];
        }
	project->variables()["QMAKE_LN_SHLIB"].append("-ln -s");
	if ( !project->variables()["DESTDIR"].isEmpty() ) {
	    project->variables()["DESTDIR_TARGET"].append(project->variables()["DESTDIR"].first() +
							  project->variables()["TARGET"].first());
	}
	if(!project->variables()["QMAKE_LFLAGS_SONAME"].isEmpty())
	    project->variables()["QMAKE_LFLAGS_SONAME"].first() += project->variables()["TARGET_x"].first();
	if(project->variables()["QMAKE_LINK_SHLIB_CMD"].isEmpty())
	    project->variables()["QMAKE_LINK_SHLIB_CMD"].append(
		"$(LINK) $(LFLAGS) -o $(TARGETD) $(OBJECTS) $(OBJMOC) $(LIBS)");
    }
    if ( project->isActiveConfig("dll") ) {
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_SHLIB"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_SHLIB"];
	if ( !project->variables()["QMAKE_APP_FLAG"].isEmpty() ) {
	    project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_SHAPP"];
	} else if( project->isActiveConfig("plugin") ) {
	    project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_PLUGIN"];
	    project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_SONAME"];
	} else {
	    project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_SHLIB"];
	    project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_SONAME"];
	}
    }
}




QString
UnixMakefileGenerator::defaultInstall(const QString &t)
{
    QStringList ret;
    if(t == "target") {
	if(project->variables()["TEMPLATE"].first() == "app") {
	    ret << "$(TARGET)";
	} else if(!project->isActiveConfig("staticlib")) {
	    if(project->isActiveConfig("plugin"))
		ret << "$(TARGETD)";
	    else if ( !project->variables()["QMAKE_HPUX_SHLIB"].isEmpty() )
		ret << "$(TARGET)" << "$(TARGET0)";
	    else
		ret << "$(TARGET)" << "$(TARGET0)" << "$(TARGET1)" << "$(TARGET2)";
	} else {
	    ret = "$(TARGETA)";
	}
    }
    if(!ret.isEmpty()) {
	QString targs, destdir=project->variables()["DESTDIR"].first();
	for(QStringList::Iterator it = ret.begin(); it != ret.end(); it++) {
	    QString tmp;
	    if(!destdir.isEmpty())
		tmp = destdir + Option::dir_sep;
	    tmp += (*it);
	    targs += Option::fixPathToTargetOS(tmp, FALSE) + " ";
	}
	if(Option::mode == Option::WIN_MODE) {
	} else if(Option::mode == Option::UNIX_MODE) {
	    return QString("$(COPY) ") + targs + project->variables()["target.path"].first();
	}
    }
    return "";
}
