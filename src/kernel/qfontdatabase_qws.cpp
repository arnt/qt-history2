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

void QFontDatabase::createDatabase()
{
    if ( db ) return;
    db = new QFontDatabasePrivate;

    QDiskFont * qdf;
    QString fname="Truetype";
    QtFontFoundry * foundry=new QtFontFoundry(fname);
    db->addFoundry(foundry);

    if(!qt_fontmanager)
        qt_fontmanager=new QFontManager();

    for(qdf=qt_fontmanager->diskfonts.first();qdf!=0;
        qdf=qt_fontmanager->diskfonts.next()) {
        QString familyname=qdf->name;
        QtFontFamily * family=foundry->familyDict.find(familyname);
        if(!family) {
            family=new QtFontFamily(foundry,familyname);
            foundry->addFamily(family);
        }
        QString weightString;
        int weight=qdf->weight;
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
        mystyle->setSmoothlyScalable();
    }
}

