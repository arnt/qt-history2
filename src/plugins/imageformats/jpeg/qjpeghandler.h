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

#ifndef QJPEGHANDLER_H
#define QJPEGHANDLER_H

#include <QtGui/qimageiohandler.h>
#include <QtCore/QSize>

QT_BEGIN_NAMESPACE

class QJpegHandler : public QImageIOHandler
{
public:
    QJpegHandler();

    bool canRead() const;
    bool read(QImage *image);
    bool write(const QImage &image);

    QByteArray name() const;

    static bool canRead(QIODevice *device);

    QVariant option(ImageOption option) const;
    void setOption(ImageOption option, const QVariant &value);
    bool supportsOption(ImageOption option) const;

private:
    int quality;
    QByteArray parameters;
    QSize scaledSize;
};

QT_END_NAMESPACE

#endif // QJPEGHANDLER_H
