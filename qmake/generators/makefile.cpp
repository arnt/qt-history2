/****************************************************************************
** $Id$
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
#include "option.h"
#include <qdir.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <qdict.h>
#if defined(Q_OS_UNIX)
#include <unistd.h>
#else
#include <io.h>
#endif
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

MakefileGenerator::MakefileGenerator(QMakeProject *p) : init_already(FALSE), moc_aware(FALSE), project(p)
{
}

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
 /* qmake ignore Q_OBJECT */
#define COMP_LEN 8 //strlen("Q_OBJECT")
#define OBJ_LEN 8 //strlen("Q_OBJECT")
#define DIS_LEN 10 //strlen("Q_DISPATCH")
    int x;
    for(x = 0; x < (total_size_read-COMP_LEN); x++) {
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
				    ignore_qobject = TRUE;
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
	    } else if(!strncmp(big_buffer+x, "Q_DISPATCH", DIS_LEN)) {
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
		if(!project->isEmpty("MOC_DIR"))
		    mocFile = project->first("MOC_DIR");
		else if(dir_pos != -1)
		    mocFile = fn_target.left(dir_pos+1);

		bool cpp_ext = FALSE;
		for(QStringList::Iterator cppit = Option::cpp_ext.begin(); cppit != Option::cpp_ext.end(); ++cppit) {
		    if((cpp_ext = (fn_target.right(ext_len) == (*cppit))))
			break;
		}
		if(cpp_ext) {
		    mocFile += fn_target.mid(dir_pos+1, ext_pos - dir_pos-1) + Option::moc_ext;
		    findDependencies(fn_target).append(mocFile);
		    project->variables()["_SRCMOC"].append(mocFile);
		} else if(project->variables()["HEADERS"].findIndex(fn_target) != -1) {
		    for(QStringList::Iterator hit = Option::h_ext.begin(); hit != Option::h_ext.end(); ++hit) {
			if((fn_target.right(ext_len) == (*hit))) {
			    mocFile += Option::moc_mod + fn_target.mid(dir_pos+1, ext_pos - dir_pos-1) +
				       Option::cpp_ext.first();
			    logicWarn(mocFile, "SOURCES");
			    project->variables()["_HDRMOC"].append(mocFile);
			    break;
			}
		    }
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
    if ( project->isActiveConfig( "activeqt" ) ) {
	for(x = 0; x < (total_size_read-COMP_LEN); x++) {
#define AQT_LEN 16 //strlen("public QActiveQt")
	    if ( *(big_buffer+x) == 'p' && !strncmp(big_buffer+x, "public QActiveQt", AQT_LEN)) {
		project->variables()["_ACTIVEQT"].append(fn_target);
	    }
	}
    }
#undef OBJ_LEN
#undef DIS_LEN
    return TRUE;
}

bool
MakefileGenerator::generateDependencies(QPtrList<MakefileDependDir> &dirs, QString fn, bool recurse)
{
    fileFixify(fn);
    QStringList &fndeps = findDependencies(fn);
    if(!fndeps.isEmpty())
	return TRUE;

    fn = Option::fixPathToLocalOS(fn, FALSE);

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
    if(fn.right(Option::ui_ext.length()) == Option::ui_ext)
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
		// skip spaces and tabs between the hash and text, so we can handle
		// #  include
		while(x < total_size_read &&
		      (*(big_buffer+x) == ' ' || *(big_buffer+x) == '\t'))
		    x++;
		if(total_size_read >= x + 8 && !strncmp(big_buffer + x, "include ", 8)) {
		    for(x+=8;
			x < total_size_read && (*(big_buffer+x) == ' ' || *(big_buffer+x) == '\t');
			x++);
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
	    // skip whitespaces
	    while(x < total_size_read &&
		  (*(big_buffer+x) == ' ' || *(big_buffer+x) == '\t'))
		x++;
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
	debug_msg(5, "Found dependency from %s to %s", fn.latin1(), inc.latin1());

	if(!inc.isEmpty()) {
	    if(!project->isEmpty("SKIP_DEPENDS")) {
		bool found = FALSE;
		QStringList &nodeplist = project->values("SKIP_DEPENDS");
		for(QStringList::Iterator it = nodeplist.begin();
		    it != nodeplist.end(); ++it) {
		    QRegExp regx((*it));
		    if(regx.match(inc) != -1) {
			found = TRUE;
			break;
		    }
		}
		if(found)
		    continue;
	    }

	    QString fqn;
	    if(project->isEmpty("QMAKE_ABSOLUTE_SOURCE_PATH") && !stat(fndir + inc, &fst)) {
		fqn = fndir + inc;
	    } else {
		if((Option::target_mode == Option::TARG_MAC9_MODE && inc.find(':')) ||
		   (Option::target_mode == Option::TARG_WIN_MODE && inc[1] != ':') ||
		   ((Option::target_mode == Option::TARG_UNIX_MODE || 
		     Option::target_mode == Option::TARG_MACX_MODE) &&
		    inc[0] != '/')) {
		    for(MakefileDependDir *mdd = dirs.first(); mdd; mdd = dirs.next() ) {
			if(QFile::exists(mdd->local_dir + QDir::separator() + inc)) {
			    fqn = mdd->real_dir + QDir::separator() + inc;
			    break;
			}
		    }
		}
	    }
	    if(fqn.isEmpty()) {
		//these are some hacky heuristics it will try to do on an include
		//however these can be turned off at runtime, I'm not sure how
		//reliable these will be, most likely when problems arise turn it off
		//and see if they go away..
		if(Option::mkfile::do_dep_heuristics && depHeuristics.contains(inc)) {
		    fqn = depHeuristics[inc];
		} else if(Option::mkfile::do_dep_heuristics) { //some heuristics..
		    //is it a file from a .ui?
		    int extn = inc.findRev('.');
		    if(extn != -1 && inc.findRev(Option::dir_sep) < extn) {
			QString uip = inc.left(extn) + Option::ui_ext + "$";
			QStringList uil = project->variables()["FORMS"];
			for(QStringList::Iterator it = uil.begin(); it != uil.end(); ++it) {
			    QString s = (*it);
			    if(!project->isEmpty("UI_DIR")) {
				int d = s.findRev(Option::dir_sep) + 1;
				s = project->first("UI_DIR") + s.right(s.length()-d);
			    }
			    if(s.find(QRegExp(uip)) != -1) {
				fqn = s.left(s.length()-3) + inc.right(inc.length()-extn);
				break;
			    }
			}
		    }
		    if(fqn.isEmpty()) { //is it from a .y?
			QString rhs = Option::yacc_mod + Option::h_ext.first();
			if(inc.right(rhs.length()) == rhs) {
			    QString lhs = inc.left(inc.length() - rhs.length()) + Option::yacc_ext;
			    QStringList yl = project->variables()["YACCSOURCES"];
			    for(QStringList::Iterator it = yl.begin(); it != yl.end(); ++it) {
				QString s = (*it), d;
				int slsh = s.findRev(Option::dir_sep);
				if(slsh != -1) {
				    d = s.left(slsh + 1);
				    s = s.right(s.length() - slsh - 1);
				}
				if(!project->isEmpty("QMAKE_ABSOLUTE_SOURCE_PATH"))
				    d = project->first("QMAKE_ABSOLUTE_SOURCE_PATH");
				if(s == lhs) {
				    fqn = d + inc;
				    break;
				}
			    }
			}
		    }
		    depHeuristics.insert(inc, fqn);
		}
		if(!Option::mkfile::do_dep_heuristics || fqn.isEmpty()) //I give up
		    continue;
	    }

	    fqn = Option::fixPathToTargetOS(fqn, FALSE);
	    fileFixify(fqn);
	    debug_msg(4, "Resolved dependancy of %s to %s", inc.latin1(), fqn.latin1());
	    if(fndeps.findIndex(fqn) == -1)
		fndeps.append(fqn);
	}
	//read past new line now..
	for( ; x < total_size_read && (*(big_buffer + x) != '\n'); x++);
    }

    if(recurse) {
	for(QStringList::Iterator fnit = fndeps.begin(); fnit != fndeps.end(); ++fnit) {
	    generateDependencies(dirs, (*fnit), recurse);
	    QStringList &deplist = findDependencies((*fnit));
	    for(QStringList::Iterator it = deplist.begin(); it != deplist.end(); ++it)
		if(fndeps.findIndex((*it)) == -1)
		    fndeps.append((*it));
	}
    }
    debug_msg(2, "Dependancies: %s -> %s", fn.latin1(), fndeps.join(" :: ").latin1());
    return TRUE;
}

void
MakefileGenerator::init()
{
    if(init_already)
	return;
    init_already = TRUE;

    QMap<QString, QStringList> &v = project->variables();
    { //opaths
	if(!v.contains("QMAKE_ABSOLUTE_SOURCE_PATH")) {
	    if(Option::mkfile::do_cache && !Option::mkfile::cachefile.isEmpty() &&
	       v.contains("QMAKE_ABSOLUTE_SOURCE_ROOT")) {
		QFileInfo fi(Option::mkfile::cachefile);
		if(!fi.convertToAbs()) {
		    QString cache_r = fi.dirPath(), pwd = Option::output_dir;
		    if ( pwd.left(cache_r.length()) == cache_r ) {
			QString root = v["QMAKE_ABSOLUTE_SOURCE_ROOT"].first();
			root = Option::fixPathToTargetOS( root );
			if(!root.isEmpty()) {
			    pwd = Option::fixPathToTargetOS(root + pwd.mid(cache_r.length()));
			    if(QFile::exists(pwd))
				v.insert("QMAKE_ABSOLUTE_SOURCE_PATH", pwd);
			}
		    }
		}
	    }
	}
	if ( !v["QMAKE_ABSOLUTE_SOURCE_PATH"].isEmpty() ) {
	    QString &asp = v["QMAKE_ABSOLUTE_SOURCE_PATH"].first();
	    asp = Option::fixPathToTargetOS( asp );
	    if(asp.isEmpty() || asp == Option::output_dir) //if they're the same, why bother?
		v["QMAKE_ABSOLUTE_SOURCE_PATH"].clear();
	}
	QString currentDir = QDir::currentDirPath();
	QString dirs[] = { QString("OBJECTS_DIR"), QString("MOC_DIR"), QString("UI_DIR"), QString("DESTDIR"), QString("SUBLIBS_DIR"), QString::null };
	for(int x = 0; dirs[x] != QString::null; x++) {
	    if ( !v[dirs[x]].isEmpty() ) {
		{
		    QString &path = v[dirs[x]].first();
		    fixEnvVariables(path);
		    if(QDir::isRelativePath(path) && !v["QMAKE_ABSOLUTE_SOURCE_PATH"].isEmpty())
			path.prepend(Option::output_dir + Option::dir_sep);
		    fileFixify(path);
		    if(path.right(Option::dir_sep.length()) != Option::dir_sep)
			path += Option::dir_sep;
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

    QString paths[] = { QString("SOURCES"), QString("FORMS"), QString("YACCSOURCES"), QString("INCLUDEPATH"),
			    QString("HEADERS"), QString("HEADERS_ORIG"),
			    QString("LEXSOURCES"), QString("QMAKE_INTERNAL_INCLUDED_FILES"), QString::null };
    for(int y = 0; paths[y] != QString::null; y++) {
	QStringList &l = v[paths[y]];
	if(!l.isEmpty())
	    fileFixify(l);
    }

    /* get deps and mocables */
    QDict<void> cache_found_files;
    QString cache_file(Option::output_dir + QDir::separator() + ".qmake.internal.cache");
    if(Option::qmake_mode == Option::QMAKE_GENERATE_PROJECT ||
       Option::mkfile::do_deps || Option::mkfile::do_mocs) {
	QPtrList<MakefileDependDir> deplist;
	deplist.setAutoDelete(TRUE);
	if((Option::qmake_mode == Option::QMAKE_GENERATE_PROJECT || Option::mkfile::do_deps) &&
	   doDepends()) {
	    QStringList incDirs = v["DEPENDPATH"] + v["QMAKE_ABSOLUTE_SOURCE_PATH"];
	    if(project->isActiveConfig("depend_includepath"))
		incDirs += v["INCLUDEPATH"];
	    for(QStringList::Iterator it = incDirs.begin(); it != incDirs.end(); ++it) {
		QString r = (*it), l = Option::fixPathToLocalOS((*it));
		deplist.append(new MakefileDependDir(r.replace(QRegExp("\""),""),
						     l.replace(QRegExp("\""),"")));
	    }
	    debug_msg(1, "Dependancy Directories: %s", incDirs.join(" :: ").latin1());
	    if(Option::output.name() != "-" && project->isActiveConfig("qmake_cache")) {
		QFile cachef(cache_file);
		if(cachef.open(IO_ReadOnly | IO_Translate)) {
		    QFileInfo cachefi(cache_file);
		    debug_msg(2, "Trying internal cache information: %s", cache_file.latin1());
		    QTextStream cachet(&cachef);
		    QString line, file;
		    enum { CacheInfo, CacheDepend, CacheMoc } state = CacheInfo;
		    while (!cachet.eof()) {
			line = cachet.readLine().stripWhiteSpace();
			int sep = line.find('=');
			if(line == "[depend]") {
			    state = CacheDepend;
			} else if(line == "[mocable]") {
			    state = CacheMoc;
			} else if(line == "[check]") {
			    state = CacheInfo;
			} else if(!line.isEmpty() && sep != -1) {
			    file = line.left(sep).stripWhiteSpace();
			    line = line.right(line.length() - sep - 1).stripWhiteSpace();
			    fileFixify(file);
			    if(state == CacheInfo) {
				if(file == "QMAKE_CACHE_VERSION") {
				    if(line != qmake_version()) 
					break;
				} else {
				    const QStringList &l = project->variables()[file];
				    if(!l.isEmpty() && !line.isEmpty() && l.join(" ") != line) 
					break;
				}
			    } else if(state == CacheDepend) {
				bool found = (bool)cache_found_files[file];
				QStringList files = QStringList::split(" ", line);
				if(!found) {
				    QFileInfo fi(file);
				    if(fi.exists() && fi.lastModified() < cachefi.lastModified()) {
					cache_found_files.insert(file, (void *)1);
					found = TRUE;
				    } 
				}
				if(found) {
				    for(QStringList::Iterator dep_it = files.begin(); 
					dep_it != files.end(); ++dep_it) {
					if(!cache_found_files[(*dep_it)]) {
					    QFileInfo fi((*dep_it));
					    if(fi.exists() && 
					       fi.lastModified() < cachefi.lastModified()) {
						cache_found_files.insert((*dep_it), (void *)1);
					    } else {
						found = FALSE;
						break;
					    }
					}
				    }
				    if(found) {
					debug_msg(2, "Dependancies (cached): %s -> %s", file.latin1(), 
						  files.join(" :: ").latin1());				
					findDependencies(file) = files;
				    }
				}
			    } else {
				void *found = cache_found_files[file];
				if(found != (void *)2) {
				    if(found) {
					cache_found_files.replace(file, (void *)2);
				    } else {
					QFileInfo fi(file);
					if(fi.exists() && fi.lastModified() < cachefi.lastModified()) {
					    cache_found_files.insert(file, (void *)2);
					    found = (void*)1;
					} 
				    }
				}
				if(found && line != "*qmake_ignore*") {
				    int ext_len = file.length() - file.findRev('.');
				    bool cpp_ext = FALSE;
				    for(QStringList::Iterator cppit = Option::cpp_ext.begin(); 
					cppit != Option::cpp_ext.end(); ++cppit) {
					if((cpp_ext = (file.right(ext_len) == (*cppit))))
					    break;
				    }
				    if(cpp_ext) {
					project->variables()["_SRCMOC"].append(line);
				    } else if(project->variables()["HEADERS"].findIndex(file) != -1) {
					for(QStringList::Iterator hit = Option::h_ext.begin(); 
					    hit != Option::h_ext.end(); ++hit) {
					    if((file.right(ext_len) == (*hit))) {
						project->variables()["_HDRMOC"].append(line);
						break;
					    }
					}
				    }
				    debug_msg(2, "Mocgen (cached): %s -> %s", file.latin1(), 
					      line.latin1());
				    mocablesToMOC[file] = line;
				    mocablesFromMOC[line] = file;
				}
			    }
			}
		    }
		    cachef.close();
		}
	    }
	}
	QString sources[] = { QString("OBJECTS"), QString("LEXSOURCES"), QString("YACCSOURCES"),
				  QString("HEADERS"), QString("SOURCES"), QString("FORMS"),
			      QString::null };
	depHeuristics.clear();
	bool write_cache = FALSE, read_cache = QFile::exists(cache_file);
	for(int x = 0; sources[x] != QString::null; x++) {
	    QStringList vpath, &l = v[sources[x]];
	    for(QStringList::Iterator val_it = l.begin(); val_it != l.end(); ++val_it) {
		if(!(*val_it).isEmpty()) {
		    QString file = Option::fixPathToLocalOS((*val_it));
		    if(!QFile::exists(file)) {
			bool found = FALSE;
			if(QDir::isRelativePath(file)) {
			    if(vpath.isEmpty())
				vpath = v["VPATH_" + sources[x]] + v["VPATH"] +
					v["QMAKE_ABSOLUTE_SOURCE_PATH"] + v["DEPENDPATH"];

			    for(QStringList::Iterator vpath_it = vpath.begin();
				vpath_it != vpath.end(); ++vpath_it) {
				QString real_dir = Option::fixPathToLocalOS((*vpath_it));
				if(QFile::exists(real_dir + QDir::separator() + (*val_it))) {
				    QString dir = (*vpath_it);
				    if(dir.right(Option::dir_sep.length()) != Option::dir_sep)
					dir += Option::dir_sep;
				    (*val_it).prepend(dir);
				    fileFixify((*val_it));
				    found = TRUE;
				    debug_msg(1, "Found file through vpath %s -> %s",
					      file.latin1(), (*val_it).latin1());
				    break;
				}
			    }
			}
			if(!found) {
			    QString dir, regex = (*val_it), real_dir;
			    if(regex.findRev(Option::dir_sep) != -1) {
				dir = regex.left(regex.findRev(Option::dir_sep) + 1);
				real_dir = Option::fixPathToLocalOS(dir);
				regex = regex.right(regex.length() - dir.length());
			    }
			    if(real_dir.isEmpty() || QFile::exists(real_dir)) {
				QDir d(real_dir, regex);
				if(!d.count()) {
				    debug_msg(1, "Failure to find %s in vpath (%s)",
					      (*val_it).latin1(), vpath.join("::").latin1());
				    warn_msg(WarnLogic, "Failure to find: %s", (*val_it).latin1());
				    continue;
				} else {
				    (*val_it) = dir + d[0];
				    for(int i = 1; i < (int)d.count(); i++)
					l.insert(val_it, dir + d[i]);
				}
			    } else {
				debug_msg(1, "Cannot match %s%c%s, as %s does not exist.",
					  real_dir.latin1(), QDir::separator(), regex.latin1(),
					  real_dir.latin1());
				warn_msg(WarnLogic, "Failure to find: %s", (*val_it).latin1());
			    }
			}
		    }

		    QString val_file = (*val_it);
		    fileFixify(val_file);
		    bool found_cache_moc = FALSE, found_cache_dep = FALSE;
		    if(read_cache && Option::output.name() != "-" && 
		       project->isActiveConfig("qmake_cache")) {
			if(!findDependencies(val_file).isEmpty()) 
			    found_cache_dep = TRUE;
			if(cache_found_files[(*val_it)] == (void *)2) 
			    found_cache_moc = TRUE;
			if(!found_cache_moc || !found_cache_dep) 
			    write_cache = TRUE;
		    }
		    if(!found_cache_dep && sources[x] != "OBJECTS") {
			debug_msg(5, "Looking for dependancies for %s", (*val_it).latin1());
			generateDependencies(deplist, (*val_it), doDepends());
		    }
		    if(found_cache_moc) {
			QString moc = findMocDestination(val_file);
			if(!moc.isEmpty()) {
			    for(QStringList::Iterator cppit = Option::cpp_ext.begin(); 
				cppit != Option::cpp_ext.end(); ++cppit) {
				if(val_file.right((*cppit).length()) == (*cppit)) {
				    QStringList &deps = findDependencies(val_file);
				    if(!deps.contains(moc)) 
					deps.append(moc);
				    break;
				}
			    }
			}
		    } else if(mocAware() && (sources[x] == "SOURCES" || sources[x] == "HEADERS") && 
			      (Option::qmake_mode == Option::QMAKE_GENERATE_PROJECT || 
			       Option::mkfile::do_mocs)) {
			generateMocList((*val_it));
		    }
		}
	    }
	}
	if(write_cache || !read_cache) {
	    QFile cachef(cache_file);
	    if(cachef.open(IO_WriteOnly | IO_Translate)) {
		debug_msg(2, "Writing internal cache information: %s", cache_file.latin1());
		QTextStream cachet(&cachef);
		cachet << "[check]" << "\n"
		       << "QMAKE_CACHE_VERSION = " << qmake_version() << "\n"
		       << "QMAKE_ABSOLUTE_SOURCE_PATH = " << var("QMAKE_ABSOLUTE_SOURCE_PATH") << "\n"
		       << "MOC_DIR = " << var("MOC_DIR") << "\n"
		       << "UI_DIR = " <<  var("UI_DIR") << "\n";
		cachet << "[depend]" << endl;
		for(QMap<QString, QStringList>::Iterator it = depends.begin(); 
		    it != depends.end(); ++it) 
		    cachet << depKeyMap[it.key()] << " = " << it.data().join(" ") << endl;
		cachet << "[mocable]" << endl;
		QString mc, moc_sources[] = { QString("HEADERS"), QString("SOURCES"), QString::null };
		for(int x = 0; moc_sources[x] != QString::null; x++) {
		    QStringList &l = v[moc_sources[x]];
		    for(QStringList::Iterator val_it = l.begin(); val_it != l.end(); ++val_it) {
			if(!(*val_it).isEmpty()) {
			    mc = mocablesToMOC[(*val_it)];
			    if(mc.isEmpty())
				mc = "*qmake_ignore*";
			    cachet << (*val_it) << " = " << mc << endl;
			}
		    }
		}
		cachef.close();
	    }
	}
    }
    v["OBJECTS"] = createObjectList("SOURCES") + v["OBJECTS"]; // init variables

    //lex files
    {
	QStringList &impls = v["LEXIMPLS"];
	QStringList &l = v["LEXSOURCES"];
	for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
	    QFileInfo fi((*it));
	    QString impl = fi.dirPath() + Option::dir_sep + fi.baseName() + Option::lex_mod + Option::cpp_ext.first();
	    logicWarn(impl, "SOURCES");
	    impls.append(impl);
	    if( ! project->isActiveConfig("lex_included")) {
		v["SOURCES"].append(impl);
		// attribute deps of lex file to impl file
		QStringList &lexdeps = findDependencies((*it));
		QStringList &impldeps = findDependencies(impl);
		for(QStringList::ConstIterator d = lexdeps.begin(); d != lexdeps.end(); ++d) {
		    if(!impldeps.contains(*d))
			impldeps.append(*d);
		}
		lexdeps.clear();
	    }
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
	    QString impl = fi.dirPath() + Option::dir_sep + fi.baseName() + 
			   Option::yacc_mod + Option::cpp_ext.first();
	    logicWarn(impl, "SOURCES");
	    QString decl = fi.dirPath() + Option::dir_sep + fi.baseName() + 
			   Option::yacc_mod + Option::h_ext.first();
	    logicWarn(decl, "HEADERS");

	    decls.append(decl);
	    impls.append(impl);
	    v["SOURCES"].append(impl);
	    QStringList &impldeps = findDependencies(impl);
 	    impldeps.append(decl);
	    // attribute deps of yacc file to impl file
	    QStringList &yaccdeps = findDependencies((*it));
	    for(QStringList::ConstIterator d = yaccdeps.begin(); d != yaccdeps.end(); ++d) {
		if(!impldeps.contains(*d))
		    impldeps.append(*d);
	    }
	    if( project->isActiveConfig("lex_included")) {
		// is there a matching lex file ? Transfer its dependencies.
		QString lexsrc = fi.baseName() + Option::lex_ext;
		if(fi.dirPath() != ".")
		    lexsrc.prepend(fi.dirPath() + Option::dir_sep);
		if(v["LEXSOURCES"].findIndex(lexsrc) != -1) {
		    QString trg = fi.dirPath() + Option::dir_sep + fi.baseName() + 
				  Option::lex_mod + Option::cpp_ext.first();
		    impldeps.append(trg);
		    impldeps += findDependencies(lexsrc);
		    depends[lexsrc].clear();
		}
	    }
	    yaccdeps.clear();
	}
	v["OBJECTS"] += (v["YACCOBJECTS"] = createObjectList("YACCIMPLS"));
    }

    //UI files
    {
	if(!project->isEmpty("UI_DIR"))
	    project->variables()["INCLUDEPATH"].append(project->first("UI_DIR"));
	QStringList &decls = v["UICDECLS"], &impls = v["UICIMPLS"];
	QStringList &l = v["FORMS"];
	for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
	    QFileInfo fi(Option::fixPathToLocalOS((*it)));
	    QString impl, decl;
	    if ( !project->isEmpty("UI_DIR") ) {
		impl = project->first("UI_DIR") + fi.baseName() + Option::cpp_ext.first();
		decl = project->first("UI_DIR") + fi.baseName() + Option::h_ext.first();

		QString d = fi.dirPath();
		if( d == ".")
		    d = QDir::currentDirPath();
		fileFixify(d);
		if( !project->variables()["INCLUDEPATH"].contains(d))
		    project->variables()["INCLUDEPATH"].append(d);
	    } else if ( fi.dirPath() == "." ) {
	    	impl = fi.baseName() + Option::cpp_ext.first();
                decl = fi.baseName() + Option::h_ext.first();
	    } else {
	    	impl = fi.dirPath() + Option::dir_sep + fi.baseName() + Option::cpp_ext.first();
                decl = fi.dirPath() + Option::dir_sep + fi.baseName() + Option::h_ext.first();
	    }
	    logicWarn(impl, "SOURCES");
	    logicWarn(decl, "HEADERS");
	    decls.append(decl);
	    impls.append(impl);
	    findDependencies(impl).append(decl);

	    QString mocable = Option::moc_mod + fi.baseName() + Option::cpp_ext.first();
	    if(!v["MOC_DIR"].isEmpty())
		mocable.prepend(v["MOC_DIR"].first());
	    else if(fi.dirPath() != ".")
		mocable.prepend(fi.dirPath() + Option::dir_sep);
	    logicWarn(mocable, "SOURCES");
	    mocablesToMOC[cleanFilePath(decl)] = mocable;
	    mocablesFromMOC[cleanFilePath(mocable)] = decl;
	    v["_UIMOC"].append(mocable);
	}
	v["OBJECTS"] += (v["UICOBJECTS"] = createObjectList("UICDECLS"));
    }

    //Image files
    if(!project->isEmpty("IMAGES")) {
	if(project->isEmpty("QMAKE_IMAGE_COLLECTION"))
	    v["QMAKE_IMAGE_COLLECTION"].append("qmake_image_collection" + Option::cpp_ext.first());
	QString imgfile = project->first("QMAKE_IMAGE_COLLECTION");
	Option::fixPathToTargetOS(imgfile);
	if(!project->isEmpty("UI_DIR")) {
	    if(imgfile.find(Option::dir_sep) != -1)
		imgfile = imgfile.right(imgfile.findRev(Option::dir_sep) + 1);
	    imgfile.prepend(project->first("UI_DIR"));
	    v["QMAKE_IMAGE_COLLECTION"] = QStringList(imgfile);
	}
	logicWarn(imgfile, "SOURCES");
	QStringList &l = v["IMAGES"];
	for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
	    QString f = (*it);
	    fileFixify(f);
	    if(!QFile::exists(f)) {
		warn_msg(WarnLogic, "Failure to open: %s", f.latin1());
		continue;
	    }
	    findDependencies(imgfile).append(f);
	}
	v["OBJECTS"] += (v["IMAGEOBJECTS"] = createObjectList("QMAKE_IMAGE_COLLECTION"));
    }

    //moc files
    if ( mocAware() ) {
	if(!project->isEmpty("MOC_DIR"))
	    project->variables()["INCLUDEPATH"].append(project->first("MOC_DIR"));
	v["OBJMOC"] = createObjectList("_HDRMOC") + createObjectList("_UIMOC");

	QStringList &l = v["SRCMOC"];
	l = v["_HDRMOC"] + v["_SRCMOC"] + v["_UIMOC"];
	for(QStringList::Iterator val_it = l.begin(); val_it != l.end(); ++val_it)
	    if(!(*val_it).isEmpty())
		(*val_it) = Option::fixPathToTargetOS((*val_it), FALSE);
    }
}

bool
MakefileGenerator::processPrlFile(QString &file)
{
    bool ret = FALSE, try_replace_file=FALSE;
    QString prl_file;
    if(file.right(Option::prl_ext.length()) == Option::prl_ext) {
	try_replace_file = TRUE;
	prl_file = file;
	file = "";
    } else {
	QString tmp = file;
	int ext = tmp.findRev('.');
	if(ext != -1)
	    tmp = tmp.left(ext);
	prl_file = tmp + Option::prl_ext;
    }
    fileFixify(prl_file);
    if(!QFile::exists(prl_file) && project->isActiveConfig("qt")) {
	QString stem = prl_file, dir, extn;
	int slsh = stem.findRev('/'), hadlib = 0;
	if(slsh != -1) {
	    dir = stem.left(slsh + 1);
	    stem = stem.right(stem.length() - slsh - 1);
	}
	if(stem.left(3) == "lib") {
	    hadlib = 1;
	    stem = stem.right(stem.length() - 3);
	}
	int dot = stem.find('.');
	if(dot != -1) {
	    extn = stem.right(stem.length() - dot);
	    stem = stem.left(dot);
	}
	if(stem == "qt" || stem == "qte" || stem == "qte-mt" || stem == "qt-mt") {
	    if(stem.right(3) == "-mt")
		stem = stem.left(stem.length() - 3); //lose the -mt
	    else
		stem += "-mt"; //try the thread case
	    prl_file = dir;
	    if(hadlib)
		prl_file += "lib";
	    prl_file += stem + extn;
	    try_replace_file = TRUE;
	}
    }
    QString real_prl_file = Option::fixPathToLocalOS(prl_file);
    if(!real_prl_file.isEmpty() && QFile::exists(real_prl_file) &&
       project->variables()["QMAKE_PRL_INTERNAL_FILES"].findIndex(prl_file) == -1) {
	project->variables()["QMAKE_PRL_INTERNAL_FILES"].append(prl_file);
	QMakeProject proj;
	debug_msg(1, "Processing PRL file: %s", real_prl_file.latin1());
	if(!proj.read(real_prl_file, QDir::currentDirPath())) {
	    fprintf(stderr, "Error processing prl file: %s\n", real_prl_file.latin1());
	} else {
	    ret = TRUE;
	    QMap<QString, QStringList> &vars = proj.variables();
	    for( QMap<QString, QStringList>::Iterator it = vars.begin(); it != vars.end(); ++it)
		processPrlVariable(it.key(), it.data());
	    if(try_replace_file && !proj.isEmpty("QMAKE_PRL_TARGET")) {
		QString dir;
		int slsh = real_prl_file.findRev(Option::dir_sep);
		if(slsh != -1)
		    dir = real_prl_file.left(slsh+1);
		file = dir + proj.first("QMAKE_PRL_TARGET");
	    }
	}
	if(ret)
	    project->variables()["QMAKE_INTERNAL_INCLUDED_FILES"].append(prl_file);
    }
    return ret;
}

void
MakefileGenerator::processPrlVariable(const QString &var, const QStringList &l)
{
    if(var == "QMAKE_PRL_LIBS") {
	QString where = "QMAKE_LIBS";
	if(!project->isEmpty("QMAKE_INTERNAL_PRL_LIBS"))
	    where = project->first("QMAKE_INTERNAL_PRL_LIBS");
	QStringList &out = project->variables()[where];
	for(QStringList::ConstIterator it = l.begin(); it != l.end(); ++it) {
	    if( out.findIndex((*it)) == -1)
		out.append((*it));
	}
    } else if(var == "QMAKE_PRL_DEFINES") {
	QStringList &out = project->variables()["DEFINES"];
	for(QStringList::ConstIterator it = l.begin(); it != l.end(); ++it) {
	    if(out.findIndex((*it)) == -1 &&
	       project->variables()["PRL_EXPORT_DEFINES"].findIndex((*it)) == -1)
		out.append((*it));
	}
    }
}

void
MakefileGenerator::processPrlFiles()
{
    for(bool ret = FALSE; TRUE; ret = FALSE) {
	//read in any prl files included..
	QStringList l_out;
	QString where = "QMAKE_LIBS";
	if(!project->isEmpty("QMAKE_INTERNAL_PRL_LIBS"))
	    where = project->first("QMAKE_INTERNAL_PRL_LIBS");
	QStringList &l = project->variables()[where];
	for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
	    QString file = (*it);
	    if(processPrlFile(file))
		ret = TRUE;
	    if(!file.isEmpty())
		l_out.append(file);
	}
	if(ret)
	    l = l_out;
	else
	    break;
    }
}

void
MakefileGenerator::writePrlFile(QTextStream &t)
{
    QString target = project->first("TARGET");
    int slsh = target.findRev(Option::dir_sep);
    if(slsh != -1)
	target = target.right(target.length() - slsh - 1);
    QString bdir = Option::output_dir;
    if(bdir.isEmpty())
	bdir = QDir::currentDirPath();
    t << "QMAKE_PRL_BUILD_DIR = " << bdir << endl;
    if(!project->isEmpty("QMAKE_ABSOLUTE_SOURCE_PATH"))
	t << "QMAKE_PRL_SOURCE_DIR = " << project->first("QMAKE_ABSOLUTE_SOURCE_PATH") << endl;
    t << "QMAKE_PRL_TARGET = " << target << endl;
    if(!project->isEmpty("PRL_EXPORT_DEFINES"))
	t << "QMAKE_PRL_DEFINES = "
	  << project->variables()["PRL_EXPORT_DEFINES"].join(" ") << endl;
    if(project->isActiveConfig("staticlib") || project->isActiveConfig("explicitlib")) {
	QStringList libs;
	if(!project->isEmpty("QMAKE_INTERNAL_PRL_LIBS"))
	    libs = project->variables()["QMAKE_INTERNAL_PRL_LIBS"];
	else
	    libs << "QMAKE_LIBS"; //obvious one
	t << "QMAKE_PRL_LIBS = ";
	for(QStringList::Iterator it = libs.begin(); it != libs.end(); ++it)
	    t << project->variables()[(*it)].join(" ") << " ";
	t << endl;
    }
}

bool
MakefileGenerator::write()
{
    init();
    findLibraries();
    if((Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE || //write prl
       Option::qmake_mode == Option::QMAKE_GENERATE_PRL) &&
       project->isActiveConfig("create_prl") && project->first("TEMPLATE") == "lib" &&
       !project->isActiveConfig("plugin")) {
	QString prl = var("TARGET");
	int slsh = prl.findRev(Option::dir_sep);
	if(slsh != -1)
	    prl = prl.right(prl.length() - slsh);
	int dot = prl.find('.');
	if(dot != -1)
	    prl = prl.left(dot);
	prl += Option::prl_ext;
	if(!project->isEmpty("DESTDIR"))
	    prl.prepend(var("DESTDIR"));
	fileFixify(prl);
	fixEnvVariables(prl);
	QFile ft(prl);
	if(ft.open(IO_WriteOnly)) {
	    project->variables()["ALL_DEPS"].append(prl);
	    project->variables()["QMAKE_INTERNAL_PRL_FILE"].append(prl);
	    QTextStream t(&ft);
	    writePrlFile(t);
	    ft.close();
	}
    }
    if(Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE &&
       project->isActiveConfig("link_prl")) //load up prl's
	processPrlFiles();

    if(Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE ||
       Option::qmake_mode == Option::QMAKE_GENERATE_PROJECT) {
	QTextStream t(&Option::output);
	writeMakefile(t);
    }
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

	if(!doDepends()) {
	    QString sdep, odep = (*sit) + " ";
	    QStringList deps = findDependencies((*sit));
	    for(QStringList::Iterator dit = deps.begin(); dit != deps.end(); dit++) {
		if((*dit).right(Option::moc_ext.length()) == Option::moc_ext)
		    odep += (*dit) + " ";
		else
		    sdep += (*dit) + " ";
	    }
	    t << (*sit) << ": " << sdep << endl
	      << (*oit) << ": " << odep ;
	} else {
	    t << (*oit) << ": " << (*sit) << " " << findDependencies((*sit)).join(" \\\n\t\t");
	}

	QString comp, cimp;
	for(QStringList::Iterator cppit = Option::cpp_ext.begin(); cppit != Option::cpp_ext.end(); ++cppit) {
	    if((*sit).right((*cppit).length()) == (*cppit)) {
		comp = "QMAKE_RUN_CXX";
		cimp = "QMAKE_RUN_CXX_IMP";
		break;
	    }
	}
	if(comp.isEmpty()) {
	    comp = "QMAKE_RUN_CC";
	    cimp = "QMAKE_RUN_CC_IMP";
	}
	if ( !project->isEmpty("OBJECTS_DIR") || project->variables()[cimp].isEmpty()) {
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
	QString deps = findDependencies((*it)).join(" \\\n\t\t"), decl, impl;
	{
	    QString tmp = (*it);
	    decl = tmp.replace(QRegExp("\\" + Option::ui_ext + "$"), Option::h_ext.first());
	    tmp = (*it);
	    impl = tmp.replace(QRegExp("\\" + Option::ui_ext + "$"), Option::cpp_ext.first());
	    if(!project->isEmpty("UI_DIR")) {
		int dlen = (*it).findRev(Option::dir_sep) + 1;
		decl = project->first("UI_DIR") + decl.right(decl.length() - dlen);
		impl = project->first("UI_DIR") + impl.right(impl.length() - dlen);
	    }
	}
	t << decl << ": " << (*it) << " " << deps << "\n\t"
	  << "$(UIC) " << (*it) << " -o " << decl << endl << endl;

	QString mildDecl = decl;
	int k = mildDecl.findRev( Option::dir_sep );
	if ( k != -1 )
	    mildDecl = mildDecl.mid( k + 1 );

	t << impl << ": " << decl << " " << (*it) << " " << deps << "\n\t"
	  << "$(UIC) " << (*it) << " -i " << mildDecl << " -o " << impl << endl << endl;
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
    if(!project->isEmpty("MOC_DIR"))
	mocdir = project->first("MOC_DIR");

    for( ;oit != objl.end(); oit++) {
	QFileInfo fi(Option::fixPathToLocalOS((*oit)));
	QString dirName;
	if( !mocdir.isEmpty() )
	    dirName = mocdir;
	else if(!fi.dirPath().isEmpty() && fi.dirPath() != "." && project->isEmpty("OBJECTS_DIR"))
	    dirName = Option::fixPathToTargetOS(fi.dirPath(), FALSE) + Option::dir_sep;
	QString src(dirName + fi.baseName() + Option::cpp_ext.first() );

	QString hdr = findMocSource(src);
	t << (*oit) << ": " << src << " "
	  << hdr << " " << findDependencies(hdr).join(" \\\n\t\t");
	if ( !project->isEmpty("OBJECTS_DIR") || !project->isEmpty("MOC_DIR") ||
	     project->isEmpty("QMAKE_RUN_CXX_IMP")) {
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
	QString m = Option::fixPathToTargetOS(findMocDestination(*it));
	if ( !m.isEmpty())
	    t << m << ": $(MOC) " << (*it) << "\n\t"
	      << "$(MOC) " << (*it) << " -o " << m << endl << endl;
    }
}

void
MakefileGenerator::writeYaccSrc(QTextStream &t, const QString &src)
{
    QStringList &l = project->variables()[src];
    if(project->isActiveConfig("yacc_no_name_mangle") && l.count() > 1)
	warn_msg(WarnLogic, "yacc_no_name_mangle specified, but multiple parsers expected."
		 "This can lead to link problems.\n");
    for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
	QFileInfo fi((*it));
	QString impl = fi.dirPath() + Option::dir_sep + fi.baseName() + Option::yacc_mod + Option::cpp_ext.first();
	QString decl = fi.dirPath() + Option::dir_sep + fi.baseName() + Option::yacc_mod + Option::h_ext.first();

	QString yaccflags = "$(YACCFLAGS)";
	if(!project->isActiveConfig("yacc_no_name_mangle"))
	    yaccflags += " -p " + fi.baseName();
	t << impl << ": " << (*it) << "\n\t"
	  << ( "$(YACC) " + yaccflags + " " ) << (*it) << "\n\t"
	  << "-$(DEL_FILE) " << impl << " " << decl << "\n\t"
	  << "-$(MOVE) y.tab.h " << decl << "\n\t"
	  << "-$(MOVE) y.tab.c " << impl << endl << endl;
	t << decl << ": " << impl << endl << endl;
    }
}

void
MakefileGenerator::writeLexSrc(QTextStream &t, const QString &src)
{
    QStringList &l = project->variables()[src];
    if(project->isActiveConfig("yacc_no_name_mangle") && l.count() > 1)
	warn_msg(WarnLogic, "yacc_no_name_mangle specified, but multiple parsers expected.\n"
		 "This can lead to link problems.\n");
    for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
	QFileInfo fi((*it));
	QString impl = fi.dirPath() + Option::dir_sep + fi.baseName() + Option::lex_mod + Option::cpp_ext.first();

	QString lexflags = "$(LEXFLAGS)", stub="yy";
	if(!project->isActiveConfig("yacc_no_name_mangle")) {
	    stub = fi.baseName();
	    lexflags += " -P" + stub;
	}
	t << impl << ": " << (*it) << " " << findDependencies((*it)).join(" \\\n\t\t") << "\n\t"
	  << ( "$(LEX) " + lexflags + " " ) << (*it) << "\n\t"
	  << "-$(DEL_FILE) " << impl << " " << "\n\t"
	  << "-$(MOVE) lex." << stub << ".c " << impl << endl << endl;
    }
}

void
MakefileGenerator::writeImageObj(QTextStream &t, const QString &obj)
{
    QStringList &objl = project->variables()[obj];
    QRegExp regexpSrc("\\$src");
    QRegExp regexpObj("\\$obj");

    QString uidir;
    for(QStringList::Iterator oit = objl.begin(); oit != objl.end(); oit++) {
        QString src(project->first("QMAKE_IMAGE_COLLECTION"));
	t << (*oit) << ": " << src;
	if ( !project->isEmpty("OBJECTS_DIR") || !project->isEmpty("UI_DIR") ||
	     project->isEmpty("QMAKE_RUN_CXX_IMP")) {
	    QString p = var("QMAKE_RUN_CXX");
	    p.replace( regexpSrc, src);
	    p.replace( regexpObj, (*oit));
	    t << "\n\t" << p;
	}
	t << endl << endl;
    }
}


void
MakefileGenerator::writeImageSrc(QTextStream &t, const QString &src)
{
    QStringList &l = project->variables()[src];
    for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
	t << (*it) << ": " << findDependencies((*it)).join(" \\\n\t\t") << "\n\t"
	  << "$(UIC) " << " -embed " << project->first("QMAKE_ORIG_TARGET")
	  << " " << findDependencies((*it)).join(" ") << " -o " << (*it) << endl << endl;
    }
}

