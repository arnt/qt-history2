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
    QMap<QString, QStringList> &v = project->variables();
    QString currentDir = QDir::currentDirPath();
    if(!v["QMAKE_ABSOLUTE_SOURCE_PATH"].isEmpty()) {
	QString &asp = v["QMAKE_ABSOLUTE_SOURCE_PATH"].first();
	asp = Option::fixPathToTargetOS(asp);
	if(asp == currentDir) 
	    v["QMAKE_ABSOLUTE_SOURCE_PATH"].clear();
    }


    QString dirs[] = { QString("OBJECTS_DIR"), QString("MOC_DIR"), QString("DESTDIR"), QString::null };
    for(int x = 0; dirs[x] != QString::null; x++) {
	QString &path = v[dirs[x]].first();
	path = Option::fixPathToTargetOS(path);
	if (!path.isEmpty() ) {
	    if(path.right(Option::dir_sep.length()) != Option::dir_sep)
		path += Option::dir_sep;

	    QDir d;

	    if ( !QDir::isRelativePath( path ) )
		d.cd( path.left( 2 ) );
	    else if(!v["QMAKE_ABSOLUTE_SOURCE_PATH"].isEmpty())
		path = v["QMAKE_ABSOLUTE_SOURCE_PATH"].first() + Option::dir_sep + path;
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

#ifdef USE_GROSS_BIG_BUFFER_THING
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
#define OBJ_LEN 8 //strlen("Q_OBJECT")
#define DIS_LEN 10 //strlen("Q_DISPATCH")
    for(int x = 0; x < (total_size_read-COMP_LEN); x++) {
	if(*(big_buffer+x) == 'Q' && 
	   (!strncmp(big_buffer+x, "Q_OBJECT", OBJ_LEN) || !strncmp(big_buffer+x, "Q_DISPATCH", DIS_LEN))) {

	    int ext_pos = fn_target.findRev('.');
	    int ext_len = fn_target.length() - ext_pos;
	    int dir_pos =  fn_target.findRev(Option::dir_sep, ext_pos);
	    QString mocFile;
	    if(!project->variables()["MOC_DIR"].isEmpty())
		mocFile = project->variables()["MOC_DIR"].first();
	    else
		mocFile = "./"; //fn_target.left(dir_pos+1);

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
		mocablesToMOC[fn_target] = mocFile;
		mocablesFromMOC[mocFile] = fn_target;
	    }
	    break;
	}
    }
#undef OBJ_LEN
#undef DIS_LEN
    return TRUE;
}

