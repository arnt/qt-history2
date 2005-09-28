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
#include "svgview.h"

#include <QSvgRenderer>

#include <QApplication>
#include <QPainter>
#include <QImage>
#include <QWheelEvent>
#include <QtDebug>

SvgRasterView::SvgRasterView(const QString &file, QWidget *parent)
    : QWidget(parent)
{
    doc = new QSvgRenderer(file, this);
}

void SvgRasterView::paintEvent(QPaintEvent *)
{
    if (buffer.size() != size()) {
        buffer = QImage(size(), QImage::Format_ARGB32_Premultiplied);
        QPainter p(&buffer);
        p.setViewport(0, 0, width(), height());
        p.eraseRect(0, 0, width(), height());
        doc->render(&p);
    }
    QPainter pt(this);
    pt.drawImage(0, 0, buffer);
}

QSize SvgRasterView::sizeHint() const
{
    if (doc)
        return doc->defaultSize();
    return QWidget::sizeHint();
}

void SvgRasterView::wheelEvent(QWheelEvent *e)
{
    const double diff = 0.1;
    QSize size = doc->defaultSize();
    int width  = size.width();
    int height = size.height();
    if (e->delta() > 0) {
        width = int(this->width()+this->width()*diff);
        height = int(this->height()+this->height()*diff);
    } else {
        width  = int(this->width()-this->width()*diff);
        height = int(this->height()-this->height()*diff);
    }

    resize(width, height);
}

SvgNativeView::SvgNativeView(const QString &file, QWidget *parent)
    : QWidget(parent)
{
    doc = new QSvgRenderer(file, this);
}

void SvgNativeView::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setViewport(0, 0, width(), height());
    doc->render(&p);
}

QSize SvgNativeView::sizeHint() const
{
    if (doc)
        return doc->defaultSize();
    return QWidget::sizeHint();
}

void SvgNativeView::wheelEvent(QWheelEvent *e)
{
    const double diff = 0.1;
    QSize size = doc->defaultSize();
    int width  = size.width();
    int height = size.height();
    if (e->delta() > 0) {
        width = int(this->width()+this->width()*diff);
        height = int(this->height()+this->height()*diff);
    } else {
        width  = int(this->width()-this->width()*diff);
        height = int(this->height()-this->height()*diff);
    }
    resize(width, height);
}

#ifndef QT_NO_OPENGL
SvgGLView::SvgGLView(const QString &file, QWidget *parent)
    : QGLWidget(parent)
{
    doc = new QSvgRenderer(file, this);
}

void SvgGLView::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    doc->render(&p);
}

QSize SvgGLView::sizeHint() const
{
    if (doc)
        return doc->defaultSize();
    return QGLWidget::sizeHint();
}

void SvgGLView::wheelEvent(QWheelEvent *e)
{
    const double diff = 0.1;
    QSize size = doc->defaultSize();
    int width  = size.width();
    int height = size.height();
    if (e->delta() > 0) {
        width = int(this->width()+this->width()*diff);
        height = int(this->height()+this->height()*diff);
    } else {
        width  = int(this->width()-this->width()*diff);
        height = int(this->height()-this->height()*diff);
    }
    resize(width, height);
}
#endif
