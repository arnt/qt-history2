/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qmovie.cpp#54 $
**
** Implementation of movie classes
**
** Created : 970617
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

// #define QT_SAVE_MOVIE_HACK

#include "qtimer.h"
#include "qpainter.h"
#include "qlist.h"
#include "qbitmap.h"
#include "qmovie.h"
#include "qfile.h"
#include "qbuffer.h"
#include "qshared.h"

#include "qasyncio.h"
#include "qasyncimageio.h"

#include <stdlib.h>

/*!
  \class QMovie qmovie.h
  \brief Incrementally loads an animation or image, signalling as it progresses.

  \ingroup images

  A QMovie provides a QPixmap as the framePixmap(), and connections can
  be made via connectResize() and connectUpdate() to receive notification
  of size and pixmap changes.  All decoding is driven by
  the normal event processing mechanisms.  The simplest way to display
  a QMovie, is to use a QLabel and QLabel::setMovie().

  The movie begins playing as soon as the QMovie is created (actually,
  once control returns to the event loop).  When the last frame in the
  movie has been played, it may loop back to the start if such looping
  is defined in the input source.

  QMovie objects are explicitly shared.  This means that a QMovie copied
  from another QMovie will be displaying the same frame at all times.
  If one shared movie pauses, all pause.  To make \e independent movies,
  they must be constructed separately.

  The set of data formats supported by QMovie is determined by the decoder
  factories which have been installed, and the format of the input is
  determined as the input is decoded.

  In Qt 1.30, the decoder factory interface is not available for adding
  support for new formats. Only GIF support is installed.  The GIF decoder
  supports interlaced images, transparency, looping, image-restore
  disposal, local color maps, and background colors.  The Netscape looping
  extension is obeyed.

  Archives of animated GIFs and tools for building them can be found
  at <a href=http://www.yahoo.com/Arts/Visual_Arts/Animation/Computer_Animation/Animated_GIFs/>
  Yahoo!</a>.

  We are required to state: The Graphics Interchange Format(c) is the
  Copyright property of CompuServe Incorporated. GIF(sm) is a Service
  Mark property of CompuServe Incorporated.

  \warning Unisys has changed its position regarding GIF.  If you are
  in a country where Unisys holds a patent on LZW compression and/or
  decompression, Unisys may require a license from you.  These
  countries include Canada, Japan, the USA, France, Germany, Italy and
  the UK.  There is more information on Unisys web site: <a
  href="http://corp2.unisys.com/LeadStory/lzwfaq.html">Overview of
  Unisys' position.</a> GIF support may be removed in a future version
  of Qt.  We recommend using the PNG format, which is available in the
  <a href="imageio.html">Qt Image IO Extension</a> package.

  <img src="qmovie.gif">

  \sa QLabel::setMovie()
*/

class QMovieFilePrivate : public QObject, public QShared,
		      private QDataSink, private QImageConsumer
{
    Q_OBJECT

public: // for QMovie

    // Creates a null QMovieFilePrivate
    QMovieFilePrivate();

    // NOTE:  The ownership of the QDataSource is transferred to the QMovieFilePrivate
    QMovieFilePrivate(QDataSource* src, QMovie* movie, int bufsize);

    virtual ~QMovieFilePrivate();

    bool isNull() const;

    // Initialize, possibly to the null state
    void init(bool fully);
    void flushBuffer();
    void updatePixmapFromImage();
    void showChanges();

    // This as QImageConsumer
    void changed(const QRect& rect);
    void end();
    void frameDone();
    void restartTimer();
    void setLooping(int l);
    void setFramePeriod(int milliseconds);
    void setSize(int w, int h);

    // This as QDataSink
    int readyToReceive();
    void receive(const uchar* b, int bytecount);
    void eof();
    void pause();

signals:
    void sizeChanged(const QSize&);
    void areaChanged(const QRect&);
    void dataStatus(int);

public slots:
    void refresh();

public:
    QMovie *that;

    QImageDecoder *decoder;

    // Cyclic buffer
    int buf_size;
    uchar *buffer;
    int buf_r, buf_w, buf_usage;

    int framenumber;
    int frameperiod;
    int speed;
    QTimer *frametimer;
    int lasttimerinterval;
    int loop;

    bool waitingForFrameTick;
    int stepping;
    QRect changed_area;
    QRect valid_area;
    QDataPump *pump;
    QDataSource *source;
    QPixmap mypixmap;
    QBitmap mymask;
    QColor bg;

    int error;
    bool empty;

#ifdef QT_SAVE_MOVIE_HACK
  bool save_image;
  int image_number;
#endif
};

class QMovieFramePrivate : public QObject, public QShared
{
    Q_OBJECT
public:
    QMovieFramePrivate();
    QMovieFramePrivate( QList<QMovieFrame> &frames );
    virtual ~QMovieFramePrivate();

    void Init();

    bool isNull() const;

    int steps();
    void step( int );
    int frameNumber();
    bool paused();
    void pause();
    void unpause();
    bool finished();
    bool running();
    void setSpeed( int );
    bool stepping();
    void restart();

    void restartTimer();

signals:
    void sizeChanged(const QSize&);
    void areaChanged(const QRect&);
    void dataStatus(int);

public slots:
    void refresh();

public:
    QTimer* frametimer;
    QPixmap mypixmap;
    QBitmap mymask;
    QList<QMovieFrame> frames;
    QColor bg;
    int frameperiod;
    int lasttimerinterval;
    QRect valid_area;
    bool is_paused;
    unsigned int step_nr;
    int speed;
    bool is_stepping;
};

