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

#ifndef QPNGHANDLER_H
#define QPNGHANDLER_H

#include "qimageiohandler.h"

class Q_GUI_EXPORT QPngHandler : public QImageIOHandler
{
public:
    QPngHandler();

    bool canLoadImage() const;
    bool load(QImage *image);
    bool save(const QImage &image);

    QByteArray name() const;

    QVariant property(ImageProperty property) const;
    void setProperty(ImageProperty property, const QVariant &value);
    bool supportsProperty(ImageProperty property) const;

    static bool canLoadImage(QIODevice *device);

private:
    float gamma;
    int quality;
};

#endif
