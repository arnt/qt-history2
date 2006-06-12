/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "project.h"
#include "property.h"
#include "option.h"
#include "cachekeys.h"

#include <qdatetime.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qstack.h>
#include <qhash.h>
#include <qdebug.h>
#ifdef Q_OS_UNIX
#include <unistd.h>
#include <sys/utsname.h>
#elif defined(Q_OS_WIN32)
#include <Windows.h>
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

static QString remove_quotes(const QString &arg)
{
    const ushort SINGLEQUOTE = '\'';
    const ushort DOUBLEQUOTE = '"';

    const QChar *arg_data = arg.data();
    const ushort first = arg_data->unicode();
    const int arg_len = arg.length();
    if(first == SINGLEQUOTE || first == DOUBLEQUOTE) {
        const ushort last = (arg_data+arg_len-1)->unicode();
        if(last == first)
            return arg.mid(1, arg_len-2);
    }
    return arg;
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
    int quote = 0;
    QStringList args;

    const ushort LPAREN = '(';
    const ushort RPAREN = ')';
    const ushort SINGLEQUOTE = '\'';
    const ushort DOUBLEQUOTE = '"';
    const ushort COMMA = ',';
    const ushort SPACE = ' ';
    const ushort TAB = '\t';

    ushort unicode;
    const QChar *params_data = params.data();
    const int params_len = params.length();
    int last = 0;
    while(last < params_len && ((params_data+last)->unicode() == SPACE
                                /*|| (params_data+last)->unicode() == TAB*/))
        ++last;
    for(int x = last, parens = 0; x <= params_len; x++) {
        unicode = (params_data+x)->unicode();
        if(x == params_len) {
            while(x && (params_data+(x-1))->unicode() == SPACE)
                --x;
            QString mid(params_data+last, x-last);
            if(quote) {
                if(mid[0] == quote && mid[(int)mid.length()-1] == quote)
                    mid = mid.mid(1, mid.length()-2);
                quote = 0;
            }
            args << mid;
            break;
        }
        if(unicode == LPAREN) {
            --parens;
        } else if(unicode == RPAREN) {
            ++parens;
        } else if(quote && unicode == quote) {
            quote = 0;
        } else if(!quote && (unicode == SINGLEQUOTE || unicode == DOUBLEQUOTE)) {
            quote = unicode;
        } else if(!parens && !quote && unicode == COMMA) {
            QString mid = params.mid(last, x - last).trimmed();
            args << mid;
            last = x+1;
            while(last < params_len && ((params_data+last)->unicode() == SPACE
                                        /*|| (params_data+last)->unicode() == TAB*/))
                ++last;
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
    QStack<char> quote;

    const ushort LPAREN = '(';
    const ushort RPAREN = ')';
    const ushort SINGLEQUOTE = '\'';
    const ushort DOUBLEQUOTE = '"';
    const ushort SLASH = '\\';
    const ushort SEMICOLON = ';';

    ushort unicode;
    const QChar *vals_data = vals.data();
    const int vals_len = vals.length();
    for(int x = 0, parens = 0; x < vals_len; x++) {
        unicode = (vals_data+x)->unicode();
        bool append_char = true;
        if(x != (int)vals_len-1 && unicode == SLASH &&
           ((vals_data+(x+1))->unicode() == '\'' || (vals_data+(x+1))->unicode() == DOUBLEQUOTE)) {
            build += *(vals_data+(x++)); //get that 'escape'
        } else if(!quote.isEmpty() && unicode == quote.top()) {
            quote.pop();
        } else if(unicode == SINGLEQUOTE || unicode == DOUBLEQUOTE) {
            quote.push(unicode);
        } else if(unicode == RPAREN) {
            --parens;
        } else if(unicode == LPAREN) {
            ++parens;
        }

        if(!parens && quote.isEmpty() && ((do_semicolon && unicode == SEMICOLON) ||
                                          *(vals_data+x) == Option::field_sep)) {
            ret << build;
            build = "";
        } else {
            build += *(vals_data+x);
        }
    }
    if(!build.isEmpty())
        ret << build;
    return ret;
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
    bool eval(QMakeProject *p, QMap<QString, QStringList> &place);
};

bool ParsableBlock::eval(QMakeProject *p, QMap<QString, QStringList> &place)
{
    //save state
    parser_info pi = ::parser;
    const int block_count = p->scope_blocks.count();

    //execute
    bool ret = true;
    for(int i = 0; i < parser.count(); i++) {
        ::parser = parser.at(i).pi;
        if(!(ret = p->parse(parser.at(i).text, place)) || !continueBlock())
            break;
    }

    //restore state
    ::parser = pi;
    while(p->scope_blocks.count() > block_count)
        p->scope_blocks.pop();
    return ret;
}

//defined functions
struct FunctionBlock : public ParsableBlock
{
    FunctionBlock() : calling_place(0), scope_level(1), cause_return(false) { }

    QMap<QString, QStringList> vars;
    QMap<QString, QStringList> *calling_place;
    QString return_value;
    int scope_level;
    bool cause_return;

    bool exec(const QStringList &args,
              QMakeProject *p, QMap<QString, QStringList> &place, QStringList &functionReturn);
    virtual bool continueBlock() { return !cause_return; }
};

bool FunctionBlock::exec(const QStringList &args,
                         QMakeProject *proj, QMap<QString, QStringList> &place,
                         QStringList &functionReturn)
{
    //save state
#if 1
    calling_place = &place;
#else
    calling_place = &proj->variables();
#endif
    return_value = "";
    cause_return = false;

    //execute
#if 0
    vars = proj->variables(); // should be place so that local variables can be inherited
#else
    vars = place;
#endif
    vars["ARGS"] = args;
    for(int i = 0; i < args.count(); i++)
        vars[QString::number(i+1)] = QStringList(args[i]);
    bool ret = ParsableBlock::eval(proj, vars);
    functionReturn = split_value_list(return_value);

    //restore state
    calling_place = 0;
    return_value.clear();
    vars.clear();
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

    bool exec(QMakeProject *p, QMap<QString, QStringList> &place);
    virtual bool continueBlock() { return !cause_next && !cause_break; }
};
bool IteratorBlock::exec(QMakeProject *p, QMap<QString, QStringList> &place)
{
    bool ret = true;
    QStringList::Iterator it;
    if(!loop_forever)
        it = list.begin();
    int iterate_count = 0;
    //save state
    IteratorBlock *saved_iterator = p->iterator;
    p->iterator = this;

    //do the loop
    while(loop_forever || it != list.end()) {
        cause_next = cause_break = false;
        if(!loop_forever && (*it).isEmpty()) { //ignore empty items
            ++it;
            continue;
        }

        //set up the loop variable
        QStringList va;
        if(!variable.isEmpty()) {
            va = place[variable];
            if(loop_forever)
                place[variable] = QStringList(QString::number(iterate_count));
            else
                place[variable] = QStringList(*it);
        }
        //do the iterations
        bool succeed = true;
        for(QList<Test>::Iterator test_it = test.begin(); test_it != test.end(); ++test_it) {
            ::parser = (*test_it).pi;
            succeed = p->doProjectTest((*test_it).func, (*test_it).args, place);
            if((*test_it).invert)
                succeed = !succeed;
            if(!succeed)
                break;
        }
        if(succeed)
            ret = ParsableBlock::eval(p, place);
        //restore the variable in the map
        if(!variable.isEmpty())
            place[variable] = va;
        //loop counters
        if(!loop_forever)
            ++it;
        iterate_count++;
        if(!ret || cause_break)
            break;
    }

    //restore state
    p->iterator = saved_iterator;
    return ret;
}

QMakeProject::ScopeBlock::~ScopeBlock()
{
#if 0
    if(iterate)
        delete iterate;
#endif
}

static void qmake_error_msg(const QString &msg)
{
    fprintf(stderr, "%s:%d: %s\n", parser.file.toLatin1().constData(), parser.line_no,
            msg.toLatin1().constData());
}

/*
   1) environment variable QMAKEFEATURES (as separated by colons)
   2) property variable QMAKEFEATURES (as separated by colons)
   3) <project_root> (where .qmake.cache lives) + FEATURES_DIR
   4) environment variable QMAKEPATH (as separated by colons) + /mkspecs/FEATURES_DIR
   5) your QMAKESPEC/features dir
   6) your data_install/mkspecs/FEATURES_DIR
   7) your QMAKESPEC/../FEATURES_DIR dir

   FEATURES_DIR is defined as:

   1) features/(unix|win32|macx)/
   2) features/
*/
QStringList qmake_feature_paths(QMakeProperty *prop=0)
{
    QStringList concat;
    {
        const QString base_concat = QDir::separator() + QString("features");
        switch(Option::target_mode) {
        case Option::TARG_MACX_MODE:                     //also a unix
            concat << base_concat + QDir::separator() + "mac";
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
            concat << base_concat + QDir::separator() + "mac";
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
    QByteArray mkspec_path = qgetenv("QMAKEFEATURES");
    if(!mkspec_path.isNull())
        feature_roots += splitPathList(QString::fromLocal8Bit(mkspec_path));
    if(prop)
        feature_roots += splitPathList(prop->value("QMAKEFEATURES"));
    if(!Option::mkfile::cachefile.isEmpty()) {
        QString path;
        int last_slash = Option::mkfile::cachefile.lastIndexOf(Option::dir_sep);
        if(last_slash != -1)
            path = Option::fixPathToLocalOS(Option::mkfile::cachefile.left(last_slash));
        for(QStringList::Iterator concat_it = concat.begin();
            concat_it != concat.end(); ++concat_it)
            feature_roots << (path + (*concat_it));
    }
    QByteArray qmakepath = qgetenv("QMAKEPATH");
    if (!qmakepath.isNull()) {
        const QStringList lst = splitPathList(QString::fromLocal8Bit(qmakepath));
        for(QStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it) {
            for(QStringList::Iterator concat_it = concat.begin();
                concat_it != concat.end(); ++concat_it)
                    feature_roots << ((*it) + mkspecs_concat + (*concat_it));
        }
    }
    if(!Option::mkfile::qmakespec.isEmpty())
        feature_roots << Option::mkfile::qmakespec + QDir::separator() + "features";
    if(!Option::mkfile::qmakespec.isEmpty()) {
        QFileInfo specfi(Option::mkfile::qmakespec);
        QDir specdir(specfi.absoluteFilePath());
        while(!specdir.isRoot()) {
            if(!specdir.cdUp() || specdir.isRoot())
                break;
            if(QFile::exists(specdir.path() + QDir::separator() + "features")) {
                for(QStringList::Iterator concat_it = concat.begin();
                    concat_it != concat.end(); ++concat_it)
                    feature_roots << (specdir.path() + (*concat_it));
                break;
            }
        }
    }
    for(QStringList::Iterator concat_it = concat.begin();
        concat_it != concat.end(); ++concat_it)
        feature_roots << (QLibraryInfo::location(QLibraryInfo::PrefixPath) +
                          mkspecs_concat + (*concat_it));
    for(QStringList::Iterator concat_it = concat.begin();
        concat_it != concat.end(); ++concat_it)
        feature_roots << (QLibraryInfo::location(QLibraryInfo::DataPath) +
                          mkspecs_concat + (*concat_it));
    return feature_roots;
}

QStringList qmake_mkspec_paths()
{
    QStringList ret;
    const QString concat = QDir::separator() + QString("mkspecs");
    QByteArray qmakepath = qgetenv("QMAKEPATH");
    if (!qmakepath.isEmpty()) {
        const QStringList lst = splitPathList(QString::fromLocal8Bit(qmakepath));
        for(QStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it)
            ret << ((*it) + concat);
    }
    ret << QLibraryInfo::location(QLibraryInfo::DataPath) + concat;

    return ret;
}

class QMakeProjectEnv
{
    QStringList envs;
    void init(const QMap<QString, QStringList> &values) {
#ifdef Q_OS_UNIX
        for(QMap<QString, QStringList>::ConstIterator it = values.begin(); it != values.end(); ++it) {
            const QString var = it.key(), val = it.value().join(" ");
            if(!var.startsWith(".")) {
                const QString env_var = Option::sysenv_mod + var;
                if(!putenv(strdup(QString(env_var + "=" + val).toAscii().data())))
                    envs.append(env_var);
            }
        }
#endif
    }
public:
    QMakeProjectEnv(QMakeProject *p) { init(p->variables()); }
    QMakeProjectEnv(const QMap<QString, QStringList> &variables) { init(variables); }
    ~QMakeProjectEnv() {
#ifdef Q_OS_UNIX
        for(QStringList::ConstIterator it = envs.begin();it != envs.end(); ++it) {
            putenv(strdup(QString(*it + "=").toAscii().data()));
        }
#endif
    }
};

QMakeProject::~QMakeProject()
{
    if(own_prop)
        delete prop;
    qDeleteAll(replaceFunctions);
    qDeleteAll(testFunctions);
}


void
QMakeProject::init(QMakeProperty *p, const QMap<QString, QStringList> *vars)
{
    if(vars)
        base_vars = *vars;
    if(!p) {
        prop = new QMakeProperty;
        own_prop = true;
    } else {
        prop = p;
        own_prop = false;
    }
    reset();
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
                    fprintf(stderr, "Braces mismatch %s:%d\n", parser.file.toLatin1().constData(), parser.line_no);
                    return false;
                }
                ScopeBlock sb = scope_blocks.pop();
                if(sb.iterate) {
                    sb.iterate->exec(this, place);
                    delete sb.iterate;
                    sb.iterate = false;
                }
                if(!scope_blocks.top().ignore) {
                    debug_msg(1, "Project Parser: %s:%d : Leaving block %d", parser.file.toLatin1().constData(),
                              parser.line_no, scope_blocks.count()+1);
                    s = s.mid(i+1).trimmed();
                    continue_parsing = !s.isEmpty();
                    break;
                }
            }
        }
        if(!continue_parsing) {
            debug_msg(1, "Project Parser: %s:%d : Ignored due to block being false.",
                      parser.file.toLatin1().constData(), parser.line_no);
            return true;
        }
    }

    if(function) {
        QString append;
        int d_off = 0;
        const QChar *d = s.unicode();
        bool function_finished = false;
        while(d_off < s.length()) {
            if(*(d+d_off) == QLatin1Char('}')) {
                function->scope_level--;
                if(!function->scope_level) {
                    function_finished = true;
                    break;
                }
            } else if(*(d+d_off) == QLatin1Char('{')) {
                function->scope_level++;
            }
            append += *(d+d_off);
            ++d_off;
        }
        if(!append.isEmpty())
            function->parser.append(IteratorBlock::Parse(append));
        if(function_finished) {
            function = 0;
            s = QString(d+d_off, s.length()-d_off);
        } else {
            return true;
        }
    } else if(IteratorBlock *it = scope_blocks.top().iterate) {
        QString append;
        int d_off = 0;
        const QChar *d = s.unicode();
        bool iterate_finished = false;
        while(d_off < s.length()) {
            if(*(d+d_off) == QLatin1Char('}')) {
                it->scope_level--;
                if(!it->scope_level) {
                    iterate_finished = true;
                    break;
                }
            } else if(*(d+d_off) == QLatin1Char('{')) {
                it->scope_level++;
            }
            append += *(d+d_off);
            ++d_off;
        }
        if(!append.isEmpty())
            scope_blocks.top().iterate->parser.append(IteratorBlock::Parse(append));
        if(iterate_finished) {
            scope_blocks.top().iterate = 0;
            bool ret = it->exec(this, place);
            delete it;
            if(!ret)
                return false;
            s = s.mid(d_off);
        } else {
            return true;
        }
    }

    QString scope, var, op;
    QStringList val;
#define SKIP_WS(d, o, l) while(o < l && (*(d+o) == QLatin1Char(' ') || *(d+o) == QLatin1Char('\t'))) ++o
    const QChar *d = s.unicode();
    int d_off = 0;
    SKIP_WS(d, d_off, s.length());
    IteratorBlock *iterator = 0;
    bool scope_failed = false, else_line = false, or_op=false;
    QChar quote = 0;
    int parens = 0, scope_count=0, start_block = 0;
    while(d_off < s.length()) {
        if(!parens) {
            if(*(d+d_off) == QLatin1Char('='))
                break;
            if(*(d+d_off) == QLatin1Char('+') || *(d+d_off) == QLatin1Char('-') ||
               *(d+d_off) == QLatin1Char('*') || *(d+d_off) == QLatin1Char('~')) {
                if(*(d+d_off+1) == QLatin1Char('=')) {
                    break;
                } else if(*(d+d_off+1) == QLatin1Char(' ')) {
                    const QChar *k = d+d_off+1;
                    int k_off = 0;
                    SKIP_WS(k, k_off, s.length()-d_off);
                    if(*(k+k_off) == QLatin1Char('=')) {
                        QString msg;
                        qmake_error_msg(QString(d+d_off, 1) + "must be followed immediately by =");
                        return false;
                    }
                }
            }
        }

        if(!quote.isNull()) {
            if(*(d+d_off) == quote)
                quote = QChar();
        } else if(*(d+d_off) == '(') {
            ++parens;
        } else if(*(d+d_off) == ')') {
            --parens;
        } else if(*(d+d_off) == '"' /*|| *(d+d_off) == '\''*/) {
            quote = *(d+d_off);
        }

        if(!parens && quote.isNull() &&
           (*(d+d_off) == QLatin1Char(':') || *(d+d_off) == QLatin1Char('{') ||
            *(d+d_off) == QLatin1Char(')') || *(d+d_off) == QLatin1Char('|'))) {
            scope_count++;
            scope = var.trimmed();
            if(*(d+d_off) == QLatin1Char(')'))
                scope += *(d+d_off); // need this
            var = "";

            bool test = scope_failed;
            if(scope.isEmpty()) {
                test = true;
            } else if(scope.toLower() == "else") { //else is a builtin scope here as it modifies state
                if(scope_count != 1 || scope_blocks.top().else_status == ScopeBlock::TestNone) {
                    qmake_error_msg(("Unexpected " + scope + " ('" + s + "')").toLatin1());
                    return false;
                }
                else_line = true;
                test = (scope_blocks.top().else_status == ScopeBlock::TestSeek);
                debug_msg(1, "Project Parser: %s:%d : Else%s %s.", parser.file.toLatin1().constData(), parser.line_no,
                          scope == "else" ? "" : QString(" (" + scope + ")").toLatin1().constData(),
                          test ? "considered" : "excluded");
            } else {
                QString comp_scope = scope;
                bool invert_test = (comp_scope.at(0) == QLatin1Char('!'));
                if(invert_test)
                    comp_scope = comp_scope.mid(1);
                int lparen = comp_scope.indexOf('(');
                if(or_op == scope_failed) {
                    if(lparen != -1) { // if there is an lparen in the scope, it IS a function
                        int rparen = comp_scope.lastIndexOf(')');
                        if(rparen == -1) {
                            qmake_error_msg("Function missing right paren: " + comp_scope);
                            return false;
                        }
                        QString func = comp_scope.left(lparen);
                        QStringList args = split_arg_list(comp_scope.mid(lparen+1, rparen - lparen - 1));
                        for(int i = 0; i < args.size(); ++i)
                            args[i] = remove_quotes(args[i].trimmed());
                        if(function) {
                            fprintf(stderr, "%s:%d: No tests can come after a function definition!\n",
                                    parser.file.toLatin1().constData(), parser.line_no);
                            return false;
                        } else if(func == "for") { //for is a builtin function here, as it modifies state
                            if(args.count() > 2 || args.count() < 1) {
                                fprintf(stderr, "%s:%d: for(iterate, list) requires two arguments.\n",
                                        parser.file.toLatin1().constData(), parser.line_no);
                                return false;
                            } else if(iterator) {
                                fprintf(stderr, "%s:%d unexpected nested for()\n",
                                        parser.file.toLatin1().constData(), parser.line_no);
                                return false;
                            }

                            iterator = new IteratorBlock;
                            QString it_list;
                            if(args.count() == 1) {
                                doVariableReplace(args[0], place);
                                it_list = args[0];
                                if(args[0] != "ever") {
                                    delete iterator;
                                    iterator = 0;
                                    fprintf(stderr, "%s:%d: for(iterate, list) requires two arguments.\n",
                                            parser.file.toLatin1().constData(), parser.line_no);
                                    return false;
                                }
                                it_list = "forever";
                            } else if(args.count() == 2) {
                                iterator->variable = args[0];
                                doVariableReplace(args[1], place);
                                it_list = args[1];
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
                                        parser.file.toLatin1().constData(), parser.line_no);
                                return false;
                            }
                            if(args.count() != 1) {
                                fprintf(stderr, "%s:%d: %s(function_name) requires one argument.\n",
                                        parser.file.toLatin1().constData(), parser.line_no, func.toLatin1().constData());
                                return false;
                            }
                            QMap<QString, FunctionBlock*> *map = 0;
                            if(func == "defineTest")
                                map = &testFunctions;
                            else
                                map = &replaceFunctions;
                            if(!map || map->contains(args[0])) {
                                fprintf(stderr, "%s:%d: Function[%s] multiply defined.\n",
                                        parser.file.toLatin1().constData(), parser.line_no, args[0].toLatin1().constData());
                                return false;
                            }
                            function = new FunctionBlock;
                            map->insert(args[0], function);
                            test = true;
                        } else {
                            test = doProjectTest(func, args, place);
                            if(*(d+d_off) == QLatin1Char(')') && d_off == s.length()-1) {
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
                debug_msg(1, "Project Parser: %s:%d : Test (%s) failed.", parser.file.toLatin1().constData(),
                          parser.line_no, scope.toLatin1().constData());
            if(test == or_op)
                scope_failed = !test;
            or_op = (*(d+d_off) == QLatin1Char('|'));

            if(*(d+d_off) == QLatin1Char('{')) { // scoping block
                start_block++;
                if(iterator) {
                    for(int off = 0, braces = 0; true; ++off) {
                        if(*(d+d_off+off) == QLatin1Char('{'))
                            ++braces;
                        else if(*(d+d_off+off) == QLatin1Char('}') && braces)
                            --braces;
                        if(!braces || d_off+off == s.length()) {
                            iterator->parser.append(s.mid(d_off, off-1));
                            if(braces > 1)
                                iterator->scope_level += braces-1;
                            d_off += off-1;
                            break;
                        }
                    }
                }
            }
    } else if(!parens && *(d+d_off) == QLatin1Char('}')) {
            if(start_block) {
                --start_block;
            } else if(!scope_blocks.count()) {
                warn_msg(WarnParser, "Possible braces mismatch %s:%d", parser.file.toLatin1().constData(), parser.line_no);
            } else {
                if(scope_blocks.count() == 1) {
                    fprintf(stderr, "Braces mismatch %s:%d\n", parser.file.toLatin1().constData(), parser.line_no);
                    return false;
                }
                debug_msg(1, "Project Parser: %s:%d : Leaving block %d", parser.file.toLatin1().constData(),
                          parser.line_no, scope_blocks.count());
                ScopeBlock sb = scope_blocks.pop();
                if(sb.iterate)
                    sb.iterate->exec(this, place);
            }
        } else {
            var += *(d+d_off);
        }
        ++d_off;
    }
    var = var.trimmed();

    if(!else_line || (else_line && !scope_failed))
        scope_blocks.top().else_status = (!scope_failed ? ScopeBlock::TestFound : ScopeBlock::TestSeek);
    if(start_block) {
        ScopeBlock next_block(scope_failed);
        next_block.iterate = iterator;
        if(iterator)
            next_block.else_status = ScopeBlock::TestNone;
        else if(scope_failed)
            next_block.else_status = ScopeBlock::TestSeek;
        else
            next_block.else_status = ScopeBlock::TestFound;
        scope_blocks.push(next_block);
        debug_msg(1, "Project Parser: %s:%d : Entering block %d (%d). [%s]", parser.file.toLatin1().constData(),
                  parser.line_no, scope_blocks.count(), scope_failed, s.toLatin1().constData());
    } else if(iterator) {
        iterator->parser.append(var+s.mid(d_off));
        bool ret = iterator->exec(this, place);
        delete iterator;
        return ret;
    }

    if((!scope_count && !var.isEmpty()) || (scope_count == 1 && else_line))
        scope_blocks.top().else_status = ScopeBlock::TestNone;
    if(d_off == s.length()) {
        if(!var.trimmed().isEmpty())
            qmake_error_msg(("Parse Error ('" + s + "')").toLatin1());
        return var.isEmpty(); // allow just a scope
    }

    SKIP_WS(d, d_off, s.length());
    for(; d_off < s.length() && op.indexOf('=') == -1; op += *(d+(d_off++)))
        ;
    op.replace(QRegExp("\\s"), "");

    SKIP_WS(d, d_off, s.length());
    QString vals = s.mid(d_off); // vals now contains the space separated list of values
    int rbraces = vals.count('}'), lbraces = vals.count('{');
    if(scope_blocks.count() > 1 && rbraces - lbraces == 1) {
        debug_msg(1, "Project Parser: %s:%d : Leaving block %d", parser.file.toLatin1().constData(),
                  parser.line_no, scope_blocks.count());
        ScopeBlock sb = scope_blocks.pop();
        if(sb.iterate)
            sb.iterate->exec(this, place);
        vals.truncate(vals.length()-1);
    } else if(rbraces != lbraces) {
        warn_msg(WarnParser, "Possible braces mismatch {%s} %s:%d",
                 vals.toLatin1().constData(), parser.file.toLatin1().constData(), parser.line_no);
    }
    if(scope_failed)
        return true; // oh well
#undef SKIP_WS

    doVariableReplace(var, place);
    var = varMap(var); //backwards compatability
    if(!var.isEmpty() && Option::mkfile::do_preprocess) {
        static QString last_file("*none*");
        if(parser.file != last_file) {
            fprintf(stdout, "#file %s:%d\n", parser.file.toLatin1().constData(), parser.line_no);
            last_file = parser.file;
        }
        fprintf(stdout, "%s %s %s\n", var.toLatin1().constData(), op.toLatin1().constData(), vals.toLatin1().constData());
    }

    if(vals.contains('='))
        warn_msg(WarnParser, "Detected possible line continuation: {%s} %s:%d",
                 var.toLatin1().constData(), parser.file.toLatin1().constData(), parser.line_no);

    QStringList &varlist = place[var]; // varlist is the list in the symbol table

    {
        QString tmp_vals = vals;
        doVariableReplace(tmp_vals, place);
        debug_msg(1, "Project Parser: %s:%d :%s: :%s: (%s)", parser.file.toLatin1().constData(), parser.line_no,
                  var.toLatin1().constData(), op.toLatin1().constData(), tmp_vals.toLatin1().constData());
    }

    // now do the operation
    if(op == "~=") {
        doVariableReplace(vals, place);
        if(vals.length() < 4 || vals.at(0) != 's') {
            qmake_error_msg(("~= operator only can handle s/// function ('" +
                            s + "')").toLatin1());
            return false;
        }
        QChar sep = vals.at(1);
        QStringList func = vals.split(sep);
        if(func.count() < 3 || func.count() > 4) {
            qmake_error_msg(("~= operator only can handle s/// function ('" +
                s + "')").toLatin1());
            return false;
        }
        bool global = false, case_sense = true, quote = false;
        if(func.count() == 4) {
            global = func[3].indexOf('g') != -1;
            case_sense = func[3].indexOf('i') == -1;
            quote = func[3].indexOf('q') != -1;
        }
        QString from = func[1], to = func[2];
        if(quote)
            from = QRegExp::escape(from);
        QRegExp regexp(from, case_sense ? Qt::CaseSensitive : Qt::CaseInsensitive);
        for(QStringList::Iterator varit = varlist.begin(); varit != varlist.end();) {
            if((*varit).contains(regexp)) {
                (*varit) = (*varit).replace(regexp, to);
                if ((*varit).isEmpty())
                    varit = varlist.erase(varit);
                else
                    ++varit;
                if(!global)
                    break;
            } else
                ++varit;
        }
    } else {
        QStringList vallist;
        {
            //doVariableReplace(vals, place);
            QStringList tmp = split_value_list(vals, (var == "DEPENDPATH" || var == "INCLUDEPATH"));
            for(int i = 0; i < tmp.size(); ++i)
                vallist += doVariableReplaceExpand(tmp[i], place);
        }

        if(op == "=") {
            if(!varlist.isEmpty())
                warn_msg(WarnParser, "Operator=(%s) clears variables previously set: %s:%d",
                         var.toLatin1().constData(), parser.file.toLatin1().constData(), parser.line_no);
            varlist.clear();
        }
        for(QStringList::ConstIterator valit = vallist.begin();
            valit != vallist.end(); ++valit) {
            if((*valit).isEmpty())
                continue;
            if((op == "*=" && !varlist.contains((*valit))) ||
               op == "=" || op == "+=")
                varlist.append((*valit));
            else if(op == "-=")
                varlist.removeAll((*valit));
        }
        if(var == "REQUIRES") // special case to get communicated to backends!
            doProjectCheckReqs(vallist, place);
    }
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
                if(!(ret = parse(s, place))) {
                    s = "";
                    break;
                }
                s = "";
            }
        }
    }
    if (!s.isEmpty())
        ret = parse(s, place);
    return ret;
}

bool
QMakeProject::read(const QString &file, QMap<QString, QStringList> &place)
{
    parser_info pi = parser;
    reset();

    const QString oldpwd = qmake_getpwd();
    QString filename = Option::fixPathToLocalOS(file);
    doVariableReplace(filename, place);
    bool ret = false, using_stdin = false;
    QFile qfile;
    if(!strcmp(filename.toLatin1(), "-")) {
        qfile.setFileName("");
        ret = qfile.open(stdin, QIODevice::ReadOnly);
        using_stdin = true;
    } else if(QFileInfo(file).isDir()) {
        return false;
    } else {
        qfile.setFileName(filename);
        ret = qfile.open(QIODevice::ReadOnly);
        qmake_setpwd(QFileInfo(filename).absolutePath());
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
    }
    if(scope_blocks.count() != 1) {
        qmake_error_msg("Unterminated conditional block at end of file");
        ret = false;
    }
    parser = pi;
    qmake_setpwd(oldpwd);
    return ret;
}

bool
QMakeProject::read(const QString &project, uchar cmd)
{
    pfile = QFileInfo(project).absoluteFilePath();
    return read(cmd);
}

bool
QMakeProject::read(uchar cmd)
{
    if(cfile.isEmpty()) {
        // hack to get the Option stuff in there
        base_vars["QMAKE_EXT_OBJ"] = QStringList(Option::obj_ext);
        base_vars["QMAKE_EXT_CPP"] = Option::cpp_ext;
        base_vars["QMAKE_EXT_C"] = Option::c_ext;
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
            QString qmakespec = fixEnvVariables(Option::mkfile::qmakespec);
            QStringList mkspec_roots = qmake_mkspec_paths();
            debug_msg(2, "Looking for mkspec %s in (%s)", qmakespec.toLatin1().constData(),
                      mkspec_roots.join("::").toLatin1().constData());
            if(qmakespec.isEmpty()) {
                for(QStringList::ConstIterator it = mkspec_roots.begin(); it != mkspec_roots.end(); ++it) {
                    QString mkspec = (*it) + QDir::separator() + "default";
                    QFileInfo default_info(mkspec);
                    if(default_info.exists() && default_info.isDir()) {
                        qmakespec = mkspec;
                        break;
                    }
                }
                if(qmakespec.isEmpty()) {
                    fprintf(stderr, "QMAKESPEC has not been set, so configuration cannot be deduced.\n");
                    return false;
                }
                Option::mkfile::qmakespec = qmakespec;
            }

            if(QDir::isRelativePath(qmakespec) &&
               !QFile::exists(qmakespec+"/qmake.conf")) {
                bool found_mkspec = false;
                for(QStringList::ConstIterator it = mkspec_roots.begin(); it != mkspec_roots.end(); ++it) {
                    QString mkspec = (*it) + QDir::separator() + qmakespec;
                    if(QFile::exists(mkspec)) {
                        found_mkspec = true;
                        Option::mkfile::qmakespec = qmakespec = mkspec;
                        break;
                    }
                }
                if(!found_mkspec) {
                    fprintf(stderr, "Could not find mkspecs for your QMAKESPEC after trying:\n\t%s\n",
                            mkspec_roots.join("\n\t").toLatin1().constData());
                    return false;
                }
            }

            // parse qmake configuration
            while(qmakespec.endsWith(QString(QChar(QDir::separator()))))
                qmakespec.truncate(qmakespec.length()-1);
            QString spec = qmakespec + QDir::separator() + "qmake.conf";
            if(!QFile::exists(spec) &&
               QFile::exists(qmakespec + QDir::separator() + "tmake.conf"))
                spec = qmakespec + QDir::separator() + "tmake.conf";
            debug_msg(1, "QMAKESPEC conf: reading %s", spec.toLatin1().constData());
            if(!read(spec, base_vars)) {
                fprintf(stderr, "Failure to read QMAKESPEC conf file %s.\n", spec.toLatin1().constData());
                return false;
            }
            if(Option::mkfile::do_cache && !Option::mkfile::cachefile.isEmpty()) {
                debug_msg(1, "QMAKECACHE file: reading %s", Option::mkfile::cachefile.toLatin1().constData());
                read(Option::mkfile::cachefile, base_vars);
            }
        }

        if(cmd & ReadFeatures) {
            debug_msg(1, "Processing default_pre: %s", vars["CONFIG"].join("::").toLatin1().constData());
            if(doProjectInclude("default_pre", IncludeFlagFeature, base_vars) == IncludeNoExist)
                doProjectInclude("default", IncludeFlagFeature, base_vars);
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
        for(QStringList::ConstIterator it = Option::before_user_vars.begin();
            it != Option::before_user_vars.end(); ++it) {
            if(!parse((*it), vars)) {
                fprintf(stderr, "Argument failed to parse: %s\n", (*it).toLatin1().constData());
                return false;
            }
            parser.line_no++;
        }
    }

    //commandline configs
    if(cmd & ReadConfigs && !Option::user_configs.isEmpty()) {
        parser.file = "(configs)";
        parser.from_file = false;
        parser.line_no = 1; //really arg count now.. duh
        parse("CONFIG += " + Option::user_configs.join(" "), vars);
    }

    if(cmd & ReadProFile) { // parse project file
        debug_msg(1, "Project file: reading %s", pfile.toLatin1().constData());
        if(pfile != "-" && !QFile::exists(pfile) && !pfile.endsWith(".pro"))
            pfile += ".pro";
        if(!read(pfile, vars))
            return false;
    }

    if(cmd & ReadPostFiles) { // parse post files
        const QStringList l = vars["QMAKE_POST_INCLUDE_FILES"];
        for(QStringList::ConstIterator it = l.begin(); it != l.end(); ++it) {
            if(read((*it), vars)) {
                if(vars["QMAKE_INTERNAL_INCLUDED_FILES"].indexOf((*it)) == -1)
                    vars["QMAKE_INTERNAL_INCLUDED_FILES"].append((*it));
            }
        }
    }

    if(cmd & ReadCmdLine) {
        parser.file = "(internal)";
        parser.from_file = false;
        parser.line_no = 1; //really arg count now.. duh
        reset();
        for(QStringList::ConstIterator it = Option::after_user_vars.begin();
            it != Option::after_user_vars.end(); ++it) {
            if(!parse((*it), vars)) {
                fprintf(stderr, "Argument failed to parse: %s\n", (*it).toLatin1().constData());
                return false;
            }
            parser.line_no++;
        }
    }

    //after configs (set in BUILDS)
    if(cmd & ReadConfigs && !Option::after_user_configs.isEmpty()) {
        parser.file = "(configs)";
        parser.from_file = false;
        parser.line_no = 1; //really arg count now.. duh
        parse("CONFIG += " + Option::after_user_configs.join(" "), vars);
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
        debug_msg(1, "Processing default_post: %s", vars["CONFIG"].join("::").toLatin1().constData());
        doProjectInclude("default_post", IncludeFlagFeature, vars);

        QHash<QString, bool> processed;
        const QStringList &configs = vars["CONFIG"];
        debug_msg(1, "Processing CONFIG features: %s", configs.join("::").toLatin1().constData());
        while(1) {
            bool finished = true;
            for(int i = configs.size()-1; i >= 0; --i) {
                if(!processed.contains(configs[i])) {
                    processed.insert(configs[i], true);
                    if(doProjectInclude(configs[i], IncludeFlagFeature, vars) == IncludeSuccess) {
                        finished = false;
                        break;
                    }
                }
            }
            if(finished)
                break;
        }
    }
    Option::postProcessProject(this);   // let Option post-process
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
        if(!buffer) {
            buffer = (char *)malloc(1024);
            qmakeAddCacheClear(qmakeFreeCacheClear, (void**)&buffer);
        }
        int l = readlink(Option::mkfile::qmakespec.toLatin1(), buffer, 1024);
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
    const QStringList &configs = (place ? (*place)["CONFIG"] : vars["CONFIG"]);
    for(QStringList::ConstIterator it = configs.begin(); it != configs.end(); ++it) {
        if(((regex && re.exactMatch((*it))) || (!regex && (*it) == x)) && re.exactMatch((*it)))
            return true;
    }
    return false;
}

bool
QMakeProject::doProjectTest(QString str, QMap<QString, QStringList> &place)
{
    QString chk = remove_quotes(str);
    if(chk.isEmpty())
        return true;
    bool invert_test = (chk.left(1) == "!");
    if(invert_test)
        chk = chk.mid(1);

    bool test=false;
    int lparen = chk.indexOf('(');
    if(lparen != -1) { // if there is an lparen in the chk, it IS a function
        int rparen = chk.indexOf(')', lparen);
        if(rparen == -1) {
            qmake_error_msg("Function (in REQUIRES) missing right paren: " + chk);
        } else {
            QString func = chk.left(lparen);
            test = doProjectTest(func, chk.mid(lparen+1, rparen - lparen - 1), place);
        }
    } else {
        test = isActiveConfig(chk, true, &place);
    }
    if(invert_test)
        return !test;
    return test;
}

bool
QMakeProject::doProjectTest(QString func, const QString &params,
                            QMap<QString, QStringList> &place)
{
    return doProjectTest(func, split_arg_list(params), place);
}

QMakeProject::IncludeStatus
QMakeProject::doProjectInclude(QString file, uchar flags, QMap<QString, QStringList> &place)
{
    if(flags & IncludeFlagFeature) {
        if(!file.endsWith(Option::prf_ext))
            file += Option::prf_ext;
        if(file.indexOf(Option::dir_sep) == -1 || !QFile::exists(file)) {
            bool found = false;
            static QStringList *feature_roots = 0;
            if(!feature_roots) {
                feature_roots = new QStringList(qmake_feature_paths(prop));
                qmakeAddCacheClear(qmakeDeleteCacheClear_QStringList, (void**)&feature_roots);
            }
            debug_msg(2, "Looking for feature '%s' in (%s)", file.toLatin1().constData(),
			feature_roots->join("::").toLatin1().constData());
            int start_root = 0;
            if(parser.from_file) {
                QFileInfo currFile(parser.file), prfFile(file);
                if(currFile.fileName() == prfFile.fileName()) {
                    currFile = QFileInfo(currFile.canonicalFilePath());
                    for(int root = 0; root < feature_roots->size(); ++root) {
                        prfFile = QFileInfo(feature_roots->at(root) +
                                            QDir::separator() + file).canonicalFilePath();
                        if(prfFile == currFile) {
                            start_root = root+1;
                            break;
                        }
                    }
                }
            }
            for(int root = start_root; root < feature_roots->size(); ++root) {
                QString prf(feature_roots->at(root) + QDir::separator() + file);
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
        if(Option::output_dir != qmake_getpwd())
            include_roots << qmake_getpwd();
        include_roots << Option::output_dir;
        for(int root = 0; root < include_roots.size(); ++root) {
		QString testName = QDir::convertSeparators(include_roots[root]);
		if (!testName.endsWith(QString(QDir::separator())))
			testName += QDir::separator();
		testName += file;

            if(QFile::exists(testName)) {
                file = testName;
                break;
            }
        }
    }
    if(!QFile::exists(file))
        return IncludeNoExist;

    if(Option::mkfile::do_preprocess) //nice to see this first..
        fprintf(stderr, "#switching file %s(%s) - %s:%d\n", (flags & IncludeFlagFeature) ? "load" : "include",
                file.toLatin1().constData(),
                parser.file.toLatin1().constData(), parser.line_no);
    debug_msg(1, "Project Parser: %s'ing file %s.", (flags & IncludeFlagFeature) ? "load" : "include",
              file.toLatin1().constData());
    QString orig_file = file;
    int di = file.lastIndexOf(QDir::separator());
    QString oldpwd = qmake_getpwd();
    if(di != -1) {
        if(!qmake_setpwd(file.left(file.lastIndexOf(QDir::separator())))) {
            fprintf(stderr, "Cannot find directory: %s\n", file.left(di).toLatin1().constData());
            return IncludeFailure;
        }
        file = file.right(file.length() - di - 1);
    }
    parser_info pi = parser;
    QStack<ScopeBlock> sc = scope_blocks;
    IteratorBlock *it = iterator;
    FunctionBlock *fu = function;
    bool parsed = false;
    if(flags & (IncludeFlagNewProject|IncludeFlagNewParser)) {
        // The "project's variables" are used in other places (eg. export()) so it's not
        // possible to use "place" everywhere. Instead just set variables and grab them later
        QMakeProject proj;
        proj.variables() = place;
        if(flags & IncludeFlagNewParser) {
            if(proj.doProjectInclude("default_pre", IncludeFlagFeature, proj.variables()) == IncludeNoExist)
                proj.doProjectInclude("default", IncludeFlagFeature, proj.variables());
            parsed = proj.read(file, proj.variables());
        } else {
            parsed = proj.read(file);
        }
        place = proj.variables();
    } else {
        parsed = read(file, place);
    }
    if(parsed) {
        if(place["QMAKE_INTERNAL_INCLUDED_FILES"].indexOf(orig_file) == -1)
            place["QMAKE_INTERNAL_INCLUDED_FILES"].append(orig_file);
    } else {
        warn_msg(WarnParser, "%s:%d: Failure to include file %s.",
                 pi.file.toLatin1().constData(), pi.line_no, orig_file.toLatin1().constData());
    }
    iterator = it;
    function = fu;
    parser = pi;
    scope_blocks = sc;
    qmake_setpwd(oldpwd);
    if(!parsed)
        return IncludeParseFailure;
    return IncludeSuccess;
}

QStringList
QMakeProject::doProjectExpand(QString func, const QString &params,
                              QMap<QString, QStringList> &place)
{
    return doProjectExpand(func, split_arg_list(params), place);
}

QStringList
QMakeProject::doProjectExpand(QString func, QStringList args,
                              QMap<QString, QStringList> &place)
{
    func = func.trimmed();
    for(QStringList::Iterator arit = args.begin(); arit != args.end(); ++arit)
        doVariableReplace((*arit), place);

    enum ExpandFunc { E_MEMBER=1, E_FIRST, E_LAST, E_CAT, E_FROMFILE, E_EVAL, E_LIST,
                      E_SPRINTF, E_JOIN, E_SPLIT, E_BASENAME, E_DIRNAME, E_SECTION,
                      E_FIND, E_SYSTEM, E_UNIQUE, E_QUOTE, E_ESCAPE_EXPAND,
                      E_UPPER, E_LOWER, E_FILES, E_PROMPT, E_RE_ESCAPE,
                      E_REPLACE };
    static QMap<QString, int> *expands = 0;
    if(!expands) {
        expands = new QMap<QString, int>;
        qmakeAddCacheClear(qmakeDeleteCacheClear_QMapStringInt, (void**)&expands);
        expands->insert("member", E_MEMBER);
        expands->insert("first", E_FIRST);
        expands->insert("last", E_LAST);
        expands->insert("cat", E_CAT);
        expands->insert("fromfile", E_FROMFILE);
        expands->insert("eval", E_EVAL);
        expands->insert("list", E_LIST);
        expands->insert("sprintf", E_SPRINTF);
        expands->insert("join", E_JOIN);
        expands->insert("split", E_SPLIT);
        expands->insert("basename", E_BASENAME);
        expands->insert("dirname", E_DIRNAME);
        expands->insert("section", E_SECTION);
        expands->insert("find", E_FIND);
        expands->insert("system", E_SYSTEM);
        expands->insert("unique", E_UNIQUE);
        expands->insert("quote", E_QUOTE);
        expands->insert("escape_expand", E_ESCAPE_EXPAND);
        expands->insert("upper", E_UPPER);
        expands->insert("lower", E_LOWER);
        expands->insert("re_escape", E_RE_ESCAPE);
        expands->insert("files", E_FILES);
        expands->insert("prompt", E_PROMPT);
        expands->insert("replace", E_REPLACE);
    }
    ExpandFunc func_t = (ExpandFunc)expands->value(func.toLower());
    debug_msg(1, "Running project expand: %s(%s) [%d]",
              func.toLatin1().constData(), args.join("::").toLatin1().constData(), func_t);

    QStringList ret;
    switch(func_t) {
    case E_MEMBER: {
        if(args.count() < 1 || args.count() > 3) {
            fprintf(stderr, "%s:%d: member(var, start, end) requires three arguments.\n",
                    parser.file.toLatin1().constData(), parser.line_no);
        } else {
            bool ok = true;
            const QStringList &var = values(args.first(), place);
            int start = 0, end = 0;
            if(args.count() >= 2) {
                QString start_str = args[1];
                start = start_str.toInt(&ok);
                if(!ok) {
                    if(args.count() == 2) {
                        int dotdot = start_str.indexOf("..");
                        if(dotdot != -1) {
                            start = start_str.left(dotdot).toInt(&ok);
                            if(ok)
                                end = start_str.mid(dotdot+2).toInt(&ok);
                        }
                    }
                    if(!ok)
                        fprintf(stderr, "%s:%d: member() argument 2 (start) '%s' invalid.\n",
                                parser.file.toLatin1().constData(), parser.line_no,
                                start_str.toLatin1().constData());
                } else {
                    end = start;
                    if(args.count() == 3)
                        end = args[2].toInt(&ok);
                    if(!ok)
                        fprintf(stderr, "%s:%d: member() argument 3 (end) '%s' invalid.\n",
                                parser.file.toLatin1().constData(), parser.line_no,
                                args[2].toLatin1().constData());
                }
            }
            if(ok) {
                if(start < 0)
                    start += var.count();
                if(end < 0)
                    end += var.count();
                if(start < 0 || start >= var.count() || end < 0 || end >= var.count()) {
                    //nothing
                } else if(start < end) {
                    for(int i = start; i <= end && (int)var.count() >= i; i++)
                        ret += var[i];
                } else {
                    for(int i = start; i >= end && (int)var.count() >= i && i >= 0; i--)
                        ret += var[i];
                }
            }
        }
        break; }
    case E_FIRST:
    case E_LAST: {
        if(args.count() != 1) {
            fprintf(stderr, "%s:%d: %s(var) requires one argument.\n",
                    parser.file.toLatin1().constData(), parser.line_no, func.toLatin1().constData());
        } else {
            const QStringList &var = values(args.first(), place);
            if(!var.isEmpty()) {
                if(func_t == E_FIRST)
                    ret = QStringList(var[0]);
                else
                    ret = QStringList(var[var.size()-1]);
            }
        }
        break; }
    case E_CAT: {
        if(args.count() < 1 || args.count() > 2) {
            fprintf(stderr, "%s:%d: cat(file) requires one argument.\n",
                    parser.file.toLatin1().constData(), parser.line_no);
        } else {
            QString file = args[0];
            file = Option::fixPathToLocalOS(file);

            bool singleLine = true;
            if(args.count() > 1)
                singleLine = (args[1].toLower() == "true");

            QFile qfile(file);
            if(qfile.open(QIODevice::ReadOnly)) {
                QTextStream stream(&qfile);
                while(!stream.atEnd()) {
                    ret += split_value_list(stream.readLine().trimmed());
                    if(!singleLine)
                        ret += "\n";
                }
                qfile.close();
            }
        }
        break; }
    case E_FROMFILE: {
        if(args.count() != 2) {
            fprintf(stderr, "%s:%d: fromfile(file, variable) requires two arguments.\n",
                    parser.file.toLatin1().constData(), parser.line_no);
        } else {
            QString file = args[0], seek_var = args[1];
            file = Option::fixPathToLocalOS(file);

            QMap<QString, QStringList> tmp;
            if(doProjectInclude(file, IncludeFlagNewParser, tmp) == IncludeSuccess) {
                if(tmp.contains("QMAKE_INTERNAL_INCLUDED_FILES")) {
                    QStringList &out = place["QMAKE_INTERNAL_INCLUDED_FILES"];
                    const QStringList &in = tmp["QMAKE_INTERNAL_INCLUDED_FILES"];
                    for(int i = 0; i < in.size(); ++i) {
                        if(out.indexOf(in[i]) == -1)
                            out += in[i];
                    }
                }
                ret = tmp[seek_var];
            }
        }
        break; }
    case E_EVAL: {
        if(args.count() < 1 || args.count() > 2) {
            fprintf(stderr, "%s:%d: eval(variable) requires one argument.\n",
                    parser.file.toLatin1().constData(), parser.line_no);

        } else {
            const QMap<QString, QStringList> *source = &place;
            if(args.count() == 2) {
                if(args.at(1) == "Global") {
                    source = &vars;
                } else if(args.at(1) == "Local") {
                    source = &place;
                } else {
                    fprintf(stderr, "%s:%d: unexpected source to eval.\n", parser.file.toLatin1().constData(),
                            parser.line_no);
                }
            }
            ret += source->value(args.at(0));
        }
        break; }
    case E_LIST: {
        static int x = 0;
        QString tmp;
        tmp.sprintf(".QMAKE_INTERNAL_TMP_VAR_%d", x++);
        ret = QStringList(tmp);
        QStringList &lst = (*((QMap<QString, QStringList>*)&place))[tmp];
        lst.clear();
        for(QStringList::ConstIterator arg_it = args.begin();
            arg_it != args.end(); ++arg_it)
            lst += split_value_list((*arg_it));
        break; }
    case E_SPRINTF: {
        if(args.count() < 1) {
            fprintf(stderr, "%s:%d: sprintf(format, ...) requires one argument.\n",
                    parser.file.toLatin1().constData(), parser.line_no);
        } else {
            QString tmp = args.at(0);
            for(int i = 1; i < args.count(); ++i)
                tmp = tmp.arg(args.at(i));
            ret = split_value_list(tmp);
        }
        break; }
    case E_JOIN: {
        if(args.count() < 1 || args.count() > 4) {
            fprintf(stderr, "%s:%d: join(var, glue, before, after) requires four"
                    "arguments.\n", parser.file.toLatin1().constData(), parser.line_no);
        } else {
            QString glue, before, after;
            if(args.count() >= 2)
                glue = args[1];
            if(args.count() >= 3)
                before = args[2];
            if(args.count() == 4)
                after = args[3];
            const QStringList &var = values(args.first(), place);
            if(!var.isEmpty())
                ret = QStringList(before + var.join(glue) + after);
        }
        break; }
    case E_SPLIT: {
        if(args.count() < 1 || args.count() > 2) {
            fprintf(stderr, "%s:%d split(var, sep) requires three arguments\n",
                    parser.file.toLatin1().constData(), parser.line_no);
        } else {
            QString sep = QString(Option::field_sep), join = QString(Option::field_sep);
            if(args.count() >= 2)
                sep = args[1];
            QStringList var = values(args.first(), place);
            for(QStringList::ConstIterator vit = var.begin(); vit != var.end(); ++vit) {
                QStringList lst = (*vit).split(sep);
                for(QStringList::ConstIterator spltit = lst.begin(); spltit != lst.end(); ++spltit)
                    ret += (*spltit);
            }
        }
        break; }
    case E_BASENAME:
    case E_DIRNAME:
    case E_SECTION: {
        bool regexp = false;
        QString sep, var;
        int beg=0, end=-1;
        if(func_t == E_SECTION) {
            if(args.count() != 3 && args.count() != 4) {
                fprintf(stderr, "%s:%d section(var, sep, begin, end) requires three argument\n",
                        parser.file.toLatin1().constData(), parser.line_no);
            } else {
                var = args[0];
                sep = args[1];
                beg = args[2].toInt();
                if(args.count() == 4)
                    end = args[3].toInt();
            }
        } else {
            if(args.count() != 1) {
                fprintf(stderr, "%s:%d %s(var) requires one argument.\n",
                        parser.file.toLatin1().constData(), parser.line_no, func.toLatin1().constData());
            } else {
                var = args[0];
                regexp = true;
                sep = "[" + QRegExp::escape(Option::dir_sep) + "/]";
                if(func_t == E_DIRNAME)
                    end = -2;
                else
                    beg = -1;
            }
        }
        if(!var.isNull()) {
            const QStringList &l = values(var, place);
            for(QStringList::ConstIterator it = l.begin(); it != l.end(); ++it) {
                QString separator = sep;
                if(regexp)
                    ret += (*it).section(QRegExp(separator), beg, end);
                else
                    ret += (*it).section(separator, beg, end);
            }
        }
        break; }
    case E_FIND: {
        if(args.count() != 2) {
            fprintf(stderr, "%s:%d find(var, str) requires two arguments\n",
                    parser.file.toLatin1().constData(), parser.line_no);
        } else {
            QRegExp regx(args[1]);
            const QStringList &var = values(args.first(), place);
            for(QStringList::ConstIterator vit = var.begin();
                vit != var.end(); ++vit) {
                if(regx.indexIn(*vit) != -1)
                    ret += (*vit);
            }
        }
        break;  }
    case E_SYSTEM: {
        if(args.count() < 1 || args.count() > 2) {
            fprintf(stderr, "%s:%d system(execut) requires one argument.\n",
                    parser.file.toLatin1().constData(), parser.line_no);
        } else {
            QMakeProjectEnv env(place);

            char buff[256];
            FILE *proc = QT_POPEN(args[0].toLatin1(), "r");
            bool singleLine = true;
            if(args.count() > 1)
                singleLine = (args[1].toLower() == "true");
            while(proc && !feof(proc)) {
                int read_in = fread(buff, 1, 255, proc);
                if(!read_in)
                    break;
                for(int i = 0; i < read_in; i++) {
                    if((singleLine && buff[i] == '\n') || buff[i] == '\t')
                        buff[i] = ' ';
                }
                buff[read_in] = '\0';
                ret += split_value_list(buff);
            }
        }
        break; }
    case E_UNIQUE: {
        if(args.count() != 1) {
            fprintf(stderr, "%s:%d unique(var) requires one argument.\n",
                    parser.file.toLatin1().constData(), parser.line_no);
        } else {
            const QStringList &var = values(args.first(), place);
            for(int i = 0; i < var.count(); i++) {
                if(!ret.contains(var[i]))
                    ret.append(var[i]);
            }
        }
        break; }
    case E_QUOTE:
        ret = args;
        break;
    case E_ESCAPE_EXPAND: {
        for(int i = 0; i < args.size(); ++i) {
            QChar *i_data = args[i].data();
            int i_len = args[i].length();
            for(int x = 0; x < i_len; ++x) {
                if(*(i_data+x) == '\\' && x < i_len-1) {
                    if(*(i_data+x+1) == '\\') {
                        ++x;
                    } else {
                        struct {
                            char in, out;
                        } mapped_quotes[] = {
                            { 'n', '\n' },
                            { 't', '\t' },
                            { 'r', '\r' },
                            { 0, 0 }
                        };
                        for(int i = 0; mapped_quotes[i].in; ++i) {
                            if(*(i_data+x+1) == mapped_quotes[i].in) {
                                *(i_data+x) = mapped_quotes[i].out;
                                if(x < i_len-2)
                                    memcpy(i_data+x+1, i_data+x+2, i_len-x);
                                --i_len;
                                break;
                            }
                        }
                    }
                }
            }
            ret.append(QString(i_data, i_len));
        }
        break; }
    case E_RE_ESCAPE: {
        for(int i = 0; i < args.size(); ++i)
            ret += QRegExp::escape(args[i]);
        break; }
    case E_UPPER:
    case E_LOWER: {
        for(int i = 0; i < args.size(); ++i) {
            if(func_t == E_UPPER)
                ret += args[i].toUpper();
            else
                ret += args[i].toLower();
        }
        break; }
    case E_FILES: {
        if(args.count() != 1 && args.count() != 2) {
            fprintf(stderr, "%s:%d files(pattern) requires one argument.\n",
                    parser.file.toLatin1().constData(), parser.line_no);
        } else {
            bool recursive = false;
            if(args.count() == 2)
                recursive = (args[1].toLower() == "true" || args[1].toInt());
            QStringList dirs;
            QString r = Option::fixPathToLocalOS(args[0]);
            int slash = r.lastIndexOf(QDir::separator());
            if(slash != -1) {
                dirs.append(r.left(slash));
                r = r.mid(slash+1);
            } else {
                dirs.append(qmake_getpwd());
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
                    if(regex.exactMatch(fname))
                        ret += fname;
                }
            }
        }
        break; }
    case E_PROMPT: {
        if(args.count() != 1) {
            fprintf(stderr, "%s:%d prompt(question) requires one argument.\n",
                    parser.file.toLatin1().constData(), parser.line_no);
        } else if(projectFile() == "-") {
            fprintf(stderr, "%s:%d prompt(question) cannot be used when '-o -' is used.\n",
                    parser.file.toLatin1().constData(), parser.line_no);
        } else {
            QString msg = fixEnvVariables(args.first());
            if(!msg.endsWith("?"))
                msg += "?";
            fprintf(stderr, "Project %s: %s ", func.toUpper().toLatin1().constData(),
                    msg.toLatin1().constData());

            QFile qfile;
            if(qfile.open(stdin, QIODevice::ReadOnly)) {
                QTextStream t(&qfile);
                ret = split_value_list(t.readLine());
            }
        }
        break; }
    case E_REPLACE: {
        if(args.count() != 3 ) {
            fprintf(stderr, "%s:%d replace(var, before, after) requires three arguments\n",
                    parser.file.toLatin1().constData(), parser.line_no);
        } else {
            const QRegExp before( args[1] );
            const QString after( args[2] );
            QStringList var = values(args.first(), place);
            for(QStringList::Iterator it = var.begin(); it != var.end(); ++it)
                ret += it->replace(before, after);
        }
        break; }
    default: {
        if(replaceFunctions.contains(func)) {
            FunctionBlock *defined = replaceFunctions[func];
            function_blocks.push(defined);
            defined->exec(args, this, place, ret);
            Q_ASSERT(function_blocks.pop() == defined);
        } else {
            fprintf(stderr, "%s:%d: Unknown replace function: %s\n",
                    parser.file.toLatin1().constData(), parser.line_no,
                    func.toLatin1().constData());
        }
        break; }
    }
    return ret;
}

bool
QMakeProject::doProjectTest(QString func, QStringList args, QMap<QString, QStringList> &place)
{
    func = func.trimmed();
    for(QStringList::Iterator arit = args.begin(); arit != args.end(); ++arit) {
        (*arit) = (*arit).trimmed(); // blah, get rid of space
        doVariableReplace((*arit), place);
    }
    debug_msg(1, "Running project test: %s(%s)", func.toLatin1().constData(), args.join("::").toLatin1().constData());

    if(func == "requires") {
        return doProjectCheckReqs(args, place);
    } else if(func == "greaterThan" || func == "lessThan") {
        if(args.count() != 2) {
            fprintf(stderr, "%s:%d: %s(variable, value) requires two arguments.\n", parser.file.toLatin1().constData(),
                    parser.line_no, func.toLatin1().constData());
            return false;
        }
        QString rhs(args[1]), lhs(values(args[0], place).join(QString(Option::field_sep)));
        bool ok;
        int rhs_int = rhs.toInt(&ok);
        if(ok) { // do integer compare
            int lhs_int = lhs.toInt(&ok);
            if(ok) {
                if(func == "greaterThan")
                    return lhs_int > rhs_int;
                return lhs_int < rhs_int;
            }
        }
        if(func == "greaterThan")
            return lhs > rhs;
        return lhs < rhs;
    } else if(func == "equals" || func == "isEqual") {
        if(args.count() != 2) {
            fprintf(stderr, "%s:%d: %s(variable, value) requires two arguments.\n", parser.file.toLatin1().constData(),
                    parser.line_no, func.toLatin1().constData());
            return false;
        }
        return values(args[0], place).join(QString(Option::field_sep)) == args[1];
    } else if(func == "exists") {
        if(args.count() != 1) {
            fprintf(stderr, "%s:%d: exists(file) requires one argument.\n", parser.file.toLatin1().constData(),
                    parser.line_no);
            return false;
        }
        QString file = args.first();
        file = Option::fixPathToLocalOS(file);

        if(QFile::exists(file))
            return true;
        //regular expression I guess
        QString dirstr = qmake_getpwd();
        int slsh = file.lastIndexOf(Option::dir_sep);
        if(slsh != -1) {
            dirstr = file.left(slsh+1);
            file = file.right(file.length() - slsh - 1);
        }
        return QDir(dirstr).entryList(QStringList(file)).count();
    } else if(func == "export") {
        if(args.count() != 1) {
            fprintf(stderr, "%s:%d: export(variable) requires one argument.\n", parser.file.toLatin1().constData(),
                    parser.line_no);
            return false;
        }
        for(int i = 0; i < function_blocks.size(); ++i) {
            FunctionBlock *f = function_blocks.at(i);
            f->vars[args[0]] = values(args[0], place);
            if(!i && f->calling_place)
                (*f->calling_place)[args[0]] = values(args[0], place);
        }
        return true;
    } else if(func == "clear") {
        if(args.count() != 1) {
            fprintf(stderr, "%s:%d: clear(variable) requires one argument.\n", parser.file.toLatin1().constData(),
                    parser.line_no);
            return false;
        }
        if(!place.contains(args[0]))
            return false;
        place[args[0]].clear();
        return true;
    } else if(func == "unset") {
        if(args.count() != 1) {
            fprintf(stderr, "%s:%d: unset(variable) requires one argument.\n", parser.file.toLatin1().constData(),
                    parser.line_no);
            return false;
        }
        if(!place.contains(args[0]))
            return false;
        place.remove(args[0]);
        return true;
    } else if(func == "eval") {
        if(args.count() < 1 && 0) {
            fprintf(stderr, "%s:%d: eval(project) requires one argument.\n", parser.file.toLatin1().constData(),
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
            fprintf(stderr, "%s:%d: CONFIG(config) requires one argument.\n", parser.file.toLatin1().constData(),
                    parser.line_no);
            return false;
        }
        if(args.count() == 1)
            return isActiveConfig(args[0]);
        const QStringList mutuals = args[1].split('|');
        const QStringList &configs = values("CONFIG", place);
        for(int i = configs.size()-1; i >= 0; i--) {
            for(int mut = 0; mut < mutuals.count(); mut++) {
                if(configs[i] == mutuals[mut].trimmed())
                    return (configs[i] == args[0]);
            }
        }
        return false;
    } else if(func == "system") {
        if(args.count() != 1) {
            fprintf(stderr, "%s:%d: system(exec) requires one argument.\n", parser.file.toLatin1().constData(),
                    parser.line_no);
            return false;
        }
        QMakeProjectEnv env(place);
        bool ret = system(args.first().toLatin1().constData()) == 0;
        return ret;
    } else if(func == "return") {
        if(function_blocks.isEmpty()) {
            fprintf(stderr, "%s:%d unexpected return()\n",
                    parser.file.toLatin1().constData(), parser.line_no);
        } else {
            FunctionBlock *f = function_blocks.top();
            f->cause_return = true;
            if(args.count() >= 1)
                f->return_value = args[0];
        }
        return true;
    } else if(func == "break") {
        if(iterator)
            iterator->cause_break = true;
        else if(!scope_blocks.isEmpty())
            scope_blocks.top().ignore = true;
        else
            fprintf(stderr, "%s:%d unexpected break()\n",
                    parser.file.toLatin1().constData(), parser.line_no);
        return true;
    } else if(func == "next") {
        if(iterator)
            iterator->cause_next = true;
        else
            fprintf(stderr, "%s:%d unexpected next()\n",
                    parser.file.toLatin1().constData(), parser.line_no);
        return true;
    } else if(func == "defined") {
        if(args.count() < 1 || args.count() > 2) {
            fprintf(stderr, "%s:%d: defined(function) requires one argument.\n",
                    parser.file.toLatin1().constData(), parser.line_no);
        } else {
           if(args.count() > 1) {
               if(args[1] == "test")
                   return testFunctions.contains(args[0]);
               else if(args[1] == "replace")
                   return replaceFunctions.contains(args[0]);
               fprintf(stderr, "%s:%d: defined(function, type): unexpected type [%s].\n",
                       parser.file.toLatin1().constData(), parser.line_no,
                       args[1].toLatin1().constData());
            } else {
                if(replaceFunctions.contains(args[0]) || testFunctions.contains(args[0]))
                    return true;
            }
        }
        return false;
    } else if(func == "contains") {
        if(args.count() < 2 || args.count() > 3) {
            fprintf(stderr, "%s:%d: contains(var, val) requires at lesat 2 arguments.\n",
                    parser.file.toLatin1().constData(), parser.line_no);
            return false;
        }
        QRegExp regx(args[1]);
        const QStringList &l = values(args[0], place);
        if(args.count() == 2) {
            for(int i = 0; i < l.size(); ++i) {
                const QString val = l[i];
                if(regx.exactMatch(val) || val == args[1])
                    return true;
            }
        } else {
            const QStringList mutuals = args[2].split('|');
            for(int i = l.size()-1; i >= 0; i--) {
                const QString val = l[i];
                for(int mut = 0; mut < mutuals.count(); mut++) {
                    if(val == mutuals[mut].trimmed())
                        return (regx.exactMatch(val) || val == args[1]);
                }
            }
        }
        return false;
    } else if(func == "infile") {
        if(args.count() < 2 || args.count() > 3) {
            fprintf(stderr, "%s:%d: infile(file, var, val) requires at least 2 arguments.\n",
                    parser.file.toLatin1().constData(), parser.line_no);
            return false;
        }

        bool ret = false;
        QMap<QString, QStringList> tmp;
        if(doProjectInclude(Option::fixPathToLocalOS(args[0]), IncludeFlagNewParser, tmp) == IncludeSuccess) {
            if(tmp.contains("QMAKE_INTERNAL_INCLUDED_FILES")) {
                QStringList &out = place["QMAKE_INTERNAL_INCLUDED_FILES"];
                const QStringList &in = tmp["QMAKE_INTERNAL_INCLUDED_FILES"];
                for(int i = 0; i < in.size(); ++i) {
                    if(out.indexOf(in[i]) == -1)
                        out += in[i];
                }
            }
            if(args.count() == 2) {
                ret = tmp.contains(args[1]);
            } else {
                QRegExp regx(args[2]);
                const QStringList &l = tmp[args[1]];
                for(QStringList::ConstIterator it = l.begin(); it != l.end(); ++it) {
                    if(regx.exactMatch((*it)) || (*it) == args[2]) {
                        ret = true;
                        break;
                    }
                }
            }
        }
        return ret;
    } else if(func == "count") {
        if(args.count() != 2 && args.count() != 3) {
            fprintf(stderr, "%s:%d: count(var, count) requires two arguments.\n", parser.file.toLatin1().constData(),
                    parser.line_no);
            return false;
        }
        if(args.count() == 3) {
            QString comp = args[2];
            if(comp == ">" || comp == "greaterThan")
                return values(args[0], place).count() > args[1].toInt();
            if(comp == ">=")
                return values(args[0], place).count() >= args[1].toInt();
            if(comp == "<" || comp == "lessThan")
                return values(args[0], place).count() < args[1].toInt();
            if(comp == "<=")
                return values(args[0], place).count() <= args[1].toInt();
            if(comp == "equals" || comp == "isEqual" || comp == "=" || comp == "==")
                return values(args[0], place).count() == args[1].toInt();
            fprintf(stderr, "%s:%d: unexpected modifier to count(%s)\n", parser.file.toLatin1().constData(),
                    parser.line_no, comp.toLatin1().constData());
            return false;
        }
        return values(args[0], place).count() == args[1].toInt();
    } else if(func == "isEmpty") {
        if(args.count() != 1) {
            fprintf(stderr, "%s:%d: isEmpty(var) requires one argument.\n", parser.file.toLatin1().constData(),
                    parser.line_no);
            return false;
        }
        return values(args[0], place).isEmpty();
    } else if(func == "include" || func == "load") {
        QString parseInto;
        const bool include_statement = (func == "include");
        bool ignore_error = include_statement;
        if(args.count() == 2) {
            if(func == "include") {
                parseInto = args[1];
            } else {
                QString sarg = args[1];
                ignore_error = (sarg.toLower() == "true" || sarg.toInt());
            }
        } else if(args.count() != 1) {
            QString func_desc = "load(feature)";
            if(include_statement)
                func_desc = "include(file)";
            fprintf(stderr, "%s:%d: %s requires one argument.\n", parser.file.toLatin1().constData(),
                    parser.line_no, func_desc.toLatin1().constData());
            return false;
        }
        QString file = args.first();
        file = Option::fixPathToLocalOS(file);
        uchar flags = IncludeFlagNone;
        if(!include_statement)
            flags |= IncludeFlagFeature;
        IncludeStatus stat = IncludeFailure;
        if(!parseInto.isEmpty()) {
            QMap<QString, QStringList> symbols;
            stat = doProjectInclude(file, flags|IncludeFlagNewProject, symbols);
            if(stat == IncludeSuccess) {
                QMap<QString, QStringList> out_place;
                for(QMap<QString, QStringList>::ConstIterator it = place.begin(); it != place.end(); ++it) {
                    const QString var = it.key();
                    if(var != parseInto && !var.startsWith(parseInto + "."))
                        out_place.insert(var, it.value());
                }
                for(QMap<QString, QStringList>::ConstIterator it = symbols.begin(); it != symbols.end(); ++it) {
                    const QString var = it.key();
                    if(!var.startsWith("."))
                        out_place.insert(parseInto + "." + it.key(), it.value());
                }
                place = out_place;
            }
        } else {
            stat = doProjectInclude(file, flags, place);
        }
        if(stat == IncludeFeatureAlreadyLoaded) {
            warn_msg(WarnParser, "%s:%d: Duplicate of loaded feature %s",
                     parser.file.toLatin1().constData(), parser.line_no, file.toLatin1().constData());
        } else if(stat == IncludeNoExist && include_statement) {
            warn_msg(WarnParser, "%s:%d: Unable to find file for inclusion %s",
                     parser.file.toLatin1().constData(), parser.line_no, file.toLatin1().constData());
            return false;
        } else if(stat >= IncludeFailure) {
            if(!ignore_error) {
                printf("Project LOAD(): Feature %s cannot be found.\n", file.toLatin1().constData());
                if (!ignore_error)
#if defined(QT_BUILD_QMAKE_LIBRARY)
                    return false;
#else
                    exit(3);
#endif
            }
            return false;
        }
        return true;
    } else if(func == "debug") {
        if(args.count() != 2) {
            fprintf(stderr, "%s:%d: debug(level, message) requires one argument.\n", parser.file.toLatin1().constData(),
                    parser.line_no);
            return false;
        }
        QString msg = fixEnvVariables(args[1]);
        debug_msg(args[0].toInt(), "Project DEBUG: %s", msg.toLatin1().constData());
        return true;
    } else if(func == "error" || func == "message" || func == "warning") {
        if(args.count() != 1) {
            fprintf(stderr, "%s:%d: %s(message) requires one argument.\n", parser.file.toLatin1().constData(),
                    parser.line_no, func.toLatin1().constData());
            return false;
        }
        QString msg = fixEnvVariables(args.first());
        fprintf(stderr, "Project %s: %s\n", func.toUpper().toLatin1().constData(), msg.toLatin1().constData());
        if(func == "error")
#if defined(QT_BUILD_QMAKE_LIBRARY)
            return false;
#else
            exit(2);
#endif
        return true;
    } else if(testFunctions.contains(func)) {
        FunctionBlock *defined = testFunctions[func];
        QStringList ret;
        function_blocks.push(defined);
        defined->exec(args, this, place, ret);
        Q_ASSERT(function_blocks.pop() == defined);

        if(ret.isEmpty()) {
            return true;
        } else {
            if(ret.first() == "true") {
                return true;
            } else if(ret.first() == "false") {
                return false;
            } else {
                bool ok;
                int val = ret.first().toInt(&ok);
                if(ok)
                    return val;
                fprintf(stderr, "%s:%d Unexpected return value from test %s [%s].\n",
                        parser.file.toLatin1().constData(),
                        parser.line_no, func.toLatin1().constData(),
                        ret.join("::").toLatin1().constData());
            }
            return false;
        }
        return false;
    } else {
        fprintf(stderr, "%s:%d: Unknown test function: %s\n", parser.file.toLatin1().constData(), parser.line_no,
                func.toLatin1().constData());
    }
    return false;
}

bool
QMakeProject::doProjectCheckReqs(const QStringList &deps, QMap<QString, QStringList> &place)
{
    bool ret = false;
    for(QStringList::ConstIterator it = deps.begin(); it != deps.end(); ++it) {
        bool test = doProjectTest((*it), place);
        if(!test) {
            debug_msg(1, "Project Parser: %s:%d Failed test: REQUIRES = %s",
                      parser.file.toLatin1().constData(), parser.line_no,
                      (*it).toLatin1().constData());
            place["QMAKE_FAILED_REQUIREMENTS"].append((*it));
            ret = false;
        }
    }
    return ret;
}

bool
QMakeProject::test(const QString &v)
{
    QMap<QString, QStringList> tmp = vars;
    return doProjectTest(v, tmp);
}

bool
QMakeProject::test(const QString &func, const QStringList &args)
{
    QMap<QString, QStringList> tmp = vars;
    return doProjectTest(func, args, tmp);
}

QString
QMakeProject::expand(const QString &str)
{
    QMap<QString, QStringList> tmp = vars;
    QString ret = str;
    if(!doVariableReplace(ret, tmp))
        return str;
    return ret;
}

bool
QMakeProject::doVariableReplace(QString &str, QMap<QString, QStringList> &place)
{
    bool ret;
    str = doVariableReplaceExpand(str, place, &ret).join(QString(Option::field_sep));
    return ret;
}

QStringList
QMakeProject::doVariableReplaceExpand(const QString &str, QMap<QString, QStringList> &place, bool *ok)
{
    QStringList ret;
    if(ok)
        *ok = true;
    if(str.isEmpty())
        return ret;

    const ushort LSQUARE = '[';
    const ushort RSQUARE = ']';
    const ushort LCURLY = '{';
    const ushort RCURLY = '}';
    const ushort LPAREN = '(';
    const ushort RPAREN = ')';
    const ushort DOLLAR = '$';
    const ushort SLASH = '\\';
    const ushort UNDERSCORE = '_';
    const ushort DOT = '.';
    const ushort SPACE = ' ';
    const ushort TAB = '\t';
    const ushort SINGLEQUOTE = '\'';
    const ushort DOUBLEQUOTE = '"';

    ushort unicode, quote = 0;
    const QChar *str_data = str.data();
    const int str_len = str.length();

    ushort term;
    QString var, args;

    int replaced = 0;
    QString current;
    for(int i = 0; i < str_len; ++i) {
        unicode = (str_data+i)->unicode();
        const int start_var = i;
        if(unicode == DOLLAR && str_len > i+2) {
            unicode = (str_data+(++i))->unicode();
            if(unicode == DOLLAR) {
                term = 0;
                var.clear();
                args.clear();
                enum { VAR, ENVIRON, FUNCTION, PROPERTY } var_type = VAR;
                unicode = (str_data+(++i))->unicode();
                if(unicode == LSQUARE) {
                    unicode = (str_data+(++i))->unicode();
                    term = RSQUARE;
                    var_type = PROPERTY;
                } else if(unicode == LCURLY) {
                    unicode = (str_data+(++i))->unicode();
                    var_type = VAR;
                    term = RCURLY;
                } else if(unicode == LPAREN) {
                    unicode = (str_data+(++i))->unicode();
                    var_type = ENVIRON;
                    term = RPAREN;
                }
                while(1) {
                    if(!(unicode & (0xFF<<8)) &&
                       unicode != DOT && unicode != UNDERSCORE &&
                       //unicode != SINGLEQUOTE && unicode != DOUBLEQUOTE &&
                       (unicode < 'a' || unicode > 'z') && (unicode < 'A' || unicode > 'Z') &&
                       (unicode < '0' || unicode > '9'))
                        break;
                    var.append(QChar(unicode));
                    if(++i == str_len)
                        break;
                    unicode = (str_data+i)->unicode();
                    // at this point, i points to either the 'term' or 'next' character (which is in unicode)
                }
                if(var_type == VAR && unicode == LPAREN) {
                    var_type = FUNCTION;
                    int depth = 0;
                    while(1) {
                        if(++i == str_len)
                            break;
                        unicode = (str_data+i)->unicode();
                        if(unicode == LPAREN) {
                            depth++;
                        } else if(unicode == RPAREN) {
                            if(!depth)
                                break;
                            --depth;
                        }
                        args.append(QChar(unicode));
                    }
                    if(++i < str_len)
                        unicode = (str_data+(i))->unicode();
                    else
                        unicode = 0;
                    // at this point i is pointing to the 'next' character (which is in unicode)
                    // this might actually be a term character since you can do $${func()}
                }
                if(term) {
                    if(unicode != term) {
                        qmake_error_msg("Missing " + QString(term) + " terminator [found " + (unicode?QString(unicode):QString("end-of-line")) + "]");
                        if(ok)
                            *ok = false;
                        return QStringList();
                    }
                } else {
                    // move the 'cursor' back to the last char of the thing we were looking at
                    --i;
                }
                // since i never points to the 'next' character, there is no reason for this to be set
                unicode = 0;

                QStringList replacement;
                if(var_type == ENVIRON) {
                    replacement = QStringList(QString::fromLocal8Bit(qgetenv(var.toLatin1().constData())));
                } else if(var_type == PROPERTY) {
                    if(prop)
                        replacement = QStringList(prop->value(var));
                } else if(var_type == FUNCTION) {
                    replacement = doProjectExpand(var, args, place);
                } else if(var_type == VAR) {
                    replacement = values(var, place);
                }
                if(!(replaced++) && start_var)
                    current = str.left(start_var);
                if(!replacement.isEmpty()) {
                    if(quote) {
                        current += replacement.join(QString(Option::field_sep));
                    } else {
                        current += replacement.takeFirst();
                        if(!replacement.isEmpty()) {
                            if(!current.isEmpty())
                                ret.append(current);
                            current = replacement.takeLast();
                            if(!replacement.isEmpty())
                                ret += replacement;
                        }
                    }
                }
                debug_msg(2, "Project Parser [var replace]: %s -> %s",
                          str.toLatin1().constData(), var.toLatin1().constData(),
                          replacement.join("::").toLatin1().constData());
            } else {
                if(replaced)
                    current.append("$");
            }
        }
        if(quote && unicode == quote) {
            unicode = 0;
            quote = 0;
        } else if(unicode == SLASH) {
            bool escape = false;
            const char *symbols = "[]{}()$\\'\"";
            for(const char *s = symbols; *s; ++s) {
                if(*(str_data+i+1) == (ushort)*s) {
                    i++;
                    escape = true;
                    if(!(replaced++))
                        current = str.left(start_var);
                    current.append(str.at(i));
                    break;
                }
            }
            if(escape || !replaced)
                unicode =0;
        } else if(!quote && (unicode == SINGLEQUOTE || unicode == DOUBLEQUOTE)) {
            quote = unicode;
            unicode = 0;
            if(!(replaced++) && i)
                current = str.left(i);
        } else if(!quote && (unicode == SPACE || unicode == TAB)) {
            unicode = 0;
            if(!current.isEmpty()) {
                ret.append(current);
                current.clear();
            }
        }
        if(replaced && unicode)
            current.append(QChar(unicode));
    }
    if(!replaced)
        ret = QStringList(str);
    else if(!current.isEmpty())
        ret.append(current);
    //qDebug() << "REPLACE" << str << ret;
    return ret;
}

QStringList &QMakeProject::values(const QString &_var, QMap<QString, QStringList> &place)
{
    QString var = varMap(_var);
    if(var == QLatin1String("LITERAL_WHITESPACE")) { //a real space in a token)
        var = ".BUILTIN." + var;
        place[var] = QStringList(QLatin1String("\t"));
    } else if(var == QLatin1String("LITERAL_DOLLAR")) { //a real $
        var = ".BUILTIN." + var;
        place[var] = QStringList(QLatin1String("$"));
    } else if(var == QLatin1String("LITERAL_HASH")) { //a real #
        var = ".BUILTIN." + var;
        place[var] = QStringList("#");
    } else if(var == QLatin1String("OUT_PWD")) { //the out going dir
        var = ".BUILTIN." + var;
        place[var] =  QStringList(Option::output_dir);
    } else if(var == QLatin1String("PWD") ||  //current working dir (of _FILE_)
              var == QLatin1String("IN_PWD")) {
        var = ".BUILTIN." + var;
        place[var] = QStringList(qmake_getpwd());
    } else if(var == QLatin1String("DIR_SEPARATOR")) {
        var = ".BUILTIN." + var;
        place[var] =  QStringList(Option::dir_sep);
    } else if(var == QLatin1String("DIRLIST_SEPARATOR")) {
        var = ".BUILTIN." + var;
        place[var] = QStringList(Option::dirlist_sep);
    } else if(var == QLatin1String("_LINE_")) { //parser line number
        var = ".BUILTIN." + var;
        place[var] = QStringList(QString::number(parser.line_no));
    } else if(var == QLatin1String("_FILE_")) { //parser file
        var = ".BUILTIN." + var;
        place[var] = QStringList(parser.file);
    } else if(var == QLatin1String("_DATE_")) { //current date/time
        var = ".BUILTIN." + var;
        place[var] = QStringList(QDateTime::currentDateTime().toString());
    } else if(var == QLatin1String("_PRO_FILE_")) {
        var = ".BUILTIN." + var;
        place[var] = QStringList(pfile);
    } else if(var == QLatin1String("_PRO_FILE_PWD_")) {
        var = ".BUILTIN." + var;
        place[var] =  QStringList(QFileInfo(pfile).absolutePath());
    } else if(var == QLatin1String("_QMAKE_CACHE_")) {
        var = ".BUILTIN." + var;
        if(Option::mkfile::do_cache)
            place[var] = QStringList(Option::mkfile::cachefile);
    } else if(var == QLatin1String("TEMPLATE")) {
        if(!Option::user_template.isEmpty()) {
            var = ".BUILTIN.USER." + var;
            place[var] =  QStringList(Option::user_template);
        } else if(!place[var].isEmpty()) {
            QString orig_template = place["TEMPLATE"].first(), real_template;
            if(!Option::user_template_prefix.isEmpty() && !orig_template.startsWith(Option::user_template_prefix))
                real_template = Option::user_template_prefix + orig_template;
            if(real_template.endsWith(".t"))
                real_template = real_template.left(real_template.length()-2);
            if(!real_template.isEmpty()) {
                var = ".BUILTIN." + var;
                place[var] = QStringList(real_template);
            }
        } else {
            var = ".BUILTIN." + var;
            place[var] =  QStringList("app");
        }
    } else if(var.startsWith(QLatin1String("QMAKE_HOST."))) {
        QString ret, type = var.mid(11);
#if defined(Q_OS_WIN32)
        if(type == "os") {
            ret = "Windows";
        } else if(type == "name") {
            DWORD name_length = 1024;
            TCHAR name[1024];
            if(GetComputerName(name, &name_length))
                ret = QString::fromUtf16((ushort*)name, name_length);
        } else if(type == "version" || type == "version_string") {
            QSysInfo::WinVersion ver = QSysInfo::WindowsVersion;
            if(type == "version")
                ret = QString::number(ver);
            else if(ver == QSysInfo::WV_Me)
                ret = "WinMe";
            else if(ver == QSysInfo::WV_95)
                ret = "Win95";
            else if(ver == QSysInfo::WV_98)
                ret = "Win98";
            else if(ver == QSysInfo::WV_NT)
                ret = "WinNT";
            else if(ver == QSysInfo::WV_2000)
                ret = "Win2000";
            else if(ver == QSysInfo::WV_2000)
                ret = "Win2003";
            else if(ver == QSysInfo::WV_XP)
                ret = "WinXP";
            else if(ver == QSysInfo::WV_VISTA)
                ret = "WinVista";
            else
                ret = "Unknown";
        } else if(type == "arch") {
            SYSTEM_INFO info;
            GetSystemInfo(&info);
            switch(info.wProcessorArchitecture) {
#ifdef PROCESSOR_ARCHITECTURE_AMD64
            case PROCESSOR_ARCHITECTURE_AMD64:
                ret = "x86_64";
                break;
#endif
            case PROCESSOR_ARCHITECTURE_INTEL:
                ret = "x86";
                break;
            case PROCESSOR_ARCHITECTURE_IA64:
#ifdef PROCESSOR_ARCHITECTURE_IA32_ON_WIN64
            case PROCESSOR_ARCHITECTURE_IA32_ON_WIN64:
#endif
                ret = "IA64";
                break;
            default:
                ret = "Unknown";
                break;
            }
        }
#elif defined(Q_OS_UNIX)
        struct utsname name;
        if(!uname(&name)) {
            if(type == "os")
                ret = name.sysname;
            else if(type == "name")
                ret = name.nodename;
            else if(type == "version")
                ret = name.release;
            else if(type == "version_string")
                ret = name.version;
            else if(type == "arch")
                ret = name.machine;
        }
#endif
        var = ".BUILTIN.HOST." + type;
        place[var] = QStringList(ret);
    }
    //qDebug("REPLACE [%s]->[%s]", qPrintable(var), qPrintable(place[var].join("::")));
    return place[var];
}

