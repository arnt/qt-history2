/****************************************************************************
** $Id: $
**
** Definition of VcprojGenerator class.
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

#include "msvc_vcproj.h"
#include "option.h"
#include <qdir.h>
#include <qregexp.h>
#include <stdlib.h>
#include <time.h>


VcprojGenerator::VcprojGenerator(QMakeProject *p) : Win32MakefileGenerator(p), init_flag(FALSE)
{

}

bool VcprojGenerator::writeMakefile(QTextStream &t)
{
    // Check if all requirements are fullfilled
    if(!project->variables()["QMAKE_FAILED_REQUIREMENTS"].isEmpty()) {
	fprintf(stderr, "Project file not generated because all requirements not met:\n\t%s\n",
		var("QMAKE_FAILED_REQUIREMENTS").latin1());
	return TRUE;
    }

    // Generate full project file
    if(project->first("TEMPLATE") == "vcapp" ||
       project->first("TEMPLATE") == "vclib") {
	return writeVcprojParts(t);
    }
    
    // Generate recursive project
    else if(project->first("TEMPLATE") == "subdirs") {
	writeHeader(t);
	writeSubDirs(t);
	return TRUE;
    }
    return FALSE;
}

bool VcprojGenerator::writeVcprojParts(QTextStream &t)
{
    // Find proper template to use, either default or specified
    QString vcprojfile;
    if ( !project->variables()["VCPROJ_TEMPLATE"].isEmpty() ) {
	vcprojfile = project->first("VCPROJ_TEMPLATE");
    } else {
	vcprojfile = project->first("MSVCPROJ_TEMPLATE");
    }

    // Search current, mkspecs and ~/.tmake for correct template file
    QString vcprojfile_loc = findTemplate(vcprojfile);

    // Open the template file
    QFile file(vcprojfile_loc);
    if(!file.open(IO_ReadOnly)) {
	fprintf(stderr, "Cannot open vcproj file: %s\n", vcprojfile.latin1());
	return FALSE;
    }
    QTextStream vcproj(&file);


    int rep;
    QString line;
    while ( !vcproj.eof() ) {
	line = vcproj.readLine();
	while((rep = line.find(QRegExp("\\$\\$[a-zA-Z0-9_-]*"))) != -1) {
	    QString torep = line.mid(rep, line.find(QRegExp("[^\\$a-zA-Z0-9_-]"), rep) - rep);
	    QString variable = torep.right(torep.length()-2);
	    t << line.left(rep); //output the left side
	    line = line.right(line.length() - (rep + torep.length())); //now past the variable

	    // Replacing variables in the template file...
	    if( variable == "MSVCPROJ_HEADERS" &&
	       !project->variables()["HEADERS"].isEmpty() ) {
		writeHeaders( t );
	    }
	    else if( variable == "MSVCPROJ_SOURCES" &&
		    !project->variables()["SOURCES"].isEmpty()) {
		writeSources( t );
	    }
	    else if( variable == "MSVCPROJ_MOCSOURCES" && project->isActiveConfig("moc") &&
		    !project->variables()["SRCMOC"].isEmpty() ) {
		writeMocs( t );
	    }
	    else if( variable == "MSVCPROJ_LEXSOURCES" &&
		    !project->variables()["LEXSOURCES"].isEmpty() ) {
		writeLexs( t );
	    } 
	    else if( variable == "MSVCPROJ_YACCSOURCES" &&
		    !project->variables()["YACCSOURCES"].isEmpty() ) {
		writeYaccs( t );
	    } 
	    else if( variable == "MSVCPROJ_PICTURES" &&
		    !project->variables()["IMAGES"].isEmpty() ) {
		writePictures( t );
	    }
	    else if( variable == "MSVCPROJ_IMAGES" &&
		    !project->variables()["IMAGES"].isEmpty()) {
		writeImages( t );
	    }
	    else if( variable == "MSVCPROJ_IDLSOURCES" ) {
	    	writeIDLs( t );
	    }
	    else if( variable == "MSVCPROJ_FORMS" &&
		    !project->variables()["FORMS"].isEmpty() ) {
		writeForms( t );
	    } 
	    else if( (variable == "MSVCPROJ_FORMSOURCES" || variable == "MSVCPROJ_FORMHEADERS" ) &&
		    !project->variables()["FORMS"].isEmpty() ) {
		writeFormsSourceHeaders( variable, t );		
	    }
	    else if( variable == "MSVCPROJ_TRANSLATIONS" &&
		    !project->variables()["TRANSLATIONS"].isEmpty() ) {
		writeTranslations( t );
	    }
	    /*else if( variable == "MSVCPROJ_STRIPPEDTRANSLATIONS" &&
		      !project->variables()["TRANSLATIONS"].isEmpty() ) {
		writeStrippedTranslations( t );
	    }*/
	    else if( variable == "MSVCPROJ_CONFIGMODE" ) {
		if( project->isActiveConfig( "release" ) )
		    t << "Release";
		else
		    t << "Debug";
	    } 
	    // Unknown variable, so look for it in default Project Variables
	    else {
	        qDebug( "Unknown variable hit! ( %s )", variable.latin1() );
		t << var(variable);
	    }

	} // No more variables?
	// Then output the remaining text on the line...
	t << line << endl;
    }
    
    t << endl;
    file.close();
    return TRUE;
}

