/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpngio.h#9 $
**
** Definition of PNG QImage IOHandler
**
** Created : 970521
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QPNGIO_H
#define QPNGIO_H

#ifndef QT_H
#include <qimage.h>
#endif // QT_H

#if QT_FEATURE_IMAGIO_PNG

void qInitPngIO();

class QIODevice;
class QImage;

class Q_EXPORT QPNGImageWriter {
public:
    QPNGImageWriter(QIODevice*);
    ~QPNGImageWriter();

    enum DisposalMethod { Unspecified, NoDisposal, RestoreBackground, RestoreImage };
    void setDisposalMethod(DisposalMethod);
    void setLooping(int loops=0); // 0 == infinity
    void setFrameDelay(int msecs);

    bool writeImage(const QImage& img, int x, int y);
    bool writeImage(const QImage& img)
	{ return writeImage(img, 0, 0); }

    QIODevice* device() { return dev; }

private:
    QIODevice* dev;
    int frames_written;
    DisposalMethod disposal;
    int looping;
    int ms_delay;
};

class Q_EXPORT QPNGImagePacker : public QPNGImageWriter {
public:
    QPNGImagePacker(QIODevice*, int depth, int convflags);

    void setPixelAlignment(int x);
    bool packImage(const QImage& img);

private:
    QImage previous;
    int depth;
    int convflags;
    int alignx;
};

#endif // QT_FEATURE_IMAGIO_PNG

#endif // QPNGIO_H
