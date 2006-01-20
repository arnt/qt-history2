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

#include "option.h"
#include <qdir.h>
#include <qregexp.h>
#include <qhash.h>
#include <qsettings.h>
#include <stdlib.h>
#include <stdarg.h>

//convenience
QString Option::prf_ext;
QString Option::prl_ext;
QString Option::libtool_ext;
QString Option::pkgcfg_ext;
QString Option::ui_ext;
QStringList Option::h_ext;
QString Option::cpp_moc_ext;
QString Option::h_moc_ext;
QStringList Option::cpp_ext;
QString Option::obj_ext;
QString Option::lex_ext;
QString Option::yacc_ext;
QString Option::pro_ext;
QString Option::dir_sep;
QString Option::dirlist_sep;
QString Option::h_moc_mod;
QString Option::cpp_moc_mod;
QString Option::yacc_mod;
QString Option::lex_mod;
QString Option::sysenv_mod;
QString Option::res_ext;
char Option::field_sep;

//mode
Option::QMAKE_MODE Option::qmake_mode = Option::QMAKE_GENERATE_NOTHING;

//all modes
QString Option::qmake_abslocation;
int Option::warn_level = WarnLogic;
int Option::debug_level = 0;
QFile Option::output;
QString Option::output_dir;
bool Option::recursive = false;
QStringList Option::before_user_vars;
QStringList Option::after_user_vars;
QStringList Option::user_configs;
QStringList Option::after_user_configs;
QString Option::user_template;
QString Option::user_template_prefix;
#if defined(Q_OS_WIN32)
Option::TARG_MODE Option::target_mode = Option::TARG_WIN_MODE;
#elif defined(Q_OS_MAC)
Option::TARG_MODE Option::target_mode = Option::TARG_MACX_MODE;
#elif defined(Q_OS_QNX6)
Option::TARG_MODE Option::target_mode = Option::TARG_QNX6_MODE;
#else
Option::TARG_MODE Option::target_mode = Option::TARG_UNIX_MODE;
#endif

//QMAKE_*_PROPERTY stuff
QStringList Option::prop::properties;

//QMAKE_GENERATE_PROJECT stuff
bool Option::projfile::do_pwd = true;
QStringList Option::projfile::project_dirs;

//QMAKE_GENERATE_MAKEFILE stuff
QString Option::mkfile::qmakespec;
int Option::mkfile::cachefile_depth = -1;
bool Option::mkfile::do_deps = true;
bool Option::mkfile::do_mocs = true;
bool Option::mkfile::do_dep_heuristics = true;
bool Option::mkfile::do_preprocess = false;
bool Option::mkfile::do_cache = true;
QString Option::mkfile::cachefile;
QStringList Option::mkfile::project_files;
QString Option::mkfile::qmakespec_commandline;

static Option::QMAKE_MODE default_mode(QString progname)
{
    int s = progname.lastIndexOf(Option::dir_sep);
    if(s != -1)
        progname = progname.right(progname.length() - (s + 1));
    if(progname == "qmakegen")
        return Option::QMAKE_GENERATE_PROJECT;
    else if(progname == "qt-config")
        return Option::QMAKE_QUERY_PROPERTY;
    return Option::QMAKE_GENERATE_MAKEFILE;
}