// $$MSVCPROJ_HEADERS ------------------------------------------------
void VcprojGenerator::writeHeaders( QTextStream &t )
{
	QStringList &list = project->variables()["HEADERS"];
	for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
	    t << "# Begin Source File\n\nSOURCE=" << (*it) << endl << endl;
	    if ( project->isActiveConfig("moc") && !findMocDestination((*it)).isEmpty()) {
		QString base = (*it);
		base.replace(QRegExp("\\..*$"), "").upper();
		base.replace(QRegExp("[^a-zA-Z]"), "_");

		QString mocpath = var( "QMAKE_MOC" );
		mocpath = mocpath.replace( QRegExp( "\\..*$" ), "" ) + " ";

		QString build = "\n\n# Begin Custom Build - Moc'ing " + (*it) +
				"...\n" "InputPath=.\\" + (*it) + "\n\n" "\"" + findMocDestination((*it)) +
				"\"" " : $(SOURCE) \"$(INTDIR)\" \"$(OUTDIR)\"\n"
				"\t" + mocpath + (*it)  + " -o " +
				findMocDestination((*it)) + "\n\n" "# End Custom Build\n\n";

		t << "USERDEP_" << base << "=\"$(QTDIR)\\bin\\moc.exe\"" << endl << endl;

		t << "!IF  \"$(CFG)\" == \"" << var("MSVCPROJ_PROJECT") << " - Win32 Release\"" << build
		  << "!ELSEIF  \"$(CFG)\" == \"" << var("MSVCPROJ_PROJECT") << " - Win32 Debug\""
		  << build << "!ENDIF " << endl << endl;
	    }
	    t << "# End Source File" << endl;
	}
}
	
// $$MSVCPROJ_SOURCES ------------------------------------------------
void VcprojGenerator::writeSources( QTextStream &t )
{
	QString mocpath = var( "QMAKE_MOC" );
	mocpath = mocpath.replace( QRegExp( "\\..*$" ), "" ) + " ";
	
	QStringList &list = project->variables()["SOURCES"];
	QStringList::Iterator it;
	for( it = list.begin(); it != list.end(); ++it) {
	    t << "# Begin Source File\n\nSOURCE=" << (*it) << endl;
	    if ( project->isActiveConfig("moc") &&
		 (*it).right(qstrlen(Option::moc_ext)) == Option::moc_ext) {
		QString base = (*it);
		base.replace(QRegExp("\\..*$"), "").upper();
		base.replace(QRegExp("[^a-zA-Z]"), "_");
	
		QString build = "\n\n# Begin Custom Build - Moc'ing " + findMocSource((*it)) +
				"...\n" "InputPath=.\\" + (*it) + "\n\n" "\"" + (*it) + "\""
				" : $(SOURCE) \"$(INTDIR)\" \"$(OUTDIR)\"\n"
				"\t" + mocpath + findMocSource((*it)) + " -o " +
				(*it) + "\n\n" "# End Custom Build\n\n";
	
		t << "USERDEP_" << base << "=\"" << findMocSource((*it)) << "\" \"$(QTDIR)\\bin\\moc.exe\"" << endl << endl;
	
		t << "!IF  \"$(CFG)\" == \"" << var("MSVCPROJ_PROJECT") << " - Win32 Release\"" << build
		  << "!ELSEIF  \"$(CFG)\" == \"" << var("MSVCPROJ_PROJECT") << " - Win32 Debug\""
		  << build << "!ENDIF " << endl << endl;
	    }
	    t << "# End Source File" << endl;
	}
	list = project->variables()["DEF_FILE"];
	for( it = list.begin(); it != list.end(); ++it ) {
	    t << "# Begin Source File\n\nSOURCE=" << (*it) << endl;
	    t << "# End Source File" << endl;
	}
}

// $$MSVCPROJ_MOCSOURCES ---------------------------------------------
void VcprojGenerator::writeMocs( QTextStream &t )
{
	QString mocpath = var( "QMAKE_MOC" );
	mocpath = mocpath.replace( QRegExp( "\\..*$" ), "" ) + " ";

	QStringList &list = project->variables()["SRCMOC"];
	for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
	    t << "# Begin Source File\n\nSOURCE=" << (*it) << endl;
	    if ( project->isActiveConfig("moc") &&
		 (*it).right(qstrlen(Option::moc_ext)) == Option::moc_ext) {
		QString base = (*it);
		base.replace(QRegExp("\\..*$"), "").upper();
		base.replace(QRegExp("[^a-zA-Z]"), "_");

		QString build = "\n\n# Begin Custom Build - Moc'ing " + findMocSource((*it)) +
				"...\n" "InputPath=.\\" + (*it) + "\n\n" "\"" + (*it) + "\""
				" : $(SOURCE) \"$(INTDIR)\" \"$(OUTDIR)\"\n"
				"\t" + mocpath + findMocSource((*it)) + " -o " +
				(*it) + "\n\n" "# End Custom Build\n\n";

		t << "USERDEP_" << base << "=\"" << findMocSource((*it)) << "\" \"$(QTDIR)\\bin\\moc.exe\"" << endl << endl;

		t << "!IF  \"$(CFG)\" == \"" << var("MSVCPROJ_PROJECT") << " - Win32 Release\"" << build
		  << "!ELSEIF  \"$(CFG)\" == \"" << var("MSVCPROJ_PROJECT") << " - Win32 Debug\""
		  << build << "!ENDIF " << endl << endl;
	    }
	    t << "# End Source File" << endl;
	}
}

// $$MSVCPROJ_LEXSOURCES ---------------------------------------------
void VcprojGenerator::writeLexs( QTextStream &t )
{
	t << "# Begin Group \"Lexables\"\n";
	t << "# Prop Default_Filter \"l\"\n";

	QString lexpath = var("QMAKE_LEX") + varGlue("QMAKE_LEXFLAGS", " ", " ", "") + " ";

	QStringList &l = project->variables()["LEXSOURCES"];
	for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
	    t <<  "# Begin Source File\n\nSOURCE=" << (*it) << endl;

	    QString fname = (*it);
	    fname.replace(QRegExp("\\.l"), Option::lex_mod + Option::cpp_ext.first());

	    QString build = "\n\n# Begin Custom Build - Lex'ing " + (*it) + "...\n"
		"InputPath=.\\" + (*it) + "\n\n"
		"\"" + fname + "\" : \"$(SOURCE)\" \"$(INTDIR)\" \"$(OUTDIR)\"" "\n"
		"\t" + lexpath + (*it) + "\\\n"
		"\tdel " + fname + "\\\n"
		"\tcopy lex.yy.c " + fname + "\n\n" +
		"# End Custom Build\n\n";


	    t << "!IF  \"$(CFG)\" == \"" << var("MSVCPROJ_PROJECT") << " - Win32 Release\"" << build
	      << "!ELSEIF  \"$(CFG)\" == \"" << var("MSVCPROJ_PROJECT") << " - Win32 Debug\"" << build
	      << "!ENDIF \n\n" << build
	
	      << "# End Source File" << endl;
	}

	t << "\n# End Group\n";
}