/************************************************************
 * QMovieFramePrivate
 ************************************************************/

QMovieFramePrivate::QMovieFramePrivate()
{
    Init();
}

QMovieFramePrivate::QMovieFramePrivate( QList<QMovieFrame> &_frames )
{
    Init();
    
    frames = _frames;
    
    if ( frames.count() == 0 )
    {
      emit dataStatus( QMovie::SourceEmpty );
      return;
    }

    // Create the timer
    frametimer = new QTimer(this);
    QObject::connect(frametimer, SIGNAL(timeout()), this, SLOT(refresh()));

    frameperiod = frames.first()->timeOffset();
    lasttimerinterval = -1;
    restartTimer();
}

QMovieFramePrivate::~QMovieFramePrivate()
{
    if ( frametimer )
      delete frametimer;
}

void QMovieFramePrivate::Init()
{
    frametimer = 0;
    is_paused = false;
    step_nr = 0;
    speed = 100;
    is_stepping = false;
}

bool QMovieFramePrivate::isNull() const
{
    // No frames ?
    return ( frames.count() == 0 );
}

void QMovieFramePrivate::restartTimer()
{
    if (speed > 0)
    {
      int i = frameperiod >= 0 ? frameperiod * 100/speed : 0;
      // Correct the timer interval and reactivate the timer
      if ( i != lasttimerinterval || !frametimer->isActive() )
      {
	lasttimerinterval = i;
	frametimer->start( i );
      }
    }
    // Speed of 0 => No animation at all.
    else
	frametimer->stop();
}

void QMovieFramePrivate::refresh()
{
  // Just make shure that there is at least one frame.
  if ( frames.count() == 0 )
  {
    frametimer->stop();
    return;
  }

  // Tell us what we have to update.
  QMovieFrame *f = frames.at(step_nr);
  int dx = f->xOffset();
  int dy = f->yOffset();
  int w = f->pixmap().width();
  int h = f->pixmap().height();

  // Calculate the valid area
  QRect r( dx, dy, w, h );
  if ( step_nr == 0 )
    valid_area = r;
  else
    valid_area = valid_area.unite( r );

  // Do we have to care about setting the background ?
  if ( bg.isValid())
  {
    QPainter p;
    p.begin(&mypixmap);
    p.fillRect(dx, dy, w, h, bg);
    p.end();
  }
  else
  {
    if ( f->pixmap().mask())
    {
      // Create a mask since we dont have one right now
      if ( mymask.isNull() )
      {
	mymask.resize( dx + w - 1, dy + h - 1 );
	emit sizeChanged( QSize( dx + w - 1, dy + h - 1 ) );
	mymask.fill( Qt::color1 );
      }
    }
    // Ok, a mask is required.
    mypixmap.setMask(QBitmap()); // Remove reference to my mask
  }

  // Is this the beginning of a loop ?
  if ( step_nr == 0 )
    mypixmap = QPixmap();

  // Do we need to resize ?
  if ( dx + w - 1 > mypixmap.width() || dy + h - 1 > mypixmap.height() )
  {
    mypixmap.resize( dx + w - 1 > mypixmap.width() ? dx + w - 1 : mypixmap.width(),
		     dy + h - 1 > mypixmap.height() ? dy + h - 1 : mypixmap.height() );
    emit sizeChanged( QSize( mypixmap.width(), mypixmap.height() ) );
  }

  // Now display the new pixmap
  bitBlt(&mypixmap, dx, dy, &f->pixmap(), 0, 0, w, h, CopyROP, !bg.isValid());

  if (!bg.isValid() && f->pixmap().mask())
  {
    bitBlt(&mymask, dx, dy, f->pixmap().mask(), 0, 0, w, h, CopyROP, TRUE);
    mypixmap.setMask(mymask);
  }
  
  emit areaChanged( QRect( dx, dy, w, h ) );

  // Check for end of loop
  step_nr++;
  if ( step_nr == frames.count() )
  {
    emit dataStatus( QMovie::EndOfLoop );
    step_nr = 0;
  }

  // How long to wait for the next one ?
  frameperiod = frames.at( step_nr )->timeOffset();
  if ( frameperiod == 0 )
  {
    lasttimerinterval = 0;
    refresh();
  }
  else
    restartTimer();
}

int QMovieFramePrivate::steps()
{
}

int QMovieFramePrivate::frameNumber()
{
}

bool QMovieFramePrivate::paused()
{
  return is_paused;
}

void QMovieFramePrivate::pause()
{
  if ( paused )
    return;

  is_paused = true;
  if ( frametimer )
    frametimer->stop();
}

void QMovieFramePrivate::unpause()
{
  if ( !paused )
    return;

  restartTimer();
}

bool QMovieFramePrivate::finished()
{
  if ( !frametimer )
    return true;
  if ( frametimer->isActive() )
    return false;
  return true;
}

bool QMovieFramePrivate::stepping()
{
}

bool QMovieFramePrivate::running()
{
  return ( frametimer && frametimer->isActive() );
}

void QMovieFramePrivate::setSpeed( int percent )
{
    int oldspeed = speed;

    if ( oldspeed != percent && percent >= 0 )
      speed = percent;
    
    if ( !running() || stepping() )
      return;

    // Restart timer only if really needed
    if ( !percent || !oldspeed    // To or from zero
	 || oldspeed*4 / percent > 4   // More than 20% slower
	 || percent*4 / oldspeed > 4   // More than 20% faster
	 )
      restartTimer();
}

