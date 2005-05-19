/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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

            penAlpha = qBound(meta.value("fade minimum").toInt(),
                              penAlpha + meta.value("fade").toInt(), 255);
            brushAlpha = qBound(meta.value("fade minimum").toInt(),
                              brushAlpha + meta.value("fade").toInt(), 255);

            penColor.setAlpha(qBound(0, penAlpha, 255));
            brushColor.setAlpha(qBound(0, brushAlpha, 255));
            pen.setColor(penColor);
            brush.setColor(brushColor);

            if (penAlpha == 0 && brushAlpha == 0) {
                meta["destroy"] = true;
                meta.remove("fade");
            } else if (penAlpha == 255 && brushAlpha == 255)
                meta.remove("fade");

            updated = true;
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
            } else {
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
            }
            updated = true;
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
    QFontMetrics fm(font);
    textRect = fm.boundingRect(QRect(QPoint(0, 0), maxSize.toSize()),
        alignment, text);

    qreal textWidth = qMax(fm.width(text), textRect.width());
    qreal textHeight = qMax(fm.height(), textRect.height());

    qreal scale = qMin(maxSize.width()/textWidth,
                       maxSize.height()/textHeight);

    font.setPointSizeF(font.pointSizeF() * scale);
    fm = QFontMetrics(font);
    textRect = fm.boundingRect(QRect(QPoint(0, 0), maxSize.toSize()),
        alignment, text);
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
    QFontMetrics fm(font);
    QRect rect = textRect;
    rect.moveTo(pos.toPoint());
    painter->save();
    painter->setRenderHint(QPainter::TextAntialiasing);
    painter->setPen(pen);
    painter->setFont(font);
    painter->drawText(rect, alignment, text);
    painter->restore();
}

QRectF TitleShape::rect() const
{
    QFontMetrics fm(font);
    return QRectF(pos, QSizeF(textRect.width(), textRect.height()));
}

ImageShape::ImageShape(const QImage &original, const QPointF &position,
                       const QSizeF &maxSize, int alpha,
                       Qt::Alignment alignment)
    : DisplayShape(position, maxSize), alpha(alpha), alignment(alignment)
{
    source = original.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    scale = qMin(qMin(maxSize.width()/source.width(),
                      maxSize.height()/source.height()), 1.0);
    image = QImage(int(scale * source.width()),
                   int(scale * source.height()),
                   QImage::Format_ARGB32_Premultiplied);

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
    image.fill(qRgba(255, 255, 255, alpha));

    QPainter painter;
    painter.begin(&image);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.scale(scale, scale);
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

DocumentShape::DocumentShape(const QString &text, const QFont &f,
                       const QPen &pen, const QPointF &position,
                       const QSizeF &maxSize)
    : DisplayShape(position, maxSize), font(f), pen(pen)
{
    QFontMetrics fm(font);
    qreal scale = qMax(maxSize.height()/(fm.lineSpacing()*20), 1.0);

    font.setPointSizeF(font.pointSizeF() * scale);

    paragraphs = text.split("\n", QString::SkipEmptyParts);
    formatText();
}

DocumentShape::~DocumentShape()
{
    qDeleteAll(layouts);
    layouts.clear();
}

bool DocumentShape::animate()
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

void DocumentShape::formatText()
{
    qDeleteAll(layouts);
    layouts.clear();

    QFontMetrics fm(font);
    qreal lineHeight = fm.height();
    qreal y = 0.0;
    qreal leftMargin = 0.0;
    qreal rightMargin = maxSize.width();
    qreal bottomMargin = maxSize.height() - lineHeight;

    foreach (QString paragraph, paragraphs) {

        QTextLayout *textLayout = new QTextLayout(paragraph, font);
        textLayout->beginLayout();

        while (y < bottomMargin) {
            QTextLine line = textLayout->createLine();
            if (!line.isValid())
                break;

            line.setLineWidth(rightMargin - leftMargin);
            line.setPosition(QPointF(leftMargin, y));
            y += line.height();
        }

        textLayout->endLayout();
        layouts.append(textLayout);

        y += lineHeight;

        if (y >= bottomMargin)
            break;
    }

    maxSize.setHeight(y);
}

void DocumentShape::paint(QPainter *painter) const
{
    painter->save();
    painter->setRenderHint(QPainter::TextAntialiasing);
    painter->setPen(pen);
    painter->setFont(font);
    foreach (QTextLayout *layout, layouts)
        layout->draw(painter, pos);
    painter->restore();
}

QRectF DocumentShape::rect() const
{
    return QRectF(pos, maxSize);
}
