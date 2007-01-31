/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTIFFHANDLER_H
#define QTIFFHANDLER_H

#include <QtGui/qimageiohandler.h>

class QTiffHandler : public QImageIOHandler
{
public:
    QTiffHandler();

    bool canRead() const;
    bool read(QImage *image);
    bool write(const QImage &image);

    QByteArray name() const;

    static bool canRead(QIODevice *device);

    QVariant option(ImageOption option) const;
    void setOption(ImageOption option, const QVariant &value);
    bool supportsOption(ImageOption option) const;

    enum Compression {
        NoCompression = 0,
        LZWCompression = 1
    };
private:
    void convert32BitOrder(const void *source, void *destination, int width);
    int compression;
};

#endif // QTIFFHANDLER_H