void QMovieFramePrivate::step( int _steps )
{
}

void QMovieFramePrivate::restart()
{
}
    
/************************************************************
 * QMovieFilePrivate
 ************************************************************/

QMovieFilePrivate::QMovieFilePrivate()
{
    buffer = 0;
    pump = 0;
    source = 0;
    decoder = 0;
    init(FALSE);
}

// NOTE:  The ownership of the QDataSource is transferred to the QMovieFilePrivate
QMovieFilePrivate::QMovieFilePrivate(QDataSource* src, QMovie* movie, int bufsize) :
    that(movie),
    buf_size(bufsize)
{
    frametimer = new QTimer(this);
    pump = new QDataPump(src, this);
    QObject::connect(frametimer, SIGNAL(timeout()), this, SLOT(refresh()));
    source = src;
    buffer = 0;
    decoder = 0;
    speed = 100;
    init(TRUE);
}

QMovieFilePrivate::~QMovieFilePrivate()
{
    if ( buffer )				// Avoid purify complaint
	delete [] buffer;
    delete pump;
    delete decoder;
    delete source;
}

bool QMovieFilePrivate::isNull() const
{
    return !pump;
}

// Initialize.  Only actually allocate any space if \a fully is TRUE,
// otherwise, just enough to be a valid null QMovieFilePrivate.
void QMovieFilePrivate::init(bool fully)
{
#ifdef QT_SAVE_MOVIE_HACK
    save_image = true;
    image_number = 0;
#endif

    buf_usage = buf_r = buf_w = 0;
    if ( buffer )				// Avoid purify complaint
	delete [] buffer;
    buffer = fully ? new uchar[buf_size] : 0;

    delete decoder;
    decoder = fully ? new QImageDecoder(this) : 0;

#ifdef AVOID_OPEN_FDS
    if ( source && !source->isOpen() )
	source->open(IO_ReadOnly);
#endif

    waitingForFrameTick = FALSE;
    stepping = -1;
    framenumber = 0;
    frameperiod = -1;
    if (fully) frametimer->stop();
    lasttimerinterval = -1;
    changed_area.setRect(0,0,-1,-1);
    valid_area = changed_area;
    loop = -1;
    error = 0;
    empty = TRUE;
}

void QMovieFilePrivate::flushBuffer()
{
    while (buf_usage && !waitingForFrameTick && stepping != 0 && !error) {
	int used = decoder->decode(buffer + buf_r,
			QMIN(buf_usage, buf_size - buf_r));
	if (used<=0) {
	    error = 1;
	    emit dataStatus(QMovie::UnrecognizedFormat);
	    break;
	}
	buf_r = (buf_r + used)%buf_size;
	buf_usage -= used;
    }
    if(error) frametimer->stop();
    maybeReady();
}

void QMovieFilePrivate::updatePixmapFromImage()
{
    if (changed_area.isEmpty()) return;

    // Create temporary QImage to hold the part we want
    const QImage& gimg = decoder->image();
    QImage img(changed_area.size(), 8, gimg.numColors());
    img.setAlphaBuffer(gimg.hasAlphaBuffer());

#ifdef QT_SAVE_MOVIE_HACK
    if ( save_image )
    {
      QString name;
      name.sprintf("movie%i.ppm",image_number++);
      gimg.save( name, "PPM" );
    }
#endif

    // Copy color map.
    memcpy(img.colorTable(), gimg.colorTable(),
	sizeof(QRgb)*gimg.numColors());

    // Copy pixels.
    const int t = changed_area.top();
    const int b = changed_area.bottom();
    const int l = changed_area.left();
    const int w = changed_area.width();
    const int h = changed_area.height();
    for (int y=t; y<=b; y++) {
	memcpy(img.scanLine(y-t), gimg.scanLine(y)+l, w);
    }

    // Resize to size of image
    if (mypixmap.width() != gimg.width() || mypixmap.height() != gimg.height())
	mypixmap.resize(gimg.width(), gimg.height());

    if (bg.isValid()) {
	QPainter p;
	p.begin(&mypixmap);
	p.fillRect(l, t, w, h, bg);
	p.end();
    } else {
	if (gimg.hasAlphaBuffer()) {
	    // Resize to size of image
	    if (mymask.isNull()) {
		mymask.resize(gimg.width(), gimg.height());
		mymask.fill( Qt::color1 );
	    }
	}
	mypixmap.setMask(QBitmap()); // Remove reference to my mask
    }

    // Convert to pixmap and paste that onto myself
    QPixmap lines;
    lines.convertFromImage(img);
    bitBlt(&mypixmap, l, t, &lines, 0, 0, w, h, CopyROP, !bg.isValid()); 

    if (!bg.isValid() && gimg.hasAlphaBuffer()) {
	bitBlt(&mymask, l, t, lines.mask(), 0, 0, w, h, CopyROP, TRUE);
	mypixmap.setMask(mymask);
    }
}

void QMovieFilePrivate::showChanges()
{
    if (changed_area.isValid()) {
	updatePixmapFromImage();

	valid_area = valid_area.unite(changed_area);
	emit areaChanged(changed_area);

	changed_area.setWidth(-1); // make empty
    }
}

// QMovieFilePrivate as QImageConsumer
void QMovieFilePrivate::changed(const QRect& rect)
{
    if (!frametimer->isActive()) frametimer->start(0);
    changed_area = changed_area.unite(rect);
}

