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

#if defined(Q_OS_UNIX)
#include <unistd.h>
#else
#include <io.h>
#endif
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


MakefileGenerator::MakefileGenerator(QMakeProject *p) : init_already(FALSE), moc_aware(FALSE), project(p)
{
}

#ifndef NO_USE_GROSS_BIG_BUFFER_THING
static char *gimme_buffer(off_t s)
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
    if(!findMocDestination(fn_target).isEmpty())
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
	(have_read = read(file, big_buffer + total_size_read,
			  fst.st_size - total_size_read));
	total_size_read += have_read);
    close(file);

    bool ignore_qobject = FALSE;
    int line_count = 0;
#define COMP_LEN 8 //strlen("Q_OBJECT")
#define OBJ_LEN 8 //strlen("Q_OBJECT")
#define DIS_LEN 10 //strlen("Q_DISPATCH")
    for(int x = 0; x < (total_size_read-COMP_LEN); x++) {
	if(*(big_buffer + x) == '/') {
	    x++;
	    if(total_size_read >= x) {
		if(*(big_buffer + x) == '/') { //c++ style comment
		    for( ;x < total_size_read && *(big_buffer + x) != '\n'; x++);
		    line_count++;
		} else if(*(big_buffer + x) == '*') { //c style comment
		    for( ;x < total_size_read; x++) {
			if(*(big_buffer + x) == 't' || *(big_buffer + x) == 'q') { //ignore
			    if(total_size_read >= (x + 20)) {
				if(!strncmp(big_buffer + x + 1, "make ignore Q_OBJECT", 20)) {
				    debug_msg(2, "Mocgen: %s:%d Found \"qmake ignore Q_OBJECT\"", 
					      fn_target.latin1(), line_count);
				    x += 20;
				    ignore_qobject = !ignore_qobject;
				}
			    }
			} else if(*(big_buffer + x) == '*') {
			    if(total_size_read >= (x+1) && *(big_buffer + (x+1)) == '/') {
				x += 2;
				break;
			    }
			} else if(*(big_buffer + x) == '\n') {
			    line_count++;
			}
		    }
		}
	    }
	}
#define SYMBOL_CHAR(x) ((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z') || \
			(x <= '0' && x >= '9') || x == '_') 
	bool interesting = *(big_buffer+x) == 'Q' && (!strncmp(big_buffer+x, "Q_OBJECT", OBJ_LEN) ||
						      !strncmp(big_buffer+x, "Q_DISPATCH", DIS_LEN));
	if(interesting) {
	    int len = 0;
	    if(!strncmp(big_buffer+x, "Q_OBJECT", OBJ_LEN)) {
		if(ignore_qobject) {
		    debug_msg(2, "Mocgen: %s:%d Ignoring Q_OBJECT", fn_target.latin1(), line_count);
		    interesting = FALSE;
		}
		len=OBJ_LEN;
	    } else if(!strncmp(big_buffer+x, "Q_DISPATH", DIS_LEN)) {
		len=DIS_LEN;
	    }
	    if(SYMBOL_CHAR(*(big_buffer+x+len)))
		interesting = FALSE;
	    if(interesting) {
		*(big_buffer+x+len) = '\0';
		debug_msg(2, "Mocgen: %s:%d Found MOC symbol %s", fn_target.latin1(), line_count, big_buffer+x);

		int ext_pos = fn_target.findRev('.');
		int ext_len = fn_target.length() - ext_pos;
		int dir_pos =  fn_target.findRev(Option::dir_sep, ext_pos);
		QString mocFile;
		if(!project->variables()["MOC_DIR"].isEmpty())
		    mocFile = project->first("MOC_DIR");
		else
		    mocFile = "." + Option::dir_sep; //fn_target.left(dir_pos+1);

		if(fn_target.right(ext_len) == Option::cpp_ext) {
		    mocFile += fn_target.mid(dir_pos+1, ext_pos - dir_pos-1) + Option::moc_ext;
		    depends[fn_target].append(mocFile);
		    project->variables()["_SRCMOC"].append(mocFile);
		} else if(fn_target.right(ext_len) == Option::h_ext &&
			  project->variables()["HEADERS"].findIndex(fn_target) != -1) {
		    mocFile += Option::moc_mod + fn_target.mid(dir_pos+1, ext_pos - dir_pos-1) + Option::cpp_ext;
		    project->variables()["_HDRMOC"].append(mocFile);
		}
		
		if(!mocFile.isEmpty()) {
		    mocFile = Option::fixPathToTargetOS(mocFile);
		    mocablesToMOC[cleanFilePath(fn_target)] = mocFile;
		    mocablesFromMOC[cleanFilePath(mocFile)] = fn_target;
		}
		break;
	    }
	}
	    while(x < total_size_read && SYMBOL_CHAR(*(big_buffer+x)))
		x++;
	if(*(big_buffer+x) == '\n') 
	    line_count++;
    }
