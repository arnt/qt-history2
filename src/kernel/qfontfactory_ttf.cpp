/****************************************************************************
** $Id: //depot/q/main/src/kernel/qpaintdevice.h#73 $
**
** Definition of QFontFactory for Truetype class for QWS
**
** Created : 940721
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qfontfactory_ttf.h"

#if QT_FEATURE_TRUETYPE

#include "qfontdata_p.h"
#include <string.h>
#include <stdio.h>

#define FLOOR(x)  ((x) & -64)
#define CEIL(x)   (((x)+63) & -64)
#define TRUNC(x)  ((x) >> 6)


extern "C" {
#include <freetype.h>
    //#include <ftoutln.h>
#include <ftbbox.h>
}

extern "C" FT_Error  FT_Outline_Get_Bitmap( FT_Library   library,
                                   FT_Outline*  outline,
                                   FT_Bitmap*   map );

class QDiskFontTTF : public QDiskFontPrivate {
public:
    FT_Face face;
};

class QRenderedFontTTF : public QRenderedFont {
public:
    QRenderedFontTTF(QDiskFont* f, const QFontDef &d) :
	QRenderedFont(f,d)
    {
	QDiskFontTTF *df = (QDiskFontTTF*)(f->p);

	int psize=(ptsize<<6)/10;

	// Assume 72 dpi for now
	FT_Error err;
	FT_Set_Char_Size(df->face,psize,psize, 72,72);

	myface=df->face;

	err=FT_New_Size(df->face,&size);
	if(err) {
	    qFatal("New size error %d\n",err);
	}

	// Hmm. This appears to be set to 0. Not implemented in Freetype 2
	// yet?

	err=FT_Set_Char_Size(myface,psize,psize,72,72);

	fascent=size->metrics.ascender >> 6;
	fdescent=size->metrics.descender >> 6;
	fmaxwidth=size->metrics.max_advance >> 6;

	// Slight fudge factor
	fascent++;
	fdescent--;
    }

    ~QRenderedFontTTF()
    {
	FT_Done_Size(size);
    }

    bool unicode(int & i) const
    {
	int ret;

	FT_Face theface=myface;

	ret=FT_Get_Char_Index(theface,i);

	if (ret==0) {
	    return FALSE;
	} else {
	    i=ret;
	    return TRUE;
	}
    }

    bool inFont(QChar ch) const
    {
	int index = ch.unicode();
	return unicode(index);
    }

    QGlyph render(QChar ch)
    {
	int psize=(ptsize<<6)/10;

	int index = ch.unicode();
	if ( !unicode(index) )
	    index = 0;
	QGlyph result;

	FT_Error err;

	err=FT_Set_Char_Size(myface, psize,psize,72,72);
	if(err)
	    qFatal("Set char size error %x for size %d",err,ptsize);

	FT_Bitmap mybits;

	err=FT_Load_Glyph(myface,index,FT_LOAD_DEFAULT);
	if(err)
	    qFatal("Load glyph error %x",err);

	int width,height,pitch,size;
	FT_GlyphSlot glyph=myface->glyph;

	FT_BBox bbox;
	FT_Raster_GetBBox(&glyph->outline,&bbox);
	//FT_Outline_Get_CBox(&glyph->outline,&bbox);

	bbox.xMin=FLOOR(bbox.xMin);
	bbox.xMax=CEIL(bbox.xMax);
	bbox.yMin=FLOOR(bbox.yMin);
	bbox.yMax=CEIL(bbox.yMax);

	width=(bbox.xMax - bbox.xMin)/64;
	height=(bbox.yMax - bbox.yMin)/64;

	if (smooth) {
	    pitch=(width+3) & -4;
	    mybits.pixel_mode=ft_pixel_mode_grays;
	} else {
	    pitch=(width+7) >> 3;
	    mybits.pixel_mode=ft_pixel_mode_mono;
	}

	size=pitch*height;

	qDebug("%d bytes for U%04x",size,ch.unicode());
	result.data = new uchar[size]; // XXX memory manage me

	mybits.width=width;
	mybits.rows=height;
	mybits.pitch=pitch;
	mybits.buffer=result.data;

	if ( size ) {
	    memset(mybits.buffer,0,size);

	    FT_Outline_Translate(&glyph->outline,-bbox.xMin,-bbox.yMin);

	    err=FT_Outline_Get_Bitmap(((QFontFactoryTTF*)diskfont->factory)->library,&glyph->outline,&mybits);
	    if(err) {
		qWarning("Get bitmap error %d [%dx%d]",err,mybits.pitch,mybits.rows);
	    }
	}

	// Convert from 0-127 values for alpha channel
	// to 0-255
	// Leaving this out makes for very pretty alpha-blended text :)

	if (smooth) {
	    int loopc,loopc2;
	    unsigned char * ptr;
	    for(loopc=0;loopc<height;loopc++) {
		ptr=((unsigned char *)mybits.buffer)+(loopc * pitch);
		for(loopc2=0;loopc2<width;loopc2++) {
		    unsigned char hold=*ptr;
		    hold=hold*2+(hold>>7);
		    *(ptr++)=hold;
		}
	    }
	}

	result.metrics = new QGlyphMetrics;
	result.metrics->bearingx=glyph->metrics.horiBearingX/64;
	result.metrics->advance=glyph->metrics.horiAdvance/64;
	result.metrics->bearingy=glyph->metrics.horiBearingY/64;

	result.metrics->linestep=pitch;
	result.metrics->width=width;
	result.metrics->height=height;

	return result;
    }

    FT_Face myface;
    FT_Size size;
};

QFontFactoryTTF::QFontFactoryTTF()
{
    FT_Error err;
    err=FT_Init_FreeType(&library);
    if(err) {
	qFatal("Couldn't initialise Freetype library");
    }
}

QFontFactoryTTF::~QFontFactoryTTF()
{
}

QString QFontFactoryTTF::name()
{
    return "TTF";
}

QRenderedFont * QFontFactoryTTF::get(const QFontDef & f,QDiskFont * f2)
{
    return new QRenderedFontTTF(f2, f);
}

void QFontFactoryTTF::load(QDiskFont * qdf) const
{
    if(qdf->loaded)
	return;
    QDiskFontTTF *f = new QDiskFontTTF;
    qdf->p=f;
    FT_Error err;
    err=FT_New_Face(library,qdf->file.ascii(),0,&(f->face));
    if(err) {
	qFatal("Error %d opening face",err);
    }
    qdf->loaded=true;
}


#endif // QT_FEATURE_TRUETYPE