void QMovieFilePrivate::end()
{
}

void QMovieFilePrivate::frameDone()
{
    if (stepping > 0) {
	stepping--;
	if (!stepping) {
	    frametimer->stop();
	    emit dataStatus( QMovie::Paused );
	}
    } else {
	waitingForFrameTick = TRUE;
	restartTimer();
    }
    showChanges();
    emit dataStatus(QMovie::EndOfFrame);
    framenumber++;
}

void QMovieFilePrivate::restartTimer()
{
    if (speed > 0) {
	int i = frameperiod >= 0 ? frameperiod * 100/speed : 0;
	if ( i != lasttimerinterval || !frametimer->isActive() ) {
	    lasttimerinterval = i;
	    frametimer->start( i );
	}
    } else {
	frametimer->stop();
    }
}

void QMovieFilePrivate::setLooping(int nloops)
{
    if (loop == -1) { // Only if we don't already know how many loops!
	if (source->rewindable()) {
	    source->enableRewind(TRUE);
	    loop = nloops;
	} else {
	    // Cannot loop from this source
	    loop = -2;
	}
    }
}

void QMovieFilePrivate::setFramePeriod(int milliseconds)
{
    // Animation:  only show complete frame
    frameperiod = milliseconds;
    if (stepping<0 && frameperiod >= 0) restartTimer();
}

void QMovieFilePrivate::setSize(int w, int h)
{
    if (mypixmap.width() != w || mypixmap.height() != h) {
	mypixmap.resize(w, h);
	emit sizeChanged(QSize(w, h));
    }
}


// QMovieFilePrivate as QDataSink

int QMovieFilePrivate::readyToReceive()
{
    // Could pre-fill buffer, but more efficient to just leave the
    // data back at the source.
    return (waitingForFrameTick || !stepping || buf_usage || error)
	? 0 : buf_size;
}

void QMovieFilePrivate::receive(const uchar* b, int bytecount)
{
    if ( bytecount ) empty = FALSE;

    while (bytecount && !waitingForFrameTick && stepping != 0) {
	int used = decoder->decode(b, bytecount);
	if (used<=0) {
	    error = 1;
	    emit dataStatus(QMovie::UnrecognizedFormat);
	    break;
	}
	b+=used;
	bytecount-=used;
    }

    // Append unused to buffer
    while (bytecount--) {
	buffer[buf_w] = *b++;
	buf_w = (buf_w+1)%buf_size;
	buf_usage++;
    }
}

void QMovieFilePrivate::eof()
{
    if ( empty )
	emit dataStatus(QMovie::SourceEmpty);

#ifdef QT_SAVE_MOVIE_HACK
    save_image = false;
#endif

    emit dataStatus(QMovie::EndOfLoop);

    if (loop >= 0) {
	if (loop) {
	    loop--;
	    if (!loop) return;
	}
	delete decoder;
	decoder = new QImageDecoder(this);
	source->rewind();
	framenumber = 0;
    } else {
	delete decoder;
	decoder = 0;
	if ( buffer )				// Avoid purify complaint
	    delete [] buffer;
	buffer = 0;
	emit dataStatus(QMovie::EndOfMovie);
#ifdef AVOID_OPEN_FDS
	if ( source )
	    source->close();
#endif
    }
}

void QMovieFilePrivate::pause()
{
    if ( stepping ) {
	stepping = 0;
	frametimer->stop();
	emit dataStatus( QMovie::Paused );
    }
}

void QMovieFilePrivate::refresh()
{
    if (!decoder) {
	frametimer->stop();
	return;
    }

    if (frameperiod < 0 && loop == -1) {
	// Only show changes if probably not an animation
	showChanges();
    }

    if (!buf_usage) {
	frametimer->stop();
    }

    waitingForFrameTick = FALSE;
    flushBuffer();
}

///////////////// End of QMovieFilePrivate /////////////////





/*!
  Creates a null QMovie.  The only interesting thing to do to such
  a movie is to assign another movie to it.

  \sa isNull()
*/
QMovie::QMovie()
{
    f = 0;
    d = new QMovieFilePrivate();
}

/*!
  Creates a QMovie which reads an image sequence from the given
  QDataSource.  The source must be allocated dynamically,
  as it becomes owned by the QMovie, and will be destroyed
  when the movie is destroyed.
  The movie starts playing as soon as event processing continues.

  The \a bufsize argument sets the maximum amount of data the movie
  will transfer from the data source per event loop.  The lower this
  value, the better interleaved the movie playback will be with other
  event processing, but the slower the overall processing.
*/
QMovie::QMovie(QDataSource* src, int bufsize)
{
    f = 0;
    d = new QMovieFilePrivate(src, this, bufsize);
}

/*!
  Creates a QMovie which reads an image sequence from the named file.
*/
QMovie::QMovie(const QString &fileName, int bufsize)
{
    f = 0;
    QFile* file = new QFile(fileName);
    file->open(IO_ReadOnly);
    d = new QMovieFilePrivate(new QIODeviceSource(file), this, bufsize);
}

/*!
  Creates a QMovie which reads an image sequence from given data.
*/
QMovie::QMovie(QByteArray data, int bufsize)
{
    f = 0;
    QBuffer* buffer = new QBuffer(data);
    buffer->open(IO_ReadOnly);
    d = new QMovieFilePrivate(new QIODeviceSource(buffer), this, bufsize);
}

