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

#include "makefile.h"
#include <stdio.h>
#include <qdir.h>
#include <qfile.h>
#include <qtextstream.h>
#include <time.h>
#include <qregexp.h>
#include "option.h"

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


MakefileGenerator::MakefileGenerator(QMakeProject *p) : project(p), init_already(FALSE), moc_aware(FALSE)
{
    QString dirs[] = { QString("OBJECTS_DIR"), QString("MOC_DIR"), QString("DESTDIR"), QString::null };
    for(int x = 0; dirs[x] != QString::null; x++) {
	QString &path = project->variables()[dirs[x]].first();
	path = Option::fixPathToTargetOS(path);
	if (!path.isEmpty()) {
	    if(path.right(Option::dir_sep.length()) != Option::dir_sep)
		path += Option::dir_sep;

	    QString build;
	    if(path.left(1) == Option::dir_sep)
		build = Option::dir_sep;
	    QStringList subs = QStringList::split(Option::dir_sep, path);
	    for(QStringList::Iterator subit = subs.begin(); subit != subs.end(); ++subit) {
		build += (*subit) + QDir::separator();
		QDir::current().mkdir(build);
	    }
	}
    }
}

char *gimme_buffer(off_t s)
{
    static char *big_buffer = NULL;
    static int big_buffer_size = 0;
    if(!big_buffer || big_buffer_size < s)
	big_buffer = (char *)realloc(big_buffer, s);
    return big_buffer;
}

bool
MakefileGenerator::generateMocList(QString fn_target)
{
    if(!mocablesToMOC[fn_target].isEmpty())
	return TRUE;
    QString fn_local = Option::fixPathToLocalOS(fn_target);

    int file = open(fn_local.latin1(), O_RDONLY);
    if(file == -1)
	return FALSE;

    struct stat fst;
    if(fstat(file, &fst))
	return FALSE; //shouldn't happen
    char *big_buffer = gimme_buffer(fst.st_size);

    int total_size_read;
    for(int have_read = total_size_read = 0; 
	have_read = read(file, big_buffer + total_size_read, fst.st_size - total_size_read);
	total_size_read += have_read);
    close(file);

#define COMP_LEN 8 //strlen("Q_OBJECT")
    for(int x = 0; x < (total_size_read-COMP_LEN); x++) {
	if(*(big_buffer+x) == 'Q' && !strncmp(big_buffer+x, "Q_OBJECT", COMP_LEN)) {

	    int ext_pos = fn_target.findRev('.');
	    int ext_len = fn_target.length() - ext_pos;
	    int dir_pos =  fn_target.findRev('/', ext_pos);
	    QString mocFile;
	    if(!project->variables()["MOC_DIR"].isEmpty())
		mocFile = project->variables()["MOC_DIR"].first();
	    else
		mocFile = fn_target.left(dir_pos+1);

	    if(fn_target.right(ext_len) == Option::cpp_ext) {
		mocFile += fn_target.mid(dir_pos+1, ext_pos - dir_pos-1) + Option::moc_ext;
		depends[fn_target].append(mocFile);
		project->variables()["_SRCMOC"].append(mocFile);
	    }
	    else if(fn_target.right(ext_len) == Option::h_ext &&
		    project->variables()["HEADERS"].findIndex(fn_target) != -1) {
		mocFile += Option::moc_mod + fn_target.mid(dir_pos+1, ext_pos - dir_pos-1) + Option::cpp_ext;
		project->variables()["_HDRMOC"].append(mocFile);
	    }

	    if(!mocFile.isEmpty()) {
		mocFile = Option::fixPathToTargetOS(mocFile);
		mocablesToMOC[fn_target] = mocFile;
		mocablesFromMOC[mocFile] = fn_target;
	    }
	    break;
	}
    }
#undef COMP_LEN
    return TRUE;
}


