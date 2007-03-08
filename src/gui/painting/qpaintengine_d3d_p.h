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

#ifndef QPAINTENGINE_D3D_P_H
#define QPAINTENGINE_D3D_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtGui/qpaintengine.h"
#include "QtGui/qpainterpath.h"
#include "private/qdatabuffer_p.h"
#include "private/qdrawhelper_p.h"
#include "private/qpaintengine_p.h"
#include "private/qstroker_p.h"

#include <stdlib.h>

#ifdef Q_WS_WIN
#include <d3d9.h>
#include <d3dx9.h>

class QD3DWindowManager : public QObject {
    Q_OBJECT

public:
    enum QD3DWindowManagerStatus {
        NoStatus = 0,
        NeedsReseting = 0x01,
        MaxSizeChanged = 0x02
    };

    QD3DWindowManager();
    ~QD3DWindowManager();

    void init(LPDIRECT3D9 object);

    void setPaintDevice(QPaintDevice *pd);

    int status() const;
    void reset();

    LPDIRECT3DSURFACE9 renderTarget();

    LPDIRECT3DSURFACE9 surface(QPaintDevice *pd);
    LPDIRECT3DSWAPCHAIN9 swapChain(QPaintDevice *pd);
    void releasePaintDevice(QPaintDevice *pd);

    LPDIRECT3DDEVICE9 device();
    void cleanup();

    QSize maxSize() const;

private:
    struct D3DSwapChain {
        QSize size;
        LPDIRECT3DSWAPCHAIN9 swapchain;
        LPDIRECT3DSURFACE9 surface;
    };

    void updateMaxSize();
    void initPresentParameters(D3DPRESENT_PARAMETERS *params);
    D3DSwapChain *createSwapChain(QWidget *w);

    QSize m_maxSize;
    int m_status;
    QMap<QPaintDevice *, D3DSwapChain *> m_swapchains;

    LPDIRECT3DDEVICE9 m_device;
    QPaintDevice *m_pd;
    QWidget *m_dummy;
    D3DSwapChain *m_current;

private Q_SLOTS:
    void cleanupPaintDevice(QObject *);
};

class QDirect3DPaintEnginePrivate;

struct vertex {
    D3DXVECTOR3 pos;
    DWORD color;
    FLOAT s0, t0, r0, q0;
    FLOAT s1, t1, r1, q1;
};

/************************************************
 * QDirect3DPaintEngine
 */

class QDirect3DPaintEngine : public QPaintEngine
{
    Q_DECLARE_PRIVATE(QDirect3DPaintEngine)
public:
    QDirect3DPaintEngine();
    ~QDirect3DPaintEngine();
    bool begin(QPaintDevice *device);

    void drawEllipse(const QRectF &rect);
    void drawEllipse(const QRect &rect);

    void drawImage(const QRectF &rectangle, const QImage &image, const QRectF &sr,
        Qt::ImageConversionFlags flags = Qt::AutoColor);

    void drawLines(const QLineF *lines, int lineCount);
    void drawLines(const QLine *lines, int lineCount);

    void drawPath(const QPainterPath &path);

    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr);

    void drawPoints(const QPointF *points, int pointCount);
    void drawPoints(const QPoint *points, int pointCount);

    void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);
    void drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode);

    void drawRects(const QRectF *rects, int rectCount);
    void drawRects(const QRect * rects, int rectCount);

    void drawTextItem(const QPointF &p, const QTextItem &textItem);

    void drawTiledPixmap(const QRectF &r, const QPixmap &pm, const QPointF &sr);

    bool end();

    Type type() const { return Direct3D; }
    void updateState(const QPaintEngineState &state);

    void cleanup();

    HDC getDC() const;
    void setFlushOnEnd(bool flushOnEnd);

public:
    void scroll(QPaintDevice *pd, const RECT &srcrect, const RECT &destrect);
    LPDIRECT3DSWAPCHAIN9 swapChain(QPaintDevice *pd);
    void releaseSwapChain(QPaintDevice *pd);

private:
    Q_DISABLE_COPY(QDirect3DPaintEngine)
    friend class QPixmap;
    friend class QD3DGlyphCache;
};

struct QD3DMaskPosition {
public:
    int x, y, channel;
};

#define QD3D_BATCH_SIZE 256

struct QD3DBatchItem {
    enum QD3DBatchInfo {
        BI_WINDING          = 0x0001,
        BI_AA               = 0x0002,
        BI_BRECT            = 0x0004,
        BI_MASKFULL         = 0x0008,
        BI_TEXT             = 0x0010,
        BI_MASK             = 0x0020,
        BI_CLIP             = 0x0040,
        BI_SCISSOR          = 0x0080,

        BI_PIXMAP           = 0x0100,
        BI_IMAGE            = 0x0200,
        BI_COMPLEXBRUSH     = 0x0400,

        BI_CLEARCLIP        = 0x0800, // clip nothing (filling the clip mask with 0)
        BI_TRANSFORM        = 0x1000,
        BI_MASKSCISSOR      = 0x2000,
        BI_FASTLINE         = 0x4000,
        BI_LINESTRIP        = 0x8000
    };

