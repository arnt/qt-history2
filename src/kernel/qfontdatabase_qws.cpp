/****************************************************************************
** $Id$
**
** Implementation of platform specific QFontDatabase
**
** Created : 970521
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qfontmanager_qws.h"
#include "qdir.h"


extern QString qws_topdir();

static void qt_setFontWeight( int &weight, QString &weightString )
{
    if ( weight <= QFont::Light ) {
	weight = QFont::Light;
	weightString = "Light";
    } else if ( weight <= QFont::Normal ) {
	weight = QFont::Normal;
	weightString = "Normal";
    } else if ( weight <= QFont::DemiBold ) {
	weight = QFont::DemiBold;
	weightString = "DemiBold";
    } else if ( weight <= QFont::Bold ) {
	weight = QFont::Bold;
	weightString = "Bold";
    } else {
	weight = QFont::Black;
	weightString = "Black";
    }
}

static QtFontFoundry *qt_ttffoundry=0;
static QtFontFoundry *qt_bdffoundry=0;

/*!
    \internal
*/
void QFontDatabase::qwsAddDiskFont( QDiskFont *qdf )
{
    if ( !db )
	return;

    QString familyname=qdf->name;
    QtFontFoundry *foundry = qt_ttffoundry;
    if ( qdf->factory->name() == "BDF" )
	foundry = qt_bdffoundry;
    QtFontFamily * family=foundry->familyDict.find(familyname);
    if(!family) {
	family=new QtFontFamily(foundry,familyname);
	foundry->addFamily(family);
    }
    QString weightString;
    int weight=qdf->weight;
    qt_setFontWeight( weight, weightString );
    QString style;
    if(qdf->italic) {
	style=weightString+" Italic";
    } else {
	style=weightString;
    }
    QtFontStyle * mystyle=family->styleDict.find(style);
    if(!mystyle) {
	mystyle=new QtFontStyle(family,style);
	mystyle->ital=qdf->italic;
	mystyle->lesserItal=FALSE;
	mystyle->weightString=weightString;
	mystyle->weightVal=weight;
	mystyle->weightDirty=FALSE;
	family->addStyle(mystyle);
    }
    if ( qdf->factory->name() == "FT" )
	mystyle->setSmoothlyScalable();
}

void QFontDatabase::createDatabase()
{
    if ( db ) return;
    db = new QFontDatabasePrivate;

    if ( !qt_ttffoundry ) {
	qt_ttffoundry=new QtFontFoundry( "Truetype" );
	db->addFoundry(qt_ttffoundry);
    }

    if ( !qt_bdffoundry ) {
	qt_bdffoundry = new QtFontFoundry( "BDF" );
	db->addFoundry(qt_bdffoundry);
    }

    QtFontFoundry *foundry = qt_ttffoundry;

    if(!qt_fontmanager)
        qt_fontmanager=new QFontManager();

    QDiskFont *qdf;
    for ( qdf=qt_fontmanager->diskfonts.first();qdf!=0;
	  qdf=qt_fontmanager->diskfonts.next()) {
	qwsAddDiskFont( qdf );
    }

#ifndef QT_NO_DIR
    foundry=new QtFontFoundry( "Qt" );
    db->addFoundry( foundry );

    QDir dir(qws_topdir()+"/lib/fonts/","*.qpf");
    for (int i=0; i<(int)dir.count(); i++) {
	int u0 = dir[i].find('_');
	int u1 = dir[i].find('_',u0+1);
	int u2 = dir[i].find('_',u1+1);
	QString familyname = dir[i].left(u0);
	int pointSize = dir[i].mid(u0+1,u1-u0-1).toInt();
	int weight = dir[i].mid(u1+1,u2-u1-1).toInt();
	bool italic = dir[i].mid(u2-1,1) == "i";
	QtFontFamily *family = foundry->familyDict.find( familyname );
	if ( !family ) {
	    family=new QtFontFamily(foundry,familyname);
	    foundry->addFamily(family);
	}
	QString weightString;
	qt_setFontWeight( weight, weightString );
	QString style;
	if(italic) {
	    style=weightString+" Italic";
	} else {
	    style=weightString;
	}
	QtFontStyle * mystyle=family->styleDict.find(style);
	if(!mystyle) {
	    mystyle=new QtFontStyle(family,style);
	    mystyle->ital=italic;
	    mystyle->lesserItal=FALSE;
	    mystyle->weightString=weightString;
	    mystyle->weightVal=weight;
	    mystyle->weightDirty=FALSE;
	    family->addStyle(mystyle);
	}
	mystyle->addPointSize( pointSize );
    }
#endif
}

