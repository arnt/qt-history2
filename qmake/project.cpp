/****************************************************************************
**
** Implementation of QMakeProject class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of qmake.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "project.h"
#include "property.h"
#include "option.h"
#include <qfile.h>
#include <qdir.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qvaluestack.h>
#include <qcstring.h>
#include <qhash.h>
#ifdef Q_OS_UNIX
# include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#ifdef Q_OS_WIN32
#define QT_POPEN _popen
#else
#define QT_POPEN popen
#endif

struct parser_info {
    QString file;
    int line_no;
    bool from_file;
} parser;

static void qmake_error_msg(const char *msg)
{
    fprintf(stderr, "%s:%d: %s\n", parser.file.latin1(), parser.line_no, msg);
}

QStringList qmake_mkspec_paths()
{
    QStringList ret;
    const QString concat = QDir::separator() + QString("mkspecs");
    if(const char *qmakepath = getenv("QMAKEPATH")) {
#ifdef Q_OS_WIN
	QStringList lst = QStringList::split(';', qmakepath);
	for(QStringList::Iterator it = lst.begin(); it != lst.end(); ++it) {
	    QStringList lst2 = QStringList::split(':', (*it));
	    for(QStringList::Iterator it2 = lst2.begin(); it2 != lst2.end(); ++it2)
		ret << ((*it2) + concat);
	}
#else
	QStringList lst = QStringList::split(':', qmakepath);
	for(QStringList::Iterator it = lst.begin(); it != lst.end(); ++it)
	    ret << ((*it) + concat);
#endif
    }
    if(const char *qtdir = getenv("QTDIR"))
	ret << (QString(qtdir) + concat);
#ifdef QT_INSTALL_PREFIX
    ret << (QT_INSTALL_PREFIX + concat);
#endif
#if defined(HAVE_QCONFIG_CPP)
    ret << (qInstallPath() + concat);
#endif
#ifdef QT_INSTALL_DATA
    ret << (QT_INSTALL_DATA + concat);
#endif
#if defined(HAVE_QCONFIG_CPP)
    ret << (qInstallPathData() + concat);
#endif

    /* prefer $QTDIR if it is set */
    if (getenv("QTDIR"))
	ret << getenv("QTDIR");
    ret << qInstallPathData();
    return ret;
}

static QString varMap(const QString &x)
{
    QString ret(x);
    if(ret.startsWith("TMAKE")) //tmake no more!
	ret = "QMAKE" + ret.mid(5);
    else if(ret == "INTERFACES")
	ret = "FORMS";
    else if(ret == "QMAKE_POST_BUILD")
	ret = "QMAKE_POST_LINK";
    else if(ret == "TARGETDEPS")
	ret = "POST_TARGETDEPS";
    else if(ret == "LIBPATH")
	ret = "QMAKE_LIBDIR";
    else if(ret == "QMAKE_EXT_MOC")
	ret = "QMAKE_EXT_CPP_MOC";
    else if(ret == "QMAKE_MOD_MOC")
	ret = "QMAKE_H_MOD_MOC";
    else if(ret == "QMAKE_LFLAGS_SHAPP")
	ret = "QMAKE_LFLAGS_APP";
    return ret;
}

static QStringList split_arg_list(const QString &params)
{
    QChar quote = 0;
    QStringList args;
    for(int x = 0, last = 0, parens = 0; x <= (int)params.length(); x++) {
	if(x == (int)params.length()) {
	    QString mid = params.mid(last, x - last).stripWhiteSpace();
	    if(quote.unicode()) {
		if(mid[(int)mid.length()-1] == quote)
		    mid = mid.left(mid.length()-1);
		if(!last && mid[0] == quote)
		    mid = mid.mid(1);
	    }
	    args << mid;
	} else if(params[x] == ')') {
	    parens--;
	} else if(params[x] == '(') {
	    parens++;
	} else if(params[x] == quote) {
	    quote = 0;
	} else if(params[x] == '\'' || params[x] == '"') {
	    quote = params[x];
	} else if(!parens && !quote.unicode() && params[x] == ',') {
	    args << params.mid(last, x - last).stripWhiteSpace();
	    last = x+1;
	} 
    }
    return args;
}

static QStringList split_value_list(const QString &vals, bool do_semicolon=FALSE)
{
    QString build;
    QStringList ret;
    QValueStack<QChar> quote;
    for(int x = 0; x < (int)vals.length(); x++) {
#if 0
	//I don't think I can do this, because you could have a trailing slash before your next
	//argument (ie a directory) which would cause problems. I will need to test more, but I'm
	//a bit nervous to change it now..
	if(x != (int)vals.length()-1 && vals[x] == '\\' && (vals[x+1] == ' '  || (do_semicolon && vals[x+1] == ';')))
	    build += QString(vals[x++]) + QString(vals[x++]);
	else
#endif
	if(x != (int)vals.length()-1 && vals[x] == '\\' && (vals[x+1] == '\'' || vals[x+1] == '"'))
	    build += vals[x++]; //get that 'escape'
	else if(!quote.isEmpty() && vals[x] == quote.top())
	    quote.pop();
	else if(vals[x] == '\'' || vals[x] == '"')
	    quote.push(vals[x]);

	if(quote.isEmpty() && ((do_semicolon && vals[x] == ';') ||  vals[x] == ' ')) {
	    ret << build;
	    build = "";
	} else {
	    build += vals[x];
	}
    }
    if(!build.isEmpty())
	ret << build;
    return ret;
}

QMakeProject::QMakeProject()
{
    prop = NULL;
}

QMakeProject::QMakeProject(QMakeProperty *p)
{
    prop = p;
}


