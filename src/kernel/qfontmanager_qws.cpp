/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontmanager_qws.cpp#2 $
**
** Definition of QFontFactory for Truetype class for Embedded Qt
**
** Created : 940721
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
#include "qfontfactoryttf_qws.h"
#include "qfontfactorybdf_qws.h"
#include "qfontdata_p.h"
#include "qfile.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//
// $QTDIR/etc/fonts/fontdir lists a sequence of:
//
//    <name> <file> <renderer> <italic> <weight> <size> <flags>
//
// eg.
//      <name> = Helvetica
//      <file> = /usr/local/qt-embedded/etc/fonts/helvR0810.bdf or
//                 /usr/local/qt-embedded/etc/fonts/verdana.ttf, etc.
//  <renderer> = BDF or FT
//    <italic> = y or n
//    <weight> = 50 is Normal, 75 is Bold, etc.
//      <size> = 0 for scalable or 10 time pointsize (eg. 120 for 12pt)
//     <flags> = flag characters:
//                 s = smooth (anti-aliased)
//                 u = unicode range when saving (default is Latin 1)
//                 a = ascii range when saving (default is Latin 1)
//
// and of course...
//    # This is a comment
//

QString qws_topdir()
{
    const char* r = getenv("QTDIR");
    if ( !r ) r = "/usr/local/qt-embedded";
    return r;
}

QFontManager * qt_fontmanager=0;

void QFontManager::initialize()
{
    qt_fontmanager=new QFontManager();
}

void QFontManager::cleanup()
{
    delete qt_fontmanager;
    qt_fontmanager = 0;
}

QRenderedFont::QRenderedFont(QDiskFont * df, const QFontDef &d)
{
    diskfont = df;
    ptsize=d.pointSize;
    refcount=0;
    ref();
    fleftbearing=0;
    frightbearing=0;
    fmaxwidth=0;
    smooth = df->flags.contains('s');
    if ( df->flags.contains('u') ) {
	maxchar = 65535;
    } else if ( df->flags.contains('a') ) {
	maxchar = 127;
    } else {
	maxchar = 255;
    }
}

QRenderedFont::~QRenderedFont()
{
}

// Triggering a whole font metrics call is bad, so right now return
// some best guesses

int QRenderedFont::minLeftBearing()
{
    return 0;
}

int QRenderedFont::minRightBearing()
{
    return 0;
}

int QRenderedFont::maxWidth()
{
    // Actually max advance
    return fmaxwidth;
}

QFontManager::QFontManager()
{
    factories.setAutoDelete(true);
    //######## memory leak instead of segfault:
    //diskfonts.setAutoDelete(true);

#ifndef QT_NO_FREETYPE
    factories.append(new QFontFactoryFT());
#endif
#ifndef QT_NO_BDF
    factories.append(new QFontFactoryBDF());
#endif

    // Load in font definition file
    QString fn = qws_topdir() + "/etc/fonts/fontdir";
    FILE* fontdef=fopen(fn.local8Bit(),"r");
    if(!fontdef) {
	QCString temp=fn.local8Bit();
	qWarning("Cannot find font definition file %s - is $QTDIR set correctly?",
	       temp.data());
	exit(1);
	//return;
    }
    char buf[200]="";
    char name[200]="";
    char render[200]="";
    char file[200]="";
    char flags[200]="";
    char isitalic[10]="";
    fgets(buf,200,fontdef);
    while(!feof(fontdef)) {
	if ( buf[0] != '#' ) {
	    QFontFactory * factoryp;
	    int weight=50;
	    int size=0;
	    flags[0]=0;
	    sscanf(buf,"%s %s %s %s %d %d %s",name,file,render,isitalic,&weight,&size,flags);
	    QString filename;
	    if ( file[0] != '/' )
		filename = qws_topdir() + "/etc/fonts/";
	    filename += file;
	    if ( QFile::exists(filename) ) {
		for(factoryp=factories.first();factoryp;factoryp=factories.next()) {
		    if( factoryp->name() == render ) {
			QDiskFont * qdf=new QDiskFont(factoryp,name,isitalic[0]=='y',
						      weight,size,flags,filename);
			diskfonts.append(qdf);
			break;
		    }
		}
	    }
	}
	fgets(buf,200,fontdef);
    }
    fclose(fontdef);
}

QFontManager::~QFontManager()
{
    if ( qt_fontmanager == this )
	qt_fontmanager = 0;
}

extern bool qws_savefonts; //in qapplication_qws.cpp

QRenderedFont * QFontManager::get(const QFontDef & f)
{
    QRenderedFont * ret;

    QDiskFont * qdf;
    QDiskFont * bestmatch=diskfonts.first();
    int bestmatchval=0;

    for(qdf=diskfonts.first();qdf;qdf=diskfonts.next()) {
	int mymatchval = 100;
	if (qdf->name.lower() == f.family.lower())
	    mymatchval += 1000;
	// Match closest weight
	mymatchval-=abs(f.weight-qdf->weight);
	// Favour italicness ahead of weight
	if (f.italic==qdf->italic)
	    mymatchval+=100;
	if ( qdf->size )
	    mymatchval+=1 - abs(qdf->size - f.pointSize);
	if ( mymatchval>bestmatchval) {
	    bestmatchval=mymatchval;
	    bestmatch=qdf;
	}
    }
    // Hmm. Bad things are likely to happen if font weights don't exactly
    // match bold and so forth
    if(bestmatch) {
        bestmatch->factory->load(bestmatch);
	ret=bestmatch->factory->get(f,bestmatch);
	ret->italic=bestmatch->italic;
	ret->weight=bestmatch->weight;
	return ret;
    } else {
	return 0;
    }
}

