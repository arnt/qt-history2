/****************************************************************************
**
** Implementation of MNG QImage IOHandler.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QT_CLEAN_NAMESPACE
#define QT_CLEAN_NAMESPACE
#endif

#ifndef QT_NO_IMAGEIO_MNG

#include "qcoreapplication.h"
#include "qdatetime.h"
#include "qimage.h"
#include "qasyncimageio.h"
#include "qiodevice.h"
#include "qmngio.h"

// Define XMD_H prohibits the included headers of libmng.h to typedef INT32.
// This is needed for Borland with STL support, since in that case, INT32 is
// already defined by some Borland header.
#define XMD_H
#if defined(Q_OS_UNIXWARE)
#  define HAVE_BOOLEAN  // libjpeg under Unixware seems to need this
#endif
#include <libmng.h>
#include <stdlib.h>


#ifndef QT_NO_ASYNC_IMAGE_IO

class QMNGFormat : public QImageFormat {
public:
    QMNGFormat();
    virtual ~QMNGFormat();

    int decode(QImage& img, QImageConsumer* consumer,
            const uchar* buffer, int length);

    bool openstream()
    {
        // ### We should figure out how many loops an MNG has, but for now always assume infinite.
        if (consumer)
            consumer->setLooping(0);
        return true;
    }
    bool closestream()
    {
        if (consumer)
            consumer->end();
        return true;
    }
    bool readdata(mng_ptr pBuf, mng_uint32 iBuflen, mng_uint32p pRead)
    {
        uint m = ndata + nbuffer - ubuffer;
        if (iBuflen > m) {
            iBuflen = m;
        }
        *pRead = iBuflen;
        uint n = nbuffer-ubuffer;
        if (iBuflen < n) {
            // enough in buffer
            memcpy(pBuf, buffer+ubuffer, iBuflen);
            ubuffer += iBuflen;
            return true;
        }
        if (n) {
            // consume buffer
            memcpy(pBuf, buffer+ubuffer, n);
            pBuf = (mng_ptr)((char*)pBuf + n);
            iBuflen -= n;
            ubuffer = nbuffer;
        }
        if (iBuflen) {
            // fill from incoming data
            memcpy(pBuf, data, iBuflen);
            data += iBuflen;
            ndata -= iBuflen;
        }
        return true;
    }
    bool errorproc(mng_int32   iErrorcode,
                   mng_int8    /*iSeverity*/,
                   mng_chunkid iChunkname,
                   mng_uint32  /*iChunkseq*/,
                   mng_int32   iExtra1,
                   mng_int32   iExtra2,
                   mng_pchar   zErrortext)
    {
        qWarning("MNG error %d: %s; chunk %c%c%c%c; subcode %d:%d",
            iErrorcode,zErrortext,
            (iChunkname>>24)&0xff,
            (iChunkname>>16)&0xff,
            (iChunkname>>8)&0xff,
            (iChunkname>>0)&0xff,
            iExtra1,iExtra2);
        return true;
    }
    bool processheader(mng_uint32 iWidth, mng_uint32 iHeight)
    {
        image->create(iWidth,iHeight,32);
        image->setAlphaBuffer(true);
        memset(image->bits(),0,iWidth*iHeight*4);
        consumer->setSize(iWidth,iHeight);
        mng_set_canvasstyle(handle,
            QImage::systemByteOrder() == QImage::LittleEndian
                ? MNG_CANVAS_BGRA8 : MNG_CANVAS_ARGB8);
        return true;
    }
    mng_ptr getcanvasline(mng_uint32 iLinenr)
    {
        return image->scanLine(iLinenr);
    }
    mng_bool refresh(mng_uint32 x, mng_uint32 y, mng_uint32 w, mng_uint32 h)
    {
        QRect r(x,y,w,h);
        consumer->changed(r);
        consumer->setFramePeriod(0);
        consumer->frameDone();
        return true;
    }
    mng_uint32 gettickcount()
    {
        return timer.elapsed() - losttime;
    }
    bool settimer(mng_uint32 iMsecs)
    {
        consumer->setFramePeriod(iMsecs);
        consumer->frameDone();
        state = Time;
        losingtimer.start();
        losttime -= iMsecs;
        return true;
    }