#undef OBJ_LEN
#undef DIS_LEN
    return TRUE;
}

bool
MakefileGenerator::generateDependancies(QStringList &dirs, QString fn)
{
    fileFixify(fn);

    QStringList &fndeps = depends[fn];
    if(!fndeps.isEmpty())
	return TRUE;
    fn = Option::fixPathToLocalOS(fn);

    QString fndir;
    int dl = fn.findRev(Option::dir_sep);
    if(dl != -1)
	fndir = fn.left(dl+1);

    int file = open(fn.latin1(), O_RDONLY);
    if(file == -1)
	return FALSE;

    struct stat fst;
    if(fstat(file, &fst))
	return FALSE; //shouldn't happen
    char *big_buffer = gimme_buffer(fst.st_size);

    int total_size_read;
    for(int have_read = total_size_read = 0;
	(have_read = read(file, big_buffer + total_size_read,
			  fst.st_size - total_size_read));
	total_size_read += have_read);
    close(file);

    enum { UI_FILE, C_FILE } ftype;
    if(fn.right(3) == ".ui")
	ftype = UI_FILE;
    else
	ftype = C_FILE;


    for(int x = 0; x < total_size_read; x++) {
	QString inc;
	if(ftype == C_FILE) {
	    if(*(big_buffer + x) == '/') {
		x++;
		if(total_size_read >= x) {
		    if(*(big_buffer + x) == '/') { //c++ style comment
			for( ; x < total_size_read && *(big_buffer + x) != '\n'; x++);
		    } else if(*(big_buffer + x) == '*') { //c style comment
			for( ; x < total_size_read; x++) {
			    if(*(big_buffer + x) == '*') {
				if(total_size_read >= (x+1) && *(big_buffer + (x+1)) == '/') {
				    x += 2;
				    break;
				}
			    }
			}
		    }
		}
	    }
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

		    inc = big_buffer + x;
		}
	    }
	} else if(ftype == UI_FILE) {
	    if(*(big_buffer + x) == '<') {
		x++;
		if(total_size_read >= x + 8 && !strncmp(big_buffer + x, "include ", 8)) {
		    x += 8;
		    while(*(big_buffer + (x++)) != '>');
		    int inc_len = 0;
		    for( ; *(big_buffer + x + inc_len) != '<'; inc_len++);
		    *(big_buffer + x + inc_len) = '\0';

		    inc = big_buffer + x;
		}
	    }
	}

	if(!inc.isEmpty()) {
	    QString fqn;
	    if(!stat(fndir + inc, &fst))
		fqn = fndir + inc;
	    else {
		if((Option::target_mode == Option::TARG_MAC9_MODE && inc.find(':')) ||
		   (Option::target_mode == Option::TARG_WIN_MODE && inc[1] != ':') ||
		   ((Option::target_mode == Option::TARG_UNIX_MODE || Option::target_mode == Option::TARG_MACX_MODE) && inc[0] != '/')) {
		    bool found = FALSE;
		    for(QStringList::Iterator it = dirs.begin(); !found && it != dirs.end(); ++it) {
			QString dep = (*it) + QDir::separator() + inc;
			if((found = QFile::exists(dep)))
			    fqn = dep;
		    }
		}
	    }
	    if(fqn.isEmpty()) {
		//these are some hacky heuristics it will try to do on an include
		//however these can be turned off at runtime, I'm not sure how
		//reliable these will be, most likely when problems arise turn it off
		//and see if they go away..
		if(Option::mkfile::do_dep_heuristics) { //some heuristics..
		    //is it a file from a .ui?
		    int extn = inc.findRev('.');
		    if(extn != -1) {
			QString uip = inc.left(extn) + Option::ui_ext + "$";
			QStringList uil = project->variables()["INTERFACES"];
			for(QStringList::Iterator it = uil.begin(); it != uil.end(); ++it) {
			    if((*it).find(QRegExp(uip)) != -1) {
				fqn = (*it).left((*it).length()-3) + inc.right(inc.length()-extn);
				break;
			    }
			}
		    }
		}
		if(!Option::mkfile::do_dep_heuristics || fqn.isEmpty()) //I give up
		    continue;
	    }

	    fqn = Option::fixPathToTargetOS(fqn);
	    fileFixify(fqn);

	    if(fndeps.findIndex(fqn) == -1)
		fndeps.append(fqn);
	}
	//read past new line now..
	for( ; x < total_size_read && (*(big_buffer + x) != '\n'); x++);
    }

    for(QStringList::Iterator fnit = fndeps.begin(); fnit != fndeps.end(); ++fnit) {
	generateDependancies(dirs, (*fnit));

	QStringList &deplist = depends[(*fnit)];
	for(QStringList::Iterator it = deplist.begin(); it != deplist.end(); ++it)
	    if(fndeps.findIndex((*it)) == -1)
		fndeps.append((*it));
    }
    debug_msg(2, "Dependancies: %s -> %s", fn.latin1(), fndeps.join(" :: ").latin1());
    return TRUE;
}
#else
bool
MakefileGenerator::generateMocList(QString fn_target)
{
    if(!findMocDestination(fn_target).isEmpty())
	return TRUE;

    QString fn_local = Option::fixPathToLocalOS(fn_target);
    QFile file(fn_local);
    const QString stringObj( "Q_OBJECT" );
    const QString stringDis( "Q_DISPATCH" );
    if ( file.open(IO_ReadOnly) ) {
	QTextStream t( &file );
	QString s;
	while ( !t.eof() ) {
	    s = t.readLine();
	    if( s.find(stringObj) != -1 || s.find(stringDis) != -1) {
		int ext_pos = fn_target.findRev('.');
		int ext_len = fn_target.length() - ext_pos;
		int dir_pos =  fn_target.findRev(Option::dir_sep, ext_pos);
		QString mocFile;
		QFileInfo fi(fn_local);
		if(!project->variables()["MOC_DIR"].isEmpty())
		    mocFile = project->first("MOC_DIR");
		else
		    mocFile = fi.dirPath() + Option::dir_sep;

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
		    mocablesToMOC[cleanFilePath(fn_target)] = mocFile;
		    mocablesFromMOC[cleanFilePath(mocFile)] = fn_target;
		}
		file.close();
		return TRUE;
	    }
	}
	file.close();
    }
    return FALSE;
}


