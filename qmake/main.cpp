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

#include "project.h"
#include "option.h"
#include "unixmake.h"
#include "borland_bmake.h"
#include "msvc_nmake.h"
#include "msvc_dsp.h"
#include "mac/metrowerks_xml.h"
#include "projectgenerator.h"
#include <qnamespace.h>
#include <qregexp.h>
#include <qdir.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

extern int line_count;
extern "C" void yyerror(const char *foo)
{
    fprintf(stderr, "%d: %s\n", line_count, foo);
}

// for Borland, main is defined to qMain which breaks qmake
#undef main

int main(int argc, char **argv)
{
    /* parse command line */
    if(!Option::parseCommandLine(argc, argv))
	return 666;

    QDir sunworkshop42workaround = QDir::current();
    QString oldpwd = sunworkshop42workaround.currentDirPath();
    QMakeProject proj;
    int exit_val = 0;
    if(Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE ||
       Option::qmake_mode == Option::QMAKE_GENERATE_PRL) {
	for(QStringList::Iterator pfile = Option::mkfile::project_files.begin();
	    pfile != Option::mkfile::project_files.end(); pfile++) {
	    QString fn = (*pfile);

	    //setup pwd properly
	    debug_msg(1, "Resetting dir to: %s", oldpwd.latin1());
	    QDir::setCurrent(oldpwd); //reset the old pwd
	    int di = fn.findRev(Option::dir_sep);
	    if(di != -1) {
		debug_msg(1, "Changing dir to: %s", fn.left(di).latin1());
		if(!QDir::setCurrent(fn.left(fn.findRev(Option::dir_sep))))
		    fprintf(stderr, "Cannot find directory: %s\n", fn.left(di).latin1());
		fn = fn.right(fn.length() - di - 1);
	    }

	    /* read project.. */
	    if(!proj.read(fn, oldpwd)) {
		fprintf(stderr, "Error processing project file: %s\n", fn == "-" ? "(stdin)" : (*pfile).latin1());
		exit_val = 2;
		continue;
	    }

	    /* let Option post-process */
	    if(!Option::postProcessProject(&proj)) {
		fprintf(stderr, "Error post-processing project file: %s", fn == "-" ? "(stdin)" : (*pfile).latin1());
		exit_val = 8;
		continue;
	    }

	    /* figure out generator */
	    MakefileGenerator *mkfile = NULL;
	    QString gen = proj.first("MAKEFILE_GENERATOR"), def_mkfile;
	    if(gen.isEmpty()) {
		fprintf(stderr, "No generator specified in config file: %s\n", fn.latin1());
		exit_val = 3;
	    } else if(gen == "UNIX") {
		mkfile = new UnixMakefileGenerator(&proj);
	    } else if(gen == "MSVC") {
		if(proj.first("TEMPLATE").find(QRegExp("^vc.*")) != -1) {
		    def_mkfile = proj.first("TARGET") + proj.first( "DSP_EXTENSION" );
		    mkfile = new DspMakefileGenerator(&proj);
		} else {
		    mkfile = new NmakeMakefileGenerator(&proj);
		}
	    } else if(gen == "BMAKE") {
		mkfile = new BorlandMakefileGenerator(&proj);
	    } else if(gen == "METROWERKS") {
		def_mkfile = proj.first("TARGET") + ".xml";
		mkfile = new MetrowerksMakefileGenerator(&proj);
	    } else {
		fprintf(stderr, "Unknown generator specified: %s\n", gen.latin1());
		exit_val = 4;
	    }

	    bool using_stdout = FALSE;
	    if(Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE && mkfile) {
		/* open make file */
		if(!(Option::output.state() & IO_Open)) {
		    QString default_makefile = proj.first("QMAKE_MAKEFILE");
		    if(default_makefile.isEmpty()) {
			if(!def_mkfile.isEmpty())
			    default_makefile = def_mkfile;
			else
			    default_makefile = "Makefile";
			proj.variables()["QMAKE_MAKEFILE"].append(default_makefile);
		    }
		    if(Option::output.name().isEmpty())
			Option::output.setName(default_makefile);
		    if(Option::output.name().isEmpty() || Option::output.name() == "-") {
			Option::output.setName("");
			Option::output.open(IO_WriteOnly | IO_Translate, stdout);
			using_stdout = TRUE;
		    } else {
			if(QDir::isRelativePath(Option::output.name())) {
			    QString ofile;
			    ofile = Option::output.name();
			    if(proj.first("TEMPLATE").find(QRegExp("^vc.*")) != -1) {
				int slashfind = ofile.findRev( '\\' );
				if ( slashfind == -1 )
				    ofile = ofile.replace( QRegExp("-"), "_" );
				else { 
				    int hypenfind = ofile.find( '-', slashfind );
				    while ( hypenfind != -1 && slashfind < hypenfind ) {
					ofile = ofile.replace( hypenfind, 1, "_" );
					hypenfind = ofile.find( '-', hypenfind + 1 );
				    }
				}
			    }
			    Option::output.setName(Option::fixPathToLocalOS(oldpwd + Option::dir_sep + ofile ));
			}

			QFileInfo fi(Option::output);
			Option::output_dir = Option::fixPathToTargetOS(fi.dirPath());
			if(!Option::output.open(IO_WriteOnly | IO_Translate)) {
			    fprintf(stderr, "Failure to open file: %s\n",
				    Option::output.name().isEmpty() ? "(stdout)" : Option::output.name().latin1());
			    return 5;
			}
		    }
		}
	    } else {
		using_stdout = TRUE; //kind of..
	    }
	    if(mkfile && !mkfile->write()) {
		fprintf(stderr, "Unable to generate makefile for: %s\n", (*pfile).latin1());
		if(!using_stdout)
		    QFile::remove(Option::output.name());
		exit_val = 6;
		delete mkfile;
	    }

	    /* debugging */
	    if(Option::debug_level) {
		QMap<QString, QStringList> &vars = proj.variables();
		for( QMap<QString, QStringList>::Iterator it = vars.begin(); it != vars.end(); ++it) {
		    if(!it.data().isEmpty())
			debug_msg(1, "%s === %s", it.key().latin1(), it.data().join(" :: ").latin1());
		}
	    }
	}
    } else if(Option::qmake_mode == Option::QMAKE_GENERATE_PROJECT) {
	/* open project file */
	bool using_stdout = FALSE;
	if(!(Option::output.state() & IO_Open)) {
	    if(Option::output.name().isEmpty()) {
		QString dir = QDir::currentDirPath();
		int s = dir.findRev('/');
		if(s != -1)
		    dir = dir.right(dir.length() - (s + 1));
		Option::output.setName(dir + ".pro");
	    }
	    if(Option::output.name().isEmpty() || Option::output.name() == "-") {
		Option::output.setName("");
		Option::output.open(IO_WriteOnly | IO_Translate, stdout);
		using_stdout = TRUE;
	    } else {
		QString ofile;
		if(QDir::isRelativePath(Option::output.name())) {
		    ofile = Option::output.name();
		    int slashfind = ofile.findRev( '\\' );
		    if ( slashfind == -1 )
			ofile = ofile.replace( QRegExp("-"), "_" );
		    else { 
			int hypenfind = ofile.find( '-', slashfind );
			while ( hypenfind != -1 && slashfind < hypenfind ) {
			    ofile = ofile.replace( hypenfind, 1, "_" );
			    hypenfind = ofile.find( '-', hypenfind + 1 );
			}
		    }
		}
		
		Option::output.setName(oldpwd + Option::dir_sep + ofile );

		QFileInfo fi(Option::output);
		Option::output_dir = Option::fixPathToTargetOS(fi.dirPath());
		if(!Option::output.open(IO_WriteOnly | IO_Translate)) {
		    fprintf(stderr, "Failure to open file: %s\n", Option::output.name().latin1());
		    return 5;
		}
	    }
	}

	ProjectGenerator mkfile(&proj);
	if(!mkfile.write()) {
	    fprintf(stderr, "Unable to generate project file\n");
	    if(!using_stdout)
		QFile::remove(Option::output.name());
	    exit_val = 7;
	}

	/* debugging */
	if(Option::debug_level) {
	    QMap<QString, QStringList> &vars = proj.variables();
	    for( QMap<QString, QStringList>::Iterator it = vars.begin(); it != vars.end(); ++it) {
		if(!it.data().isEmpty())
		    debug_msg(1, "%s === %s", it.key().latin1(), it.data().join(" ").latin1());
	    }
	}
    }
    return exit_val;
}
