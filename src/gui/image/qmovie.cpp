/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

// #define QT_SAVE_MOVIE_HACK

#include "qtimer.h"
#include "qpainter.h"
#include "qbitmap.h"
#include "qmovie.h"
#include "qfile.h"
#include "qbuffer.h"
#include "qobject.h"
#include "qpixmapcache.h"

#ifndef QT_NO_MOVIE

#include "qasyncimageio.h"

#include <stdlib.h>

/*!
    \class QMovie qmovie.h
    \brief The QMovie class provides incremental loading of animations or images, signalling as it progresses.

    \ingroup multimedia
    \mainclass

    The simplest way to display a QMovie is to use a QLabel and
    QLabel::setMovie().

    A QMovie provides a QPixmap as the framePixmap(); connections can
    be made via connectResize() and connectUpdate() to receive
    notification of size and pixmap changes. All decoding is driven
    by the normal event-processing mechanisms.

    The movie begins playing as soon as the QMovie is created
    (actually, once control returns to the event loop). When the last
    frame in the movie has been played, it may loop back to the start
    if such looping is defined in the input source.

    QMovie objects are explicitly shared. This means that a QMovie
    copied from another QMovie will be displaying the same frame at
    all times. If one shared movie pauses, all pause. To make \e
    independent movies, they must be constructed separately.

    The set of data formats supported by QMovie is determined by the
    decoder factories that have been installed; the format of the
    input is determined as the input is decoded.

    The supported formats are GIF (if Qt is configured with GIF
    support enabled).

    \img qmovie.png QMovie

    \legalese

    Qt supports GIF reading if it is configured that way during
    installation. If it is, we are required to state that
    "The Graphics Interchange Format(c) is the Copyright property of
    CompuServe Incorporated. GIF(sm) is a Service Mark property of
    CompuServe Incorporated."

    \warning If you are in a country that recognizes software patents
    and in which Unisys holds a patent on LZW compression and/or
    decompression and you want to use GIF, Unisys may require you to
    license the technology. Such countries include Canada, Japan, the
    USA, France, Germany, Italy, and the UK. We believe that this
    patent will have expired world-wide by the end of 2004.
    Nonetheless, GIF support may be removed completely in a future
    version of Qt. We recommend using the PNG format instead.

    \sa QLabel::setMovie()
*/

/*!
    \enum QMovie::Status

    \value SourceEmpty
    \value UnrecognizedFormat
    \value Paused
    \value EndOfFrame
    \value EndOfLoop
    \value EndOfMovie
    \value SpeedChanged
*/

class QMoviePrivate : public QObject, private QImageConsumer
{
    Q_OBJECT

public: // for QMovie

    // Creates a null Private
    QMoviePrivate();

    // NOTE:  The ownership of the QDataSource is transferred to the Private
    QMoviePrivate(QIODevice* src, QMovie* movie, int bufsize);

    virtual ~QMoviePrivate();

    bool isNull() const;

    // Initialize, possibly to the null state
    void init(bool fully);
    void flushBuffer();
    void updatePixmapFromImage();
    void updatePixmapFromImage(const QPoint& off, const QRect& area);
    void showChanges();

    // This as QImageConsumer
    void changed(const QRect& rect);
    void end();
    void preFrameDone(); //util func
    void frameDone();
    void frameDone(const QPoint&, const QRect& rect);
    void restartTimer();
    void setLooping(int l);
    void setFramePeriod(int milliseconds);
    void setSize(int w, int h);

    int  bufferSpace();
    void receive(const uchar* b, int bytecount);
    void pause();

signals:
    void sizeChanged(const QSize&);
    void areaChanged(const QRect&);
    void dataStatus(int);

public slots:
    void pollForData();
    void refresh();

public:
    QAtomic ref;
    QMovie *that;
    QWidget * display_widget;

    QImageDecoder *decoder;

    // Cyclic buffer
    int buf_size, buf_usage;
    uchar *buffer;