/*!
  Constructs a movie that uses the same data as another movie.
  QMovies use explicit sharing, so operations on the copy will
  effect the same operations on the original.
*/
QMovie::QMovie(const QMovie& movie)
{
    d = movie.d;
    f = movie.f;
    if ( d )
      d->ref();
    if ( f )
      f->ref();
}

/*!
  Destroys the QMovie.  If this is the last reference to the data of the
  movie, that will also be destroyed.
*/
QMovie::~QMovie()
{
    if (d && d->deref()) delete d;
    if (f && f->deref()) delete f;
}

QMovie::QMovie(QList<QMovieFrame>& frames)
{
    d = 0;
    f = new QMovieFramePrivate( frames );
}

/*!
  Returns TRUE if the movie is null.
*/
bool QMovie::isNull() const
{
    if ( f )
      return f->isNull();
    ASSERT( d );
    return f->isNull();
}

/*!
  Makes this movie use the same data as another movie.
  QMovies use explicit sharing.
*/
QMovie& QMovie::operator=(const QMovie& movie)
{
    if ( movie.d )
      movie.d->ref();
    if ( movie.f )
      movie.f->ref();
    if (d && d->deref()) delete d;
    if (f && f->deref()) delete f;
    d = movie.d;
    f = movie.f;
    return *this;
}


/*!
  Set the background color of the pixmap.  If the background color
  isValid(), the pixmap will never have a mask, as the background
  color will be used in transparent regions of the image.

  \sa backgroundColor()
*/
void QMovie::setBackgroundColor(const QColor& c)
{
    if ( d )
      d->bg = c;
    else if ( f )
      f->bg = c;
    else
      ASSERT( 0 );
}

/*!
  Returns the background color of the movie set by setBackgroundColor().
*/
const QColor& QMovie::backgroundColor() const
{
    if ( d )
      return d->bg;
    else if ( f )
      return f->bg;
    else
      ASSERT( 0 );
}

/*!
  Returns the area of the pixmap for which pixels have been generated.
*/
const QRect& QMovie::getValidRect() const
{
    if ( d )
      return d->valid_area;
    else if ( f )
      return f->valid_area;
    else
      ASSERT( 0 );
}

/*!
  Returns the current frame of the movie.  It is not generally useful to
  keep a copy of this pixmap.  Better to keep a copy of the QMovie and
  get the framePixmap() only when needed for drawing.
*/
const QPixmap& QMovie::framePixmap() const
{
    if ( d )
      return d->mypixmap;
    else if ( f )
      return f->mypixmap;
    else
      ASSERT( 0 );
}

/*!
  Returns the number of steps remaining after a call to step(), 0 if paused,
  or a negative value if the movie is running normally or is finished.
*/
int QMovie::steps() const
{
    if ( d )
      return d->stepping;
    else if ( f )
      return f->steps();
    else
      ASSERT( 0 );
}

/*!
  Returns the number of times EndOfFrame has been emitted since the
  start of the current loop of the movie.  Thus, before
  any EndOfFrame has been emitted, the value will be 0,
  within slots processing the first signal, frameNumber() will be 1, and
  so on.
*/
int QMovie::frameNumber() const
{
    if ( d )
      return d->framenumber;
    else if ( f )
      return f->frameNumber();
}

/*!
  Returns TRUE if the image is paused.
*/
bool QMovie::paused() const
{
    if ( d )
      return (d->stepping == 0);
    else if ( f )
      return f->paused();
    else
      ASSERT( 0 );
}

/*!
  Returns TRUE if the image is no longer playing - this happens when all
  loops of all frames is complete.
*/
bool QMovie::finished() const
{
    if ( d )
      return !d->decoder;
    else if ( f )
      return f->finished();
    else
      ASSERT( 0 );
}

/*!
  Returns TRUE if the image is not single-stepping, not paused,
  and not finished.
*/
bool QMovie::running() const
{
    if ( d )
      return d->stepping<0 && d->decoder;
    ASSERT( f );
    return f->running();
}

/*!
  Pauses the progress of the animation.

  \sa unpause()
*/
void QMovie::pause()
{
    if ( d )
      d->pause();
    else if ( f )
      f->pause();
    else
      ASSERT( 0 );
}

/*!
  Unpauses the progress of the animation.

  \sa pause()
*/
void QMovie::unpause()
{
    if ( f )
      f->unpause();
    else if ( d && d->stepping >= 0 )
    {
      if (d->isNull()) return;
      d->stepping = -1;
      d->restartTimer();
    }
}

/*!
  Steps forward, showing the given number of frames, then pauses.
*/
void QMovie::step(int steps)
{
    if ( f )
    {
      f->step( steps );
    }
    else if ( d )
    {
      if (d->isNull()) return;

      d->stepping = steps;
      d->frametimer->start(0);
      d->waitingForFrameTick = FALSE; // Full speed ahead!
    }
    else
      ASSERT( 0 );
}

/*!
  Steps forward 1 frame, then pauses.
*/
void QMovie::step()
{
    step(1);
}

/*!
  Rewinds the movie to the beginning.  If the movie has not been paused,
  it begins playing again.
*/
void QMovie::restart()
{
    if ( f )
    {
      f->restart();
    }
    else if ( d )
    {
      if (d->isNull()) return;
      
      if (d->source->rewindable())
      {
	d->source->enableRewind(TRUE);
	d->source->rewind();
	int s = d->stepping;
	d->init(TRUE);
	if ( !s ) s = 1; // Don't pause or we'll not get to the FIRST frame
	if (s>0) step(s);
      }
    }
    else
      ASSERT( 0 );
}