bool
MakefileGenerator::generateDependancies(QStringList &dirs, QString fn)
{
    bool absolute = !project->variables()["QMAKE_ABSOLUTE_SOURCE_PATH"].isEmpty();
    if(absolute) 
	fileAbsolute(fn);
    
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
	have_read = read(file, big_buffer + total_size_read, fst.st_size - total_size_read);
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
	    else if((Option::mode == Option::WIN_MODE && inc[1] != ':') || 
		    (Option::mode == Option::UNIX_MODE && inc[0] != '/')) {
		bool found = FALSE;
		for(QStringList::Iterator it = dirs.begin(); !found && it != dirs.end(); ++it) {
		    QString dep = (*it) + QDir::separator() + inc;
		    if((found = QFile::exists(dep)))
			fqn = dep;
		}
	    }
	    if(fqn.isEmpty()) {
		//these are some hacky heuristics it will try to do on an include
		//however these can be turned off at runtime, I'm not sure how
		//reliable these will be, most likely when problems arise turn it off
		//and see if they go away..
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
	    if(absolute)
		fileAbsolute(fqn);

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

    if(Option::debug_level >= 2)
	printf("Dependancies: %s -> %s\n", fn.latin1(), fndeps.join(" :: ").latin1());

    return TRUE;
}
#else
bool
MakefileGenerator::generateMocList(QString fn_target)
{
    if(!mocablesToMOC[fn_target].isEmpty())
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
		    mocFile = project->variables()["MOC_DIR"].first();
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
		    mocablesToMOC[fn_target] = mocFile;
		    mocablesFromMOC[mocFile] = fn_target;
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
    bool absolute = !project->variables()["QMAKE_ABSOLUTE_SOURCE_PATH"].isEmpty();
    if(absolute) 
	fileAbsolute(fn);
    
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
	    if(absolute)
		fileAbsolute(fqn);

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

	if(Option::debug_level >= 2)
	    printf("Dependancies: %s -> %s\n", fn.latin1(), fndeps.join(" :: ").latin1());

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

    if(!v["QMAKE_ABSOLUTE_SOURCE_PATH"].isEmpty()) {
	QString paths[] = { QString("SOURCES"), QString("INTERFACES"), QString("YACCSOURCES"), QString("INCLUDEPATH"),
				QString("HEADERS"),
				QString("LEXSOURCES"), QString("QMAKE_INTERNAL_INCLUDED_FILES"), QString::null };
	for(int y = 0; paths[y] != QString::null; y++) {
	    QStringList &l = v[paths[y]];
	    if(!l.isEmpty()) 
		fileAbsolute(l);
	}
    }

    /* get deps and mocables */
    {
	QStringList incDirs;
	if(Option::do_deps) {
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
	    QString impl =  fi.dirPath() + Option::dir_sep + fi.baseName() + Option::cpp_ext;
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
	QFileInfo fi(*it);
	QString deps =  depends[(*it)].join(" \\\n\t\t");
	QString decl = fi.dirPath() + Option::dir_sep + fi.baseName() + Option::h_ext;
	QString impl = fi.dirPath() + Option::dir_sep + fi.baseName() + Option::cpp_ext;

	t << decl << ": " << (*it) << " " << deps << "\n\t"
	  << "$(UIC) " << (*it) << " -o " << decl << endl << endl;

	t << impl << ": " << (*it) << " " << deps << "\n\t"
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
	mocdir = project->variables()["MOC_DIR"].first();

    for( ;oit != objl.end(); oit++) {
	QFileInfo fi(Option::fixPathToLocalOS((*oit)));
	QString dirName;
	if ( mocdir.isEmpty() ) 
	    dirName = Option::fixPathToTargetOS(fi.dirPath()) + Option::dir_sep;
	else
	    dirName = mocdir;
	QString src(dirName + fi.baseName() + Option::cpp_ext );

	QString &hdr = mocablesFromMOC[src];
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
	QString &m = mocablesToMOC[(*it)];
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
	    if(Option::mode == Option::WIN_MODE) {
	    } else if(Option::mode == Option::UNIX_MODE) {
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
	}   else if(Option::debug_level >= 1) {
	    fprintf(stderr, "no definition for install %s: install target not created\n",(*it).latin1());
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
    QString dir;
    if(!project->variables()["OBJECTS_DIR"].isEmpty())
	dir = project->variables()["OBJECTS_DIR"].first();
    for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
	QFileInfo fi(Option::fixPathToLocalOS((*it)));
	QString dirName;
	if ( dir.isEmpty() ) {
	    dirName = Option::fixPathToTargetOS(fi.dirPath()) + Option::dir_sep;
	} else {
	    dirName = dir;
	}
	ret.append( dirName + fi.baseName() + Option::obj_ext);
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


//makes my life easier..
bool
MakefileGenerator::writeMakeQmake(QTextStream &t)
{
    QString ofile = Option::output.name();
    if(ofile.findRev(Option::dir_sep) != -1)
	ofile = ofile.right(ofile.length() - ofile.findRev(Option::dir_sep) -1);

    QString pfile = project->projectFile();
    QString args;
    if(!project->variables()["QMAKE_ABSOLUTE_SOURCE_PATH"].isEmpty()) {
	args += "QMAKE_ABSOLUTE_SOURCE_PATH=\"" + project->variables()["QMAKE_ABSOLUTE_SOURCE_PATH"].first() + "\"";
	fileAbsolute(pfile);
    }

    QString qmake_path = project->variables()["QMAKE_QMAKE"].first();
    if(qmake_path.isEmpty())
	qmake_path = "qmake"; //hope its in your path
    if(!ofile.isEmpty()) {
	t << ofile << ": " << pfile << " "
	  << project->variables()["QMAKE_INTERNAL_INCLUDED_FILES"].join(" \\\n\t\t") << "\n\t"
	  << qmake_path << " " << args << " " << pfile <<  " -o " << ofile << endl;
    }

    t << "qmake: " << "\n\t"
      << "@" << qmake_path << " " << args << " " << pfile;
    if (!ofile.isEmpty())
	t << " -o " << ofile;
    t << endl << endl;
    
    return TRUE;
}

bool
MakefileGenerator::fileAbsolute(QStringList &files)
{
    if(files.isEmpty())
	return FALSE;
    int ret = 0;
    for(QStringList::Iterator it = files.begin(); it != files.end(); ++it) 
	if(!(*it).isEmpty()) 
	    ret += (int)fileAbsolute(*it);
    return ret != 0;
}

bool
MakefileGenerator::fileAbsolute(QString &file)
{
    if(file.isEmpty())
	return FALSE;
    file = Option::fixPathToTargetOS(file);
    if(!QDir::isRelativePath(file)) //already absolute
	return FALSE;
    QFileInfo fi(file);
    if(fi.convertToAbs()) //strange
	return FALSE;
    file = fi.filePath();

    return TRUE;
}


