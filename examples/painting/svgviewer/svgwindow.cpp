/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>

#include "svgview.h"
#include "svgwindow.h"

SvgWindow::SvgWindow()
    : QScrollArea(),
      fastAntialiasing(false)
{
    QWidget *view = new QWidget(this);
#ifdef QT_NO_OPENGL
    renderer = SvgWindow::Native;
#else
    renderer = SvgWindow::OpenGL;
#endif
    setWidget(view);
}

void SvgWindow::openFile(const QString &file)
{
    currentPath = file;
    setRenderer(renderer);
}

void SvgWindow::setRenderer(RendererType type)
{
    renderer = type;
    QWidget *view;

    if (renderer == OpenGL) {
        #ifndef QT_NO_OPENGL
        view = new SvgGLView(currentPath, this);
        dynamic_cast<SvgGLView *>(view)->setFastAntialiasing(fastAntialiasing);
        #endif
    } else if (renderer == Image) {
        view = new SvgRasterView(currentPath, this);
    } else {
        view = new SvgNativeView(currentPath, this);
    }

    setWidget(view);
    view->show();
}

void SvgWindow::setFastAntialiasing(bool fa)
{
    fastAntialiasing = fa;

    #ifndef QT_NO_OPENGL
    QWidget *view = widget();
    if (renderer == OpenGL)
        dynamic_cast<SvgGLView *>(view)->setFastAntialiasing(fastAntialiasing);
    #endif
}

void SvgWindow::mousePressEvent(QMouseEvent *event)
{
    mousePressPos = event->pos();
    scrollBarValuesOnMousePress.rx() = horizontalScrollBar()->value();
    scrollBarValuesOnMousePress.ry() = verticalScrollBar()->value();
    event->accept();
}

void SvgWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (mousePressPos.isNull()) {
        event->ignore();
        return;
    }

    horizontalScrollBar()->setValue(scrollBarValuesOnMousePress.x() - event->pos().x() + mousePressPos.x());
    verticalScrollBar()->setValue(scrollBarValuesOnMousePress.y() - event->pos().y() + mousePressPos.y());
    horizontalScrollBar()->update();
    verticalScrollBar()->update();
    event->accept();
}

void SvgWindow::mouseReleaseEvent(QMouseEvent *event)
{
    mousePressPos = QPoint();
    event->accept();
}
