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

#include "project.h"

#include "unixmake.h"
#include "borland_bmake.h"
#include "msvc_nmake.h"
#include "msvc_dsp.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "option.h"
#include <qnamespace.h>
#include <qregexp.h>
#include <qdir.h>

extern int line_count;
extern "C" void yyerror(const char *foo)
{
    printf("%d: %s\n", line_count, foo);
}

// for Borland, main is defined to qMain which breaks qmake
#undef main

int main(int argc, char **argv)
{
    /* parse command line */
    if(!Option::parseCommandLine(argc, argv))
	return 666;

    QString oldpwd = QDir::current().currentDirPath();

    QMakeProject proj;
    int exit_val = 0;
    for(QStringList::Iterator pfile = Option::project_files.begin();
	pfile != Option::project_files.end(); pfile++) {
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
	    fprintf(stderr, "Error processing project file: %s\n", (*pfile).latin1());
	    exit_val = 2;
	    continue;
	}

	/* figure out generator */
	MakefileGenerator *mkfile = NULL;
	QString gen = proj.variables()["MAKEFILE_GENERATOR"].first(), def_mkfile;
	if(gen.isEmpty()) {
	    fprintf(stderr, "No generator specified in config file: %s\n", fn.latin1());
	    exit_val = 3;
	} else if(gen == "UNIX") {
	    mkfile = new UnixMakefileGenerator(&proj);
	} else if(gen == "MSVC") {
	    if(proj.variables()["TEMPLATE"].first().find(QRegExp("^vc.*")) != -1) {
		def_mkfile = proj.variables()["QMAKE_TARGET"].first() + ".dsp";
		mkfile = new DspMakefileGenerator(&proj);
	    } else {
		mkfile = new NmakeMakefileGenerator(&proj);
	    }
	} else if(gen == "BMAKE") {
	    mkfile = new BorlandMakefileGenerator(&proj);
	} else {
	    fprintf(stderr, "Unknown generator specified: %s\n", gen.latin1());
	    exit_val = 4;
	}

	if(mkfile) {
	    /* open make file */
	    bool using_stdout = FALSE;
	    if(!(Option::output.state() & IO_Open)) {
		if(Option::output.name().isEmpty()) {
		    if(!proj.variables()["QMAKE_MAKEFILE"].isEmpty()) 
			Option::output.setName(proj.variables()["QMAKE_MAKEFILE"].first());
		    else if(!def_mkfile.isEmpty()) 
			Option::output.setName(def_mkfile);
		    else
			Option::output.setName("Makefile");
		}
		if(Option::output.name().isEmpty() || Option::output.name() == "-") {
		    Option::output.setName("");
		    Option::output.open(IO_WriteOnly | IO_Translate, stdout);
		    using_stdout = TRUE;
		} else {
		    if(QDir::isRelativePath(Option::output.name()))
			Option::output.setName(oldpwd + Option::dir_sep + Option::output.name());

		    QFileInfo fi(Option::output);
		    Option::output_dir = Option::fixPathToTargetOS(fi.dirPath());
		    if(!Option::output.open(IO_WriteOnly | IO_Translate)) {
			fprintf(stderr, "Failure to open file: %s\n", Option::output.name().latin1());
			return 5;
		    }
		}
	    }
	    if(!mkfile->write()) {
		fprintf(stderr, "Unable to generate makefile for: %s\n", (*pfile).latin1());
		if(!using_stdout)
		    QFile::remove(Option::output.name());
		exit_val = 6;
	    }
	    delete mkfile;
	}

	/* debugging */
	if(Option::debug_level) {
	    QMap<QString, QStringList> &vars = proj.variables();
	    for( QMap<QString, QStringList>::Iterator it = vars.begin(); it != vars.end(); ++it) {
		debug_msg(1, "%s === %s", it.key().latin1(), it.data().join(" ").latin1());
	    }
	}
    }
    return exit_val;
}