    int framenumber;
    int frameperiod;
    int speed;
    QTimer *frametimer, *polltimer;
    int lasttimerinterval;
    int loop;
    bool movie_ended;
    bool waitingForFrameTick;
    int stepping;
    QRect changed_area;
    QRect valid_area;
    QIODevice *source;
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


QMoviePrivate::QMoviePrivate()
    : display_widget(0), decoder(0), buf_size(0), buffer(0), polltimer(0), source(0)
{
    ref = 1;
    init(false);
}

// NOTE:  The ownership of the QDataSource is transferred to the Private
QMoviePrivate::QMoviePrivate(QIODevice *src, QMovie *movie, int bufsize) :
    that(movie), buf_size(bufsize)
{
    ref = 1;
    polltimer = new QTimer(this);
    polltimer->setSingleShot(true);
    connect(polltimer, SIGNAL(timeout()), this, SLOT(pollForData()));
    frametimer = new QTimer(this);
    connect(frametimer, SIGNAL(timeout()), this, SLOT(refresh()));
    source = src;
    buffer = 0;
    decoder = 0;
    speed = 100;
    display_widget = 0;
    init(true);
}

QMoviePrivate::~QMoviePrivate()
{
    if (buffer)                                // Avoid purify complaint
        delete [] buffer;
    delete decoder;
    delete source;
}

bool QMoviePrivate::isNull() const
{
    return !buf_size;
}

// Initialize. Only actually allocate any space if \a fully is true,
// otherwise, just enough to be a valid null Private.
void QMoviePrivate::init(bool fully)
{
#ifdef QT_SAVE_MOVIE_HACK
    save_image = true;
    image_number = 0;
#endif

    buf_usage = 0;
    if (buffer)                                // Avoid purify complaint
        delete [] buffer;
    buffer = fully ? new uchar[buf_size] : 0;
    if (buffer)
        memset(buffer, 0, buf_size);

    delete decoder;
    decoder = fully ? new QImageDecoder(this) : 0;

#ifdef AVOID_OPEN_FDS
    if (source && !source->isOpen())
        source->open(QIODevice::ReadOnly);
#endif
    waitingForFrameTick = false;
    stepping = -1;
    framenumber = 0;
    frameperiod = -1;
    if (fully) {
        polltimer->start(0);
        frametimer->stop();
    }
    lasttimerinterval = -1;
    changed_area.setRect(0, 0, -1, -1);
    valid_area = changed_area;
    loop = -1;
    movie_ended = false;
    error = 0;
    empty = true;
}

void QMoviePrivate::flushBuffer()
{
    int used;
    int off = 0;
    while (off < buf_usage && !waitingForFrameTick && stepping != 0 && !error) {
        used = decoder->decode(buffer + off, buf_usage-off);
        if (used <= 0) {
            if (used < 0) {
                error = 1;
                emit dataStatus(QMovie::UnrecognizedFormat);
            }
            break;
        }
        off += used;
    }
    if (off) {
        buf_usage -= off;
        if (buf_usage)
            memcpy(buffer, buffer + off, buf_usage);
    }

    // Some formats, like MNG, can make stuff happen without any extra data.
    // Only do this if the movie hasn't ended, however or we'll never get the end of loop signal.
    if (!movie_ended) {
        used = decoder->decode(buffer, 0);
        if (used <= 0) {
            if (used < 0) {
                error = 1;
                emit dataStatus(QMovie::UnrecognizedFormat);
            }
        }
    }

    if (error)
        frametimer->stop();
    pollForData();
}

void QMoviePrivate::updatePixmapFromImage()
{
    if (changed_area.isEmpty())
        return;
    updatePixmapFromImage(QPoint(0, 0), changed_area);
}

void QMoviePrivate::updatePixmapFromImage(const QPoint &off, const QRect &area)
{
    // Create temporary QImage to hold the part we want
    const QImage& gimg = decoder->image();
    QImage img = gimg.copy(area);

#ifdef QT_SAVE_MOVIE_HACK
    if (save_image) {
        QString name;
        name.sprintf("movie%i.ppm",image_number++);
        gimg.save(name, "PPM");
    }
#endif

    // Resize to size of image
    if (mypixmap.width() != gimg.width() || mypixmap.height() != gimg.height())
        mypixmap.resize(gimg.width(), gimg.height());

    // Convert to pixmap and bitBlt that onto myself
    QPixmap lines;

    if (!(frameperiod < 0 && loop == -1)) {
        // its an animation, lets see if we converted
        // this frame already.
        QString key("qt_movie012301"); // reserve 8+4 bytes for the key
        void *p = this;
        memcpy(static_cast<void *>(const_cast<QChar *>(key.unicode() + 8)),
               static_cast<void *>(&p), sizeof(void *));
        memcpy(static_cast<void *>(const_cast<QChar *>(key.unicode() + 12)), &framenumber,
               sizeof(framenumber));
        if (!QPixmapCache::find(key, lines)) {
            lines.fromImage(img, Qt::ColorOnly);
            QPixmapCache::insert(key, lines);
        }
    } else {
        lines.fromImage(img, Qt::ColorOnly);
    }

    if (bg.isValid()) {
        QPainter p;
        p.begin(&mypixmap);
        p.fillRect(area, bg);
        p.drawPixmap(area, lines);
        p.end();
    } else {
        if (gimg.hasAlphaBuffer()) {
            // Resize to size of image
            if (mymask.isNull()) {
                mymask.resize(gimg.width(), gimg.height());
                mymask.fill(Qt::color1);
            }
        }
        mypixmap.setMask(QBitmap()); // Remove reference to my mask
        QPainter p(&mypixmap);
        p.drawPixmap(area.left(), area.top(), lines, off.x(), off.y(), area.width(), area.height());
    }

#if 0//def Q_WS_QWS

//########################
// do we need this at all ???
    if(display_widget) {
        QWSPaintEngine *pe = static_cast<QWSPaintEngine*>(display_widget->paintEngine());
        pe->begin(display_widget);
        double xscale,yscale;
        xscale=display_widget->width();
        yscale=display_widget->height();
        xscale=xscale / ((double)mypixmap.width());
        yscale=yscale / ((double)mypixmap.height());
        double xh,yh;
        xh = xscale * ((double)area.left());
        yh = yscale * ((double)area.top());

        pe->stretchBlt(mypixmap,0, 0, display_widget->width(),
                          display_widget->height(), mypixmap.width(),
                          mypixmap.height());
        pe->end();
    }
}
#endif
}

