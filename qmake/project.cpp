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
#include <qfileinfo.h>
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

static QString remove_quotes(const QString &arg)
{
    static bool symbols_init = false;
    enum { SINGLEQUOTE, DOUBLEQUOTE };
    static ushort symbols[2];
    if(!symbols_init) {
        symbols_init = true;
        symbols[SINGLEQUOTE] = QChar('\'').unicode();
        symbols[DOUBLEQUOTE] = QChar('"').unicode();
    }

    const QChar *arg_data = arg.data();
    const ushort first = arg_data->unicode();
    const int arg_len = arg.length();
    if(first == symbols[SINGLEQUOTE] || first == symbols[DOUBLEQUOTE]) {
        const ushort last = (arg_data+arg_len-1)->unicode();
        if(last == first)
            return arg.mid(1, arg_len-2);
    }
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
    bool eval(QMakeProject *p, QMap<QString, QStringList> &place);
};

bool ParsableBlock::eval(QMakeProject *p, QMap<QString, QStringList> &place)
{
    //save state
    parser_info pi = ::parser;

    //execute
    bool ret = true;
    for(int i = 0; i < parser.count(); i++) {
        ::parser = parser.at(i).pi;
        if(!(ret = p->parse(parser.at(i).text, place)) || !continueBlock())
            break;
    }

    //restore state
    ::parser = pi;
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
              QMakeProject *p, QMap<QString, QStringList> &place, QString &functionReturn);
    virtual bool continueBlock() { return !cause_return; }
};

