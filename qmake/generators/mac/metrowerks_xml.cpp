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
#include "metrowerks_xml.h"
#include <time.h>
#include <qdir.h>
#include <qregexp.h>
#include <stdlib.h>

MetrowerksMakefileGenerator::MetrowerksMakefileGenerator(QMakeProject *p) : MakefileGenerator(p), init_flag(FALSE)
{

}

bool
MetrowerksMakefileGenerator::writeMakefile(QTextStream &t)
{
    if(!project->variables()["QMAKE_FAILED_REQUIREMENTS"].isEmpty()) {
	/* for now just dump, I need to generated an empty dsp or something.. */
	fprintf(stderr, "Project file not generated because all requirements not met:\n\t%s\n",
		var("QMAKE_FAILED_REQUIREMENTS").latin1());
	return TRUE;
    }

    if(project->first("TEMPLATE") == "vcapp" ||
       project->first("TEMPLATE") == "vclib") {
	return writeMakeParts(t);
    }
    else if(project->first("TEMPLATE") == "subdirs") {
	writeHeader(t);
	qDebug("Not supported!");
	return TRUE;
    }
    return FALSE;
}

bool
MetrowerksMakefileGenerator::writeMakeParts(QTextStream &t)
{
    QString dspfile;
    if ( !project->variables()["DSP_TEMPLATE"].isEmpty() ) {
	dspfile = project->first("DSP_TEMPLATE");
    } else {
	dspfile = project->first("MSVCDSP_TEMPLATE");
    }
    dspfile = findTemplate(dspfile);

    QFile file(dspfile);
    if(!file.open(IO_ReadOnly)) {
	fprintf(stderr, "Cannot open dsp file: %s\n", dspfile.latin1());
	return FALSE;
    }
    QTextStream dsp(&file);

    int rep;
    QString line;
    while ( !dsp.eof() ) {
	line = dsp.readLine();
	while((rep = line.find(QRegExp("\\$\\$[a-zA-Z0-9_-]*"))) != -1) {
	    QString torep = line.mid(rep, line.find(QRegExp("[^\\$a-zA-Z0-9_-]"), rep) - rep);
	    QString variable = torep.right(torep.length()-2);

	    t << line.left(rep); //output the left side
	    line = line.right(line.length() - (rep + torep.length())); //now past the variable
	    if(variable == "MSVCDSP_SOURCES") {
		if(project->variables()["SOURCES"].isEmpty())
		    continue;

		QStringList &list = project->variables()["SOURCES"];
		for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
		}
	    }
	    else if(variable == "MSVCDSP_HEADERS") {
		if(project->variables()["HEADERS"].isEmpty())
		    continue;

		QStringList &list = project->variables()["HEADERS"];
		for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
		}
	    }
	    else
		t << var(variable);
	}
	t << line << endl;
    }
    t << endl;
    file.close();
    return TRUE;
}



void
MetrowerksMakefileGenerator::init()
{
    if(init_flag)
	return;
    QStringList::Iterator it;
    init_flag = TRUE;

    /* this should probably not be here, but I'm using it to wrap the .t files */
    if(project->first("TEMPLATE") == "vcapp" )
	project->variables()["QMAKE_APP_FLAG"].append("1");
    else if(project->first("TEMPLATE") == "vclib")
	project->variables()["QMAKE_LIB_FLAG"].append("1");
    
    QStringList &configs = project->variables()["CONFIG"];
    if (project->isActiveConfig("qt_dll"))
	if(configs.findIndex("qt") == -1) configs.append("qt");

    MakefileGenerator::init();
}


QString
MetrowerksMakefileGenerator::findTemplate(QString file)
{
    QString ret;
    if(!QFile::exists(ret = file) && !QFile::exists((ret = (QString(getenv("HOME")) + "/.tmake/" + file))) &&
	!QFile::exists((ret = QString(getenv("QTDIR")) + "/mkspecs/mac-mwerks/" + file)))
	return "";
    return ret;
}

