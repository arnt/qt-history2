/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** WARNING:
**      A separate license from Unisys may be required to use the gif
**      reader. See http://www.unisys.com/about__unisys/lzw/
**      for information from Unisys
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

    bool canRead() const;
    bool read(QImage *image);
    bool write(const QImage &image);

    QByteArray name() const;

    static bool canRead(QIODevice *device);

    QVariant option(ImageOption option) const;
    void setOption(ImageOption option, const QVariant &value);
    bool supportsOption(ImageOption option) const;

    int loopCount() const;
    int nextImageDelay() const;
    int currentImageNumber() const;

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
