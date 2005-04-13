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

static QPixmap getPixmap(QTextDocument *doc, const QTextImageFormat &format)
{
    QPixmap pm;

    const bool hasWidth = format.hasProperty(QTextFormat::ImageWidth);
    const int width = qRound(format.width());
    const bool hasHeight = format.hasProperty(QTextFormat::ImageHeight);
    const int height = qRound(format.height());

    QString name = format.name();
    const QVariant data = doc->resource(QTextDocument::ImageResource, name);
    if (data.type() == QVariant::Pixmap || data.type() == QVariant::Image) {
        pm = qvariant_cast<QPixmap>(data);
    } else if (data.type() == QVariant::ByteArray) {
        pm.loadFromData(data.toByteArray());
    }

    if (pm.isNull()) {
        QTextBrowser *browser = qobject_cast<QTextBrowser *>(doc->parent());
        QString context;
        if (browser)
            context = browser->source().toString();
        QImage img;
        if (QTextImageHandler::externalLoader)
            img = QTextImageHandler::externalLoader(name, context);

        if (img.isNull()) // try direct loading
            if (name.isEmpty() || !img.load(name))
                return QPixmap(":/trolltech/styles/commonstyle/images/file-16.png");
        pm = QPixmap::fromImage(img);
        doc->addResource(QTextDocument::ImageResource, name, pm);
    }

    QSize size = pm.size();
    if (hasWidth)
        size.setWidth(width);
    if (hasHeight)
        size.setHeight(height);
    if (size.isValid() && pm.size() != size)
        pm = pm.scaled(size);

    return pm;
}

QTextImageHandler::QTextImageHandler(QObject *parent)
    : QObject(parent)
{
}

QSizeF QTextImageHandler::intrinsicSize(QTextDocument *doc, const QTextFormat &format)
{
    const QTextImageFormat imageFormat = format.toImageFormat();

    return getPixmap(doc, imageFormat).size();
}

void QTextImageHandler::drawObject(QPainter *p, const QRectF &rect, QTextDocument *doc, const QTextFormat &format)
{
    const QTextImageFormat imageFormat = format.toImageFormat();
    const QPixmap pixmap = getPixmap(doc, imageFormat);

    p->drawPixmap(rect, pixmap, pixmap.rect());
}

