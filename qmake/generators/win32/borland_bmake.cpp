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

#include <qdir.h>
#include "borland_bmake.h"
#include "option.h"
#include <time.h>
#include <stdlib.h>

BorlandMakefileGenerator::BorlandMakefileGenerator(QMakeProject *p) : Win32MakefileGenerator(p), init_flag(FALSE)
{
    
}

bool
BorlandMakefileGenerator::writeMakefile(QTextStream &t)
{
    writeHeader(t);
    if(!project->variables()["TMAKE_FAILED_REQUIREMENTS"].isEmpty()) {
	t << "all clean:" << "\n\t"
	  << "@echo \"Some of the required modules (" 
	  << var("TMAKE_FAILED_REQUIREMENTS") << ") are not available.\"" << "\n\t"
	  << "@echo \"Skipped.\"" << endl << endl;
	return TRUE;
    }

    if(project->variables()["TEMPLATE"].first() == "app" || 
       project->variables()["TEMPLATE"].first() == "lib") {
	writeBorlandParts(t);
	return MakefileGenerator::writeMakefile(t);
    }	
    else if(project->variables()["TEMPLATE"].first() == "subdirs") {
	writeSubDirs(t);
	return TRUE; 
    }
    return FALSE;
}

void
BorlandMakefileGenerator::writeBorlandParts(QTextStream &t)
{
    t << "####### Compiler, tools and options" << endl << endl;
    t << "CC	=	" << var("TMAKE_CC") << endl;
    t << "CXX	=	" << var("TMAKE_CXX") << endl;
    t << "LEX     = " << var("TMAKE_LEX") << endl;
    t << "YACC    = " << var("TMAKE_YACC") << endl;
    t << "CFLAGS	=	" << var("TMAKE_CFLAGS") << " " <<  varGlue("DEFINES","-D"," -D","") << endl;
    t << "CXXFLAGS=	" << var("TMAKE_CXXFLAGS") << " " << varGlue("DEFINES","-D"," -D","") << endl;
    t << "LEXFLAGS=" << var("TMAKE_LEXFLAGS") << endl;
    t << "YACCFLAGS=" << var("TMAKE_YACCFLAGS") << endl;
    t << "INCPATH	=	" << varGlue("INCLUDEPATH","-I\"","\" -I\"","\"") << endl;
    if(!project->variables()["TMAKE_APP_OR_DLL"].isEmpty()) {
	t << "LINK	=	" << var("TMAKE_LINK") << endl;
	t << "LFLAGS	=	" << var("TMAKE_LFLAGS") << endl;
	t << "LIBS	=	" << var("TMAKE_LIBS") << endl;
    }
    else {
	t << "LIB	=	" << var("TMAKE_LIB") << endl;
    }
    t << "MOC	=	" << var("TMAKE_MOC") << endl;
    t << "UIC	=	" << var("TMAKE_UIC") << endl;
    t << "ZIP	=	" << var("TMAKE_ZIP") << endl;
    t << endl;

    t << "####### Files" << endl << endl;
    t << "HEADERS =	" << varList("HEADERS") << endl;
    t << "SOURCES =	" << varList("SOURCES") << endl;
    t << "OBJECTS =	" << varList("OBJECTS") << endl;
    t << "INTERFACES =	" << varList("INTERFACES") << endl;
    t << "UICDECLS =	" << varList("UICDECLS") << endl;
    t << "UICIMPLS =	" << varList("UICIMPLS") << endl;
    t << "SRCMOC	=	" << varList("SRCMOC") << endl;
    t << "OBJMOC	=	" << varList("OBJMOC") << endl;
    t << "DIST	=	" << varList("DISTFILES") << endl;
    t << "TARGET	=	" 
      << varGlue("TARGET",project->variables()["DESTDIR"].first(),"",project->variables()["TARGET_EXT"].first())
      << endl;
    t << endl;

    t << "####### Implicit rules" << endl << endl;
    t << ".SUFFIXES: .cpp .cxx .cc .c" << endl << endl;
    t << ".cpp.obj:\n\t" << var("TMAKE_RUN_CXX_IMP") << endl << endl;
    t << ".cxx.obj:\n\t" << var("TMAKE_RUN_CXX_IMP") << endl << endl;
    t << ".cc.obj:\n\t" << var("TMAKE_RUN_CXX_IMP") << endl << endl;
    t << ".c.obj:\n\t" << var("TMAKE_RUN_CC_IMP") << endl << endl;

    t << "####### Build rules" << endl << endl;
    t << "all: " << varGlue("ALL_DEPS",""," "," ") << " $(TARGET)" << endl << endl;
    t << "$(TARGET): $(UICDECLS) $(OBJECTS) $(OBJMOC) " << var("TARGETDEPS");
    if(!project->variables()["TMAKE_APP_OR_DLL"].isEmpty()) {
	t << "\n\t" << "$(LINK) @&&| $(LFLAGS) $(OBJECTS) $(OBJMOC), $(TARGET),,$(LIBS)" << "\n\t"
	  << "-del $(TARGET)";
    } else {
	t << "\n\t" << "$(LIB) $(TARGET) @&&|" << " \n+"
	  << project->variables()["OBJECTS"].join(" \\\n+") << " \\\n+"
	  << project->variables()["OBJMOC"].join(" \\\n+");
    }
    t << endl << "|" << endl;
    if(project->isActiveConfig("dll") && !project->variables()["DLLDESTDIR"].isEmpty()) {
	t << "\n\t" << "-copy $(TARGET) " << var("DLLDESTDIR");
    }
    t << endl << endl;

    if(!project->variables()["RC_FILE"].isEmpty()) {
	t << var("RES_FILE") << ": " << var("RC_FILE") << "\n\t"
	  << var("TMAKE_RC") << " " << var("RC_FILE") << endl << endl;
    }
    t << "moc: $(SRCMOC)" << endl << endl;

    t << "qmake: " << "\n\t"
      << "qmake " << project->projectFile();
    if (Option::output.name())
	t << " -o " << Option::output.name();
    t << endl << endl;

    t << "dist:" << "\n\t"
      << "$(ZIP) " << var("PROJECT") << ".zip " << var("PROJECT") << ".pro $(SOURCES) $(HEADERS) $(DIST)"
      << endl << endl;

    t << "clean:\n\t"
      << varGlue("OBJECTS","-del ","\n\t-del ","") << "\n\t"
      << varGlue("SRCMOC" ,"-del ","\n\t-del ","") << "\n\t"
      << varGlue("OBJMOC" ,"-del ","\n\t-del ","") << "\n\t"
      << "-del $(TARGET)" << "\n\t"
      << varGlue("TMAKE_CLEAN","-del ","\n\t-del ","") << "\n\t"
      << varGlue("CLEAN_FILES","-del ","\n\t-del ","") << endl << endl;
}