#define GOOD_FIND

bool
MakefileGenerator::generateDependancies(QStringList &dirs, QString fn)
{
    fileFixify(fn);

    int pos;
    int end;
    int l;
    // don't search for '#include' since you can write '#  include' (using
    // QRegExp instead is too expensive)
    const QString stringInc( "include" );
    QChar term;

    QStringList &fndeps = depends[fn];
    if(!fndeps.isEmpty())
	return TRUE;
    fn = Option::fixPathToLocalOS(fn);

    QString fndir;
    int dl = fn.findRev(Option::dir_sep);
    if(dl != -1)
	fndir = fn.left(dl+1);

    QFile file(fn);
    if ( file.open(IO_ReadOnly) ) {
	QTextStream t( &file );
	QString s, inc;
	while ( !t.eof() ) {
	    s = t.readLine();
	    pos = s.find( stringInc );
	    if ( pos == -1 )
		continue;

	    // find the name of the header
	    l = s.length();
#if defined(GOOD_FIND)
	    pos += 7; // go to character after 'include'
	    while ( pos < l ) {
		if ( s[pos] == '<' ) {
		    term = '>';
		    break;
		} else if ( s[pos] == '"' ) {
		    term = '"';
		    break;
		}
		pos++;
	    }
	    if ( pos >= l )
		continue; // no quoting found
#else
	    pos += 8; // go to character after 'include '
	    if ( pos >= l )
		continue;
	    if ( s[pos] == '<' ) {
		term = '>';
	    } else if ( s[pos] == '"' ) {
		term = '"';
	    } else {
		continue;
	    }
#endif
	    pos++;
	    end = s.find( term, pos );
	    if ( end == -1 )
		continue;
	    inc = s.mid( pos, end-pos );

	    QString fqn;
	    if( QFile::exists(inc) )
		fqn = inc;
	    else if( QDir::isRelativePath(inc) ) {
		bool found = FALSE;
		for(QStringList::Iterator it = dirs.begin(); !found && it != dirs.end(); ++it) {
		    QString dep = (*it) + QDir::separator() + inc;
		    if((found = QFile::exists(dep)))
			fqn = dep;
		}
	    }
	    if(fqn.isEmpty()) {
		//these are some hacky heuristics it will try to do on an
		//include however these can be turned off at runtime, I'm not
		//sure how reliable these will be, most likely when problems
		//arise turn it off and see if they go away..
		if(Option::do_dep_heuristics) { //some heuristics..
		    //is it a file from a .ui?
		    int extn = inc.findRev('.');
		    if(extn != -1) {
			QString uip = inc.left(extn) + Option::ui_ext + "$";
			QStringList uil = project->variables()["INTERFACES"];
			for(QStringList::Iterator it = uil.begin(); it != uil.end(); ++it) {
			    if((*it).find(QRegExp(uip)) != -1) {
				fqn = (*it).left((*it).length()-3) + inc.right(inc.length()-extn);
				break;
			    }
			}
		    }
		}
		if(!Option::do_dep_heuristics || fqn.isEmpty()) //I give up
		    continue;
	    }
	    fqn = Option::fixPathToTargetOS(fqn);
	    fileFixify(fqn);

	    if(fndeps.findIndex(fqn) == -1)
		fndeps.append(fqn);

	}
	file.close();
	for(QStringList::Iterator fnit = fndeps.begin(); fnit != fndeps.end(); ++fnit) {
	    generateDependancies(dirs, (*fnit));

	    QStringList &deplist = depends[(*fnit)];
	    for(QStringList::Iterator it = deplist.begin(); it != deplist.end(); ++it)
		if(fndeps.findIndex((*it)) == -1)
		    fndeps.append((*it));
	}

	debug_msg(2, "Dependancies: %s -> %s", fn.latin1(), fndeps.join(" :: ").latin1());

	return TRUE;
    }
    return FALSE;
}
#endif