QString project_builtin_regx();
bool usage(const char *a0)
{
    fprintf(stdout, "Usage: %s [mode] [options] [files]\n"
            "\n"
            "QMake has two modes, one mode for generating project files based on\n"
            "some heuristics, and the other for generating makefiles. Normally you\n"
            "shouldn't need to specify a mode, as makefile generation is the default\n"
            "mode for qmake, but you may use this to test qmake on an existing project\n"
            "\n"
            "Mode:\n"
            "\t-project       Put qmake into project file generation mode%s\n"
            "\t               In this mode qmake interprets files as files to\n"
            "\t               be built,\n"
            "\t               defaults to %s\n"
            "\t-makefile      Put qmake into makefile generation mode%s\n"
            "\t               In this mode qmake interprets files as project files to\n"
            "\t               be processed, if skipped qmake will try to find a project\n"
            "\t               file in your current working directory\n"
            "\n"
            "Warnings Options:\n"
            "\t-Wnone         Turn off all warnings\n"
            "\t-Wall          Turn on all warnings\n"
            "\t-Wparser       Turn on parser warnings\n"
            "\t-Wlogic        Turn on logic warnings\n"
            "\n"
            "Options:\n"
            "\t * You can place any variable assignment in options and it will be     *\n"
            "\t * processed as if it was in [files]. These assignments will be parsed *\n"
            "\t * before [files].                                                     *\n"
            "\t-o file        Write output to file\n"
            "\t-unix          Run in unix mode\n"
            "\t-win32         Run in win32 mode\n"
            "\t-macx          Run in Mac OS X mode\n"
            "\t-d             Increase debug level\n"
            "\t-t templ       Overrides TEMPLATE as templ\n"
            "\t-tp prefix     Overrides TEMPLATE so that prefix is prefixed into the value\n"
            "\t-help          This help\n"
            "\t-v             Version information\n"
            "\t-after         All variable assignments after this will be\n"
            "\t               parsed after [files]\n"
            "\t-norecursive   Don't do a recursive search\n"
            "\t-recursive     Do a recursive search\n"
            "\t-cache file    Use file as cache           [makefile mode only]\n"
            "\t-spec spec     Use spec as QMAKESPEC       [makefile mode only]\n"
            "\t-nocache       Don't use a cache file      [makefile mode only]\n"
            "\t-nodepend      Don't generate dependencies [makefile mode only]\n"
            "\t-nomoc         Don't generate moc targets  [makefile mode only]\n"
            "\t-nopwd         Don't look for files in pwd [project mode only]\n"
            ,a0,
            default_mode(a0) == Option::QMAKE_GENERATE_PROJECT  ? " (default)" : "", project_builtin_regx().toLatin1().constData(),
            default_mode(a0) == Option::QMAKE_GENERATE_MAKEFILE ? " (default)" : ""
        );
    return false;
}

