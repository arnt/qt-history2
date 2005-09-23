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
#include "qsvgrenderer.h"
#include "qsvgrenderer_p.h"

#include "qsvgtinydocument_p.h"

#include "QtCore/qbytearray.h"
#include "QtCore/qdebug.h"
#include "private/qobject_p.h"


class QSvgRendererPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QSvgRenderer)
public:
    explicit QSvgRendererPrivate()
        : QObjectPrivate(),
          render(0), currentFrame(0),
          fps(0)
    {}
    void init()
    {
    }
    QSvgTinyDocument *render;
    int currentFrame;
    int fps;
};

QSvgRenderer::QSvgRenderer(QObject *parent)
    : QObject(*(new QSvgRendererPrivate), parent)
{
    Q_D(QSvgRenderer);
    d->init();
}

QSvgRenderer::QSvgRenderer(const QString &filename, QObject *parent)
    : QObject(*new QSvgRendererPrivate, parent)
{
    Q_D(QSvgRenderer);
    d->init();
    load(filename);
}

QSvgRenderer::QSvgRenderer(const QByteArray &contents, QObject *parent )
    : QObject(*new QSvgRendererPrivate, parent)
{
    Q_D(QSvgRenderer);
    d->init();
    load(contents);
}

QSvgRenderer::~QSvgRenderer()
{

}

bool QSvgRenderer::isValid() const
{
    Q_D(const QSvgRenderer);
    return d->render;
}

QSize QSvgRenderer::defaultSize() const
{
    Q_D(const QSvgRenderer);
    if (d->render)
        return d->render->size();
    else
        return QSize();
}

QRect QSvgRenderer::viewBox() const
{
    Q_D(const QSvgRenderer);
    if (d->render)
        return d->render->viewBox();
    else
        return QRect();
}

void QSvgRenderer::setViewBox(const QRect &viewbox)
{
    Q_D(QSvgRenderer);
    if (d->render)
        d->render->setViewBox(viewbox);
}

bool QSvgRenderer::animated() const
{
    return false;
}

int QSvgRenderer::framesPerSecond() const
{
    Q_D(const QSvgRenderer);
    return d->fps;
}

void QSvgRenderer::setFramesPerSecond(int num)
{
    Q_D(QSvgRenderer);
    d->fps = num;
}

int QSvgRenderer::currentFrame() const
{
    Q_D(const QSvgRenderer);
    return d->currentFrame;
}

void QSvgRenderer::setCurrentFrame(int frame)
{
    Q_D(QSvgRenderer);
    d->currentFrame = frame;
}

int QSvgRenderer::animationDuration() const
{
    return 0;
}

bool QSvgRenderer::load(const QString &filename)
{
    Q_D(QSvgRenderer);
    d->render = QSvgTinyDocument::load(filename);
    return d->render;
}

bool QSvgRenderer::load(const QByteArray &contents)
{
    Q_D(QSvgRenderer);
    d->render = QSvgTinyDocument::load(contents);
    return d->render;
}

void QSvgRenderer::render(QPainter *p)
{
    Q_D(QSvgRenderer);
    if (d->render) {
        d->render->draw(p);
    }
}

#include "moc_qsvgrenderer.cpp"
