/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevice.h#73 $
**
** Definition of QFontFactory for Truetype class for Embedded Qt
**
** Created : 940721
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include "qfontmanager_qws.h"
#include "qfontfactoryttf_qws.h"
#include "qfontfactorybdf_qws.h"
#include "qfontdata_p.h"
#include "qfile.h"
#include <stdio.h>
#include <string.h>

//
// /usr/local/qt-embedded/etc/fonts/fontdir lists a sequence of:
//
//    <name> <file> <renderer> <italic> <weight> <size> <flags>
//
// eg.
//      <name> = Helvetica
//      <file> = /usr/local/qt-embedded/etc/fonts/helvR0810.bdf or
//                 /usr/local/qt-embedded/etc/fonts/verdana.ttf, etc.
//  <renderer> = BDF or TTF
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
    const char* r = getenv("QWSDIR");
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

#if QT_FEATURE_TRUETYPE
    factories.append(new QFontFactoryTTF());
#endif
#if QT_FEATURE_BDF
    factories.append(new QFontFactoryBDF());
#endif

    // Load in font definition file
    QString fn = qws_topdir() + "/etc/fonts/fontdir";
    FILE* fontdef=fopen(fn.local8Bit(),"r");
    if(!fontdef) {
	qFatal("Need a fonts definition file - see qfontmanager_qws.cpp"
	       " for the format");
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