int
Option::parseCommandLine(int argc, char **argv, int skip)
{
    bool before = true;
    for(int x = skip; x < argc; x++) {
        if(*argv[x] == '-' && strlen(argv[x]) > 1) { /* options */
            QString opt = argv[x] + 1;

            //first param is a mode, or we default
            if(x == 1) {
                bool specified = true;
                if(opt == "project") {
                    Option::recursive = true;
                    Option::qmake_mode = Option::QMAKE_GENERATE_PROJECT;
                } else if(opt == "prl") {
                    Option::mkfile::do_deps = false;
                    Option::mkfile::do_mocs = false;
                    Option::qmake_mode = Option::QMAKE_GENERATE_PRL;
                } else if(opt == "set") {
                    Option::qmake_mode = Option::QMAKE_SET_PROPERTY;
                } else if(opt == "query") {
                    Option::qmake_mode = Option::QMAKE_QUERY_PROPERTY;
                } else if(opt == "makefile") {
                    Option::qmake_mode = Option::QMAKE_GENERATE_MAKEFILE;
                } else {
                    specified = false;
                }
                if(specified)
                    continue;
            }
            //all modes
            if(opt == "o" || opt == "output") {
                Option::output.setFileName(argv[++x]);
            } else if(opt == "after") {
                before = false;
            } else if(opt == "t" || opt == "template") {
                Option::user_template = argv[++x];
            } else if(opt == "tp" || opt == "template_prefix") {
                Option::user_template_prefix = argv[++x];
            } else if(opt == "mac9") {
                Option::target_mode = TARG_MAC9_MODE;
            } else if(opt == "macx") {
                Option::target_mode = TARG_MACX_MODE;
            } else if(opt == "unix") {
                Option::target_mode = TARG_UNIX_MODE;
            } else if(opt == "win32") {
                Option::target_mode = TARG_WIN_MODE;
            } else if(opt == "d") {
                Option::debug_level++;
            } else if(opt == "version" || opt == "v" || opt == "-version") {
                fprintf(stdout,
                        "QMake version: %s\n"
                        "Using Qt version %s in %s\n",
                        qmake_version(), QT_VERSION_STR,
                        QLibraryInfo::location(QLibraryInfo::LibrariesPath).toLatin1().constData());
#ifdef QMAKE_OPENSOURCE_VERSION
                fprintf(stdout, "QMake is Open Source software from Trolltech AS.\n");
#endif
                return Option::QMAKE_CMDLINE_BAIL;
            } else if(opt == "h" || opt == "help") {
                return Option::QMAKE_CMDLINE_SHOW_USAGE;
            } else if(opt == "Wall") {
                Option::warn_level |= WarnAll;
            } else if(opt == "Wparser") {
                Option::warn_level |= WarnParser;
            } else if(opt == "Wlogic") {
                Option::warn_level |= WarnLogic;
            } else if(opt == "Wnone") {
                Option::warn_level = WarnNone;
            } else if(opt == "r" || opt == "recursive") {
                Option::recursive = true;
            } else if(opt == "norecursive") {
                Option::recursive = false;
            } else if(opt == "config") {
                Option::user_configs += argv[++x];
            } else {
                if(Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE ||
                   Option::qmake_mode == Option::QMAKE_GENERATE_PRL) {
                    if(opt == "nodepend" || opt == "nodepends") {
                        Option::mkfile::do_deps = false;
                    } else if(opt == "nomoc") {
                        Option::mkfile::do_mocs = false;
                    } else if(opt == "nocache") {
                        Option::mkfile::do_cache = false;
                    } else if(opt == "nodependheuristics") {
                        Option::mkfile::do_dep_heuristics = false;
                    } else if(opt == "E") {
                        Option::mkfile::do_preprocess = true;
                    } else if(opt == "cache") {
                        Option::mkfile::cachefile = argv[++x];
                    } else if(opt == "platform" || opt == "spec") {
                        Option::mkfile::qmakespec = argv[++x];
                        Option::mkfile::qmakespec_commandline = argv[x];
                    } else {
                        fprintf(stderr, "***Unknown option -%s\n", opt.toLatin1().constData());
                        return Option::QMAKE_CMDLINE_SHOW_USAGE | Option::QMAKE_CMDLINE_ERROR;
                    }
                } else if(Option::qmake_mode == Option::QMAKE_GENERATE_PROJECT) {
                    if(opt == "nopwd") {
                        Option::projfile::do_pwd = false;
                    } else {
                        fprintf(stderr, "***Unknown option -%s\n", opt.toLatin1().constData());
                        return Option::QMAKE_CMDLINE_SHOW_USAGE | Option::QMAKE_CMDLINE_ERROR;
                    }
                }
            }
        } else {
            QString arg = argv[x];
            if(arg.indexOf('=') != -1) {
                if(before)
                    Option::before_user_vars.append(arg);
                else
                    Option::after_user_vars.append(arg);
            } else {
                bool handled = true;
                if(Option::qmake_mode == Option::QMAKE_QUERY_PROPERTY ||
                    Option::qmake_mode == Option::QMAKE_SET_PROPERTY) {
                    Option::prop::properties.append(arg);
                } else {
                    QFileInfo fi(arg);
                    if(!fi.makeAbsolute()) //strange
                        arg = fi.filePath();
                    if(Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE ||
                       Option::qmake_mode == Option::QMAKE_GENERATE_PRL)
                        Option::mkfile::project_files.append(arg);
                    else if(Option::qmake_mode == Option::QMAKE_GENERATE_PROJECT)
                        Option::projfile::project_dirs.append(arg);
                    else
                        handled = false;
                }
                if(!handled) {
                    return Option::QMAKE_CMDLINE_SHOW_USAGE | Option::QMAKE_CMDLINE_ERROR;
                }
            }
        }
    }

    return Option::QMAKE_CMDLINE_SUCCESS;
}

