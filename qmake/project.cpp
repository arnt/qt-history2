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
static void qmake_error_msg(const char *msg)
{
    fprintf(stderr, "%d: %s\n", line_count, msg);
}

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

    if(s.stripWhiteSpace().left(1) == "}") {
	debug_msg(1, "Project Parser: %s:%d : Leaving block %d", file.latin1(), 
		  line_count, scope_block);
	test_status = ((scope_flag & (0x01 << scope_block)) ? TestFound : TestSeek);
	scope_block--;
	s = s.mid(1).stripWhiteSpace();
	if(s.isEmpty())
	    return TRUE;
    } 
    if(!(scope_flag & (0x01 << scope_block))) {
	/* adjust scope for each block which appears on a single line */
	for(int i = (s.contains('{')-s.contains('}')); i; i--)
	    scope_flag &= ~(0x01 << (++scope_block));
	debug_msg(1, "Project Parser: %s:%d : Ignored due to block being false.", 
		  file.latin1(), line_count);
	return TRUE;
    }

    QString scope, var, op;
    QStringList val;
#define SKIP_WS(d) while(*d && (*d == ' ' || *d == '\t')) d++
    const char *d = s.latin1();
    SKIP_WS(d);
    bool scope_failed = FALSE, else_line = FALSE;
    int parens = 0, scope_count=0;
    while(*d && *d != '=') {
	if((*d == '+' || *d == '-' || *d == '*' || *d == '~')) {
	    if(*(d+1) == '=') {
		break;
	    } else if(*(d+1) == ' ') {
		const char *k = d + 1;
		SKIP_WS(k);
		if(*k == '=') {
		    QString msg;
		    qmake_error_msg(file + ": + *d + must be followed immediatly by =");
		    return FALSE;
		}
	    }
	}

	if ( *d == '(' )
	    ++parens;
	else if ( *d == ')' )
	    --parens;

	if(*d == ':' || *d == '{' || ( *d == ')' && !parens )) {
	    scope_count++;
	    scope = var.stripWhiteSpace();
	    if ( *d == ')' )
		scope += *d; /* need this */
	    var = "";

	    bool test = FALSE;
	    if(scope.lower() == "else" || scope == "|") {
		if(scope_count != 1 || test_status == TestNone) {
		    qmake_error_msg(file + ": Unexpected " + scope + " ('" + s + "')");
		    return FALSE;
		} 
		else_line = TRUE;
		test = (test_status == TestSeek);
		debug_msg(1, "Project Parser: %s:%d : Else%s %s.", file.latin1(), line_count, 
			  scope == "else" ? "" : QString(" (" + scope + ")").latin1(), 
			  test ? "considered" : "excluded");
	    } else {
		QString comp_scope = scope;
		bool invert_test = (comp_scope.left(1) == "!");
		if(invert_test)
		    comp_scope = comp_scope.right(comp_scope.length()-1);
		int lparen = comp_scope.find('(');
		if(lparen != -1) { /* if there is an lparen in the scope, it IS a function */
		    if(!scope_failed) {
			int rparen = comp_scope.findRev(')');
			if(rparen == -1) {
			    QCString error;
			    error.sprintf("%s: Function missing right paren: %s ('%s')", 
					  file.latin1(), comp_scope.latin1(), s.latin1());
			    qmake_error_msg(error);
			    return FALSE;
			}
			QString func = comp_scope.left(lparen);
			QStringList args = QStringList::split(',', 
							      comp_scope.mid(
								  lparen+1, rparen - lparen - 1));
			for(QStringList::Iterator arit = args.begin(); arit != args.end(); ++arit)
			    (*arit) = (*arit).stripWhiteSpace(); /* blah, get rid of space */
			test = doProjectTest(func, args, place);
			if ( *d == ')' && !*(d+1) ) {
			    if(invert_test)
				test = !test;
			    test_status = (test ? TestFound : TestSeek);
			    return TRUE;  /* assume we are done */
			}
		    }
		} else {
		    test = isActiveConfig(comp_scope.stripWhiteSpace());
		}
		if(invert_test)
		    test = !test;
	    }
	    if(!test && !scope_failed) {
		debug_msg(1, "Project Parser: %s:%d : Test (%s) failed.", file.latin1(), 
			  line_count, scope.latin1());
		scope_failed = TRUE;
	    }
	    if(*d == '{') { /* scoping block */
		if(!scope_failed)
		    scope_flag |= (0x01 << (++scope_block));
		else
		    scope_flag &= ~(0x01 << (++scope_block));
		debug_msg(1, "Project Parser: %s:%d : Entering block %d (%d).", file.latin1(), 
			  line_count, scope_block, !scope_failed);
	    }
	} else {
	    var += *d;
	}
	d++;
    }
    if(!scope_count || (scope_count == 1 && else_line)) 
	test_status = TestNone;
    else if(!else_line || test_status != TestFound)
	test_status = (scope_failed ? TestSeek : TestFound);
    if(scope_failed)
	return TRUE; /* oh well */
    if(!*d) {
	if(!var.isEmpty())
	    qmake_error_msg(file + ": Parse Error ('" + s + "')");
	return var.isEmpty(); /* allow just a scope */
    }

    SKIP_WS(d);
    for( ; *d && op.find('=') == -1; op += *(d++));
    op.replace(QRegExp("\\s"), "");

    SKIP_WS(d);
    QString vals(d); /* vals now contains the space separated list of values */
    if(vals.right(1) == "}") {
	debug_msg(1, "Project Parser: %s:%d : Leaving block %d", file.latin1(), 
		  line_count, scope_block);
	test_status = ((scope_flag & (0x01 << scope_block)) ? TestFound : TestSeek);
	scope_block--;
	vals.truncate(vals.length()-1);
    }
    doVariableReplace(vals, place);

    var = var.stripWhiteSpace();
