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
#include "project.h"
#include <stdio.h>
#include <stdlib.h>
#include <qfile.h>
#include <qtextstream.h>

int line_count;
extern "C" int yyerror(const char *);

/* pro file */
extern QMakeProject *g_proj;
extern FILE *yyin;
extern "C" int yyparse();

QMakeProject::QMakeProject()
{
    if(Option::debug_level)
	printf("Config file: reading %s\n", Option::specfile.latin1());
    
    QFile qfile(cfile = Option::specfile);
    if ( qfile.open(IO_ReadOnly) ) {
	QTextStream t( &qfile );
	QString s;
	line_count = 0;
	while ( !t.eof() ) {
	    line_count++;
	    s += t.readLine();
	    if(s.right(1) == "\\")
		s.truncate(s.length() - 1);
	    else {
		if(!parse(s))
		    break;
		s = "";
	    }
	}
	qfile.close();
    }
    for(QStringList::Iterator it = Option::user_vars.begin(); it != Option::user_vars.end(); ++it)
	parse((*it));
    base_vars = vars;
}

bool
QMakeProject::parse(QString t)
{
    QString s = t.simplifyWhiteSpace();

    s.replace(QRegExp("#.*$"), ""); /* bye comments */
    if(s.isEmpty()) /* blank_line */
	return TRUE;

    int equals = s.find('=');
    if(equals == -1) {
	yyerror("Parse error");
	return FALSE;
    }

    QString scope, var, op;
    QStringList val;
#define SKIP_WS(d) while(*d && (*d == ' ' || *d == '\t')) d++;
    const char *d = s.latin1();
    SKIP_WS(d);
    while(*d && *d != '+' && *d != '-' && *d != '=') {
	if(*d == ':') {
	    scope = var.stripWhiteSpace();
	    var = "";

	    int lparen = scope.find('(');
	    if(lparen != -1) { /* if there is an lparen in the scope, it IS a function */
		int rparen = scope.find(')', lparen);
		if(rparen == -1) {
		    QCString error;
		    error.sprintf("Function missing right paren: %s", scope.latin1());
		    yyerror(error);
		    return FALSE;
		}
		QString func = scope.left(lparen);
		QStringList args = QStringList::split(',', scope.mid(lparen+1, rparen - lparen - 1));
		for(QStringList::Iterator arit = args.begin(); arit != args.end(); ++arit) 
		    (*arit) = (*arit).stripWhiteSpace(); /* blah, get rid of space */
		if(!doProjectTest(func, args)) {
		    if(Option::debug_level)
			printf("Project Parser: Test (%s) has failed.\n", scope.latin1());
		    return TRUE;
		}
	    }
	    else if(!isActiveConfig(scope.stripWhiteSpace())) {
		if(Option::debug_level)
		    printf("Project Parser: Scope (%s) is not in scope.\n", scope.latin1());
		return TRUE;
	    }
	}
	else var += *d;
	d++;
    }
    QStringList &varlist = vars[var.stripWhiteSpace()];

    if(!*d)
	return FALSE;
    SKIP_WS(d);
    for( ; *d && op.find('=') == -1; op += *(d++));
    op.replace(QRegExp("\\s"), "");

    SKIP_WS(d);
    val = QStringList::split(' ', d);
#undef SKIP_WS

    if(Option::debug_level)
	printf("Project Parser: %s %s (%s)\n", var.latin1(), op.latin1(), val.join(" :: ").latin1());

    if(op == "=")
	varlist.clear();
    for(QStringList::Iterator valit = val.begin(); valit != val.end(); ++valit) {
	int rep;
	while((rep = (*valit).find(QRegExp("\\$\\$[a-zA-Z0-9_-]*"))) != -1) {
	    QString torep = (*valit).mid(rep, (*valit).find(QRegExp("[^\\$a-zA-Z0-9_-]"), rep) - rep);
	    QStringList &l = vars[torep.right(torep.length()-2)];
	    if(Option::debug_level >= 2)
		printf("Project parser: (%s) :: %s -> %s\n", (*valit).latin1(), torep.latin1(), l.join(" ").latin1());
	    if(l.count() > 1) {
		(*valit).replace(rep, torep.length(), "");

		for(QStringList::Iterator lit = l.begin(); lit != l.end(); ++lit) {
		    if(op == "=" || op == "+=")
			varlist.append((*lit));
		    else if(op == "-=")
			varlist.remove((*lit));
		}
	    }
	    else
		(*valit).replace(rep, torep.length(), l.first());
	}
	if((*valit).isEmpty())
	    continue;

	if(op == "=" || op == "+=")
	    varlist.append((*valit));
	else if(op == "-=")
	    varlist.remove((*valit));
    }
    return TRUE;
}

bool
QMakeProject::read(const char *project)
{
    vars = base_vars; /* start with the base */
    
    /* parse project file */
    if(Option::debug_level)
	printf("Project file: reading %s\n", project);

    pfile = project;
    if(!QFile::exists(pfile) && pfile.right(4) != ".pro")
	pfile += ".pro";

    bool ret = FALSE;
    QFile qfile(pfile = project);
    if ( (ret = qfile.open(IO_ReadOnly)) ) {
	QString projname = project;
	projname.replace(QRegExp("\\.pro$"), "");
	vars["PROJECT"].append(projname);
	vars["TARGET"].append(projname);

	QTextStream t( &qfile );
	QString s;
	line_count = 0;
	while ( !t.eof() ) {
	    line_count++;
	    s += t.readLine();
	    if(s.right(1) == "\\")
		s.truncate(s.length() - 1);
	    else {
		if(!(ret = parse(s)))
		    break;
		s = "";
	    }
	}
	qfile.close();
    }
    return ret;
}

bool
QMakeProject::isActiveConfig(const QString &x)
{
    if(x.isEmpty()) 
	return TRUE;
    if(Option::mode == Option::UNIX_MODE && x == "unix")
	return TRUE;
    else if(Option::mode == Option::WIN_MODE && x == "win32")
	return TRUE;
    return vars["CONFIG"].findIndex(x) != -1;
}

bool
QMakeProject::doProjectTest(QString func, const QStringList &args)
{
    if(func == "system") {
	if(args.count() != 1) {
	    fprintf(stderr, "%d: runTest(exec) requires one argument.\n", line_count);
	    return FALSE;
	}
	return system(args.first().latin1()) == 0;
    } else if(func == "contains") {
	if(args.count() != 2) {
	    fprintf(stderr, "%d: contains(var, val) requires two argument.\n", line_count);
	    return FALSE;
	}
	return vars[args[0]].findIndex(args[1]) != -1;
	    
    } else {
	fprintf(stderr, "Unknown test function: %s\n", func.latin1());
    }
    return FALSE;
}
