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

#include "qarchive.h"
#include <qapplication.h>
#include <qdir.h>
#include <qfileinfo.h>

class ConsoleOutput : public QObject
{
    Q_OBJECT
public:
    ConsoleOutput() : QObject() { }
    ~ConsoleOutput() { }
public slots:
    void updateProgress( const QString& str) {	qDebug("%s", str.latin1());   }
};

int main( int argc, char** argv )
{
    bool output = TRUE;
    QStringList files;
    for(int i = 1; i < argc; i++) {
	if(!strcmp(argv[i], "-s")) {
	    output = FALSE;
	} else {
	    files.append(argv[i]);
	}
    }
    if(!files.isEmpty()) {
	QArchive archive;
	ConsoleOutput out;
	if(output) {	
	    archive.setVerbosity( QArchive::Destination | QArchive::Verbose );
	    QObject::connect( &archive, SIGNAL( operationFeedback( const QString& ) ), 
			      &out, SLOT( updateProgress( const QString& ) ) );
	}
	for(QStringList::Iterator it = files.begin(); it != files.end(); ++it) {
	    archive.setPath( (*it) );
	    if( !archive.open( IO_ReadOnly ) ) {
		qDebug("Failed to open input %s", (*it).latin1());
		continue;
	    } 
	    if(!archive.readArchive( QDir::currentDirPath() )) 
		qDebug("Failed to unpack %s", (*it).latin1());
	    archive.close();
	}
    }
    return 0;
}

#include "main.moc"