int
Option::init(int argc, char **argv)
{
    Option::cpp_moc_mod = "";
    Option::h_moc_mod = "moc_";
    Option::lex_mod = "_lex";
    Option::yacc_mod = "_yacc";
    Option::prl_ext = ".prl";
    Option::libtool_ext = ".la";
    Option::pkgcfg_ext = ".pc";
    Option::prf_ext = ".prf";
    Option::ui_ext = ".ui";
    Option::h_ext << ".h" << ".hpp" << ".hh" << ".hxx";
#ifndef Q_OS_WIN
    Option::h_ext << ".H";
#endif
    Option::cpp_moc_ext = ".moc";
    Option::h_moc_ext = ".cpp";
    Option::cpp_ext << ".cpp" << ".cc" << ".cxx";
#ifndef Q_OS_WIN
    Option::cpp_ext << ".C";
#endif
    Option::lex_ext = ".l";
    Option::yacc_ext = ".y";
    Option::pro_ext = ".pro";
#ifdef Q_OS_WIN
    Option::dirlist_sep = ";";
#else
    Option::dirlist_sep = ":";
#endif
    Option::sysenv_mod = "QMAKE_ENV_";
    Option::field_sep = ' ';

    if(argc && argv) {
        QString argv0 = argv[0];
        if(Option::qmake_mode == Option::QMAKE_GENERATE_NOTHING)
            Option::qmake_mode = default_mode(argv0);
        if(!argv0.isEmpty() && argv0.at(0) == QLatin1Char('/')) {
            Option::qmake_abslocation = argv0;
        } else if (argv0.contains(QLatin1Char('/'))) { //relative PWD
            Option::qmake_abslocation = QDir::current().absoluteFilePath(argv0);
        } else { //in the PATH
            QByteArray pEnv = qgetenv("PATH");
            QDir currentDir = QDir::current();
#ifdef Q_OS_WIN
            QStringList paths = QString::fromLocal8Bit(pEnv).split(QLatin1String(";"));
#else
            QStringList paths = QString::fromLocal8Bit(pEnv).split(QLatin1String(":"));
#endif
            for (QStringList::const_iterator p = paths.constBegin(); p != paths.constEnd(); ++p) {
                if ((*p).isEmpty())
                    continue;
                QString candidate = currentDir.absoluteFilePath(*p + QLatin1Char('/') + argv0);
                if (QFile::exists(candidate)) {
                    Option::qmake_abslocation = candidate;
                    break;
                }
            }
        }
        if(!Option::qmake_abslocation.isNull())
            Option::qmake_abslocation = QDir::cleanPath(Option::qmake_abslocation);
    } else {
        Option::qmake_mode = Option::QMAKE_GENERATE_MAKEFILE;
    }

    const QByteArray envflags = qgetenv("QMAKEFLAGS");
    if (!envflags.isNull()) {
        int env_argc = 0, env_size = 0, currlen=0;
        char quote = 0, **env_argv = NULL;
        for (int i = 0; i < envflags.size(); ++i) {
            if (!quote && (envflags.at(i) == '\'' || envflags.at(i) == '"')) {
                quote = envflags.at(i);
            } else if (envflags.at(i) == quote) {
                quote = 0;
            } else if (!quote && envflags.at(i) == ' ') {
                if (currlen && env_argv && env_argv[env_argc]) {
                    env_argv[env_argc][currlen] = '\0';
                    currlen = 0;
                    env_argc++;
                }
            } else {
                if(!env_argv || env_argc > env_size) {
                    env_argv = (char **)realloc(env_argv, sizeof(char *)*(env_size+=10));
                    for(int i2 = env_argc; i2 < env_size; i2++)
                        env_argv[i2] = NULL;
                }
                if(!env_argv[env_argc]) {
                    currlen = 0;
                    env_argv[env_argc] = (char*)malloc(255);
                }
                if(currlen < 255)
                    env_argv[env_argc][currlen++] = envflags.at(i);
            }
        }
        if(env_argv) {
            if(env_argv[env_argc]) {
                env_argv[env_argc][currlen] = '\0';
                currlen = 0;
                env_argc++;
            }
            parseCommandLine(env_argc, env_argv);
            for(int i2 = 0; i2 < env_size; i2++) {
                if(env_argv[i2])
                    free(env_argv[i2]);
            }
            free(env_argv);
        }
    }
    if(argc && argv) {
        int ret = parseCommandLine(argc, argv, 1);
        if(ret != Option::QMAKE_CMDLINE_SUCCESS) {
            if ((ret & Option::QMAKE_CMDLINE_SHOW_USAGE) != 0)
                usage(argv[0]);
            return ret;
            //return ret == QMAKE_CMDLINE_SHOW_USAGE ? usage(argv[0]) : false;
        }
    }

    //last chance for defaults
    if(Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE ||
        Option::qmake_mode == Option::QMAKE_GENERATE_PRL) {
        if(Option::mkfile::qmakespec.isNull() || Option::mkfile::qmakespec.isEmpty())
            Option::mkfile::qmakespec = QString::fromLocal8Bit(qgetenv("QMAKESPEC").constData());

        //try REALLY hard to do it for them, lazy..
        if(Option::mkfile::project_files.isEmpty()) {
            QString pwd = qmake_getpwd(),
                   proj = pwd + "/" + pwd.right(pwd.length() - (pwd.lastIndexOf('/') + 1)) + Option::pro_ext;
            if(QFile::exists(proj)) {
                Option::mkfile::project_files.append(proj);
            } else { //last try..
                QStringList profiles = QDir(pwd).entryList(QStringList("*" + Option::pro_ext));
                if(profiles.count() == 1)
                    Option::mkfile::project_files.append(pwd + "/" + profiles[0]);
            }
#ifndef QT_BUILD_QMAKE_LIBRARY
            if(Option::mkfile::project_files.isEmpty()) {
                usage(argv[0]);
                return Option::QMAKE_CMDLINE_ERROR;
            }
#endif
        }
    }

    //defaults for globals
    if(Option::target_mode == Option::TARG_WIN_MODE) {
        Option::dir_sep = "\\";
        Option::obj_ext = ".obj";
        Option::res_ext = ".res";
    } else {
        if(Option::target_mode == Option::TARG_MAC9_MODE)
            Option::dir_sep = ":";
        else
            Option::dir_sep = "/";
        Option::obj_ext = ".o";
    }
    return QMAKE_CMDLINE_SUCCESS;
}