bool FunctionBlock::exec(const QStringList &args,
                         QMakeProject *proj, QMap<QString, QStringList> &place, QString &functionReturn)
{
    //save state
#if 0
    calling_place = &place;
#else
    calling_place = &proj->variables();
#endif
    return_value = "";
    cause_return = false;

    //execute
    vars = proj->variables();
    vars["ARGS"] = args;
    for(int i = 0; i < args.count(); i++)
        vars[QString::number(i+1)] = QStringList(args[i]);
    bool ret = ParsableBlock::eval(proj, vars);
    functionReturn = return_value;

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
   7) environment variable QTDIR/mkspecs/FEATURES_DIR
   8) your QMAKESPEC/../FEATURES_DIR dir

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
    if(!Option::mkfile::qmakespec.isEmpty())
        feature_roots << Option::mkfile::qmakespec + QDir::separator() + "features";
    if(const char *qtdir = qgetenv("QTDIR")) {
        for(QStringList::Iterator concat_it = concat.begin();
            concat_it != concat.end(); ++concat_it)
            feature_roots << (QString(qtdir) + mkspecs_concat + (*concat_it));
    }
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
    ret << QLibraryInfo::location(QLibraryInfo::PrefixPath) + concat;
    ret << QLibraryInfo::location(QLibraryInfo::DataPath) + concat;

    // prefer $QTDIR if it is set
    if(qgetenv("QTDIR"))
        ret << qgetenv("QTDIR");
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
    int quote = 0;
    QStringList args;

    static bool symbols_init = false;
    enum { LPAREN, RPAREN, SINGLEQUOTE, DOUBLEQUOTE, COMMA, SPACE, TAB  };
    static ushort symbols[7];
    if(!symbols_init) {
        symbols_init = true;
        symbols[LPAREN] = QChar('(').unicode();
        symbols[RPAREN] = QChar(')').unicode();
        symbols[SINGLEQUOTE] = QChar('\'').unicode();
        symbols[DOUBLEQUOTE] = QChar('"').unicode();
        symbols[COMMA] = QChar(',').unicode();
        symbols[SPACE] = QChar(' ').unicode();
        symbols[TAB] = QChar('\t').unicode();
    }

    ushort unicode;
    const QChar *params_data = params.data();
    const int params_len = params.length();
    int last = 0;
    while(last < params_len && ((params_data+last)->unicode() == symbols[SPACE]
                                /*|| (params_data+last)->unicode() == symbols[TAB]*/))
        ++last;
    for(int x = last, parens = 0; x <= params_len; x++) {
        unicode = (params_data+x)->unicode();
        if(x == params_len) {
            while(x && (params_data+(x-1))->unicode() == symbols[SPACE])
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
        if(unicode == symbols[LPAREN]) {
            --parens;
        } else if(unicode == symbols[RPAREN]) {
            ++parens;
        } else if(quote && unicode == quote) {
            quote = 0;
        } else if(!quote && (unicode == symbols[SINGLEQUOTE] || unicode == symbols[DOUBLEQUOTE])) {
            quote = unicode;
        } else if(!parens && !quote && unicode == symbols[COMMA]) {
            QString mid = params.mid(last, x - last).trimmed();
            if(quote) {
                if(mid[0] == quote && mid[(int)mid.length()-1] == quote)
                    mid = mid.mid(1, mid.length()-2);
                quote = 0;
            }
            args << mid;
            last = x+1;
            while(last < params_len && ((params_data+last)->unicode() == symbols[SPACE]
                                        /*|| (params_data+last)->unicode() == symbols[TAB]*/))
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

    static bool symbols_init = false;
    enum { LPAREN, RPAREN, SINGLEQUOTE, DOUBLEQUOTE, SLASH, SEMICOLON };
    static ushort symbols[6];
    if(!symbols_init) {
        symbols_init = true;
        symbols[LPAREN] = QChar('(').unicode();
        symbols[RPAREN] = QChar(')').unicode();
        symbols[SINGLEQUOTE] = QChar('\'').unicode();
        symbols[DOUBLEQUOTE] = QChar('"').unicode();
        symbols[SLASH] = QChar('\\').unicode();
        symbols[SEMICOLON] = QChar(';').unicode();
    }


    ushort unicode;
    const QChar *vals_data = vals.data();
    const int vals_len = vals.length();
    for(int x = 0, parens = 0; x < vals_len; x++) {
        unicode = (vals_data+x)->unicode();
        if(x != (int)vals_len-1 && unicode == symbols[SLASH] &&
           ((vals_data+(x+1))->unicode() == '\'' || (vals_data+(x+1))->unicode() == symbols[DOUBLEQUOTE])) {
            build += *(vals_data+(x++)); //get that 'escape'
        } else if(!quote.isEmpty() && unicode == quote.top()) {
            quote.pop();
        } else if(unicode == symbols[SINGLEQUOTE] || unicode == symbols[DOUBLEQUOTE]) {
            quote.push(unicode);
        } else if(unicode == symbols[RPAREN]) {
            --parens;
        } else if(unicode == symbols[LPAREN]) {
            ++parens;
        }

        if(!parens && quote.isEmpty() && ((do_semicolon && unicode == symbols[SEMICOLON]) ||
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
                if(sb.iterate)
                    sb.iterate->exec(this, place);
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
        QByteArray dd = s.toLatin1();
        const char *d = dd.constData();
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
        QByteArray dd = s.toLatin1();
        const char *d = dd;
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
            bool ret = it->exec(this, place);
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
    QByteArray dd = s.toLatin1();
    const char *d = dd;
    SKIP_WS(d);
    IteratorBlock *iterator = 0;
    bool scope_failed = false, else_line = false, or_op=false;
    char quote = 0;
    int parens = 0, scope_count=0, start_block = 0;
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
#if defined(_MSC_VER) && _MSC_VER >= 1400
                            sprintf_s(error.data(), 256, "Function missing right paren: %s ('%s')",
                                    comp_scope.toLatin1().constData(), s.toLatin1().constData());
#else
                            sprintf(error.data(), "Function missing right paren: %s ('%s')",
                                    comp_scope.toLatin1().constData(), s.toLatin1().constData());
#endif
                            qmake_error_msg(error);
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
                debug_msg(1, "Project Parser: %s:%d : Test (%s) failed.", parser.file.toLatin1().constData(),
                          parser.line_no, scope.toLatin1().constData());
            if(test == or_op)
                scope_failed = !test;
            or_op = (*d == '|');

            if(*d == '{') { // scoping block
                start_block++;
                if(iterator) {
                    for(int off = 0, braces = 0; true; ++off) {
                        if(*(d+off) == '{')
                            ++braces;
                        else if(*(d+off) == '}' && braces)
                            --braces;
                        if(!braces || !*(d+off)) {
                            iterator->parser.append(QString(QByteArray(d+1, off-1)));
                            if(braces > 1)
                                iterator->scope_level += braces-1;
                            d += off-1;
                            break;
                        }
                    }
                }
            }
        } else if(!parens && *d == '}') {
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
            var += *d;
        }
        d++;
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
        debug_msg(1, "Project Parser: %s:%d : Entering block %d (%d).", parser.file.toLatin1().constData(),
                  parser.line_no, scope_blocks.count(), scope_failed);
    } else if(iterator) {
        iterator->parser.append(var+QString(d));
        bool ret = iterator->exec(this, place);
        delete iterator;
        return ret;
    }

    if((!scope_count && !var.isEmpty()) || (scope_count == 1 && else_line))
        scope_blocks.top().else_status = ScopeBlock::TestNone;
    if(!*d) {
        if(!var.trimmed().isEmpty())
            qmake_error_msg(("Parse Error ('" + s + "')").toLatin1());
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

    doVariableReplace(vals, place);
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

    // vallist is the broken up list of values
    QStringList vallist = split_value_list(vals, (var == "DEPENDPATH" || var == "INCLUDEPATH"));
    if(!vallist.filter("=").isEmpty())
        warn_msg(WarnParser, "Detected possible line continuation: {%s} %s:%d",
                 var.toLatin1().constData(), parser.file.toLatin1().constData(), parser.line_no);

    QStringList &varlist = place[var]; // varlist is the list in the symbol table
    debug_msg(1, "Project Parser: %s:%d :%s: :%s: (%s)", parser.file.toLatin1().constData(), parser.line_no,
              var.toLatin1().constData(), op.toLatin1().constData(), vallist.isEmpty() ? "" : vallist.join(" :: ").toLatin1().constData());

    // now do the operation
    if(op == "~=") {
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
                 file.toLatin1().constData());
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
            debug_msg(2, "Looking for mkspec %s in (%s)", Option::mkfile::qmakespec.toLatin1().constData(),
                      mkspec_roots.join("::").toLatin1().constData());
            if(Option::mkfile::qmakespec.isEmpty()) {
                for(QStringList::ConstIterator it = mkspec_roots.begin(); it != mkspec_roots.end(); ++it) {
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
                for(QStringList::ConstIterator it = mkspec_roots.begin(); it != mkspec_roots.end(); ++it) {
                    QString mkspec = (*it) + QDir::separator() + Option::mkfile::qmakespec;
                    if(QFile::exists(mkspec)) {
                        found_mkspec = true;
                        Option::mkfile::qmakespec = mkspec;
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
            while(Option::mkfile::qmakespec.endsWith(QString(QChar(QDir::separator()))))
                Option::mkfile::qmakespec.truncate(Option::mkfile::qmakespec.length()-1);
            QString spec = Option::mkfile::qmakespec + QDir::separator() + "qmake.conf";
            if(!QFile::exists(spec) &&
               QFile::exists(Option::mkfile::qmakespec + QDir::separator() + "tmake.conf"))
                spec = Option::mkfile::qmakespec + QDir::separator() + "tmake.conf";
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
            if(read((*it), vars))
                vars["QMAKE_INTERNAL_INCLUDED_FILES"].append((*it));
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

    // now let the user override the template from an option..
    if(!Option::user_template.isEmpty()) {
        QString s;
        if (!vars["TEMPLATE"].isEmpty())
	    s = vars["TEMPLATE"].first();
        debug_msg(1, "Overriding TEMPLATE (%s) with: %s", s.toLatin1().constData(),
                  Option::user_template.toLatin1().constData());
        vars["TEMPLATE"].clear();
        vars["TEMPLATE"].append(Option::user_template);
    }

    QStringList &templ = vars["TEMPLATE"];
    if(templ.isEmpty())
        templ.append(QString("app"));
    else if(vars["TEMPLATE"].first().endsWith(".t"))
        templ = QStringList(templ.first().left(templ.first().length() - 2));
    if(!Option::user_template_prefix.isEmpty() && !templ.first().startsWith(Option::user_template_prefix))
        templ.first().prepend(Option::user_template_prefix);

    QString test_version = qgetenv("QTESTVERSION");
    if(!test_version.isEmpty()) {
        QString s = vars["TARGET"].first();
        if(s == "qt" || s == "qt-mt" || s == "qte" || s == "qte-mt") {
            QString &ver = vars["VERSION"].first();
//            fprintf(stderr,"Current QT version number: " + ver + "\n");
            if(!ver.isEmpty() && ver != test_version) {
                ver = test_version;
                fprintf(stderr,("Changed QT version number to " + test_version + "!\n").toLatin1());
            }
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
        if(!buffer)
            buffer = (char *)malloc(1024);
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
        chk = chk.right(chk.length() - 1);

    bool test=false;
    int lparen = chk.indexOf('(');
    if(lparen != -1) { // if there is an lparen in the chk, it IS a function
        int rparen = chk.indexOf(')', lparen);
        if(rparen == -1) {
            QByteArray error;
            error.reserve(256);
#if defined(_MSC_VER) && _MSC_VER >= 1400
            sprintf_s(error.data(), 256, "Function (in REQUIRES) missing right paren: %s",
                    chk.toLatin1().constData());
#else
            sprintf(error.data(), "Function (in REQUIRES) missing right paren: %s",
                    chk.toLatin1().constData());
#endif
            qmake_error_msg(error);
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
            if(!feature_roots)
                feature_roots = new QStringList(qmake_feature_paths(prop));
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
        include_roots << Option::output_dir;
        if(Option::output_dir != qmake_getpwd())
            include_roots << qmake_getpwd();
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
    int di = file.lastIndexOf(Option::dir_sep);
    QString oldpwd = qmake_getpwd();
    if(di != -1) {
        if(!qmake_setpwd(file.left(file.lastIndexOf(Option::dir_sep)))) {
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
    if(flags & IncludeFlagNewProject) {
        QMakeProject proj(place);
        if(proj.doProjectInclude("default_pre", IncludeFlagFeature, place) == IncludeNoExist)
            proj.doProjectInclude("default", IncludeFlagFeature, place);
        parsed = proj.read(file, place);
    } else {
        parsed = read(file, place);
    }
    if(parsed)
        place["QMAKE_INTERNAL_INCLUDED_FILES"].append(orig_file);
    else
        warn_msg(WarnParser, "%s:%d: Failure to include file %s.",
                 pi.file.toLatin1().constData(), pi.line_no, orig_file.toLatin1().constData());
    iterator = it;
    function = fu;
    parser = pi;
    scope_blocks = sc;
    qmake_setpwd(oldpwd);
    if(!parsed)
        return IncludeParseFailure;
    return IncludeSuccess;
}

QString
QMakeProject::doProjectExpand(QString func, const QString &params,
                              QMap<QString, QStringList> &place)
{
    return doProjectExpand(func, split_arg_list(params), place);
}

QString
QMakeProject::doProjectExpand(QString func, QStringList args,
                              QMap<QString, QStringList> &place)
{
    func = func.trimmed();
    for(QStringList::Iterator arit = args.begin(); arit != args.end(); ++arit)
        doVariableReplace((*arit), place);

    enum ExpandFunc { E_MEMBER=1, E_FIRST, E_LAST, E_CAT, E_FROMFILE, E_EVAL, E_LIST,
                      E_SPRINTF, E_JOIN, E_SPLIT, E_BASENAME, E_DIRNAME, E_SECTION,
                      E_FIND, E_SYSTEM, E_UNIQUE, E_QUOTE, E_UPPER, E_LOWER, E_FILES,
                      E_PROMPT, E_RE_ESCAPE };
    static QMap<QString, int> *expands = 0;
    if(!expands) {
        expands = new QMap<QString, int>;
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
        expands->insert("upper", E_UPPER);
        expands->insert("lower", E_LOWER);
        expands->insert("re_escape", E_RE_ESCAPE);
        expands->insert("files", E_FILES);
        expands->insert("prompt", E_PROMPT);
    }
    ExpandFunc func_t = (ExpandFunc)expands->value(func.toLower());
    debug_msg(1, "Running project expand: %s(%s) [%d]",
              func.toLatin1().constData(), args.join("::").toLatin1().constData(), func_t);

    QString ret;
    switch(func_t) {
    case E_MEMBER: {
        if(args.count() < 1 || args.count() > 3) {
            fprintf(stderr, "%s:%d: member(var, start, end) requires three arguments.\n",
                    parser.file.toLatin1().constData(), parser.line_no);
        } else {
            bool ok = true;
            const QStringList &var = place[varMap(args.first())];
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
                    for(int i = start; i <= end && (int)var.count() >= i; i++) {
                        if(!ret.isEmpty())
                            ret += Option::field_sep;
                        ret += var[i];
                    }
                } else {
                    for(int i = start; i >= end && (int)var.count() >= i && i >= 0; i--) {
                        if(!ret.isEmpty())
                            ret += Option::field_sep;
                        ret += var[i];
                    }
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
            const QStringList &var = place[varMap(args.first())];
            if(!var.isEmpty()) {
                if(func_t == E_FIRST)
                    ret = var[0];
                else
                    ret = var[var.size()-1];
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
                    ret += stream.readLine().trimmed();
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
            if(doProjectInclude(file, IncludeFlagNewProject, tmp) == IncludeSuccess) {
                if(tmp.contains("QMAKE_INTERNAL_INCLUDED_FILES"))
                    place["QMAKE_INTERNAL_INCLUDED_FILES"] += tmp["QMAKE_INTERNAL_INCLUDED_FILES"];
                ret = tmp[seek_var].join(QString(Option::field_sep));
            }
        }
        break; }
    case E_EVAL: {
        for(QStringList::ConstIterator arg_it = args.begin();
            arg_it != args.end(); ++arg_it) {
            if(!ret.isEmpty())
                ret += Option::field_sep;
            ret += place[(*arg_it)].join(QString(Option::field_sep));
        }
        break; }
    case E_LIST: {
        static int x = 0;
        ret.sprintf(".QMAKE_INTERNAL_TMP_VAR_%d", x++);
        QStringList &lst = (*((QMap<QString, QStringList>*)&place))[ret];
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
            ret = args.at(0);
            QStringList::ConstIterator arg_it = args.begin();
            ++arg_it;
            for(int i = 1; i < args.count(); ++i)
                ret = ret.arg(args.at(i));
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
            const QStringList &var = place[varMap(args.first())];
            if(!var.isEmpty())
                ret = before + var.join(glue) + after;
        }
        break; }
    case E_SPLIT: {
        if(args.count() < 2 || args.count() > 3) {
            fprintf(stderr, "%s:%d split(var, sep, join) requires three arguments\n",
                    parser.file.toLatin1().constData(), parser.line_no);
        } else {
            QString sep = args[1], join = QString(Option::field_sep);
            if(args.count() == 3)
                join = args[2];
            QStringList var = place[varMap(args.first())];
            for(QStringList::ConstIterator vit = var.begin(); vit != var.end(); ++vit) {
                QStringList lst = (*vit).split(sep);
                for(QStringList::ConstIterator spltit = lst.begin(); spltit != lst.end(); ++spltit) {
                    if(!ret.isEmpty())
                        ret += join;
                    ret += (*spltit);
                }
            }
        }
        break; }
    case E_BASENAME:
    case E_DIRNAME:
    case E_SECTION: {
        QString sep, var;
        int beg=0, end=-1;;
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
                sep = Option::dir_sep;
                if(func_t == E_DIRNAME)
                    end = -2;
                else
                    beg = -1;
            }
        }
        if(!var.isNull()) {
            const QStringList &l = place[varMap(var)];
            for(QStringList::ConstIterator it = l.begin(); it != l.end(); ++it) {
                if(!ret.isEmpty())
                    ret += Option::field_sep;
                ret += (*it).section(sep, beg, end);
            }
        }
        break; }
    case E_FIND: {
        if(args.count() != 2) {
            fprintf(stderr, "%s:%d find(var, str) requires two arguments\n",
                    parser.file.toLatin1().constData(), parser.line_no);
        } else {
            QRegExp regx(args[1]);
            const QStringList &var = place[varMap(args.first())];
            for(QStringList::ConstIterator vit = var.begin();
                vit != var.end(); ++vit) {
                if(regx.indexIn(*vit) != -1) {
                    if(!ret.isEmpty())
                        ret += Option::field_sep;
                    ret += (*vit);
                }
            }
        }
        break;  }
    case E_SYSTEM: {
        if(args.count() < 1 || args.count() > 2) {
            fprintf(stderr, "%s:%d system(execut) requires one argument.\n",
                    parser.file.toLatin1().constData(), parser.line_no);
        } else {
#ifdef Q_OS_UNIX
            for(QMap<QString, QStringList>::ConstIterator it = place.begin();
                it != place.end(); ++it) {
                if(!it.key().startsWith("."))
                    putenv(const_cast<char*>(QString(Option::sysenv_mod + it.key() + '=' + it.value().join(" ")).toAscii().constData()));
            }
#endif
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
                ret += buff;
            }
#ifdef Q_OS_UNIX
            for(QMap<QString, QStringList>::ConstIterator it = place.begin();
                it != place.end(); ++it) {
                if(!it.key().startsWith("."))
                    putenv(const_cast<char*>(QString(Option::sysenv_mod
                                                     + it.key()).toAscii().constData()));
            }
#endif
        }
        break; }
    case E_UNIQUE: {
        if(args.count() != 1) {
            fprintf(stderr, "%s:%d unique(var) requires one argument.\n",
                    parser.file.toLatin1().constData(), parser.line_no);
        } else {
            QStringList uniq;
            const QStringList &var = place[varMap(args.first())];
            for(int i = 0; i < var.count(); i++) {
                if(!uniq.contains(var[i]))
                    uniq.append(var[i]);
            }
            ret = uniq.join(" ");
        }
        break; }
    case E_QUOTE: {
        ret = args.join(" ");
        ret = ret.replace("\\n", "\n");
        ret = ret.replace("\\t", "\t");
        ret = ret.replace("\\r", "\r");
        break; }
    case E_RE_ESCAPE: {
        ret = QRegExp::escape(args.join(QString(Option::field_sep)));
        break; }
    case E_UPPER:
    case E_LOWER: {
        ret = args.join(QString(Option::field_sep));
        if(func_t == E_UPPER)
            ret = ret.toUpper();
        else
            ret = ret.toLower();
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
                    if(regex.exactMatch(fname)) {
                        if(!ret.isEmpty())
                            ret += Option::field_sep;
                        ret += fname;
                    }
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
                ret = t.readLine();
            }
        }
        break;
    }
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
        QString rhs(args[1]), lhs(place[args[0]].join(QString(Option::field_sep)));
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
        return place[args[0]].join(QString(Option::field_sep)) == args[1];
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
            f->vars[args[0]] = place[args[0]];
            if(!i && f->calling_place)
                (*f->calling_place)[args[0]] = place[args[0]];
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
            fprintf(stderr, "%s:%d: system(exec) requires one argument.\n", parser.file.toLatin1().constData(),
                    parser.line_no);
            return false;
        }
#ifdef Q_OS_UNIX
        for(QMap<QString, QStringList>::ConstIterator it = place.begin();
            it != place.end(); ++it) {
            if(!it.key().startsWith("."))
                putenv((Option::sysenv_mod + it.key() + '=' + it.value().join(" ")).toAscii().data());
        }
#endif
        bool ret = system(args.first().toLatin1().constData()) == 0;
#ifdef Q_OS_UNIX
        for(QMap<QString, QStringList>::ConstIterator it = place.begin();
            it != place.end(); ++it) {
            if(!it.key().startsWith("."))
                putenv((Option::sysenv_mod + it.key()).toAscii().data());
        }
#endif
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
        if(args.count() != 2) {
            fprintf(stderr, "%s:%d: contains(var, val) requires two arguments.\n", parser.file.toLatin1().constData(),
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
                    parser.file.toLatin1().constData(), parser.line_no);
            return false;
        }

        bool ret = false;
        QMap<QString, QStringList> tmp;
        if(doProjectInclude(Option::fixPathToLocalOS(args[0]), IncludeFlagNewProject, tmp) == IncludeSuccess) {
            if(tmp.contains("QMAKE_INTERNAL_INCLUDED_FILES"))
                place["QMAKE_INTERNAL_INCLUDED_FILES"] += tmp["QMAKE_INTERNAL_INCLUDED_FILES"];
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
                return place[args[0]].count() > args[1].toInt();
            if(comp == ">=")
                return place[args[0]].count() >= args[1].toInt();
            if(comp == "<" || comp == "lessThan")
                return place[args[0]].count() < args[1].toInt();
            if(comp == "<=")
                return place[args[0]].count() <= args[1].toInt();
            if(comp == "equals" || comp == "isEqual" || comp == "=" || comp == "==")
                return place[args[0]].count() == args[1].toInt();
            fprintf(stderr, "%s:%d: unexpected modifier to count(%s)\n", parser.file.toLatin1().constData(),
                    parser.line_no, comp.toLatin1().constData());
            return false;
        }
        return place[args[0]].count() == args[1].toInt();
    } else if(func == "isEmpty") {
        if(args.count() != 1) {
            fprintf(stderr, "%s:%d: isEmpty(var) requires one argument.\n", parser.file.toLatin1().constData(),
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
            fprintf(stderr, "%s:%d: %s requires one argument.\n", parser.file.toLatin1().constData(),
                    parser.line_no, func_desc.toLatin1().constData());
            return false;
        }
        QString file = args.first();
        file = Option::fixPathToLocalOS(file);
        uchar flags = IncludeFlagNone;
        if(!include_statement)
            flags |= IncludeFlagFeature;
        IncludeStatus stat = doProjectInclude(file, flags, place);
        if(stat == IncludeFeatureAlreadyLoaded) {
            warn_msg(WarnParser, "%s:%d: Duplicate of loaded feature %s",
                     parser.file.toLatin1().constData(), parser.line_no, file.toLatin1().constData());
        } else if(stat == IncludeNoExist && include_statement) {
            warn_msg(WarnParser, "%s:%d: Unable to find file for inclusion %s",
                     parser.file.toLatin1().constData(), parser.line_no, file.toLatin1().constData());
        } else if(stat >= IncludeFailure) {
            if(!ignore_error) {
                printf("Project LOAD(): Feature %s cannot be found.\n", file.toLatin1().constData());
                if (!ignore_error)
                    exit(3);
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
            exit(2);
        return true;
    } else if(testFunctions.contains(func)) {
        FunctionBlock *defined = testFunctions[func];
        QString ret;
        function_blocks.push(defined);
        defined->exec(args, this, place, ret);
        Q_ASSERT(function_blocks.pop() == defined);

        if(ret.isEmpty()) {
            return true;
        } else {
            if(ret == "true") {
                return true;
            } else if(ret == "false") {
                return false;
            } else {
                bool ok;
                int val = ret.toInt(&ok);
                if(ok)
                    return val;
                fprintf(stderr, "%s:%d Unexpected return value from test %s [%s].\n", parser.file.toLatin1().constData(),
                        parser.line_no, func.toLatin1().constData(), ret.toLatin1().constData());
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
    if(str.isEmpty())
        return true;

    static bool symbols_init = false;
    enum { LSQUARE, RSQUARE, LCURLY, RCURLY, LPAREN, RPAREN, DOLLAR, SLASH, UNDERSCORE, DOT };
    static ushort symbols[10];
    if(!symbols_init) {
        symbols_init = true;
        symbols[LSQUARE] = QChar('[').unicode();
        symbols[RSQUARE] = QChar(']').unicode();
        symbols[LCURLY] = QChar('{').unicode();
        symbols[RCURLY] = QChar('}').unicode();
        symbols[LPAREN] = QChar('(').unicode();
        symbols[RPAREN] = QChar(')').unicode();
        symbols[DOLLAR] = QChar('$').unicode();
        symbols[SLASH] = QChar('\\').unicode();
        symbols[UNDERSCORE] = QChar('_').unicode();
        symbols[DOT] = QChar('.').unicode();
    }

    ushort unicode;
    const QChar *str_data = str.data();
    const int str_len = str.length();

    ushort term;
    QString replacement;
    QString var, args;

    QString ret;
    int replaced = 0;
    for(int i = 0; i < str_len; ++i) {
        unicode = (str_data+i)->unicode();
        const int start_var = i;
        if(unicode == symbols[SLASH]) {
            if(*(str_data+i+1) == symbols[DOLLAR]) {
                i++;
                if(!(replaced++))
                    ret = str.left(start_var);
                ret.append(str.at(i));
            } else if(replaced) {
                ret.append(QChar(unicode));
            }
            continue;
        }
        if(unicode == symbols[DOLLAR] && str_len > i+2) {
            unicode = (str_data+(++i))->unicode();
            if(unicode == symbols[DOLLAR]) {
                term = 0;
                var.clear();
                args.clear();
                enum { VAR, ENVIRON, FUNCTION, PROPERTY } var_type = VAR;
                unicode = (str_data+(++i))->unicode();
                if(unicode == symbols[LSQUARE]) {
                    unicode = (str_data+(++i))->unicode();
                    term = symbols[RSQUARE];
                    var_type = PROPERTY;
                } else if(unicode == symbols[LCURLY]) {
                    unicode = (str_data+(++i))->unicode();
                    var_type = VAR;
                    term = symbols[RCURLY];
                } else if(unicode == symbols[LPAREN]) {
                    unicode = (str_data+(++i))->unicode();
                    var_type = ENVIRON;
                    term = symbols[RPAREN];
                }
                while(1) {
                    if(!(unicode & (0xFF<<8)) &&
                       unicode != symbols[DOT] && unicode != symbols[UNDERSCORE] &&
                       (unicode < 'a' || unicode > 'z') && (unicode < 'A' || unicode > 'Z') &&
                       (unicode < '0' || unicode > '9'))
                        break;
                    var.append(QChar(unicode));
                    if(++i == str_len)
                        break;
                    unicode = (str_data+i)->unicode();
                }
                if(var_type == VAR && unicode == symbols[LPAREN]) {
                    var_type = FUNCTION;
                    int depth = 0;
                    while(1) {
                        if(++i == str_len)
                            break;
                        unicode = (str_data+i)->unicode();
                        if(unicode == symbols[LPAREN]) {
                            depth++;
                        } else if(unicode == symbols[RPAREN]) {
                            if(!depth)
                                break;
                            --depth;
                        }
                        args.append(QChar(unicode));
                    }
                    if(i < str_len-1)
                        unicode = (str_data+(++i))->unicode();
                    else
                        unicode = 0;
                }
                if(term) {
                    if(unicode != term) {
                        qmake_error_msg("Missing " + QChar(term) + " terminator [found " + QChar(unicode) + "]");
                        return false;
                    }
                    unicode = 0;
                } else if(i > str_len-1) {
                    unicode = 0;
                }

                replacement.clear();
                if(var_type == ENVIRON) {
                    replacement = qgetenv(var.toLatin1().constData());
                } else if(var_type == PROPERTY) {
                    if(prop)
                        replacement = prop->value(var);
                } else if(var_type == FUNCTION) {
                    replacement = doProjectExpand(var, args, place);
                } else if(var_type == VAR) {
                    if(var == QLatin1String("LITERAL_WHITESPACE")) { //a real space in a token)
                        replacement = QLatin1String("\t");
                    } else if(var == QLatin1String("LITERAL_DOLLAR")) { //a real $
                        replacement = "$";
                    } else if(var == QLatin1String("LITERAL_HASH")) { //a real #
                        replacement = "#";
                    } else if(var == QLatin1String("OUT_PWD")) { //the out going dir
                        replacement = Option::output_dir;
                    } else if(var == QLatin1String("PWD") ||  //current working dir (of _FILE_)
                              var == QLatin1String("IN_PWD")) {
                        replacement = qmake_getpwd();
                    } else if(var == QLatin1String("DIR_SEPARATOR")) {
                        replacement = Option::dir_sep;
                    } else if(var == QLatin1String("_LINE_")) { //parser line number
                        replacement = QString::number(parser.line_no);
                    } else if(var == QLatin1String("_FILE_")) { //parser file
                        replacement = parser.file;
                    } else if(var == QLatin1String("_DATE_")) { //current date/time
                        replacement = QDateTime::currentDateTime().toString();
                    } else if(var == QLatin1String("_QMAKE_CACHE_")) {
                        if(Option::mkfile::do_cache)
                            replacement = Option::mkfile::cachefile;
                    } else {
                        replacement = place[varMap(var)].join(QString(Option::field_sep));
                    }
                }
                if(!(replaced++))
                    ret = str.left(start_var);
                ret.append(replacement);
                debug_msg(2, "Project Parser [var replace]: %s -> %s",
                          str.toLatin1().constData(), var.toLatin1().constData(),
                          replacement.toLatin1().constData());
            } else {
                if(replaced)
                    ret.append("$");
            }
        }
        if(replaced && unicode)
            ret.append(QChar(unicode));
    }
    if(replaced)
        str = ret;
    return true;
}