// $$MSVCPROJ_YACCSOURCES --------------------------------------------
void VcprojGenerator::writeYaccs( QTextStream &t )
{
	t << "# Begin Group \"Yaccables\"\n";
	t << "# Prop Default_Filter \"y\"\n";

	QString yaccpath = var("QMAKE_YACC") + varGlue("QMAKE_YACCFLAGS", " ", " ", "") + " ";

	QStringList &l = project->variables()["YACCSOURCES"];
	for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
	    t <<  "# Begin Source File\n\nSOURCE=" << (*it) << endl;

	    QString fname = (*it);
	    fname.replace(QRegExp("\\.y"), Option::yacc_mod);

	    QString build = "\n\n# Begin Custom Build - Yacc'ing " + (*it) + "...\n"
		"InputPath=.\\" + (*it) + "\n\n"
		"\"" + fname + Option::cpp_ext.first() + "\" : \"$(SOURCE)\" \"$(INTDIR)\" \"$(OUTDIR)\"" "\n"
		"\t" + yaccpath + (*it) + "\\\n"
		"\tdel " + fname + Option::h_ext.first() + "\\\n"
		"\tmove y.tab.h " + fname + Option::h_ext.first() + "\n\n" +
		"\tdel " + fname + Option::cpp_ext.first() + "\\\n"
		"\tmove y.tab.c " + fname + Option::cpp_ext.first() + "\n\n" +
		"# End Custom Build\n\n";

	    t << "!IF  \"$(CFG)\" == \"" << var("MSVCPROJ_PROJECT") << " - Win32 Release\"" << build
	      << "!ELSEIF  \"$(CFG)\" == \"" << var("MSVCPROJ_PROJECT") << " - Win32 Debug\"" << build
	      << "!ENDIF \n\n"
	      << "# End Source File" << endl;
	}

	t << "\n# End Group\n";
}

// $$MSVCPROJ_PICTURES -----------------------------------------------
void VcprojGenerator::writePictures( QTextStream &t )
{
	t << "# Begin Group \"Images\"\n";
	t << "# Prop Default_Filter \"png jpeg bmp xpm\"\n";
	
	QStringList &list = project->variables()["IMAGES"];
	for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
	    QString base = (*it);
	    t << "# Begin Source File\n\nSOURCE=" << base << endl;
	    t << "# End Source File" << endl;
	}

	t << "\n# End Group\n";
}

// $$MSVCPROJ_IMAGES -------------------------------------------------
void VcprojGenerator::writeImages( QTextStream &t )
{
	t << "# Begin Source File\n\nSOURCE=" << project->first("QMAKE_IMAGE_COLLECTION") << endl;
	t << "# End Source File" << endl;
}

// $$MSVCPROJ_IDLSOURCES ---------------------------------------------
void VcprojGenerator::writeIDLs( QTextStream &t )
{
	QStringList &l = project->variables()["MSVCPROJ_IDLSOURCES"];
	for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
	    t << "# Begin Source File" << endl << endl;
	    t << "SOURCE=" << (*it) << endl;
	    t << "# PROP Exclude_From_Build 1" << endl;
	    t << "# End Source File" << endl << endl;
	}
}

// $$MSVCPROJ_FORMS --------------------------------------------------
void VcprojGenerator::writeForms( QTextStream &t )
{
	t << "# Begin Group \"Forms\"\n";
	t << "# Prop Default_Filter \"ui\"\n";

	bool imagesBuildDone = FALSE;	    // Dirty hack to make it not create an output step for images more than once

	QString uicpath = var("QMAKE_UIC");
	uicpath = uicpath.replace(QRegExp("\\..*$"), "") + " ";
	QString mocpath = var( "QMAKE_MOC" );
	mocpath = mocpath.replace( QRegExp( "\\..*$" ), "" ) + " ";

	QStringList &list = project->variables()["FORMS"];
	for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
	    QString base = (*it);
	    t <<  "# Begin Source File\n\nSOURCE=" << base << endl;

	    QString fname = base;
	    fname.replace(QRegExp("\\.ui"), "");
	    int lbs = fname.findRev( "\\" );
	    QString fpath; 
	    if ( lbs != -1 )
		fpath = fname.left( lbs + 1 );
	    fname = fname.right( fname.length() - lbs - 1 );

	    QString mocFile;
	    if(!project->variables()["MOC_DIR"].isEmpty())
		mocFile = project->first("MOC_DIR");
	    else
		mocFile = fpath;

	    QString uiSourcesDir;
	    QString uiHeadersDir;
	    if(!project->variables()["UI_DIR"].isEmpty()) {
		uiSourcesDir = project->first("UI_DIR");
		uiHeadersDir = project->first("UI_DIR");
	    } else {
		if ( !project->variables()["UI_SOURCES_DIR"].isEmpty() )
		    uiSourcesDir = project->first("UI_SOURCES_DIR");
		else
		    uiSourcesDir = fpath;
		if ( !project->variables()["UI_HEADERS_DIR"].isEmpty() )
		    uiHeadersDir = project->first("UI_HEADERS_DIR");
		else
		    uiHeadersDir = fpath;
	    }

	    t << "USERDEP_" << base << "=\"$(QTDIR)\\bin\\moc.exe\" \"$(QTDIR)\\bin\\uic.exe\"" << endl << endl;

	    QString build = "\n\n# Begin Custom Build - Uic'ing " + base + "...\n"
		"InputPath=.\\" + base + "\n\n" "BuildCmds= \\\n\t" + uicpath + base +
		" -o " + uiHeadersDir + fname + ".h \\\n" "\t" + uicpath  + base +
		" -i " + fname + ".h -o " + uiSourcesDir + fname + ".cpp \\\n"
		"\t" + mocpath + uiHeadersDir + fname + ".h -o " + mocFile + "moc_" + fname + ".cpp \\\n";
	    
	    if ( !imagesBuildDone && !project->variables()["IMAGES"].isEmpty() ) {
		QString imagesBuild;
		QStringList &list = project->variables()["IMAGES"];
		for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
		    imagesBuild.append(" " + *it );
		}
		build.append("\t" + uicpath + "-embed " + project->first("QMAKE_ORIG_TARGET") + imagesBuild + " -o "
		    + project->first("QMAKE_IMAGE_COLLECTION") + " \\\n"); 
	    } 
	    
	    build.append("\n\"" + uiHeadersDir + fname + ".h\" : \"$(SOURCE)\" \"$(INTDIR)\" \"$(OUTDIR)\""  "\n"
		"\t$(BuildCmds)\n\n"
		"\"" + uiSourcesDir + fname + ".cpp\" : \"$(SOURCE)\" \"$(INTDIR)\" \"$(OUTDIR)\"" "\n"
		"\t$(BuildCmds)\n\n"
		"\"" + mocFile + "moc_" + fname + ".cpp\" : \"$(SOURCE)\" \"$(INTDIR)\" \"$(OUTDIR)\"" "\n"
		"\t$(BuildCmds)\n\n");
	    
	    if ( !imagesBuildDone && !project->variables()["IMAGES"].isEmpty() ) {
		build.append("\"" + project->first("QMAKE_IMAGE_COLLECTION") + "\"" + " : \"$(SOURCE)\" \"$(INTDIR)\" \"$(OUTDIR)\"" 
		    "\n" "\t$(BuildCmds)\n\n");
	    }
	    
	    build.append("# End Custom Build\n\n");

	    t << "!IF  \"$(CFG)\" == \"" << var("MSVCPROJ_PROJECT") << " - Win32 Release\"" << build
	      << "!ELSEIF  \"$(CFG)\" == \"" << var("MSVCPROJ_PROJECT") << " - Win32 Debug\"" << build
	      << "!ENDIF \n\n" << "# End Source File" << endl;
	    if ( !imagesBuildDone )
		imagesBuildDone = TRUE;
	}

	t << "\n# End Group\n";
}

