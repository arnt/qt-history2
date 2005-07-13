/*

 Copyright (C) 2003, 2004, 2005 Texas Instruments, Inc.
 Copyright (C)       2004, 2005 Holger Hans Peter Freyther
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

   Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   Neither the name Texas Instruments, Inc nor the names of its
   contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#include "tslibmousehandler.h"

#ifdef QT_QWS_TSLIB
#include <QFile>
#include <QTextStream>
#include <QScreen>
#include <QSocketNotifier>
#include <QDebug>

#include <tslib.h>

#include <string.h>
#include <errno.h>
#include <stdlib.h>

TSLibMouseHandler::TSLibMouseHandler()
    : m_raw(false), m_notify(0 )
    , m_ts( 0 )
{
    setObjectName( "TSLib Mouse Handler" );
    openTs();
}

TSLibMouseHandler::~TSLibMouseHandler()
{
    closeTs();
}

void TSLibMouseHandler::openTs()
{
    char *tsdevice;

    if( ( tsdevice = getenv( "TSLIB_TSDEVICE" ) ) != NULL ) {
        m_ts = ts_open( tsdevice, 1 );
    } else {
        m_ts = ts_open( "/dev/ts", 1 );
    }

    if (!m_ts) {
        qWarning( "Cannot open touchscreen (%s)", strerror( errno ) );
        return;
    }

    if (ts_config( m_ts )) {
        qWarning( "Cannot configure touchscreen (%s)", strerror( errno ) );
        return;
    }


    m_notify = new QSocketNotifier( ts_fd(m_ts), QSocketNotifier::Read, this );
    connect( m_notify, SIGNAL(activated(int)), this, SLOT(readMouseData()));
}

void TSLibMouseHandler::closeTs()
{
    if (m_ts)
        ts_close(m_ts);
    m_ts = 0;

    delete m_notify;
    m_notify = 0;
    m_raw = false;
}

void TSLibMouseHandler::clearCalibration()
{
    m_raw = true;
}


void TSLibMouseHandler::calibrate( QWSPointerCalibrationData *cd )
{
    QPoint dev_tl = cd->devPoints[ QWSPointerCalibrationData::TopLeft ];
    QPoint dev_br = cd->devPoints[ QWSPointerCalibrationData::BottomRight ];
    QPoint screen_tl = cd->screenPoints[ QWSPointerCalibrationData::TopLeft ];
    QPoint screen_br = cd->screenPoints[ QWSPointerCalibrationData::BottomRight ];
    int a, b, c, d, e, f, s;

    s = 1 << 16;

    a = s * (screen_tl.x() - screen_br.x() ) / (dev_tl.x() - dev_br.x());
    b = 0;
    c = s * screen_tl.x() - a * dev_tl.x();

    d = 0;
    e = s * (screen_tl.y() - screen_br.y() ) / (dev_tl.y() - dev_br.y());
    f = s * screen_tl.y() - e * dev_tl.y();

    QString calFile = "/etc/pointercal";
#ifndef QT_NO_TEXTSTREAM
    QFile file( calFile );
    if ( file.open( QFile::WriteOnly ) ) {
        QTextStream t( &file );
        t << a << " " << b << " " << c << " ";
        t << d << " " << e << " " << f << " " << s;
       file.flush(); closeTs();
       openTs();
    } else
#endif
    {
        qCritical() << "Could not save calibration:" << calFile;
    }
}

void TSLibMouseHandler::suspend()
{
    m_notify->setEnabled( false );
}

void TSLibMouseHandler::resume()
{
    m_notify->setEnabled( true );
}

void TSLibMouseHandler::readMouseData()
{
    if(!qt_screen)
        return;

    /*
     * After clear Calibration
     * we're in raw mode and do some easy median
     * search.
     */
    if ( m_raw )
        return interpolateSample();

    static struct ts_sample sample;
    static int ret;

    /*
     * Ok. We need to see if we can read more than one event
     * We do this not to lose an update.
     */
    while ( true ) {
        if ((ret = ts_read(m_ts, &sample, 1)) != 1 )
            return;


        QPoint pos( sample.x, sample.y );
        mouseChanged( pos, sample.pressure != 0 ? 1 : 0 );
    }
}


/*
 * Lets take all down events and then sort them
 * and take the event in the middle.
 *
 * inspired by testutils.c
 */
void TSLibMouseHandler::interpolateSample()
{
    static struct ts_sample samples[25];
    int index = 0;
    int ret;

    do {
        /* fill only the last sample again */
        if ( index >= 25 )
            index = 24;

        /* we're opened non-blocking */
        if((ret= ts_read_raw(m_ts, &samples[index], 1 ) ) !=  1 ) {
            /* no event yet, so try again */
            if (ret==-1 ) {
                index--;
                continue;
            }
        }
    }while (samples[index++].pressure != 0);

    /*
     * index is maximal 25  and we at least one sample
     */
    if( index >= 25 )
        index = 24;
    int x, y;

    /*
     * now let us use the median value
     * even index does not have an item in the middle
     * so let us take the average of n/2 and (n/2)-1 as the middle
     */
    int m = index/2;
    ::qsort(samples, index, sizeof(ts_sample), TSLibMouseHandler::sortByX);
    x = (index % 2 ) ? samples[m].x :
        ( samples[m-1].x + samples[m].x )/2;

    ::qsort(samples, index, sizeof(ts_sample), TSLibMouseHandler::sortByY);
    y = (index % 2 ) ? samples[m].y :
        ( samples[m-1].y + samples[m].y )/2;

    emit mouseChanged( QPoint(x, y), 1 );
    emit mouseChanged( QPoint(0, 0), 0 );
}

int TSLibMouseHandler::sortByX( const void* one, const void* two) {
    return reinterpret_cast<const struct ts_sample*>(one)->x -
        reinterpret_cast<const struct ts_sample*>(two)->x;
}

int TSLibMouseHandler::sortByY( const void* one, const void* two) {
    return reinterpret_cast<const struct ts_sample*>(one)->y -
        reinterpret_cast<const struct ts_sample*>(two)->y;
}

#endif // QT_QWS_TSLIB
