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

#ifndef TSLIB_MOUSE_HANDLER_H
#define TSLIB_MOUSE_HANDLER_H

#include <QWSCalibratedMouseHandler>

#ifdef QT_QWS_TSLIB

class QSocketNotifier;
class TSLibMouseHandler : public QObject, public QWSCalibratedMouseHandler
{
    Q_OBJECT
public:
    TSLibMouseHandler();
    ~TSLibMouseHandler();

    void clearCalibration();
    void calibrate( QWSPointerCalibrationData * );
    void suspend();
    void resume();

    static int sortByX( const void*, const void* );
    static int sortByY( const void*, const void* );
private:
    void openTs();
    void closeTs();
    void interpolateSample();

private:
    bool m_raw : 1;
    QSocketNotifier *m_notify;
    struct tsdev *m_ts;

private slots:
    void readMouseData();
};

#endif // QT_QWS_TSLIB
#endif // TSLIB_MOUSE_HANDLER_H