#undef SKIP_WS

    if(!var.isEmpty() && Option::mkfile::do_preprocess) {
	static QString last_file("*none*");
	if(file != last_file) {
	    fprintf(stderr, "#file %s:%d\n", file.latin1(), line_count);
	    last_file = file;
	}
	fprintf(stderr, "%s %s %s\n", var.latin1(), op.latin1(), vals.latin1());
    }
    var = varMap(var); //backwards compatability

    QStringList vallist;  /* vallist is the broken up list of values */
    if((var == "DEPENDPATH" || var == "INCLUDEPATH") && vals.find(';') != -1) { 
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
	    qmake_error_msg(file + ": ~= operator only accepts one right hand paramater ('" + 
		s + "')");
	    return FALSE;
	}
	QString val(vallist.first());
	if(val.length() < 4 || val.at(0) != 's') {
	    qmake_error_msg(file + ": ~= operator only can handle s/// function ('" +
		s + "')");
	    return FALSE;
	}
	QChar sep = val.at(1);
	QStringList func = QStringList::split(sep, val, TRUE);
	if(func.count() < 3 || func.count() > 4) {
	    qmake_error_msg(file + ": ~= operator only can handle s/// function ('" + 
		s + "')");
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
    test_status = TestNone;
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
	    int prelen = line.length();
	    line.replace(QRegExp("#.*$"), ""); // bye comments
	    if(!line.isEmpty() && line.right(1) == "\\") {
		line.truncate(line.length() - 1);
		s += line + " ";
	    } else if(!line.isEmpty() || (line.isEmpty() && !prelen)) {
		if(s.isEmpty() && line.isEmpty())
		    continue;
		if(!line.isEmpty())
		    s += line;
		if(!s.isEmpty()) {
		    if(!(ret = parse(file, s, place)))
			break;
		    s = "";
		}
	    }
	}
	if(!using_stdin)
	    qfile.close();
    }
    return ret;
}

