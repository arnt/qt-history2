/****************************************************************************
**
** Definition of PNG QImage IOHandler.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPNGIO_H
#define QPNGIO_H

#ifndef QT_H
#include "qimage.h"
#endif // QT_H

#ifndef QT_NO_IMAGEIO_PNG

void qInitPngIO();

class QIODevice;

#ifndef Q_PNGEXPORT
#if !defined(QT_PLUGIN)
#define Q_PNGEXPORT Q_EXPORT
#else
#define Q_PNGEXPORT
#endif
#endif

class Q_PNGEXPORT QPNGImageWriter {
public:
    QPNGImageWriter(QIODevice*);
    ~QPNGImageWriter();

    enum DisposalMethod { Unspecified, NoDisposal, RestoreBackground, RestoreImage };
    void setDisposalMethod(DisposalMethod);
    void setLooping(int loops=0); // 0 == infinity
    void setFrameDelay(int msecs);
    void setGamma(float);

    bool writeImage(const QImage& img, int x, int y);
    bool writeImage(const QImage& img, int quality, int x, int y);
    bool writeImage(const QImage& img)
	{ return writeImage(img, 0, 0); }
    bool writeImage(const QImage& img, int quality)
	{ return writeImage(img, quality, 0, 0); }

    QIODevice* device() { return dev; }

private:
    QIODevice* dev;
    int frames_written;
    DisposalMethod disposal;
    int looping;
    int ms_delay;
    float gamma;
};

class Q_PNGEXPORT QPNGImagePacker : public QPNGImageWriter {
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

#endif // QT_NO_IMAGEIO_PNG

#endif // QPNGIO_H
