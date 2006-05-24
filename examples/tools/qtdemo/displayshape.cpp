/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <math.h>
#include <QtGui>

#include "displayshape.h"

DisplayShape::DisplayShape(const QPointF &position, const QSizeF &maxSize)
    : pos(position), maxSize(maxSize), interactive(false)
{
}

bool DisplayShape::animate()
{
    if (!targetPos.isNull()) {
        QLineF displacement(pos, targetPos);
        QPointF newPosition = displacement.pointAt(0.25);
        if (displacement.length() <= 1.0) {
            pos = targetPos;
            targetPos = QPointF();
        } else {
            pos = newPosition;
        }
        return true;
    }

    return false;
}

bool DisplayShape::contains(const QString &key) const
{
    return meta.contains(key);
}

bool DisplayShape::isInteractive() const
{
    return interactive;
}

QVariant DisplayShape::metaData(const QString &key) const
{
    return meta.value(key);
}

void DisplayShape::paint(QPainter *painter) const
{
    painter->save();
    painter->drawImage(pos, image);
    painter->restore();
}

QPointF DisplayShape::position() const
{
    return pos;
}

QRectF DisplayShape::rect() const
{
    return QRectF(pos, image.size());
}

void DisplayShape::removeMetaData(const QString &key)
{
    meta.remove(key);
}

void DisplayShape::setInteractive(bool enable)
{
    interactive = enable;
}

void DisplayShape::setMetaData(const QString &key, const QVariant &value)
{
    meta[key] = value;
}

void DisplayShape::setPosition(const QPointF &position)
{
    pos = position;
}

void DisplayShape::setSize(const QSizeF &size)
{
    maxSize = size;
}

void DisplayShape::setTarget(const QPointF &position)
{
    targetPos = position;
}

QSizeF DisplayShape::size() const
{
    return maxSize;
}

QPointF DisplayShape::target() const
{
    return targetPos;
}

PanelShape::PanelShape(const QPainterPath &path, const QBrush &normal,
                       const QBrush &highlighted, const QPen &pen,
                       const QPointF &position, const QSizeF &maxSize)
    : DisplayShape(position, maxSize), highlightedBrush(highlighted),
      normalBrush(normal), path(path), pen(pen)
{
    brush = normalBrush;
}

bool PanelShape::animate()
{
    bool updated = false;

    if (!meta.contains("destroy")) {
        if (meta.contains("fade")) {
            QColor penColor = pen.color();
            QColor brushColor = brush.color();
            int penAlpha = penColor.alpha();
            int brushAlpha = brushColor.alpha();
            int fadeMinimum = meta.value("fade minimum").toInt();

            if (penAlpha != fadeMinimum || brushAlpha != fadeMinimum
                || meta.value("fade").toInt() > 0) {

                penAlpha = qBound(fadeMinimum,
                                  penAlpha + meta.value("fade").toInt(), 255);
                brushAlpha = qBound(fadeMinimum,
                                  brushAlpha + meta.value("fade").toInt(), 255);

                penColor.setAlpha(penAlpha);
                brushColor.setAlpha(brushAlpha);
                pen.setColor(penColor);
                brush.setColor(brushColor);

                if (penAlpha == 0 && brushAlpha == 0) {
                    meta["destroy"] = true;
                    meta.remove("fade");
                } else if (penAlpha == 255 && brushAlpha == 255)
                    meta.remove("fade");

                updated = true;
            }
        } else if (meta.contains("highlight")) {
            qreal scale = meta.value("highlight scale").toDouble();
            QColor color = brush.color();

            if (meta.value("highlight").toBool())
                scale = qBound(0.0, scale + 0.5, 1.0);
            else
                scale = qBound(0.0, scale - 0.2, 1.0);

            if (scale == 0.0) {
                brush = normalBrush;
                meta.remove("highlight");
                meta.remove("highlight scale");
                updated = true;
            } else if (scale != meta["highlight scale"]) {
                meta["highlight scale"] = scale;

                if (scale == 1.0)
                    brush = highlightedBrush;
                else {
                    QColor normal = normalBrush.color();
                    QColor highlighted = highlightedBrush.color();

                    color.setRedF((1.0-scale) * normal.redF()
                                  + scale*highlighted.redF());
                    color.setGreenF((1.0-scale) * normal.greenF()
                                    + scale*highlighted.greenF());
                    color.setBlueF((1.0-scale) * normal.blueF()
                                   + scale*highlighted.blueF());
                    brush.setColor(color);
                }
                updated = true;
            }
        }
    }

    return DisplayShape::animate() || updated;
}

void PanelShape::paint(QPainter *painter) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(brush);
    painter->setPen(pen);
    painter->translate(pos);
    painter->drawPath(path);
    painter->restore();
}

QRectF PanelShape::rect() const
{
    return QRectF(pos + path.boundingRect().topLeft(),
                  path.boundingRect().size());
}

TitleShape::TitleShape(const QString &text, const QFont &f,
                       const QPen &pen, const QPointF &position,
                       const QSizeF &maxSize, Qt::Alignment alignment)
    : DisplayShape(position, maxSize), font(f), text(text), pen(pen),
      alignment(alignment)
{
    QFontMetricsF fm(font);
    textRect = fm.boundingRect(QRectF(QPointF(0, 0), maxSize), alignment, text);

    qreal textWidth = qMax(fm.width(text), textRect.width());
    qreal textHeight = qMax(fm.height(), textRect.height());

    qreal scale = qMin(maxSize.width()/textWidth,
                       maxSize.height()/textHeight);

    font.setPointSizeF(font.pointSizeF() * scale);
    fm = QFontMetricsF(font);
    textRect = fm.boundingRect(QRectF(QPointF(0, 0), maxSize), alignment, text);
    baselineStart = QPointF(textRect.left(), textRect.bottom() - fm.descent());
}