bool
MakefileGenerator::generateDependancies(QStringList &dirs, QString fn)
{
    QStringList &fndeps = depends[fn];
    if(!fndeps.isEmpty())
	return TRUE;
    fn = Option::fixPathToLocalOS(fn);

    int file = open(fn.latin1(), O_RDONLY);
    if(file == -1)
	return FALSE;

    struct stat fst;
    if(fstat(file, &fst))
	return FALSE; //shouldn't happen
    char *big_buffer = gimme_buffer(fst.st_size);

    int total_size_read;
    for(int have_read = total_size_read = 0; 
	have_read = read(file, big_buffer + total_size_read, fst.st_size - total_size_read);
	total_size_read += have_read);
    close(file);

    for(int x = 0; x < total_size_read; x++) {
	if(*(big_buffer + x) == '#') {
	    x++;
	    if(total_size_read >= x + 8 && !strncmp(big_buffer + x, "include ", 8)) {
		x += 8;
		char term = *(big_buffer + x);
		if(term == '"');
		else if(term == '<')
		    term = '>';
		else
		    continue; //wtf?
		x++;

		int inc_len;
		for(inc_len = 0; *(big_buffer + x + inc_len) != term; inc_len++);
		*(big_buffer + x + inc_len) = '\0';
		
		QString fqn, inc = big_buffer + x;
		if(!stat(inc, &fst))
		    fqn = inc;
		else if((Option::mode == Option::WIN_MODE && inc[1] != ':') || 
			(Option::mode == Option::UNIX_MODE && inc[0] != '/')) {
		    bool found = FALSE;
		    for(QStringList::Iterator it = dirs.begin(); !found && it != dirs.end(); ++it) {
			found = QFile::exists((fqn = ((*it) + QDir::separator() + inc)));
		    }
		    if(!found)
			continue;
		} else {
		    continue;
		}
		fqn = Option::fixPathToTargetOS(fqn);

		if(fndeps.findIndex(fqn) == -1)
		    fndeps.append(fqn);
	    }
	}
    }

    for(QStringList::Iterator fnit = fndeps.begin(); fnit != fndeps.end(); ++fnit) {
	generateDependancies(dirs, (*fnit));

	QStringList &deplist = depends[(*fnit)];
	for(QStringList::Iterator it = deplist.begin(); it != deplist.end(); ++it)
	    if(fndeps.findIndex((*it)) == -1)
		fndeps.append((*it));
    }	

    if(Option::debug_level >= 2)
	printf("Dependancies: %s -> %s\n", fn.latin1(), fndeps.join(" :: ").latin1());

    return TRUE;
}