// $$MSVCPROJ_FORMSOURCES / $$MSVCPROJ_FORMHEADERS -------------------
void VcprojGenerator::writeFormsSourceHeaders( QString &variable, QTextStream &t )
{
	QString uiSourcesDir;
	QString uiHeadersDir;
	if(!project->variables()["UI_DIR"].isEmpty()) {
	    uiSourcesDir = project->first("UI_DIR");
	    uiHeadersDir = project->first("UI_DIR");
	} else {
	    if ( !project->variables()["UI_SOURCES_DIR"].isEmpty() )
		uiSourcesDir = project->first("UI_SOURCES_DIR");
	    else
		uiSourcesDir = "";
	    if ( !project->variables()["UI_HEADERS_DIR"].isEmpty() )
		uiHeadersDir = project->first("UI_HEADERS_DIR");
	    else
		uiHeadersDir = "";
	}

	QStringList &list = project->variables()["FORMS"];
	QString ext = variable == "MSVCPROJ_FORMSOURCES" ? ".cpp" : ".h";
	for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
	    QString base = (*it);
	    int dot = base.findRev(".");
	    base.replace( dot, base.length() - dot, ext );
	    QString fname = base;

	    int lbs = fname.findRev( "\\" );
	    QString fpath; 
	    if ( lbs != -1 )
		fpath = fname.left( lbs + 1 );
	    fname = fname.right( fname.length() - lbs - 1 );

	    t << "# Begin Source File\n\nSOURCE=";
	    if ( ext == ".cpp" && !uiSourcesDir.isEmpty() )
		t << uiSourcesDir << fname;
	    else if ( ext == ".h" && !uiHeadersDir.isEmpty() )
		t << uiHeadersDir << fname;
	    else
		t << base;
	    t << "\n# End Source File" << endl;
	}
}

// $$MSVCPROJ_TRANSLATIONS -------------------------------------------
void VcprojGenerator::writeTranslations( QTextStream &t )
{
	t << "# Begin Group \"Translations\"\n";
	t << "# Prop Default_Filter \"ts\"\n";

	QStringList &list = project->variables()["TRANSLATIONS"];
	for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
	    QString sify = *it;
	    sify.replace(QRegExp("/"), "\\" );
	    QString base = (*it);
	    base.replace(QRegExp("\\..*$"), "").upper();
	    base.replace(QRegExp("[^a-zA-Z]"), "_");

	    t << "# Begin Source File\n\nSOURCE=" << sify << endl;

	    /*
	    QString userdep = "USERDEP_" + base + "=";
	    QStringList forms = project->variables()["FORMS"];
	    for ( QStringList::Iterator form = forms.begin(); form != forms.end();++form )
		userdep += "\"" + (*form) + "\"\t";
	    QStringList sources = project->variables()["SOURCES"];
	    for ( QStringList::Iterator source = sources.begin(); source != sources.end();++source )
		userdep += "\"" + (*source) + "\"\t";
	    userdep += "\n\n";

	    QString build = userdep + 
			    "# Begin Custom Build - lupdate'ing " + sify + "...\n"
			    "InputPath=.\\" + sify + "\n\n"
			    "\"tmp\\" + sify + "\" : $(SOURCE) \"$(INTDIR)\" \"$(QUTDIR)\"\n"
			    "\t$(QTDIR)\\bin\\lupdate -ts " + sify + " " + project->projectFile() + "\n"
			    "\tcopy " + sify +" tmp\\" + sify + "\n"
			    "# End Custom Build\n\n";

	    t << "USERDEP_" << base << "=\"$(QTDIR)\\bin\\lupdate.exe\" \"$(QTDIR)\\bin\\lrelease.exe\"" << endl << endl;

	    t << "!IF  \"$(CFG)\" == \"" << var("MSVCPROJ_PROJECT") << " - Win32 Release\"\n" << build
	      << "!ELSEIF  \"$(CFG)\" == \"" << var("MSVCPROJ_PROJECT") << " - Win32 Debug\"\n" << build 
	      << "!ENDIF " << endl << endl;
	    */

	    t << "\n# End Source File" << endl;
	}

	t << "\n# End Group\n";
}