void QMoviePrivate::showChanges()
{
    if (changed_area.isValid()) {
        updatePixmapFromImage();

        valid_area = valid_area.unite(changed_area);
        emit areaChanged(changed_area);

        changed_area.setWidth(-1); // make empty
    }
}

// Private as QImageConsumer
void QMoviePrivate::changed(const QRect& rect)
{
    if (!frametimer->isActive())
        frametimer->start(lasttimerinterval);
    changed_area = changed_area.unite(rect);
}

void QMoviePrivate::end()
{
    movie_ended = true;
}

void QMoviePrivate::preFrameDone()
{
    if (stepping > 0) {
        stepping--;
        if (!stepping) {
            frametimer->stop();
            emit dataStatus(QMovie::Paused);
        }
    } else {
        waitingForFrameTick = true;
        restartTimer();
    }
}
void QMoviePrivate::frameDone()
{
    preFrameDone();
    showChanges();
    emit dataStatus(QMovie::EndOfFrame);
    ++framenumber;
}
void QMoviePrivate::frameDone(const QPoint& p,
                                const QRect& rect)
{
    preFrameDone();
    const QImage& gimg = decoder->image();
    QPoint point = p - gimg.offset();
    if (framenumber==0)
        emit sizeChanged(gimg.size());
    valid_area = valid_area.unite(QRect(point,rect.size()));
    updatePixmapFromImage(point,rect);
    emit areaChanged(QRect(point,rect.size()));
    emit dataStatus(QMovie::EndOfFrame);
    ++framenumber;
}

void QMoviePrivate::restartTimer()
{
    if (speed > 0) {
        int i = frameperiod >= 0 ? frameperiod * 100 / speed : 0;
        if (i != lasttimerinterval || !frametimer->isActive()) {
            lasttimerinterval = i;
            frametimer->start(i);
        }
    } else {
        frametimer->stop();
    }
}

void QMoviePrivate::setLooping(int nloops)
{
    if (loop == -1) // Only if we don't already know how many loops!
        loop = nloops;
}

void QMoviePrivate::setFramePeriod(int milliseconds)
{
    // Animation:  only show complete frame
    frameperiod = milliseconds;
    if (stepping<0 && frameperiod >= 0)
        restartTimer();
}

void QMoviePrivate::setSize(int w, int h)
{
    if (mypixmap.width() != w || mypixmap.height() != h) {
        mypixmap.resize(w, h);
        emit sizeChanged(QSize(w, h));
    }
}