private:
    // Animation-level information
    enum { MovieStart, Time, Data, Data2 } state;

    // Image-level information
    mng_handle handle;

    // For storing unused data
    uchar *buffer;
    uint maxbuffer;
    uint nbuffer;

    // Timing
    QTime timer;
    QTime losingtimer;
    int losttime;

    void enlargeBuffer(uint n)
    {
        if (n > maxbuffer) {
            maxbuffer = n;
            buffer = (uchar*)realloc(buffer,n);
        }
    }

    // Temporary locals during single data-chunk processing
    const uchar* data;
    uint ndata;
    uint ubuffer;
    QImageConsumer* consumer;
    QImage* image;
};

class QMNGFormatType : public QImageFormatType
{
    QImageFormat* decoderFor(const uchar* buffer, int length);
    QByteArray formatName() const;
};


/*
  \class QMNGFormat qmngio.h
  \brief Incremental image decoder for MNG image format.

  \ingroup images
  \ingroup graphics

  This subclass of QImageFormat decodes MNG format images,
  including animated MNGs.

  Animated MNG images are standard MNG images.  The MNG standard
  defines two extension chunks that are useful for animations:

  <dl>
   <dt>gIFg - GIF-like Graphic Control Extension
    <dd>Includes frame disposal, user input flag (we ignore this),
            and inter-frame delay.
   <dt>gIFx - GIF-like Application Extension
    <dd>Multi-purpose, but we just use the Netscape extension
            which specifies looping.
  </dl>

  The subimages usually contain a offset chunk (oFFs) but need not.

  The first image defines the "screen" size.  Any subsequent image that
  doesn't fit is clipped.

TODO: decide on this point.  gIFg gives disposal types, so it can be done.
  All images paste (\e not composite, just place all-channel copying)
  over the previous image to produce a subsequent frame.
*/

/*
  \class QMNGFormatType qasyncimageio.h
  \brief Incremental image decoder for MNG image format.

  \ingroup images
  \ingroup graphics
  \ingroup io

  This subclass of QImageFormatType recognizes MNG
  format images, creating a QMNGFormat when required.  An instance
  of this class is created automatically before any other factories,
  so you should have no need for such objects.
*/

QImageFormat* QMNGFormatType::decoderFor(const uchar* buffer, int length)
{
    if (length < 8) return 0;

    if (buffer[0]==138 // MNG signature
     && buffer[1]=='M'
     && buffer[2]=='N'
     && buffer[3]=='G'
     && buffer[4]==13
     && buffer[5]==10
     && buffer[6]==26
     && buffer[7]==10
     || buffer[0]==139 // JNG signature
     && buffer[1]=='J'
     && buffer[2]=='N'
     && buffer[3]=='G'
     && buffer[4]==13
     && buffer[5]==10
     && buffer[6]==26
     && buffer[7]==10
#ifdef QT_NO_IMAGEIO_PNG // if we don't have native PNG support use libmng
     || buffer[0]==137 // PNG signature
     && buffer[1]=='P'
     && buffer[2]=='N'
     && buffer[3]=='G'
     && buffer[4]==13
     && buffer[5]==10
     && buffer[6]==26
     && buffer[7]==10
#endif
   )
        return new QMNGFormat;
    return 0;
}

QByteArray QMNGFormatType::formatName() const
{
    static const QByteArray nm("MNG");
    return nm;
}


/*!
  Constructs a QMNGFormat.
*/
QMNGFormat::QMNGFormat()
{
    state = MovieStart;
    handle = 0;
    nbuffer = 0;
    maxbuffer = 0;
    buffer = 0;
    losttime = 0;
}

/*
  Destroys a QMNGFormat.
*/
QMNGFormat::~QMNGFormat()
{
    // We're setting the consumer to 0 since it may have been
    // deleted by read_async_image in qimage.cpp
    consumer = 0;
    if (handle) mng_cleanup(&handle);
}