// $$MSVCPROJ_STRIPPEDTRANSLATIONS -----------------------------------
void VcprojGenerator::writeStrippedTranslations( QTextStream &t )
{
	QStringList &list = project->variables()["TRANSLATIONS"];
	for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
	    QString sify = *it;
	    sify.replace( QRegExp("/"), "\\" );
	    QString qmfile = sify;
	    qmfile.replace( QRegExp("\\..*$"), ".qm" );
	    QString base = sify;
	    base.replace(QRegExp("\\..*$"), "").upper();
	    base.replace(QRegExp("[^a-zA-Z]"), "_");

	    QString build = "USERDEP_" + base + "=\"" + sify + "\"\t\"tmp\\" + sify + "\"\n\n" 
			    "# Begin Custom Build - lrelease'ing " + sify + "...\n"
			    "InputPath=.\\" + qmfile + "\n\n"
			    "\"" + qmfile + "\" : $(SOURCE) \"$(INTDIR)\" \"$(QUTDIR)\"\n"
			    "\t$(QTDIR)\\bin\\lrelease " + sify + "\n\n"
			    "# End Custom Build\n\n";
	    
	    t << "# Begin Source File\n\nSOURCE=.\\" << qmfile << endl;
	    t << "!IF  \"$(CFG)\" == \"" << var("MSVCPROJ_PROJECT") << " - Win32 Release\"\n" << build
	      << "!ELSEIF  \"$(CFG)\" == \"" << var("MSVCPROJ_PROJECT") << " - Win32 Debug\"\n" << build 
	      << "!ENDIF " << endl << endl;
	    t << "\n# End Source File" << endl;
	}
}






