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

QString Option::ui_ext;
QString Option::h_ext;
QString Option::moc_ext;
QString Option::cpp_ext;
QString Option::obj_ext;
QString Option::dir_sep;
QString Option::moc_mod;
QString Option::yacc_mod;
QString Option::lex_mod;
#if defined(Q_OS_WIN32)
Option::QMODE Option::mode = Option::WIN_MODE;
#else
Option::QMODE Option::mode = Option::UNIX_MODE;
#endif
bool Option::do_deps = TRUE;
bool Option::do_dep_heuristics = TRUE;
bool Option::do_cache = TRUE;
int Option::debug_level = 0;

QString Option::user_template;
QString Option::specfile;
QString Option::cachefile;
QStringList Option::user_vars;
QFile Option::output;
QString Option::output_dir;
QStringList Option::project_files;


bool usage(const char *a0)
{
    fprintf(stderr, "Usage: %s [options] project-files\n"
	   "Options:\n"
	   "\t-nocache      Don't use a cache file\n"
	   "\t-nodepend     Don't generate dependency information\n"
	   "\t-o file       Write output to file\n"
	   "\t-unix         Run in unix mode\n"
	   "\t-win32        Run in win32 mode\n"
	   "\t-mkcache file Use file as cache\n"
	   "\t-mkspec file  Use file as spec\n"
	   "\t-d            Increase debug level\n", a0);
    return FALSE;
}

bool
Option::parseCommandLine(int argc, char **argv)
{
    if(argc == 1)
	return usage(argv[0]);

    for(int x = 1; x < argc; x++) {
	if(*argv[x] == '-') { /* options */
	    QString opt = argv[x] + 1;
	    if(opt == "nodepend") {
		Option::do_deps = FALSE;
	    } else if(opt == "nocache") {
		Option::do_cache = FALSE;
	    } else if(opt == "nodependheuristics") {
		Option::do_dep_heuristics = FALSE;
	    } else if(opt == "mkcache") {
		Option::cachefile = argv[++x];
	    } else if(opt == "t" || opt == "template") {
		Option::user_template = argv[++x];
	    } else if(opt == "o" || opt == "output") {
		Option::output.setName(argv[++x]);
	    } else if(opt == "unix") {
		Option::mode = UNIX_MODE;
	    } else if(opt == "win32") {
		Option::mode = WIN_MODE;
	    } else if(opt == "mkspec") {
		Option::specfile = argv[++x];
	    }  else if(opt == "v" || opt == "d") {
		Option::debug_level++;
	    } else {
		return usage(argv[0]);
	    }
	}
	else {
	    QString arg = argv[x];
	    if(arg.find('=') != -1) {
		Option::user_vars.append(arg);
	    } else {
		Option::project_files.append(arg);
	    }
	}
    }
    if(Option::cachefile.isNull() || Option::cachefile.isEmpty())
	Option::cachefile = ".qmake.cache";

    Option::moc_mod = "moc_";
    Option::lex_mod = "_lex";
    Option::yacc_mod = "_yacc";
    Option::ui_ext = ".ui";
    Option::h_ext = ".h";
    Option::moc_ext = ".moc";
    Option::cpp_ext = ".cpp";
    if(Option::mode == Option::WIN_MODE) {
	Option::dir_sep = "\\";
	Option::obj_ext =  ".obj";
    } else {
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
    return in.replace(QRegExp(Option::mode == UNIX_MODE ? "\\" : "/"), Option::dir_sep);
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