    int m_info;

    int m_count;
    int m_offset;

    QD3DMaskPosition m_maskpos;
    qreal m_xoffset;
    qreal m_yoffset;
    qreal m_opacity;

    QPixmap m_pixmap;
    QRectF m_brect;
    QBrush m_brush;

    IDirect3DTexture9 *m_texture;

    qreal m_width;
    qreal m_distance;

    D3DXMATRIX m_matrix;
    QPainter::CompositionMode m_cmode;

    QVector<int> m_pointstops;
};

class QD3DBatch {
public:
    int m_itemIndex;
    QD3DBatchItem items[QD3D_BATCH_SIZE];
};

class QD3DStateManager;
class QD3DFontCache;
class QD3DMaskAllocator;
class QD3DVertexBuffer;
class QD3DGradientCache;

class QDirect3DPaintEnginePrivate : public QPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QDirect3DPaintEngine)

public:
    enum RenderTechnique {
        RT_NoTechnique,
        RT_Antialiased,
        RT_Aliased,
    };

    QDirect3DPaintEnginePrivate()
        : m_d3dObject(0)
        , m_d3dDevice(0)
        , m_txop(QTransform::TxNone)
        , m_effect(0)
        , m_flushOnEnd(0)
    { init(); }

    ~QDirect3DPaintEnginePrivate();

    bool init();
    void initDevice();

    inline QD3DBatchItem *nextBatchItem();

    QPolygonF brushCoordinates(const QRectF &r, bool stroke, qreal *fp) const;
    void fillAliasedPath(QPainterPath path, const QRectF &brect, const QTransform &txform);
    void fillAntialiasedPath(const QPainterPath &path, const QRectF &brect,
        const QTransform &txform, bool stroke);
    void fillPath(const QPainterPath &path, QRectF brect);

    void strokePath(const QPainterPath &path, QRectF brect, bool simple = false);
    QPainterPath strokePathFastPen(const QPainterPath &path);
    void strokeAliasedPath(QPainterPath path, const QRectF &brect, const QTransform &txform);

    void flushBatch();
    int flushAntialiased(int offset);
    void flushAliased(QD3DBatchItem *item, int offset);
    void flushText(QD3DBatchItem *item, int offset);
    void flushLines(QD3DBatchItem *item, int offset);

    void updateTransform(const QTransform &matrix);
    void updatePen(const QPen &pen);
    void updateBrush(const QBrush &pen);
    void updateClipRegion(const QRegion &clipregion, Qt::ClipOperation op = Qt::ReplaceClip);
    void updateClipPath(const QPainterPath &clipregion, Qt::ClipOperation op = Qt::ReplaceClip);
    void updateFont(const QFont &font);

    void setRenderTechnique(RenderTechnique technique);

    QPointF transformPoint(const QPointF &p, qreal *w) const;

    bool prepareBatch(QD3DBatchItem *item, int offset);
    void prepareItem(QD3DBatchItem *item);
    void cleanupItem(QD3DBatchItem *item);
    void setCompositionMode(QPainter::CompositionMode mode);

    bool isFastRect(const QRectF &rect);
    
    void releaseDC();

    void cleanup();

    // clipping
    QPainterPath m_sysClipPath;
    QPainterPath m_clipPath;
    QRegion m_sysClipRegion;
    QRegion m_clipRegion;

    qreal m_opacity;
    D3DCOLOR m_opacityColor;

    int m_currentState;

    ID3DXEffect* m_effect;

    RenderTechnique m_currentTechnique;

    QTransform m_matrix;
    D3DXMATRIX m_d3dxmatrix;
    D3DXMATRIX m_d3dxidentmatrix;
    qreal m_invScale;

    QPen m_pen;
    Qt::BrushStyle m_pen_brush_style;
    QTransform m_inv_pen_matrix;
    D3DCOLOR m_penColor;
    qreal m_pen_width;

    QBrush m_brush;
    Qt::BrushStyle m_brush_style;
    QTransform m_inv_brush_matrix;
    D3DCOLOR m_brushColor;
    QTransform m_brushOrigin;

    uint clipping_enabled : 1;
    uint has_complex_clipping : 1;
    uint use_complex_clipping : 1;
    uint cleartype_text: 1;
    uint has_pen : 1;
    uint has_cosmetic_pen : 1;
    uint has_brush : 1;
    uint has_fast_pen : 1;
    uint m_flushOnEnd : 1;

    QTransform::TransformationCodes m_txop;

    QPainter::CompositionMode m_cmode;

    QD3DWindowManager m_winManager;
    QSize m_winSize;

    LPDIRECT3D9 m_d3dObject;
    LPDIRECT3DDEVICE9 m_d3dDevice;
    IDirect3DSurface9 *m_defaultSurface;
    bool m_inScene;

    QD3DGradientCache *m_gradCache;
    QD3DVertexBuffer *m_vBuffer;
    QD3DBatch m_batch;
    QD3DStateManager *m_statemanager;

    HDC m_dc;
    IDirect3DSurface9 *m_dcsurface;
};

#endif //Q_WS_WIN
#endif
