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
#include <qregexp.h>
#include <stdlib.h>

DspMakefileGenerator::DspMakefileGenerator(QMakeProject *p) : Win32MakefileGenerator(p), init_flag(FALSE)
{

}

bool
DspMakefileGenerator::writeMakefile(QTextStream &t)
{
    if(!project->variables()["QMAKE_FAILED_REQUIREMENTS"].isEmpty()) {
	/* for now just dump, I need to generated an empty dsp or something.. */
	fprintf(stderr, "Project file not generated because all requirements not met:\n\t%s\n",
		var("QMAKE_FAILED_REQUIREMENTS").latin1());
	return TRUE;
    }

    if(project->variables()["TEMPLATE"].first() == "vcapp" ||
       project->variables()["TEMPLATE"].first() == "vclib") {
	return writeDspParts(t);
    }
    else if(project->variables()["TEMPLATE"].first() == "subdirs") {
	writeHeader(t);
	writeSubDirs(t);
	return TRUE;
    }
    return FALSE;
}

bool
DspMakefileGenerator::writeDspParts(QTextStream &t)
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

			QString mocpath = var( "QMAKE_MOC" );
			mocpath = mocpath.replace( QRegExp( "\\..*$" ), "" ) + " ";

			QString build = "\n\n# Begin Custom Build - Moc'ing " + mocablesFromMOC[(*it)] +
		    "...\n" "InputPath=.\\" + (*it) + "\n\n" "\"" + (*it) + "\""
					" : $(SOURCE) \"$(INTDIR)\" \"$(OUTDIR)\"\n"
					"\t" + mocpath + mocablesFromMOC[(*it)] + " -o " +
					(*it) + "\n\n" "# End Custom Build\n\n";

			t << "USERDEP_" << base << "=\"" << mocablesFromMOC[(*it)] << "\"" << endl << endl;
			t << "!IF  \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - Win32 Release\"" << build
			  << "!ELSEIF  \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - Win32 Debug\""
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
		    t << "# Begin Source File\n\nSOURCE=.\\" << (*it) << endl << endl;

		    if ( project->isActiveConfig("moc") && !mocablesToMOC[(*it)].isEmpty()) {
			QString base = (*it);
			base.replace(QRegExp("\\..*$"), "").upper();
			base.replace(QRegExp("[^a-zA-Z]"), "_");

			QString mocpath = var( "QMAKE_MOC" );
			mocpath = mocpath.replace( QRegExp( "\\..*$" ), "" ) + " ";

			QString build = "\n\n# Begin Custom Build - Moc'ing " + (*it) +
		    "...\n" "InputPath=.\\" + (*it) + "\n\n" "\"" + mocablesToMOC[(*it)] +
					"\"" " : $(SOURCE) \"$(INTDIR)\" \"$(OUTDIR)\"\n"
					"\t" + mocpath + (*it)  + " -o " +
					mocablesToMOC[(*it)] + "\n\n" "# End Custom Build\n\n";

			t << "!IF  \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - Win32 Release\"" << build
			  << "!ELSEIF  \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - Win32 Debug\""
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
		    t << "# Begin Source File\n\nSOURCE=.\\" << base << "\n# End Source File" << endl;
		}
	    }
	    else if(variable == "MSVCDSP_INTERFACES") {
		if(project->variables()["INTERFACES"].isEmpty())
		    continue;

		QString uicpath = var("QMAKE_UIC");
		uicpath = uicpath.replace(QRegExp("\\..*$"), "") + " ";
		QString mocpath = var( "QMAKE_MOC" );
		mocpath = mocpath.replace( QRegExp( "\\..*$" ), "" ) + " ";

		QStringList &list = project->variables()["INTERFACES"];
		for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
		    t <<  "# Begin Source File\n\nSOURCE=.\\" << (*it) << endl;

		    QString fname = (*it);
		    fname.replace(QRegExp("\\.ui"), "");
		    int lbs = fname.find( "\\" );
		    QString fpath;
		    if ( lbs != -1 )
			fpath = fname.left( lbs + 1 );
		    fname = fname.right( fname.length() - fname.find( "\\" ) - 1 );
		    
		    QString mocFile;
		    if(!project->variables()["MOC_DIR"].isEmpty())
			mocFile = project->variables()["MOC_DIR"].first();
		    else
			mocFile = fpath;

		    QString build = "\n\n# Begin Custom Build - Uic'ing " + (*it) + "...\n"
			"InputPath=.\\" + (*it) + "\n\n" "BuildCmds= \\\n\t" + uicpath + (*it) +
		     " -o " + fpath + fname + ".h \\\n" "\t" + uicpath  + (*it) +
		     " -i " + fpath + fname + ".h -o " + fpath + fname + ".cpp \\\n"
		     "\t" + mocpath + fpath + fname + ".h -o " + mocFile + "moc_" + fname + ".cpp \\\n\n"
//		     "\t" + mocpath + fname + ".h -o " + mocablesToMOC[ fname + ".h" ] + "\\\n\n"
		     "\"" + fpath + fname + ".h\" : \"$(SOURCE)\" \"$(INTDIR)\" \"$(OUTDIR)\""  "\n"
		     "\t$(BuildCmds)\n\n"
		     "\"" + fpath + fname + ".cpp\" : \"$(SOURCE)\" \"$(INTDIR)\" \"$(OUTDIR)\"" "\n"
		     "\t$(BuildCmds)\n\n"
		     "\"" + mocFile + "moc_" + fname + ".cpp\" : \"$(SOURCE)\" \"$(INTDIR)\" \"$(OUTDIR)\"" "\n"
		     "\t$(BuildCmds)\n\n" "# End Custom Build\n\n";

		    t << "!IF  \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - Win32 Release\"" << build
		      << "!ELSEIF  \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - Win32 Debug\"" << build
		      << "!ENDIF \n\n"
		      << "# End Source File" << endl;
		}
	    } else if(variable == "MSVCDSP_LEXSOURCES") {
		if(project->variables()["LEXSOURCES"].isEmpty())
		    continue;

		QString lexpath = var("QMAKE_LEX") + varGlue("QMAKE_LEXFLAGS", " ", " ", "") + " ";

		QStringList &l = project->variables()["LEXSOURCES"];
		for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
		    t <<  "# Begin Source File\n\nSOURCE=.\\" << (*it) << endl;

		    QString fname = (*it);
		    fname.replace(QRegExp("\\.l"), Option::lex_mod + Option::cpp_ext);

		    QString build = "\n\n# Begin Custom Build - Lex'ing " + (*it) + "...\n"
			"InputPath=.\\" + (*it) + "\n\n"
			"\"" + fname + "\" : \"$(SOURCE)\" \"$(INTDIR)\" \"$(OUTDIR)\"" "\n"
			"\t" + lexpath + (*it) + "\\\n"
			"\tdel " + fname + "\\\n"
			"\tcopy lex.yy.c " + fname + "\n\n" +
			"# End Custom Build\n\n";

		    t << "!IF  \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - Win32 Release\"" << build
		      << "!ELSEIF  \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - Win32 Debug\"" << build
		      << "!ENDIF \n\n"
		      << "# End Source File" << endl;
		}
	    } else if(variable == "MSVCDSP_YACCSOURCES") {
		if(project->variables()["YACCSOURCES"].isEmpty())
		    continue;

		QString yaccpath = var("QMAKE_YACC") + varGlue("QMAKE_YACCFLAGS", " ", " ", "") + " ";

		QStringList &l = project->variables()["YACCSOURCES"];
		for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
		    t <<  "# Begin Source File\n\nSOURCE=.\\" << (*it) << endl;

		    QString fname = (*it);
		    fname.replace(QRegExp("\\.y"), Option::yacc_mod);

		    QString build = "\n\n# Begin Custom Build - Yacc'ing " + (*it) + "...\n"
			"InputPath=.\\" + (*it) + "\n\n"
			"\"" + fname + Option::cpp_ext + "\" : \"$(SOURCE)\" \"$(INTDIR)\" \"$(OUTDIR)\"" "\n"
			"\t" + yaccpath + (*it) + "\\\n"
			"\tdel " + fname + Option::h_ext + "\\\n"
			"\trename y.tab.h " + fname + Option::h_ext + "\n\n" +
			"\tdel " + fname + Option::cpp_ext + "\\\n"
			"\trename y.tab.c " + fname + Option::cpp_ext + "\n\n" +
			"# End Custom Build\n\n";

		    t << "!IF  \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - Win32 Release\"" << build
		      << "!ELSEIF  \"$(CFG)\" == \"" << var("MSVCDSP_PROJECT") << " - Win32 Debug\"" << build
		      << "!ENDIF \n\n"
		      << "# End Source File" << endl;
		}
	    }
	    else
		t << var(variable);
	}
	t << line << endl;
    }
    t << endl;
    file.close();
    return TRUE;
}