void
MakefileGenerator::init()
{
    if(init_already)
	return;
    init_already = TRUE;

    QMap<QString, QStringList> &v = project->variables();
    /* get deps and mocables */
    {
	QStringList incDirs;
	if(Option::do_deps) {
	    QString dirs[] = { QString("DEPENDPATH"), QString("INCLUDEPATH"), QString::null };
	    for(int y = 0; dirs[y] != QString::null; y++) {
		QStringList &l = v[dirs[y]];
		for(QStringList::Iterator val_it = l.begin(); val_it != l.end(); ++val_it)
		{
		    //apparently tmake used colon separation...
		    QStringList damn = QStringList::split(':', (*val_it));
		    if(!damn.isEmpty())
			incDirs += damn;
		    else
			incDirs.append((*val_it));
		}
	    }
	    for(QStringList::Iterator it = incDirs.begin(); it != incDirs.end(); ++it)
		(*it) = Option::fixPathToLocalOS((*it)); /* this is a local path */
	    if(Option::debug_level)
		printf("Dependancy Directories: %s\n", incDirs.join(" :: ").latin1());
	}


	QString sources[] = { QString("LEXSOURCES"), QString("YACCSOURCES"),
				  QString("HEADERS"), QString("SOURCES"), QString("INTERFACES"), QString::null };
	for(int x = 0; sources[x] != QString::null; x++) {
	    QStringList &l = v[sources[x]];
	    for(QStringList::Iterator val_it = l.begin(); val_it != l.end(); ++val_it) {
		if(!(*val_it).isEmpty()) {
		    if(Option::do_deps)
			generateDependancies(incDirs, (*val_it));
		    if(mocAware())
			generateMocList((*val_it));
		}
	    }
	}
    }

    /* init variables */
    v["OBJECTS"].clear();
    v["OBJECTS"] += createObjectList("SOURCES");

    //UI files
    {
	QStringList &decls = v["UICDECLS"], &impls = v["UICIMPLS"];
	QStringList &l = v["INTERFACES"];
	for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
	    QFileInfo fi((*it));
	    QString impl = fi.dirPath() + Option::dir_sep + fi.baseName() + Option::cpp_ext;
	    QString decl = fi.dirPath() + Option::dir_sep + fi.baseName() + Option::h_ext;
	    decls.append(decl);
	    impls.append(impl);
	    depends[impl].append(decl);

	    QString mocable = (v["MOC_DIR"].isEmpty() ? (fi.dirPath() + Option::dir_sep): v["MOC_DIR"].first()) +
			      Option::moc_mod + fi.baseName() + Option::cpp_ext;
	    mocablesToMOC[decl] = mocable;
	    mocablesFromMOC[mocable] = decl;
	    v["_UIMOC"].append(mocable);
	}
	v["OBJECTS"] += (v["UICOBJECTS"] = createObjectList("INTERFACES"));
    }

#if 1
//this is defined for now because moc would have to change to get this done properly, I will
//revisit this once the build process soldifies, FIXME
#define MOC_YACC_LEX_HACKS
#endif

    //lex files
    {
	QStringList &impls = v["LEXIMPLS"];
	QStringList &l = v["LEXSOURCES"];
	for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
	    QFileInfo fi((*it));
	    QString impl = fi.dirPath() + Option::dir_sep + fi.baseName() + Option::lex_mod + Option::cpp_ext;
	    impls.append(impl);
#ifndef MOC_YACC_LEX_HACKS
	    v["SOURCES"].append(impl);
#endif
	}
#ifndef MOC_YACC_LEX_HACKS
	v["OBJECTS"] += (v["LEXOBJECTS"] = createObjectList("LEXIMPLS"));
#endif
    }
    //yacc files
    {
	QStringList &decls = v["YACCCDECLS"], &impls = v["YACCIMPLS"];
	QStringList &l = v["YACCSOURCES"];
	for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
	    QFileInfo fi((*it));
	    QString impl = fi.dirPath() + Option::dir_sep + fi.baseName() + Option::yacc_mod + Option::cpp_ext;
	    QString decl = fi.dirPath() + Option::dir_sep + fi.baseName() + Option::yacc_mod + Option::h_ext;
	    decls.append(decl);
	    impls.append(impl);
	    v["SOURCES"].append(impl);
	    depends[impl].append(decl);

#ifdef MOC_YACC_LEX_HACKS
	    QString leximpl = fi.dirPath() + Option::dir_sep + fi.baseName() + Option::lex_mod + Option::cpp_ext;
	    if(v["LEXIMPLS"].findIndex(leximpl) != -1)
		depends[impl].append(leximpl);
#endif
	}
	v["OBJECTS"] += (v["YACCOBJECTS"] = createObjectList("YACCIMPLS"));
    }

    //moc files
    if ( mocAware() ) {
	if(!project->variables()["MOC_DIR"].isEmpty())
	    project->variables()["INCLUDEPATH"].append(project->variables()["MOC_DIR"].first());
	v["OBJMOC"] = createObjectList("_HDRMOC") + createObjectList("_UIMOC");
	v["SRCMOC"] = v["_HDRMOC"] + v["_SRCMOC"] + v["_UIMOC"];
    }
}

bool
MakefileGenerator::write()
{
    init();

    QTextStream t(&Option::output);
    writeMakefile(t);
    return TRUE;
}

