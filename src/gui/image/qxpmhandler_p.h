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

#ifndef QXPMHANDLER_H
#define QXPMHANDLER_H

#include "qimageiohandler.h"

#ifndef QT_NO_IMAGEFORMAT_XPM

class Q_GUI_EXPORT QXpmHandler : public QImageIOHandler
{
public:
    bool canRead() const;
    bool read(QImage *image);
    bool write(const QImage &image);

    static bool canRead(QIODevice *device);

    QByteArray name() const;

    QVariant option(ImageOption option) const;
    void setOption(ImageOption option, const QVariant &value);
    bool supportsOption(ImageOption option) const;

private:
    QString fileName;
};

#endif // QT_NO_IMAGEFORMAT_XPM
#endif // QXPMHANDLER_H
