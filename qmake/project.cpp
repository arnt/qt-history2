/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "project.h"
#include "property.h"
#include "option.h"
#include <qdatetime.h>
#include <qfile.h>
#include <qdir.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qstack.h>
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

static QString remove_quotes(QString arg)
{
    if(arg.size() >= 2 &&
       (arg.at(0) == '\'' || arg.at(0) == '"') && arg.at(arg.length()-1) == arg.at(0))
        return arg.mid(1, arg.length()-2);
    return arg;
}

//just a parsable entity
struct ParsableBlock
{
    ParsableBlock() { }
    virtual ~ParsableBlock() { }

    struct Parse {
        QString text;
        parser_info pi;
        Parse(const QString &t) : text(t){ pi = ::parser; }
    };
    QList<Parse> parser;

protected:
    virtual bool continueBlock() = 0;
    bool eval(QMakeProject *p);
};

bool ParsableBlock::eval(QMakeProject *p)
{
    //save state
    parser_info pi = ::parser;

    //execute
    bool ret = true;
    for(int i = 0; i < parser.count(); i++) {
        ::parser = parser.at(i).pi;
        if(!(ret = p->parse(parser.at(i).text, p->variables())) || !continueBlock())
            break;
    }

    //restore state
    ::parser = pi;
    return ret;
}


//defined functions
struct FunctionBlock : public ParsableBlock
{
    FunctionBlock() : scope_level(1), cause_return(false) { }

    QString return_value;
    int scope_level;
    bool cause_return;

    bool exec(QMakeProject *p, const QStringList &args, QString &functionReturn);
    virtual bool continueBlock() { return !cause_return; }

protected:
    bool eval(QMakeProject *p);
};

bool FunctionBlock::exec(QMakeProject *p, const QStringList &args, QString &functionReturn)
{
    //save state
    return_value = "";
    cause_return = false;
    p->function_blocks.push(this);

    //execute
    QList<QStringList> va;
    QStringList args_old = p->variables()["ARGS"];
    p->variables()["ARGS"] = args;
    for(int i = 0; i < args.count(); i++) {
        va.append(p->variables()[QString::number(i+1)]);
        p->variables()[QString::number(i+1)] = QStringList(args[i]);
    }
    bool ret = ParsableBlock::eval(p);
    functionReturn = return_value;
    for(int i = 0; i < va.count(); i++)
        p->variables()[QString::number(i+1)] = va[i];
    p->variables()["ARG_COUNT"] = args_old; //just to be carefull

    //restore state
    p->iterator = 0;
    p->function = 0;
    return_value = QString::null;
    Q_ASSERT(p->function_blocks.pop() == this);
    return ret;
}

//loops
struct IteratorBlock : public ParsableBlock
{
    IteratorBlock() : scope_level(1), loop_forever(false), cause_break(false), cause_next(false) { }

    int scope_level;

    struct Test {
        QString func;
        QStringList args;
        bool invert;
        parser_info pi;
        Test(const QString &f, QStringList &a, bool i) : func(f), args(a), invert(i) { pi = ::parser; }
    };
    QList<Test> test;

    QString variable;

    bool loop_forever, cause_break, cause_next;
    QStringList list;

    bool exec(QMakeProject *p);
    virtual bool continueBlock() { return !cause_next && !cause_break; }
};
bool IteratorBlock::exec(QMakeProject *p)
{
    bool ret = true;
    QStringList::Iterator it;
    if(!loop_forever)
        it = list.begin();
    int iterate_count = 0;
    //save state
    p->iterator = this;

    //do the loop
    while(loop_forever || it != list.end()) {
        cause_next = cause_break = false;
        if(!loop_forever && (*it).isEmpty()) //ignore empty items
            continue;

        //set up the loop variable
        QStringList va;
        if(!variable.isEmpty()) {
            va = p->variables()[variable];
            if(loop_forever)
                p->variables()[variable] = QStringList(QString::number(iterate_count));
            else
                p->variables()[variable] = QStringList(*it);
        }
        //do the iterations
        bool succeed = true;
        for(QList<Test>::Iterator test_it = test.begin(); test_it != test.end(); ++test_it) {
            ::parser = (*test_it).pi;
            succeed = p->doProjectTest((*test_it).func, (*test_it).args, p->variables());
            if((*test_it).invert)
                succeed = !succeed;
            if(!succeed)
                break;
        }
        if(succeed)
            ret = ParsableBlock::eval(p);
        //restore the variable in the map
        if(!variable.isEmpty())
            p->variables()[variable] = va;
        //loop counters
        if(!loop_forever)
            ++it;
        iterate_count++;
        if(!ret || cause_break)
            break;
    }

    //restore state
    p->iterator = 0;
    p->function = 0;
    return ret;
}

QMakeProject::ScopeBlock::~ScopeBlock()
{
#if 0
    if(iterate)
        delete iterate;
#endif
}


static void qmake_error_msg(const char *msg)
{
    fprintf(stderr, "%s:%d: %s\n", parser.file.latin1(), parser.line_no, msg);
}

QStringList qmake_mkspec_paths()
{
    QStringList ret;
    const QString concat = QDir::separator() + QString("mkspecs");
    if(const char *qmakepath = qgetenv("QMAKEPATH")) {
#ifdef Q_OS_WIN
        QStringList lst = QString(qmakepath).split(';');
        for(QStringList::Iterator it = lst.begin(); it != lst.end(); ++it) {
            QStringList lst2 = (*it).split(':');
            for(QStringList::Iterator it2 = lst2.begin(); it2 != lst2.end(); ++it2)
                ret << ((*it2) + concat);
        }
#else
        QStringList lst = QString(qmakepath).split(':');
        for(QStringList::Iterator it = lst.begin(); it != lst.end(); ++it)
            ret << ((*it) + concat);
#endif
    }
    if(const char *qtdir = qgetenv("QTDIR"))
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

    // prefer $QTDIR if it is set
    if(qgetenv("QTDIR"))
        ret << qgetenv("QTDIR");
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
    else if(ret == "PRECOMPH")
        ret = "PRECOMPILED_HEADER";
    else if(ret == "PRECOMPCPP")
        ret = "PRECOMPILED_SOURCE";
    else if(ret == "INCPATH")
        ret = "INCLUDEPATH";
    else if(ret == "QMAKE_EXTRA_WIN_COMPILERS" || ret == "QMAKE_EXTRA_UNIX_COMPILERS")
        ret = "QMAKE_EXTRA_COMPILERS";
    else if(ret == "QMAKE_EXTRA_WIN_TARGETS" || ret == "QMAKE_EXTRA_UNIX_TARGETS")
        ret = "QMAKE_EXTRA_TARGETS";
    else if(ret == "QMAKE_EXTRA_UNIX_INCLUDES")
        ret = "QMAKE_EXTRA_INCLUDES";
    else if(ret == "QMAKE_EXTRA_UNIX_VARIABLES")
        ret = "QMAKE_EXTRA_VARIABLES";
    else if(ret == "QMAKE_RPATH")
        ret = "QMAKE_LFLAGS_RPATH";
    return ret;
}

static QStringList split_arg_list(QString params)
{
    QChar quote = 0;
    QStringList args;
    for(int x = 0, last = 0, parens = 0; x <= (int)params.length(); x++) {
        if(x == (int)params.length()) {
            QString mid = params.mid(last, x - last).trimmed();
            if(quote.unicode()) {
                if(mid[0] == quote && mid[(int)mid.length()-1] == quote)
                    mid = mid.mid(1, mid.length()-2);
                quote = 0;
            }
            args << mid;
        } else if(params[x] == ')') {
            parens--;
        } else if(params[x] == '(') {
            parens++;
        } else if(quote.unicode() && params[x] == quote) {
            quote = 0;
        } else if(!quote.unicode() && (params[x] == '\'' || params[x] == '"')) {
            quote = params[x];
        } else if(!parens && !quote.unicode() && params[x] == ',') {
            QString mid = params.mid(last, x - last).trimmed();
            if(quote.unicode()) {
                if(mid[0] == quote && mid[(int)mid.length()-1] == quote)
                    mid = mid.mid(1, mid.length()-2);
                quote = 0;
            }
            args << mid;
            last = x+1;
        }
    }
    for(int i = 0; i < args.count(); i++)
        args[i] = remove_quotes(args[i]);
    return args;
}

