/****************************************************************************
** $Id: $
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

#include "winmakefile.h"
#include "option.h"
#include "project.h"
#include <qtextstream.h>
#include <qstring.h>
#include <qregexp.h>
#include <qstringlist.h>
#include <qdir.h>


Win32MakefileGenerator::Win32MakefileGenerator(QMakeProject *p) : MakefileGenerator(p)
{

}


void
Win32MakefileGenerator::writeSubDirs(QTextStream &t)
{
    unsigned int subLevels;
    unsigned int i;
    if(!project->isEmpty("MAKEFILE"))
	t << "MAKEFILE=	" << var("MAKEFILE") << endl;
    t << "QMAKE =	" << (project->isEmpty("QMAKE_QMAKE") ? QString("qmake") : var("QMAKE_QMAKE")) << endl;
    t << "SUBDIRS	=" << varList("SUBDIRS") << endl;

    t << "all: qmake_all $(SUBDIRS)" << endl << endl;

    QStringList &sdirs = project->variables()["SUBDIRS"];
    QStringList::Iterator sdirit;

    t << "### DEBUG" << endl;
    for(sdirit = sdirs.begin(); sdirit != sdirs.end(); ++sdirit) {
	QString subdir = *sdirit;
	t << subdir << ":";
	subLevels = 1;
	for( i = 0; i < subdir.length(); i++ ) {
	    if( subdir.at( i ) == '/' ) subLevels++;
	}
	if(project->variables()["QMAKE_NOFORCE"].isEmpty())
	    t << " FORCE";
	t << "\n\t"
	  << "cd " << subdir << "\n\t"
	  << "$(MAKE)" << "\n\t"
	  << "@cd ..";
        for( i = 1; i < subLevels; i++ )
        {
	    t << "\\..";
	}
	t << endl << endl;
    }

    if(project->variables()["QMAKE_INTERNAL_QMAKE_DEPS"].findIndex("qmake_all") == -1)
	project->variables()["QMAKE_INTERNAL_QMAKE_DEPS"].append("qmake_all");
    writeMakeQmake(t);

    t << "qmake_all:";
    if( sdirs.count() > 0 ) {
	for(sdirit = sdirs.begin(); sdirit != sdirs.end(); ++sdirit) {
	    QString subdir = *sdirit;
	    int lastSeparator( 0 );
	    subLevels = 1;
	    for( i = 0; i < subdir.length(); i++ ) {
		if( subdir.at( i ) == '/' ) {
		    subLevels++;
		    lastSeparator = i;
		}
	    }
	    t << "\n\t"
	      << "cd " << subdir << "\n\t";
	    if( lastSeparator ) {
		subdir = subdir.mid( lastSeparator + 1 );
	    }
	    t << "$(QMAKE) " << subdir << ".pro" 
	      << (!project->isEmpty("MAKEFILE") ? QString(" -o ") + var("MAKEFILE") : QString(""))
	      << " " << buildArgs() << "\n\t"
	      << "@cd ..";
	    for( i = 1; i < subLevels; i++ ) {
		t << "\\..";
	    }
	}
    } else {
	// Borland make does not like empty an empty command section, so insert
	// a dummy command.
	t << "\n\t" << "@cd .";
    }
    t << endl << endl;

    QString targs[] = { QString("clean"), QString("install"), QString("mocclean"), QString::null };
    for(int x = 0; targs[x] != QString::null; x++) {
        t << targs[x] << ":";
	if(targs[x] == "install")
	    t << " qmake_all";
	if ( sdirs.count() > 0 ) {
	    for(sdirit = sdirs.begin(); sdirit != sdirs.end(); ++sdirit) {
		QString subdir = *sdirit;
		subLevels = 1;
		for( i = 0; i < subdir.length(); i++ ) {
		    if( subdir.at( i ) == '/' ) subLevels++;
		}
		t << "\n\t"
		    << "cd " << subdir << "\n\t"
		    << "$(MAKE) " << targs[x] << "\n\t"
		    << "@cd ..";
		for( i = 1; i < subLevels; i++ ) {
		    t << "\\..";
		}
	    }
	} else {
	    // Borland make does not like empty an empty command section, so
	    // insert a dummy command.
	    t << "\n\t" << "@cd .";
	}
	t << endl << endl;
    }

    if(project->variables()["QMAKE_NOFORCE"].isEmpty())
	t << "FORCE:" << endl << endl;
}


int
Win32MakefileGenerator::findHighestVersion(const QString &d, const QString &stem)
{
    if(!QFile::exists(d)) 
	return -1;

    if(!project->variables()["QMAKE_" + stem.upper() + "_VERSION_OVERRIDE"].isEmpty()) 
	return project->variables()["QMAKE_" + stem.upper() + "_VERSION_OVERRIDE"].first().toInt();

    QString bd = d;
    fixEnvVariables(bd);
    QDir dir(bd, stem + "*.lib");
    QStringList entries = dir.entryList();
    int nbeg, nend, biggest=-1;
    for(QStringList::Iterator it = entries.begin(); it != entries.end(); ++it) {
	if((nbeg = (*it).find(QRegExp("[0-9]"))) != -1) {
	    nend = (*it).findRev(QRegExp("[0-9]", nbeg)) + 1;
	    int n = (*it).mid(nbeg, nend - nbeg).toInt();
	    if(n > biggest)
		biggest = n;
	} else if(biggest == -1) {
	    biggest = 0;
	}
    }
    return biggest;
}

bool 
Win32MakefileGenerator::findLibraries(const QString &where)
{
    QStringList &l = project->variables()[where];
    QPtrList<MakefileDependDir> dirs;
    dirs.setAutoDelete(TRUE);
    for(QStringList::Iterator it = l.begin(); it != l.end(); ) {
        QString opt = (*it);
        bool remove = FALSE;
        if(opt.left(2) == "-L" || opt.left(2) == "/L") {
            QString r = opt.right(opt.length() - 2), l = Option::fixPathToLocalOS(r);
            dirs.append(new MakefileDependDir(r.replace(QRegExp("\""),""), 
                                              l.replace(QRegExp("\""),"")));
            remove = TRUE;
        } else if(opt.left(2) == "-l" || opt.left(2) == "/l") {
            QString lib = opt.right(opt.length() - 2), out;
            if(!lib.isEmpty()) {
                for(MakefileDependDir *mdd = dirs.first(); mdd; mdd = dirs.next() ) {
                    int ver = findHighestVersion(mdd->local_dir, lib);
                    if(ver != -1) {
                        out = QString(mdd->real_dir + Option::dir_sep + lib + "%1" + ".lib");
			if(ver)
			    out = out.arg(ver);
			else
			    out = out.arg("");
                        break;
                    }
                }
            }
            if(out.isEmpty())
                remove = TRUE;
            else
                (*it) = out;
        } else if(!QFile::exists(Option::fixPathToLocalOS(opt))) {
            QString dir, file = opt;
            int slsh = file.findRev(Option::dir_sep);
            if(slsh != -1) {
                dir = file.left(slsh+1);
                file = file.right(file.length() - slsh - 1);
            }
            if(file.right(4) == ".lib") {
                file = file.left(file.length() - 4);
                if(!file.at(file.length()-1).isNumber()) {
                    int ver = findHighestVersion(dir, file);
                    if(ver != -1) {
                        file = QString(dir + file + "%1" + ".lib");
			if(ver)
			    (*it) = file.arg(ver);
			else
			    (*it) = file.arg("");
		    }
                }
            }
        }
        if(remove)
            it = l.remove(it);
        else
            ++it;
    }
    return TRUE;
}
