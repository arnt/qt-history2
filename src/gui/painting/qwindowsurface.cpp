/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <private/qwindowsurface_p.h>
#include <qwidget.h>
#include <private/qbackingstore_p.h>

class QWindowSurfacePrivate
{
public:
    QWindowSurfacePrivate(QWidget *w) : window(w) {}

    QWidget *window;
    QRect geometry;
    QList<QImage*> bufferImages;
};

/*!
    \class QWindowSurface
    \since 4.3
    \internal
    \preliminary

    \brief The QWindowSurface class provides the drawing area for top-level
    windows.
*/


/*!
    \fn void QWindowSurface::beginPaint(const QRegion &region)

    This function is called before painting onto the surface begins,
    with the \a region in which the painting will occur.

    \sa endPaint(), paintDevice()
*/

/*!
    \fn void QWindowSurface::endPaint(const QRegion &region)

    This function is called after painting onto the surface has ended,
    with the \a region in which the painting was performed.

    \sa beginPaint(), paintDevice()
*/

/*!
    \fn void QWindowSurface::flush(QWidget *widget, const QRegion &region,
                                  const QPoint &offset)

    Flushes the given \a region from the specified \a widget onto the
    screen.

    Note that the \a offset parameter is currently unused.
*/

/*!
    \fn QPaintDevice* QWindowSurface::paintDevice()

    Implement this function to return the appropriate paint device.
*/

/*!
    Constructs an empty surface for the given top-level \a window.
*/
QWindowSurface::QWindowSurface(QWidget *window)
    : d_ptr(new QWindowSurfacePrivate(window))
{
    if (window)
        window->setWindowSurface(this);
}

/*!
    Destroys this surface.
*/
QWindowSurface::~QWindowSurface()
{
    if (d_ptr->window)
        d_ptr->window->d_func()->extra->topextra->windowSurface = 0;
    delete d_ptr;
}

/*!
    Returns a pointer to the top-level window associated with this
    surface.
*/
QWidget* QWindowSurface::window() const
{
    return d_ptr->window;
}

void QWindowSurface::beginPaint(const QRegion &)
{
}

void QWindowSurface::endPaint(const QRegion &)
{
    qDeleteAll(d_ptr->bufferImages);
}

/*!
    Sets the currently allocated area to be the given \a rect.

    This function is called whenever area covered by the top-level
    window changes.

    \sa geometry()
*/
void QWindowSurface::setGeometry(const QRect &rect)
{
    d_ptr->geometry = rect;
}

/*!
    Returns the currently allocated area on the screen.
*/
QRect QWindowSurface::geometry() const
{
    return d_ptr->geometry;
}

/*!
    Scrolls the given \a area \a dx pixels to the right and \a dy
    downward; both \a dx and \a dy may be negative.

    Returns true if the area was scrolled successfully; false otherwise.
*/
bool QWindowSurface::scroll(const QRegion &area, int dx, int dy)
{
    Q_UNUSED(area);
    Q_UNUSED(dx);
    Q_UNUSED(dy);

    return false;
}

/*!
    Returns a QImage pointer which represents the actual buffer the \a widget
    is drawn into or 0 if this is unavailable.

    You must call beginPaint() before you call this function and the returned
    pointer is only valid until endPaint() is called.
*/
QImage* QWindowSurface::buffer(const QWidget *widget)
{
    if (widget->window() != window())
        return 0;

    QPaintDevice *pdev = paintDevice();
    if (!pdev || pdev->devType() != QInternal::Image)
        return 0;

    const QPoint off = offset(widget);
    QImage *img = static_cast<QImage*>(pdev);

    QRect rect(off, widget->size());
    rect &= QRect(QPoint(), img->size());

    if (rect.isEmpty())
        return 0;

    img = new QImage(img->scanLine(rect.y()) + rect.x() * img->depth() / 8,
                     rect.width(), rect.height(),
                     img->bytesPerLine(), img->format());
    d_ptr->bufferImages.append(img);

    return img;
}

/*!
  Returns a QPixmap generated from the part of the backing store
  corresponding to \a widget. Returns a null QPixmap if an error
  occurs. The contents of the pixmap are only defined for the regions
  of \a widget that have received paint events since the last resize
  of the backing store.

  If \a rectangle is a null rectangle (the default), the entire widget
  is grabbed. Otherwise, the grabbed area is limited to \a rectangle.

  The default implementation returns a null pixmap.

  \sa QPixmap::grabWidget()
*/
QPixmap QWindowSurface::grabWidget(const QWidget *widget, const QRect &rectangle) const
{
    Q_UNUSED(widget);
    Q_UNUSED(rectangle);
    return QPixmap();
}

/*!
  Returns the offset of \a widget in the coordinates of this
  window surface.
 */
QPoint QWindowSurface::offset(const QWidget *widget) const
{
    QWidget *window = d_ptr->window;
    QPoint offset = widget->mapTo(window, QPoint());
#ifdef Q_WS_QWS
    offset += window->geometry().topLeft() - window->frameGeometry().topLeft();
#endif
    return offset;
}

/*!
  \fn QRect QWindowSurface::rect(const QWidget *widget) const

  Returns the rectangle for \a widget in the coordinates of this
  window surface.
*/
