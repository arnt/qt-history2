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

#include "svgview.h"
#include "svgwindow.h"

SvgWindow::SvgWindow()
    : QScrollArea()
{
    QWidget *view = new QWidget(this);
    renderer = SvgWindow::Native;
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
        #endif
    } else if (renderer == Image)
        view = new SvgRasterView(currentPath, this);
    else
        view = new SvgNativeView(currentPath, this);

    setWidget(view);
    view->show();
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