/*!
  Returns the speed-up factor of the movie.  The default is 100 percent.
  \sa setSpeed()
*/
int QMovie::speed() const
{
    if ( d )
      return d->speed;
    ASSERT( f );
    return f->speed;
}

/*!
  Sets the speed-up factor of the movie.  This is a percentage of the
  speed dictated by the input data format.  The default is 100 percent.
*/
void QMovie::setSpeed(int percent)
{
    if ( f )
    {
      f->setSpeed( percent );
    }
    else if ( d )
    {
      int oldspeed = d->speed;

      if ( oldspeed != percent && percent >= 0 ) {
	d->speed = percent;

	// Restart timer only if really needed
	if (d->stepping < 0) {
	  if ( !percent || !oldspeed    // To or from zero
	       || oldspeed*4 / percent > 4   // More than 20% slower
	       || percent*4 / oldspeed > 4   // More than 20% faster
	       )
	    d->restartTimer();
	}
      }
    }
    else
      ASSERT( 0 );
}

/*!
  Connects the given member, of type \code void member(const QSize&) \endcode
  such that it is signalled when the movie changes size.

  Note that due to the explicit sharing of QMovie objects, these connections
  persist until they are explicitly disconnected with disconnectResize(), or
  until \e every shared copy of the movie is deleted.
*/
void QMovie::connectResize(QObject* receiver, const char *member)
{
    if ( d )
      QObject::connect(d, SIGNAL(sizeChanged(const QSize&)), receiver, member);
    else if ( f )
      QObject::connect(f, SIGNAL(sizeChanged(const QSize&)), receiver, member);
    else
      ASSERT( 0 );
}

/*!
  Disconnects the given member, or all members if member is zero,
  previously connected by
  connectResize().
*/
void QMovie::disconnectResize(QObject* receiver, const char *member)
{
    if ( d )
      QObject::disconnect(d, SIGNAL(sizeChanged(const QSize&)), receiver, member);
    else if ( f )
      QObject::disconnect(f, SIGNAL(sizeChanged(const QSize&)), receiver, member);
    else
      ASSERT( 0 );
}

/*!
  Connects the given member, of type \code void member(const QRect&) \endcode
  such that it is signalled when an area of the framePixmap() has
  changed since the previous frame.

  Note that due to the explicit sharing of QMovie objects, these connections
  persist until they are explicitly disconnected with disconnectUpdate(), or
  until \e every shared copy of the movie is deleted.
*/
void QMovie::connectUpdate(QObject* receiver, const char *member)
{
    if ( d )
      QObject::connect(d, SIGNAL(areaChanged(const QRect&)), receiver, member);
    else if ( f )
      QObject::connect(f, SIGNAL(areaChanged(const QRect&)), receiver, member);
    else
      ASSERT( 0 );
}

/*!
  Disconnects the given member, or all members if member is zero,
  previously connected by
  connectUpdate().
*/
void QMovie::disconnectUpdate(QObject* receiver, const char *member)
{
    if ( d )
      QObject::disconnect(d, SIGNAL(areaChanged(const QRect&)), receiver, member);
    else if ( f )
      QObject::disconnect(f, SIGNAL(areaChanged(const QRect&)), receiver, member);
    else
      ASSERT( 0 );
}

/*!
  Connects the given member, of type \code void member(int) \endcode
  such that it is signalled when the movie changes status.  The status
  code are negative for errors and positive for information, and they
  are currently:

  <ul>
   <li> \c QMovie::SourceEmpty - signalled if the input cannot be read.
   <li> \c QMovie::UnrecognizedFormat - signalled if the input data is unrecognized.
   <li> \c QMovie::Paused - signalled when the movie is paused by a call to paused(),
			or by after \link step() stepping \endlink pauses.
   <li> \c QMovie::EndOfFrame - signalled at end-of-frame, after any update and Paused signals.
   <li> \c QMovie::EndOfLoop - signalled at end-of-loop, after any update signals,
				EndOfFrame, but before EndOfMovie.
   <li> \c QMovie::EndOfMovie - signalled when the movie completes and is not about
				 to loop.
  </ul>

  More status messages may be added in the future, so a general test for
  error would test for negative.

  Note that due to the explicit sharing of QMovie objects, these connections
  persist until they are explicitly disconnected with disconnectStatus(), or
  until \e every shared copy of the movie is deleted.
*/
void QMovie::connectStatus(QObject* receiver, const char *member)
{
    if ( d )
      QObject::connect(d, SIGNAL(dataStatus(int)), receiver, member);
    else if ( f )
      QObject::connect(f, SIGNAL(dataStatus(int)), receiver, member);
    else
      ASSERT( 0 );
}

/*!
  Disconnects the given member, or all members if member is zero,
  previously connected by
  connectStatus().
*/
void QMovie::disconnectStatus(QObject* receiver, const char *member)
{
    if ( d )
      QObject::disconnect(d, SIGNAL(dataStatus(int)), receiver, member);
    else if ( f )
      QObject::disconnect(f, SIGNAL(dataStatus(int)), receiver, member);
    else
      ASSERT( 0 );
}


QMovieFrame::QMovieFrame()
{
  x_offset = 0;
  y_offset = 0;
  time_offset = 0;
}

