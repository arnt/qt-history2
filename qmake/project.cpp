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
#include <stdio.h>
#include <stdlib.h>

#if 0
/* conf file */
#include <qfile.h>
#include <qtextstream.h>


extern int line_count;
extern "C" int yyerror(const char *);

bool
QMakeConfig::read(const char *fn)
{
    vars.clear(); /* always clear */

    QString file;
    if(!fn)
	file.sprintf("%s/tmake.conf", getenv("TMAKEPATH"));
    else 
	file = fn;

#ifdef QMAKE_DEBUG
    printf("Config API: reading %s\n", file.latin1());
#endif
    QFile qfile(file);
    if ( qfile.open(IO_ReadOnly) ) {
	cfile = file;

	QTextStream t( &qfile );
	QString s, var, val;
	line_count = 0;
	while ( !t.eof() ) {
	    line_count++;
	    s = t.readLine().stripWhiteSpace();

	    if(s[0] == '#' || s.isEmpty()) /* comment || blank_line */
		continue;

	    int equals = s.find('=');
	    if(equals == -1) {
		yyerror("Parse error");
		return FALSE;
	    }
	    var = s.left(equals).stripWhiteSpace();
	    val = s.right(s.length() - equals - 1).stripWhiteSpace();

	    int rep;
	    while((rep = val.find(QRegExp("\\$\\$[a-zA-Z0-9_-]*"))) != -1) {
		QString torep = val.mid(rep, val.find(QRegExp("[^\\$a-zA-Z0-9_-]"), rep) - rep);
		QString &l = vars[torep.right(torep.length()-2)];
#ifdef QMAKE_DEBUG
		printf("Config parser: (%s) :: %s -> %s\n", var.latin1(), torep.latin1(), l.latin1());
#endif	
		val.replace(rep, torep.length(), l);
	    }
#ifdef QMAKE_DEBUG
	    printf("Config Parser: %s == %s\n", var.latin1(), val.latin1());
#endif
	    vars[var] = val;
	}
	qfile.close();
	return TRUE;
    }
    return FALSE;
}
#endif

/* pro file */
extern QMakeProject *g_proj;
extern FILE *yyin;
extern "C" int yyparse();

bool
QMakeProject::read(const char *project, const char *config)
{
    bool ret = FALSE;
    vars.clear(); /* always clear */
    
    /* parse config file */
    QString conf;
    if(!config)
	conf.sprintf("%s/tmake.conf", getenv("TMAKEPATH"));
    else 
	conf = config;
#ifdef QMAKE_DEBUG
    printf("Config file: reading %s\n", conf.latin1());
#endif
    if(!(yyin = fopen(conf.latin1(), "r")))
       return FALSE;
    cfile = conf;
    g_proj = this;
    ret = !yyparse();
    g_proj = NULL;
    if(!ret)
	return FALSE;

    /* parse project file */
#ifdef QMAKE_DEBUG
    printf("Project file: reading %s\n", project);
#endif
    if(!(yyin = fopen(project, "r")))
	return FALSE;
    pfile = project;
    g_proj = this;
    ret = !yyparse();
    g_proj = NULL;

    /* done */
    return ret;
}

bool
QMakeProject::isActiveConfig(const QString &x)
{
    if(x.isEmpty()) 
	return TRUE;
#if defined(WIN32)
    if(x == "win32")
	return TRUE;
#else
    if(x == "unix")
	return TRUE;
#endif
    return vars["CONFIG"].findIndex(x) != -1;
}
