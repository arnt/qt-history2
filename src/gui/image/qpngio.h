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

#ifndef QPNGIO_H
#define QPNGIO_H

#include "QtGui/qimage.h"

#ifndef QT_NO_IMAGEIO_PNG

void qInitPngIO();

class QIODevice;

#ifndef Q_PNGEXPORT
#if !defined(QT_PLUGIN)
#define Q_PNGEXPORT Q_GUI_EXPORT
#else
#define Q_PNGEXPORT
#endif
#endif

class Q_PNGEXPORT QPNGImageWriter {
public:
    explicit QPNGImageWriter(QIODevice*);
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
    QPNGImagePacker(QIODevice*, int depth, Qt::ImageConversionFlags flags);

    void setPixelAlignment(int x);
    bool packImage(const QImage& img);

private:
    QImage previous;
    int depth;
    Qt::ImageConversionFlags convflags;
    int alignx;
};

#endif // QT_NO_IMAGEIO_PNG

#endif // QPNGIO_H