bool
QMakeProject::parse(const QString &t, QMap<QString, QStringList> &place)
{
    QString s = t.simplifyWhiteSpace();
    int hash_mark = s.find("#");
    if(hash_mark != -1) //good bye comments
	s = s.left(hash_mark);
    if(s.isEmpty()) /* blank_line */
	return TRUE;

    if(scope_blocks.top().ignore) {
	bool continue_parsing = FALSE;
	/* adjust scope for each block which appears on a single line */
	for(int i = 0; i < s.length(); i++) {
	    if(s[i] == '{') {
		scope_blocks.push(ScopeBlock(TRUE));		
	    } else if(s[i] == '}') {
		scope_blocks.pop();
		if(!scope_blocks.top().ignore) {
		    debug_msg(1, "Project Parser: %s:%d : Leaving block %d", parser.file.latin1(),
			      parser.line_no, scope_blocks.count()+1);
		    s = s.mid(i+1).stripWhiteSpace();
		    continue_parsing = !s.isEmpty();
		    break;
		}
	    }
	}
	if(!continue_parsing) {
	    debug_msg(1, "Project Parser: %s:%d : Ignored due to block being false.",
		      parser.file.latin1(), parser.line_no);
	    return TRUE;
	}
    }

    QString scope, var, op;
    QStringList val;
#define SKIP_WS(d) while(*d && (*d == ' ' || *d == '\t')) d++
    const char *d = s.latin1();
    SKIP_WS(d);
    bool scope_failed = FALSE, else_line = FALSE, or_op=FALSE, start_scope=FALSE;
    int parens = 0, scope_count=0;
    while(*d) {
	if(!parens) {
	    if(*d == '=')
		break;
	    if(*d == '+' || *d == '-' || *d == '*' || *d == '~') {
		if(*(d+1) == '=') {
		    break;
		} else if(*(d+1) == ' ') {
		    const char *k = d + 1;
		    SKIP_WS(k);
		    if(*k == '=') {
			QString msg;
			qmake_error_msg(*d + "must be followed immediately by =");
			return FALSE;
		    }
		}
	    }
	}

	if ( *d == '(' )
	    ++parens;
	else if ( *d == ')' )
	    --parens;

	if(!parens && (*d == ':' || *d == '{' || *d == ')' || *d == '|')) {
	    scope_count++;
	    scope = var.stripWhiteSpace();
	    if ( *d == ')' )
		scope += *d; /* need this */
	    var = "";

	    bool test = scope_failed;
	    if(scope.lower() == "else") {
		if(scope_count != 1 || scope_blocks.top().else_status == ScopeBlock::TestNone) {
		    qmake_error_msg("Unexpected " + scope + " ('" + s + "')");
		    return FALSE;
		}
		else_line = TRUE;
		test = (scope_blocks.top().else_status == ScopeBlock::TestSeek);
		debug_msg(1, "Project Parser: %s:%d : Else%s %s.", parser.file.latin1(), parser.line_no,
			  scope == "else" ? "" : QString(" (" + scope + ")").latin1(),
			  test ? "considered" : "excluded");
	    } else {
		QString comp_scope = scope;
		bool invert_test = (comp_scope.left(1) == "!");
		if(invert_test)
		    comp_scope = comp_scope.right(comp_scope.length()-1);
		int lparen = comp_scope.find('(');
		if(or_op || !scope_failed) {
		    if(lparen != -1) { /* if there is an lparen in the scope, it IS a function */
			int rparen = comp_scope.findRev(')');
			if(rparen == -1) {
			    QCString error;
			    error.sprintf("Function missing right paren: %s ('%s')",
					  comp_scope.latin1(), s.latin1());
			    qmake_error_msg(error);
			    return FALSE;
			}
			QString func = comp_scope.left(lparen);
			test = doProjectTest(func, comp_scope.mid(lparen+1, rparen - lparen - 1), place);
			if ( *d == ')' && !*(d+1) ) {
			    if(invert_test)
				test = !test;
			    scope_blocks.top().else_status = 
				(test ? ScopeBlock::TestFound : ScopeBlock::TestSeek);
			    return TRUE;  /* assume we are done */
			}
		    } else {
			test = isActiveConfig(comp_scope.stripWhiteSpace(), TRUE, &place);
		    }
		    if(invert_test)
			test = !test;
		}
	    }
	    if(!test && !scope_failed)
		debug_msg(1, "Project Parser: %s:%d : Test (%s) failed.", parser.file.latin1(),
			  parser.line_no, scope.latin1());
	    if(test == or_op)
		scope_failed = !test;
	    or_op = (*d == '|');

	    if(*d == '{') /* scoping block */
		start_scope = TRUE;
	} else if(!parens && *d == '}') {
	    if(!scope_blocks.count()) {
		warn_msg(WarnParser, "Possible braces mismatch %s:%d", parser.file.latin1(), parser.line_no);
	    } else {
		debug_msg(1, "Project Parser: %s:%d : Leaving block %d", parser.file.latin1(),
			  parser.line_no, scope_blocks.count());
		scope_blocks.pop();
	    }
	} else {
	    var += *d;
	}
	d++;
    }
    var = var.stripWhiteSpace();

    if(!else_line || (else_line && !scope_failed)) 
	scope_blocks.top().else_status = (!scope_failed ? ScopeBlock::TestFound : ScopeBlock::TestSeek);
    if(start_scope) {
	ScopeBlock next_scope(scope_failed);
	next_scope.else_status = (scope_failed ? ScopeBlock::TestSeek : ScopeBlock::TestFound);
	scope_blocks.push(ScopeBlock(scope_failed));
	debug_msg(1, "Project Parser: %s:%d : Entering block %d (%d).", parser.file.latin1(),
		  parser.line_no, scope_blocks.count(), scope_failed);
    }
    if((!scope_count && !var.isEmpty()) || (scope_count == 1 && else_line)) 
	scope_blocks.top().else_status = ScopeBlock::TestNone;
    if(scope_failed)
	return TRUE; /* oh well */
    if(!*d) {
	if(!var.stripWhiteSpace().isEmpty())
	    qmake_error_msg("Parse Error ('" + s + "')");
	return var.isEmpty(); /* allow just a scope */
    }

    SKIP_WS(d);
    for( ; *d && op.find('=') == -1; op += *(d++))
	;
    op.replace(QRegExp("\\s"), "");

    SKIP_WS(d);
    QString vals(d); /* vals now contains the space separated list of values */
    int rbraces = vals.count('}'), lbraces = vals.count('{');
    if(scope_blocks.count() > 1 && rbraces - lbraces == 1) {
	debug_msg(1, "Project Parser: %s:%d : Leaving block %d", parser.file.latin1(),
		  parser.line_no, scope_blocks.count());
	scope_blocks.pop();
	vals.truncate(vals.length()-1);
    } else if(rbraces != lbraces) {
	warn_msg(WarnParser, "Possible braces mismatch {%s} %s:%d",
		 vals.latin1(), parser.file.latin1(), parser.line_no);
    }
    doVariableReplace(vals, place);

#undef SKIP_WS

    if(!var.isEmpty() && Option::mkfile::do_preprocess) {
	static QString last_file("*none*");
	if(parser.file != last_file) {
	    fprintf(stdout, "#file %s:%d\n", parser.file.latin1(), parser.line_no);
	    last_file = parser.file;
	}
	fprintf(stdout, "%s %s %s\n", var.latin1(), op.latin1(), vals.latin1());
    }
    var = varMap(var); //backwards compatability

    /* vallist is the broken up list of values */
    QStringList vallist = split_value_list(vals, (var == "DEPENDPATH" || var == "INCLUDEPATH"));
    if(!vallist.grep("=").isEmpty())
	warn_msg(WarnParser, "Detected possible line continuation: {%s} %s:%d",
		 var.latin1(), parser.file.latin1(), parser.line_no);

    QStringList &varlist = place[var]; /* varlist is the list in the symbol table */
    debug_msg(1, "Project Parser: %s:%d :%s: :%s: (%s)", parser.file.latin1(), parser.line_no,
	var.latin1(), op.latin1(), vallist.isEmpty() ? "" : vallist.join(" :: ").latin1());

    /* now do the operation */
    if(op == "~=") {
	if(vallist.count() != 1) {
	    qmake_error_msg("~= operator only accepts one right hand paramater ('" +
		s + "')");
	    return FALSE;
	}
	QString val(vallist.first());
	if(val.length() < 4 || val.at(0) != 's') {
	    qmake_error_msg("~= operator only can handle s/// function ('" +
		s + "')");
	    return FALSE;
	}
	QChar sep = val.at(1);
	QStringList func = QStringList::split(sep, val, TRUE);
	if(func.count() < 3 || func.count() > 4) {
	    qmake_error_msg("~= operator only can handle s/// function ('" +
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
			 var.latin1(), parser.file.latin1(), parser.line_no);
	    varlist.clear();
	}
	for(QStringList::Iterator valit = vallist.begin();
	    valit != vallist.end(); ++valit) {
	    if((*valit).isEmpty())
		continue;
	    if((op == "*=" && !varlist.contains((*valit))) ||
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
QMakeProject::read(QTextStream &file, QMap<QString, QStringList> &place)
{
    bool ret = TRUE;
    QString s, line;
    while ( !file.eof() ) {
	parser.line_no++;
	line = file.readLine().stripWhiteSpace();
	int prelen = line.length();

	int hash_mark = line.find("#");
	if(hash_mark != -1) //good bye comments
	    line = line.left(hash_mark).stripWhiteSpace();
	if(!line.isEmpty() && line.right(1) == "\\") {
	    if (!line.startsWith("#")) {
		line.truncate(line.length() - 1);
		s += line + " ";
	    }
	} else if(!line.isEmpty() || (line.isEmpty() && !prelen)) {
	    if(s.isEmpty() && line.isEmpty())
		continue;
	    if(!line.isEmpty())
		s += line;
	    if(!s.isEmpty()) {
		if(!(ret = parse(s, place)))
		    break;
		s = "";
	    }
	}
    }
    return ret;
}

bool
QMakeProject::read(const QString &file, QMap<QString, QStringList> &place)
{
    QString filename = Option::fixPathToLocalOS(file);
    doVariableReplace(filename, place);
    bool ret = FALSE, using_stdin = FALSE;
    QFile qfile;
    if(!strcmp(filename, "-")) {
	qfile.setName("");
	ret = qfile.open(IO_ReadOnly, stdin);
	using_stdin = TRUE;
    } else {
	qfile.setName(filename);
	ret = qfile.open(IO_ReadOnly);
    }
    if ( ret ) {
	parser_info pi = parser;
	parser.from_file = TRUE;
	parser.file = filename;
	parser.line_no = 0;
	/* scope_blocks starts with one non-ignoring entity */
	scope_blocks.clear();
	scope_blocks.push(ScopeBlock());
	QTextStream t( &qfile );
	ret = read(t, place);
	if(!using_stdin)
	    qfile.close();
	parser = pi;
    }
    return ret;
}

bool
QMakeProject::read(const QString &project, const QString &, uchar cmd)
{
    pfile = project;
    return read(cmd);
}

bool
QMakeProject::read(uchar cmd)
{
    if(cfile.isEmpty()) {
	// hack to get the Option stuff in there
	base_vars["QMAKE_EXT_CPP"] = Option::cpp_ext;
	base_vars["QMAKE_EXT_H"] = Option::h_ext;
	if(!Option::user_template_prefix.isEmpty())
	    base_vars["TEMPLATE_PREFIX"] = Option::user_template_prefix;

	if(cmd & ReadCache && Option::mkfile::do_cache) {	/* parse the cache */
	    if(Option::mkfile::cachefile.isEmpty())  { //find it as it has not been specified
		QString dir = QDir::convertSeparators(Option::output_dir);
		while(!QFile::exists((Option::mkfile::cachefile = dir +
				      QDir::separator() + ".qmake.cache"))) {
		    dir = dir.left(dir.findRev(QDir::separator()));
		    if(dir.isEmpty() || dir.find(QDir::separator()) == -1) {
			Option::mkfile::cachefile = "";
			break;
		    }
		    if(Option::mkfile::cachefile_depth == -1)
			Option::mkfile::cachefile_depth = 1;
		    else
			Option::mkfile::cachefile_depth++;
		}
	    }
	    if(!Option::mkfile::cachefile.isEmpty()) {
		read(Option::mkfile::cachefile, cache);
		if(Option::mkfile::qmakespec.isEmpty() && !cache["QMAKESPEC"].isEmpty())
		    Option::mkfile::qmakespec = cache["QMAKESPEC"].first();
	    }
	}
	if(cmd & ReadConf) { 	    /* parse mkspec */
	    QStringList mkspec_roots = qmake_mkspec_paths();
	    if(Option::mkfile::qmakespec.isEmpty()) {
		for(QStringList::Iterator it = mkspec_roots.begin(); it != mkspec_roots.end(); ++it) {
		    QString mkspec = (*it) + QDir::separator() + "default";
		    if(QFile::exists(mkspec)) {
			Option::mkfile::qmakespec = mkspec;
			break;
		    }
		}
		if(Option::mkfile::qmakespec.isEmpty()) {
		    fprintf(stderr, "QMAKESPEC has not been set, so configuration cannot be deduced.\n");
		    return FALSE;
		}
	    }

	    if(QDir::isRelativePath(Option::mkfile::qmakespec)) {
		bool found_mkspec = FALSE;
		for(QStringList::Iterator it = mkspec_roots.begin(); it != mkspec_roots.end(); ++it) {
		    QString mkspec = (*it) + QDir::separator() + Option::mkfile::qmakespec;
		    if(QFile::exists(mkspec)) {
			found_mkspec = TRUE;
			Option::mkfile::qmakespec = mkspec;
			break;
		    }
		}
		if(!found_mkspec) {
		    fprintf(stderr, "Could not find mkspecs for your QMAKESPEC after trying:\n\t%s\n",
			    mkspec_roots.join("\n\t").latin1());
		    return FALSE;
		}
	    }

	    /* parse qmake configuration */
	    while(Option::mkfile::qmakespec.endsWith(QString(QChar(QDir::separator()))))
		Option::mkfile::qmakespec.truncate(Option::mkfile::qmakespec.length()-1);
	    QString spec = Option::mkfile::qmakespec + QDir::separator() + "qmake.conf";
	    if(!QFile::exists(spec) &&
	       QFile::exists(Option::mkfile::qmakespec + QDir::separator() + "tmake.conf"))
		spec = Option::mkfile::qmakespec + QDir::separator() + "tmake.conf";
	    debug_msg(1, "QMAKESPEC conf: reading %s", spec.latin1());
	    if(!read(spec, base_vars)) {
		fprintf(stderr, "Failure to read QMAKESPEC conf file %s.\n", spec.latin1());
		return FALSE;
	    }
	    if(Option::mkfile::do_cache && !Option::mkfile::cachefile.isEmpty()) {
		debug_msg(1, "QMAKECACHE file: reading %s", Option::mkfile::cachefile.latin1());
		read(Option::mkfile::cachefile, base_vars);
	    }
	}
	if(cmd & ReadCmdLine) {
	    /* commandline */
	    cfile = pfile;
	    parser.file = "(internal)";
	    parser.from_file = FALSE;
	    parser.line_no = 1; //really arg count now.. duh
	    for(QStringList::Iterator it = Option::before_user_vars.begin();
		it != Option::before_user_vars.end(); ++it) {
		if(!parse((*it), base_vars)) {
		    fprintf(stderr, "Argument failed to parse: %s\n", (*it).latin1());
		    return FALSE;
		}
		parser.line_no++;
	    }
	}
    }

    vars = base_vars; /* start with the base */

    if(cmd & ReadProFile) { /* parse project file */
	debug_msg(1, "Project file: reading %s", pfile.latin1());
	if(pfile != "-" && !QFile::exists(pfile) && !pfile.endsWith(".pro"))
	    pfile += ".pro";
	if(!read(pfile, vars))
	    return FALSE;
    }

    if(cmd & ReadPostFiles) { /* parse post files */
	const QStringList l = vars["QMAKE_POST_INCLUDE_FILES"];
	for(QStringList::ConstIterator it = l.begin(); it != l.end(); ++it)
	    read((*it), vars);
    }

    if(cmd & ReadCmdLine) {
	parser.file = "(internal)";
	parser.from_file = FALSE;
	parser.line_no = 1; //really arg count now.. duh
	for(QStringList::Iterator it = Option::after_user_vars.begin();
	    it != Option::after_user_vars.end(); ++it) {
	    if(!parse((*it), vars)) {
		fprintf(stderr, "Argument failed to parse: %s\n", (*it).latin1());
		return FALSE;
	    }
	    parser.line_no++;
	}
    }

    if(cmd & ReadFeatures) {
	QStringList configs = vars["CONFIG"], processed = configs;
	while(1) {
	    debug_msg(1, "Processing CONFIG features");
	    for(QStringList::ConstIterator it = configs.begin(); it != configs.end(); ++it) 
		doProjectInclude((*it), TRUE, vars);
	    /* Process the CONFIG again to see if anything has been added (presumably by the feature files)
	       We cannot handle them being removed, but we want to process those that are added! */
	    configs.clear();
	    QStringList new_configs = vars["CONFIG"];
	    for(QStringList::ConstIterator it = new_configs.begin(); it != new_configs.end(); ++it) {
		if(processed.findIndex((*it)) == -1) {
		    configs.append((*it));
		    processed.append((*it));
		}
	    }
	    if(configs.isEmpty())
		break;
	}
	doProjectInclude("feature_default", TRUE, vars); //final to do the last setup
    }

    /* now let the user override the template from an option.. */
    if(!Option::user_template.isEmpty()) {
	debug_msg(1, "Overriding TEMPLATE (%s) with: %s", vars["TEMPLATE"].first().latin1(),
		  Option::user_template.latin1());
	vars["TEMPLATE"].clear();
	vars["TEMPLATE"].append(Option::user_template);
    }

    QStringList &templ = vars["TEMPLATE"];
    if(templ.isEmpty())
	templ.append(QString("app"));
    else if(vars["TEMPLATE"].first().endsWith(".t"))
	templ = QStringList(templ.first().left(templ.first().length() - 2));
    if ( !Option::user_template_prefix.isEmpty() )
	templ.first().prepend(Option::user_template_prefix);

    if(vars["TARGET"].isEmpty()) {
	// ### why not simply use:
	// QFileInfo fi(pfile);
	// fi.baseName();
	QString tmp = pfile;
	if(tmp.findRev('/') != -1)
	    tmp = tmp.right( tmp.length() - tmp.findRev('/') - 1 );
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
QMakeProject::isActiveConfig(const QString &x, bool regex, QMap<QString, QStringList> *place)
{
    if(x.isEmpty())
	return TRUE;

    if((Option::target_mode == Option::TARG_MACX_MODE || Option::target_mode == Option::TARG_QNX6_MODE ||
	Option::target_mode == Option::TARG_UNIX_MODE) && x == "unix")
	return TRUE;
    else if(Option::target_mode == Option::TARG_MACX_MODE && x == "macx")
	return TRUE;
    else if(Option::target_mode == Option::TARG_QNX6_MODE && x == "qnx6")
	return TRUE;
    else if(Option::target_mode == Option::TARG_MAC9_MODE && x == "mac9")
	return TRUE;
    else if((Option::target_mode == Option::TARG_MAC9_MODE || Option::target_mode == Option::TARG_MACX_MODE) &&
	    x == "mac")
	return TRUE;
    else if(Option::target_mode == Option::TARG_WIN_MODE && x == "win32")
	return TRUE;


    QRegExp re(x, FALSE, TRUE);
    QString spec = Option::mkfile::qmakespec.right(Option::mkfile::qmakespec.length() -
						   (Option::mkfile::qmakespec.findRev(QDir::separator())+1));
    if((regex && re.exactMatch(spec)) || (!regex && spec == x))
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
	    if((regex && re.exactMatch(r)) || (!regex && r == x))
		return TRUE;
	}
    }
#endif


    QStringList &configs = (place ? (*place)["CONFIG"] : vars["CONFIG"]);
    for(QStringList::Iterator it = configs.begin(); it != configs.end(); ++it) {
	if((regex && re.exactMatch((*it))) || (!regex && (*it) == x))
	if(re.exactMatch((*it)))
	    return TRUE;
    }
    return FALSE;
}

bool
QMakeProject::doProjectTest(const QString& func, const QString &params, QMap<QString, QStringList> &place)
{
    QStringList args = split_arg_list(params);
    for(QStringList::Iterator arit = args.begin(); arit != args.end(); ++arit) {
	QString tmp = (*arit).stripWhiteSpace();
	if((tmp[0] == '\'' || tmp[0] == '"') && tmp.right(1) == tmp.left(1))
	    tmp = tmp.mid(1, tmp.length() - 2);
    }
    return doProjectTest(func.stripWhiteSpace(), args, place);
}

/* If including a feature it will look in:

   1) environment variable QMAKEFEATURES (as separated by colons)
   2) <project_root> (where .qmake.cache lives) + FEATURES_DIR
   3) environment variable QMAKEPATH (as separated by colons) + /mkspecs/ + FEATURES_DIR
   4) your QMAKESPEC dir
   5) environment variable QTDIR + /mkspecs/ + FEATURES_DIR

   FEATURES_DIR is defined as:

   1) features/(unix|win32|macx)
   2) features
*/
bool
QMakeProject::doProjectInclude(QString file, bool feature, QMap<QString, QStringList> &place,
			       const QString &seek_var)
{

    if(feature) {
	if(!file.endsWith(Option::prf_ext))
	    file += Option::prf_ext;
	if(file.find(Option::dir_sep) == -1 || !QFile::exists(file)) {
	    bool found = FALSE;
	    QStringList concat;
	    {
		const QString base_concat = QDir::separator() + QString("features");
		switch(Option::target_mode) {
		case Option::TARG_MACX_MODE: 		    //also a unix
		    concat << base_concat + QDir::separator() + "macx";
		    concat << base_concat + QDir::separator() + "unix";
		    break;
		case Option::TARG_UNIX_MODE:
		    concat << base_concat + QDir::separator() + "unix";
		    break;
		case Option::TARG_WIN_MODE:
		    concat << base_concat + QDir::separator() + "win32";
		    break;
		case Option::TARG_MAC9_MODE:
		    concat << base_concat + QDir::separator() + "mac9";
		    break;
		case Option::TARG_QNX6_MODE: //also a unix
		    concat << base_concat + QDir::separator() + "qnx6";
		    concat << base_concat + QDir::separator() + "unix";
		    break;
		}
		concat << base_concat;
	    }
	    const QString mkspecs_concat = QDir::separator() + QString("mkspecs");
	    QStringList feature_roots;
	    if(const char *mkspec_path = getenv("QMAKEFEATURES")) {
#ifdef Q_OS_WIN
		QStringList lst = QStringList::split(';', mkspec_path);
		for(QStringList::Iterator it = lst.begin(); it != lst.end(); ++it)
		    feature_roots += QStringList::split(':', (*it));
#else
		feature_roots += QStringList::split(':', mkspec_path);
#endif
	    }
	    if(!Option::mkfile::cachefile.isEmpty()) {
		QString path;
		int last_slash = Option::mkfile::cachefile.findRev(Option::dir_sep);
		if(last_slash != -1)
		    path = Option::fixPathToLocalOS(Option::mkfile::cachefile.left(last_slash));
		for(QStringList::Iterator concat_it = concat.begin();
		    concat_it != concat.end(); ++concat_it)
		    feature_roots << (path + (*concat_it));
	    }
	    if(const char *qmakepath = getenv("QMAKEPATH")) {
#ifdef Q_OS_WIN
		QStringList lst = QStringList::split(';', qmakepath);
		for(QStringList::Iterator it = lst.begin(); it != lst.end(); ++it) {
		    QStringList lst2 = QStringList::split(':', (*it));
		    for(QStringList::Iterator it2 = lst2.begin(); it2 != lst2.end(); ++it2) {
			for(QStringList::Iterator concat_it = concat.begin();
			    concat_it != concat.end(); ++concat_it)
			    feature_roots << ((*it2) + mkspecs_concat + (*concat_it));
		    }
		}
#else
		QStringList lst = QStringList::split(':', qmakepath);
		for(QStringList::Iterator it = lst.begin(); it != lst.end(); ++it) {
		    for(QStringList::Iterator concat_it = concat.begin();
			concat_it != concat.end(); ++concat_it)
			feature_roots << ((*it) + mkspecs_concat + (*concat_it));
		}
#endif
	    }
	    feature_roots << Option::mkfile::qmakespec + QDir::separator() + "features";
	    if(const char *qtdir = getenv("QTDIR")) {
		for(QStringList::Iterator concat_it = concat.begin();
		    concat_it != concat.end(); ++concat_it) 
		    feature_roots << (QString(qtdir) + mkspecs_concat + (*concat_it));
	    }
#ifdef QT_INSTALL_PREFIX
	    for(QStringList::Iterator concat_it = concat.begin();
		concat_it != concat.end(); ++concat_it)
		feature_roots << (QT_INSTALL_PREFIX + mkspecs_concat + (*concat_it));
#endif
#if defined(HAVE_QCONFIG_CPP)
	    for(QStringList::Iterator concat_it = concat.begin();
		concat_it != concat.end(); ++concat_it)
		feature_roots << (qInstallPath() + mkspecs_concat + (*concat_it));
#endif
#ifdef QT_INSTALL_DATA
	    for(QStringList::Iterator concat_it = concat.begin();
		concat_it != concat.end(); ++concat_it)
		feature_roots << (QT_INSTALL_DATA + mkspecs_concat + (*concat_it));
#endif
#if defined(HAVE_QCONFIG_CPP)
	    for(QStringList::Iterator concat_it = concat.begin();
		concat_it != concat.end(); ++concat_it)
		feature_roots << (qInstallPathData() + mkspecs_concat + (*concat_it));
#endif
	    debug_msg(2, "Looking for feature '%s' in (%s)", file.latin1(), feature_roots.join("::").latin1());
	    for(QStringList::Iterator it = feature_roots.begin(); it != feature_roots.end(); ++it) {
		QString prf = (*it) + QDir::separator() + file;
		if(QFile::exists(prf)) {
		    found = TRUE;
		    file = prf;
		    break;
		}
	    }
	    if(!found)
		return FALSE;
	    if(vars["QMAKE_INTERNAL_INCLUDED_FEATURES"].findIndex(file) != -1)
		return TRUE;
	    vars["QMAKE_INTERNAL_INCLUDED_FEATURES"].append(file);

	}
    }
    if(QDir::isRelativePath(file)) {
	QStringList include_roots;
	include_roots << Option::output_dir;
	if(parser.from_file) {
	    QString pfilewd = QFileInfo(parser.file).dirPath();
	    if(pfilewd.isEmpty())
		include_roots << pfilewd;
	}
	if(Option::output_dir != QDir::currentDirPath())
	    include_roots << QDir::currentDirPath();
	for(QStringList::Iterator it = include_roots.begin(); it != include_roots.end(); ++it) {
	    if(QFile::exists((*it) + QDir::separator() + file)) {
		file = (*it) + QDir::separator() + file;
		break;
	    }
	}
    }

    if(Option::mkfile::do_preprocess) //nice to see this first..
	fprintf(stderr, "#switching file %s(%s) - %s:%d\n", feature ? "load" : "include", file.latin1(),
		parser.file.latin1(), parser.line_no);
    debug_msg(1, "Project Parser: %s'ing file %s.", feature ? "load" : "include", file.latin1());
    QString orig_file = file;
    int di = file.findRev(Option::dir_sep);
    QDir sunworkshop42workaround = QDir::current();
    QString oldpwd = sunworkshop42workaround.currentDirPath();
    if(di != -1) {
	if(!QDir::setCurrent(file.left(file.findRev(Option::dir_sep)))) {
	    fprintf(stderr, "Cannot find directory: %s\n", file.left(di).latin1());
	    return FALSE;
	}
	file = file.right(file.length() - di - 1);
    }
    parser_info pi = parser;
    QStack<ScopeBlock> sc = scope_blocks;
    bool r = FALSE;
    if(!seek_var.isNull()) {
	QMap<QString, QStringList> tmp;
	if((r = read(file.latin1(), tmp)))
	    place[seek_var] += tmp[seek_var];
    } else {
	r = read(file.latin1(), place);
    }
    if(r)
	vars["QMAKE_INTERNAL_INCLUDED_FILES"].append(orig_file);
    else
	warn_msg(WarnParser, "%s:%d: Failure to include file %s.",
		 pi.file.latin1(), pi.line_no, orig_file.latin1());
    parser = pi;
    scope_blocks = sc;
    QDir::setCurrent(oldpwd);
    return r;
}

bool
QMakeProject::doProjectTest(const QString& func, QStringList args, QMap<QString, QStringList> &place)
{
    for(QStringList::Iterator arit = args.begin(); arit != args.end(); ++arit) {
	(*arit) = (*arit).stripWhiteSpace(); // blah, get rid of space
	doVariableReplace((*arit), place);
    }
    debug_msg(1, "Running project test: %s( %s )", func.latin1(), args.join("::").latin1());

    if(func == "requires") {
	return doProjectCheckReqs(args, place);
    } else if(func == "equals" || func == "isEqual") {
	if(args.count() != 2) {
	    fprintf(stderr, "%s:%d: %s(variable, value) requires two arguments.\n", parser.file.latin1(),
		    parser.line_no, func.latin1());
	    return FALSE;
	}
	QString value = args[1];
	if((value.left(1) == "\"" || value.left(1) == "'") && value.right(1) == value.left(1))
	    value = value.mid(1, value.length()-2);
	return vars[args[0]].join(" ") == value;
    } else if(func == "exists") {
	if(args.count() != 1) {
	    fprintf(stderr, "%s:%d: exists(file) requires one argument.\n", parser.file.latin1(),
		    parser.line_no);
	    return FALSE;
	}
	QString file = args.first();
	file = Option::fixPathToLocalOS(file);
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
    } else if(func == "eval") {
	if(args.count() < 1) {
	    fprintf(stderr, "%s:%d: eval(project) requires one argument.\n", parser.file.latin1(),
		    parser.line_no);
	    return FALSE;
	}
	QString project = args.first();
	parser_info pi = parser;
	parser.from_file = FALSE;
	parser.file = "(eval)";
	parser.line_no = 0;
	QTextStream t(&project, IO_ReadOnly);
	bool ret = read(t, place);
	parser = pi;
	return ret;
    } else if(func == "system") {
	if(args.count() != 1) {
	    fprintf(stderr, "%s:%d: system(exec) requires one argument.\n", parser.file.latin1(),
		    parser.line_no);
	    return FALSE;
	}
	return system(args.first().latin1()) == 0;
    } else if(func == "contains") {
	if(args.count() != 2) {
	    fprintf(stderr, "%s:%d: contains(var, val) requires two arguments.\n", parser.file.latin1(),
		    parser.line_no);
	    return FALSE;
	}
	QRegExp regx(args[1]);
	QStringList &l = place[args[0]];
	for(QStringList::ConstIterator it = l.begin(); it != l.end(); ++it) {
	    if(regx.exactMatch((*it)))
		return TRUE;
	}
	return FALSE;
    } else if(func == "infile") {
	if(args.count() < 2 || args.count() > 3) {
	    fprintf(stderr, "%s:%d: infile(file, var, val) requires at least 2 arguments.\n",
		    parser.file.latin1(), parser.line_no);
	    return FALSE;
	}
	QMakeProject proj;
	QString file = args[0];
	doVariableReplace(file, place);
	fixEnvVariables(file);
	int di = file.findRev(Option::dir_sep);
	QDir sunworkshop42workaround = QDir::current();
	QString oldpwd = sunworkshop42workaround.currentDirPath();
	if(di != -1) {
	    if(!QDir::setCurrent(file.left(file.findRev(Option::dir_sep)))) {
		fprintf(stderr, "Cannot find directory: %s\n", file.left(di).latin1());
		return FALSE;
	    }
	    file = file.right(file.length() - di - 1);
	}
	parser_info pi = parser;
	bool ret = !proj.read(file, oldpwd);
	parser = pi;
	if(ret) {
	    fprintf(stderr, "Error processing project file: %s\n", file.latin1());
	    QDir::setCurrent(oldpwd);
	    return FALSE;
	}
	if(args.count() == 2) {
	    ret = !proj.isEmpty(args[1]);
	} else {
	    QRegExp regx(args[2]);
	    QStringList &l = proj.values(args[1]);
	    for(QStringList::ConstIterator it = l.begin(); it != l.end(); ++it) {
		if(regx.exactMatch((*it))) {
		    ret = TRUE;
		    break;
		}
	    }
	}
	QDir::setCurrent(oldpwd);
	return ret;
    } else if(func == "count") {
	if(args.count() != 2) {
	    fprintf(stderr, "%s:%d: count(var, count) requires two arguments.\n", parser.file.latin1(),
		    parser.line_no);
	    return FALSE;
	}
	return vars[args[0]].count() == args[1].toInt();
    } else if(func == "isEmpty") {
	if(args.count() != 1) {
	    fprintf(stderr, "%s:%d: isEmpty(var) requires one argument.\n", parser.file.latin1(),
		    parser.line_no);
	    return FALSE;
	}
	return vars[args[0]].isEmpty();
    } else if(func == "include" || func == "load") {
	QString seek_var;
	const bool include_statement = (func == "include");
	bool ignore_error = include_statement;
	if(args.count() == 2) {
	    if(func == "include") {
		seek_var = args[1];
	    } else {
		QString sarg = args[1];
		ignore_error = (sarg.lower() == "true" || sarg.toInt());
	    }
	} else if(args.count() != 1) {
	    QString func_desc = "load(feature)";
	    if(include_statement)
		func_desc = "include(file)";
	    fprintf(stderr, "%s:%d: %s requires one argument.\n", parser.file.latin1(),
		    parser.line_no, func_desc.latin1());
	    return FALSE;
	}
	QString file = args.first();
	file = Option::fixPathToLocalOS(file);
	file.replace("\"", "");
	bool ret = doProjectInclude(file, !include_statement, place, seek_var);
	if(!ret && !ignore_error) {
	    printf("Project LOAD(): Feature %s cannot be found.\n", file.latin1());
	    exit(3);
	}
	return ret;
    } else if(func == "error" || func == "message" || func == "warning") {
	if(args.count() != 1) {
	    fprintf(stderr, "%s:%d: %s(message) requires one argument.\n", parser.file.latin1(),
		    parser.line_no, func.latin1());
	    return FALSE;
	}
	QString msg = args.first();
	if((msg.startsWith("\"") || msg.startsWith("'")) && msg.endsWith(msg.left(1)))
	    msg = msg.mid(1, msg.length()-2);
	msg.replace(QString("${QMAKE_FILE}"), parser.file.latin1());
	msg.replace(QString("${QMAKE_LINE_NUMBER}"), QString::number(parser.line_no));
	msg.replace(QString("${QMAKE_DATE}"), QDateTime::currentDateTime().toString());
	doVariableReplace(msg, place);
	fixEnvVariables(msg);
	fprintf(stderr, "Project %s: %s\n", func.upper().latin1(), msg.latin1());
	if(func == "error")
	    exit(2);
	return TRUE;
    } else {
	fprintf(stderr, "%s:%d: Unknown test function: %s\n", parser.file.latin1(), parser.line_no,
		func.latin1());
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
		error.sprintf("Function (in REQUIRES) missing right paren: %s", chk.latin1());
		qmake_error_msg(error);
	    } else {
		QString func = chk.left(lparen);
		test = doProjectTest(func, chk.mid(lparen+1, rparen - lparen - 1), place);
	    }
	} else {
	    test = isActiveConfig(chk, TRUE, &place);
	}
	if(invert_test) {
	    chk.prepend("!");
	    test = !test;
	}
	if(!test) {
	    debug_msg(1, "Project Parser: %s:%d Failed test: REQUIRES = %s",
		      parser.file.latin1(), parser.line_no, chk.latin1());
	    place["QMAKE_FAILED_REQUIREMENTS"].append(chk);
	    ret = FALSE;
	}
    }
    return ret;
}


QString
QMakeProject::doVariableReplace(QString &str, const QMap<QString, QStringList> &place)
{
    for(int var_begin, var_last=0; (var_begin = str.find("$$", var_last)) != -1; var_last = var_begin) {
	if(var_begin >= (int)str.length() + 2) {
	    break;
	} else if(var_begin != 0 && str[var_begin-1] == '\\') {
	    str.replace(var_begin-1, 1, "");
	    var_begin += 1;
	    continue;
	}

	int var_incr = var_begin + 2;
	bool in_braces = FALSE, as_env = FALSE, as_prop = FALSE;
	if(str[var_incr] == '{') {
	    in_braces = TRUE;
	    var_incr++;
	    while(var_incr < (int)str.length() &&
		  (str[var_incr] == ' ' || str[var_incr] == '\t' || str[var_incr] == '\n'))
		var_incr++;
	}
	if(str[var_incr] == '(') {
	    as_env = TRUE;
	    var_incr++;
	} else if(str[var_incr] == '[') {
	    as_prop = TRUE;
	    var_incr++;
	}
	QString val, args;
	while(var_incr < (int)str.length() &&
	      (str[var_incr].isLetter() || str[var_incr].isNumber() || str[var_incr] == '.' || str[var_incr] == '_'))
	    val += str[var_incr++];
	if(as_env) {
	    if(str[var_incr] != ')') {
		var_incr++;
		warn_msg(WarnParser, "%s:%d: Unterminated env-variable replacement '%s' (%s)",
			 parser.file.latin1(), parser.line_no,
			 str.mid(var_begin, QMAX(var_incr - var_begin,
						 (int)str.length())).latin1(), str.latin1());
		var_begin += var_incr;
		continue;
	    }
	    var_incr++;
	} else if(as_prop) {
	    if(str[var_incr] != ']') {
		var_incr++;
		warn_msg(WarnParser, "%s:%d: Unterminated prop-variable replacement '%s' (%s)",
			 parser.file.latin1(), parser.line_no,
			 str.mid(var_begin, QMAX(var_incr - var_begin, int(str.length()))).latin1(), str.latin1());
		var_begin += var_incr;
		continue;
	    }
	    var_incr++;
	} else if(str[var_incr] == '(') { //args
	    for(int parens = 0; var_incr < (int)str.length(); var_incr++) {
		if(str[var_incr] == '(') {
		    parens++;
		    if(parens == 1)
			continue;
		} else if(str[var_incr] == ')') {
		    parens--;
		    if(!parens) {
			var_incr++;
			break;
		    }
		}
		args += str[var_incr];
	    }
	}
	if(var_incr > (int)str.length() || (in_braces && str[var_incr] != '}')) {
	    var_incr++;
	    warn_msg(WarnParser, "%s:%d: Unterminated variable replacement '%s' (%s)",
		     parser.file.latin1(), parser.line_no,
		     str.mid(var_begin, QMAX(var_incr - var_begin,
					     (int)str.length())).latin1(), str.latin1());
	    var_begin += var_incr;
	    continue;
	} else if(in_braces) {
	    var_incr++;
	}

	QString replacement;
	if(as_env) {
	    replacement = getenv(val);
	} else if(as_prop) {
	    if(prop)
		replacement = prop->value(val);
	} else if(args.isEmpty()) {
	    if(val.left(1) == ".")
		replacement = "";
	    else if(val == "LITERAL_WHITESPACE")
		replacement = "\t";
	    else if(val == "LITERAL_DOLLAR")
		replacement = "$";
	    else if(val == "LITERAL_HASH")
		replacement = "#";
	    else if(val == "PWD")
		replacement = QDir::currentDirPath();
	    else
		replacement = place[varMap(val)].join(" ");
	} else {
	    QStringList arg_list = split_arg_list(doVariableReplace(args, place));
	    debug_msg(1, "Running function: %s( %s )", val.latin1(), arg_list.join("::").latin1());
	    if(val.lower() == "member") {
		if(arg_list.count() < 1 || arg_list.count() > 3) {
		    fprintf(stderr, "%s:%d: member(var, start, end) requires three arguments.\n",
			    parser.file.latin1(), parser.line_no);
		} else {
		    const QStringList &var = place[varMap(arg_list.first())];
		    int start = 0;
		    if(arg_list.count() >= 2)
			start = arg_list[1].toInt();
		    if(start < 0)
			start += var.count();
		    int end = start;
		    if(arg_list.count() == 3)
			end = arg_list[2].toInt();
		    if(end < 0)
			end += var.count();
		    if(end < start)
			end = start;
		    for(int i = start; i <= end && (int)var.count() >= i; i++) {
			if(!replacement.isEmpty())
			    replacement += " ";
			replacement += var[i];
		    }
		}
	    } else if(val.lower() == "fromfile") {
		if(arg_list.count() != 2) {
		    fprintf(stderr, "%s:%d: fromfile(file, variable) requires two arguments.\n",
			    parser.file.latin1(), parser.line_no);
		} else {
		    QString file = arg_list[0], seek_var = arg_list[1];
		    file = Option::fixPathToLocalOS(file);
		    file.replace("\"", "");

		    QMap<QString, QStringList> tmp;
		    if(doProjectInclude(file, FALSE, tmp, seek_var))
			replacement = tmp[seek_var].join(" "); //should I use " "?
		}
	    } else if(val.lower() == "eval") {
		for(QStringList::ConstIterator arg_it = arg_list.begin();
		    arg_it != arg_list.end(); ++arg_it) {
		    if(!replacement.isEmpty())
			replacement += " ";
		    replacement += place[(*arg_it)].join(" ");
		}
	    } else if(val.lower() == "list") {
		static int x = 0;
		replacement.sprintf(".QMAKE_INTERNAL_TMP_VAR_%d", x++);
		QStringList &lst = (*((QMap<QString, QStringList>*)&place))[replacement];
		lst.clear();
		for(QStringList::ConstIterator arg_it = arg_list.begin();
		    arg_it != arg_list.end(); ++arg_it)
		    lst += split_value_list((*arg_it));
	    } else if(val.lower() == "printf") {
		if(arg_list.count() < 1) {
		    fprintf(stderr, "%s:%d: printf(format, ...) requires one argument.\n",
			    parser.file.latin1(), parser.line_no);
		} else {
		    replacement = arg_list.first().replace("\"", "" );
		    QStringList::Iterator arg_it = arg_list.begin();
		    ++arg_it;
		    for( ; arg_it != arg_list.end(); ++arg_it) 
			replacement = replacement.arg((*arg_it).replace("\"", "" ));
		}
	    } else if(val.lower() == "join") {
		if(arg_list.count() < 1 || arg_list.count() > 4) {
		    fprintf(stderr, "%s:%d: join(var, glue, before, after) requires four"
			    "arguments.\n", parser.file.latin1(), parser.line_no);
		} else {
		    QString glue, before, after;
		    if(arg_list.count() >= 2)
			glue = arg_list[1].replace("\"", "" );
		    if(arg_list.count() >= 3)
			before = arg_list[2].replace("\"", "" );
		    if(arg_list.count() == 4)
			after = arg_list[3].replace("\"", "" );
		    const QStringList &var = place[varMap(arg_list.first())];
		    if(!var.isEmpty())
			replacement = before + var.join(glue) + after;
		}
	    } else if(val.lower() == "split") {
		if(arg_list.count() < 2 || arg_list.count() > 3) {
		    fprintf(stderr, "%s:%d split(var, sep, join) requires three arguments\n",
			    parser.file.latin1(), parser.line_no);
		} else {
		    QString sep = arg_list[1], join = " ";
		    if(arg_list.count() == 3)
			join = arg_list[2];
		    QStringList var = place[varMap(arg_list.first())];
		    for(QStringList::Iterator vit = var.begin(); vit != var.end(); ++vit) {
			QStringList lst = QStringList::split(sep, (*vit));
			for(QStringList::Iterator spltit = lst.begin(); spltit != lst.end(); ++spltit) {
			    if(!replacement.isEmpty())
				replacement += join;
			    replacement += (*spltit);
			}
		    }
		}
	    } else if(val.lower() == "find") {
		if(arg_list.count() != 2) {
		    fprintf(stderr, "%s:%d find(var, str) requires two arguments\n",
			    parser.file.latin1(), parser.line_no);
		} else {
		    QRegExp regx(arg_list[1]);
		    const QStringList &var = place[varMap(arg_list.first())];
		    for(QStringList::ConstIterator vit = var.begin();
			vit != var.end(); ++vit) {
			if(regx.search(*vit) != -1) {
			    if(!replacement.isEmpty())
				replacement += " ";
			    replacement += (*vit);
			}
		    }
		}
	    } else if(val.lower() == "system") {
		if(arg_list.count() != 1) {
		    fprintf(stderr, "%s:%d system(execut) requires one argument\n",
			    parser.file.latin1(), parser.line_no);
		} else {
		    char buff[256];
		    FILE *proc = QT_POPEN(arg_list.join(" ").latin1(), "r");
		    while(proc && !feof(proc)) {
			int read_in = fread(buff, 1, 255, proc);
			if(!read_in)
			    break;
			for(int i = 0; i < read_in; i++) {
			    if(buff[i] == '\n' || buff[i] == '\t')
				buff[i] = ' ';
			}
			buff[read_in] = '\0';
			replacement += buff;
		    }
		}
	    } else if(val.lower() == "files") {
		if(arg_list.count() != 1) {
		    fprintf(stderr, "%s:%d files(pattern) requires one argument\n",
			    parser.file.latin1(), parser.line_no);
		} else {
		    QString dir, regex = arg_list[0];
		    regex = Option::fixPathToLocalOS(regex);
		    regex.replace("\"", "");
		    if(regex.findRev(QDir::separator()) != -1) {
			dir = regex.left(regex.findRev(QDir::separator()) + 1);
			regex = regex.right(regex.length() - dir.length());
		    }
		    QDir d(dir, regex);
		    for(int i = 0; i < (int)d.count(); i++) {
			if(!replacement.isEmpty())
			    replacement += " ";
			replacement += dir + d[i];
		    }
		}
	    } else if(val.lower() == "prompt") {
		if(arg_list.count() != 1) {
		    fprintf(stderr, "%s:%d prompt(question) requires one argument\n",
			    parser.file.latin1(), parser.line_no);
		} else if(projectFile() == "-") {
		    fprintf(stderr, "%s:%d prompt(question) cannot be used when '-o -' is used.\n",
			    parser.file.latin1(), parser.line_no);
		} else {
		    QString msg = arg_list.first();
		    if((msg.startsWith("\"") || msg.startsWith("'")) && msg.endsWith(msg.left(1)))
			msg = msg.mid(1, msg.length()-2);
		    msg.replace(QString("${QMAKE_FILE}"), parser.file.latin1());
		    msg.replace(QString("${QMAKE_LINE_NUMBER}"), QString::number(parser.line_no));
		    msg.replace(QString("${QMAKE_DATE}"), QDateTime::currentDateTime().toString());
		    doVariableReplace(msg, place);
		    fixEnvVariables(msg);
		    if(!msg.endsWith("?"))
			msg += "?";
		    fprintf(stderr, "Project %s: %s ", val.upper().latin1(), msg.latin1());

		    QFile qfile;
		    if(qfile.open(IO_ReadOnly, stdin)) {
			QTextStream t(&qfile);
			replacement = t.readLine();
		    }
		}
	    } else {
		fprintf(stderr, "%s:%d: Unknown replace function: %s\n",
			parser.file.latin1(), parser.line_no, val.latin1());
	    }
	}
	//actually do replacement now..
	int mlen = var_incr - var_begin;
	debug_msg(2, "Project Parser [var replace]: '%s' :: %s -> %s", str.latin1(),
		  str.mid(var_begin, mlen).latin1(), replacement.latin1());
	str.replace(var_begin, mlen, replacement);
	var_begin += replacement.length();
    }
    return str;
}