void QMoviePrivate::receive(const uchar* b, int bytecount)
{
    if (bytecount)
        empty = false;

    while (bytecount && !waitingForFrameTick && stepping != 0) {
        int used = decoder->decode(b, bytecount);
        if (used<=0) {
            if (used < 0) {
                error = 1;
                emit dataStatus(QMovie::UnrecognizedFormat);
            }
            break;
        }
        b += used;
        bytecount -= used;
    }

    // Append unused to buffer
    int buffer_end = buf_usage;
    buf_usage += bytecount;
    Q_ASSERT(bytecount <= buf_size);
    memcpy(buffer + buffer_end, b, bytecount);
}

int QMoviePrivate::bufferSpace()
{
    return qMin(buf_size, (int)(source->size()-source->pos()));
}

void QMoviePrivate::pollForData()
{
    if(waitingForFrameTick || !stepping || buf_usage || error || !source->isOpen())
        return;

    if(!bufferSpace()) { //EOF
        if (!movie_ended)
            return;
        if (empty)
            emit dataStatus(QMovie::SourceEmpty);

#ifdef QT_SAVE_MOVIE_HACK
        save_image = false;
#endif
        emit dataStatus(QMovie::EndOfLoop);

        if (loop >= 0) {
            if (loop) {
                loop--;
                if (!loop)
                    return;
            }
            source->reset();
            framenumber = 0;
            movie_ended = false;
            polltimer->start(0);
        } else {
            delete decoder;
            decoder = 0;
            if (buffer)                                // Avoid purify complaint
                delete [] buffer;
            buffer = 0;
            emit dataStatus(QMovie::EndOfMovie);
#ifdef AVOID_OPEN_FDS
            if (source)
                source->close();
#endif
        }
    } else if(int space = bufferSpace()) {
        int avail = source->read((char*)buffer, space);
        if(avail > 0)
            receive(buffer, avail);
        if(avail == -1)
            source->close();
        else
            polltimer->start(0);
    }
}

void QMoviePrivate::pause()
{
    if (stepping) {
        stepping = 0;
        frametimer->stop();
        emit dataStatus(QMovie::Paused);
    }
}

void QMoviePrivate::refresh()
{
    if (!decoder) {
        frametimer->stop();
        return;
    }

    if (frameperiod < 0 && loop == -1) // Only show changes if probably not an animation
        showChanges();
    if (!buf_usage)
        frametimer->stop();
    waitingForFrameTick = false;
    flushBuffer();
}

///////////////// End of Private /////////////////





/*!
    Constructs a null QMovie. The only interesting thing to do with
    such a movie is to assign another movie to it.

    \sa isNull()
*/
QMovie::QMovie()
{
    d = new QMoviePrivate();
}

/*!
    Constructs a QMovie with an external data source. You should later
    call pushData() to send incoming animation data to the movie.

    The \a bufsize argument sets the maximum amount of data the movie
    will transfer from the data source per event loop. The lower this
    value, the better interleaved the movie playback will be with
    other event processing, but the slower the overall processing will
    be.

    \sa pushData()
*/
QMovie::QMovie(int bufsize)
{
    d = new QMoviePrivate(0, this, bufsize);
}

/*!
    Returns the maximum amount of data that can currently be pushed
    into the movie by a call to pushData(). This is affected by the
    initial buffer size, but varies as the movie plays and data is
    consumed.
*/
int QMovie::pushSpace() const
{
    return d->bufferSpace();
}

/*!
    Pushes \a length bytes from \a data into the movie. \a length must
    be no more than the amount returned by pushSpace() since the
    previous call to pushData().
*/
void QMovie::pushData(const uchar* data, int length)
{
    d->receive(data,length);
}

#ifdef Q_WS_QWS // ##### Temporary performance experiment
/*!
    \internal
*/
void QMovie::setDisplayWidget(QWidget * w)
{
    d->display_widget = w;
}
#endif

/*!
    Constructs a QMovie that reads an image sequence from the given
    data source, \a src. The source must be allocated dynamically,
    because QMovie will take ownership of it and will destroy it when
    the movie is destroyed. The movie starts playing as soon as event
    processing continues.

    The \a bufsize argument sets the maximum amount of data the movie
    will transfer from the data source per event loop. The lower this
    value, the better interleaved the movie playback will be with
    other event processing, but the slower the overall processing will
    be.
*/
QMovie::QMovie(QIODevice* src, int bufsize)
{
    d = new QMoviePrivate(src, this, bufsize);
}