bool Option::postProcessProject(QMakeProject *project)
{
    Option::cpp_ext = project->variables()["QMAKE_EXT_CPP"];
    if(cpp_ext.isEmpty())
        cpp_ext << ".cpp"; //something must be there
    Option::h_ext = project->variables()["QMAKE_EXT_H"];
    if(h_ext.isEmpty())
        h_ext << ".h";

    if(!project->isEmpty("QMAKE_EXT_RES"))
        Option::res_ext = project->first("QMAKE_EXT_RES");
    if(!project->isEmpty("QMAKE_EXT_PKGCONFIG"))
        Option::pkgcfg_ext = project->first("QMAKE_EXT_PKGCONFIG");
    if(!project->isEmpty("QMAKE_EXT_LIBTOOL"))
        Option::libtool_ext = project->first("QMAKE_EXT_LIBTOOL");
    if(!project->isEmpty("QMAKE_EXT_PRL"))
        Option::prl_ext = project->first("QMAKE_EXT_PRL");
    if(!project->isEmpty("QMAKE_EXT_PRF"))
        Option::prf_ext = project->first("QMAKE_EXT_PRF");
    if(!project->isEmpty("QMAKE_EXT_UI"))
        Option::ui_ext = project->first("QMAKE_EXT_UI");
    if(!project->isEmpty("QMAKE_EXT_CPP_MOC"))
        Option::cpp_moc_ext = project->first("QMAKE_EXT_CPP_MOC");
    if(!project->isEmpty("QMAKE_EXT_H_MOC"))
        Option::h_moc_ext = project->first("QMAKE_EXT_H_MOC");
    if(!project->isEmpty("QMAKE_EXT_LEX"))
        Option::lex_ext = project->first("QMAKE_EXT_LEX");
    if(!project->isEmpty("QMAKE_EXT_YACC"))
        Option::yacc_ext = project->first("QMAKE_EXT_YACC");
    if(!project->isEmpty("QMAKE_EXT_OBJ"))
        Option::obj_ext = project->first("QMAKE_EXT_OBJ");
    if(!project->isEmpty("QMAKE_H_MOD_MOC"))
        Option::h_moc_mod = project->first("QMAKE_H_MOD_MOC");
    if(!project->isEmpty("QMAKE_CPP_MOD_MOC"))
        Option::cpp_moc_mod = project->first("QMAKE_CPP_MOD_MOC");
    if(!project->isEmpty("QMAKE_MOD_LEX"))
        Option::lex_mod = project->first("QMAKE_MOD_LEX");
    if(!project->isEmpty("QMAKE_MOD_YACC"))
        Option::yacc_mod = project->first("QMAKE_MOD_YACC");
    if(!project->isEmpty("QMAKE_DIR_SEP"))
        Option::dir_sep = project->first("QMAKE_DIR_SEP");
    if(!project->isEmpty("QMAKE_DIRLIST_SEP"))
        Option::dirlist_sep = project->first("QMAKE_DIRLIST_SEP");
    if(!project->isEmpty("QMAKE_MOD_SYSTEM_ENV"))
        Option::sysenv_mod = project->first("QMAKE_MOD_SYSTEM_ENV");
    return true;
}