void
MakefileGenerator::writeObj(QTextStream &t, const QString &obj, const QString &src)
{
    QStringList &objl = project->variables()[obj];
    QStringList &srcl = project->variables()[src];

    QStringList::Iterator oit = objl.begin();
    QStringList::Iterator sit = srcl.begin();
    for( ;oit != objl.end(); oit++, sit++) {
	if((*sit).isEmpty())
	    continue;

	t << (*oit) << ": " << (*sit) << " \\\n\t\t"
	  << depends[(*sit)].join(" \\\n\t\t");

	QString comp, cimp;
	if((*sit).right(strlen(Option::cpp_ext)) == Option::cpp_ext) {
	    comp = "QMAKE_RUN_CXX";
	    cimp = "QMAKE_RUN_CXX_IMP";
	} else {
	    comp = "QMAKE_RUN_CC";
	    cimp = "QMAKE_RUN_CC_IMP";
	}
	if ( !project->variables()["OBJECTS_DIR"].isEmpty() || project->variables()[cimp].isEmpty()) {
	    QString p = var(comp);
	    p.replace(QRegExp("\\$src"), (*sit));
	    p.replace(QRegExp("\\$obj"), (*oit));
	    t << "\n\t" << p;
	}
	t << endl << endl;
    }
}


void
MakefileGenerator::writeUicSrc(QTextStream &t, const QString &ui)
{
    QStringList &uil = project->variables()[ui];
    for(QStringList::Iterator it = uil.begin(); it != uil.end(); it++) {
	QFileInfo fi(*it);
	QString decl = fi.dirPath() + Option::dir_sep + fi.baseName() + Option::h_ext;
	QString impl = fi.dirPath() + Option::dir_sep + fi.baseName() + Option::cpp_ext;

	t << decl << ": " << (*it) << "\n\t"
	  << "$(UIC) " << (*it) << " -o " << decl << endl << endl;

	QString declnopath = fi.baseName() + Option::h_ext;
	if(declnopath != decl)
	    t << decl << ": " << (*it) << "\n\t"
	      << "$(UIC) " << (*it) << " -o " << decl << endl << endl;

	t << impl << ": " << (*it) << "\n\t"
	  << "$(UIC) " << (*it) << " -i " << decl << " -o " << impl << endl << endl;
    }
}


void
MakefileGenerator::writeMocObj(QTextStream &t, const QString &obj, const QString &src)
{
    QStringList &objl = project->variables()[obj];
    QStringList &srcl = project->variables()[src];

    QStringList::Iterator oit = objl.begin();
    QStringList::Iterator sit = srcl.begin();
    for( ;oit != objl.end(); oit++, sit++) {
	if((*sit).isEmpty())
	    continue;

	QString &hdr = mocablesFromMOC[(*sit)];
	t << (*oit) << ": " << (*sit) << " \\\n\t\t"
	  << hdr << " \\\n\t\t"
	  << depends[hdr].join(" \\\n\t\t");
	if ( !project->variables()["OBJECTS_DIR"].isEmpty() ||
	     !project->variables()["MOC_DIR"].isEmpty() ||
	     project->variables()["QMAKE_RUN_CXX_IMP"].isEmpty()) {
	    QString p = var("QMAKE_RUN_CXX");
	    p.replace(QRegExp("\\$src"), (*sit));
	    p.replace(QRegExp("\\$obj"), (*oit));
	    t << "\n\t" << p;
	}
	t << endl << endl;
    }
}


void
MakefileGenerator::writeMocSrc(QTextStream &t, const QString &src)
{
    QStringList &l = project->variables()[src];
    for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
	QString &m = mocablesToMOC[(*it)];
	if ( !m.isEmpty())
	    t << m << ": " << (*it) << "\n\t"
	      << "$(MOC) " << (*it) << " -o " << m << endl << endl;
    }
}

