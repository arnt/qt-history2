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
	debug_msg(1, "Project Parser: %s:%d : Leaving block %d", file.latin1(), line_count, scope_block);
	scope_block--;
	return TRUE;
    }
    else if(!(scope_flag & (0x01 << scope_block))) {
	for(int i = (s.contains('{')-s.contains('}')); i; i--)
	    scope_flag &= ~(0x01 << (++scope_block));
	debug_msg(1, "Project Parser: %s:%d : Ignored due to block being false.", file.latin1(), line_count);
	return TRUE;
    }

    QString scope, var, op;
    QStringList val;
#define SKIP_WS(d) while(*d && (*d == ' ' || *d == '\t')) d++
    const char *d = s.latin1();
    SKIP_WS(d);
    bool scope_failed = FALSE;
    while(*d && *d != '=') {
	if((*d == '+' || *d == '-' || *d == '*' || *d == '~') && *(d+1) == '=')
	    break;
	
	if(*d == ':' || *d == '{' || *d == ')' ) {
	    scope = var.stripWhiteSpace();
	    if ( *d == ')' )
		scope += *d; /* need this */
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
		    if ( *d == ')' && !*(d+1) ) 
			return TRUE;  /* assume we are done */
		}
	    }
	    else test = isActiveConfig(scope.stripWhiteSpace());

	    if(invert_test)
		test = !test;
	    if(!test && !scope_failed) {
		debug_msg(1, "Project Parser: %s:%d : Test (%s%s) failed.", file.latin1(), line_count,
			  invert_test ? "not " : "", scope.latin1());
		scope_failed = TRUE;
	    }
	    if(*d == '{') {/* scoping block */
		if(!scope_failed)
		    scope_flag |= (0x01 << (++scope_block));
		else
		    scope_flag &= ~(0x01 << (++scope_block));

		debug_msg(1, "Project Parser: %s:%d : Entering block %d (%d).", file.latin1(), line_count,
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
    QRegExp reg_var;
    int left, right;
    for(int x = 0; x < 2; x++) {
	if( x == 0 ) {
	    reg_var = QRegExp("\\$\\$\\{[a-zA-Z0-9_\\.-]*\\}");
	    left = 3;
	    right = 1;
	} else if(x == 1) {
	    reg_var = QRegExp("\\$\\$[a-zA-Z0-9_\\.-]*");
	    left = 2;
	    right = 0;
	}
	while((rep = reg_var.match(vals, 0, &rep_len)) != -1) {
	    QString rep_var = UN_TMAKEIFY(vals.mid(rep + left, rep_len - (left + right)));
	    const QString &replacement = place[rep_var].join(" ");
	    debug_msg(2, "Project parser: (%s) :: %s -> %s", vals.latin1(),
		   vals.mid(rep, rep_len).latin1(), replacement.latin1());
	    vals.replace(rep, rep_len, replacement);
	}
    }
#undef SKIP_WS

    var = UN_TMAKEIFY(var.stripWhiteSpace()); //backwards compatability

    QStringList vallist;  /* vallist is the broken up list of values */
    QRegExp quoted("[^\\\\](\"[^\"]*[^\\\\]\")");
    {
	for(int x = 0; (x = quoted.search(vals, x)) != -1; ) {
	vallist += QStringList::split(' ', vals.left(x));
	vallist.append(quoted.cap(1));
	vals.remove(0, x + quoted.matchedLength());
	}
    }
    vallist += QStringList::split(' ', vals);

    QStringList &varlist = place[var]; /* varlist is the list in the symbol table */
    debug_msg(1, "Project Parser: %s:%d :%s: :%s: (%s)", file.latin1(), line_count,
	      var.latin1(), op.latin1(), vallist.join(" :: ").latin1());

    /* now do the operation */
    if(op == "~=") {
	if(vallist.count() != 1) {
	    yyerror("~= operator only accepts one right hand paramater");
	    return FALSE;
	}
	QString val(vallist.first());
	if(val.length() < 4 || val.at(0) != 's') {
	    yyerror("~= operator only can handle s/// function");
	    return FALSE;
	}
	QChar sep = val.at(1);
	QStringList func = QStringList::split(sep, val, TRUE);
	if(func.count() < 3 || func.count() > 4) {
	    yyerror("~= operator only can handle s/// function");
	    return FALSE;
	}
	bool global = FALSE, case_sense = TRUE;
	if(func.count() == 4) {
	    global = func[3].find('g') != -1;
	    case_sense = func[3].find('i') == -1;
	}
	QRegExp regexp(func[1], case_sense);
	for(QStringList::Iterator varit = varlist.begin(); 
	    varit != varlist.end(); ++varit) {
	    if((*varit).contains(regexp)) {
		(*varit) = (*varit).replace(regexp, func[2]);
		if(!global)
		    break;
	    }
	}
    } else {
	if(op == "=")
	    varlist.clear();
	for(QStringList::Iterator valit = vallist.begin(); 
	    valit != vallist.end(); ++valit) {
	    if((*valit).isEmpty())
		continue;
	    if((op == "*=" && !(*varlist.find((*valit)))) || 
	       op == "=" || op == "+=")
		varlist.append((*valit));
	    else if(op == "-=")
		varlist.remove((*valit));
	}
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
	QString s, line;
	line_count = 0;
	while ( !t.eof() ) {
	    line_count++;
	    line = t.readLine().stripWhiteSpace();
	    line.replace(QRegExp("#.*$"), ""); /* bye comments */
	    if(line.isEmpty())
		continue;

	    if(line.right(1) == "\\") {
		line.truncate(line.length() - 1);
		s += line + " ";
	    } else {
		s += line;
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
QMakeProject::read(QString project, QString pwd)
{
    QString cachefile;
    if(cfile.isEmpty()) {
	/* parse the cache */
	if(Option::do_cache) {
	    cachefile = Option::cachefile;
	    if(cachefile.find(QDir::separator()) == -1) {
		/* find the cache file, otherwise return false */
		QString start_dir;
		if(pwd.isEmpty())
		    start_dir = QDir::currentDirPath();
		else
		    start_dir = pwd;
		    
		QString dir = QDir::convertSeparators(start_dir);
		QString ofile = Option::output.name();
		if(ofile.findRev(Option::dir_sep) != -1) {
		    dir = ofile.left(ofile.findRev(Option::dir_sep));
		    if(QDir::isRelativePath(dir))
			dir.prepend(QDir::convertSeparators(start_dir));
		}

		while(!QFile::exists((cachefile = dir + QDir::separator() + Option::cachefile))) {
		    dir = dir.left(dir.findRev(QDir::separator()));
			if(dir.isEmpty() || dir.find(QDir::separator()) == -1) {
			cachefile = "";
			break;
		    }
		}
	    }
	    if(!cachefile.isEmpty()) {
		read(cachefile, cache);
		if(Option::qmakepath.isEmpty() && !cache["QMAKEPATH"].isEmpty())
		    Option::qmakepath = cache["QMAKEPATH"].first();
	    }
	}

	/* parse mkspec */
	if(Option::qmakepath.isNull() || Option::qmakepath.isEmpty()) {
	    if(!getenv("QMAKEPATH")) {
		fprintf(stderr, "QMAKEPATH has not been set, so configuration cannot be deduced.\n");
		return FALSE;
	    }
	    Option::qmakepath = getenv("QMAKEPATH");
	}
	if(QDir::isRelativePath(Option::qmakepath)) {
	    if(!getenv("QTDIR")) {
		fprintf(stderr, "QTDIR has not been set, so mkspec cannot be deduced.\n");
		return FALSE;
	    }
	    Option::qmakepath.prepend(QString(getenv("QTDIR")) + QDir::separator() + "mkspecs" + QDir::separator());
	}
	QString spec = Option::qmakepath + QDir::separator() + "qmake.conf";
	debug_msg(1, "QMAKEPATH conf: reading %s", spec.latin1());

	if(!read(spec, base_vars)) {
	    fprintf(stderr, "Failure to read QMAKEPATH conf file %s.\n", spec.latin1());
	    return FALSE;
	}
	if(!cachefile.isEmpty()) {
	    debug_msg(1, "QMAKECACHE file: reading %s", cachefile.latin1());
	    read(cachefile, base_vars);
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
    debug_msg(1, "Project file: reading %s", project.latin1());
    vars = base_vars; /* start with the base */

    pfile = project;
    if(!QFile::exists(pfile) && pfile.right(4) != ".pro")
	pfile += ".pro";

    if(!read(pfile, vars))
	return FALSE;

    /* now let the user override the template from an option.. */
    if(!Option::user_template.isEmpty()) {
	debug_msg(1, "Overriding TEMPLATE (%s) with: %s", vars["TEMPLATE"].first().latin1(), Option::user_template.latin1());
	vars["TEMPLATE"].clear();
	vars["TEMPLATE"].append(Option::user_template);
    }

    if(Option::user_template.isEmpty())
	vars["TEMPLATE"].append("app");
    else
	vars["TEMPLATE"].first().replace(QRegExp("\\.t$"), "");

    if(vars["TARGET"].isEmpty()) {
	QFileInfo fi(project);
	vars["TARGET"].append(fi.fileName().replace(QRegExp("\\.pro$"), ""));
    }

    return TRUE;
}

bool
QMakeProject::isActiveConfig(const QString &x)
{
    if(x.isEmpty())
	return TRUE;
	
    if((Option::mode == Option::MACX_MODE || Option::Option::mode == Option::UNIX_MODE) && x == "unix")
	return TRUE;
    else if((Option::mode == Option::MAC9_MODE || Option::mode == Option::MACX_MODE) && x == "mac")
	return TRUE;
    else if(Option::mode == Option::WIN_MODE && x == "win32")
	return TRUE;
    else if(Option::qmakepath.right(x.length()) == x)
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
    } else if(func == "count") {
	if(args.count() != 2) {
	    fprintf(stderr, "%d: count(var, count) requires two argument.\n", line_count);
	    return FALSE;
	}
	return vars[args[0]].count() == args[1].toInt();
    } else if(func == "isEmpty") {
	if(args.count() != 1) {
	    fprintf(stderr, "%d: isEmpty(var) requires one argument.\n", line_count);
	    return FALSE;
	}
	return vars[args[0]].isEmpty();
    } else if(func == "include") {
	if(args.count() != 1) {
	    fprintf(stderr, "%d: include(file) requires one argument.\n", line_count);
	    return FALSE;
	}

	QString file = args.first();
	file.replace(QRegExp("\""), "");

	int rep, rep_len;
	QRegExp reg_var;
	int left, right;
	for(int x = 0; x < 2; x++) {
	    if( x == 0 ) {
		reg_var = QRegExp("\\$\\$\\{[a-zA-Z0-9_\\.-]*\\}");
		left = 3;
		right = 1;
	    } else if(x == 1) {
		reg_var = QRegExp("\\$\\$[a-zA-Z0-9_\\.-]*");
		left = 2;
		right = 0;
	    }
	    while((rep = reg_var.match(file, 0, &rep_len)) != -1)
		file.replace(rep, rep_len, place[file.mid(rep + left, rep_len - (left+right))].join(" "));
	}

	debug_msg(1, "Project Parser: Including file %s.", file.latin1());
	int l = line_count;
	bool r = read(file.latin1(), place);
	if(r)
	    vars["QMAKE_INTERNAL_INCLUDED_FILES"].append(file);
	line_count = l;
	return r;
    } else if(func == "error" || func == "message") {
	if(args.count() != 1) {
	    fprintf(stderr, "%d: %s(message) requires one argument.\n", line_count, func.latin1());
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

    QStringList &configs = vars["CONFIG"];
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
