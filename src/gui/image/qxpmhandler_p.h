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

class Q_GUI_EXPORT QXpmHandler : public QImageIOHandler
{
public:
    bool canLoadImage() const;
    bool load(QImage *image);
    bool save(const QImage &image);

    static bool canLoadImage(QIODevice *device);

    QByteArray name() const;

    QVariant property(ImageProperty property) const;
    void setProperty(ImageProperty property, const QVariant &value);
    bool supportsProperty(ImageProperty property) const;

private:
    QString fileName;
};

#endif