void
MakefileGenerator::writeIdlSrc(QTextStream &t, const QString &src)
{
    QStringList &l = project->variables()[src];
    QString input = project->variables()["_ACTIVEQT"].first();
    for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
	QString file = *it;
	if ( file.right( 4 ) != ".res" )
	    continue;
	QString idlfile = file.left( file.length()-3) += "idl";
	QString rcfile = file.left( file.length()-3) += "rc";
	t << file << ": " << findDependencies(file).join(" \\\n\t\t") << "\n\t"
	    << "$(IDC) " << input << " -o " << idlfile << " -rc " << rcfile << "\n\t"
	    << "midl " << idlfile << " /tlb " << file.left( file.length()-3) << "tlb"
	    << " /iid tmp\\iid_i.c /dlldata tmp\\dlldata.c /cstub tmp\\cstub.c /header tmp\\cstub.h /proxy tmp\\proxy.c /sstub tmp\\sstub.c\n\t"
	    << var("QMAKE_RC") << " " << rcfile << endl << endl;
	break;
    }
}

void
MakefileGenerator::writeInstalls(QTextStream &t, const QString &installs)
{
    QString all_installs, all_uninstalls;
    QStringList &l = project->variables()[installs];
    for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
	QString pvar = (*it) + ".path";
	if(project->variables()[pvar].isEmpty()) {
	    warn_msg(WarnLogic, "%s is not defined: install target not created\n", pvar.latin1());
	    continue;
	}

	bool do_default = TRUE;
	QString target, dst=project->variables()[pvar].first();
	if(dst.right(1) != Option::dir_sep)
	    dst += Option::dir_sep;
 	QStringList tmp, &uninst = project->variables()[(*it) + ".uninstall"];
 	//other
 	tmp = project->variables()[(*it) + ".extra"];
 	if(!tmp.isEmpty()) {
	    do_default = FALSE;
	    if(!target.isEmpty())
 		target += "\n\t";
 	    target += tmp.join(" ");
 	}
 	//masks
 	tmp = project->variables()[(*it) + ".files"];
 	if(!tmp.isEmpty()) {
	    if(!target.isEmpty())
 		target += "\n\t";
 	    if(Option::target_mode == Option::TARG_WIN_MODE || Option::target_mode == Option::TARG_MAC9_MODE) {
 	    } else if(Option::target_mode == Option::TARG_UNIX_MODE || Option::target_mode == Option::TARG_MACX_MODE) {
		do_default = FALSE;
		for(QStringList::Iterator wild_it = tmp.begin(); wild_it != tmp.end(); ++wild_it) {
		    QString wild = Option::fixPathToLocalOS((*wild_it)), wild_var = wild;
		    fileFixify(wild);
		    fileFixify(wild_var);
		    if(QFile::exists(wild)) { //real file
			QString file = wild;
			QFileInfo fi(file);
			target += QString("-") + (fi.isDir() ? "$(COPY_DIR)" : "$(COPY_FILE)") +
				  " \"" + fi.filePath() + "\" \"" + dst + "\"\n\t";
			if(fi.isExecutable() && !project->isEmpty("QMAKE_STRIP"))
			    target += var("QMAKE_STRIP") + " \"" + dst + "\"\n\t";
			uninst.append(QString("-$(DEL_FILE) -r") + " \"" + dst + "\"");
			continue;
		    }
		    QString dirstr = QDir::currentDirPath(), f = wild; 		    //wild
		    int slsh = f.findRev(Option::dir_sep);
		    if(slsh != -1) {
			dirstr = f.left(slsh+1);
			f = f.right(f.length() - slsh - 1);
		    }
		    if(dirstr.right(Option::dir_sep.length()) != Option::dir_sep)
			dirstr += Option::dir_sep;
		    if(!uninst.isEmpty())
			uninst.append("\n\t");
		    uninst.append(QString("-$(DEL_FILE) -r") + " " + dst + f + "");

		    QDir dir(dirstr, f);
		    for(uint x = 0; x < dir.count(); x++) {
			QString file = dir[x];
			if(file == "." || file == "..") //blah
			    continue;
			file = dirstr + file;
			fileFixify(file);
			QFileInfo fi(file);
			target += QString("-") + (fi.isDir() ? "$(COPY_DIR)" : "$(COPY_FILE)") +
				  " \"" + fi.filePath() + "\" \"" + dst + "\"\n\t";
			if(fi.isExecutable() && !project->isEmpty("QMAKE_STRIP"))
			    target += var("QMAKE_STRIP") + " \"" + dst + "\"\n\t";
		    }
		}
 	    }
 	}
 	//default?
	if(do_default)
	    target = defaultInstall((*it));

	if(!target.isEmpty()) {
	    t << "install_" << (*it) << ": " << "\n\t"
	      << "@test -d " << dst << " || mkdir -p " << dst << "\n\t"
	      << target << endl;
	    all_installs += QString("install_") + (*it) + " ";
	    if(!uninst.isEmpty()) {
		t << "uninstall_" << (*it) << ": " << "\n\t"
		  << uninst.join(" ") << "\n\t"
		  << "-$(DEL_DIR) \"" << dst << "\"" << endl;
		all_uninstalls += "uninstall_" + (*it) + " ";
	    }
	    t << endl;
	}   else {
	    debug_msg(1, "no definition for install %s: install target not created",(*it).latin1());
	}
    }
    t << "install: all " << all_installs << "\n\n";
    t << "uninstall: " << all_uninstalls << "\n\n";
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
	    dirName = Option::fixPathToTargetOS(fi.dirPath(), FALSE) + Option::dir_sep;
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
    writeUicSrc(t, "FORMS");
    writeObj(t, "UICOBJECTS", "UICIMPLS");
    writeMocObj(t, "OBJMOC" );
    writeMocSrc(t, "HEADERS");
    writeMocSrc(t, "SOURCES");
    writeMocSrc(t, "UICDECLS");
    writeYaccSrc(t, "YACCSOURCES");
    writeLexSrc(t, "LEXSOURCES");
    writeImageObj(t, "IMAGEOBJECTS");
    writeImageSrc(t, "QMAKE_IMAGE_COLLECTION");
    writeIdlSrc(t, "SRCIDC" );

    t << "####### Install" << endl << endl;
    writeInstalls(t, "INSTALLS");
    return TRUE;
}

