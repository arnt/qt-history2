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

#ifndef __OPTION_H__
#define __OPTION_H__

#include "project.h"
#include <qstring.h>
#include <qstringlist.h>
#include <qfile.h>

#define QMAKE_VERSION_MAJOR 2
#define QMAKE_VERSION_MINOR 0
#define QMAKE_VERSION_PATCH 0
const char *qmake_version();

QString qmake_getpwd();
bool qmake_setpwd(const QString &p);

void fixEnvVariables(QString &x);
#define debug_msg if(Option::debug_level) debug_msg_internal
void debug_msg_internal(int level, const char *fmt, ...); //don't call directly, use debug_msg
enum QMakeWarn {
    WarnNone    = 0x00,
    WarnParser  = 0x01,
    WarnLogic   = 0x02,
    WarnAll     = 0xFF
};
void warn_msg(QMakeWarn t, const char *fmt, ...);

struct Option
{
    //simply global convenience
    static QString libtool_ext;
    static QString pkgcfg_ext;
    static QString prf_ext;
    static QString prl_ext;
    static QString ui_ext;
    static QStringList h_ext;
    static QStringList cpp_ext;
    static QString h_moc_ext;
    static QString cpp_moc_ext;
    static QString obj_ext;
    static QString lex_ext;
    static QString yacc_ext;
    static QString h_moc_mod;
    static QString cpp_moc_mod;
    static QString lex_mod;
    static QString yacc_mod;
    static QString dir_sep;
    static QString sysenv_mod;
    static QString pro_ext;
    static QString res_ext;
    static char field_sep;
    //both of these must be called..
    static bool init(int argc=0, char **argv=0); //parse cmdline
    static bool postProcessProject(QMakeProject *);

    //and convenience functions
    static QString fixPathToLocalOS(const QString& in, bool fix_env=true, bool canonical=true);
    static QString fixPathToTargetOS(const QString& in, bool fix_env=true, bool canonical=true);

    //global qmake mode, can only be in one mode per invocation!
    enum QMAKE_MODE { QMAKE_GENERATE_NOTHING, QMAKE_GENERATE_PROJECT, QMAKE_GENERATE_MAKEFILE,
                      QMAKE_GENERATE_PRL, QMAKE_SET_PROPERTY, QMAKE_QUERY_PROPERTY };
    static QMAKE_MODE qmake_mode;

    //all modes
    static QString qmake_abslocation;
    static QFile output;
    static QString output_dir;
    static int debug_level;
    static int warn_level;
    static bool recursive;
    static QStringList before_user_vars, after_user_vars, user_configs;
    enum TARG_MODE { TARG_UNIX_MODE, TARG_WIN_MODE, TARG_MACX_MODE, TARG_MAC9_MODE, TARG_QNX6_MODE };
    static TARG_MODE target_mode;
    static QString user_template, user_template_prefix;
    static QString qtconfig_commandline;

    //QMAKE_*_PROPERTY options
    struct prop {
        static QStringList properties;
    };

    //QMAKE_GENERATE_PROJECT options
    struct projfile {
        static bool do_pwd;
        static QStringList project_dirs;
    };

    //QMAKE_GENERATE_MAKEFILE options
    struct mkfile {
        static QString qmakespec;
        static bool do_cache;
        static bool do_deps;
        static bool do_mocs;
        static bool do_dep_heuristics;
        static bool do_preprocess;
        static QString cachefile;
        static int cachefile_depth;
        static QStringList project_files;
        static QString qmakespec_commandline;
    };

private:
    static int parseCommandLine(int, char **, int=0);
};


#endif /* __OPTION_H__ */
