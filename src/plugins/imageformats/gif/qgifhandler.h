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

#ifndef QGIFHANDLER_H
#define QGIFHANDLER_H

#include "qimageiohandler.h"

#include <qbytearray.h>
#include <qimage.h>

class QGIFFormat;
class QGifHandler : public QImageIOHandler
{
public:
    QGifHandler();
    ~QGifHandler();

    bool canLoadImage() const;
    bool load(QImage *image);
    bool save(const QImage &image);

    QByteArray name() const;

    static bool canLoadImage(QIODevice *device);

    QVariant property(ImageProperty property) const;
    void setProperty(ImageProperty property, const QVariant &value);
    bool supportsProperty(ImageProperty property) const;

    int loopCount() const;
    int nextFrameDelay() const;
    int currentFrameNumber() const;

private:
    QGIFFormat *gifFormat;
    QString fileName;
    QByteArray buffer;
    QImage lastImage;

    int nextDelay;
    int loopCnt;
    int frameNumber;
    QSize nextSize;
};

#endif