QString MakefileGenerator::buildArgs()
{
    static QString ret;
    if(ret.isEmpty()) {
	//special variables
	if(!project->isEmpty("QMAKE_ABSOLUTE_SOURCE_PATH"))
	    ret += " QMAKE_ABSOLUTE_SOURCE_PATH=\"" + project->first("QMAKE_ABSOLUTE_SOURCE_PATH") + "\"";

	//warnings
	else if(Option::warn_level == WarnNone)
	    ret += " -Wnone";
	else if(Option::warn_level == WarnAll)
	    ret += " -Wall";
	else if(Option::warn_level & WarnParser)
		ret += " -Wparser";
	//other options
	if(!Option::user_template.isEmpty())
	    ret += " -t " + Option::user_template;
	if(!Option::mkfile::do_cache)
	    ret += " -nocache";
	if(!Option::mkfile::do_deps)
	    ret += " -nodepend";
	if(!Option::mkfile::do_mocs)
	    ret += " -nomoc";
	if(!Option::mkfile::do_dep_heuristics)
	    ret += " -nodependheuristics";
	if(!Option::mkfile::qmakespec_commandline.isEmpty())
	    ret += " -spec " + Option::mkfile::qmakespec_commandline;

	//arguments
	for(QStringList::Iterator it = Option::before_user_vars.begin();
	    it != Option::before_user_vars.end(); ++it) {
	    if((*it).left(qstrlen("QMAKE_ABSOLUTE_SOURCE_PATH")) != "QMAKE_ABSOLUTE_SOURCE_PATH")
		ret += " \"" + (*it) + "\"";
	}
	if(Option::after_user_vars.count()) {
	    ret += " -after ";
	    for(QStringList::Iterator it = Option::after_user_vars.begin();
		it != Option::after_user_vars.end(); ++it) {
		if((*it).left(qstrlen("QMAKE_ABSOLUTE_SOURCE_PATH")) != "QMAKE_ABSOLUTE_SOURCE_PATH")
		    ret += " \"" + (*it) + "\"";
	    }
	}
    }
    return ret;
}