QMovieFrame::QMovieFrame( const QPixmap& pix, int dx, int dy, int dtime )
{
  x_offset = dx;
  y_offset = dy;
  time_offset = dtime;
  mypixmap = pix;
}

void QMovieFrame::set( const QPixmap& pix, int dx, int dy, int dtime )
{
  x_offset = dx;
  y_offset = dy;
  time_offset = dtime;
  mypixmap = pix;
}

bool QMovieFrame::operator==( const QMovieFrame& )
{
  return false;
}

bool QMovieFrame::operator<( const QMovieFrame& _frame )
{
  return ( ( y_offset < _frame.yOffset() ) ? true : false );
}

const QPixmap& QMovieFrame::pixmap() const
{
  return mypixmap;
}

int QMovieFrame::xOffset() const
{
  return x_offset;
}

int QMovieFrame::yOffset() const
{
  return y_offset;
}

int QMovieFrame::timeOffset() const
{
  return time_offset;
}

QDataStream& operator>>(QDataStream& str, QMovieFrame& frame)
{
  Q_INT32 x, y, t;
  QPixmap p;
  str >> x >> y >> t >> p;
  frame.set( p, x, y, t );

  return str;
}

QDataStream& operator>>(QDataStream& str, QMovieFrames& frames)
{
  frames.clear();
  frames.setAutoDelete( true );

  Q_INT32 count;
  str >> count;

  for( int i = 0; i < count; i++ )
  {
    Q_INT32 x, y, t;
    QPixmap p;
    str >> x >> y >> t >> p;

    QMovieFrame* f = new QMovieFrame(p,x,y,t);
    // str >> *f;
    frames.append( f );
  }

  return str;
}

QDataStream& operator<<(QDataStream& str, QMovieFrame& frame)
{
  str << (Q_INT32)frame.xOffset() << (Q_INT32)frame.yOffset()
      << (Q_INT32)frame.timeOffset() << frame.pixmap();

  return str;
}

QDataStream& operator<<(QDataStream& str, QMovieFrames& frames)
{
  str << (Q_INT32)frames.count();

  QMovieFrame* f;
  for( f = frames.first(); f != 0; f = frames.next() )
  {
    str << *f;
  }

  return str;
}

/* tmake ignore Q_OBJECT */

