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

#include "option.h"
#include "msvc_dsp.h"
#include <time.h>
#include <qdir.h>
#include <stdlib.h>

DspMakefileGenerator::DspMakefileGenerator(QMakeProject *p) : MakefileGenerator(p), init_flag(FALSE)
{
    
}

bool
DspMakefileGenerator::writeMakefile(QTextStream &t)
{
    QString dspfile;
    if ( !project->variables()["DSP_TEMPLATE"].isEmpty() ) {
	dspfile = project->variables()["DSP_TEMPLATE"].first();
    } else {
	dspfile = project->variables()["MSVCDSP_TEMPLATE"].first();
    }
    dspfile = findTemplate(dspfile);

    QFile file(dspfile);
    if(!file.open(IO_ReadOnly)) {
	fprintf(stderr, "Cannot open dsp file: %s\n", dspfile.latin1());
	return FALSE;
    }
    QTextStream dsp(&file);
    
    int rep;
    QString line;
    while ( !dsp.eof() ) {
	line = dsp.readLine();
	while((rep = line.find(QRegExp("\\$\\$[a-zA-Z0-9_-]*"))) != -1) {
	    QString torep = line.mid(rep, line.find(QRegExp("[^\\$a-zA-Z0-9_-]"), rep) - rep);
	    QString variable = torep.right(torep.length()-2);

	    t << line.left(rep); //output the left side
	    line = line.right(line.length() - (rep + torep.length())); //now past the variable
	    if(variable == "MSVCDSP_SOURCES") {
		if(project->variables()["SOURCES"].isEmpty())
		    continue;

		QStringList &list = project->variables()["SOURCES"];
		for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
		    t << "# Begin Source File\n\nSOURCE=.\\" << (*it) << endl;

		    if ( project->isActiveConfig("moc") && 
			 (*it).right(strlen(Option::moc_ext)) == Option::moc_ext) {
			QString base = (*it);
			base.replace(QRegExp("\\..*$"), "").upper();
			base.replace(QRegExp("[^a-zA-Z]"), "_");

			QString build = "\n\n# Begin Custom Build - Moc'ing " + mocablesFromMOC[(*it)] +
		    "...\n" "InputPath=.\\" + (*it) + "\n\n" "\"" + (*it) + "\"" 
					" : $(SOURCE) \"$(INTDIR)\" \"$(OUTDIR)\"\n"
					"\t%QTDIR%\\bin\\moc.exe " + mocablesFromMOC[(*it)] + " -o " +
					(*it) + "\n\n" "#End Custom Build\n\n";

			t << "USERDEP_" << base << "\"" << mocablesFromMOC[(*it)] << "\"" << endl << endl;
			t << "!IF \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - Win32 Release\"" << build
			  << "!ELSEIF \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - Win32 Debug\"" 
			  << build << "!ENDIF " << endl << endl;
		    }
		    t << "# End Source File" << endl;
		}
	    }
	    else if(variable == "MSVCDSP_HEADERS") { 
		if(project->variables()["HEADERS"].isEmpty())
		    continue;

		QStringList &list = project->variables()["HEADERS"];
		for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
		    t << "# Begin Source File\n\nSOURCE=.\\" << (*it) << endl;

		    if ( project->isActiveConfig("moc") && !mocablesToMOC[(*it)].isEmpty()) {
			QString base = (*it);
			base.replace(QRegExp("\\..*$"), "").upper();
			base.replace(QRegExp("[^a-zA-Z]"), "_");

			QString build = "\n\n# Begin Custom Build - Moc'ing " + (*it) +
		    "...\n" "InputPath=.\\" + (*it) + "\n\n" "\"" + mocablesToMOC[(*it)] + 
					"\"" " : $(SOURCE) \"$(INTDIR)\" \"$(OUTDIR)\"\n"
					"\t%QTDIR%\\bin\\moc.exe " + (*it)  + " -o " +
					mocablesToMOC[(*it)] + "\n\n" "#End Custom Build\n\n";

			t << "!IF \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - Win32 Release\"" << build
			  << "!ELSEIF \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - Win32 Debug\"" 
			  << build << "!ENDIF " << endl << endl;
		    }
		    t << "# End Source File" << endl;
		}
	    }
	    else if(variable == "MSVCDSP_INTERFACESOURCES" || variable == "MSVCDSP_INTERFACEHEADERS") {
		if(project->variables()["INTERFACES"].isEmpty())
		    continue;

		QStringList &list = project->variables()["INTERFACES"];
		QString ext = variable == "MSVCDSP_INTERFACESOURCES" ? ".cpp" : ".h";
		for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
		    QString base = (*it);
		    base.replace(QRegExp("\\..*$"), ext);
		    t << "Begin Source File\n\nSOURCE=.\\" << base << "\n# End Source File" << endl;
		}
	    }
	    else if(variable == "MSVCDSP_INTERFACES") {
		if(project->variables()["INTERFACES"].isEmpty())
		    continue;

		QString uicpath = var("TMAKE_UIC");
		uicpath = uicpath.replace(QRegExp("\\..*$"), "") + " ";
		QStringList &list = project->variables()["INTERFACES"];
		for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
		    t <<  "# Begin Source File\n\nSOURCE=.\\" << (*it) << endl;

		    QString fname = (*it);
		    fname.replace(QRegExp("\\.ui"), "");


		    QString build = "\n\n# Begin Custom Build - Uic'ing " + (*it) + "...\n" 
			"InputPath=.\\" + (*it) + "\n\n" "BuildCmds= " + uicpath + (*it) + 
		     " -o " + fname + ".h\\\n" "\t" + uicpath  + (*it) +
		     " -i " + fname + ".h -o " + fname + ".cpp\\\n" 
		     "\t%QTDIR%\\bin\\moc " + fname + ".h -o moc_" + fname + ".cpp \\\n\n" 
		     "\"" + fname + ".h" ": \"$(SOURCE)\" \"$(INTDIR)\" \"$(OUTDIR)\""  "\n" 
		     "\t$(BuildCmds)\n\n" 
		     "\"" + fname + ".cpp" ": \"$(SOURCE)\" \"$(INTDIR)\" \"$(OUTDIR)\"" "\n" 
		     "\t$(BuildCmds)\n\n" 
		     "\"moc_" + fname + ".cpp" ": \"$(SOURCE)\" \"$(INTDIR)\" \"$(OUTDIR)\"" "\n"
		     "\t$(BuildCmds)\n\n" "# End Custom Build\n\n";

		    t << "!IF  \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - Win32 Release\"" << build 
		      << "!ELSEIF  \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - Win32 Debug\"" << build
		      << "!ENDIF \n\n"
		      << "# End Source File" << endl;
		}
	    }
	    else
		t << var(variable);
	}
    }
    file.close();

}