//could get stored argv, but then it would have more options than are
//probably necesary this will try to guess the bare minimum..
QString MakefileGenerator::build_args()
{
    static QString ret;
    if(ret.isEmpty()) {
	ret = "$(QMAKE)";

	// general options and arguments
	ret += buildArgs();

	//output
	QString ofile = Option::fixPathToTargetOS(Option::output.name());
	fileFixify(ofile, QDir::currentDirPath());
	if (!ofile.isEmpty() && ofile != project->first("QMAKE_MAKEFILE"))
	    ret += " -o " + ofile;

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
    t << "# Generated by qmake (" << qmake_version() << ") on: " << ctime(&foo);
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
    fileFixify(ofile, QDir::currentDirPath());
    ofile = Option::fixPathToTargetOS(ofile);

    if(project->isEmpty("QMAKE_FAILED_REQUIREMENTS") &&
       !project->isEmpty("QMAKE_INTERNAL_PRL_FILE")) {
	QStringList files = Option::mkfile::project_files;
	fileFixify(files);
	t << project->first("QMAKE_INTERNAL_PRL_FILE") << ": " << "\n\t"
	  << "@$(QMAKE) -prl " << buildArgs() << " " << files.join(" ") << endl;
    }

    QString pfile = project->projectFile();
    if(pfile != "(stdin)") {
	QString qmake = build_args();
	fileFixify(pfile, Option::output_dir);
	if(!ofile.isEmpty() && !project->isActiveConfig("no_autoqmake")) {
	    t << ofile << ": " << pfile << " ";
	    if(Option::mkfile::do_cache) {
		QString s = Option::mkfile::cachefile;
		fileFixify(s);
		t << s << " ";
	    }
	    t << project->variables()["QMAKE_INTERNAL_INCLUDED_FILES"].join(" \\\n\t\t") << "\n\t"
	      << qmake <<endl;
	}
	if(project->first("QMAKE_ORIG_TARGET") != "qmake") {
	    t << "qmake: " <<
		project->variables()["QMAKE_INTERNAL_QMAKE_DEPS"].join(" \\\n\t\t") << "\n\t"
	      << "@" << qmake << endl << endl;
	}
    }
    return TRUE;
}

bool
MakefileGenerator::fileFixify(QStringList &files, const QString &dir) const
{
    if(files.isEmpty())
	return FALSE;
    int ret = 0;
    for(QStringList::Iterator it = files.begin(); it != files.end(); ++it)
	if(!(*it).isEmpty())
	    ret += (int)fileFixify((*it), dir);
    return ret != 0;
}

bool
MakefileGenerator::fileFixify(QString &file, const QString &d) const
{
    if(file.isEmpty())
	return FALSE;
    QString dir( d );
    if(dir.isNull())
	dir = Option::output_dir;

    int depth = 4;
    if(Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE ||
       Option::qmake_mode == Option::QMAKE_GENERATE_PRL) {
	if(project && !project->isEmpty("QMAKE_PROJECT_DEPTH"))
	    depth = project->first("QMAKE_PROJECT_DEPTH").toInt();
	else if(Option::mkfile::cachefile_depth != -1)
	    depth = Option::mkfile::cachefile_depth;
    }

    QString orig_file = file;
    if(!project->isEmpty("QMAKE_ABSOLUTE_SOURCE_PATH")) { //absoluteify it
	file = Option::fixPathToTargetOS(file);
	if(!QDir::isRelativePath(file)) //already absolute
	    return FALSE;
	QFileInfo fi(file);
	if(fi.convertToAbs()) //strange
	    return FALSE;
	file = fi.filePath();
    } else if(!project->isActiveConfig("no_fixpath")) { //relative
	file = Option::fixPathToTargetOS(file, FALSE);
	if(QDir::isRelativePath(file))
	    return FALSE;
	QString match_dir = dir;
	if(file.left(match_dir.length()) == match_dir &&
	   file.mid(match_dir.length(), Option::dir_sep.length()) == Option::dir_sep) {
	    file = file.right(file.length() - (match_dir.length() + 1));
	} else {
	    for(int i = 1; i <= depth; i++) {
		int sl = match_dir.findRev(Option::dir_sep);
		if(sl == -1)
		    break;
		match_dir = match_dir.left(sl);
		if(match_dir.isEmpty())
		    break;
		if(file.left(match_dir.length()) == match_dir &&
		   file.mid(match_dir.length(), Option::dir_sep.length()) == Option::dir_sep) {
		    //concat
		    int remlen = file.length() - (match_dir.length() + 1);
		    if (remlen < 0)
			remlen = 0;
		    file = file.right(remlen);
		    //prepend
		    for(int o = 0; o < i; o++)
			file.prepend(".." + Option::dir_sep);
		}
	    }
	}
    } else { //just clean it
	file = Option::fixPathToTargetOS(file, FALSE);
    }
    debug_msg(3, "Fixed %s :: to :: %s (%d)", orig_file.latin1(), file.latin1(), depth);
    return TRUE;
}

QString
MakefileGenerator::cleanFilePath(const QString &file) const
{
    QString ret = Option::fixPathToTargetOS(file);
    fileFixify(ret);
    return ret;
}

void MakefileGenerator::logicWarn(const QString &f, const QString &w)
{
    if(!(Option::warn_level & WarnLogic))
	return;
    QString file = f;
    int slsh = f.findRev(Option::dir_sep);
    if(slsh != -1)
	file = file.right(file.length() - slsh - 1);
    QStringList &l = project->variables()[w];
    for(QStringList::Iterator val_it = l.begin(); val_it != l.end(); ++val_it) {
	QString file2((*val_it));
	slsh = file2.findRev(Option::dir_sep);
	if(slsh != -1)
	    file2 = file2.right(file2.length() - slsh - 1);
	if(file2 == file) {
	    warn_msg(WarnLogic, "Found potential symbol conflict of %s (%s) in %s",
		     file.latin1(), (*val_it).latin1(), w.latin1());
	    break;
	}
    }
}

QStringList
&MakefileGenerator::findDependencies(const QString &file)
{
    QString key = file;
    Option::fixPathToTargetOS(key);
    if(key.find(Option::dir_sep))
	key = key.right(key.length() - key.findRev(Option::dir_sep) - 1);
    if(!depKeyMap.contains(key))
	depKeyMap.insert(key, file);
    return depends[key];
}


QString
MakefileGenerator::specdir()
{
    if(!spec.isEmpty())
	return spec;
    spec = Option::mkfile::qmakespec;
    const char *d = getenv("QTDIR");
    if(d) {
	QString qdir = Option::fixPathToTargetOS(QString(d));
	if(qdir.right(1) == QString( QChar( QDir::separator() ) ))
	    qdir.truncate(qdir.length()-1);
	//fix path
	QFileInfo fi(spec);
	QString absSpec(fi.absFilePath());
	absSpec = Option::fixPathToTargetOS(absSpec);
	//replace what you can
	if(absSpec.left(qdir.length()) == qdir) {
	    absSpec.replace(0, qdir.length(), "$(QTDIR)");
	    spec = absSpec;
	}
    }
    return spec;
}


//Factory thing
#include "unixmake.h"
#include "borland_bmake.h"
#include "msvc_nmake.h"
#include "msvc_dsp.h"
#include "metrowerks_xml.h"
#include "pbuilder_pbx.h"
#include "projectgenerator.h"

MakefileGenerator *
MakefileGenerator::create(QMakeProject *proj)
{
    if(Option::qmake_mode == Option::QMAKE_GENERATE_PROJECT)
	return new ProjectGenerator(proj);

    MakefileGenerator *mkfile = NULL;
    QString gen = proj->first("MAKEFILE_GENERATOR");
    if(gen.isEmpty()) {
	fprintf(stderr, "No generator specified in config file: %s\n",
		proj->projectFile().latin1());
    } else if(gen == "UNIX") {
	mkfile = new UnixMakefileGenerator(proj);
    } else if(gen == "MSVC") {
	if(proj->first("TEMPLATE").find(QRegExp("^vc.*")) != -1)
	    mkfile = new DspMakefileGenerator(proj);
	else
	    mkfile = new NmakeMakefileGenerator(proj);
    } else if(gen == "BMAKE") {
	mkfile = new BorlandMakefileGenerator(proj);
    } else if(gen == "METROWERKS") {
	mkfile = new MetrowerksMakefileGenerator(proj);
    } else if(gen == "PROJECTBUILDER") {
	mkfile = new ProjectBuilderMakefileGenerator(proj);
    } else {
	fprintf(stderr, "Unknown generator specified: %s\n", gen.latin1());
    }
    return mkfile;
}
