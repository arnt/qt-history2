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
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <qdir.h>
#include <qfile.h>
#include <qtextstream.h>

MakefileGenerator::MakefileGenerator(QMakeProject *p) : project(p), init_already(FALSE), moc_aware(FALSE)
{
}

bool
MakefileGenerator::generateMocList(QString fn)
{
    QString &m = mocablesToMOC[fn];
    if(!m.isEmpty())
	return TRUE;

    QFile file(fn);
    if ( file.open(IO_ReadOnly) ) {
	QTextStream t( &file );
	QString s;
	while ( !t.eof() ) {
	    s = t.readLine();
	    if(s.find(QRegExp("Q_OBJECT")) != -1) {
		QString mocFile;
		QFileInfo fi(fn);
		if(!project->variables()["MOC_DIR"].isEmpty())
		    mocFile = project->variables()["MOC_DIR"].first() + "/";
		else
		    mocFile = fi.dirPath() + "/";

		if(fi.extension(FALSE) == CPP_EXT+1) {
		    mocFile += fi.baseName() + MOC_EXT;
		    project->variables()["_SRCMOC"].append(mocFile);
		}
		else if(fi.extension(FALSE) == H_EXT+1 && 
			project->variables()["HEADERS"].findIndex(fn) != -1) {
		    mocFile += "moc_" + fi.baseName() + CPP_EXT;
		    project->variables()["_HDRMOC"].append(mocFile);
		}
		else break;
		mocablesToMOC[fn] = mocFile;
		mocablesFromMOC[mocFile] = fn;
		file.close();
		return TRUE;
	    }
	}
	file.close();
    }
    return FALSE;
}


bool
MakefileGenerator::generateDependancies(QStringList &dirs, QString fn)
{
    QStringList &fndeps = depends[fn];
    if(!fndeps.isEmpty())
	return TRUE;

    QFile file(fn);
    if ( file.open(IO_ReadOnly) ) {
	QTextStream t( &file );
	QString s, inc;
	while ( !t.eof() ) {
	    s = t.readLine().stripWhiteSpace();
	    if(s.left(9) == "#include ")
		inc = s.mid(10, s.length() - 11);
	    else
		continue;
		
	    QString fqn;
	    if(inc.right(strlen(MOC_EXT)) == MOC_EXT) {
//		fndeps.append(inc);
		continue;
	    }
	    else if(QFile::exists(inc))
		fqn = inc;
	    else if(QDir::isRelativePath(inc)) {
		bool found=false;
		for(QStringList::Iterator it = dirs.begin(); !found && it != dirs.end(); ++it) {
		    found = QFile::exists((fqn = ((*it) + "/" + inc)));
		}
		if(!found)
		    continue;
	    } else {
		continue;
	    }
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

#ifdef QMAKE_DEBUG
	printf("Dependancies: %s -> %s\n", fn.latin1(), fndeps.join(" :: ").latin1());
#endif
	return TRUE;
    }
    return FALSE;
}

void
MakefileGenerator::init(QTextStream &t)
{
    if(init_already)
	return;
    init_already = TRUE;

    /* fix up dirs */
    {
	QString dirs[] = { QString("OBJECTS_DIR"), QString("MOC_DIR"), QString("DESTDIR"), QString::null };
	for(int x = 0; dirs[x] != QString::null; x++) {
	    if ( !project->variables()[dirs[x]].isEmpty()) {
		QDir::cleanDirPath(project->variables()[dirs[x]].first());
		mkdir(project->variables()[dirs[x]].first().latin1(),0777);
	    }
	}
    }

    /* get deps and mocables */
    {
	QMap<QString, QStringList> &v = project->variables();

	QStringList incDirs;
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
#ifdef QMAKE_DEBUG
	printf("Dependancy Directories: %s\n", incDirs.join(" :: ").latin1());
#endif

	QString sources[] = { QString("HEADERS"), QString("SOURCES"), QString("INTERFACES"), QString::null };
	printf("Generating dependancies.\n");
	for(int x = 0; sources[x] != QString::null; x++) {
	    QStringList &l = v[sources[x]];
	    for(QStringList::Iterator val_it = l.begin(); val_it != l.end(); ++val_it) {
		if(!(*val_it).isEmpty()) {
		    generateDependancies(incDirs, (*val_it));
		}
	    }
	}
	QString headers[] = { QString("HEADERS"), QString("INTERFACES"), QString::null };
	printf("Generating mocables.\n");
	for(int x = 0; headers[x] != QString::null; x++) {
	    QStringList &l = v[headers[x]];
	    for(QStringList::Iterator val_it = l.begin(); val_it != l.end(); ++val_it) {
		if(!(*val_it).isEmpty()) {
		    generateMocList((*val_it));
		}
	    }
	}
    }

    /* init variables */
    project->variables()["OBJECTS"].clear();
    project->variables()["OBJECTS"] += createObjectList("SOURCES");


    {
	QStringList &decls = project->variables()["UICDECLS"], &impls = project->variables()["UICIMPLS"];
	QStringList &l = project->variables()["INTERFACES"];
	for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
	    QFileInfo fi((*it));
	    QString impl = fi.dirPath() + "/" + fi.baseName() + CPP_EXT;
	    QString decl = fi.dirPath() + "/" + fi.baseName() + H_EXT;
	    decls.append(decl);
	    impls.append(impl);
	    depends[impl].append(decl);
	    if(!project->variables()["MOC_DIR"].isEmpty())
		project->variables()["_UIMOC"].append(project->variables()["MOC_DIR"].first() + "/moc_" + 
						      fi.baseName() + CPP_EXT);
	    else
		project->variables()["_UIMOC"].append(fi.dirPath() + "moc_" + fi.baseName() + CPP_EXT);
	}
	project->variables()["OBJECTS"] += project->variables()["UICOBJECTS"] = createObjectList("INTERFACES");
    }

    if ( moc_aware ) {
	project->variables()["OBJMOC"] = createObjectList("_HDRMOC") + createObjectList("_UIMOC");
	project->variables()["SRCMOC"] = project->variables()["_HDRMOC"] + project->variables()["_SRCMOC"]
					 + project->variables()["_UIMOC"];
    }
}