void
DspMakefileGenerator::init()
{
    if(init_flag)
	return;
    QStringList::Iterator it;
    init_flag = TRUE;

    /* this should probably not be here, but I'm using it to wrap the .t files */
    if(project->variables()["TEMPLATE"].first() == "vcapp" )
	project->variables()["QMAKE_APP_FLAG"].append("1");
    else if(project->variables()["TEMPLATE"].first() == "vclib")
	project->variables()["QMAKE_LIB_FLAG"].append("1");
    
    QStringList &configs = project->variables()["CONFIG"];
    if (project->isActiveConfig("qt_dll"))
	if(configs.findIndex("qt") == -1) configs.append("qt");
    if ( project->isActiveConfig("qt") ) {
	if ( project->isActiveConfig( "plugin" ) ) {
	    project->variables()["CONFIG"].append("dll");
	    project->variables()["DEFINES"].append("QT_DLL");
	}
	if ( (project->variables()["DEFINES"].findIndex("QT_NODLL") == -1) &&
	     ((project->variables()["DEFINES"].findIndex("QT_MAKEDLL") != -1 ||
	       project->variables()["DEFINES"].findIndex("QT_DLL") != -1) ||
	      (getenv("QT_DLL") && !getenv("QT_NODLL"))) ) {
	    project->variables()["QMAKE_QT_DLL"].append("1");
	    if ( (project->variables()["TARGET"].first() == "qt" ||
		  (project->variables()["TARGET"].first() == "qt-mt") &&
		  !project->variables()["QMAKE_LIB_FLAG"].isEmpty() ))
		project->variables()["CONFIG"].append("dll");
	}
    }
    if ( project->isActiveConfig("dll") || !project->variables()["QMAKE_APP_FLAG"].isEmpty() ) {
	project->variables()["CONFIG"].remove("staticlib");
	project->variables()["QMAKE_APP_OR_DLL"].append("1");
    } else {
	project->variables()["CONFIG"].append("staticlib");
    }

    if ( project->isActiveConfig("qt") || project->isActiveConfig("opengl") ) {
	project->variables()["CONFIG"].append("windows");
    }

    if ( project->isActiveConfig("qt") ) {
	project->variables()["CONFIG"].append("moc");
	project->variables()["DEFINES"].append("UNICODE");
	project->variables()["INCLUDEPATH"] +=	project->variables()["QMAKE_INCDIR_QT"];
	project->variables()["QMAKE_LIBS"] += QStringList::split(' ', "imm32.lib wsock32.lib winmm.lib winspool.lib");
	if ( project->isActiveConfig("opengl") ) {
	    project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_QT_OPENGL"];
	}

	if ( (project->variables()["TARGET"].first() == "qt" ||
	      project->variables()["TARGET"].first() == "qt-mt") &&
	     !project->variables()["QMAKE_LIB_FLAG"].isEmpty() ) {
	    if ( !project->variables()["QMAKE_QT_DLL"].isEmpty() ) {
		project->variables()["DEFINES"].append("QT_MAKEDLL");
		project->variables()["MSVCDSP_DLLBASE"].append("/base:\"0x39D00000\"");
	    }
	} else {
	    if(project->isActiveConfig("thread"))
		project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_QT_THREAD"];
	    else
		project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_QT"];
	    if ( !project->variables()["QMAKE_QT_DLL"].isEmpty() ) {
		int hver = findHighestVersion(project->variables()["QMAKE_LIBDIR_QT"].first(), "qt");
		if(hver != -1) {
		    QString ver;
		    ver.sprintf("qt%s%d.lib", (project->isActiveConfig("thread") ? "-mt" : ""), hver);
		    QStringList &libs = project->variables()["QMAKE_LIBS"];
		    for(QStringList::Iterator libit = libs.begin(); libit != libs.end(); ++libit)
			(*libit).replace(QRegExp("qt(-mt)?\\.lib"), ver);
		}
		if ( !project->isActiveConfig("dll") ) {
		    project->variables()["QMAKE_LIBS"] +=project->variables()["QMAKE_LIBS_QT_DLL"];
		}
	    }
	}

    }
    if ( project->isActiveConfig("opengl") ) {
	project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_OPENGL"];
    }
    if ( project->isActiveConfig("thread") ) {
	project->variables()["DEFINES"].append("QT_THREAD_SUPPORT" );
        if ( project->isActiveConfig("dll") || project->variables()["TARGET"].first() == "qtmain" 
            || !project->variables()["QMAKE_QT_DLL"].isEmpty() ) {
	    project->variables()["MSVCDSP_MTDEFD"].append("-MDd");
	    project->variables()["MSVCDSP_MTDEF"].append("-MD");
	} else {
	    project->variables()["MSVCDSP_MTDEFD"].append("-MTd");
	    project->variables()["MSVCDSP_MTDEF"].append("-MT");
	}
    }
    if ( project->isActiveConfig("dll") ) {
	if ( !project->variables()["QMAKE_LIB_FLAG"].isEmpty() ) {
	    QString ver_xyz(project->variables()["VERSION"].first());
	    ver_xyz.replace(QRegExp("\\."), "");
	    project->variables()["TARGET_EXT"].append(ver_xyz + ".dll");
	} else {
	    project->variables()["TARGET_EXT"].append(".dll");
	}
    } else {
	if ( !project->variables()["QMAKE_APP_FLAG"].isEmpty() ) {
	    project->variables()["TARGET_EXT"].append(".exe");
	} else {
	    project->variables()["TARGET_EXT"].append(".lib");
	}
    }
    project->variables()["TARGET"].first() += project->variables()["TARGET_EXT"].first();
    if ( project->isActiveConfig("moc") ) {
	setMocAware(TRUE);
    }
    project->variables()["QMAKE_LIBS"] += project->variables()["LIBS"];
    project->variables()["QMAKE_FILETAGS"] += QStringList::split(' ',
								 "HEADERS SOURCES DEF_FILE RC_FILE TARGET QMAKE_LIBS DESTDIR DLLDESTDIR INCLUDEPATH");
    QStringList &l = project->variables()["QMAKE_FILETAGS"];
    for(it = l.begin(); it != l.end(); ++it) {
	QStringList &gdmf = project->variables()[(*it)];
	for(QStringList::Iterator inner = gdmf.begin(); inner != gdmf.end(); ++inner)
	    (*inner) = Option::fixPathToTargetOS((*inner), FALSE);
    }
    MakefileGenerator::init();
    project->variables()["MSVCDSP_VER"] = "6.00";
    project->variables()["MSVCDSP_DEBUG_OPT"] = "/GZ /ZI";
    QString msvcdsp_project = Option::output.name();
    msvcdsp_project = msvcdsp_project.right( msvcdsp_project.length() - msvcdsp_project.findRev( "\\" ) - 1 );
    msvcdsp_project = msvcdsp_project.left( msvcdsp_project.findRev( "." ) );

    project->variables()["MSVCDSP_PROJECT"].append(msvcdsp_project);
    QStringList &proj = project->variables()["MSVCDSP_PROJECT"];

    for(it = proj.begin(); it != proj.end(); ++it)
	(*it).replace(QRegExp("\\.[a-zA-Z0-9_]*$"), "");

    if ( !project->variables()["QMAKE_APP_FLAG"].isEmpty() ) {
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
    project->variables()["MSVCDSP_LIBS"] = project->variables()["QMAKE_LIBS"];
    project->variables()["MSVCDSP_DEFINES"].append(varGlue("DEFINES","/D ","" " /D ",""));
    project->variables()["MSVCDSP_INCPATH"].append(varGlue("INCLUDEPATH","/I "," /I ",""));
    if ( project->isActiveConfig("qt") ) {
	project->variables()["MSVCDSP_RELDEFS"].append("/D \"QT_NO_DEBUG\"");
    } else {
	project->variables()["MSVCDSP_RELDEFS"].clear();
    }
    if ( !project->variables()["DESTDIR"].isEmpty() ) {
	project->variables()["TARGET"].first().prepend(project->variables()["DESTDIR"].first());
	Option::fixPathToTargetOS(project->variables()["TARGET"].first());
	project->variables()["MSVCDSP_TARGET"].append(
	    QString("/out:\"") + project->variables()["TARGET"].first() + "\"");
	if ( project->isActiveConfig("dll") ) {
	    QString imp = project->variables()["TARGET"].first();
	    imp.replace(QRegExp("\\.dll"), ".lib");
	    project->variables()["MSVCDSP_TARGET"].append(QString(" /implib:\"") + imp + "\"");
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
	!QFile::exists((ret = QString(getenv("QTDIR")) + "/mkspecs/etc/" + file)))
	return "";
    return ret;
}
