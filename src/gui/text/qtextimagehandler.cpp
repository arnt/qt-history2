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


#include "qtextimagehandler_p.h"

#include <qtextformat.h>
#include <qpainter.h>
#include <qpixmapcache.h>
#include <qdragobject.h>
#include <qdebug.h>
#include <private/qtextengine_p.h>
#include <qpalette.h>

QTextImageHandler::ExternalImageLoaderFunction QTextImageHandler::externalLoader = 0;

static QPixmap getPixmap(const QTextImageFormat &format)
{
    QPixmap pm;

    QSize size;

    if (format.hasProperty(QTextFormat::ImageWidth))
        size.setWidth(format.width());

    if (format.hasProperty(QTextFormat::ImageHeight))
        size.setHeight(format.height());

    QString key = QString("$qt_rt_%1_%2_%3").arg(format.name()).arg(size.width()).arg(size.height());
    if (!QPixmapCache::find(key, pm)) {
        QImage img;
        const QString name = format.name();

        if (QTextImageHandler::externalLoader) {
            // ###
            QString context;
            img = QTextImageHandler::externalLoader(name, context);
        }

        if (img.isNull()) // try direct loading
            if (!img.load(name))
                return pm;

        if (size.isValid() && img.size() != size)
            img = img.smoothScale(size);
        pm.convertFromImage(img);
        QPixmapCache::insert(key, pm);
    }
    return pm;
}

QTextImageHandler::QTextImageHandler(QObject *parent)
    : QObject(parent)
{
}

QSize QTextImageHandler::intrinsicSize(const QTextFormat &format)
{
    QTextImageFormat imageFormat = format.toImageFormat();

    QPixmap pixmap = getPixmap(imageFormat);
    return pixmap.size();
}

void QTextImageHandler::drawObject(QPainter *p, const QRect &rect, const QTextFormat &format)
{
    QTextImageFormat imageFormat = format.toImageFormat();
    QPixmap pixmap = getPixmap(imageFormat);

    p->drawPixmap(rect, pixmap);
}