void
MakefileGenerator::init()
{
    if(init_already)
	return;
    init_already = TRUE;

    QMap<QString, QStringList> &v = project->variables();
    { //opaths
	if ( !v["QMAKE_ABSOLUTE_SOURCE_PATH"].isEmpty() ) {
	    QString &asp = v["QMAKE_ABSOLUTE_SOURCE_PATH"].first();
	    asp = Option::fixPathToTargetOS( asp );
	    if(asp.isEmpty() || asp == Option::output_dir) //if they're the same, why bother?
		v["QMAKE_ABSOLUTE_SOURCE_PATH"].clear();
	}
	QString currentDir = QDir::currentDirPath();
	QString dirs[] = { QString("OBJECTS_DIR"), QString("MOC_DIR"), QString("DESTDIR"), QString::null };
	for(int x = 0; dirs[x] != QString::null; x++) {
	    if ( !v[dirs[x]].isEmpty() ) {
		{
		    QString &path = v[dirs[x]].first();
		    if(path.right(Option::dir_sep.length()) != Option::dir_sep)
			path += Option::dir_sep;
		    if(QDir::isRelativePath(path) && !v["QMAKE_ABSOLUTE_SOURCE_PATH"].isEmpty())
			path.prepend(Option::output_dir + Option::dir_sep);
		}
		QString path = project->first(dirs[x]); //not to be changed any further
		path = Option::fixPathToTargetOS(path);

		QDir d;
		if ( !QDir::isRelativePath( path ) )
		    d.cd( path.left( 2 ) );
		if(path.left(1) == Option::dir_sep)
		    d.cd(Option::dir_sep);

		QStringList subs = QStringList::split(Option::dir_sep, path);
		for(QStringList::Iterator subit = subs.begin(); subit != subs.end(); ++subit) {
		    if(!d.cd(*subit)) {
			d.mkdir((*subit));
			d.cd((*subit));
		    }
		}
	    }
	}
	QDir::current().cd( currentDir );
    }

    QString paths[] = { QString("SOURCES"), QString("INTERFACES"), QString("YACCSOURCES"), QString("INCLUDEPATH"),
			    QString("HEADERS"),
			    QString("LEXSOURCES"), QString("QMAKE_INTERNAL_INCLUDED_FILES"), QString::null };
    for(int y = 0; paths[y] != QString::null; y++) {
	QStringList &l = v[paths[y]];
	if(!l.isEmpty())
	    fileFixify(l);
    }

    /* get deps and mocables */
    {
	QStringList incDirs;
	if(Option::mkfile::do_deps) {
	    QString dirs[] = { QString("QMAKE_ABSOLUTE_SOURCE_PATH"),
				   QString("INCLUDEPATH"), QString("DEPENDPATH"), QString::null };
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
	    debug_msg(1, "Dependancy Directories: %s", incDirs.join(" :: ").latin1());
	}

	QString sources[] = { QString("LEXSOURCES"), QString("YACCSOURCES"),
				  QString("HEADERS"), QString("SOURCES"), QString("INTERFACES"), QString::null };
	for(int x = 0; sources[x] != QString::null; x++) {
	    QStringList &l = v[sources[x]];
	    for(QStringList::Iterator val_it = l.begin(); val_it != l.end(); ++val_it) {
		if(!(*val_it).isEmpty()) {
		    if(Option::mkfile::do_deps)
			generateDependancies(incDirs, (*val_it));
		    if(mocAware()) {
			if(!generateMocList((*val_it))) {
			    fprintf(stderr, "Failure to open: %s\n", (*val_it).latin1());
			}
		    }
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
	    QString impl, decl;
            if ( fi.dirPath() == "." ) {
	    	impl = fi.baseName() + Option::cpp_ext;
                decl = fi.baseName() + Option::h_ext;
	    } else {
	    	impl = fi.dirPath() + Option::dir_sep + fi.baseName() + Option::cpp_ext;
                decl = fi.dirPath() + Option::dir_sep + fi.baseName() + Option::h_ext;
	    }
	    decls.append(decl);
	    impls.append(impl);
	    depends[impl].append(decl);

	    QString mocable = (v["MOC_DIR"].isEmpty() ? (fi.dirPath() + Option::dir_sep): v["MOC_DIR"].first()) +
			      Option::moc_mod + fi.baseName() + Option::cpp_ext;
	    mocablesToMOC[cleanFilePath(decl)] = mocable;
	    mocablesFromMOC[cleanFilePath(mocable)] = decl;
	    v["_UIMOC"].append(mocable);
	}
	v["OBJECTS"] += (v["UICOBJECTS"] = createObjectList("UICDECLS"));
    }

    //lex files
    {
	QStringList &impls = v["LEXIMPLS"];
	QStringList &l = v["LEXSOURCES"];
	for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
	    QFileInfo fi((*it));
	    QString impl = fi.dirPath() + Option::dir_sep + fi.baseName() + Option::lex_mod + Option::cpp_ext;
	    impls.append(impl);
	    if( ! project->isActiveConfig("lex_included"))
		v["SOURCES"].append(impl);
	}
	if( ! project->isActiveConfig("lex_included"))
	    v["OBJECTS"] += (v["LEXOBJECTS"] = createObjectList("LEXIMPLS"));
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

	    if( project->isActiveConfig("lex_included")) {
		QString leximpl = fi.dirPath() + Option::dir_sep + fi.baseName() + Option::lex_mod + Option::cpp_ext;
		if(v["LEXIMPLS"].findIndex(leximpl) != -1)
		    depends[impl].append(leximpl);
	    }
	}
	v["OBJECTS"] += (v["YACCOBJECTS"] = createObjectList("YACCIMPLS"));
    }

    //moc files

    if ( mocAware() ) {
	if(!project->variables()["MOC_DIR"].isEmpty())
	    project->variables()["INCLUDEPATH"].append(project->first("MOC_DIR"));
	v["OBJMOC"] = createObjectList("_HDRMOC") + createObjectList("_UIMOC");

	QStringList &l = v["SRCMOC"];
	l = v["_HDRMOC"] + v["_SRCMOC"] + v["_UIMOC"];
	for(QStringList::Iterator val_it = l.begin(); val_it != l.end(); ++val_it)
	    if(!(*val_it).isEmpty())
		(*val_it) = Option::fixPathToTargetOS((*val_it));
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
    QRegExp regexpSrc("\\$src");
    QRegExp regexpObj("\\$obj");
    for( ;sit != srcl.end() && oit != objl.end(); oit++, sit++) {
	if((*sit).isEmpty())
	    continue;

	t << (*oit) << ": " << (*sit) << " "
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
	    p.replace(regexpSrc, (*sit));
	    p.replace(regexpObj, (*oit));
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
	QString deps =  depends[(*it)].join(" \\\n\t\t"), decl, impl;
	{
	    QString tmp = (*it);
	    decl = tmp.replace(QRegExp("\\.ui$"), Option::h_ext);
	    tmp = (*it);
	    impl = tmp.replace(QRegExp("\\.ui$"), Option::cpp_ext);
	}
	t << decl << ": " << (*it) << " " << deps << "\n\t"
	  << "$(UIC) " << (*it) << " -o " << decl << endl << endl;

	t << impl << ": " << decl << " " << (*it) << " " << deps << "\n\t"
	  << "$(UIC) " << (*it) << " -i " << decl << " -o " << impl << endl << endl;
    }
}


void
MakefileGenerator::writeMocObj(QTextStream &t, const QString &obj)
{
    QStringList &objl = project->variables()[obj];
    QStringList::Iterator oit = objl.begin();
    QRegExp regexpSrc("\\$src");
    QRegExp regexpObj("\\$obj");

    QString mocdir;
    if(!project->variables()["MOC_DIR"].isEmpty())
	mocdir = project->first("MOC_DIR");

    for( ;oit != objl.end(); oit++) {
	QFileInfo fi(Option::fixPathToLocalOS((*oit)));
	QString dirName;
	if ( mocdir.isEmpty() )
	    dirName = Option::fixPathToTargetOS(fi.dirPath()) + Option::dir_sep;
	else
	    dirName = mocdir;
	QString src(dirName + fi.baseName() + Option::cpp_ext );

	QString hdr = findMocSource(src);
	t << (*oit) << ": " << src << " "
	  << hdr << " "
	  << depends[hdr].join(" \\\n\t\t");
	if ( !project->variables()["OBJECTS_DIR"].isEmpty() ||
	     !project->variables()["MOC_DIR"].isEmpty() ||
	     project->variables()["QMAKE_RUN_CXX_IMP"].isEmpty()) {
	    QString p = var("QMAKE_RUN_CXX");
	    p.replace( regexpSrc, src);
	    p.replace( regexpObj, (*oit));
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
	QString m = findMocDestination(*it);
	if ( !m.isEmpty())
	    t << m << ": $(MOC) " << (*it) << "\n\t"
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

	t << impl << ": " << (*it) << " "
	  << depends[(*it)].join(" \\\n\t\t") << "\n\t"
	  << "$(YACC) $(YACCFLAGS) " << (*it) << "\n\t"
	  << "-$(DEL) " << impl << " " << decl << "\n\t"
	  << "-$(MOVE)  y.tab.h " << decl << "\n\t"
	  << "-$(MOVE) y.tab.c " << impl << endl << endl;
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

	t << impl << ": " << (*it) << " "
	  << depends[(*it)].join(" \\\n\t\t") << "\n\t"
	  << "$(LEX) $(LEXFLAGS) " << (*it) << "\n\t"
	  << "-$(DEL) " << impl << " " << "\n\t"
	  << "-$(MOVE) lex.yy.c " << impl << endl << endl;
    }
}

void
MakefileGenerator::writeInstalls(QTextStream &t, const QString &installs)
{
    QString all_installs;
    QStringList &l = project->variables()[installs];
    for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
	QString pvar = (*it) + ".path";
	if(project->variables()[pvar].isEmpty()) {
	    fprintf(stderr, "%s is not defined: install target not created\n", pvar.latin1());
	    continue;
	}

	QString target;
	QStringList tmp;
	//masks
	tmp = project->variables()[(*it) + ".files"];
	if(!tmp.isEmpty()) {
	    if(Option::target_mode == Option::TARG_WIN_MODE || Option::target_mode == Option::TARG_MAC9_MODE) {
	    } else if(Option::target_mode == Option::TARG_UNIX_MODE || Option::target_mode == Option::TARG_MACX_MODE) {
		target += QString("$(COPY) -pR ") + tmp.join(" ") + QString(" ") + project->variables()[pvar].first();
	    }
	}
	//other
	tmp = project->variables()[(*it) + ".extra"];
	if(!tmp.isEmpty()) {
	    if(!target.isEmpty())
		target += "\n\t";
	    target += tmp.join(" ");
	}
	//default?
	if(target.isEmpty())
	    target = defaultInstall((*it));

	if(!target.isEmpty()) {
	    t << "install_" << (*it) << ": " << "\n\t"
	      << "[ -d " << project->variables()[pvar].first() << " ] || mkdir -p "
	      << project->variables()[pvar].first() << "\n\t"
	      << target << endl << endl;
	    all_installs += QString("install_") + (*it) + " ";
	}   else {
	    debug_msg(1, "no definition for install %s: install target not created",(*it).latin1());
	}
    }
    t << "install: all " << all_installs << "\n\n";
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
    QString dir, file;
    if(!project->variables()["OBJECTS_DIR"].isEmpty())
	dir = project->first("OBJECTS_DIR");
    for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
	QFileInfo fi(Option::fixPathToLocalOS((*it)));
	QString dirName;
	if ( dir.isEmpty() )
	    dirName = Option::fixPathToTargetOS(fi.dirPath()) + Option::dir_sep;
	else
	    dirName = dir;

	file = dirName + fi.baseName() + Option::obj_ext;
	fileFixify(file);
	ret.append( file );
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
    writeMocObj(t, "OBJMOC" );
    writeMocSrc(t, "HEADERS");
    writeMocSrc(t, "SOURCES");
    writeMocSrc(t, "UICDECLS");
    writeYaccSrc(t, "YACCSOURCES");
    writeLexSrc(t, "LEXSOURCES");

    t << "####### Install" << endl << endl;
    writeInstalls(t, "INSTALLS");
    return TRUE;
}

//get store argv, but then it would have more options than are probably necesary
//this will try to guess the bare minimum..
QString MakefileGenerator::build_args()
{
    static QString ret;
    if(ret.isEmpty()) {
	//qmake itself
	ret = project->first("QMAKE_QMAKE");
	if(ret.isEmpty())
	    ret = "qmake"; //hope its in your path

	//special variables
	if(!project->variables()["QMAKE_ABSOLUTE_SOURCE_PATH"].isEmpty())
	    ret += " QMAKE_ABSOLUTE_SOURCE_PATH=\"" + project->first("QMAKE_ABSOLUTE_SOURCE_PATH") + "\"";

	//output
	QString ofile = Option::fixPathToTargetOS(Option::output.name());
	if(ofile.findRev(Option::dir_sep) != -1)
	    ofile = ofile.right(ofile.length() - ofile.findRev(Option::dir_sep) -1);
	if (!ofile.isEmpty() && ofile != project->first("QMAKE_MAKEFILE"))
	    ret += " -o " + ofile;
	//other options
	if(!Option::mkfile::do_cache)
	    ret += " -nocache";
	if(!Option::mkfile::do_deps)
	    ret += " -nodeps";
	if(!Option::mkfile::do_dep_heuristics)
	    ret += " -nodependheuristics";

	//arguments
	for(QStringList::Iterator it = Option::user_vars.begin(); it != Option::user_vars.end(); ++it) {
	    if((*it).left(strlen("QMAKE_ABSOLUTE_SOURCE_PATH")) != "QMAKE_ABSOLUTE_SOURCE_PATH")
		ret += " \"" + (*it) + "\"";
	}

	//inputs
	QStringList files = Option::mkfile::project_files;
	fileFixify(files);
	ret += " " + files.join(" ");
    }
    return ret;
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
    t << "# Command: " << build_args() << endl;
    t << "#############################################################################" << endl;
    t << endl;
	return TRUE;
}


//makes my life easier..
bool
MakefileGenerator::writeMakeQmake(QTextStream &t)
{
    QString ofile = Option::output.name();
    if(ofile.findRev(Option::dir_sep) != -1)
	ofile = ofile.right(ofile.length() - ofile.findRev(Option::dir_sep) -1);
    ofile = Option::fixPathToTargetOS(ofile);

    QString pfile = project->projectFile();
    if(pfile != "(stdin)") {
	QString qmake = build_args();
	fileFixify(pfile);
	if(!ofile.isEmpty()) {
	    t << ofile << ": " << pfile << " ";
	    if(Option::mkfile::do_cache)
		t << Option::mkfile::cachefile << " ";
	    t << project->variables()["QMAKE_INTERNAL_INCLUDED_FILES"].join(" \\\n\t\t") << "\n\t"
	      << qmake <<endl;
	}
	t << "qmake: " << "\n\t"
	  << "@" << qmake << endl << endl;
    }
    return TRUE;
}

bool
MakefileGenerator::fileFixify(QStringList &files) const
{
    if(files.isEmpty())
	return FALSE;
    int ret = 0;
    for(QStringList::Iterator it = files.begin(); it != files.end(); ++it)
	if(!(*it).isEmpty())
	    ret += (int)fileFixify((*it));
    return ret != 0;
}

bool
MakefileGenerator::fileFixify(QString &file) const
{
    if(file.isEmpty())
	return FALSE;
    QString orig_file = file;
    file = Option::fixPathToTargetOS(file);
    if(project->variables()["QMAKE_ABSOLUTE_SOURCE_PATH"].isEmpty()) { //relative
	if(QDir::isRelativePath(file))
	    return FALSE;
	QString match_dir = Option::output_dir;
	if(file.left(match_dir.length()) == match_dir) {
	    file = file.right(file.length() - (match_dir.length() + 1));
	} else {
	    //try to make no more than five ..'s
	    for(int i = 1; i <= 5; i++) {
		int sl = match_dir.findRev(Option::dir_sep);
		if(sl == -1)
		    break;
		match_dir = match_dir.left(sl);
		if(match_dir.isEmpty())
		    break;
		if(file.left(match_dir.length()) == match_dir) {
		    file = file.right(file.length() - (match_dir.length() + 1));
		    for(int o = 0; o < i; o++)
			file.prepend(".." + Option::dir_sep);
		}
	    }
	}
    } else { //absoluteify it
	if(!QDir::isRelativePath(file)) //already absolute
	    return FALSE;
	QFileInfo fi(file);
	if(fi.convertToAbs()) //strange
	    return FALSE;
	file = fi.filePath();
    }
    debug_msg(3, "Fixed %s :: to :: %s", orig_file.latin1(), file.latin1());
    return TRUE;
}

QString
MakefileGenerator::cleanFilePath(const QString &file) const
{
    QString ret = Option::fixPathToTargetOS(file);
    fileFixify(ret);
    return ret;
}