struct FixStringCacheKey
{
    mutable uint hash;
    QString string, pwd;
    uchar flags;
    FixStringCacheKey(const QString &s, uchar f)
    {
        hash = 0;
        pwd = qmake_getpwd();
        string = s;
        flags = f;
    }
    bool operator==(const FixStringCacheKey &f) const
    {
        return (hashCode() == f.hashCode() &&
                f.flags == flags &&
                f.string == string &&
                f.pwd == pwd);
    }
    inline uint hashCode() const {
        if(!hash)
            hash = qHash(string) | qHash(flags) /*| qHash(pwd)*/;
        return hash;
    }
};
uint qHash(const FixStringCacheKey &f) { return f.hashCode(); }

QString
Option::fixString(QString string, uchar flags)
{
    static QHash<FixStringCacheKey, QString> *cache = 0;
    if(!cache) {
        cache = new QHash<FixStringCacheKey, QString>;
        qmakeAddCacheClear(qmakeDeleteCacheClear<QHash<FixStringCacheKey, QString> >, (void**)&cache);
    }
    FixStringCacheKey cacheKey(string, flags);
    if(cache->contains(cacheKey))
        return cache->value(cacheKey);

    //fix the environment variables
    if(flags & Option::FixEnvVars) {
        int rep;
        QRegExp reg_var("\\$\\(.*\\)");
        reg_var.setMinimal(true);
        while((rep = reg_var.indexIn(string)) != -1)
            string.replace(rep, reg_var.matchedLength(),
                           QString::fromLocal8Bit(qgetenv(string.mid(rep + 2, reg_var.matchedLength() - 3).toLatin1().constData()).constData()));
    }

    //canonicalize it (and treat as a path)
    if(flags & Option::FixPathCanonicalize) {
#if 0
        string = QFileInfo(string).canonicalFilePath();
#endif
        string = QDir::cleanPath(string);
    }

    //fix separators
    Q_ASSERT(!((flags & Option::FixPathToLocalSeparators) && (flags & Option::FixPathToTargetSeparators)));
    if(flags & Option::FixPathToLocalSeparators) {
#if defined(Q_OS_WIN32)
        string = string.replace('/', '\\');
#else
        string = string.replace('\\', '/');
#endif
    } else if(flags & Option::FixPathToTargetSeparators) {
        string = string.replace('/', Option::dir_sep).replace('\\', Option::dir_sep);
    }

    if (string.startsWith("\"") && string.endsWith("\"") ||
        string.startsWith("\'") && string.endsWith("\'"))
        string = string.mid(1, string.length()-2);

    //cache
    cache->insert(cacheKey, string);
    return string;
}

const char *qmake_version()
{
    static char *ret = NULL;
    if(ret)
        return ret;
    ret = (char *)malloc(15);
    qmakeAddCacheClear(qmakeFreeCacheClear, (void**)&ret);
#if defined(_MSC_VER) && _MSC_VER >= 1400
    sprintf_s(ret, 15, "%d.%02d%c", QMAKE_VERSION_MAJOR, QMAKE_VERSION_MINOR, 'a' + QMAKE_VERSION_PATCH);
#else
    sprintf(ret, "%d.%02d%c", QMAKE_VERSION_MAJOR, QMAKE_VERSION_MINOR, 'a' + QMAKE_VERSION_PATCH);
#endif
    return ret;
}

