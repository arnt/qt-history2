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
#include <qdir.h>
#include <qregexp.h>
#include <qtextstream.h>

int line_count;
extern "C" int yyerror(const char *);

QMakeProject::QMakeProject()
{
}

bool
QMakeProject::parse(QString file, QString t, QMap<QString, QStringList> &place)
{
    QString s = t.simplifyWhiteSpace();

    s.replace(QRegExp("#.*$"), ""); /* bye comments */
    if(s.isEmpty()) /* blank_line */
	return TRUE;

    if(s == "}") {
	if(Option::debug_level >= 1)
	    printf("Project Parser: %s:%d : Leaving block %d\n", file.latin1(), line_count, scope_block);
	scope_block--;
	return TRUE;
    }
    else if(!(scope_flag & (0x01 << scope_block))) {
	for(int i = (s.contains('{')-s.contains('}')); i; i--)
	    scope_flag &= ~(0x01 << (++scope_block));
	if(Option::debug_level >= 1)
	    printf("Project Parser: %s:%d : Ignored due to block being false.\n", file.latin1(), line_count);
	return TRUE;
    }

    QString scope, var, op;
    QStringList val;
#define SKIP_WS(d) while(*d && (*d == ' ' || *d == '\t')) d++
    const char *d = s.latin1();
    SKIP_WS(d);
    bool scope_failed = FALSE;
    while(*d && ((*d != '+' || *d != '-') && *(d+1) != '=') && *d != '=') {
	if(*d == ':' || *d == '{') {
	    scope = var.stripWhiteSpace();
	    var = "";

	    bool test = FALSE, invert_test = (scope.left(1) == "!");
	    if(invert_test)
		scope = scope.right(scope.length()-1);

	    int lparen = scope.find('(');
	    if(lparen != -1) { /* if there is an lparen in the scope, it IS a function */
		if(!scope_failed) {
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
		    test = doProjectTest(func, args, place);
		}
	    }
	    else test = isActiveConfig(scope.stripWhiteSpace());

	    if(invert_test)
		test = !test;
	    if(!test && !scope_failed) {
		if(Option::debug_level)
		    printf("Project Parser: %s:%d : Test (%s) failed.\n", file.latin1(), line_count,
			   scope.latin1());
		scope_failed = TRUE;
	    }
	    if(*d == '{') {/* scoping block */
		if(!scope_failed)
		    scope_flag |= (0x01 << (++scope_block));
		else
		    scope_flag &= ~(0x01 << (++scope_block));

		if(Option::debug_level >= 1)
		    printf("Project Parser: %s:%d : Entering block %d (%d).\n", file.latin1(), line_count,
			   scope_block, !scope_failed);
	    }
	}
	else var += *d;
	d++;
    }
    if(scope_failed)
	return TRUE; /* oh well */
    if(!*d)
	return var.isEmpty(); /* allow just a scope */

    SKIP_WS(d);
    for( ; *d && op.find('=') == -1; op += *(d++));
    op.replace(QRegExp("\\s"), "");

    SKIP_WS(d);
    QString vals(d); /* vals now contains the space separated list of values */
    if(vals.right(1) == "}") {
	scope_block--;
	vals.truncate(vals.length()-1);
    }

#define UN_TMAKEIFY(x) x.replace(QRegExp("^TMAKE"), "QMAKE")
    int rep, rep_len;
    QRegExp reg_var("\\$\\$[a-zA-Z0-9_-]*");
    while((rep = reg_var.match(vals, 0, &rep_len)) != -1) {
	QString rep_var = UN_TMAKEIFY(vals.mid(rep + 2, rep_len - 2));
	const QString &replacement = place[rep_var].join(" ");
	if(Option::debug_level >= 2)
	    printf("Project parser: (%s) :: %s -> %s\n", vals.latin1(),
		   vals.mid(rep, rep_len).latin1(), replacement.latin1());
	vals.replace(rep, rep_len, replacement);
    }
#undef SKIP_WS

    var = UN_TMAKEIFY(var.stripWhiteSpace()); //backwards compatability

    QStringList &varlist = place[var]; /* varlist is the list in the symbol table */
    QStringList vallist = QStringList::split(' ', vals);  /* vallist is the broken up list of values */
    if(Option::debug_level)
	printf("Project Parser: %s:%d :%s: :%s: (%s)\n", file.latin1(), line_count,
	       var.latin1(), op.latin1(), vallist.join(" :: ").latin1());

    /* now do the operation */
    if(op == "=")
	varlist.clear();
    for(QStringList::Iterator valit = vallist.begin(); valit != vallist.end(); ++valit) {
	if((*valit).isEmpty())
	    continue;

	if(op == "=" || op == "+=")
	    varlist.append((*valit));
	else if(op == "-=")
	    varlist.remove((*valit));
    }
    if(var == "REQUIRES") /* special case to get communicated to backends! */
	doProjectCheckReqs(vallist);

    return TRUE;
}

bool
QMakeProject::read(const char *file, QMap<QString, QStringList> &place)
{
    /* scope blocks start at true */
    scope_flag = 0x01;
    scope_block = 0;

    bool ret;
    QFile qfile(file);
    if ( (ret = qfile.open(IO_ReadOnly)) ) {
	QTextStream t( &qfile );
	QString s;
	line_count = 0;
	while ( !t.eof() ) {
	    line_count++;
	    s += t.readLine().stripWhiteSpace();
	    if(s.right(1) == "\\") {
		s.truncate(s.length() - 1);
		s += " ";
	    }
	    else {
		if(!(ret = parse(file, s, place)))
		    break;
		s = "";
	    }
	}
	qfile.close();
    }
    return ret;
}

