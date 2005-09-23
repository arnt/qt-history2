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
#include "qsvgview.h"

#include <qsvgrenderer.h>

#include <qapplication.h>
#include <qpainter.h>
#include <qimage.h>
#include <qevent.h>
#include <qdebug.h>

QSvgRasterView::QSvgRasterView(const QString &file, QWidget *parent)
    : QWidget(parent)
{
    doc = new QSvgRenderer(file, this);
}

void QSvgRasterView::paintEvent(QPaintEvent *)
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

QSize QSvgRasterView::sizeHint() const
{
    if (doc)
        return doc->defaultSize();
    return QWidget::sizeHint();
}

void QSvgRasterView::wheelEvent(QWheelEvent *e)
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

QSvgNativeView::QSvgNativeView(const QString &file, QWidget *parent)
    : QWidget(parent)
{
    doc = new QSvgRenderer(file, this);
}

void QSvgNativeView::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    doc->render(&p);
}

QSize QSvgNativeView::sizeHint() const
{
    if (doc)
        return doc->defaultSize();
    return QWidget::sizeHint();
}

void QSvgNativeView::wheelEvent(QWheelEvent *e)
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

QSvgGLView::QSvgGLView(const QString &file, QWidget *parent)
    : QGLWidget(parent)
{
    doc = new QSvgRenderer(file, this);
}

void QSvgGLView::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    doc->render(&p);
}

QSize QSvgGLView::sizeHint() const
{
    if (doc)
        return doc->defaultSize();
    return QGLWidget::sizeHint();
}

void QSvgGLView::wheelEvent(QWheelEvent *e)
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