void debug_msg_internal(int level, const char *fmt, ...)
{
    if(Option::debug_level < level)
        return;
    fprintf(stderr, "DEBUG %d: ", level);
    {
        va_list ap;
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
    }
    fprintf(stderr, "\n");
}

void warn_msg(QMakeWarn type, const char *fmt, ...)
{
    if(!(Option::warn_level & type))
        return;
    fprintf(stderr, "WARNING: ");
    {
        va_list ap;
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
    }
    fprintf(stderr, "\n");
}

#include "../src/corelib/global/qconfig.cpp"
QString QLibraryInfo::location(QLibraryInfo::LibraryLocation loc)
{
    QString ret;
    const char *path = 0;
    switch (loc) {
#ifdef QT_CONFIGURE_PREFIX_PATH
    case PrefixPath:
        path = QT_CONFIGURE_PREFIX_PATH;
        break;
#endif
#ifdef QT_CONFIGURE_DOCUMENTATION_PATH
    case DocumentationPath:
        path = QT_CONFIGURE_DOCUMENTATION_PATH;
        break;
#endif
#ifdef QT_CONFIGURE_HEADERS_PATH
    case HeadersPath:
        path = QT_CONFIGURE_HEADERS_PATH;
        break;
#endif
#ifdef QT_CONFIGURE_LIBRARIES_PATH
    case LibrariesPath:
        path = QT_CONFIGURE_LIBRARIES_PATH;
        break;
#endif
#ifdef QT_CONFIGURE_BINARIES_PATH
    case BinariesPath:
        path = QT_CONFIGURE_BINARIES_PATH;
        break;
#endif
#ifdef QT_CONFIGURE_PLUGINS_PATH
    case PluginsPath:
        path = QT_CONFIGURE_PLUGINS_PATH;
        break;
#endif
#ifdef QT_CONFIGURE_DATA_PATH
    case DataPath:
        path = QT_CONFIGURE_DATA_PATH;
        break;
#endif
#ifdef QT_CONFIGURE_TRANSLATIONS_PATH
    case TranslationsPath:
        path = QT_CONFIGURE_TRANSLATIONS_PATH;
        break;
#endif
#ifdef QT_CONFIGURE_SETTINGS_PATH
    case SettingsPath:
        path = QT_CONFIGURE_SETTINGS_PATH;
        break;
#endif
#ifdef QT_CONFIGURE_EXAMPLES_PATH
        case ExamplesPath:
            path = QT_CONFIGURE_EXAMPLES_PATH;
            break;
#endif
#ifdef QT_CONFIGURE_DEMOS_PATH
        case DemosPath:
            path = QT_CONFIGURE_DEMOS_PATH;
            break;
#endif
    default:
        break;
    }

    if (path)
        ret = QString::fromLocal8Bit(path);

    if (QDir::isRelativePath(ret)) {
        if (loc == PrefixPath) {
            // we make the prefix path absolute to the current directory
            return QDir::current().absoluteFilePath(ret);
        } else {
            // we make any other path absolute to the prefix directory
            return QDir(location(PrefixPath)).absoluteFilePath(ret);
        }
    }
    return ret;
}

class QMakeCacheClearItem {
private:
    qmakeCacheClearFunc func;
    void **data;
public:
    QMakeCacheClearItem(qmakeCacheClearFunc f, void **d) : func(f), data(d) { }
    ~QMakeCacheClearItem() {
        (*func)(*data);
        *data = 0;
    }
};
static QList<QMakeCacheClearItem*> cache_items;

void
qmakeClearCaches()
{
    qDeleteAll(cache_items);
    cache_items.clear();
}

void
qmakeAddCacheClear(qmakeCacheClearFunc func, void **data)
{
    cache_items.append(new QMakeCacheClearItem(func, data));
}
