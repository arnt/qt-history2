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
#include <stdlib.h>
#include <qdir.h>
#include <qglobal.h>
#include <qregexp.h>
#include <stdarg.h>

//convenience
QString Option::ui_ext;
QString Option::h_ext;
QString Option::moc_ext;
QString Option::cpp_ext;
QString Option::obj_ext;
QString Option::dir_sep;
QString Option::moc_mod;
QString Option::yacc_mod;
QString Option::lex_mod;

//mode
Option::QMAKE_MODE Option::qmake_mode = Option::QMAKE_GENERATE_NOTHING;

//all modes
int Option::debug_level = 0;
QFile Option::output;
QString Option::output_dir;
QStringList Option::user_vars;
QString Option::user_template;
#if defined(Q_OS_WIN32)
Option::TARG_MODE Option::target_mode = Option::TARG_WIN_MODE;
#elif defined(Q_OS_MAC9)
Option::TARG_MODE Option::target_mode = Option::TARG_MAC9_MODE;
#elif defined(Q_OS_MACX)
Option::TARG_MODE Option::target_mode = Option::TARG_MACX_MODE;
#else
Option::TARG_MODE Option::target_mode = Option::TARG_UNIX_MODE;
#endif

//QMAKE_GENERATE_PROJECT stuff
bool Option::projfile::do_pwd = TRUE;
QStringList Option::projfile::project_dirs;

//QMAKE_GENERATE_MAKEFILE stuff
QString Option::mkfile::qmakepath;
bool Option::mkfile::do_deps = TRUE;
bool Option::mkfile::do_dep_heuristics = TRUE;
bool Option::mkfile::do_cache = TRUE;
QString Option::mkfile::cachefile;
QStringList Option::mkfile::project_files;


bool usage(const char *a0)
{
    fprintf(stderr, "Usage: %s [mode] [options] [files]\n"
	    "\n"
	    "   QMake has two modes, one mode for generating project files based on\n"
	    "some heuristics, and the other for generating makefiles. Normally you\n"
	    "shouldn't need to specify a mode, as makefile generation is the default\n"
	    "mode for qmake, but you may use this to test qmake on an existing project\n"
	    "\n"
	    "Mode:\n"
	    "\t-project       Put qmake into project file generation mode\n"
	    "\t               In this mode qmake interprets files as files to\n"
	    "\t               be built,\n"
	    "\t               defaults to *.cpp; *.l; *.y; *.ui\n"
	    "\t-makefile      Put qmake into makefile generation mode (assumed)\n"
	    "\t               In this mode qmake interprets files project files to\n"
	    "\t               be built, if skipped qmake will try to find a project\n"
	    "\t               file in your current working directory\n"
	    "\n"
	    "Options:\n"
	    "\t-o file        Write output to file\n"
	    "\t-unix          Run in unix mode\n"
	    "\t-win32         Run in win32 mode\n"
	    "\t-d             Increase debug level\n"
	    "\t-t templ       Overrides TEMPLATE as templ\n"
	    "\t-help          This help\n"
	    "\t-cache file    Use file as cache           [makefile mode only]\n"
	    "\t-path dir      Use dir as QMAKEPATH        [makefile mode only]\n"
	    "\t-nocache       Don't use a cache file      [makefile mode only]\n"
	    "\t-nodepend      Don't generate dependencies [makefile mode only]\n"
	    "\t-nopwd         Don't look for files in pwd [ project mode only]\n"
	    ,a0);
    return FALSE;
}