void VcprojGenerator::init()
{
    if( init_flag )
	return;
    init_flag = TRUE;
    QStringList::Iterator it;

    // this should probably not be here, but I'm using it to wrap the .t files
    if(project->first("TEMPLATE") == "vcapp" )
	project->variables()["QMAKE_APP_FLAG"].append("1");
    else if(project->first("TEMPLATE") == "vclib")
	project->variables()["QMAKE_LIB_FLAG"].append("1");

    // If QMakeSpec is not set in the .pro file, 
    // grab it from the environment
    if ( project->variables()["QMAKESPEC"].isEmpty() )
	project->variables()["QMAKESPEC"].append( getenv("QMAKESPEC") );

    bool is_qt = 
	( project->first("TARGET") == "qt"QTDLL_POSTFIX || 
	  project->first("TARGET") == "qt-mt"QTDLL_POSTFIX );

   QStringList &configs = project->variables()["CONFIG"];

    if ( project->isActiveConfig( "shared" ) )
	project->variables()["DEFINES"].append( "QT_DLL" );

    if ( project->isActiveConfig( "qt_dll" ) &&
	 configs.findIndex("qt") == -1 )
	    configs.append("qt");

    if ( project->isActiveConfig( "qt" ) ) {
	if ( project->isActiveConfig( "plugin" ) ) {
	    project->variables()["CONFIG"].append( "dll" );
	    project->variables()["DEFINES"].append( "QT_PLUGIN" );
	}
	if ( ( project->variables()["DEFINES"].findIndex( "QT_NODLL" )   == -1 ) &&
	    (( project->variables()["DEFINES"].findIndex( "QT_MAKEDLL" ) != -1   ||
	       project->variables()["DEFINES"].findIndex( "QT_DLL" )     != -1 ) ||
	     ( getenv( "QT_DLL" ) && !getenv( "QT_NODLL" ))) ) {
	    project->variables()["QMAKE_QT_DLL"].append( "1" );
	    if ( is_qt && !project->variables()["QMAKE_LIB_FLAG"].isEmpty() )
		project->variables()["CONFIG"].append( "dll" );
	}
    }

    // If we are a dll, then we cannot be a staticlib at the same time...
    if ( project->isActiveConfig( "dll" ) || !project->variables()["QMAKE_APP_FLAG"].isEmpty() ) {
	project->variables()["CONFIG"].remove( "staticlib" );
	project->variables()["QMAKE_APP_OR_DLL"].append( "1" );
    } else {
	project->variables()["CONFIG"].append( "staticlib" );
    }

    // If we need 'qt' and/or 'opengl', then we need windows and not console
    if ( project->isActiveConfig( "qt" ) || project->isActiveConfig( "opengl" ) ) {
	project->variables()["CONFIG"].append( "windows" );
    }
    
    // Decode version, and add it to $$MSVCPROJ_VERSION --------------
    if ( !project->variables()["VERSION"].isEmpty() ) {
	QString version = project->variables()["VERSION"][0];
	int firstDot = version.find( "." );
	QString major = version.left( firstDot );
	QString minor = version.right( version.length() - firstDot - 1 );
	minor.replace( QRegExp( "\\." ), "" );
	project->variables()["MSVCPROJ_VERSION"].append( "/VERSION:" + major + "." + minor );
    }

    // QT ------------------------------------------------------------
    if ( project->isActiveConfig("qt") ) {
	project->variables()["CONFIG"].append("moc");
	project->variables()["INCLUDEPATH"] +=	project->variables()["QMAKE_INCDIR_QT"];
	project->variables()["QMAKE_LIBDIR"] += project->variables()["QMAKE_LIBDIR_QT"];

	if ( is_qt && !project->variables()["QMAKE_LIB_FLAG"].isEmpty() ) {
	    if ( !project->variables()["QMAKE_QT_DLL"].isEmpty() ) {
		project->variables()["DEFINES"].append("QT_MAKEDLL");
		project->variables()["QMAKE_LFLAGS"].append("/base:\"0x39D00000\"");
	    }
	} else {
	    if(project->isActiveConfig("thread"))
		project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_QT_THREAD"];
	    else
		project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_QT"];
	    if ( !project->variables()["QMAKE_QT_DLL"].isEmpty() ) {
		int hver = findHighestVersion(project->first("QMAKE_LIBDIR_QT"), "qt");
		if(hver != -1) {
		    QString ver;
		    ver.sprintf("qt%s" QTDLL_POSTFIX "%d.lib", (project->isActiveConfig("thread") ? "-mt" : ""), hver);
		    QStringList &libs = project->variables()["QMAKE_LIBS"];
		    for(QStringList::Iterator libit = libs.begin(); libit != libs.end(); ++libit)
			(*libit).replace(QRegExp("qt(-mt)?\\.lib"), ver);
		}
	    }
	    if ( project->isActiveConfig( "activeqt" ) ) {
		project->variables().remove("QMAKE_LIBS_QT_ENTRY");
		project->variables()["QMAKE_LIBS_QT_ENTRY"] = "qaxserver.lib";
		if ( project->isActiveConfig( "dll" ) )
		    project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_QT_ENTRY"];
	    }
	    if ( !project->isActiveConfig("dll") && !project->isActiveConfig("plugin") ) {
		project->variables()["QMAKE_LIBS"] +=project->variables()["QMAKE_LIBS_QT_ENTRY"];
	    }
	}
    }

    // Set target directories ----------------------------------------
    if ( !project->first("OBJECTS_DIR").isEmpty() )
	project->variables()["MSVCPROJ_OBJECTSDIR"] = project->first("OBJECTS_DIR");
    else
	project->variables()["MSVCPROJ_OBJECTSDIR"] = project->isActiveConfig( "release" )?"Release":"Debug";
    if ( !project->first("DESTDIR").isEmpty() )
	project->variables()["MSVCPROJ_TARGETDIR"] = project->first("DESTDIR");
    else
	project->variables()["MSVCPROJ_TARGETDIR"] = project->isActiveConfig( "release" )?"Release":"Debug";
    
    // OPENGL --------------------------------------------------------
    if ( project->isActiveConfig("opengl") ) {
	project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_OPENGL"];
	project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_OPENGL"];
    }

    // THREAD --------------------------------------------------------
    if ( project->isActiveConfig("thread") ) {
	if(project->isActiveConfig("qt"))
	    project->variables()[is_qt ? "PRL_EXPORT_DEFINES" : "DEFINES"].append("QT_THREAD_SUPPORT" );
        if ( project->isActiveConfig("dll") || project->first("TARGET") == "qtmain"
            || !project->variables()["QMAKE_QT_DLL"].isEmpty() ) {
	    project->variables()["MSVCPROJ_MTDEFD"] += project->variables()["QMAKE_CXXFLAGS_MT_DLLDBG"];
	    project->variables()["MSVCPROJ_MTDEF"] += project->variables()["QMAKE_CXXFLAGS_MT_DLL"];
	} else {
	    // YES we want to use the DLL even in a static build
	    project->variables()["MSVCPROJ_MTDEFD"] += project->variables()["QMAKE_CXXFLAGS_MT_DBG"];
	    project->variables()["MSVCPROJ_MTDEF"] += project->variables()["QMAKE_CXXFLAGS_MT"];
	}
	if ( !project->variables()["DEFINES"].contains("QT_DLL") && is_qt
	     && project->first("TARGET") != "qtmain" )
	    project->variables()["QMAKE_LFLAGS"].append("/NODEFAULTLIB:\"libc\"");
    }

    // STL -----------------------------------------------------------
    if ( project->isActiveConfig("stl") )
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_STL"];

    // ACCESSIBILITY -------------------------------------------------
    if(project->isActiveConfig("qt")) {
	if ( project->isActiveConfig("accessibility" ) )
	    project->variables()[is_qt ? "PRL_EXPORT_DEFINES" : "DEFINES"].append("QT_ACCESSIBILITY_SUPPORT");
	if ( project->isActiveConfig("tablet") )
	    project->variables()[is_qt ? "PRL_EXPORT_DEFINES" : "DEFINES"].append("QT_TABLET_SUPPORT");
    }

    // DLL -----------------------------------------------------------
    if ( project->isActiveConfig("dll") ) {
	if ( !project->variables()["QMAKE_LIB_FLAG"].isEmpty() ) {
	    QString ver_xyz(project->first("VERSION"));
	    ver_xyz.replace(QRegExp("\\."), "");
	    project->variables()["TARGET_EXT"].append(ver_xyz + ".dll");
	} else {
	    project->variables()["TARGET_EXT"].append(".dll");
	}
    } 
    // EXE / LIB -----------------------------------------------------
    else {
	if ( !project->variables()["QMAKE_APP_FLAG"].isEmpty() ) 
	    project->variables()["TARGET_EXT"].append(".exe");
	else 
	    project->variables()["TARGET_EXT"].append(".lib");
    }

    project->variables()["MSVCPROJ_VER"] = "6.00";
    project->variables()["MSVCPROJ_DEBUG_OPT"] = "/GZ /ZI";

    // INCREMENTAL:NO ------------------------------------------------
    if(!project->isActiveConfig("incremental")) {
	project->variables()["QMAKE_LFLAGS"].append(QString("/incremental:no"));
        if ( is_qt )
	    project->variables()["MSVCPROJ_DEBUG_OPT"] = "/GZ /Zi";
    }

    // MOC -----------------------------------------------------------
    if ( project->isActiveConfig("moc") ) 
	setMocAware(TRUE);


    project->variables()["QMAKE_LIBS"] += project->variables()["LIBS"];
    
    // Run through all variables containing filepaths, and -----------
    // slash-slosh them correctly depending on current OS  -----------
    project->variables()["QMAKE_FILETAGS"] += QStringList::split(' ', "HEADERS SOURCES DEF_FILE RC_FILE TARGET QMAKE_LIBS DESTDIR DLLDESTDIR INCLUDEPATH");
    QStringList &l = project->variables()["QMAKE_FILETAGS"];
    for(it = l.begin(); it != l.end(); ++it) {
	QStringList &gdmf = project->variables()[(*it)];
	for(QStringList::Iterator inner = gdmf.begin(); inner != gdmf.end(); ++inner)
	    (*inner) = Option::fixPathToTargetOS((*inner), FALSE);
    }

     // Get filename w/o extention -----------------------------------
    QString msvcproj_project = ""; 
    QString targetfilename = "";
    if ( project->variables()["TARGET"].count() ) {
	msvcproj_project = project->variables()["TARGET"].first();
	targetfilename = msvcproj_project;
    }

    // Save filename w/o extention in $$QMAKE_ORIG_TARGET ------------
    project->variables()["QMAKE_ORIG_TARGET"] = project->variables()["TARGET"];

    // TARGET (add extention to $$TARGET)
    //project->variables()["MSVCPROJ_DEFINES"].append(varGlue(".first() += project->first("TARGET_EXT");

    // Init base class too -------------------------------------------
    MakefileGenerator::init();
    
    
    if ( msvcproj_project.isEmpty() )
	msvcproj_project = Option::output.name();

    msvcproj_project = msvcproj_project.right( msvcproj_project.length() - msvcproj_project.findRev( "\\" ) - 1 );
    msvcproj_project = msvcproj_project.left( msvcproj_project.findRev( "." ) );
    msvcproj_project.replace(QRegExp("-"), "");

    project->variables()["MSVCPROJ_PROJECT"].append(msvcproj_project);
    QStringList &proj = project->variables()["MSVCPROJ_PROJECT"];

    for(it = proj.begin(); it != proj.end(); ++it)
	(*it).replace(QRegExp("\\.[a-zA-Z0-9_]*$"), "");

    // SUBSYSTEM -----------------------------------------------------
    if ( !project->variables()["QMAKE_APP_FLAG"].isEmpty() ) {
	    project->variables()["MSVCPROJ_TEMPLATE"].append("win32app" + project->first( "VCPROJ_EXTENSION" ) );
	    if ( project->isActiveConfig("console") ) {
		project->variables()["MSVCPROJ_CONSOLE"].append("Console");
		project->variables()["MSVCPROJ_WINCONDEF"].append("_CONSOLE");
		project->variables()["MSVCPROJ_VCPROJTYPE"].append("0x0103");
		project->variables()["MSVCPROJ_SUBSYSTEM"].append("console");
	    } else {
		project->variables()["MSVCPROJ_CONSOLE"].clear();
		project->variables()["MSVCPROJ_WINCONDEF"].append("_WINDOWS");
		project->variables()["MSVCPROJ_VCPROJTYPE"].append("0x0101");
		project->variables()["MSVCPROJ_SUBSYSTEM"].append("windows");
	    }
    } else {
        if ( project->isActiveConfig("dll") ) {
            project->variables()["MSVCPROJ_TEMPLATE"].append("win32dll" + project->first( "VCPROJ_EXTENSION" ) );
        } else {
            project->variables()["MSVCPROJ_TEMPLATE"].append("win32lib" + project->first( "VCPROJ_EXTENSION" ) );
        }
    }

    // $$QMAKE.. -> $$MSVCPROJ.. -------------------------------------
    project->variables()["MSVCPROJ_LIBS"] += project->variables()["QMAKE_LIBS"];
    project->variables()["MSVCPROJ_LIBS"] += project->variables()["QMAKE_LIBS_WINDOWS"];
    project->variables()["MSVCPROJ_LFLAGS" ] += project->variables()["QMAKE_LFLAGS"];
    if ( !project->variables()["QMAKE_LIBDIR"].isEmpty() )
	project->variables()["MSVCPROJ_LFLAGS" ].append(varGlue("QMAKE_LIBDIR","/LIBPATH:\"","\" /LIBPATH:\"","\""));
    project->variables()["MSVCPROJ_CXXFLAGS" ] += project->variables()["QMAKE_CXXFLAGS"];
    project->variables()["MSVCPROJ_DEFINES"].append(varGlue("DEFINES","/D ","" " /D ",""));
    project->variables()["MSVCPROJ_DEFINES"].append(varGlue("PRL_EXPORT_DEFINES","/D ","" " /D ",""));
    QStringList &incs = project->variables()["INCLUDEPATH"];
    for(QStringList::Iterator incit = incs.begin(); incit != incs.end(); ++incit) {
	QString inc = (*incit);
	inc.replace(QRegExp("\""), "");
	project->variables()["MSVCPROJ_INCPATH"].append("/I \"" + inc + "\"");
    }
    project->variables()["MSVCPROJ_INCPATH"].append("/I \"" + specdir() + "\"");
    if ( project->isActiveConfig("qt") ) {
	project->variables()["MSVCPROJ_RELDEFS"].append("/D \"QT_NO_DEBUG\"");
    } else {
	project->variables()["MSVCPROJ_RELDEFS"].clear();
    }

    QString dest;
    if ( !project->variables()["DESTDIR"].isEmpty() ) {
	project->variables()["TARGET"].first().prepend(project->first("DESTDIR"));
	Option::fixPathToTargetOS(project->first("TARGET"));
	dest = project->first("TARGET");
        if ( project->first("TARGET").startsWith("$(QTDIR)") )
	    dest.replace( QRegExp("\\$\\(QTDIR\\)"), getenv("QTDIR") );
	project->variables()["MSVCPROJ_TARGET"].append(
	    QString("/out:\"") + dest + "\"");
	if ( project->isActiveConfig("dll") ) {
	    QString imp = dest;
	    imp.replace(QRegExp("\\.dll"), ".lib");
	    project->variables()["MSVCPROJ_TARGET"].append(QString(" /implib:\"") + imp + "\"");
	}
    }
    
    // DLL COPY ------------------------------------------------------
    if ( project->isActiveConfig("dll") && !project->variables()["DLLDESTDIR"].isEmpty() ) {
	QStringList dlldirs = project->variables()["DLLDESTDIR"];
	QString copydll = "# Begin Special Build Tool\n"
			"TargetPath=" + dest + "\n"
			"SOURCE=$(InputPath)\n"
			"PostBuild_Desc=Copy DLL to " + project->first("DLLDESTDIR") + "\n"
			"PostBuild_Cmds=";

	for ( QStringList::Iterator dlldir = dlldirs.begin(); dlldir != dlldirs.end(); ++dlldir ) {
	    copydll += "copy \"" + dest + "\" \"" + *dlldir + "\"\t";
	}

	copydll += "\n# End Special Build Tool";
	project->variables()["MSVCPROJ_COPY_DLL_REL"].append( copydll );
	project->variables()["MSVCPROJ_COPY_DLL_DBG"].append( copydll );
    }
    
    // ACTIVEQT ------------------------------------------------------
    if ( project->isActiveConfig("activeqt") ) {
	QString idl = project->variables()["QMAKE_IDL"].first();
	QString idc = project->variables()["QMAKE_IDC"].first();
	QString version = project->variables()["VERSION"].first();
	if ( version.isEmpty() )
	    version = "1.0";

	project->variables()["MSVCPROJ_IDLSOURCES"].append( "tmp\\" + targetfilename + ".idl" );
	project->variables()["MSVCPROJ_IDLSOURCES"].append( "tmp\\" + targetfilename + ".tlb" );
	project->variables()["MSVCPROJ_IDLSOURCES"].append( "tmp\\" + targetfilename + ".midl" );
	if ( project->isActiveConfig( "dll" ) ) {
	    QString regcmd = "# Begin Special Build Tool\n"
			    "TargetPath=" + targetfilename + "\n"
			    "SOURCE=$(InputPath)\n"
			    "PostBuild_Desc=Finalizing ActiveQt server...\n"
			    "PostBuild_Cmds=" +
			    idc + " %1 -idl tmp\\" + targetfilename + ".idl -version " + version +
			    "\t" + idl + " tmp\\" + targetfilename + ".idl /nologo /o tmp\\" + targetfilename + ".midl /tlb tmp\\" + targetfilename + ".tlb /iid tmp\\dump.midl /dlldata tmp\\dump.midl /cstub tmp\\dump.midl /header tmp\\dump.midl /proxy tmp\\dump.midl /sstub tmp\\dump.midl"
			    "\t" + idc + " %1 /tlb tmp\\" + targetfilename + ".tlb"
			    "\tregsvr32 /s %1\n"
			    "# End Special Build Tool";

	    QString executable = project->variables()["MSVCPROJ_TARGETDIRREL"].first() + "\\" + project->variables()["TARGET"].first();
	    project->variables()["MSVCPROJ_COPY_DLL_REL"].append( regcmd.arg(executable).arg(executable).arg(executable) );
	    
	    executable = project->variables()["MSVCPROJ_TARGETDIRDEB"].first() + "\\" + project->variables()["TARGET"].first();
	    project->variables()["MSVCPROJ_COPY_DLL_DBG"].append( regcmd.arg(executable).arg(executable).arg(executable) );
	} else {
	    QString regcmd = "# Begin Special Build Tool\n"
			    "TargetPath=" + targetfilename + "\n"
			    "SOURCE=$(InputPath)\n"
			    "PostBuild_Desc=Finalizing ActiveQt server...\n"
			    "PostBuild_Cmds="
			    "%1 -dumpidl tmp\\" + targetfilename + ".idl -version " + version +
			    "\t" + idl + " tmp\\" + targetfilename + ".idl /nologo /o tmp\\" + targetfilename + ".midl /tlb tmp\\" + targetfilename + ".tlb /iid tmp\\dump.midl /dlldata tmp\\dump.midl /cstub tmp\\dump.midl /header tmp\\dump.midl /proxy tmp\\dump.midl /sstub tmp\\dump.midl"
			    "\t" + idc + " %1 /tlb tmp\\" + targetfilename + ".tlb"
			    "\t%1 -regserver\n"
			    "# End Special Build Tool";

	    QString executable = project->variables()["MSVCPROJ_TARGETDIRREL"].first() + "\\" + project->variables()["TARGET"].first();
	    project->variables()["MSVCPROJ_REGSVR_REL"].append( regcmd.arg(executable).arg(executable).arg(executable) );
	    
	    executable = project->variables()["MSVCPROJ_TARGETDIRDEB"].first() + "\\" + project->variables()["TARGET"].first();
	    project->variables()["MSVCPROJ_REGSVR_DBG"].append( regcmd.arg(executable).arg(executable).arg(executable) );
	}
	
    }

    // RC_FILE -------------------------------------------------------
    if ( !project->variables()["SOURCES"].isEmpty() || !project->variables()["RC_FILE"].isEmpty() ) {
	project->variables()["SOURCES"] += project->variables()["RC_FILE"];
    }
    
    // FORMS ---------------------------------------------------------
    QStringList &list = project->variables()["FORMS"];
    for( it = list.begin(); it != list.end(); ++it ) {
	if ( QFile::exists( *it + ".h" ) )
	    project->variables()["SOURCES"].append( *it + ".h" );
    }
    
    project->variables()["QMAKE_INTERNAL_PRL_LIBS"] << "MSVCPROJ_LIBS";
}


QString VcprojGenerator::findTemplate(QString file)
{
    QString ret;
    if(!QFile::exists((ret = file)) &&
       !QFile::exists((ret = QString(Option::mkfile::qmakespec + "/" + file))) &&
       !QFile::exists((ret = QString(getenv("QTDIR")) + "/mkspecs/win32-msvc.NET/" + file)) &&
       !QFile::exists((ret = (QString(getenv("HOME")) + "/.tmake/" + file))))
	return "";
    return ret;
}


void VcprojGenerator::processPrlVariable(const QString &var, const QStringList &l) 
{
    if(var == "QMAKE_PRL_DEFINES") {
	QStringList &out = project->variables()["MSVCPROJ_DEFINES"];
	for(QStringList::ConstIterator it = l.begin(); it != l.end(); ++it) {
	    if(out.findIndex((*it)) == -1)
		out.append((" /D \"" + *it + "\""));
	}
    } else {
	MakefileGenerator::processPrlVariable(var, l);
    }
}