//       MANUALLY INCLUDED.  Regenerate in vi with:   !Gmoc %
/****************************************************************************
** QMovieFilePrivate meta object code from reading C++ file 'standard input'
**
** Created: Fri Aug 21 01:55:09 1998
**      by: The Qt Meta Object Compiler ($Revision: 1.54 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 3
#elif Q_MOC_OUTPUT_REVISION != 3
#error "Moc format conflict - please regenerate all moc files"
#endif

#include <qmetaobject.h>


const char *QMovieFilePrivate::className() const
{
    return "QMovieFilePrivate";
}

QMetaObject *QMovieFilePrivate::metaObj = 0;

void QMovieFilePrivate::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QObject::className(), "QObject") != 0 )
	badSuperclassWarning("QMovieFilePrivate","QObject");
    if ( !QObject::metaObject() )
	QObject::initMetaObject();
    typedef void(QMovieFilePrivate::*m1_t0)();
    m1_t0 v1_0 = &QMovieFilePrivate::refresh;
    QMetaData *slot_tbl = new QMetaData[1];
    slot_tbl[0].name = "refresh()";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    typedef void(QMovieFilePrivate::*m2_t0)(const QSize&);
    typedef void(QMovieFilePrivate::*m2_t1)(const QRect&);
    typedef void(QMovieFilePrivate::*m2_t2)(int);
    m2_t0 v2_0 = &QMovieFilePrivate::sizeChanged;
    m2_t1 v2_1 = &QMovieFilePrivate::areaChanged;
    m2_t2 v2_2 = &QMovieFilePrivate::dataStatus;
    QMetaData *signal_tbl = new QMetaData[3];
    signal_tbl[0].name = "sizeChanged(const QSize&)";
    signal_tbl[1].name = "areaChanged(const QRect&)";
    signal_tbl[2].name = "dataStatus(int)";
    signal_tbl[0].ptr = *((QMember*)&v2_0);
    signal_tbl[1].ptr = *((QMember*)&v2_1);
    signal_tbl[2].ptr = *((QMember*)&v2_2);
    metaObj = new QMetaObject( "QMovieFilePrivate", "QObject",
	slot_tbl, 1,
	signal_tbl, 3 );
}

#include "qsignalslotimp.h"

// SIGNAL sizeChanged
void QMovieFilePrivate::sizeChanged( const QSize& t0 )
{
    QConnectionList *clist = receivers("sizeChanged(const QSize&)");
    if ( !clist || signalsBlocked() )
	return;
    typedef void (QObject::*RT0)();
    typedef RT0 *PRT0;
    typedef void (QObject::*RT1)(const QSize&);
    typedef RT1 *PRT1;
    RT0 r0;
    RT1 r1;
    QConnectionListIt it(*clist);
    QConnection   *c;
    QSenderObject *object;
    while ( (c=it.current()) ) {
	++it;
	object = (QSenderObject*)c->object();
	object->setSender( this );
	switch ( c->numArgs() ) {
	    case 0:
		r0 = *((PRT0)(c->member()));
		(object->*r0)();
		break;
	    case 1:
		r1 = *((PRT1)(c->member()));
		(object->*r1)(t0);
		break;
	}
    }
}

// SIGNAL areaChanged
void QMovieFilePrivate::areaChanged( const QRect& t0 )
{
    QConnectionList *clist = receivers("areaChanged(const QRect&)");
    if ( !clist || signalsBlocked() )
	return;
    typedef void (QObject::*RT0)();
    typedef RT0 *PRT0;
    typedef void (QObject::*RT1)(const QRect&);
    typedef RT1 *PRT1;
    RT0 r0;
    RT1 r1;
    QConnectionListIt it(*clist);
    QConnection   *c;
    QSenderObject *object;
    while ( (c=it.current()) ) {
	++it;
	object = (QSenderObject*)c->object();
	object->setSender( this );
	switch ( c->numArgs() ) {
	    case 0:
		r0 = *((PRT0)(c->member()));
		(object->*r0)();
		break;
	    case 1:
		r1 = *((PRT1)(c->member()));
		(object->*r1)(t0);
		break;
	}
    }
}

// SIGNAL dataStatus
void QMovieFilePrivate::dataStatus( int t0 )
{
    activate_signal( "dataStatus(int)", t0 );
}

#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 3
#elif Q_MOC_OUTPUT_REVISION != 3
#error "Moc format conflict - please regenerate all moc files"
#endif

#include <qmetaobject.h>
#include <qapplication.h>

const char *QMovieFramePrivate::className() const
{
    return "QMovieFramePrivate";
}

QMetaObject *QMovieFramePrivate::metaObj = 0;


#if QT_VERSION >= 199
static QMetaObjectInit init_QMovieFramePrivate(&QMovieFramePrivate::staticMetaObject);

#endif

void QMovieFramePrivate::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QObject::className(), "QObject") != 0 )
	badSuperclassWarning("QMovieFramePrivate","QObject");

#if QT_VERSION >= 199
    staticMetaObject();
}

QString QMovieFramePrivate::tr(const char* s)
{
    return qApp->translate("QMovieFramePrivate",s);
}

void QMovieFramePrivate::staticMetaObject()
{
    if ( metaObj )
	return;
    QObject::staticMetaObject();
#else

    QObject::initMetaObject();
#endif

    typedef void(QMovieFramePrivate::*m1_t0)();
    m1_t0 v1_0 = &QMovieFramePrivate::refresh;
    QMetaData *slot_tbl = new QMetaData[1];
    slot_tbl[0].name = "refresh()";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    typedef void(QMovieFramePrivate::*m2_t0)(const QSize&);
    typedef void(QMovieFramePrivate::*m2_t1)(const QRect&);
    typedef void(QMovieFramePrivate::*m2_t2)(int);
    m2_t0 v2_0 = &QMovieFramePrivate::sizeChanged;
    m2_t1 v2_1 = &QMovieFramePrivate::areaChanged;
    m2_t2 v2_2 = &QMovieFramePrivate::dataStatus;
    QMetaData *signal_tbl = new QMetaData[3];
    signal_tbl[0].name = "sizeChanged(const QSize&)";
    signal_tbl[1].name = "areaChanged(const QRect&)";
    signal_tbl[2].name = "dataStatus(int)";
    signal_tbl[0].ptr = *((QMember*)&v2_0);
    signal_tbl[1].ptr = *((QMember*)&v2_1);
    signal_tbl[2].ptr = *((QMember*)&v2_2);
    metaObj = new QMetaObject( "QMovieFramePrivate", "QObject",
	slot_tbl, 1,
	signal_tbl, 3 );
}

// SIGNAL sizeChanged
void QMovieFramePrivate::sizeChanged( const QSize& t0 )
{
    QConnectionList *clist = receivers("sizeChanged(const QSize&)");
    if ( !clist || signalsBlocked() )
	return;
    typedef void (QObject::*RT0)();
    typedef RT0 *PRT0;
    typedef void (QObject::*RT1)(const QSize&);
    typedef RT1 *PRT1;
    RT0 r0;
    RT1 r1;
    QConnectionListIt it(*clist);
    QConnection   *c;
    QSenderObject *object;
    while ( (c=it.current()) ) {
	++it;
	object = (QSenderObject*)c->object();
	object->setSender( this );
	switch ( c->numArgs() ) {
	    case 0:
		r0 = *((PRT0)(c->member()));
		(object->*r0)();
		break;
	    case 1:
		r1 = *((PRT1)(c->member()));
		(object->*r1)(t0);
		break;
	}
    }
}

// SIGNAL areaChanged
void QMovieFramePrivate::areaChanged( const QRect& t0 )
{
    QConnectionList *clist = receivers("areaChanged(const QRect&)");
    if ( !clist || signalsBlocked() )
	return;
    typedef void (QObject::*RT0)();
    typedef RT0 *PRT0;
    typedef void (QObject::*RT1)(const QRect&);
    typedef RT1 *PRT1;
    RT0 r0;
    RT1 r1;
    QConnectionListIt it(*clist);
    QConnection   *c;
    QSenderObject *object;
    while ( (c=it.current()) ) {
	++it;
	object = (QSenderObject*)c->object();
	object->setSender( this );
	switch ( c->numArgs() ) {
	    case 0:
		r0 = *((PRT0)(c->member()));
		(object->*r0)();
		break;
	    case 1:
		r1 = *((PRT1)(c->member()));
		(object->*r1)(t0);
		break;
	}
    }
}

// SIGNAL dataStatus
void QMovieFramePrivate::dataStatus( int t0 )
{
    activate_signal( "dataStatus(int)", t0 );
}