void
MakefileGenerator::writeYaccSrc(QTextStream &t, const QString &src)
{
    QStringList &l = project->variables()[src];
    for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
	QFileInfo fi((*it));
	QString impl = fi.dirPath() + Option::dir_sep + fi.baseName() + Option::yacc_mod + Option::cpp_ext;
	QString decl = fi.dirPath() + Option::dir_sep + fi.baseName() + Option::yacc_mod + Option::h_ext;

	t << impl << ": " << (*it) << " \\\n\t\t"
	  << depends[(*it)].join(" \\\n\t\t") << "\n\t"
	  << "$(YACC) $(YACCFLAGS) " << (*it) << "\n\t"
	  << "-rm -f " << impl << " " << decl << "\n\t"
	  << "-mv y.tab.h " << decl << "\n\t"
	  << "-mv y.tab.c " << impl << endl << endl;

	t << decl << ": " << impl << endl << endl;
    }
}

void
MakefileGenerator::writeLexSrc(QTextStream &t, const QString &src)
{
    QStringList &l = project->variables()[src];
    for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
	QFileInfo fi((*it));
	QString impl = fi.dirPath() + Option::dir_sep + fi.baseName() + Option::lex_mod + Option::cpp_ext;

	t << impl << ": " << (*it) << " \\\n\t\t"
	  << depends[(*it)].join(" \\\n\t\t") << "\n\t"
	  << "$(LEX) $(LEXFLAGS) " << (*it) << "\n\t"
	  << "-rm -f " << impl << " " << "\n\t"
	  << "-mv lex.yy.c " << impl << endl << endl;
    }
}

QString
MakefileGenerator::var(const QString &var)
{
    return varGlue(var, "", " ", "");
}


QString
MakefileGenerator::varGlue(const QString &var, const QString &before, const QString &glue, const QString &after)
{
    QString ret;
    QStringList &l = project->variables()[var];
    for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
	if(!(*it).isEmpty()) {
	    if(!ret.isEmpty())
		ret += glue;
	    ret += (*it);
	}
    }
    return ret.isEmpty() ? QString("") : before + ret + after;
}


QString
MakefileGenerator::varList(const QString &var)
{
    return varGlue(var, "", " \\\n\t\t", "");
}


QStringList
MakefileGenerator::createObjectList(const QString &var)
{
    QStringList ret;
    QStringList &l = project->variables()[var];
    QString dir;
    if(!project->variables()["OBJECTS_DIR"].isEmpty())
	dir = project->variables()["OBJECTS_DIR"].first();
    for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
	QFileInfo fi(Option::fixPathToLocalOS((*it)));
	ret.append((dir.isEmpty() ? fi.dirPath() + Option::dir_sep : dir) + fi.baseName() + Option::obj_ext);
    }
    return ret;
}

bool
MakefileGenerator::writeMakefile(QTextStream &t)
{
    t << "####### Compile" << endl << endl;
    writeObj(t, "OBJECTS", "SOURCES");
    writeUicSrc(t, "INTERFACES");
    writeObj(t, "UICOBJECTS", "UICIMPLS");
    writeMocObj(t, "OBJMOC", "SRCMOC");
    writeMocSrc(t, "HEADERS");
    writeMocSrc(t, "SOURCES");
    writeMocSrc(t, "UICDECLS");
    writeYaccSrc(t, "YACCSOURCES");
    writeLexSrc(t, "LEXSOURCES");
	return TRUE;
}


bool
MakefileGenerator::writeHeader(QTextStream &t)
{
    time_t foo = time(NULL);
    t << "#############################################################################" << endl;
    t << "# Makefile for building: " << var("TARGET") << endl;
    t << "# Generated by qmake on: " << ctime(&foo);
    t << "# Project:  " << project->projectFile() << endl;
    t << "# Template: " << var("TEMPLATE") << endl;
    t << "#############################################################################" << endl;
    t << endl;
	return TRUE;
}
