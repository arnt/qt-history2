/****************************************************************************
**
** Implementation of asynchronous I/O classes.
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

#include "qasyncio.h"
#include "qiodevice.h"
#include <stdlib.h>

#ifndef QT_NO_ASYNC_IO

/*!
  Destroys the async IO object.
*/
QAsyncIO::~QAsyncIO()
{
}

/*!
  Ensures that only one object, \a obj and function, \a member, can
  respond to changes in readiness.
*/
void QAsyncIO::connect(QObject* obj, const char *member)
{
    signal.disconnect(0, 0);
    signal.connect(obj, member);
}

/*!
  Derived classes should call this when they change from being
  unready to ready.
*/
void QAsyncIO::ready()
{
    signal.activate();
}



/*!
  This should be called whenever readyToReceive() might have become non-zero.
  It is merely calls QAsyncIO::ready() if readyToReceive() is non-zero.
*/
void QDataSink::maybeReady()
{
    if (readyToReceive()) ready();
}

/*!
  This should be called whenever readyToSend() might have become non-zero.
  It is merely calls QAsyncIO::ready() if readyToSend() is non-zero.
*/
void QDataSource::maybeReady()
{
    if (readyToSend()) ready();
}

/*!
  This function should return true if the data source can be rewound.

  The default returns false.
*/
bool QDataSource::rewindable() const
{
    return false;
}

/*!
  If this function is called with \a on set to true, and rewindable()
  is true, then the data source must take measures to allow the rewind()
  function to subsequently operate as described.  If rewindable() is false,
  the function should call QDataSource::enableRewind(), which aborts with
  a qFatal() error.

  For example, a network connection may choose to use a disk cache
  of input only if rewinding is enabled before the first buffer-full of
  data is discarded, returning false in rewindable() if that first buffer
  is discarded.
*/
void QDataSource::enableRewind(bool /* on */)
{
    qFatal("Attempted to make unrewindable QDataSource rewindable");
}

/*!
  This function rewinds the data source.  This may only be called if
  enableRewind(true) has been previously called.
*/
void QDataSource::rewind()
{
    qFatal("Attempted to rewind unrewindable QDataSource");
}

/*!
  Constructs a QIODeviceSource from the QIODevice \a device.  The QIODevice
  \e must be dynamically allocated, becomes owned by the QIODeviceSource,
  and will be deleted when the QIODeviceSource is destroyed. \a buffer_size
  determines the size of buffering to use between asynchronous operations.
  The higher the \a buffer_size, the more efficient, but the less interleaved
  the operation will be with other processing.
*/
QIODeviceSource::QIODeviceSource(QIODevice* device, int buffer_size) :
    buf_size(buffer_size),
    buffer(new uchar[buf_size]),
    iod(device),
    rew(false)
{
}

/*!
  Destroys the QIODeviceSource, deleting the QIODevice from which it was
  constructed.
*/
QIODeviceSource::~QIODeviceSource()
{
    delete iod;
    delete [] buffer;
}

/*!
  Ready until end-of-file.
*/
int QIODeviceSource::readyToSend()
{
    if (iod->status() != IO_Ok || !(iod->state() & IO_Open))
        return -1;

    int n = qMin((uint)buf_size, iod->size()-iod->at()); // ### not 64-bit safe
                                                         // ### not large file safe
    return n ? n : -1;
}

/*!
  Reads a block of data and sends up to \a n bytes to the \a sink.
*/
void QIODeviceSource::sendTo(QDataSink* sink, int n)
{
    iod->readBlock((char*)buffer, n);
    sink->receive(buffer, n);
}

/*!
  All QIODeviceSource's are rewindable.
*/
bool QIODeviceSource::rewindable() const
{
    return true;
}

/*!
  If \a on is set to true then rewinding is enabled.
  No special action is taken.  If \a on is set to
  false then rewinding is disabled.
*/
void QIODeviceSource::enableRewind(bool on)
{
    rew = on;
}

/*!
  Calls reset() on the QIODevice.
*/
void QIODeviceSource::rewind()
{
    if (!rew) {
        QDataSource::rewind();
    } else {
        iod->reset();
        ready();
    }
}


/*!
  Constructs a QDataPump to move data from a given \a data_source
  to a given \a data_sink.
*/
QDataPump::QDataPump(QDataSource* data_source, QDataSink* data_sink) :
    source(data_source), sink(data_sink)
{
    source->connect(this, SLOT(kickStart()));
    sink->connect(this, SLOT(kickStart()));
    connect(&timer, SIGNAL(timeout()), this, SLOT(tryToPump()));
    timer.start(0, true);
}

void QDataPump::kickStart()
{
    if (!timer.isActive()) {
        interval = 0;
        timer.start(0, true);
    }
}

void QDataPump::tryToPump()
{
    int supply, demand;

    supply = source->readyToSend();
    demand = sink->readyToReceive();
    if (demand <= 0) {
        return;
    }
    interval = 0;
    if (supply < 0) {
        // All done (until source signals change in readiness)
        sink->eof();
        return;
    }
    if (!supply)
        return;
    source->sendTo(sink, qMin(supply, demand));

    timer.start(0, true);
}

#endif // QT_NO_ASYNC_IO