bool TitleShape::animate()
{
    bool updated = false;

    if (!meta.contains("destroy")) {
        if (meta.contains("fade")) {
            QColor penColor = pen.color();
            int penAlpha = penColor.alpha();

            penAlpha = qBound(meta.value("fade minimum").toInt(),
                              penAlpha + meta.value("fade").toInt(), 255);

            penColor.setAlpha(penAlpha);
            pen.setColor(penColor);

            if (penAlpha == 0) {
                meta["destroy"] = true;
                meta.remove("fade");
            } else if (penAlpha == 255)
                meta.remove("fade");

            updated = true;
        }
    }

    return DisplayShape::animate() || updated;
}

void TitleShape::paint(QPainter *painter) const
{
    QRectF rect(textRect);
    rect.translate(pos);
    painter->save();
    painter->setRenderHint(QPainter::TextAntialiasing);
    painter->setPen(pen);
    painter->setFont(font);
    painter->drawText(pos + baselineStart, text);
    painter->restore();
}

QRectF TitleShape::rect() const
{
    QRectF rect(textRect);
    return rect.translated(pos);
}

ImageShape::ImageShape(const QImage &original, const QPointF &position,
                       const QSizeF &maxSize, int alpha,
                       Qt::Alignment alignment)
    : DisplayShape(position, maxSize), alpha(alpha), alignment(alignment)
{
    source = original.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    qreal scale = qMin(qMin(maxSize.width()/source.width(),
                            maxSize.height()/source.height()), qreal(1.0));

    source = source.scaled(int(ceil(source.width() * scale)),
                           int(ceil(source.height() * scale)),
                           Qt::KeepAspectRatio,
                           Qt::SmoothTransformation);

    image = QImage(source.size(), QImage::Format_ARGB32_Premultiplied);

    offset = QPointF(0.0, 0.0);

    if (alignment & Qt::AlignHCenter)
        offset.setX((maxSize.width() - image.width())/2);
    else if (alignment & Qt::AlignRight)
        offset.setX(maxSize.width() - image.width());

    if (alignment & Qt::AlignVCenter)
        offset.setY((maxSize.height() - image.height())/2);
    else if (alignment & Qt::AlignBottom)
        offset.setY(maxSize.height() - image.height());

    redraw();
}

void ImageShape::redraw()
{
    image.fill(qRgba(alpha, alpha, alpha, alpha));

    QPainter painter;
    painter.begin(&image);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.drawImage(0, 0, source);
    painter.end();
}

void ImageShape::paint(QPainter *painter) const
{
    painter->drawImage(pos + offset, image);
}

QRectF ImageShape::rect() const
{
    return QRectF(pos, maxSize);
}

bool ImageShape::animate()
{
    bool updated = false;

    if (!meta.contains("destroy")) {
        if (meta.contains("fade")) {
            alpha = qBound(meta.value("fade minimum").toInt(),
                           alpha + meta.value("fade").toInt(), 255);
            redraw();

            if (alpha == 0) {
                meta["destroy"] = true;
                meta.remove("fade");
            } else if (alpha == 255)
                meta.remove("fade");

            updated = true;
        }
    }

    return DisplayShape::animate() || updated;
}

DocumentShape::DocumentShape(const QString &text, const QFont &font,
                             const QPointF &position, const QSizeF &maxSize,
                             int alpha)
    : DisplayShape(position, maxSize), alpha(alpha)
{
    textDocument.setHtml(text);
    textDocument.setDefaultFont(font);
    textDocument.setPageSize(maxSize);
    QSizeF documentSize = textDocument.documentLayout()->documentSize();
    setSize(QSizeF(maxSize.width(),
                      qMin(maxSize.height(), documentSize.height())));

    source = QImage(int(ceil(documentSize.width())),
                    int(ceil(documentSize.height())),
                    QImage::Format_ARGB32_Premultiplied);
    source.fill(qRgba(255, 255, 255, 255));

    QAbstractTextDocumentLayout::PaintContext context;
    textDocument.documentLayout()->setPaintDevice(&source);

    QPainter painter;
    painter.begin(&source);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setRenderHint(QPainter::Antialiasing);
    textDocument.documentLayout()->draw(&painter, context);
    painter.end();

    source = source.scaled(int(ceil(maxSize.width())),
                           int(ceil(maxSize.height())),
                           Qt::KeepAspectRatio,
                           Qt::SmoothTransformation);

    image = QImage(source.size(), source.format());
    redraw();
}

DocumentShape::~DocumentShape()
{
}

bool DocumentShape::animate()
{
    bool updated = false;

    if (!meta.contains("destroy")) {
        if (meta.contains("fade")) {
            alpha = qBound(meta.value("fade minimum").toInt(),
                           alpha + meta.value("fade").toInt(), 255);
            redraw();

            if (alpha == 0) {
                meta["destroy"] = true;
                meta.remove("fade");
            } else if (alpha == 255)
                meta.remove("fade");

            updated = true;
        }
    }

    return DisplayShape::animate() || updated;
}

void DocumentShape::redraw()
{
    image.fill(qRgba(alpha, alpha, alpha, alpha));

    QPainter painter;
    painter.begin(&image);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.drawImage(0, 0, source);
    painter.end();
}

void DocumentShape::paint(QPainter *painter) const
{
    painter->drawImage(pos, image);
}

QRectF DocumentShape::rect() const
{
    return QRectF(pos, maxSize);
}