bool
QMakeProject::read(QString project, QString)
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
	line_count = 1; //really arg count now.. duh
	for(QStringList::Iterator it = Option::before_user_vars.begin();
	    it != Option::before_user_vars.end(); ++it) {
	    if(!parse("(internal)", (*it), base_vars)) {
		fprintf(stderr, "Argument failed to parse: %s\n", (*it).latin1());
		return FALSE;
	    }
	    line_count++;
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

    line_count = 1; //really arg count now.. duh
    for(QStringList::Iterator it = Option::after_user_vars.begin();
	it != Option::after_user_vars.end(); ++it) {
	if(!parse("(internal after)", (*it), vars)) {
	    fprintf(stderr, "Argument failed to parse: %s\n", (*it).latin1());
	    return FALSE;
	}
	line_count++;
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
    if (!test_version.isEmpty()) {
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
    if(func == "requires") {
	return doProjectCheckReqs(args, place);
    } else if(func == "exists") {
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
	TestStatus sc = test_status;
	bool r = read(file.latin1(), place);
	if(r)
	    vars["QMAKE_INTERNAL_INCLUDED_FILES"].append(file);
	line_count = l;
	test_status = sc;
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

bool
QMakeProject::doProjectCheckReqs(const QStringList &deps, QMap<QString, QStringList> &place)
{
    bool ret = FALSE;
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
		qmake_error_msg(error);
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
	    ret = FALSE;
	}
    }
    return ret;
}


QString
QMakeProject::doVariableReplace(QString &str, const QMap<QString, QStringList> &place)
{
    for(int x = 0, rep; x < 5; x++) {
	QRegExp reg_var;
	if( x == 0 ) //function blocked out by {}'s
	    reg_var = QRegExp("\\$\\$\\{([a-zA-Z0-9_]*)\\(([^)]*)\\)\\}");
	else if( x == 1 ) //variables blocked out by {}'s
	    reg_var = QRegExp("\\$\\$\\{([a-zA-Z0-9_\\.-]*)\\}");
	else if(x == 2) //environment
	    reg_var = QRegExp("\\$\\$\\(([a-zA-Z0-9_\\.-]*)\\)");
	else if(x == 3) //function
	    reg_var = QRegExp("\\$\\$([a-zA-Z0-9_]*)\\(([^)]*)\\)");
	else if(x == 4) //normal variable
	    reg_var = QRegExp("\\$\\$([a-zA-Z0-9_\\.-]*)");
	while((rep = reg_var.search(str)) != -1) {
	    QString replacement;
	    if(x == 2) {//environment
		replacement = getenv(reg_var.cap(1));
	    } else if(x == 0 || x == 3) { //function 
		QStringList args = QStringList::split(',', reg_var.cap(2));
		for(QStringList::Iterator arit = args.begin(); arit != args.end(); ++arit)
		    (*arit) = (*arit).stripWhiteSpace(); // blah, get rid of space
		if(reg_var.cap(1).lower() == "member") {
		    if(args.count() < 1 || args.count() > 2) {
			fprintf(stderr, "%d: member(var, place) requires two arguments.\n", 
				line_count);
		    } else {
			uint pos = 0;
			if(args.count() == 2)
			    pos = args[1].toInt();
			const QStringList &var = place[varMap(args.first())];
			if(var.count() >= pos) 
			    replacement = var[pos];
		    }
		} else if(reg_var.cap(1).lower() == "join") {
		    if(args.count() < 1 || args.count() > 4) {
			fprintf(stderr, "%d: join(var, glue, before, after) requires four"
				"arguments.\n", line_count);
		    } else {
			QString glue, before, after;
			if(args.count() >= 2)
			    glue = args[1].replace(QRegExp("\""), "");
			if(args.count() >= 3)
			    before = args[2].replace(QRegExp("\""), "");
			if(args.count() == 4)
			    after = args[3].replace(QRegExp("\""), "");
			const QStringList &var = place[varMap(args.first())];
			if(!var.isEmpty())
			    replacement = before + var.join(glue) + after;
		    }
		} else if(reg_var.cap(1).lower() == "find") {
		    if(args.count() != 2) {
			fprintf(stderr, "%d find(var, str) requires two arguments\n", 
				line_count);
		    } else {
			QRegExp regx = QRegExp(args[1]);
			const QStringList &var = place[varMap(args.first())];
			for(QStringList::ConstIterator vit = var.begin(); 
			    vit != var.end(); ++vit) {
			    if(regx.match(*vit) != -1) {
				if(!replacement.isEmpty())
				    replacement += " ";
				replacement += (*vit);
			    }
			}
		    }
		} else {
		    fprintf(stderr, "Unknown replace function: %s\n", 
			    reg_var.cap(1).latin1());		    
		}
	    } else { //variable
		if(reg_var.cap(1) == "LITERAL_WHITESPACE") 
		    replacement = "\t";
		else
		    replacement = place[varMap(reg_var.cap(1))].join(" ");
	    }
	    debug_msg(2, "Project parser: %d (%s) :: %s -> %s", x, str.latin1(),
		   reg_var.cap(0).latin1(), replacement.latin1());
	    str.replace(rep, reg_var.matchedLength(), replacement);
	}
    }
    return str;
}
