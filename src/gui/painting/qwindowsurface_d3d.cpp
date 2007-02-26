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

//#define D3D_DEBUG_BACKBUFFER

#include <QtGui/QPaintDevice>
#include <QtGui/QWidget>
#include "qdebug.h"

#include "qpaintengine_d3d_p.h"
#include "qwindowsurface_d3d_p.h"
#include "private/qwidget_p.h"
#include "private/qbackingstore_p.h"

#include <d3d9.h>

extern QDirect3DPaintEngine *qt_d3dEngine();

struct QD3DWindowSurfacePrivate
{
    QSize m_lastSize;
    QWidget *m_widget;
};

QD3DWindowSurface::QD3DWindowSurface(QWidget *window)
    : d_ptr(new QD3DWindowSurfacePrivate)
{
    Q_ASSERT(window->isTopLevel());
    d_ptr->m_widget = window;
}


QD3DWindowSurface::~QD3DWindowSurface()
{
    delete d_ptr;
}

QPaintDevice *QD3DWindowSurface::paintDevice()
{
    return d_ptr->m_widget;
}


void QD3DWindowSurface::flush(QWidget *widget, const QRegion &rgn, const QPoint &offset)
{
    QPoint wOffset = qt_qwidget_data(widget)->wrect.topLeft();

    QDirect3DPaintEngine *engine = qt_d3dEngine();
    LPDIRECT3DSWAPCHAIN9 swapchain = engine->swapChain(d_ptr->m_widget);

    if (swapchain) {
        QRect br = rgn.boundingRect();
        QRect wbr = br.translated(-wOffset);

        RECT destrect;
        destrect.left   = wbr.x();
        destrect.top    = wbr.y();
        destrect.right  = destrect.left + wbr.width();
        destrect.bottom = destrect.top  + wbr.height();

        RECT srcrect;
        srcrect.left    = br.x() + offset.x();
        srcrect.top     = br.y() + offset.y();
        srcrect.right   = wbr.width() + srcrect.left;
        srcrect.bottom  = wbr.height() + srcrect.top;
        int devwidth = d_ptr->m_lastSize.width();
        int devheight = d_ptr->m_lastSize.height();

        if (devwidth <= srcrect.right) {
            int diff = srcrect.right - devwidth;
            srcrect.right -= diff;
            destrect.right -= diff;
            if (srcrect.right <= srcrect.left)
                return;
        }       
        if (devheight <= srcrect.bottom) {
            int diff = srcrect.bottom - devheight;
            srcrect.bottom -= diff;
            destrect.bottom -= diff;
            if (srcrect.bottom <= srcrect.top)
                return;
        }       

        if (FAILED(swapchain->Present(&srcrect, &destrect, widget->winId(), 0, 0)))
            qWarning("QDirect3DPaintEngine: failed to present back buffer.");

#ifdef D3D_DEBUG_BACKBUFFER
        qDebug() << widget << srcrect.left << srcrect.top << wbr.width() << wbr.height() << "Dest: " << destrect.left << destrect.top;
        IDirect3DSurface9 *surface;
        swapchain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &surface);
        QString filename("C:\\test.bmp");
        D3DXSaveSurfaceToFile(filename.utf16(), D3DXIFF_BMP, surface, 0, 0);
        surface->Release();
#endif
    }
}

void QD3DWindowSurface::setGeometry(const QRect &rect)
{
    qDebug() << "QD3DWindowSurface::setGeometry" << rect;
    Q_UNUSED(rect);
}

void QD3DWindowSurface::release()
{
    QDirect3DPaintEngine *engine = qt_d3dEngine();
    engine->releaseSwapChain(d_ptr->m_widget);
}


void QD3DWindowSurface::scroll(const QRegion &area, int dx, int dy)
{
    QDirect3DPaintEngine *engine = qt_d3dEngine();
    QRect rect = area.boundingRect();

    RECT destrect;
    destrect.left   = rect.x()+dx;
    destrect.top    = rect.y()+dy;
    destrect.right  = rect.width() + destrect.left;
    destrect.bottom = rect.height() + destrect.top;

    RECT srcrect;
    srcrect.left    = rect.x();
    srcrect.top     = rect.y();
    srcrect.right   = rect.width() + srcrect.left;
    srcrect.bottom  = rect.height() + srcrect.top;

    engine->scroll(d_ptr->m_widget, srcrect, destrect);
}

QRect QD3DWindowSurface::geometry() const
{
    QSize currentSize = d_ptr->m_widget->size();
    if (d_ptr->m_lastSize != currentSize) {
        d_ptr->m_lastSize = currentSize;

        QWidget *w = d_ptr->m_widget;
        QTLWExtra *topData = w->d_func()->topData();
        QWidgetBackingStore *bs = topData->backingStore;
        bs->dirtyRegion(QRegion(0,0,w->width(), w->height()),w);
    }
    return d_ptr->m_widget->geometry();
}