/*!
    Constructs a QMovie that reads an image sequence from the file, \a
    fileName.

    The \a bufsize argument sets the maximum amount of data the movie
    will transfer from the data source per event loop. The lower this
    value, the better interleaved the movie playback will be with
    other event processing, but the slower the overall processing will
    be.
*/
QMovie::QMovie(const QString &fileName, int bufsize)
{
    QFile *file = new QFile(fileName);
    if (!fileName.isEmpty())
        file->open(QIODevice::ReadOnly);
    d = new QMoviePrivate(file, this, bufsize);
}

/*!
    Constructs a movie that uses the same data as movie \a movie.
    QMovies use explicit sharing, so operations on the copy will
    affect both.
*/
QMovie::QMovie(const QMovie& movie)
    : d(movie.d)
{
    ++d->ref;
}

/*!
    Destroys the QMovie. If this is the last reference to the data of
    the movie, the data is deallocated.
*/
QMovie::~QMovie()
{
    if (!--d->ref)
        delete d;
}

/*!
    Returns true if the movie is null; otherwise returns false.
*/
bool QMovie::isNull() const
{
    return d->isNull();
}

/*!
    Makes this movie use the same data as movie \a movie. QMovies use
    explicit sharing.
*/
QMovie& QMovie::operator=(const QMovie& movie)
{
    qAtomicAssign(d, movie.d);
    return *this;
}


/*!
    Sets the background color of the pixmap to \a c. If the background
    color isValid(), the pixmap will never have a mask because the
    background color will be used in transparent regions of the image.

    \sa backgroundColor()
*/
void QMovie::setBackgroundColor(const QColor& c)
{
    d->bg = c;
}

/*!
    Returns the background color of the movie set by
    setBackgroundColor().
*/
const QColor& QMovie::backgroundColor() const
{
    return d->bg;
}

/*!
    Returns the area of the pixmap for which pixels have been
    generated.
*/
const QRect& QMovie::getValidRect() const
{
    return d->valid_area;
}

/*!
    Returns the current frame of the movie, as a QPixmap. It is not
    generally useful to keep a copy of this pixmap. It is better to
    keep a copy of the QMovie and get the framePixmap() only when
    needed for drawing.

    \sa frameImage()
*/
const QPixmap& QMovie::framePixmap() const
{
    return d->mypixmap;
}

/*!
    Returns the current frame of the movie, as a QImage. It is not
    generally useful to keep a copy of this image. Also note that you
    must not call this function if the movie is finished(), since by
    then the image will not be available.

    \sa framePixmap()
*/
const QImage& QMovie::frameImage() const
{
    return d->decoder->image();
}

/*!
    Returns the number of steps remaining after a call to step(). If
    the movie is paused, steps() returns 0. If it's running normally
    or is finished, steps() returns a negative number.
*/
int QMovie::steps() const
{
    return d->stepping;
}

/*!
    Returns the number of times EndOfFrame has been emitted since the
    start of the current loop of the movie. Thus, before any
    EndOfFrame has been emitted the value will be 0; within slots
    processing the first signal, frameNumber() will be 1, and so on.
*/
int QMovie::frameNumber() const
{
    return d->framenumber;
}

/*!
    Returns true if the image is paused; otherwise returns false.
*/
bool QMovie::paused() const
{
    return d->stepping == 0;
}

/*!
    Returns true if the image is no longer playing: this happens when
    all loops of all frames are complete; otherwise returns false.
*/
bool QMovie::finished() const
{
    return !d->decoder;
}

/*!
    Returns true if the image is not single-stepping, not paused, and
    not finished; otherwise returns false.
*/
bool QMovie::running() const
{
    return d->stepping < 0 && d->decoder;
}

/*!
    Pauses the progress of the animation.

    \sa unpause()
*/
void QMovie::pause()
{
    d->pause();
}

/*!
    Unpauses the progress of the animation.

    \sa pause()
*/
void QMovie::unpause()
{
    if (d->stepping >= 0 && !d->isNull()) {
        d->stepping = -1;
        d->restartTimer();
    }
}

/*!
    \overload

    Steps forward, showing \a steps frames, and then pauses.
*/
void QMovie::step(int steps)
{
    if (d->isNull())
        return;
    d->stepping = steps;
    d->frametimer->start(0);
    d->waitingForFrameTick = false; // Full speed ahead!
}

/*!
    Steps forward 1 frame and then pauses.
*/
void QMovie::step()
{
    step(1);
}