bool
QMakeProject::read(const char *project)
{

    if(cfile.isEmpty()) {
	/* parse the cache */
	if(Option::do_cache) {
	    QString cachefile = Option::cachefile;
	    if(cachefile.find(QDir::separator()) == -1) {
		/* find the cache file, otherwise return false */
			QString dir = QDir::convertSeparators(QDir::currentDirPath());
		while(!QFile::exists((cachefile = dir + QDir::separator() + Option::cachefile))) {
		    dir = dir.left(dir.findRev(QDir::separator()));
			if(dir.isEmpty() || dir.find(QDir::separator()) == -1) {
			cachefile = "";
			break;
		    }
		}
	    }
	    if(!cachefile.isEmpty()) {
		if(Option::debug_level)
		    printf("MKSCACHE file: reading %s\n", cachefile.latin1());

		read(cachefile, cache);
		if(Option::specfile.isEmpty() && !cache["MKSPEC"].isEmpty())
		    Option::specfile = cache["MKSPEC"].first();
	    }
	}

	/* parse mkspec */
	if(Option::specfile.isNull() || Option::specfile.isEmpty()) {
	    if(!getenv("MKSPEC")) {
		fprintf(stderr, "MKSPEC has not been set, so mkspec cannot be deduced.\n");
		return FALSE;
	    }
	    Option::specfile = getenv("MKSPEC");
	}
	if(Option::specfile.find(QDir::separator()) == -1) {
	    if(!getenv("QTDIR")) {
		fprintf(stderr, "QTDIR has not been set, so mkspec cannot be deduced.\n");
		return FALSE;
	    }
	    Option::specfile.prepend(QString(getenv("QTDIR")) + QDir::separator() + "mkspecs" + QDir::separator());
	}

	if(Option::debug_level)
	    printf("MKSPEC file: reading %s\n", Option::specfile.latin1());

	if(!read(Option::specfile, base_vars)) {
	    fprintf(stderr, "Failure to read MKSPEC file.\n");
	    return FALSE;
	}
	cfile = project;
	for(QStringList::Iterator it = Option::user_vars.begin(); it != Option::user_vars.end(); ++it) {
	    if(!parse("(internal)", (*it), base_vars)) {
		fprintf(stderr, "Argument failed to parse: %s\n", (*it).latin1());
		return FALSE;
	    }
	}
    }

    /* parse project file */
    if(Option::debug_level)
	printf("Project file: reading %s\n", project);

    vars = base_vars; /* start with the base */
    vars["CONFIG"] += cache["CONFIG"];
    pfile = project;
    if(!QFile::exists(pfile) && pfile.right(4) != ".pro")
	pfile += ".pro";

    if(!read(pfile, vars))
	return FALSE;
    
    /* now let the user override the template from an option.. */
    if(!Option::user_template.isEmpty()) {
	if(Option::debug_level)
	    fprintf(stderr, "Overriding TEMPLATE (%s) with: %s\n", vars["TEMPLATE"].first().latin1(),
	    Option::user_template.latin1());
	vars["TEMPLATE"].first() = Option::user_template;
    }
    
    return TRUE;
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
    else if(Option::specfile.right(x.length()) == x)
	return TRUE;
    return ( vars["CONFIG"].findIndex(x) != -1 );
}

bool
QMakeProject::doProjectTest(QString func, const QStringList &args, QMap<QString, QStringList> &place)
{
    if(func == "system") {
	if(args.count() != 1) {
	    fprintf(stderr, "%d: system(exec) requires one argument.\n", line_count);
	    return FALSE;
	}
	return system(args.first().latin1()) == 0;
    } else if(func == "contains") {
	if(args.count() != 2) {
	    fprintf(stderr, "%d: contains(var, val) requires two argument.\n", line_count);
	    return FALSE;
	}
	return vars[args[0]].findIndex(args[1]) != -1;
    } else if(func == "include") {
	if(args.count() != 1) {
	    fprintf(stderr, "%d: include(file) requires one argument.\n", line_count);
	    return FALSE;
	}

	QString file = args.first();
	file.replace(QRegExp("\""), "");

	int rep, rep_len;
	QRegExp reg_var("\\$\\$[a-zA-Z0-9_-]*");
	while((rep = reg_var.match(file, 0, &rep_len)) != -1)
	    file.replace(rep, rep_len, place[file.mid(rep + 2, rep_len - 2)].join(" "));

	if(Option::debug_level)
	    printf("Project Parser: Including file %s.\n", file.latin1());
	int l = line_count;
	bool r = read(file.latin1(), place);
	line_count = l;
	return r;
    } else if(func == "error" || func == "message") {
	if(args.count() != 1) {
	    fprintf(stderr, "%d: %s(message) requires one argument.\n", func.latin1(), line_count);
	    return FALSE;
	}

	printf("Project %s: %s\n", func.upper().latin1(), args.first().latin1());
	if(func == "message")
	    return TRUE;
	exit(2);
    } else {
	fprintf(stderr, "Unknown test function: %s\n", func.latin1());
    }
    return FALSE;
}

void
QMakeProject::doProjectCheckReqs(const QStringList &deps)
{
    if(!Option::do_cache)
	return;

    QStringList &configs = cache["CONFIG"];
    for(QStringList::ConstIterator it = deps.begin(); it != deps.end(); ++it) {
	if((*it).isEmpty())
	    continue;

	QString dep = (*it);
	bool invert_test = (dep.left(1) == "!");
	if(invert_test)
	    dep = dep.right(dep.length() - 1);

	bool test = (configs.findIndex(dep) != -1);
	if(invert_test)
	    test = !test;
	if(!test) {
	    vars["QMAKE_FAILED_REQUIREMENTS"].append(dep);
	    return;
	}
    }
}
