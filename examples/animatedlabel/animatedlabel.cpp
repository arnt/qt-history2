/****************************************************************************
**
** Copyright (C) 2003 Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "animatedlabel.h"
#include <qfile.h>
#include <qevent.h>
#include <qpainter.h>
#include <qapplication.h>

/*

  This example shows how to use the Async Image I/O classes for a simple
  animated label. While a similar result can be achieved by using
  QLabel::setMovie(), this example is more efficient since it stores
  all animation frames and uses the most efficient painting methods.
  It also has the advantage that it can function reasonably without
  an event loop, thus it can be used for "busy" feedback.

  The AnimatedLabel shows an animation file by implementing the
  QImageConsumer interface and feeding file data to a QImageDecoder
  which in turn feeds image data back to the QImageConsumer, which
  this widget then displays.

  Both normal event-based animation and push-based animation is supported.

  For event-based animation, the widget just operates automatically
  as part of normal event processing.

  For push-based animation, you must call pushUpdate() when you want
  the animation to progress) is supported. This will ensure the animation
  displays with the correct frame rate, or slower if not called
  sufficiently often.

*/

AnimatedLabel::AnimatedLabel(const QString& animfile, QWidget* parent, const char* name) :
    QWidget(parent,name)
{
    input = new QFile(animfile);
    input->open(IO_ReadOnly);
    decoder = new QImageDecoder(this);
    tid = startTimer(100);
    loops = -1;
    loop = 0;
    playframe = 0;
    gotframes = 0;
    period = 0;
}

AnimatedLabel::~AnimatedLabel()
{
    delete decoder;
    delete input;
}

/*
  Returns the status of the file. Immediately after construction, this
  should be IO_Ok if everything is fine.
*/
int AnimatedLabel::status() const
{
    return input->status();
}

QSize AnimatedLabel::sizeHint() const
{
    // Ensure we have the size
    const int chunksize=16;
    char buffer[chunksize];
    while (sh.isNull() && input->status()==IO_Ok) {
	int n = input->readBlock(buffer,chunksize);
	if ( n <= 0 )
	    break;
	int r = decoder->decode((uchar*)buffer,n);
	if ( r < n )
	    input->at(input->at()+r-n);
    }

    return sh;
}

/*
  Call this to update the animation even if no
  event loop is running (eg. call it periodically during
  a busy operation to give animated feedback).
*/
void AnimatedLabel::pushUpdate()
{
    if ( timer.elapsed() >= period ) {
	showNextFrame();
	qApp->flush();
	timer.start();
    }
}

void AnimatedLabel::showNextFrame()
{
    if ( playframe ) {
	QPainter p(this);
	p.drawPixmap(*offset.at(playframe-1),*frame.at(playframe-1));
	playframe++;
	if ( playframe > (int)frame.count() )
	    end();
    } else {
	// Keep sending until we get a frame
	const int chunksize=256;
	char buffer[chunksize];
	int fn = gotframes;
	while (fn == gotframes && tid && !playframe) {
	    int n = input->readBlock(buffer,chunksize);
	    if ( n < 0 )
		break;
	    int r = decoder->decode((uchar*)buffer,n);
	    if ( r < 0 ) {
		// Bad anim
		input->close();
		loops = -1;
		end();
		break;
	    }
	    if ( r < n )
		input->at(input->at()+r-n);
	}
    }
}

void AnimatedLabel::timerEvent(QTimerEvent* event)
{
    if ( event->timerId() == tid )
	showNextFrame();
}

// QImageConsumer interface
void AnimatedLabel::end()
{
    if ( loops >= 0 ) {
	if ( !loops || loop++ != loops ) {
	    playframe = 1;
	    input->close();
	    return;
	}
    }

    // Stop
    if ( tid ) {
	killTimer(tid);
	tid = 0;
    }
}

void AnimatedLabel::changed( const QRect& rect )
{
    change |= rect;
}

void AnimatedLabel::frameDone()
{
    gotframes++;
    QPainter p(this);
    QPoint off = change.topLeft();
    if ( loops >= 0 ) {
	// Looping, so store frame
	QPixmap *pm = new QPixmap;
	if ( off == QPoint(0,0) && change.size() == sh ) {
	    pm->convertFromImage(decoder->image());
	} else {
	    pm->convertFromImage(decoder->image().copy(change));
	}
	frame.append(pm);
	offset.append(new QPoint(off));
	p.drawPixmap(off,*pm);
    } else {
	// Draw frame now
	p.drawImage(off,decoder->image(),change);
    }

    change = QRect();
}

void AnimatedLabel::frameDone( const QPoint&, const QRect& )
{
    qWarning("Not supported");
    // Left as an exercise for the reader.
    //
    // Note that Qt's MNG and GIF support do not require
    // this function.
}

void AnimatedLabel::setLooping( int l )
{
    loops = l;
}

void AnimatedLabel::setFramePeriod( int ms )
{
    if ( tid )
	killTimer(tid);
    period = ms;
    tid = startTimer(ms);
    timer.start();
}

void AnimatedLabel::setSize( int w, int h )
{
    bool isnew = sh.isValid() && (sh.width() != w || sh.height() != h);
    sh = QSize(w,h);
    if ( isnew )
	updateGeometry();
}