bool
Option::parseCommandLine(int argc, char **argv)
{
    for(int x = 1; x < argc; x++) {
	if(*argv[x] == '-') { /* options */
	    QString opt = argv[x] + 1;

	    //first param is a mode, or we default
	    if(x == 1) {
		if(opt == "project")
		    Option::qmake_mode = Option::QMAKE_GENERATE_PROJECT;
		else
		    Option::qmake_mode = Option::QMAKE_GENERATE_MAKEFILE;		
		if(opt == "project" || opt == "makefile")
		    continue;
	    }
	    //all modes
	    if(opt == "o" || opt == "output") {
		Option::output.setName(argv[++x]);
	    } else if(opt == "t" || opt == "template") {
		Option::user_template = argv[++x];
	    } else if(opt == "mac9") {
		Option::target_mode = TARG_MAC9_MODE;
	    } else if(opt == "macx") {
		Option::target_mode = TARG_MACX_MODE;
	    } else if(opt == "unix") {
		Option::target_mode = TARG_UNIX_MODE;
	    } else if(opt == "win32") {
		Option::target_mode = TARG_WIN_MODE;
	    } else if(opt == "v" || opt == "d") {
		Option::debug_level++;
	    } else if(opt == "h" || opt == "help") {
		return usage(argv[0]);
	    } else {
		if(Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE) {
		    if(opt == "nodepend") {
			Option::mkfile::do_deps = FALSE;
		    } else if(opt == "nocache") {
			Option::mkfile::do_cache = FALSE;
		    } else if(opt == "nodependheuristics") {
			Option::mkfile::do_dep_heuristics = FALSE;
		    } else if(opt == "mkcache") {
			Option::mkfile::cachefile = argv[++x];
		    } else if(opt == "path") {
			Option::mkfile::qmakepath = argv[++x];
		    } else {
			fprintf(stderr, "***Unknown option -%s\n", opt.latin1());
			return usage(argv[0]);
		    }
		} else if(Option::qmake_mode == Option::QMAKE_GENERATE_PROJECT) {
		    if(opt == "nopwd") {
			Option::projfile::do_pwd = FALSE;
		    } else {
			fprintf(stderr, "***Unknown option -%s\n", opt.latin1());
			return usage(argv[0]);
		    }
		}
	    } 
	}
	else {
	    if(x == 1)
		Option::qmake_mode = Option::QMAKE_GENERATE_MAKEFILE;

	    QString arg = argv[x];
	    if(arg.find('=') != -1) {
		Option::user_vars.append(arg);
	    } else {
		if(Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE)
		    Option::mkfile::project_files.append(arg);
		else
		    Option::projfile::project_dirs.append(arg);
	    }
	}
    }
    
    if(Option::qmake_mode == Option::QMAKE_GENERATE_NOTHING)
	Option::qmake_mode = Option::QMAKE_GENERATE_MAKEFILE; 

    //last chance for defaults
    if(Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE) {
	if(Option::mkfile::cachefile.isNull() || Option::mkfile::cachefile.isEmpty())
	    Option::mkfile::cachefile = ".qmake.cache";

	//try REALLY hard to do it for them, lazy..
	if(!Option::mkfile::project_files.count()) {
	    QString proj = QDir::currentDirPath();
	    proj = proj.right(proj.length() - (proj.findRev(QDir::separator()) + 1)) + ".pro";
	    if(QFile::exists(proj)) {
		Option::qmake_mode = Option::QMAKE_GENERATE_MAKEFILE;		
		Option::mkfile::project_files.append(proj);
	    } else {
		return usage(argv[0]);
	    }
	}
    }

    //defaults for globals
    Option::moc_mod = "moc_";
    Option::lex_mod = "_lex";
    Option::yacc_mod = "_yacc";
    Option::ui_ext = ".ui";
    Option::h_ext = ".h";
    Option::moc_ext = ".moc";
    Option::cpp_ext = ".cpp";
    if(Option::target_mode == Option::TARG_WIN_MODE) {
	Option::dir_sep = "\\";
	Option::obj_ext =  ".obj";
    } else {
	if(Option::target_mode == Option::TARG_MAC9_MODE)
	    Option::dir_sep = ":";
	else
	    Option::dir_sep = "/";
	Option::obj_ext = ".o";
    }
    return TRUE;
}

void fixEnvVariables(QString &x)
{
    int rep, rep_len;
    QRegExp reg_var("\\$\\(.*\\)");
    while((rep = reg_var.match(x, 0, &rep_len)) != -1)
	x.replace(rep, rep_len, QString(getenv(x.mid(rep + 2, rep_len - 3).latin1())));
}

QString
Option::fixPathToTargetOS(QString in, bool fix_env)
{
    if(fix_env)
	fixEnvVariables(in);
    in = QDir::cleanDirPath(in);
    QString rep;
    if(Option::target_mode == TARG_UNIX_MODE || Option::target_mode == TARG_MACX_MODE)
	rep = "[\\\\]";
    else if(Option::target_mode == TARG_MAC9_MODE)
	rep = "[/\\\\]";
    else if(Option::target_mode == TARG_WIN_MODE)
	rep = "[/]";
    return in.replace(QRegExp(rep), Option::dir_sep);
}

QString
Option::fixPathToLocalOS(QString in)
{
    fixEnvVariables(in);
    in = QDir::cleanDirPath(in);
#if defined(Q_OS_WIN32)
    return in.replace(QRegExp("/"), "\\");
#else
    return in.replace(QRegExp("\\"), "/");
#endif
}

void debug_msg(int level, const char *fmt, ...)
{
    if(Option::debug_level < level)
	return;
    fprintf(stdout, "DEBUG %d: ", level);
    {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
    }
    fprintf(stdout, "\n");
}
