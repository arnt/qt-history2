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

#include "qsvgtinydocument_p.h"

#include "qbytearray.h"
#include "qdebug.h"
#include "private/qobject_p.h"


/*!
    \class QSvgRenderer
    \brief The QSvgRenderer class is used to draw the contents of SVG files onto paint devices.
    \since 4.1

    Using QSvgRenderer, Scalable Vector Graphics (SVG) can be rendered onto any QPaintDevice
    subclass, including QWidget, QImage, and QGLWidget. 

    QSvgRenderer provides an API that supports basic features of SVG rendering, such as loading
    and rendering of static drawings, and more interactive features like animation. Since the
    rendering is performed using QPainter, SVG drawings can be rendered on any subclass of
    QPaintDevice.

    SVG drawings are either loaded when an QSvgRenderer is constructed, or loaded later
    using the load() functions. Data is either supplied directly as serialized XML, or
    indirectly using a file name. If a valid file has been loaded, either when the renderer
    is constructed or at some later time, isValid() returns true; otherwise it returns false.

    The defaultSize() function provides information about the amount of space that is required
    to render the currently loaded SVG file. This is useful for paint devices, such as QWidget,
    that often need to supply a size hint to their parent layout.
    The default size of a drawing may differ from its visible area, found using the \l viewBox
    property.

    Animated SVG drawings are supported, and can be controlled with a simple collection of
    functions and properties:

    \list
    \o The animated() function indicates whether a drawing contains animation information.
    \o The animationDuration() function provides the duration in milliseconds of the
       animation, without taking any looping into account.
    \o The \l currentFrame property contains the current frame of the animation.
    \o The \l framesPerSecond property contains the rate at which the animation runs.
    \endlist

    \sa QSvgWidget, {QtSvg Module}, QPicture
*/

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

/*!
    Constructs a new renderer with the given \a parent.
*/
QSvgRenderer::QSvgRenderer(QObject *parent)
    : QObject(*(new QSvgRendererPrivate), parent)
{
    Q_D(QSvgRenderer);
    d->init();
}

/*!
    Constructs a new renderer with the given \a parent and loads the contents of the
    SVG file with the specified \a filename.
*/
QSvgRenderer::QSvgRenderer(const QString &filename, QObject *parent)
    : QObject(*new QSvgRendererPrivate, parent)
{
    Q_D(QSvgRenderer);
    d->init();
    load(filename);
}

/*!
    Constructs a new renderer with the given \a parent and loads the specified SVG format
    \a contents.
*/
QSvgRenderer::QSvgRenderer(const QByteArray &contents, QObject *parent )
    : QObject(*new QSvgRendererPrivate, parent)
{
    Q_D(QSvgRenderer);
    d->init();
    load(contents);
}

/*!
    Destroys the renderer.
*/
QSvgRenderer::~QSvgRenderer()
{

}

/*!
    Returns true if there is a valid current document; otherwise returns false.
*/
bool QSvgRenderer::isValid() const
{
    Q_D(const QSvgRenderer);
    return d->render;
}

/*!
    Returns the default size of the document contents.
*/
QSize QSvgRenderer::defaultSize() const
{
    Q_D(const QSvgRenderer);
    if (d->render)
        return d->render->size();
    else
        return QSize();
}

/*!
    \property QSvgRenderer::viewBox
    \brief the rectangle specifying the visible area of the document in logical coordinates
*/
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

/*!
    Returns true if the current document contains animated elements; otherwise returns false.

    \sa currentFrame(), animationDuration(), framesPerSecond()
*/
bool QSvgRenderer::animated() const
{
    return false;
}

/*!
    \property QSvgRenderer::framesPerSecond
    \brief the number of frames per second to be shown

    The number of frames per second is 0 if the current document is not animated.

    \sa currentFrame, animationDuration(), animated()
*/
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

/*!
    \property QSvgRenderer::currentFrame
    \brief the current frame of the document's animation, or 0 if the document is not animated

    \sa animationDuration(), framesPerSecond, animated()
*/
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

/*!
    Returns the number of frames in the animation, or 0 if the current document is not
    animated.

    \sa animated(), currentFrame(), framesPerSecond()
*/
int QSvgRenderer::animationDuration() const
{
    return 0;
}

/*!
    Loads the SVG file specified by \a filename.
*/
bool QSvgRenderer::load(const QString &filename)
{
    Q_D(QSvgRenderer);
    d->render = QSvgTinyDocument::load(filename);
    return d->render;
}

/*!
    Loads the specified SVG format \a contents.
*/
bool QSvgRenderer::load(const QByteArray &contents)
{
    Q_D(QSvgRenderer);
    d->render = QSvgTinyDocument::load(contents);
    return d->render;
}

/*!
    \fn void QSvgRenderer::render(QPainter *painter)

    Renders the document, or current frame of an animated document, using the given
    \a painter.
*/
void QSvgRenderer::render(QPainter *p)
{
    Q_D(QSvgRenderer);
    if (d->render) {
        d->render->draw(p);
    }
}

/*!
    \fn void QSvgRenderer::repaintNeeded()

    This signal is emitted whenever the document needs to be updated, usually
    for the purposes of animation.
*/

#include "moc_qsvgrenderer.cpp"
