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
#include <qfile.h>
#include <qdir.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef Q_OS_UNIX
# include <unistd.h>
#endif

int line_count;
extern "C" void yyerror(const char *);

static QString varMap(const QString &x)
{
    QString ret(x);
    ret.replace(QRegExp("^TMAKE"), "QMAKE");
    if(ret == "INTERFACES")
	ret = "FORMS";
    return ret;
}

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
    } else if(!(scope_flag & (0x01 << scope_block))) {
	/* adjust scope for each block which appears on a single line */
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
    int parens = 0;
    while(*d && *d != '=') {
	if((*d == '+' || *d == '-' || *d == '*' || *d == '~') && *(d+1) == '=')
	    break;

	if ( *d == '(' )
	    ++parens;
	else if ( *d == ')' )
	    --parens;

	if(*d == ':' || *d == '{' || ( *d == ')' && !parens )) {
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
		    int rparen = scope.findRev(')');
		    if(rparen == -1) {
			QCString error;
			error.sprintf("%s: Function missing right paren: %s", file.latin1(), scope.latin1());
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
	    } else {
		test = isActiveConfig(scope.stripWhiteSpace());
	    }

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
	} else {
	    var += *d;
	}
	d++;
    }
    if(scope_failed)
	return TRUE; /* oh well */
    if(!*d) {
	if(!var.isEmpty())
	    yyerror(file + ": Parse Error");
	return var.isEmpty(); /* allow just a scope */
    }

    SKIP_WS(d);
    for( ; *d && op.find('=') == -1; op += *(d++));
    op.replace(QRegExp("\\s"), "");

    SKIP_WS(d);
    QString vals(d); /* vals now contains the space separated list of values */
    if(vals.right(1) == "}") {
	debug_msg(1, "Project Parser: %s:%d : Leaving block %d", file.latin1(), line_count, scope_block);
	scope_block--;
	vals.truncate(vals.length()-1);
    }
    doVariableReplace(vals, place);

    var = var.stripWhiteSpace();
#undef SKIP_WS

    if(!var.isEmpty() && Option::mkfile::do_preprocess)
	debug_msg(0, "%s:%d :: %s %s %s",  file.latin1(), line_count, var.latin1(), op.latin1(), vals.latin1());
    var = varMap(var); //backwards compatability

    QStringList vallist;  /* vallist is the broken up list of values */
    if((var == "DEPENDPATH" || var == "INCLUDEPATH") && vals.find(';') != -1) { //these guys use ; for space reasons I guess
	QRegExp rp("([^;]*)[;$]");
	for(int x = 0; (x = rp.search(vals, 0)) != -1; ) {
	    vallist.append("\"" + rp.cap(1) + "\"");
	    vals.remove(x, rp.matchedLength());
	}
	if(!vals.isEmpty()) {
	    vallist.append("\"" + vals + "\"");
	    vals = "";
	}
    }
    if(vals.find('"') != -1) { //strip out quoted entities
	QRegExp quoted("( |^)(\"[^\"]*\")( |$)");
	for(int x = 0; (x = quoted.search(vals, 0)) != -1; ) {
	    vallist.append(quoted.cap(2));
	    vals.remove(x, quoted.matchedLength());
	}
    }

    //now split on space
    vallist += QStringList::split(' ', vals);
    if(!vallist.grep("=").isEmpty())
	warn_msg(WarnParser, "Detected possible line continuation: {%s} %s:%d",
		 var.latin1(), file.latin1(), line_count);

    QStringList &varlist = place[var]; /* varlist is the list in the symbol table */
    debug_msg(1, "Project Parser: %s:%d :%s: :%s: (%s)", file.latin1(), line_count,
	      var.latin1(), op.latin1(), vallist.join(" :: ").latin1());

    /* now do the operation */
    if(op == "~=") {
	if(vallist.count() != 1) {
	    yyerror(file + ": ~= operator only accepts one right hand paramater");
	    return FALSE;
	}
	QString val(vallist.first());
	if(val.length() < 4 || val.at(0) != 's') {
	    yyerror(file + ": ~= operator only can handle s/// function");
	    return FALSE;
	}
	QChar sep = val.at(1);
	QStringList func = QStringList::split(sep, val, TRUE);
	if(func.count() < 3 || func.count() > 4) {
	    yyerror(file + ": ~= operator only can handle s/// function");
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
	if(op == "=") {
	    if(!varlist.isEmpty())
		warn_msg(WarnParser, "Operator=(%s) clears variables previously set: %s:%d",
			 var.latin1(), file.latin1(), line_count);
	    varlist.clear();
	}
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
	doProjectCheckReqs(vallist, place);

    return TRUE;
}

bool
QMakeProject::read(QString file, QMap<QString, QStringList> &place)
{
    /* scope blocks start at true */
    scope_flag = 0x01;
    scope_block = 0;

    file = Option::fixPathToLocalOS(file);
    doVariableReplace(file, place);
    bool ret = FALSE, using_stdin = FALSE;
    QFile qfile;
    if(!strcmp(file, "-")) {
	qfile.setName("");
	ret = qfile.open(IO_ReadOnly, stdin);
	using_stdin = TRUE;
    } else {
	qfile.setName(file);
	ret = qfile.open(IO_ReadOnly);
    }
    if ( ret ) {
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
	if(!using_stdin)
	    qfile.close();
    }
    return ret;
}

bool
QMakeProject::read(QString project, QString pwd)
{
    if(cfile.isEmpty()) {
	// hack to get the Option stuff in there
	base_vars["QMAKE_EXT_CPP"] = Option::cpp_ext;
	base_vars["QMAKE_EXT_H"] = Option::h_ext;

	/* parse the cache */
	if(Option::mkfile::do_cache && !Option::mkfile::cachefile.isEmpty()) {
	    read(Option::mkfile::cachefile, cache);
	    if(Option::mkfile::qmakespec.isEmpty() && !cache["QMAKESPEC"].isEmpty())
		Option::mkfile::qmakespec = cache["QMAKESPEC"].first();
	}

	/* parse mkspec */
	if(Option::mkfile::qmakespec.isEmpty()) {
	    if(getenv("QTDIR") &&
	       !QFile::exists(Option::mkfile::qmakespec = QString(getenv("QTDIR")) +
			      QDir::separator() + QString("mkspecs") +
			      QDir::separator() + "default")) {
		fprintf(stderr, "QMAKESPEC has not been set, so configuration cannot be deduced.\n");
		return FALSE;
	    }
	}
	if(QDir::isRelativePath(Option::mkfile::qmakespec)) {
	    if(!getenv("QTDIR")) {
		fprintf(stderr, "QTDIR has not been set, so mkspec cannot be deduced.\n");
		return FALSE;
	    }
	    Option::mkfile::qmakespec.prepend(QString(getenv("QTDIR")) +
					      QDir::separator() + "mkspecs" + QDir::separator());
	}
	QString spec = Option::mkfile::qmakespec + QDir::separator() + "qmake.conf";
	debug_msg(1, "QMAKESPEC conf: reading %s", spec.latin1());
	if(!read(spec, base_vars)) {
	    fprintf(stderr, "Failure to read QMAKESPEC conf file %s.\n", spec.latin1());
	    return FALSE;
	}
	if(Option::mkfile::do_cache && !Option::mkfile::cachefile.isEmpty()) {
	    debug_msg(1, "QMAKECACHE file: reading %s", Option::mkfile::cachefile.latin1());
	    read(Option::mkfile::cachefile, base_vars);
	}

	/* commandline */
	cfile = project;
	for(QStringList::Iterator it = Option::before_user_vars.begin();
	    it != Option::before_user_vars.end(); ++it) {
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
    if(pfile != "-" && !QFile::exists(pfile) && pfile.right(4) != ".pro")
	pfile += ".pro";

    if(!read(pfile, vars))
	return FALSE;

    for(QStringList::Iterator it = Option::after_user_vars.begin();
	it != Option::after_user_vars.end(); ++it) {
	if(!parse("(internal after)", (*it), vars)) {
	    fprintf(stderr, "Argument failed to parse: %s\n", (*it).latin1());
	    return FALSE;
	}
    }

    /* now let the user override the template from an option.. */
    if(!Option::user_template.isEmpty()) {
	debug_msg(1, "Overriding TEMPLATE (%s) with: %s", vars["TEMPLATE"].first().latin1(), Option::user_template.latin1());
	vars["TEMPLATE"].clear();
	vars["TEMPLATE"].append(Option::user_template);
    }

    if(vars["TEMPLATE"].isEmpty())
	vars["TEMPLATE"].append(QString("app"));
    else
	vars["TEMPLATE"].first().replace(QRegExp("\\.t$"), "");

    if(vars["TARGET"].isEmpty()) {
	QFileInfo fi(pfile);
	QString tmp = pfile;
	if(tmp.findRev(Option::dir_sep) != -1)
	    pfile = pfile.right(pfile.length() - pfile.findRev(Option::dir_sep));
	if(tmp.findRev('.') != -1)
	    tmp = tmp.left(tmp.findRev('.'));
	vars["TARGET"].append(tmp);
    }

    QString test_version = getenv("QTESTVERSION");
    if (!test_version.isNull() && test_version != "") {
	QString s = vars["TARGET"].first();
	if (s == "qt" || s == "qt-mt" || s == "qte" || s == "qte-mt") {
	    QString &ver = vars["VERSION"].first();
//	    fprintf(stderr,"Current QT version number: " + ver + "\n");
	    if (ver != "" && ver != test_version) {
		ver = test_version;
		fprintf(stderr,"Changed QT version number to " + test_version + "!\n");
	    }
	}
    }
    return TRUE;
}

bool
QMakeProject::isActiveConfig(const QString &x)
{
    if(x.isEmpty())
	return TRUE;

    QRegExp re(x, FALSE, TRUE);
    if((Option::target_mode == Option::TARG_MACX_MODE || Option::target_mode == Option::TARG_UNIX_MODE) &&
       x == "unix")
	return TRUE;
    else if(Option::target_mode == Option::TARG_MACX_MODE && x == "macx")
	return TRUE;
    else if(Option::target_mode == Option::TARG_MAC9_MODE && x == "mac9")
	return TRUE;
    else if((Option::target_mode == Option::TARG_MAC9_MODE || Option::target_mode == Option::TARG_MACX_MODE) &&
	    x == "mac")
	return TRUE;
    else if(Option::target_mode == Option::TARG_WIN_MODE && x == "win32")
	return TRUE;


    QString spec = Option::mkfile::qmakespec.right(Option::mkfile::qmakespec.length() -
						   (Option::mkfile::qmakespec.findRev(QDir::separator())+1));
    if(re.exactMatch(spec))
	return TRUE;
#ifdef Q_OS_UNIX
    else if(spec == "default") {
	static char *buffer = NULL;
	if(!buffer)
	    buffer = (char *)malloc(1024);
	int l = readlink(Option::mkfile::qmakespec, buffer, 1024);
	if(l != -1) {
	    buffer[l] = '\0';
	    QString r = buffer;
	    if(r.findRev('/') != -1)
		r = r.mid(r.findRev('/') + 1);
	    if(re.exactMatch(r))
		return TRUE;
	}
    }
#endif


    QStringList &configs = vars["CONFIG"];
    for(QStringList::Iterator it = configs.begin(); it != configs.end(); ++it) {
	if(re.exactMatch((*it)))
	    return TRUE;
    }
    return FALSE;
}

bool
QMakeProject::doProjectTest(QString func, const QStringList &args, QMap<QString, QStringList> &place)
{
    if( func == "exists") {
	if(args.count() != 1) {
	    fprintf(stderr, "%d: exists(file) requires one argument.\n", line_count);
	    return FALSE;
	}
	QString file = args.first();
	file = Option::fixPathToLocalOS(file);
	file.replace(QRegExp("\""), "");
	doVariableReplace(file, place);

	if(QFile::exists(file))
	    return TRUE;
	//regular expression I guess
	QString dirstr = QDir::currentDirPath();
	int slsh = file.findRev(Option::dir_sep);
	if(slsh != -1) {
	    dirstr = file.left(slsh+1);
	    file = file.right(file.length() - slsh - 1);
	}
	QDir dir(dirstr, file);
	return dir.count() != 0;
    } else if(func == "system") {
	if(args.count() != 1) {
	    fprintf(stderr, "%d: system(exec) requires one argument.\n", line_count);
	    return FALSE;
	}
	return system(args.first().latin1()) == 0;
    } else if(func == "contains") {
	if(args.count() != 2) {
	    fprintf(stderr, "%d: contains(var, val) requires two arguments.\n", line_count);
	    return FALSE;
	}
	return vars[args[0]].findIndex(args[1]) != -1;
    } else if(func == "infile") {
	if(args.count() < 2 || args.count() > 3) {
	    fprintf(stderr, "%d: infile(file, var, val) requires at least 2 arguments.\n", line_count);
	    return FALSE;
	}
	QMakeProject proj;
	QString file = args[0];
	int di = file.findRev(Option::dir_sep);
	QDir sunworkshop42workaround = QDir::current();
	QString oldpwd = sunworkshop42workaround.currentDirPath();
	if(di != -1) {
	    if(!QDir::setCurrent(file.left(file.findRev(Option::dir_sep))))
		fprintf(stderr, "Cannot find directory: %s\n", file.left(di).latin1());
	    file = file.right(file.length() - di - 1);
	    return FALSE;
	}
	if(!proj.read(file, oldpwd)) {
	    fprintf(stderr, "Error processing project file: %s\n", file.latin1());
	    QDir::setCurrent(oldpwd);
	    return FALSE;
	}
	bool ret = FALSE;
	if(args.count() == 2)
	    ret = !proj.isEmpty(args[1]);
	else
	    ret = (proj.isEmpty(args[1]) ? FALSE : (proj.values(args[1]).findIndex(args[2]) != -1));
	QDir::setCurrent(oldpwd);
	return ret;
    } else if(func == "count") {
	if(args.count() != 2) {
	    fprintf(stderr, "%d: count(var, count) requires two arguments.\n", line_count);
	    return FALSE;
	}
	return vars[args[0]].count() == args[1].toUInt();
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
	file = Option::fixPathToLocalOS(file);
	file.replace(QRegExp("\""), "");
	doVariableReplace(file, place);
	debug_msg(1, "Project Parser: Including file %s.", file.latin1());
	int l = line_count;
	int sb = scope_block;
	int sf = scope_flag;
	bool r = read(file.latin1(), place);
	if(r)
	    vars["QMAKE_INTERNAL_INCLUDED_FILES"].append(file);
	line_count = l;
	scope_flag = sf;
	scope_block = sb;
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
QMakeProject::doProjectCheckReqs(const QStringList &deps, QMap<QString, QStringList> &place)
{
    for(QStringList::ConstIterator it = deps.begin(); it != deps.end(); ++it) {
	QString chk = (*it);
	if(chk.isEmpty())
	    continue;
	bool invert_test = (chk.left(1) == "!");
	if(invert_test)
	    chk = chk.right(chk.length() - 1);

	bool test;
	int lparen = chk.find('(');
	if(lparen != -1) { /* if there is an lparen in the chk, it IS a function */
	    int rparen = chk.findRev(')');
	    if(rparen == -1) {
		QCString error;
		error.sprintf("%s: Function (in REQUIRES) missing right paren: %s",
			      projectFile().latin1(), chk.latin1());
		yyerror(error);
	    } else {
		QString func = chk.left(lparen);
		QStringList args = QStringList::split(',', chk.mid(lparen+1, rparen - lparen - 1));
		for(QStringList::Iterator arit = args.begin(); arit != args.end(); ++arit)
		    (*arit) = (*arit).stripWhiteSpace(); /* blah, get rid of space */
		test = doProjectTest(func, args, place);
	    }
	} else {
	    test = isActiveConfig(chk);
	}
	if(invert_test) {
	    chk.prepend("!");
	    test = !test;
	}
	if(!test) {
	    debug_msg(1, "Project Parser: %s:%d Failed test: REQUIRES = %s", 
		      projectFile().latin1(), line_count, chk.latin1());
	    place["QMAKE_FAILED_REQUIREMENTS"].append(chk);
	}
    }
}


QString
QMakeProject::doVariableReplace(QString &str, const QMap<QString, QStringList> &place)
{
    int rep, rep_len;
    QRegExp reg_var;
    int left = 0, right = 0;
    for(int x = 0; x < 3; x++) {
	if( x == 0 ) { //just to block out {}'s
	    reg_var = QRegExp("\\$\\$\\{[a-zA-Z0-9_\\.-]*\\}");
	    left = 3;
	    right = 1;
	} else if(x == 1) { //environment
	    reg_var = QRegExp("\\$\\$\\([a-zA-Z0-9_\\.-]*\\)");
	    left = 3;
	    right = 1;
	} else if(x == 2) { //normal case
	    reg_var = QRegExp("\\$\\$[a-zA-Z0-9_\\.-]*");
	    left = 2;
	    right = 0;
	} 
	while((rep = reg_var.search(str)) != -1) {
	    rep_len = reg_var.matchedLength();
	    QString rep_var = str.mid(rep + left, rep_len - (left + right)), replacement;
	    if(rep_var == "LITERAL_WHITESPACE")
		replacement = "\t";
	    else if(x == 1) //environment
		replacement = getenv(rep_var);
	    else 
		replacement = place[varMap(rep_var)].join(" ");
	    debug_msg(2, "Project parser: (%s) :: %s -> %s", str.latin1(),
		      str.mid(rep, rep_len).latin1(), replacement.latin1());
	    str.replace(rep, rep_len, replacement);
	}
    }
    return str;
}
