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

#ifndef QPPMHANDLER_H
#define QPPMHANDLER_H

#include "qimageiohandler.h"

#ifndef QT_NO_IMAGEFORMAT_PPM

class QByteArray;
class Q_GUI_EXPORT QPpmHandler : public QImageIOHandler
{
public:
    bool canRead() const;
    bool read(QImage *image);
    bool write(const QImage &image);

    QByteArray name() const;

    static bool canRead(QIODevice *device, QByteArray *subType = 0);

    QVariant option(ImageOption option) const;
    void setOption(ImageOption option, const QVariant &value);
    bool supportsOption(ImageOption option) const;

private:
    mutable QByteArray subType;
};

#endif // QT_NO_IMAGEFORMAT_PPM
#endif