bool
MakefileGenerator::write(const char *file)
{
    QFile f;
    if(file) {
	f.setName(file);
	if(!f.open(IO_WriteOnly))
	    return FALSE;
	mfile = file;
    }
    else {
	if(!f.open(IO_WriteOnly, stdout))
	    return FALSE;
	mfile = "";
    }

    QTextStream t(&f);
    init(t);
    writeMakefile(t);
    f.close();
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
	if((*sit).right(strlen(CPP_EXT)) == CPP_EXT) {
	    comp = "TMAKE_RUN_CC";
	    cimp = "TMAKE_RUN_CC_IMP";
	} else {
	    comp = "TMAKE_RUN_CXX";
	    cimp = "TMAKE_RUN_CXX_IMP";
	}
	if ( !project->variables()["OBJECTS_DIR"].isEmpty() || project->variables()[comp].isEmpty())
	    t << "\n\t" << var(comp);
	t << endl << endl;
    }
}


void
MakefileGenerator::writeUicSrc(QTextStream &, const QString &ui)
{
#if 0
    my($f) = @_;
    my(@v,$m,$o);
    @v = split(/\s+/,$f);
    foreach $m ( @v ) {
	$decl = $m;
	$decl =~ s/\.ui$/\.$uic_decl/;
	$impl = $m;
	$impl =~ s/\.ui$/\.$uic_impl/;
	$text .= "$decl: $m\n\t$uic_cmd $m -o $decl\n\n";
	$fulldecl = $decl;
	$decl =~ s/.*\///; # No path - use -I... to find it.
	if ( ! ( $fulldecl eq $decl ) ) {
	  $text .= "$decl: $m\n\t$uic_cmd $m -o $decl\n\n";
	}
	$text .= "$impl: $m\n\t$uic_cmd $m -i $decl -o $impl\n\n";
    }
    chop $text;
#endif
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
	     project->variables()["TMAKE_RUN_CXX_IMP"].isEmpty())
	    t << "\n\t" << var("TMAKE_RUN_CXX");
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


QString
MakefileGenerator::var(const QString &var)
{
    return varGlue(var, "", " ", "");
}


QString
MakefileGenerator::varGlue(const QString &var, const QString &before, const QString &glue, const QString &after)
{
    return before + project->variables()[var].join(glue) + after;
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
	if((*it).isEmpty())
	    continue;
	QFileInfo fi((*it));
	ret.append((dir.isEmpty() ? fi.dirPath() : dir) + "/" + fi.baseName() + OBJ_EXT);
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
    writeMocSrc(t, "UICDELCS");
}
