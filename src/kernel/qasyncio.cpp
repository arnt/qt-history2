/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qasyncio.cpp#5 $
**
** Implementation of asynchronous I/O classes
**
** Created : 970617
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qasyncio.h"
#include "qiodev.h"
#include "qtimer.h"
#include <stdlib.h>

/*!
  \class QAsyncIO qasyncio.h
  \brief Encapsulates I/O asynchronicity.

  \internal
*/


/*!
  Destroys the async IO object.
*/
QAsyncIO::~QAsyncIO()
{
}

/*!
  Ensures only one object can respond to changes in readiness.
*/
void QAsyncIO::connect(QObject* obj, const char* member)
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
  \class QDataSink qasyncio.h
  \brief A QDataSink is an asynchronous consumer of data.

  \internal

  A data sink is an object which receives data from some source in an
  asynchronous manner.  This means that at some time not determined by
  the data sink, blocks of data are given to it from processing.  The
  data sink is able to limit the maximum size of such blocks.

  \sa QDataSource, QDataPump
*/

/*!
  \fn int QDataSink::readyToReceive()

  The data sink should return a value indicating how much data it is ready
  to consume.  This may be 0.
*/

/*!
  This should be called whenever readyToReceive() might have become non-zero.
  It is merely calls QAsyncIO::ready() if readyToReceive() is non-zero.
*/
void QDataSink::maybeReady()
{
    if (readyToReceive()) ready();
}

/*!
  \fn void QDataSink::receive(const uchar*, int count)

  This function is called to provide data for the data sink.  The count
  will be no more than the amount indicated by the most recent call to
  readyToReceive().
*/

/*!
  \fn void QDataSink::eof()

  This function will be called when no more data is available for
  processing.
*/


/*!
  \class QDataSource qasyncio.h
  \brief A QDataSource is an asynchronous producer of data.

  \internal

  A data source is an object which provides data from some source in an
  asynchronous manner.  This means that at some time not determined by
  the data source, blocks of data will be taken from it for processing.
  The data source is able to limit the maximum size of such blocks.

  \sa QDataSink, QDataPump
*/

/*!
  \fn int QDataSource::readyToSend()

  The data source should return a value indicating how much data it is ready
  to provide.  This may be 0.  If the data source knows it will never be
  able to provide any more data (until after a rewind()), it may return -1.
*/

/*!
  This should be called whenever readyToSend() might have become non-zero.
  It is merely calls QAsyncIO::ready() if readyToReceive() is non-zero.
*/
void QDataSource::maybeReady()
{
    if (readyToSend()) ready();
}

/*!
  \fn void QDataSource::sendTo(QDataSink*, int count)

  Send the given amount of data to the given sink.
*/

/*!
  This function should return TRUE if the data source can be rewound.
*/
bool QDataSource::rewindable() const
{
    return FALSE;
}

/*!
  If this function is called with \a on set to TRUE, and rewindable()
  is TRUE, then the data source must take measures to allow the rewind()
  function to subsequently operate as described.  If rewindable() is FALSE,
  the function should call QDataSource::enableRewind(), which aborts with
  a fatal() error.

  For example, a network connection may choose to utilize a disk cache
  of input only if rewinding is enabled before the first buffer-full of
  data is discarded.
*/
void QDataSource::enableRewind(bool)
{
    fatal("Attempted to make unrewindable QDataSource rewindable");
}

/*!
  This function rewinds the data source.  This may only be called if
  enableRewind(TRUE) has been previously called.
*/
void QDataSource::rewind()
{
    fatal("Attempted to rewind unrewindable QDataSource");
}

/*!
  \class QIODeviceSource qasyncio.h
  \brief A QIODeviceSource is a QDataSource that draws data from a QIODevice

  \internal
*/

/*!
  Constructs a QIODeviceSource from a pointer to an QIODevice.  The QIODevice
  \e must be dynamically allocated, becomes owned by the QIODeviceSource,
  and will be deleted when the QIODeviceSource destructs. \a buffer_size
  determines the size of buffering to use between asynchronous operations.
  The higher \a buffer_size, the more efficient but the less interleaved
  the operation will be with other processing.
*/
QIODeviceSource::QIODeviceSource(QIODevice* device, int buffer_size) :
    buf_size(buffer_size),
    buffer(new uchar[buf_size]),
    iod(device),
    rew(FALSE)
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
    if ( iod->status() != IO_Ok || !(iod->state() & IO_Open) )
	return -1;

    int n = QMIN((uint)buf_size, iod->size()-iod->at());
    return n ? n : -1;
}

/*!
  Reads and sends a block of data.
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
    return TRUE;
}

/*!
  Enables rewinding.  No special action is taken.
*/
void QIODeviceSource::enableRewind(bool on)
{
    rew = on;
}

/*
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
  \class QDataPump qasyncio.h
  \brief Moves data from a QDataSource to a QDataSink during event processing.

  \internal
*/

/*!
  Creates a QDataPump to move data from a given \a data_source
  to a given \a data_sink.
*/
QDataPump::QDataPump(QDataSource* data_source, QDataSink* data_sink) :
    source(data_source), sink(data_sink)
{
    source->connect(this, SLOT(kickStart()));
    sink->connect(this, SLOT(kickStart()));
    connect(&timer, SIGNAL(timeout()), this, SLOT(tryToPump()));
    timer.start(0, TRUE);
}

void QDataPump::kickStart()
{
    if (!timer.isActive()) {
	interval = 0;
	timer.start(0, TRUE);
    }
}

void QDataPump::tryToPump()
{
    int supply, demand;
    do {
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
	source->sendTo(sink, QMIN(supply, demand));
    } while (1);

    timer.start(0, TRUE);
}