void
DspMakefileGenerator::init()
{
    if(init_flag)
	return;
    init_flag = TRUE;

    /* this should probably not be here, but I'm using it to wrap the .t files */
    if(project->variables()["TEMPLATE"].first() == "vcapp")
	project->variables()["TMAKE_APP_FLAG"].append("1");
    else if(project->variables()["TEMPLATE"].first() == "vclib")
	project->variables()["TMAKE_LIB_FLAG"].append("1");

    QStringList &configs = project->variables()["CONFIG"];
    if (project->isActiveConfig("qt_dll"))
	if(configs.findIndex("qt") == -1) configs.append("qt");
    if ( project->isActiveConfig("qt") ) {
	if ( (project->variables()["DEFINES"].findIndex("QT_NODLL") == -1) &&
	     ((project->variables()["DEFINES"].findIndex("QT_MAKEDLL") != -1 &&
	       project->variables()["DEFINES"].findIndex("QT_DLL") != -1) ||
	      project->isActiveConfig("qt_dll") ||(getenv("QT_DLL") && !getenv("QT_NODLL"))) ) {
	    project->variables()["TMAKE_QT_DLL"].append("1");
	    if ( (project->variables()["TARGET"].first() == "qt" ||
		  (project->variables()["TARGET"].first() == "qt-mt") && 
		  !project->variables()["TMAKE_LIB_FLAG"].isEmpty() ))
		project->variables()["CONFIG"].append("dll");
	}
    }
    if ( project->isActiveConfig("dll") || !project->variables()["TMAKE_APP_FLAG"].isEmpty() ) {
	project->variables()["CONFIG"].remove("staticlib");
	project->variables()["TMAKE_APP_OR_DLL"].append("1");
    } else {
	project->variables()["CONFIG"].append("staticlib");
    }

    if ( project->isActiveConfig("qt") || project->isActiveConfig("opengl") ) {
	project->variables()["CONFIG"].append("windows");
    }

    if ( project->isActiveConfig("qt") ) {
	project->variables()["CONFIG"].append("moc");
	project->variables()["DEFINES"].append("UNICODE");
	project->variables()["INCLUDEPATH"] +=	project->variables()["TMAKE_INCDIR_QT"];
	project->variables()["TMAKE_LIBS"] += QStringList::split(' ', "imm32.lib wsock32.lib winmm.lib");
	if ( project->isActiveConfig("opengl") ) {
	    project->variables()["TMAKE_LIBS"] += project->variables()["TMAKE_LIBS_QT_OPENGL"];
	}

	if ( (project->variables()["TARGET"].first() == "qt" || 
	      project->variables()["TARGET"].first() == "qt-mt") && 
	     !project->variables()["TMAKE_LIB_FLAG"].isEmpty() ) {
	    if ( !project->variables()["TMAKE_QT_DLL"].isEmpty() ) {
		project->variables()["DEFINES"].append("QT_MAKEDLL");
		project->variables()["MSVCDSP_DLLBASE"].append("/base:\"0x39D00000\"");
	    }
	} else {
	    project->variables()["TMAKE_LIBS"] += project->variables()["TMAKE_LIBS_QT"];
	    if ( !project->variables()["TMAKE_QT_DLL"].isEmpty() ) {
//		my $qtver =FindHighestLibVersion($ENV{"QTDIR"} . "/lib", "qt");
//		if ( project->isActiveConfig("thread") ) {
//		    project->variables()["TMAKE_LIBS /= s/qt.lib/qt-mt${qtver}.lib/"];
//		} else {
//		    project->variables()["TMAKE_LIBS /= s/qt.lib/qt${qtver}.lib/"];
//		}
		if ( !project->isActiveConfig("dll") ) {
		    project->variables()["TMAKE_LIBS"] +=project->variables()["TMAKE_LIBS_QT_DLL"];
		}
	    }
	}

    }
    if ( project->isActiveConfig("opengl") ) {
	project->variables()["TMAKE_LIBS"] += project->variables()["TMAKE_LIBS_OPENGL"];
    }
    if ( project->isActiveConfig("thread") ) {
	project->variables()["DEFINES"].append("QT_THREAD_SUPPORT" );
	if ( project->isActiveConfig("debug") ) {
	    if ( project->isActiveConfig("dll") ) {
		project->variables()["MSVCDSP_MTDEF"].append("-MDd");
	    } else {
		project->variables()["MSVCDSP_MTDEF"].append("-MTd");
	    }
	} else {
	    if ( project->isActiveConfig("dll") ) {
		project->variables()["MSVCDSP_MTDEF"].append("-MD");
	    } else {
		project->variables()["MSVCDSP_MTDEF"].append("-MT");
	    }
	}
    }
    if ( project->isActiveConfig("dll") ) {
	if ( !project->variables()["TMAKE_LIB_FLAG"].isEmpty() ) {
	    project->variables()["TARGET_EXT"].append(
		QStringList::split('.', project->variables()["VERSION"].first(), ".")[0] + ".dll");
	} else {
	    project->variables()["TARGET_EXT"].append(".dll");
	}
    } else {
	if ( !project->variables()["TMAKE_APP_FLAG"].isEmpty() ) {
	    project->variables()["TARGET_EXT"].append(".exe");
	} else {
	    project->variables()["TARGET_EXT"].append(".lib");
	}
    }
    project->variables()["TARGET"].first() += project->variables()["TARGET_EXT"].first();
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
    MakefileGenerator::init();
#if 0 //FIXME?
    tmake_use_win32_registry();
    $HKEY_CURRENT_USER->Open("Software\\Microsoft\\DevStudio\\5.0",$is_msvc5);
    if ( $is_msvc5 ) {
	project->variables()["MSVCDSP_VER"] = "5.00";
	project->variables()["MSVCDSP_DEBUG_OPT"] = "/Zi";
    } else {
	project->variables()["MSVCDSP_VER"] = "6.00";
	project->variables()["MSVCDSP_DEBUG_OPT"] = "/GZ /ZI";
    }
#endif
    project->variables()["MSVCDSP_PROJECT"] = project->variables()["OUTFILE"];
    QStringList &proj = project->variables()["MSVCDSP_PROJECT"];
    for(QStringList::Iterator it = proj.begin(); it != proj.end(); ++it)
	(*it).replace(QRegExp("\\.[a-zA-Z0-9_]*$"), "");

    if ( !project->variables()["TMAKE_APP_FLAG"].isEmpty() ) {
	project->variables()["MSVCDSP_TEMPLATE"].append("win32app.dsp");
	if ( project->isActiveConfig("console") ) {
	    project->variables()["MSVCDSP_CONSOLE"].append("Console");
	    project->variables()["MSVCDSP_WINCONDEF"].append("_CONSOLE");
	    project->variables()["MSVCDSP_DSPTYPE"].append("0x0103");
	    project->variables()["MSVCDSP_SUBSYSTEM"].append("console");
	} else {
	    project->variables()["MSVCDSP_CONSOLE"].clear();
	    project->variables()["MSVCDSP_WINCONDEF"].append("_WINDOWS");
	    project->variables()["MSVCDSP_DSPTYPE"].append("0x0101");
	    project->variables()["MSVCDSP_SUBSYSTEM"].append("windows");
	}
    } else {
	if ( project->isActiveConfig("dll") ) {
	    project->variables()["MSVCDSP_TEMPLATE"].append("win32dll.dsp");
	} else {
	    project->variables()["MSVCDSP_TEMPLATE"].append("win32lib.dsp");
	}
    }
    project->variables()["MSVCDSP_LIBS"] = project->variables()["TMAKE_LIBS"];
    project->variables()["MSVCDSP_DEFINES"].append(varGlue("DEFINES","/D ","" "/D ",""));
    project->variables()["MSVCDSP_INCPATH"].append(varGlue("INCPATH","/I "," /I ",""));
    if ( project->isActiveConfig("qt") ) {
	project->variables()["MSVCDSP_RELDEFS"].append("/D \"NO_DEBUG\"");
    } else {
	project->variables()["MSVCDSP_RELDEFS"].clear();
    }
    if ( !project->variables()["DESTDIR"].isEmpty() ) {
	project->variables()["TARGET"].first().prepend(project->variables()["DESTDIR"].first() +  "\\");
	Option::fixPathToTargetOS(project->variables()["TARGET"].first());
	project->variables()["MSVCDSP_TARGET"].append(
	    QString("/out:\"") + project->variables()["TARGET"].first() + "\"");
	if ( project->isActiveConfig("dll") ) {
	    project->variables()["MSVCDSP_TARGET"].append(
		QString(" /implib:\"") + project->variables()["TARGET"].first() + "\"");
	}
    }
    if ( project->isActiveConfig("dll") && !project->variables()["DLLDESTDIR"].isEmpty() ) {
	project->variables()["MSVCDSP_COPY_DLL"].append(
	    "# Begin Special Build Tool\n"
	    "TargetPath=" + project->variables()["TARGET"].first() + "\n"
	    "SOURCE=$(InputPath)\n"
	    "PostBuild_Desc=Copy DLL to " + project->variables()["DLLDESTDIR"].first() + "\n"
	    "PostBuild_Cmds=copy $(TargetPath) \"" + project->variables()["DLLDESTDIR"].first() + "\"\n"
	    "# End Special Build Tool");
    }
    if ( project->isActiveConfig("moc") ) {
	project->variables()["SOURCES"] += project->variables()["SRCMOC"];
    }
    if ( !project->variables()["SOURCES"].isEmpty() || !project->variables()["RC_FILE"].isEmpty() ) {
	project->variables()["SOURCES"] += project->variables()["RC_FILE"];
    }
}


QString
DspMakefileGenerator::findTemplate(QString file)
{
    QString ret;
    if(!QFile::exists(ret = file) && !QFile::exists((ret = (QString(getenv("HOME")) + "/.tmake/" + file))) &&
       !QFile::exists((ret = (QString(getenv("TMAKEPATH")) + file))))
	return "";
    return ret;
}