// C-callback to C++-member-function conversion
//
static mng_bool openstream(mng_handle handle)
{
    return ((QMNGFormat*)mng_get_userdata(handle))->openstream();
}
static mng_bool closestream(mng_handle handle)
{
    return ((QMNGFormat*)mng_get_userdata(handle))->closestream();
}
static mng_bool readdata(mng_handle handle, mng_ptr pBuf, mng_uint32 iBuflen, mng_uint32p pRead)
{
    return ((QMNGFormat*)mng_get_userdata(handle))->readdata(pBuf,iBuflen,pRead);
}
static mng_bool errorproc(mng_handle handle,
                       mng_int32   iErrorcode,
                       mng_int8    iSeverity,
                       mng_chunkid iChunkname,
                       mng_uint32  iChunkseq,
                       mng_int32   iExtra1,
                       mng_int32   iExtra2,
                       mng_pchar   zErrortext)
{
    return ((QMNGFormat*)mng_get_userdata(handle))->errorproc(iErrorcode,
        iSeverity,iChunkname,iChunkseq,iExtra1,iExtra2,zErrortext);
}
static mng_bool processheader(mng_handle handle,
                           mng_uint32 iWidth, mng_uint32 iHeight)
{
    return ((QMNGFormat*)mng_get_userdata(handle))->processheader(iWidth,iHeight);
}
static mng_ptr getcanvasline(mng_handle handle, mng_uint32 iLinenr)
{
    return ((QMNGFormat*)mng_get_userdata(handle))->getcanvasline(iLinenr);
}
static mng_bool refresh(mng_handle handle,
                      mng_uint32  iTop,
                      mng_uint32  iLeft,
                      mng_uint32  iBottom,
                      mng_uint32  iRight       )
{
    return ((QMNGFormat*)mng_get_userdata(handle))->refresh(iTop,iLeft,iBottom,iRight);
}
static mng_uint32 gettickcount(mng_handle handle)
{
    return ((QMNGFormat*)mng_get_userdata(handle))->gettickcount();
}
static mng_bool settimer(mng_handle handle, mng_uint32  iMsecs)
{
    return ((QMNGFormat*)mng_get_userdata(handle))->settimer(iMsecs);
}

static mng_ptr memalloc(mng_size_t iLen)
{
    return calloc(1,iLen);
}
static void memfree(mng_ptr iPtr, mng_size_t /*iLen*/)
{
    free(iPtr);
}

/*!
  This function decodes some data into image changes.

  Returns the number of bytes consumed.
*/
int QMNGFormat::decode(QImage& img, QImageConsumer* cons,
                        const uchar* buf, int length)
{
    consumer = cons;
    image = &img;

    data = buf;
    ndata = length;
    ubuffer = 0;

    if (state == MovieStart) {
        handle = mng_initialize((mng_ptr)this, ::memalloc, ::memfree, 0);
        mng_set_suspensionmode(handle, MNG_TRUE);
        mng_setcb_openstream(handle, ::openstream);
        mng_setcb_closestream(handle, ::closestream);
        mng_setcb_readdata(handle, ::readdata);
        mng_setcb_errorproc(handle, ::errorproc);
        mng_setcb_processheader(handle, ::processheader);
        mng_setcb_getcanvasline(handle, ::getcanvasline);
        mng_setcb_refresh(handle, ::refresh);
        mng_setcb_gettickcount(handle, ::gettickcount);
        mng_setcb_settimer(handle, ::settimer);
        state = Data;
        mng_readdisplay(handle);
        losingtimer.start();
    }

    losttime += losingtimer.elapsed();
    if (ndata || !length)
        mng_display_resume(handle);
    losingtimer.start();

    image = 0;

    nbuffer -= ubuffer;
    if (nbuffer) {
        // Move back unused tail
        memcpy(buffer,buffer+ubuffer,nbuffer);
    }
    if (ndata) {
        // Not all used.
        enlargeBuffer(nbuffer+ndata);
        memcpy(buffer+nbuffer,data,ndata);
        nbuffer += ndata;
    }

    return length;
}

static QMNGFormatType* globalMngFormatTypeObject = 0;

#endif // QT_NO_ASYNC_IMAGE_IO

#ifndef QT_NO_ASYNC_IMAGE_IO
void qCleanupMngIO()
{
    if (globalMngFormatTypeObject) {
        delete globalMngFormatTypeObject;
        globalMngFormatTypeObject = 0;
    }
}
#endif

void qInitMngIO()
{
    static bool done = false;
    if (!done) {
        done = true;
#ifndef QT_NO_ASYNC_IMAGE_IO
        globalMngFormatTypeObject = new QMNGFormatType;
        qAddPostRoutine(qCleanupMngIO);
#endif
    }
}

#endif // QT_NO_IMAGEIO_MNG