void
BorlandMakefileGenerator::init()
{
    if(init_flag)
	return;
    init_flag = TRUE;

    /* this should probably not be here, but I'm using it to wrap the .t files */
    if(project->variables()["TEMPLATE"].first() == "app")
	project->variables()["TMAKE_APP_FLAG"].append("1");
    else if(project->variables()["TEMPLATE"].first() == "lib")
	project->variables()["TMAKE_LIB_FLAG"].append("1");
    else if(project->variables()["TEMPLATE"].first() == "subdirs") {
	MakefileGenerator::init();
	if(project->variables()["MAKEFILE"].isEmpty())
	    project->variables()["MAKEFILE"].append("Makefile");
	if(project->variables()["TMAKE"].isEmpty())
	    project->variables()["TMAKE"].append("qmake");
	return;
    }

    QStringList &configs = project->variables()["CONFIG"];
    if (project->isActiveConfig("qt_dll"))
	if(configs.findIndex("qt") == -1) configs.append("qt");
    if ( project->isActiveConfig("qt") ) {
	if ( (project->variables()["DEFINES"].findIndex("QT_NODLL") == -1) &&
	     ((project->variables()["DEFINES"].findIndex("QT_MAKEDLL") != -1 ||
	       project->variables()["DEFINES"].findIndex("QT_DLL") != -1) ||
	      (getenv("QT_DLL") && !getenv("QT_NODLL"))) ) {
	    project->variables()["TMAKE_QT_DLL"].append("1");
	    if ( (project->variables()["TARGET"].first() == "qt") && 
		 !project->variables()["TMAKE_LIB_FLAG"].isEmpty() )
		project->variables()["CONFIG"].append("dll");
	}
    }
    if ( project->isActiveConfig("dll") || !project->variables()["TMAKE_APP_FLAG"].isEmpty() ) {
	project->variables()["CONFIG"].remove("staticlib");
	project->variables()["TMAKE_APP_OR_DLL"].append("1");
    } else {
	project->variables()["CONFIG"].append("staticlib");
    }
    if ( project->isActiveConfig("warn_off") ) {
	project->variables()["TMAKE_CFLAGS"] += project->variables()["TMAKE_CFLAGS_WARN_OFF"];
	project->variables()["TMAKE_CXXFLAGS"] += project->variables()["TMAKE_CXXFLAGS_WARN_OFF"];
    } else if ( project->isActiveConfig("warn_on") ) {
	project->variables()["TMAKE_CFLAGS"] += project->variables()["TMAKE_CFLAGS_WARN_ON"];
	project->variables()["TMAKE_CXXFLAGS"] += project->variables()["TMAKE_CXXFLAGS_WARN_ON"];
    }
    if ( project->isActiveConfig("thread") ) {
        project->variables()["DEFINES"].append("QT_THREAD_SUPPORT");
    }
    if ( project->isActiveConfig("debug") ) {
        if ( project->isActiveConfig("thread") ) {
	    if ( project->isActiveConfig("dll") ) {
	        project->variables()["TMAKE_CFLAGS"] += project->variables()["TMAKE_CFLAGS_MT_DLLDBG"];
	        project->variables()["TMAKE_CXXFLAGS"] += project->variables()["TMAKE_CXXFLAGS_MT_DLLDBG"];
 	    } else {
		project->variables()["TMAKE_CFLAGS"] += project->variables()["TMAKE_CFLAGS_MT_DBG"];
		project->variables()["TMAKE_CXXFLAGS"] += project->variables()["TMAKE_CXXFLAGS_MT_DBG"];
	    }
        } else {
	    project->variables()["TMAKE_CFLAGS"] += project->variables()["TMAKE_CFLAGS_DEBUG"];
	    project->variables()["TMAKE_CXXFLAGS"] += project->variables()["TMAKE_CXXFLAGS_DEBUG"];
	}
	project->variables()["TMAKE_LFLAGS"] += project->variables()["TMAKE_LFLAGS_DEBUG"];
    } else if ( project->isActiveConfig("release") ) {
	if ( project->isActiveConfig("thread") ) {
	    if ( project->isActiveConfig("dll") ) {
		project->variables()["TMAKE_CFLAGS"] += project->variables()["TMAKE_CFLAGS_MT_DLL"];
		project->variables()["TMAKE_CXXFLAGS"] += project->variables()["TMAKE_CXXFLAGS_MT_DLL"];
	    } else {
		project->variables()["TMAKE_CFLAGS"] += project->variables()["TMAKE_CFLAGS_MT"];
		project->variables()["TMAKE_CXXFLAGS"] += project->variables()["TMAKE_CXXFLAGS_MT"];
	    }
	}
	project->variables()["TMAKE_CFLAGS"] += project->variables()["TMAKE_CFLAGS_RELEASE"];
	project->variables()["TMAKE_CXXFLAGS"] += project->variables()["TMAKE_CXXFLAGS_RELEASE"];
	project->variables()["TMAKE_LFLAGS"] += project->variables()["TMAKE_LFLAGS_RELEASE"];
    }

    if ( !project->variables()["TMAKE_INCDIR"].isEmpty()) {
	project->variables()["INCLUDEPATH"] += project->variables()["TMAKE_INCDIR"];
    }
    if ( project->isActiveConfig("qt") || project->isActiveConfig("opengl") ) {
	project->variables()["CONFIG"].append("windows");
    }
    if ( project->isActiveConfig("qt") ) {
	project->variables()["CONFIG"].append("moc");
	project->variables()["INCLUDEPATH"] +=	project->variables()["TMAKE_INCDIR_QT"];
	if ( !project->isActiveConfig("debug") ) {
	    project->variables()["DEFINES"].append("NO_DEBUG");
	}
	if ( (project->variables()["TARGET"].first() == "qt") && 
	     !project->variables()["TMAKE_LIB_FLAG"].isEmpty() ) {
	    if ( !project->variables()["TMAKE_QT_DLL"].isEmpty()) {
		project->variables()["DEFINES"].append("QT_MAKEDLL");
		project->variables()["TMAKE_LFLAGS"] += project->variables()["TMAKE_LFLAGS_QT_DLL"];
	    }
	} else {
	    project->variables()["TMAKE_LIBS"] += project->variables()["TMAKE_LIBS_QT"];
	    if ( !project->variables()["TMAKE_QT_DLL"].isEmpty() ) {
		int hver = findHighestVersion(QString(getenv("QTDIR")) + "/lib", "qt");
		if(hver != -1) {
		    QString ver;
		    ver.sprintf("qt%d.lib", hver);

    		    QStringList &libs = project->variables()["TMAKE_LIBS"];
		    for(QStringList::Iterator libit = libs.begin(); libit != libs.end(); ++libit)
			(*libit).replace(QRegExp("qt\\.lib"), ver);
		}
		if ( !project->isActiveConfig("dll") ) {
		    project->variables()["TMAKE_LIBS"] += project->variables()["TMAKE_LIBS_QT_DLL"];
		}
	    }
	}
    }
    if ( project->isActiveConfig("opengl") ) {
	project->variables()["TMAKE_LIBS"] += project->variables()["TMAKE_LIBS_OPENGL"];
    }
    if ( project->isActiveConfig("dll") ) {
	project->variables()["TMAKE_LFLAGS_CONSOLE_ANY"] = project->variables()["TMAKE_LFLAGS_CONSOLE_DLL"];
	project->variables()["TMAKE_LFLAGS_WINDOWS_ANY"] = project->variables()["TMAKE_LFLAGS_WINDOWS_DLL"];
	if ( !project->variables()["TMAKE_LIB_FLAG"].isEmpty()) {
	    project->variables()["TARGET_EXT"].append(
		QStringList::split('.',project->variables()["VERSION"].first()).join("") + ".dll");
	} else {
	    project->variables()["TARGET_EXT"].append(".dll");
	}
    } else {
	project->variables()["TMAKE_LFLAGS_CONSOLE_ANY"] = project->variables()["TMAKE_LFLAGS_CONSOLE"];
	project->variables()["TMAKE_LFLAGS_WINDOWS_ANY"] = project->variables()["TMAKE_LFLAGS_WINDOWS"];
	if ( !project->variables()["TMAKE_APP_FLAG"].isEmpty()) {
	    project->variables()["TARGET_EXT"].append(".exe");
	} else {
	    project->variables()["TARGET_EXT"].append(".lib");
	}
    }
    if ( project->isActiveConfig("windows") ) {
	if ( project->isActiveConfig("console") ) {
	    project->variables()["TMAKE_LFLAGS"] += project->variables()["TMAKE_LFLAGS_CONSOLE_ANY"];
	    project->variables()["TMAKE_LIBS  "] += project->variables()["TMAKE_LIBS_CONSOLE"];
	} else {
	    project->variables()["TMAKE_LFLAGS"] += project->variables()["TMAKE_LFLAGS_WINDOWS_ANY"];
	}
	project->variables()["TMAKE_LIBS  "] += project->variables()["TMAKE_LIBS_WINDOWS"];
    } else {
	project->variables()["TMAKE_LFLAGS"] += project->variables()["TMAKE_LFLAGS_CONSOLE_ANY"];
	project->variables()["TMAKE_LIBS  "] += project->variables()["TMAKE_LIBS_CONSOLE"];
    }
    if ( project->isActiveConfig("thread") ) {
        project->variables()["TMAKE_LIBS  "] += project->variables()["TMAKE_LIBS_RTMT"];
    } else {
        project->variables()["TMAKE_LIBS  "] += project->variables()["TMAKE_LIBS_RT"];
    }
    if ( project->isActiveConfig("moc") ) {
	setMocAware(TRUE);
    }
    project->variables()["TMAKE_LIBS"] += project->variables()["LIBS"];
    project->variables()["TMAKE_FILETAGS"] += QStringList::split(' ',
	"HEADERS SOURCES DEF_FILE RC_FILE TARGET TMAKE_LIBS DESTDIR DLLDESTDIR INCLUDEPATH");
    QStringList &l = project->variables()["TMAKE_FILETAGS"];
    for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
	QStringList &gdmf = project->variables()[(*it)];
	for(QStringList::Iterator inner = gdmf.begin(); inner != gdmf.end(); ++inner)
	    (*inner) = Option::fixPathToTargetOS((*inner));
    }

    if ( !project->variables()["DEF_FILE"].isEmpty() ) {
	project->variables()["TMAKE_LFLAGS"].append(QString("/DEF:") + project->variables()["DEF_FILE"].first());
    }
    if ( !project->variables()["RC_FILE"].isEmpty()) {
	if ( !project->variables()["RES_FILE"].isEmpty()) {
	    fprintf(stderr, "Both .rc and .res file specified.\n");
	    fprintf(stderr, "Please specify one of them, not both.");
	    exit(666);
	}
	project->variables()["RES_FILE"] = project->variables()["RC_FILE"];
	project->variables()["RES_FILE"].first().replace(QRegExp("\\.rc"),".res");
	project->variables()["TARGETDEPS"] += project->variables()["RES_FILE"];
    }
    if ( !project->variables()["RES_FILE"].isEmpty()) {
	project->variables()["TMAKE_LIBS"] += project->variables()["RES_FILE"];
    }
    MakefileGenerator::init();
    if ( !project->variables()["VERSION"].isEmpty()) {
	QStringList l = QStringList::split('.', project->variables()["VERSION"].first());
	project->variables()["VER_MAJ"].append(l[0]);
	project->variables()["VER_MIN"].append(l[1]);
    }
    project->variables()["TMAKE_CLEAN"].append(project->variables()["TARGET"].first() + ".tds");
}

