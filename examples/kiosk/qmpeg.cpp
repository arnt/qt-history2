/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#define GUI_Qt // define it here too

#include <qpainter.h>
#include "qmpeg.h"
#include "video.h"
#include "proto.h"
#include "dither.h"
#include <stdlib.h>

// global variables that mpeg_play uses
#include "globals.h"
#include "util.h"

class QMpegWidgetPrivate {
public:
    VidStream* stream;

    QMpegWidgetPrivate()
    {
	stream = 0;
	input_local = 0;
	EOF_flag_local = 0;
	curBits_local = 0;
	bitOffset_local = 0;
	bufLength_local = 0;
	seekValue_local = 0;
	bitBuffer_local = NULL;
	curVidStream_local = NULL;
    }
    ~QMpegWidgetPrivate()
    {
	if ( stream )
	    DestroyVidStream( stream );
	if ( input_local )
	    fclose( input_local );
    }

    void open(const QString filename)
    {
	input_local = fopen(filename.local8Bit(), "r");
    }

    
    void begin()
    {
	::input = input_local;
	::EOF_flag = EOF_flag_local;
	::curBits = curBits_local;
	::seekValue = seekValue_local;
	::bitBuffer = bitBuffer_local;
	::bufLength = bufLength_local;
	::bitOffset = bitOffset_local;
	::curVidStream = curVidStream_local;
    }

    void end()
    {
	input_local = ::input;
	EOF_flag_local = ::EOF_flag;
	curBits_local = ::curBits;
	seekValue_local = ::seekValue;
	bitBuffer_local = ::bitBuffer;
	bufLength_local = ::bufLength;
	bitOffset_local = ::bitOffset;
	curVidStream_local = ::curVidStream;
    }

private:
    FILE* input_local;
    int EOF_flag_local;
    int seekValue_local;
    unsigned int *bitBuffer_local;
    int bufLength_local;
    int bitOffset_local;
    unsigned int curBits_local;
    VidStream* curVidStream_local;
};

QMpegWidget::QMpegWidget( QWidget* parent, const char* name ) : 
    QFrame(parent,name),
    d(0)
{
    // Hack mpeg_play global variables
#ifndef GUI_Qt_16BIT
    wpixel[0] = 0xff0000;
    wpixel[1] = 0x00ff00;
    wpixel[2] = 0x0000ff;
    ditherType = FULL_COLOR_DITHER;
    matched_depth = 32;
#else
    wpixel[0] = 0x00f800;
    wpixel[1] = 0x0007e0;
    wpixel[2] = 0x00001f;
    ditherType = FULL_COLOR_DITHER;
    matched_depth = 16;
#endif    
    InitColorDither(TRUE);

    // Initialize mpeg_play global variables
    if ( !lum_values ) {
        lum_values = (int *) malloc(LUM_RANGE*sizeof(int));
        cr_values = (int *) malloc(CR_RANGE*sizeof(int));
        cb_values = (int *) malloc(CB_RANGE*sizeof(int));
	mpeg_global_error = 0;
        init_tables();
    }
}

#define BUF_LENGTH 8000

QSize QMpegWidget::sizeHint() const
{
    QSize s;
    if ( d && d->stream )
	s = QSize( d->stream->h_size+frameWidth()*2, 
		   d->stream->v_size+frameWidth()*2 );
    return s.expandedTo(QSize(30,30));
}


QSizePolicy QMpegWidget::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
}



void QMpegWidget::play(const QString& filename)
{
    if ( d )
	stop();
    d = new QMpegWidgetPrivate;
    d->open(filename);

    d->stream = NewVidStream((unsigned int) BUF_LENGTH);

    d->begin();
    d->stream->view = this;
    d->stream = mpegVidRsrc(0, d->stream);
    d->end();

    int x = (width() -  d->stream->h_size)/2;
    int y = (height() -  d->stream->v_size)/2;
    offset = QPoint(x,y);
    updateGeometry();
    timer=startTimer(0);
    realTimeStart = ReadSysClock(); // need to be global to work
}

bool QMpegWidget::isPlaying() const
{
    return d && d->stream;
}


/*
  This one is slightly more complex than expected, because we want
  to make sure that we don't emit finished(), which would trigger looping.
 */
void QMpegWidget::stop()
{
    if ( timer )
	killTimer(timer); 
    timer=0;
    delete d;
    d = 0;
    emit stopped();
}


void QMpegWidget::timerEvent(QTimerEvent* )
{
    killTimer(timer); timer=0;
    if ( d ) {
	d->begin();
	d->stream = mpegVidRsrc(0, d->stream);
	if ( d->stream && !timer )
	    timer=startTimer(0);
	d->end();
    }
    if ( !d || !d->stream )
	emit finished();
}
  

void QMpegWidget::showFrame(const QImage& image, int ms)
{
    //    qDebug( "%p showFrame()", this );
    QPainter p(this);
    p.drawImage(offset,image);
    if ( !timer )
	timer=startTimer(ms);
}


static QRegion roundRectRegion( const QRect& g, int r )
{
    QPointArray a;
    a.setPoints( 8, g.x()+r, g.y(), g.right()-r, g.y(),
		 g.right(), g.y()+r, g.right(), g.bottom()-r,
		 g.right()-r, g.bottom(), g.x()+r, g.bottom(),
		 g.x(), g.bottom()-r, g.x(), g.y()+r );  
    QRegion reg( a );
    int d = r*2-1;
    reg += QRegion( g.x(),g.y(),r*2,r*2, QRegion::Ellipse );
    reg += QRegion( g.right()-d,g.y(),r*2,r*2, QRegion::Ellipse );
    reg += QRegion( g.x(),g.bottom()-d,r*2,r*2, QRegion::Ellipse );
    reg += QRegion( g.right()-d,g.bottom()-d,r*2,r*2, QRegion::Ellipse );
    return reg;
}



void QMpegWidget::resizeEvent( QResizeEvent*)
{
    if ( d && d->stream ) {
	int x = (width() -  d->stream->h_size)/2;
	int y = (height() -  d->stream->v_size)/2;
	offset = QPoint(x,y);
    }
    setMask( roundRectRegion(rect(), 20) );
}