static QStringList split_value_list(const QString &vals, bool do_semicolon=false)
{
    QString build;
    QStringList ret;
    QStack<QChar> quote;
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

        if(quote.isEmpty() && ((do_semicolon && vals[x] == ';') ||  vals[x] == Option::field_sep)) {
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

QMakeProject::~QMakeProject()
{
    if(own_prop)
        delete prop;
}


void
QMakeProject::init(QMakeProperty *p, const QMap<QString, QStringList> *vars)
{
    if(vars)
        base_vars = *vars;
    iterator = 0;
    function = 0;
    if(!p) {
        prop = new QMakeProperty;
        own_prop = true;
    } else {
        prop = p;
        own_prop = false;
    }
}

void
QMakeProject::reset()
{
    // scope_blocks starts with one non-ignoring entity
    scope_blocks.clear();
    scope_blocks.push(ScopeBlock());
    iterator = 0;
    function = 0;
}

bool
QMakeProject::parse(const QString &t, QMap<QString, QStringList> &place)
{
    QString s = t.simplified();
    int hash_mark = s.indexOf("#");
    if(hash_mark != -1) //good bye comments
        s = s.left(hash_mark);
    if(s.isEmpty()) // blank_line
        return true;

    if(scope_blocks.top().ignore) {
        bool continue_parsing = false;
        // adjust scope for each block which appears on a single line
        for(int i = 0; i < s.length(); i++) {
            if(s[i] == '{') {
                scope_blocks.push(ScopeBlock(true));
            } else if(s[i] == '}') {
                if(scope_blocks.count() == 1) {
                    fprintf(stderr, "Braces mismatch %s:%d\n", parser.file.latin1(), parser.line_no);
                    return false;
                }
                ScopeBlock sb = scope_blocks.pop();
                if(sb.iterate)
                    sb.iterate->exec(this);
                if(!scope_blocks.top().ignore) {
                    debug_msg(1, "Project Parser: %s:%d : Leaving block %d", parser.file.latin1(),
                              parser.line_no, scope_blocks.count()+1);
                    s = s.mid(i+1).trimmed();
                    continue_parsing = !s.isEmpty();
                    break;
                }
            }
        }
        if(!continue_parsing) {
            debug_msg(1, "Project Parser: %s:%d : Ignored due to block being false.",
                      parser.file.latin1(), parser.line_no);
            return true;
        }
    }

    if(function) {
        QString append;
        const char *d = s.latin1();
        bool function_finished = false;
        while(*d) {
            if((*d) == '}') {
                function->scope_level--;
                if(!function->scope_level) {
                    function_finished = true;
                    break;
                }
            } else if((*d) == '{') {
                function->scope_level++;
            }
            append += *d;
            d++;
        }
        if(!append.isEmpty())
            function->parser.append(IteratorBlock::Parse(append));
        if(function_finished) {
            function = 0;
            s = QString(d);
        } else {
            return true;
        }
    } else if(IteratorBlock *it = scope_blocks.top().iterate) {
        QString append;
        const char *d = s.latin1();
        bool iterate_finished = false;
        while(*d) {
            if((*d) == '}') {
                it->scope_level--;
                if(!it->scope_level) {
                    iterate_finished = true;
                    break;
                }
            } else if((*d) == '{') {
                it->scope_level++;
            }
            append += *d;
            d++;
        }
        if(!append.isEmpty())
            scope_blocks.top().iterate->parser.append(IteratorBlock::Parse(append));
        if(iterate_finished) {
            scope_blocks.top().iterate = 0;
            bool ret = it->exec(this);
            delete it;
            if(!ret)
                return false;
            s = QString(d);
        } else {
            return true;
        }
    }

    QString scope, var, op;
    QStringList val;
#define SKIP_WS(d) while(*d && (*d == ' ' || *d == '\t')) d++
    const char *d = s.latin1();
    SKIP_WS(d);
    IteratorBlock *iterator = 0;
    bool scope_failed = false, else_line = false, or_op=false, start_scope=false;
    char quote = 0;
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
                        return false;
                    }
                }
            }
        }

        if(quote) {
            if(*d == quote)
                quote = 0;
        } else if(*d == '(') {
            ++parens;
        } else if(*d == ')') {
            --parens;
        } else if(*d == '"' /*|| *d == '\''*/) {
            quote = *d;
        }

        if(!parens && !quote && (*d == ':' || *d == '{' || *d == ')' || *d == '|')) {
            scope_count++;
            scope = var.trimmed();
            if(*d == ')')
                scope += *d; // need this
            var = "";

            bool test = scope_failed;
            if(scope.toLower() == "else") { //else is a builtin scope here as it modifies state
                if(scope_count != 1 || scope_blocks.top().else_status == ScopeBlock::TestNone) {
                    qmake_error_msg("Unexpected " + scope + " ('" + s + "')");
                    return false;
                }
                else_line = true;
                test = (scope_blocks.top().else_status == ScopeBlock::TestSeek);
                debug_msg(1, "Project Parser: %s:%d : Else%s %s.", parser.file.latin1(), parser.line_no,
                          scope == "else" ? "" : QString(" (" + scope + ")").latin1(),
                          test ? "considered" : "excluded");
            } else {
                QString comp_scope = scope;
                bool invert_test = (comp_scope.left(1) == "!");
                if(invert_test)
                    comp_scope = comp_scope.right(comp_scope.length()-1);
                int lparen = comp_scope.indexOf('(');
                if(or_op == scope_failed) {
                    if(lparen != -1) { // if there is an lparen in the scope, it IS a function
                        int rparen = comp_scope.lastIndexOf(')');
                        if(rparen == -1) {
                            QByteArray error;
                            error.reserve(256);
                            sprintf(error.data(), "Function missing right paren: %s ('%s')",
                                    comp_scope.latin1(), s.latin1());
                            qmake_error_msg(error);
                            return false;
                        }
                        QString func = comp_scope.left(lparen);
                        QStringList args = split_arg_list(comp_scope.mid(lparen+1, rparen - lparen - 1));
                        for(int i = 0; i < args.size(); ++i)
                            args[i] = remove_quotes(args[i].trimmed());
                        if(function) {
                            fprintf(stderr, "%s:%d: No tests can come after a function definition!\n",
                                    parser.file.latin1(), parser.line_no);
                            return false;
                        } else if(func == "for") { //for is a builtin function here, as it modifies state
                            if(args.count() > 2 || args.count() < 1) {
                                fprintf(stderr, "%s:%d: for(iterate, list) requires two arguments.\n",
                                        parser.file.latin1(), parser.line_no);
                                return false;
                            }

                            iterator = new IteratorBlock;
                            QString it_list;
                            if(args.count() == 1) {
                                it_list = doVariableReplace(args[0], place);
                                if(it_list != "ever") {
                                    delete iterator;
                                    iterator = 0;
                                    fprintf(stderr, "%s:%d: for(iterate, list) requires two arguments.\n",
                                            parser.file.latin1(), parser.line_no);
                                    return false;
                                }
                                it_list = "forever";
                            } else if(args.count() == 2) {
                                iterator->variable = args[0];
                                it_list = doVariableReplace(args[1], place);
                            }
                            QStringList list = place[it_list];
                            if(list.isEmpty()) {
                                if(it_list == "forever") {
                                    iterator->loop_forever = true;
                                } else {
                                    int dotdot = it_list.indexOf("..");
                                    if(dotdot != -1) {
                                        bool ok;
                                        int start = it_list.left(dotdot).toInt(&ok);
                                        if(ok) {
                                            int end = it_list.mid(dotdot+2).toInt(&ok);
                                            if(ok) {
                                                if(start < end) {
                                                    for(int i = start; i <= end; i++)
                                                        list << QString::number(i);
                                                } else {
                                                    for(int i = start; i >= end; i--)
                                                        list << QString::number(i);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            iterator->list = list;
                            test = !invert_test;
                        } else if(iterator) {
                            iterator->test.append(IteratorBlock::Test(func, args, invert_test));
                            test = !invert_test;
                        } else if(func == "defineTest" || func == "defineReplace") {
                            if(!function_blocks.isEmpty()) {
                                fprintf(stderr,
                                        "%s:%d: cannot define a function within another definition.\n",
                                        parser.file.latin1(), parser.line_no);
                                return false;
                            }
                            if(args.count() != 1) {
                                fprintf(stderr, "%s:%d: %s(function_name) requires one arguments.\n",
                                        parser.file.latin1(), parser.line_no, func.latin1());
                                return false;
                            }
                            QMap<QString, FunctionBlock*> *map = 0;
                            if(func == "defineTest")
                                map = &testFunctions;
                            else
                                map = &replaceFunctions;
                            if(!map || map->contains(args[0])) {
                                   fprintf(stderr, "%s:%d: Function[%s] multiply defined.\n",
                                           parser.file.latin1(), parser.line_no, args[0].latin1());
                                   return false;
                            }
                            function = new FunctionBlock;
                            map->insert(args[0], function);
                            test = true;
                        } else {
                            test = doProjectTest(func, args, place);
                            if(*d == ')' && !*(d+1)) {
                                if(invert_test)
                                    test = !test;
                                scope_blocks.top().else_status =
                                    (test ? ScopeBlock::TestFound : ScopeBlock::TestSeek);
                                return true;  // assume we are done
                            }
                        }
                    } else {
                        QString cscope = comp_scope.trimmed();
                        doVariableReplace(cscope, place);
                        test = isActiveConfig(cscope.trimmed(), true, &place);
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

            if(*d == '{') // scoping block
                start_scope = true;
        } else if(!parens && *d == '}') {
            if(!scope_blocks.count()) {
                warn_msg(WarnParser, "Possible braces mismatch %s:%d", parser.file.latin1(), parser.line_no);
            } else {
                if(scope_blocks.count() == 1) {
                    fprintf(stderr, "Braces mismatch %s:%d\n", parser.file.latin1(), parser.line_no);
                    return false;
                }
                debug_msg(1, "Project Parser: %s:%d : Leaving block %d", parser.file.latin1(),
                          parser.line_no, scope_blocks.count());
                ScopeBlock sb = scope_blocks.pop();
                if(sb.iterate)
                    sb.iterate->exec(this);
            }
        } else {
            var += *d;
        }
        d++;
    }
    var = var.trimmed();

    if(!else_line || (else_line && !scope_failed))
        scope_blocks.top().else_status = (!scope_failed ? ScopeBlock::TestFound : ScopeBlock::TestSeek);
    if(start_scope) {
        ScopeBlock next_scope(scope_failed);
        next_scope.iterate = iterator;
        if(iterator)
            next_scope.else_status = ScopeBlock::TestNone;
        else if(scope_failed)
            next_scope.else_status = ScopeBlock::TestSeek;
        else
            next_scope.else_status = ScopeBlock::TestFound;
        scope_blocks.push(next_scope);
        debug_msg(1, "Project Parser: %s:%d : Entering block %d (%d).", parser.file.latin1(),
                  parser.line_no, scope_blocks.count(), scope_failed);
    } else if(iterator) {
        iterator->parser.append(var+QString(d));
        bool ret = iterator->exec(this);
        delete iterator;
        return ret;
    }

    if((!scope_count && !var.isEmpty()) || (scope_count == 1 && else_line))
        scope_blocks.top().else_status = ScopeBlock::TestNone;
    if(!*d) {
        if(!var.trimmed().isEmpty())
            qmake_error_msg("Parse Error ('" + s + "')");
        return var.isEmpty(); // allow just a scope
    }

    SKIP_WS(d);
    for(; *d && op.indexOf('=') == -1; op += *(d++))
        ;
    op.replace(QRegExp("\\s"), "");

    SKIP_WS(d);
    QString vals(d); // vals now contains the space separated list of values
    int rbraces = vals.count('}'), lbraces = vals.count('{');
    if(scope_blocks.count() > 1 && rbraces - lbraces == 1) {
        debug_msg(1, "Project Parser: %s:%d : Leaving block %d", parser.file.latin1(),
                  parser.line_no, scope_blocks.count());
        ScopeBlock sb = scope_blocks.pop();
        if(sb.iterate)
            sb.iterate->exec(this);
        vals.truncate(vals.length()-1);
    } else if(rbraces != lbraces) {
        warn_msg(WarnParser, "Possible braces mismatch {%s} %s:%d",
                 vals.latin1(), parser.file.latin1(), parser.line_no);
    }
    if(scope_failed)
        return true; // oh well
#undef SKIP_WS

    doVariableReplace(vals, place);
    doVariableReplace(var, place);
    var = varMap(var); //backwards compatability
    if(!var.isEmpty() && Option::mkfile::do_preprocess) {
        static QString last_file("*none*");
        if(parser.file != last_file) {
            fprintf(stdout, "#file %s:%d\n", parser.file.latin1(), parser.line_no);
            last_file = parser.file;
        }
        fprintf(stdout, "%s %s %s\n", var.latin1(), op.latin1(), vals.latin1());
    }

    // vallist is the broken up list of values
    QStringList vallist = split_value_list(vals, (var == "DEPENDPATH" || var == "INCLUDEPATH"));
    if(!vallist.find("=").isEmpty())
        warn_msg(WarnParser, "Detected possible line continuation: {%s} %s:%d",
                 var.latin1(), parser.file.latin1(), parser.line_no);

    QStringList &varlist = place[var]; // varlist is the list in the symbol table
    debug_msg(1, "Project Parser: %s:%d :%s: :%s: (%s)", parser.file.latin1(), parser.line_no,
              var.latin1(), op.latin1(), vallist.isEmpty() ? "" : vallist.join(" :: ").latin1());

    // now do the operation
    if(op == "~=") {
        if(vals.length() < 4 || vals.at(0) != 's') {
            qmake_error_msg("~= operator only can handle s/// function ('" +
                            s + "')");
            return false;
        }
        QChar sep = vals.at(1);
        QStringList func = vals.split(sep);
        if(func.count() < 3 || func.count() > 4) {
            qmake_error_msg("~= operator only can handle s/// function ('" +
                s + "')");
            return false;
        }
        bool global = false, case_sense = true;
        if(func.count() == 4) {
            global = func[3].indexOf('g') != -1;
            case_sense = func[3].indexOf('i') == -1;
        }
        QRegExp regexp(func[1], case_sense ? Qt::CaseSensitive : Qt::CaseInsensitive);
        for(QStringList::Iterator varit = varlist.begin(); varit != varlist.end(); ++varit) {
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
                varlist.removeAll((*valit));
        }
    }
    if(var == "REQUIRES") // special case to get communicated to backends!
        doProjectCheckReqs(vallist, place);
    return true;
}

bool
QMakeProject::read(QTextStream &file, QMap<QString, QStringList> &place)
{
    bool ret = true;
    QString s, line;
    while(!file.atEnd()) {
        parser.line_no++;
        line = file.readLine().trimmed();
        int prelen = line.length();

        int hash_mark = line.indexOf("#");
        if(hash_mark != -1) //good bye comments
            line = line.left(hash_mark).trimmed();
        if(!line.isEmpty() && line.right(1) == "\\") {
            if(!line.startsWith("#")) {
                line.truncate(line.length() - 1);
                s += line + Option::field_sep;
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
    parser_info pi = parser;
    reset();

    QString filename = Option::fixPathToLocalOS(file);
    doVariableReplace(filename, place);
    bool ret = false, using_stdin = false;
    QFile qfile;
    if(!strcmp(filename, "-")) {
        qfile.setFileName("");
        ret = qfile.open(QIODevice::ReadOnly, stdin);
        using_stdin = true;
    } else {
        qfile.setFileName(filename);
        ret = qfile.open(QIODevice::ReadOnly);
    }
    if(ret) {
        parser_info pi = parser;
        parser.from_file = true;
        parser.file = filename;
        parser.line_no = 0;
        QTextStream t(&qfile);
        ret = read(t, place);
        if(!using_stdin)
            qfile.close();
        parser = pi;
    }
    parser = pi;
    if(scope_blocks.count() != 1)
        warn_msg(WarnParser, "%s: Unterminated conditional at end of file.",
                 file.latin1());
    return ret;
}

bool
QMakeProject::read(const QString &project, uchar cmd)
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
            base_vars["TEMPLATE_PREFIX"] = QStringList(Option::user_template_prefix);

        if(cmd & ReadCache && Option::mkfile::do_cache) {        // parse the cache
            int cache_depth = -1;
            QString qmake_cache = Option::mkfile::cachefile;
            if(qmake_cache.isEmpty())  { //find it as it has not been specified
                QString dir = QDir::convertSeparators(Option::output_dir);
                while(!QFile::exists((qmake_cache = dir + QDir::separator() + ".qmake.cache"))) {
                    dir = dir.left(dir.lastIndexOf(QDir::separator()));
                    if(dir.isEmpty() || dir.indexOf(QDir::separator()) == -1) {
                        qmake_cache = "";
                        break;
                    }
                    if(cache_depth == -1)
                        cache_depth = 1;
                    else
                        cache_depth++;
                }
            } else {
                QString abs_cache = QFileInfo(Option::mkfile::cachefile).absoluteDir().path();
                if(Option::output_dir.startsWith(abs_cache))
                    cache_depth = Option::output_dir.mid(abs_cache.length()).count('/');
            }
            if(!qmake_cache.isEmpty()) {
                if(read(qmake_cache, cache)) {
                    Option::mkfile::cachefile_depth = cache_depth;
                    Option::mkfile::cachefile = qmake_cache;
                    if(Option::mkfile::qmakespec.isEmpty() && !cache["QMAKESPEC"].isEmpty())
                        Option::mkfile::qmakespec = cache["QMAKESPEC"].first();
                }
            }
        }
        if(cmd & ReadConf) {             // parse mkspec
            QStringList mkspec_roots = qmake_mkspec_paths();
            debug_msg(2, "Looking for mkspec %s in (%s)", Option::mkfile::qmakespec.latin1(),
                      mkspec_roots.join("::").latin1());
            if(Option::mkfile::qmakespec.isEmpty()) {
                for(QStringList::Iterator it = mkspec_roots.begin(); it != mkspec_roots.end(); ++it) {
                    QString mkspec = (*it) + QDir::separator() + "default";
                    QFileInfo default_info(mkspec);
                    if(default_info.exists() && default_info.isSymLink()) {
                        Option::mkfile::qmakespec = mkspec;
                        break;
                    }
                }
                if(Option::mkfile::qmakespec.isEmpty()) {
                    fprintf(stderr, "QMAKESPEC has not been set, so configuration cannot be deduced.\n");
                    return false;
                }
            }

            if(QDir::isRelativePath(Option::mkfile::qmakespec)) {
                bool found_mkspec = false;
                for(QStringList::Iterator it = mkspec_roots.begin(); it != mkspec_roots.end(); ++it) {
                    QString mkspec = (*it) + QDir::separator() + Option::mkfile::qmakespec;
                    if(QFile::exists(mkspec)) {
                        found_mkspec = true;
                        Option::mkfile::qmakespec = mkspec;
                        break;
                    }
                }
                if(!found_mkspec) {
                    fprintf(stderr, "Could not find mkspecs for your QMAKESPEC after trying:\n\t%s\n",
                            mkspec_roots.join("\n\t").latin1());
                    return false;
                }
            }

            // parse qmake configuration
            while(Option::mkfile::qmakespec.endsWith(QString(QChar(QDir::separator()))))
                Option::mkfile::qmakespec.truncate(Option::mkfile::qmakespec.length()-1);
            QString spec = Option::mkfile::qmakespec + QDir::separator() + "qmake.conf";
            if(!QFile::exists(spec) &&
               QFile::exists(Option::mkfile::qmakespec + QDir::separator() + "tmake.conf"))
                spec = Option::mkfile::qmakespec + QDir::separator() + "tmake.conf";
            debug_msg(1, "QMAKESPEC conf: reading %s", spec.latin1());
            if(!read(spec, base_vars)) {
                fprintf(stderr, "Failure to read QMAKESPEC conf file %s.\n", spec.latin1());
                return false;
            }
            if(Option::mkfile::do_cache && !Option::mkfile::cachefile.isEmpty()) {
                debug_msg(1, "QMAKECACHE file: reading %s", Option::mkfile::cachefile.latin1());
                read(Option::mkfile::cachefile, base_vars);
            }
        }
    }

    vars = base_vars; // start with the base

    //get a default
    if(pfile != "-" && vars["TARGET"].isEmpty())
        vars["TARGET"].append(QFileInfo(pfile).baseName());

    //before commandline
    if(cmd & ReadCmdLine) {
        cfile = pfile;
        parser.file = "(internal)";
        parser.from_file = false;
        parser.line_no = 1; //really arg count now.. duh
        reset();
        for(QStringList::Iterator it = Option::before_user_vars.begin();
            it != Option::before_user_vars.end(); ++it) {
            if(!parse((*it), vars)) {
                fprintf(stderr, "Argument failed to parse: %s\n", (*it).latin1());
                return false;
            }
            parser.line_no++;
        }
    }

    //"magical" configs
    if(cmd & ReadConfigs && !Option::user_configs.isEmpty()) {
        parser.file = "(configs)";
        parser.from_file = false;
        parser.line_no = 1; //really arg count now.. duh
        parse("CONFIG += " + Option::user_configs.join(" "), vars);
    }

    if(cmd & ReadProFile) { // parse project file
        debug_msg(1, "Project file: reading %s", pfile.latin1());
        if(pfile != "-" && !QFile::exists(pfile) && !pfile.endsWith(".pro"))
            pfile += ".pro";
        if(!read(pfile, vars))
            return false;
    }

    if(cmd & ReadPostFiles) { // parse post files
        const QStringList l = vars["QMAKE_POST_INCLUDE_FILES"];
        for(QStringList::ConstIterator it = l.begin(); it != l.end(); ++it)
            read((*it), vars);
    }

    if(cmd & ReadCmdLine) {
        parser.file = "(internal)";
        parser.from_file = false;
        parser.line_no = 1; //really arg count now.. duh
        reset();
        for(QStringList::Iterator it = Option::after_user_vars.begin();
            it != Option::after_user_vars.end(); ++it) {
            if(!parse((*it), vars)) {
                fprintf(stderr, "Argument failed to parse: %s\n", (*it).latin1());
                return false;
            }
            parser.line_no++;
        }
    }

    if(pfile != "-" && vars["TARGET"].isEmpty())
        vars["TARGET"].append(QFileInfo(pfile).baseName());

    if(cmd & ReadConfigs && !Option::user_configs.isEmpty()) {
        parser.file = "(configs)";
        parser.from_file = false;
        parser.line_no = 1; //really arg count now.. duh
        parse("CONFIG += " + Option::user_configs.join(" "), base_vars);
    }

    if(cmd & ReadFeatures) {
        debug_msg(1, "Processing CONFIG features: %s", vars["CONFIG"].join("::").latin1());
        doProjectInclude("default", true, vars); //find the default CONFIG line
        while(1) {
            const QStringList &configs = vars["CONFIG"];
            int i = configs.size()-1;
            for( ; i >= 0; i--) {
                if(doProjectInclude(configs[i], true, vars) == IncludeSuccess)
                    break;
            }
            if(i == -1)
                break;
        }
    }

    // now let the user override the template from an option..
    if(!Option::user_template.isEmpty()) {
        QString s;
        if (!vars["TEMPLATE"].isEmpty())
	    s = vars["TEMPLATE"].first();
        debug_msg(1, "Overriding TEMPLATE (%s) with: %s", s.latin1(),
                  Option::user_template.latin1());
        vars["TEMPLATE"].clear();
        vars["TEMPLATE"].append(Option::user_template);
    }

    QStringList &templ = vars["TEMPLATE"];
    if(templ.isEmpty())
        templ.append(QString("app"));
    else if(vars["TEMPLATE"].first().endsWith(".t"))
        templ = QStringList(templ.first().left(templ.first().length() - 2));
    if(!Option::user_template_prefix.isEmpty())
        templ.first().prepend(Option::user_template_prefix);

    QString test_version = qgetenv("QTESTVERSION");
    if(!test_version.isEmpty()) {
        QString s = vars["TARGET"].first();
        if(s == "qt" || s == "qt-mt" || s == "qte" || s == "qte-mt") {
            QString &ver = vars["VERSION"].first();
//            fprintf(stderr,"Current QT version number: " + ver + "\n");
            if(!ver.isEmpty() && ver != test_version) {
                ver = test_version;
                fprintf(stderr,"Changed QT version number to " + test_version + "!\n");
            }
        }
    }
    return true;
}

bool
QMakeProject::isActiveConfig(const QString &x, bool regex, QMap<QString, QStringList> *place)
{
    if(x.isEmpty())
        return true;

    //magic types for easy flipping
    if(x == "true")
        return true;
    else if(x == "false")
        return false;

    //mkspecs
    if((Option::target_mode == Option::TARG_MACX_MODE || Option::target_mode == Option::TARG_QNX6_MODE ||
        Option::target_mode == Option::TARG_UNIX_MODE) && x == "unix")
        return true;
    else if(Option::target_mode == Option::TARG_MACX_MODE && x == "macx")
        return true;
    else if(Option::target_mode == Option::TARG_QNX6_MODE && x == "qnx6")
        return true;
    else if(Option::target_mode == Option::TARG_MAC9_MODE && x == "mac9")
        return true;
    else if((Option::target_mode == Option::TARG_MAC9_MODE || Option::target_mode == Option::TARG_MACX_MODE) &&
            x == "mac")
        return true;
    else if(Option::target_mode == Option::TARG_WIN_MODE && x == "win32")
        return true;
    QRegExp re(x, Qt::CaseSensitive, QRegExp::Wildcard);
    QString spec = Option::mkfile::qmakespec.right(Option::mkfile::qmakespec.length() -
                                                   (Option::mkfile::qmakespec.lastIndexOf(QDir::separator())+1));
    if((regex && re.exactMatch(spec)) || (!regex && spec == x))
        return true;
#ifdef Q_OS_UNIX
    else if(spec == "default") {
        static char *buffer = NULL;
        if(!buffer)
            buffer = (char *)malloc(1024);
        int l = readlink(Option::mkfile::qmakespec, buffer, 1024);
        if(l != -1) {
            buffer[l] = '\0';
            QString r = buffer;
            if(r.lastIndexOf('/') != -1)
                r = r.mid(r.lastIndexOf('/') + 1);
            if((regex && re.exactMatch(r)) || (!regex && r == x))
                return true;
        }
    }
#endif

    //simple matching
    QStringList &configs = (place ? (*place)["CONFIG"] : vars["CONFIG"]);
    for(QStringList::Iterator it = configs.begin(); it != configs.end(); ++it) {
        if(((regex && re.exactMatch((*it))) || (!regex && (*it) == x)) && re.exactMatch((*it)))
            return true;
    }
    return false;
}

bool
QMakeProject::doProjectTest(const QString& func, const QString &params, QMap<QString, QStringList> &place)
{
    return doProjectTest(func.trimmed(), split_arg_list(params), place);
}

/* If including a feature it will look in:

   1) environment variable QMAKEFEATURES (as separated by colons)
   2) property variable QMAKEFEATURES (as separated by colons)
   3) <project_root> (where .qmake.cache lives) + FEATURES_DIR
   4) environment variable QMAKEPATH (as separated by colons) + /mkspecs/FEATURES_DIR
   5) your QMAKESPEC/features dir
   6) your data_install/mkspecs/FEATURES_DIR
   7) environment variable QTDIR/mkspecs/FEATURES_DIR

   FEATURES_DIR is defined as:

   1) features/(unix|win32|macx)/
   2) features/
*/
QMakeProject::IncludeStatus
QMakeProject::doProjectInclude(QString file, bool feature, QMap<QString, QStringList> &place,
                               const QString &seek_var)
{
    if(feature) {
        if(!file.endsWith(Option::prf_ext))
            file += Option::prf_ext;
        if(file.indexOf(Option::dir_sep) == -1 || !QFile::exists(file)) {
            bool found = false;
            QStringList concat;
            {
                const QString base_concat = QDir::separator() + QString("features");
                switch(Option::target_mode) {
                case Option::TARG_MACX_MODE:                     //also a unix
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
            if(const char *mkspec_path = qgetenv("QMAKEFEATURES")) {
#ifdef Q_OS_WIN
                QStringList lst = QString(mkspec_path).split(';');
                for(QStringList::Iterator it = lst.begin(); it != lst.end(); ++it)
                    feature_roots += (*it).split(':');
#else
                feature_roots += QString(mkspec_path).split(':');
#endif
            }
            if(prop) {
#ifdef Q_OS_WIN
                QStringList lst = prop->value("QMAKEFEATURES").split(';');
                for(QStringList::Iterator it = lst.begin(); it != lst.end(); ++it)
                    feature_roots += (*it).split(':');
#else
                feature_roots += prop->value("QMAKEFEATURES").split(':');
#endif
            }
            if(!Option::mkfile::cachefile.isEmpty()) {
                QString path;
                int last_slash = Option::mkfile::cachefile.lastIndexOf(Option::dir_sep);
                if(last_slash != -1)
                    path = Option::fixPathToLocalOS(Option::mkfile::cachefile.left(last_slash));
                for(QStringList::Iterator concat_it = concat.begin();
                    concat_it != concat.end(); ++concat_it)
                    feature_roots << (path + (*concat_it));
            }
            if(const char *qmakepath = qgetenv("QMAKEPATH")) {
#ifdef Q_OS_WIN
                QStringList lst = QString(qmakepath).split(';');
                for(QStringList::Iterator it = lst.begin(); it != lst.end(); ++it) {
                    QStringList lst2 = (*it).split(':');
                    for(QStringList::Iterator it2 = lst2.begin(); it2 != lst2.end(); ++it2) {
                        for(QStringList::Iterator concat_it = concat.begin();
                            concat_it != concat.end(); ++concat_it)
                            feature_roots << ((*it2) + mkspecs_concat + (*concat_it));
                    }
                }
#else
                QStringList lst = QString(qmakepath).split(':');
                for(QStringList::Iterator it = lst.begin(); it != lst.end(); ++it) {
                    for(QStringList::Iterator concat_it = concat.begin();
                        concat_it != concat.end(); ++concat_it)
                        feature_roots << ((*it) + mkspecs_concat + (*concat_it));
                }
#endif
            }
            feature_roots << Option::mkfile::qmakespec + QDir::separator() + "features";
            if(const char *qtdir = qgetenv("QTDIR")) {
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
            int start_root = 0;
            if(parser.from_file) {
                QFileInfo currFile(QFileInfo(parser.file).canonicalFilePath());
                for(int root = 0; root < feature_roots.size(); ++root) {
                    QString prf(feature_roots[root] + QDir::separator() + file);
                    QFileInfo prfFile(QFileInfo(prf).canonicalFilePath());
                    if(prfFile == currFile) {
                        start_root = root+1;
                        break;
                    }
                }
            }
            for(int root = start_root; root < feature_roots.size(); ++root) {
                QString prf(feature_roots[root] + QDir::separator() + file);
                if(QFile::exists(prf)) {
                    found = true;
                    file = prf;
                    break;
                }
            }
            if(!found)
                return IncludeNoExist;
            if(place["QMAKE_INTERNAL_INCLUDED_FEATURES"].indexOf(file) != -1)
                return IncludeFeatureAlreadyLoaded;
            place["QMAKE_INTERNAL_INCLUDED_FEATURES"].append(file);
        }
    }
    if(QDir::isRelativePath(file)) {
        QStringList include_roots;
        include_roots << Option::output_dir;
        if(parser.from_file) {
            QString pfilewd = QFileInfo(parser.file).path();
            if(pfilewd.isEmpty())
                include_roots << pfilewd;
        }
        if(Option::output_dir != QDir::currentPath())
            include_roots << QDir::currentPath();
        for(int root = 0; root < include_roots.size(); ++root) {
            if(QFile::exists(include_roots[root] + QDir::separator() + file)) {
                file = include_roots[root] + QDir::separator() + file;
                break;
            }
        }
    }
    if(!QFile::exists(file))
        return IncludeNoExist;

    if(Option::mkfile::do_preprocess) //nice to see this first..
        fprintf(stderr, "#switching file %s(%s) - %s:%d\n", feature ? "load" : "include", file.latin1(),
                parser.file.latin1(), parser.line_no);
    debug_msg(1, "Project Parser: %s'ing file %s.", feature ? "load" : "include", file.latin1());
    QString orig_file = file;
    int di = file.lastIndexOf(Option::dir_sep);
    QDir sunworkshop42workaround = QDir::current();
    QString oldpwd = sunworkshop42workaround.currentPath();
    if(di != -1) {
        if(!QDir::setCurrent(file.left(file.lastIndexOf(Option::dir_sep)))) {
            fprintf(stderr, "Cannot find directory: %s\n", file.left(di).latin1());
            return IncludeFailure;
        }
        file = file.right(file.length() - di - 1);
    }
    parser_info pi = parser;
    QStack<ScopeBlock> sc = scope_blocks;
    bool parsed = false;
    if(!seek_var.isNull()) {
        QMap<QString, QStringList> tmp;
        if((parsed = read(file.latin1(), tmp)))
            place[seek_var] += tmp[seek_var];
    } else {
        parsed = read(file.latin1(), place);
    }
    if(parsed)
        place["QMAKE_INTERNAL_INCLUDED_FILES"].append(orig_file);
    else
        warn_msg(WarnParser, "%s:%d: Failure to include file %s.",
                 pi.file.latin1(), pi.line_no, orig_file.latin1());
    parser = pi;
    scope_blocks = sc;
    QDir::setCurrent(oldpwd);
    if(!parsed)
        return IncludeParseFailure;
    return IncludeSuccess;
}

bool
QMakeProject::doProjectTest(const QString& func, QStringList args, QMap<QString, QStringList> &place)
{
    for(QStringList::Iterator arit = args.begin(); arit != args.end(); ++arit) {
        (*arit) = (*arit).trimmed(); // blah, get rid of space
        doVariableReplace((*arit), place);
    }
    debug_msg(1, "Running project test: %s(%s)", func.latin1(), args.join("::").latin1());

    if(func == "requires") {
        return doProjectCheckReqs(args, place);
    } else if(func == "equals" || func == "isEqual") {
        if(args.count() != 2) {
            fprintf(stderr, "%s:%d: %s(variable, value) requires two arguments.\n", parser.file.latin1(),
                    parser.line_no, func.latin1());
            return false;
        }
        return place[args[0]].join(QString(Option::field_sep)) == args[1];
    } else if(func == "exists") {
        if(args.count() != 1) {
            fprintf(stderr, "%s:%d: exists(file) requires one argument.\n", parser.file.latin1(),
                    parser.line_no);
            return false;
        }
        QString file = args.first();
        file = Option::fixPathToLocalOS(file);

        if(QFile::exists(file))
            return true;
        //regular expression I guess
        QString dirstr = QDir::currentPath();
        int slsh = file.lastIndexOf(Option::dir_sep);
        if(slsh != -1) {
            dirstr = file.left(slsh+1);
            file = file.right(file.length() - slsh - 1);
        }
        return QDir(dirstr).entryList(QStringList(file)).count();
    } else if(func == "unset") {
        if(args.count() != 1) {
            fprintf(stderr, "%s:%d: unset(variable) requires one argument.\n", parser.file.latin1(),
                    parser.line_no);
            return false;
        }
        if(!place.contains(args[0]))
            return false;
        place.remove(args[0]);
        return true;
    } else if(func == "eval") {
        if(args.count() < 1 && 0) {
            fprintf(stderr, "%s:%d: eval(project) requires one argument.\n", parser.file.latin1(),
                    parser.line_no);
            return false;
        }
        QString project = args.join(" ");
        parser_info pi = parser;
        parser.from_file = false;
        parser.file = "(eval)";
        parser.line_no = 0;
        QTextStream t(&project, QIODevice::ReadOnly);
        bool ret = read(t, place);
        parser = pi;
        return ret;
    } else if(func == "CONFIG") {
        if(args.count() < 1 || args.count() > 2) {
            fprintf(stderr, "%s:%d: CONFIG(config) requires one argument.\n", parser.file.latin1(),
                    parser.line_no);
            return false;
        }
        if(args.count() == 1)
            return isActiveConfig(args[0]);
        const QStringList mutuals = args[1].split('|');
        const QStringList &configs = place["CONFIG"];
        for(int i = configs.size()-1; i >= 0; i--) {
            for(int mut = 0; mut < mutuals.count(); mut++) {
                if(configs[i] == mutuals[mut].trimmed())
                    return (configs[i] == args[0]);
            }
        }
        return false;
    } else if(func == "system") {
        if(args.count() != 1) {
            fprintf(stderr, "%s:%d: system(exec) requires one argument.\n", parser.file.latin1(),
                    parser.line_no);
            return false;
        }
#ifdef Q_OS_UNIX
        for(QMap<QString, QStringList>::ConstIterator it = place.begin();
            it != place.end(); ++it) {
            if(!it.key().startsWith("."))
                putenv(const_cast<char*>(QString(Option::sysenv_mod + it.key() + '=' + it.value().join(" ")).ascii()));
        }
#endif
        bool ret = system(args.first().latin1()) == 0;
#ifdef Q_OS_UNIX
        for(QMap<QString, QStringList>::ConstIterator it = place.begin();
            it != place.end(); ++it) {
            if(!it.key().startsWith("."))
                putenv(const_cast<char*>(QString(Option::sysenv_mod + it.key()).ascii()));
        }
#endif
        return ret;
    } else if(func == "return") {
        if(function_blocks.isEmpty()) {
            fprintf(stderr, "%s:%d unexpected return()\n",
                    parser.file.latin1(), parser.line_no);
        } else {
            FunctionBlock *f = function_blocks.top();
            f->cause_return = true;
            if(args.count() >= 1)
                f->return_value = args[0];
        }
        return true;
    } else if(func == "break") {
        if(!iterator)
            fprintf(stderr, "%s:%d unexpected break()\n",
                    parser.file.latin1(), parser.line_no);
        else
            iterator->cause_break = true;
        return true;
    } else if(func == "next") {
        if(!iterator)
            fprintf(stderr, "%s:%d unexpected next()\n",
                    parser.file.latin1(), parser.line_no);
        else
            iterator->cause_next = true;
        return true;
    } else if(func == "contains") {
        if(args.count() != 2) {
            fprintf(stderr, "%s:%d: contains(var, val) requires two arguments.\n", parser.file.latin1(),
                    parser.line_no);
            return false;
        }
        QRegExp regx(args[1]);
        const QStringList &l = place[args[0]];
        for(QStringList::ConstIterator it = l.begin(); it != l.end(); ++it) {
            if(regx.exactMatch((*it)) || (*it) == args[1])
                return true;
        }
        return false;
    } else if(func == "infile") {
        if(args.count() < 2 || args.count() > 3) {
            fprintf(stderr, "%s:%d: infile(file, var, val) requires at least 2 arguments.\n",
                    parser.file.latin1(), parser.line_no);
            return false;
        }
        QMakeProject proj;
        QString file = args[0];
        fixEnvVariables(file);
        int di = file.lastIndexOf(Option::dir_sep);
        QDir sunworkshop42workaround = QDir::current();
        QString oldpwd = sunworkshop42workaround.currentPath();
        if(di != -1) {
            if(!QDir::setCurrent(file.left(file.lastIndexOf(Option::dir_sep)))) {
                fprintf(stderr, "Cannot find directory: %s\n", file.left(di).latin1());
                return false;
            }
            file = file.right(file.length() - di - 1);
        }
        parser_info pi = parser;
        bool ret = !proj.read(file);
        parser = pi;
        if(ret) {
            fprintf(stderr, "Error processing project file: %s\n", file.latin1());
            QDir::setCurrent(oldpwd);
            return false;
        }
        if(args.count() == 2) {
            ret = !proj.isEmpty(args[1]);
        } else {
            QRegExp regx(args[2]);
            QStringList &l = proj.values(args[1]);
            for(QStringList::ConstIterator it = l.begin(); it != l.end(); ++it) {
                if(regx.exactMatch((*it)) || (*it) == args[2]) {
                    ret = true;
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
            return false;
        }
        return place[args[0]].count() == args[1].toInt();
    } else if(func == "isEmpty") {
        if(args.count() != 1) {
            fprintf(stderr, "%s:%d: isEmpty(var) requires one argument.\n", parser.file.latin1(),
                    parser.line_no);
            return false;
        }
        return place[args[0]].isEmpty();
    } else if(func == "include" || func == "load") {
        QString seek_var;
        const bool include_statement = (func == "include");
        bool ignore_error = include_statement;
        if(args.count() == 2) {
            if(func == "include") {
                seek_var = args[1];
            } else {
                QString sarg = args[1];
                ignore_error = (sarg.toLower() == "true" || sarg.toInt());
            }
        } else if(args.count() != 1) {
            QString func_desc = "load(feature)";
            if(include_statement)
                func_desc = "include(file)";
            fprintf(stderr, "%s:%d: %s requires one argument.\n", parser.file.latin1(),
                    parser.line_no, func_desc.latin1());
            return false;
        }
        QString file = args.first();
        file = Option::fixPathToLocalOS(file);
        IncludeStatus stat = doProjectInclude(file, !include_statement, place, seek_var);
        if(stat == IncludeFeatureAlreadyLoaded) {
            warn_msg(WarnParser, "%s:%d: Duplicate of loaded feature %s",
                     parser.file.latin1(), parser.line_no, file.latin1());
        } else if(stat == IncludeNoExist && include_statement) {
            warn_msg(WarnParser, "%s:%d: Unable to find file for inclusion %s",
                     parser.file.latin1(), parser.line_no, file.latin1());
        } else if(stat >= IncludeFailure) {
            if(!ignore_error) {
                printf("Project LOAD(): Feature %s cannot be found.\n", file.latin1());
                if (!ignore_error)
                    exit(3);
            }
            return false;
        }
        return true;
    } else if(func == "error" || func == "message" || func == "warning") {
        if(args.count() != 1) {
            fprintf(stderr, "%s:%d: %s(message) requires one argument.\n", parser.file.latin1(),
                    parser.line_no, func.latin1());
            return false;
        }
        QString msg = args.first();
        fixEnvVariables(msg);
        fprintf(stderr, "Project %s: %s\n", func.toUpper().latin1(), msg.latin1());
        if(func == "error")
            exit(2);
        return true;
    } else if(FunctionBlock *defined = testFunctions[func]) {
        QString ret;
        defined->exec(this, args, ret);
        if(ret.isEmpty()) {
            return true;
        } else {
            bool ok;
            int val = ret.toInt(&ok);
            if(ok)
                return val;
            else
                return (ret.toLower() == "true");
        }
        return false;
    } else {
        fprintf(stderr, "%s:%d: Unknown test function: %s\n", parser.file.latin1(), parser.line_no,
                func.latin1());
    }
    return false;
}

bool
QMakeProject::doProjectCheckReqs(const QStringList &deps, QMap<QString, QStringList> &place)
{
    bool ret = false;
    for(QStringList::ConstIterator it = deps.begin(); it != deps.end(); ++it) {
        QString chk = remove_quotes((*it).trimmed());
        if(chk.isEmpty())
            continue;
        bool invert_test = (chk.left(1) == "!");
        if(invert_test)
            chk = chk.right(chk.length() - 1);

        bool test=false;
        int lparen = chk.indexOf('(');
        if(lparen != -1) { // if there is an lparen in the chk, it IS a function
            int rparen = chk.indexOf(')', lparen);
            if(rparen == -1) {
                QByteArray error;
                error.reserve(256);
                sprintf(error.data(), "Function (in REQUIRES) missing right paren: %s",
                        chk.latin1());
                qmake_error_msg(error);
            } else {
                QString func = chk.left(lparen);
                test = doProjectTest(func, chk.mid(lparen+1, rparen - lparen - 1), place);
            }
        } else {
            test = isActiveConfig(chk, true, &place);
        }
        if(invert_test) {
            chk.prepend("!");
            test = !test;
        }
        if(!test) {
            debug_msg(1, "Project Parser: %s:%d Failed test: REQUIRES = %s",
                      parser.file.latin1(), parser.line_no, chk.latin1());
            place["QMAKE_FAILED_REQUIREMENTS"].append(chk);
            ret = false;
        }
    }
    return ret;
}


QString
QMakeProject::doVariableReplace(QString &str, const QMap<QString, QStringList> &place)
{
    for(int var_begin, var_last=0; (var_begin = str.indexOf("$$", var_last)) != -1; var_last = var_begin) {
        if(var_begin >= (int)str.length() + 2) {
            break;
        } else if(var_begin != 0 && str[var_begin-1] == '\\') {
            str.replace(var_begin-1, 1, "");
            var_begin += 1;
            continue;
        }

        int var_incr = var_begin + 2;
        bool in_braces = false, as_env = false, as_prop = false;
        if(str[var_incr] == '{') {
            in_braces = true;
            var_incr++;
            while(var_incr < (int)str.length() &&
                  (str[var_incr] == ' ' || str[var_incr] == '\t' || str[var_incr] == '\n'))
                var_incr++;
        }
        if(str[var_incr] == '(') {
            as_env = true;
            var_incr++;
        } else if(str[var_incr] == '[') {
            as_prop = true;
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
                         str.mid(var_begin, qMax(var_incr - var_begin,
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
                         str.mid(var_begin, qMax(var_incr - var_begin, int(str.length()))).latin1(), str.latin1());
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
                     str.mid(var_begin, qMax(var_incr - var_begin,
                                             (int)str.length())).latin1(), str.latin1());
            var_begin += var_incr;
            continue;
        } else if(in_braces) {
            var_incr++;
        }

        QString replacement;
        if(as_env) {
            replacement = qgetenv(val);
        } else if(as_prop) {
            if(prop)
                replacement = prop->value(val);
        } else if(args.isEmpty()) {
            if(val == "LITERAL_WHITESPACE") { //a real space in a token)
                replacement = "\t";
            } else if(val == "LITERAL_DOLLAR") { //a real $ (not a variable replace)
                replacement = "$";
            } else if(val == "LITERAL_HASH") { //a real # (ie not a comment)
                replacement = "#";
            } else if(val == "PWD") { //current working dir (of _FILE_)
                replacement = QDir::currentPath();
            } else if(val == "DIR_SEPARATOR") {
                replacement = Option::dir_sep;
            } else if(val == "_LINE_") { //parser line number
                replacement = QString::number(parser.line_no);
            } else if(val == "_FILE_") { //parser file
                replacement = parser.file;
            } else if(val == "_DATE_") { //current date/time
                replacement = QDateTime::currentDateTime().toString();
            } else if(val == "_QMAKE_CACHE_") { //the .qmake.cache loaded
                if(Option::mkfile::do_cache)
                    replacement = Option::mkfile::cachefile;
            } else {
                replacement = place[varMap(val)].join(QString(Option::field_sep));
            }
        } else {
            QStringList arg_list = split_arg_list(doVariableReplace(args, place));
            debug_msg(1, "Running function: %s(%s)", val.latin1(), arg_list.join("::").latin1());
            if(val.toLower() == "member") {
                if(arg_list.count() < 1 || arg_list.count() > 3) {
                    fprintf(stderr, "%s:%d: member(var, start, end) requires three arguments.\n",
                            parser.file.latin1(), parser.line_no);
                } else {
                    bool ok = true;
                    const QStringList &var = place[varMap(arg_list.first())];
                    int start = 0, end = 0;
                    if(arg_list.count() >= 2) {
                        QString start_str = arg_list[1];
                        start = start_str.toInt(&ok);
                        if(!ok) {
                            if(arg_list.count() == 2) {
                                int dotdot = start_str.indexOf("..");
                                if(dotdot != -1) {
                                    start = start_str.left(dotdot).toInt(&ok);
                                    if(ok)
                                        end = start_str.mid(dotdot+2).toInt(&ok);
                                }
                            }
                            if(!ok)
                                fprintf(stderr, "%s:%d: member() argument 2 (start) '%s' invalid.\n",
                                        parser.file.latin1(), parser.line_no, start_str.latin1());
                        } else {
                            end = start;
                            if(arg_list.count() == 3)
                                end = arg_list[2].toInt(&ok);
                            if(!ok)
                                fprintf(stderr, "%s:%d: member() argument 3 (end) '%s' invalid.\n",
                                        parser.file.latin1(), parser.line_no, arg_list[2].latin1());
                        }
                    }
                    if(ok) {
                        if(start < 0)
                            start += var.count();
                        if(end < 0)
                            end += var.count();
                        if(start < end) {
                            for(int i = start; i <= end && (int)var.count() >= i; i++) {
                                if(!replacement.isEmpty())
                                    replacement += Option::field_sep;
                                replacement += var[i];
                            }
                        } else {
                            for(int i = start; i >= end && (int)var.count() >= i && i >= 0; i--) {
                                if(!replacement.isEmpty())
                                    replacement += Option::field_sep;
                                replacement += var[i];
                            }
                        }
                    }
                }
            } else if(val.toLower() == "first" || val.toLower() == "last") {
                if(arg_list.count() != 1) {
                    fprintf(stderr, "%s:%d: %s(var) requires one argument.\n",
                            parser.file.latin1(), parser.line_no, val.latin1());
                } else {
                    const QStringList &var = place[varMap(arg_list.first())];
                    if(!var.isEmpty()) {
                        if(val.toLower() == "first")
                            replacement = var[0];
                        else
                            replacement = var[var.size()-1];
                    }
                }
            } else if(val.toLower() == "cat") {
                if(arg_list.count() < 1 || arg_list.count() > 2) {
                    fprintf(stderr, "%s:%d: cat(file) requires one arguments.\n",
                            parser.file.latin1(), parser.line_no);
                } else {
                    QString file = arg_list[0];
                    file = Option::fixPathToLocalOS(file);

                    bool singleLine = true;
                    if(arg_list.count() > 1) 
                        singleLine = (arg_list[1].toLower() == "true");

                    QFile qfile(file);
                    if(qfile.open(QIODevice::ReadOnly)) {
                        QTextStream stream(&qfile);
                        while(!stream.atEnd()) {
                            replacement += stream.readLine().trimmed();
                            if(!singleLine)
                                replacement += "\n";
                        }
                        qfile.close();
                    }
                }
            } else if(val.toLower() == "fromfile") {
                if(arg_list.count() != 2) {
                    fprintf(stderr, "%s:%d: fromfile(file, variable) requires two arguments.\n",
                            parser.file.latin1(), parser.line_no);
                } else {
                    QString file = arg_list[0], seek_var = arg_list[1];
                    file = Option::fixPathToLocalOS(file);

                    QMap<QString, QStringList> tmp;
                    if(doProjectInclude(file, false, tmp, seek_var) == IncludeSuccess)
                        replacement = tmp[seek_var].join(QString(Option::field_sep));
                }
            } else if(val.toLower() == "eval") {
                for(QStringList::ConstIterator arg_it = arg_list.begin();
                    arg_it != arg_list.end(); ++arg_it) {
                    if(!replacement.isEmpty())
                        replacement += Option::field_sep;
                    replacement += place[(*arg_it)].join(QString(Option::field_sep));
                }
            } else if(val.toLower() == "list") {
                static int x = 0;
                replacement.sprintf(".QMAKE_INTERNAL_TMP_VAR_%d", x++);
                QStringList &lst = (*((QMap<QString, QStringList>*)&place))[replacement];
                lst.clear();
                for(QStringList::ConstIterator arg_it = arg_list.begin();
                    arg_it != arg_list.end(); ++arg_it)
                    lst += split_value_list((*arg_it));
            } else if(val.toLower() == "sprintf") {
                if(arg_list.count() < 1) {
                    fprintf(stderr, "%s:%d: sprintf(format, ...) requires one argument.\n",
                            parser.file.latin1(), parser.line_no);
                } else {
                    replacement = arg_list.first();
                    QStringList::Iterator arg_it = arg_list.begin();
                    ++arg_it;
                    for(; arg_it != arg_list.end(); ++arg_it)
                        replacement = replacement.arg((*arg_it));
                }
            } else if(val.toLower() == "join") {
                if(arg_list.count() < 1 || arg_list.count() > 4) {
                    fprintf(stderr, "%s:%d: join(var, glue, before, after) requires four"
                            "arguments.\n", parser.file.latin1(), parser.line_no);
                } else {
                    QString glue, before, after;
                    if(arg_list.count() >= 2)
                        glue = arg_list[1];
                    if(arg_list.count() >= 3)
                        before = arg_list[2];
                    if(arg_list.count() == 4)
                        after = arg_list[3];
                    const QStringList &var = place[varMap(arg_list.first())];
                    if(!var.isEmpty())
                        replacement = before + var.join(glue) + after;
                }
            } else if(val.toLower() == "split") {
                if(arg_list.count() < 2 || arg_list.count() > 3) {
                    fprintf(stderr, "%s:%d split(var, sep, join) requires three arguments\n",
                            parser.file.latin1(), parser.line_no);
                } else {
                    QString sep = arg_list[1], join = QString(Option::field_sep);
                    if(arg_list.count() == 3)
                        join = arg_list[2];
                    QStringList var = place[varMap(arg_list.first())];
                    for(QStringList::Iterator vit = var.begin(); vit != var.end(); ++vit) {
                        QStringList lst = (*vit).split(sep);
                        for(QStringList::Iterator spltit = lst.begin(); spltit != lst.end(); ++spltit) {
                            if(!replacement.isEmpty())
                                replacement += join;
                            replacement += (*spltit);
                        }
                    }
                }
            } else if(val.toLower() == "basename" || val.toLower() == "dirname" || val.toLower() == "section") {
                QString sep, var;
                int beg=0, end=-1;;
                if(val.toLower() == "section") {
                    if(arg_list.count() != 3 && arg_list.count() != 4) {
                        fprintf(stderr, "%s:%d section(var, sep, begin, end) requires three argument\n",
                                parser.file.latin1(), parser.line_no);
                    } else {
                        var = arg_list[0];
                        sep = arg_list[1];
                        beg = arg_list[2].toInt();
                        if(arg_list.count() == 4)
                            end = arg_list[3].toInt();
                    }
                } else {
                    if(arg_list.count() != 1) {
                        fprintf(stderr, "%s:%d %s(var) requires one argument\n",
                                parser.file.latin1(), parser.line_no, val.toLower().latin1());
                    } else {
                        var = arg_list[0];
                        sep = Option::dir_sep;
                        if(val.toLower() == "dirname")
                            end = -2;
                        else
                            beg = -1;
                    }
                }
                if(!var.isNull()) {
                    const QStringList &l = place[varMap(var)];
                    for(QStringList::ConstIterator it = l.begin(); it != l.end(); ++it) {
                        if(!replacement.isEmpty())
                            replacement += Option::field_sep;
                        replacement += (*it).section(sep, beg, end);
                    }
                }
            } else if(val.toLower() == "find") {
                if(arg_list.count() != 2) {
                    fprintf(stderr, "%s:%d find(var, str) requires two arguments\n",
                            parser.file.latin1(), parser.line_no);
                } else {
                    QRegExp regx(arg_list[1]);
                    const QStringList &var = place[varMap(arg_list.first())];
                    for(QStringList::ConstIterator vit = var.begin();
                        vit != var.end(); ++vit) {
                        if(regx.indexIn(*vit) != -1) {
                            if(!replacement.isEmpty())
                                replacement += Option::field_sep;
                            replacement += (*vit);
                        }
                    }
                }
            } else if(val.toLower() == "system") {
                if(arg_list.count() < 1 || arg_list.count() > 2) {
                    fprintf(stderr, "%s:%d system(execut) requires one argument\n",
                            parser.file.latin1(), parser.line_no);
                } else {
#ifdef Q_OS_UNIX
                    for(QMap<QString, QStringList>::ConstIterator it = place.begin();
                        it != place.end(); ++it) {
                        if(!it.key().startsWith("."))
                            putenv(const_cast<char*>(QString(Option::sysenv_mod + it.key() + '=' + it.value().join(" ")).ascii()));
                    }
#endif

                    char buff[256];
                    FILE *proc = QT_POPEN(arg_list[0].latin1(), "r");
                    bool singleLine = true;
                    if(arg_list.count() > 1) 
                        singleLine = (arg_list[1].toLower() == "true");
                    while(proc && !feof(proc)) {
                        int read_in = (int)fread(buff, 1, 255, proc);
                        if(!read_in)
                            break;
                        for(int i = 0; i < read_in; i++) {
                            if((singleLine && buff[i] == '\n') || buff[i] == '\t')
                                buff[i] = ' ';
                        }
                        buff[read_in] = '\0';
                        replacement += buff;
                    }
#ifdef Q_OS_UNIX
                    for(QMap<QString, QStringList>::ConstIterator it = place.begin();
                        it != place.end(); ++it) {
                        if(!it.key().startsWith("."))
                            putenv(const_cast<char*>(QString(Option::sysenv_mod
                                                     + it.key()).ascii()));
                    }
#endif
                }
            } else if(val.toLower() == "unique") {
                if(arg_list.count() != 1) {
                    fprintf(stderr, "%s:%d unique(var) requires one argument\n",
                            parser.file.latin1(), parser.line_no);
                } else {
                    QStringList uniq;
                    const QStringList &var = place[varMap(arg_list.first())];
                    for(int i = 0; i < var.count(); i++) {
                        if(!uniq.contains(var[i]))
                            uniq.append(var[i]);
                    }
                    replacement = uniq.join(" ");
                }
            } else if(val.toLower() == "quote") {
                replacement = arg_list.join(" ");
                replacement = replacement.replace("\\n", "\n");
                replacement = replacement.replace("\\t", "\t");
                replacement = replacement.replace("\\r", "\r");
            } else if(val.toLower() == "upper" || val.toLower() == "lower") {
                replacement = arg_list.join(QString(Option::field_sep));
                if(val.toLower() == "upper")
                    replacement = replacement.toUpper();
                else
                    replacement = replacement.toLower();
            } else if(val.toLower() == "files") {
                if(arg_list.count() != 1 && arg_list.count() != 2) {
                    fprintf(stderr, "%s:%d files(pattern) requires one argument\n",
                            parser.file.latin1(), parser.line_no);
                } else {
                    bool recursive = false;
                    if(arg_list.count() == 2)
                        recursive = (arg_list[1].toLower() == "true" || arg_list[1].toInt());
                    QStringList dirs;
                    QString r = Option::fixPathToLocalOS(arg_list[0]);
                    int slash = r.lastIndexOf(QDir::separator());
                    if(slash != -1) {
                        dirs.append(r.left(slash));
                        r = r.mid(slash+1);
                    } else {
                        dirs.append(QDir::currentPath());
                    }

                    const QRegExp regex(r, Qt::CaseSensitive, QRegExp::Wildcard);
                    for(int d = 0; d < dirs.count(); d++) {
                        QString dir = dirs[d];
                        if(!dir.endsWith(Option::dir_sep))
                           dir += "/";
                        QDir qdir(dir);
                        for(int i = 0; i < (int)qdir.count(); ++i) {
                            if(qdir[i] == "." || qdir[i] == "..")
                                continue;
                            QString fname = dir + qdir[i];
                            if(QFileInfo(fname).isDir()) {
                                if(recursive)
                                    dirs.append(fname);
                            }
                            if(regex.exactMatch(fname)) {
                                if(!replacement.isEmpty())
                                    replacement += Option::field_sep;
                                replacement += fname;
                            }
                        }
                    }
                }
            } else if(val.toLower() == "prompt") {
                if(arg_list.count() != 1) {
                    fprintf(stderr, "%s:%d prompt(question) requires one argument\n",
                            parser.file.latin1(), parser.line_no);
                } else if(projectFile() == "-") {
                    fprintf(stderr, "%s:%d prompt(question) cannot be used when '-o -' is used.\n",
                            parser.file.latin1(), parser.line_no);
                } else {
                    QString msg = arg_list.first();
                    fixEnvVariables(msg);
                    if(!msg.endsWith("?"))
                        msg += "?";
                    fprintf(stderr, "Project %s: %s ", val.toUpper().latin1(), msg.latin1());

                    QFile qfile;
                    if(qfile.open(QIODevice::ReadOnly, stdin)) {
                        QTextStream t(&qfile);
                        replacement = t.readLine();
                    }
                }
            } else if(FunctionBlock *defined = replaceFunctions[val]) {
                defined->exec(this, QStringList(args), replacement);
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
