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
#include <qdebug.h>
#include <private/qtextengine_p.h>
#include <qpalette.h>
#include <qtextbrowser.h>

// set by the mime source factory in Qt3Compat
QTextImageHandler::ExternalImageLoaderFunction QTextImageHandler::externalLoader = 0;

static QPixmap getPixmap(const QTextDocument *doc, const QTextImageFormat &format)
{
    QPixmap pm;

    const bool hasWidth = format.hasProperty(QTextFormat::ImageWidth);
    const int width = format.width();
    const bool hasHeight = format.hasProperty(QTextFormat::ImageHeight);
    const int height = format.height();

    QString key = QString("$qt_rt_%1_%2_%3").arg(format.name()).arg(width).arg(height);
    if (!QPixmapCache::find(key, pm)) {
        QImage img;
        const QString name = format.name();

        QString context;
        if (QTextBrowser *browser = qt_cast<QTextBrowser *>(doc->parent())) {
            QVariant data = browser->loadResource(QTextBrowser::ImageResource, name);
            if (data.type() == QVariant::Pixmap) {
                pm = data.toPixmap();
                QPixmapCache::insert(key, pm);
                return pm;
            } else if (data.type() == QVariant::Image) {
                img = data.toImage();
            } else if (data.type() == QVariant::ByteArray) {
                img.loadFromData(data.toByteArray());
            }
            context = browser->source().toString();
        }

        if (img.isNull() && QTextImageHandler::externalLoader)
            img = QTextImageHandler::externalLoader(name, context);

        if (img.isNull()) // try direct loading
            if (!img.load(name))
                return pm;

        QSize size = img.size();
        if (hasWidth)
            size.setWidth(width);
        if (hasHeight)
            size.setHeight(height);
        if (size.isValid() && img.size() != size)
            img = img.scale(size);

        pm.fromImage(img);
        QPixmapCache::insert(key, pm);
    }
    return pm;
}

QTextImageHandler::QTextImageHandler(QObject *parent)
    : QObject(parent)
{
}

QSize QTextImageHandler::intrinsicSize(const QTextDocument *doc, const QTextFormat &format)
{
    const QTextImageFormat imageFormat = format.toImageFormat();

    return getPixmap(doc, imageFormat).size();
}

void QTextImageHandler::drawObject(QPainter *p, const QRect &rect, const QTextDocument *doc, const QTextFormat &format)
{
    const QTextImageFormat imageFormat = format.toImageFormat();
    const QPixmap pixmap = getPixmap(doc, imageFormat);

    p->drawPixmap(rect, pixmap);
}