/*!
    Rewinds the movie to the beginning. If the movie has not been
    paused, it begins playing again.
*/
void QMovie::restart()
{
    if (d->isNull())
        return;
    d->source->reset();
    int s = d->stepping;
    d->init(true);
    if (s > 0)
        step(s);
    else if (s == 0)
        pause();
}

/*!
    Returns the movie's play speed as a percentage. The default is 100
    percent.

    \sa setSpeed()
*/
int QMovie::speed() const
{
    return d->speed;
}

/*!
    Sets the movie's play speed as a percentage, to \a percent. This
    is a percentage of the speed dictated by the input data format.
    The default is 100 percent.
*/
void QMovie::setSpeed(int percent)
{
    int oldspeed = d->speed;
    if (oldspeed != percent && percent >= 0) {
        d->speed = percent;
        // Restart timer only if really needed
        if (d->stepping < 0) {
            if (!percent || !oldspeed    // To or from zero
                 || oldspeed*4 / percent > 4   // More than 20% slower
                 || percent*4 / oldspeed > 4   // More than 20% faster
               )
                d->restartTimer();
        }
    }
}

/*!
    Connects the \a{receiver}'s \a member of type \c{void member(const
    QSize&)} so that it is signalled when the movie changes size.

    Note that due to the explicit sharing of QMovie objects, these
    connections persist until they are explicitly disconnected with
    disconnectResize() or until \e every shared copy of the movie is
    deleted.
*/
void QMovie::connectResize(QObject* receiver, const char *member)
{
    QObject::connect(d, SIGNAL(sizeChanged(QSize)), receiver, member);
}

/*!
    Disconnects the \a{receiver}'s \a member (or all members if \a
    member is zero) that were previously connected by connectResize().
*/
void QMovie::disconnectResize(QObject* receiver, const char *member)
{
    QObject::disconnect(d, SIGNAL(sizeChanged(QSize)), receiver, member);
}

/*!
    Connects the \a{receiver}'s \a member of type \c{void member(const
    QRect&)} so that it is signalled when an area of the framePixmap()
    has changed since the previous frame.

    Note that due to the explicit sharing of QMovie objects, these
    connections persist until they are explicitly disconnected with
    disconnectUpdate() or until \e every shared copy of the movie is
    deleted.
*/
void QMovie::connectUpdate(QObject* receiver, const char *member)
{
    QObject::connect(d, SIGNAL(areaChanged(QRect)), receiver, member);
}

/*!
    Disconnects the \a{receiver}'s \a member (or all members if \a
    member is zero) that were previously connected by connectUpdate().
*/
void QMovie::disconnectUpdate(QObject* receiver, const char *member)
{
    QObject::disconnect(d, SIGNAL(areaChanged(QRect)), receiver, member);
}

/*!
    Connects the \a{receiver}'s \a member, of type \c{void
    member(int)} so that it is signalled when the movie changes
    status. The status codes are negative for errors and positive for
    information.

    \table
    \header \i Status Code \i Meaning
    \row \i QMovie::SourceEmpty
         \i signalled if the input cannot be read.
    \row \i QMovie::UnrecognizedFormat
         \i signalled if the input data is unrecognized.
    \row \i QMovie::Paused
          \i signalled when the movie is paused by a call to paused()
          or by after \link step() stepping \endlink pauses.
    \row \i QMovie::EndOfFrame
         \i signalled at end-of-frame after any update and Paused signals.
    \row \i QMovie::EndOfLoop
         \i signalled at end-of-loop, after any update signals,
         EndOfFrame - but before EndOfMovie.
    \row \i QMovie::EndOfMovie
         \i signalled when the movie completes and is not about to loop.
    \endtable

    More status messages may be added in the future, so a general test
    for errors would test for negative.

    Note that due to the explicit sharing of QMovie objects, these
    connections persist until they are explicitly disconnected with
    disconnectStatus() or until \e every shared copy of the movie is
    deleted.
*/
void QMovie::connectStatus(QObject* receiver, const char *member)
{
    QObject::connect(d, SIGNAL(dataStatus(int)), receiver, member);
}

/*!
    Disconnects the \a{receiver}'s \a member (or all members if \a
    member is zero) that were previously connected by connectStatus().
*/
void QMovie::disconnectStatus(QObject* receiver, const char *member)
{
    QObject::disconnect(d, SIGNAL(dataStatus(int)), receiver, member);
}


#include "qmovie.moc"

#endif        // QT_NO_MOVIE
