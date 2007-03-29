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

#include <qdebug.h>
#include "qpaintengine_d3d_p.h"

#include "private/qdrawhelper_p.h"
#include "private/qfont_p.h"
#include "private/qfontengine_p.h"
#include "private/qmath_p.h"
#include "private/qpaintengine_p.h"
#include "private/qtessellator_p.h"
#include <private/qbezier_p.h>
#include <private/qpainter_p.h>
#include <private/qpixmap_p.h>
#include <private/qpolygonclipper_p.h>
#include <qbuffer.h>
#include <qcache.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qlibrary.h>
#include <qlibraryinfo.h>
#include <qpaintdevice.h>
#include <qpixmapcache.h>

#include <d3d9.h>
#include <d3dx9.h>

#include <mmintrin.h>
#include <xmmintrin.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

// for the ClearType detection stuff..
#ifndef SPI_GETFONTSMOOTHINGTYPE
#define SPI_GETFONTSMOOTHINGTYPE 0x200A
#endif

#ifndef FE_FONTSMOOTHINGCLEARTYPE
#define FE_FONTSMOOTHINGCLEARTYPE 0x0002
#endif

//#include <performance.h>
#define PM_INIT
#define PM_MEASURE(A)
#define PM_DISPLAY

//debugging
//#define QT_DEBUG_VERTEXBUFFER_ACCESS
//#define QT_DEBUG_D3D
//#define QT_DEBUG_D3D_CALLS

#define QD3D_SET_MARK(output) \
    D3DPERF_SetMarker(0, QString(output).utf16());

#define QT_VERTEX_RESET_LIMIT   24576
#define QT_VERTEX_BUF_SIZE      32768
#define QD3DFVF_CSVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE4(0) |  D3DFVF_TEXCOORDSIZE4(1))

// this is a different usage of the effect framework than intended,
// but it's convinient for us to use (See effect file)
#define PASS_STENCIL_ODDEVEN                0
#define PASS_STENCIL_WINDING                1
#define PASS_STENCIL_DRAW                   2
#define PASS_STENCIL_DRAW_DIRECT            3
#define PASS_STENCIL_CLIP                   4
#define PASS_STENCIL_NOSTENCILCHECK         5
#define PASS_STENCIL_NOSTENCILCHECK_DIRECT  6
#define PASS_TEXT                           7
#define PASS_CLEARTYPE_TEXT                 8

#define PASS_AA_CREATEMASK                  0
#define PASS_AA_DRAW                        1
#define PASS_AA_DRAW_DIRECT                 2

#define D3D_STAGE_COUNT             2
#define D3D_RENDER_STATES           210
#define D3D_TEXTURE_STATES          33
#define D3D_SAMPLE_STATES           14


typedef HRESULT (APIENTRY *PFND3DXCREATEBUFFER)(DWORD, LPD3DXBUFFER *);
typedef HRESULT (APIENTRY *PFND3DXCREATEEFFECT)(LPDIRECT3DDEVICE9, LPCVOID, UINT, CONST D3DXMACRO *,
                                                LPD3DXINCLUDE, DWORD, LPD3DXEFFECTPOOL,
                                                LPD3DXEFFECT *, LPD3DXBUFFER *);
typedef D3DXMATRIX *(APIENTRY *PFND3DXMATRIXORTHOOFFCENTERLH)(D3DMATRIX *, FLOAT, FLOAT,
                                                              FLOAT, FLOAT, FLOAT, FLOAT);
typedef IDirect3D9 *(APIENTRY *PFNDIRECT3DCREATE9)(uint);

static PFNDIRECT3DCREATE9 pDirect3DCreate9 = 0;
static PFND3DXCREATEBUFFER pD3DXCreateBuffer = 0;
static PFND3DXCREATEEFFECT pD3DXCreateEffect = 0;
static PFND3DXMATRIXORTHOOFFCENTERLH pD3DXMatrixOrthoOffCenterLH = 0;



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
    HWND m_dummy;
    D3DSwapChain *m_current;

private Q_SLOTS:
    void cleanupPaintDevice(QObject *);
};

struct vertex {
    D3DXVECTOR3 pos;
    DWORD color;
    FLOAT s0, t0, r0, q0;
    FLOAT s1, t1, r1, q1;
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
    bool testCaps();

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
    uint m_supports_d3d : 1;

    QTransform::TransformationType m_txop;

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


class QD3DStateManager : public ID3DXEffectStateManager {
public:
    QD3DStateManager(LPDIRECT3DDEVICE9 pDevice, ID3DXEffect *effect);
    void reset();

    inline void startStateBlock();
    inline void endStateBlock();

    inline void setBrushMode(int mode);
    inline void setTexture(LPDIRECT3DBASETEXTURE9 pTexture, QGradient::Spread spread = QGradient::PadSpread);
    inline void setTransformation(const D3DXMATRIX *pMatrix);
    inline void setProjection(const D3DXMATRIX *pMatrix);
    inline void setMaskChannel(int channel);
    inline void setMaskOffset(qreal x, qreal y);
    inline void setFocalDistance(const qreal &fd);

    inline void beginPass(int pass);
    inline void endPass();

    STDMETHOD(QueryInterface)(REFIID iid, LPVOID *ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    STDMETHOD(SetTransform)(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix);
    STDMETHOD(SetMaterial)(CONST D3DMATERIAL9 *pMaterial);
    STDMETHOD(SetLight)(DWORD Index, CONST D3DLIGHT9 *pLight);
    STDMETHOD(LightEnable)(DWORD Index, BOOL Enable);
    STDMETHOD(SetRenderState)(D3DRENDERSTATETYPE State, DWORD Value);
    STDMETHOD(SetTexture)(DWORD Stage, LPDIRECT3DBASETEXTURE9 pTexture);
    STDMETHOD(SetTextureStageState)(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
    STDMETHOD(SetSamplerState)(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);
    STDMETHOD(SetNPatchMode)(FLOAT NumSegments);
    STDMETHOD(SetFVF)(DWORD FVF);
    STDMETHOD(SetVertexShader)(LPDIRECT3DVERTEXSHADER9 pShader);
    STDMETHOD(SetVertexShaderConstantF)(UINT RegisterIndex, CONST FLOAT *pConstantData, UINT RegisterCount);
    STDMETHOD(SetVertexShaderConstantI)(UINT RegisterIndex, CONST INT *pConstantData, UINT RegisterCount);
    STDMETHOD(SetVertexShaderConstantB)(UINT RegisterIndex, CONST BOOL *pConstantData, UINT RegisterCount);
    STDMETHOD(SetPixelShader)(LPDIRECT3DPIXELSHADER9 pShader);
    STDMETHOD(SetPixelShaderConstantF)(UINT RegisterIndex, CONST FLOAT *pConstantData, UINT RegisterCount);
    STDMETHOD(SetPixelShaderConstantI)(UINT RegisterIndex, CONST INT *pConstantData, UINT RegisterCount);
    STDMETHOD(SetPixelShaderConstantB)(UINT RegisterIndex, CONST BOOL *pConstantData, UINT RegisterCount);
private:
    LPDIRECT3DVERTEXSHADER9 m_vertexshader;
    LPDIRECT3DPIXELSHADER9 m_pixelshader;

    LPDIRECT3DBASETEXTURE9 m_textures[D3D_STAGE_COUNT];
    DWORD m_texturestates[D3D_STAGE_COUNT][D3D_TEXTURE_STATES];
    DWORD m_samplerstates[D3D_STAGE_COUNT][D3D_SAMPLE_STATES];
    DWORD m_renderstate[D3D_RENDER_STATES];

    qreal m_radgradfd;

    int m_pass;
    int m_maskchannel;
    int m_brushmode;
    LPDIRECT3DBASETEXTURE9 m_texture;
    D3DXMATRIX m_projection;
    D3DXMATRIX m_transformation;

    LPDIRECT3DDEVICE9 m_pDevice;
    ID3DXEffect *m_effect;

    LONG m_refs;
    bool m_changed;
    qreal m_xoffset, m_yoffset;
    static int m_maskChannels[4][4];
};

//
// font cache stuff
//

struct QD3DGlyphCoord {
    // stores the offset and size of a glyph texture
    qreal x;
    qreal y;
    qreal width;
    qreal height;
    qreal log_width;
    qreal log_height;
    QFixed x_offset;
    QFixed y_offset;
};

struct QD3DFontTexture {
    int x_offset; // current glyph offset within the texture
    int y_offset;
    int width;
    int height;
    IDirect3DTexture9 *texture;
};

typedef QHash<glyph_t, QD3DGlyphCoord*>  QD3DGlyphHash;
typedef QHash<QFontEngine*, QD3DGlyphHash*> QD3DFontGlyphHash;
typedef QHash<quint64, QD3DFontTexture*> QD3DFontTexHash;

class QD3DGlyphCache : public QObject
{
    Q_OBJECT
public:
    QD3DGlyphCache()
        : QObject(0)
        , current_cache(0) {}
    ~QD3DGlyphCache();
    QD3DGlyphCoord *lookup(QFontEngine *, glyph_t);
    void cacheGlyphs(QDirect3DPaintEngine *, const QTextItemInt &, const QVarLengthArray<glyph_t> &,
                     bool);
    void cleanCache();
    inline QD3DFontTexture *fontTexture(QFontEngine *engine) {
        return font_textures.constFind(reinterpret_cast<quint64>(engine)).value();
    }

public slots:
    void fontEngineDestroyed(QObject *);

private:
    QImage clearTypeGlyph(QFontEngine *, glyph_t glyph);
    QD3DGlyphHash *current_cache;
    QD3DFontTexHash font_textures;
    QD3DFontGlyphHash font_cache;
};

QD3DGlyphCache::~QD3DGlyphCache()
{
}

QD3DGlyphCoord *QD3DGlyphCache::lookup(QFontEngine *, glyph_t g)
{
    Q_ASSERT(current_cache != 0);
    QD3DGlyphHash::const_iterator it = current_cache->constFind(g);
    if (it == current_cache->constEnd())
        return 0;
    return it.value();
}

void QD3DGlyphCache::cleanCache()
{
    QList<quint64> keys = font_textures.keys();
    for (int i=0; i<keys.size(); ++i)
        font_textures.value(keys.at(i))->texture->Release();

    qDeleteAll(font_textures);
    qDeleteAll(font_cache);
    font_textures.clear();
    font_cache.clear();
    current_cache = 0;
}

void QD3DGlyphCache::fontEngineDestroyed(QObject *object)
{
//     qDebug() << "=> font engine destroyed: " << object;
    QFontEngine *engine = static_cast<QFontEngine *>(object);

    QD3DFontGlyphHash::iterator cache_it = font_cache.find(engine);
    if (cache_it != font_cache.end()) {
        QD3DGlyphHash *cache = font_cache.take(engine);
        delete cache;
    }

    quint64 font_key = reinterpret_cast<quint64>(engine);
    QD3DFontTexture *tex = font_textures.take(font_key);
    if (tex) {
        tex->texture->Release();
        delete tex;
    }
}

QImage QD3DGlyphCache::clearTypeGlyph(QFontEngine *engine, glyph_t glyph)
{
    glyph_metrics_t gm = engine->boundingBox(glyph);
    int glyph_x = qFloor(gm.x.toReal());
    int glyph_y = qFloor(gm.y.toReal());
    int glyph_width = qCeil((gm.x + gm.width).toReal()) -  glyph_x + 2;
    int glyph_height = qCeil((gm.y + gm.height).toReal()) - glyph_y + 2;

    if (glyph_width + glyph_x <= 0 || glyph_height <= 0)
        return QImage();
    QImage im(glyph_width + glyph_x, glyph_height, QImage::Format_ARGB32_Premultiplied);
    im.fill(0xff000000); // solid black
    QPainter p(&im);

    p.setPen(Qt::white);
    p.setBrush(Qt::NoBrush);

    QTextItemInt ti;
    ti.ascent = engine->ascent();
    ti.descent = engine->descent();
    ti.width = glyph_width;
    ti.fontEngine = engine;

    ti.num_glyphs = 1;
    QGlyphLayout glyphLayout;
    ti.glyphs = &glyphLayout;
    glyphLayout.glyph = glyph;
    memset(&glyphLayout.attributes, 0, sizeof(glyphLayout.attributes));
    glyphLayout.advance.x = glyph_width;
    p.drawTextItem(QPointF(-glyph_x, -glyph_y), ti);
    p.end();
    return im;
}

#if 0
static void dump_font_texture(QD3DFontTexture *tex)
{
    QColor color(Qt::red);
    D3DLOCKED_RECT rect;
    if (FAILED(tex->texture->LockRect(0, &rect, 0, 0))) {
        qDebug() << "debug: unable to lock texture rect.";
        return;
    }

// cleartype version
//     uint *tex_data = (uint *) rect.pBits;
//     QImage im(tex->width, tex->height, QImage::Format_ARGB32);
//     for (int y=0; y<tex->height; ++y) {
//         for (int x=0; x<tex->width; ++x) {
//             im.setPixel(x, y, ((*(tex_data+x+y*tex->width))));
//         }
//     }
    uchar *tex_data = (uchar *) rect.pBits;
    QImage im(rect.Pitch, tex->height, QImage::Format_ARGB32);
    for (int y=0; y<tex->height; ++y) {
        for (int x=0; x<rect.Pitch; ++x) {
            uchar val = ((*(tex_data+x+y*rect.Pitch)));
            im.setPixel(x, y, 0xff000000 | (val << 16) | (val << 8) | val);
        }
    }
    tex->texture->UnlockRect(0);
    static  int i= 0;
    im.save(QString("tx%1.png").arg(i++));
}
#endif

void QD3DGlyphCache::cacheGlyphs(QDirect3DPaintEngine *engine, const QTextItemInt &ti,
                                 const QVarLengthArray<glyph_t> &glyphs, bool clearType)
{
    IDirect3DDevice9 *device = engine->d_func()->m_d3dDevice;
    QD3DFontGlyphHash::const_iterator cache_it = font_cache.constFind(ti.fontEngine);
    QD3DGlyphHash *cache = 0;
    if (cache_it == font_cache.constEnd()) {
        cache = new QD3DGlyphHash;
        font_cache.insert(ti.fontEngine, cache);
        connect(ti.fontEngine, SIGNAL(destroyed(QObject *)), SLOT(fontEngineDestroyed(QObject *)));
    } else {
        cache = cache_it.value();
    }

    current_cache = cache;

    D3DFORMAT tex_format = clearType ? D3DFMT_A8R8G8B8 : D3DFMT_A8;
    quint64 font_key = reinterpret_cast<quint64>(ti.fontEngine);
    QD3DFontTexHash::const_iterator it = font_textures.constFind(font_key);
    QD3DFontTexture *font_tex = 0;
    if (it == font_textures.constEnd()) {
        // alloc a new texture, put it into the cache
        int tex_height = qCeil(ti.ascent.toReal() + ti.descent.toReal()) + 5;
        int tex_width = tex_height * 30; // ###
        IDirect3DTexture9 *tex;
        if (FAILED(device->CreateTexture(tex_width, tex_height, 1, 0,
                                         tex_format, D3DPOOL_MANAGED, &tex, NULL)))
        {
            qWarning("QD3DGlyphCache::cacheGlyphs(): can't allocate font texture (%dx%d).",
                     tex_width, tex_height);
            return;
        } else {
//             qDebug() << "=> new font texture: " << QSize(tex_width,tex_height);
            font_tex = new QD3DFontTexture;
            font_tex->texture = tex;
            font_tex->x_offset = 0;
            font_tex->y_offset = 0;
            font_tex->width = tex_width;
            font_tex->height = tex_height;
            font_textures.insert(font_key, font_tex);
        }
    } else {
        font_tex = it.value();
        // make it current render target..
    }

    // cache each glyph
    for (int i=0; i<glyphs.size(); ++i) {
        QD3DGlyphHash::const_iterator it = cache->constFind(glyphs[i]);
        if (it == cache->constEnd()) {
            glyph_metrics_t metrics = ti.fontEngine->boundingBox(glyphs[i]);
            int glyph_width = qCeil(metrics.width.toReal()) + 5;
            int glyph_height = qCeil(ti.ascent.toReal() + ti.descent.toReal()) + 5;
            if (font_tex->x_offset + glyph_width > font_tex->width) {
                // no room on the current line, start new glyph strip
                int strip_height = glyph_height;
                font_tex->x_offset = 0;
                font_tex->y_offset += strip_height;
                if (font_tex->y_offset >= font_tex->height) {
                    // if no room in the current texture - realloc a larger texture
                    int old_tex_height = font_tex->height;
                    font_tex->height += strip_height;

                    IDirect3DTexture9 *new_tex;
                    if (FAILED(device->CreateTexture(font_tex->width, font_tex->height, 1, 0,
                                                     tex_format, D3DPOOL_MANAGED, &new_tex, NULL)))
                    {
                        qWarning("QD3DGlyphCache(): can't re-allocate font texture.");
                        return;
                    } else {
//                         qDebug() << " -> new glyph strip added:" << QSize(font_tex->width,font_tex->height);

                        D3DLOCKED_RECT new_rect, old_rect;
                        if (FAILED(font_tex->texture->LockRect(0, &old_rect, 0, D3DLOCK_READONLY))) {
                            qDebug() << "QD3DGlyphCache: unable to lock texture rect.";
                            return;
                        }
                        if (FAILED(new_tex->LockRect(0, &new_rect, 0, 0))) {
                            qDebug() << "QD3DGlyphCache: unable to lock texture rect.";
                            return;
                        }
                        memcpy(new_rect.pBits, old_rect.pBits, new_rect.Pitch * old_tex_height);
                        font_tex->texture->UnlockRect(0);
                        new_tex->UnlockRect(0);
                        engine->d_func()->flushBatch();
                        font_tex->texture->Release();
                        font_tex->texture = new_tex;
                    }

                    // update the texture coords and the y offset for the existing glyphs in
                    // the cache, because of the texture size change
                    QD3DGlyphHash::iterator it = cache->begin();
                    while (it != cache->end()) {
                        it.value()->height = (it.value()->height * old_tex_height) / font_tex->height;
                        it.value()->y = (it.value()->y * old_tex_height) / font_tex->height;
                        ++it;
                    }
                }
            }
            QD3DGlyphCoord *d3d_glyph = new QD3DGlyphCoord;
            d3d_glyph->x = qreal(font_tex->x_offset) / font_tex->width;
            d3d_glyph->y = qreal(font_tex->y_offset) / font_tex->height;
            d3d_glyph->width = qreal(glyph_width) / font_tex->width;
            d3d_glyph->height = qreal(glyph_height) / font_tex->height;
            d3d_glyph->log_width = d3d_glyph->width * font_tex->width;
            d3d_glyph->log_height = d3d_glyph->height * font_tex->height;
            d3d_glyph->x_offset = -metrics.x;
            d3d_glyph->y_offset = metrics.y;

            QImage glyph_im;
            if (clearType)
                glyph_im = clearTypeGlyph(ti.fontEngine, glyphs[i]);
            else
                glyph_im = ti.fontEngine->alphaMapForGlyph(glyphs[i]).convertToFormat(QImage::Format_Indexed8);

            // ### write glyph to texture
            D3DLOCKED_RECT rect;
            RECT glyph_rect = { font_tex->x_offset, font_tex->y_offset,
                                font_tex->x_offset + glyph_im.width(),
                                font_tex->y_offset + glyph_im.height() };

//             qDebug() << "  > new glyph char added:" << QSize(glyph_im.width(), glyph_im.height());
            if (FAILED(font_tex->texture->LockRect(0, &rect, &glyph_rect, 0))) {
                qDebug() << "QD3DGlyphCache: unable to lock texture rect.";
                return;
            }

            // ### unify these loops
            if (clearType) {
                int ppl = rect.Pitch / 4;
                uint *tex_data = (uint *) rect.pBits;
                for (int y=0; y<glyph_im.height(); ++y) {
                    uint *s = (uint *) glyph_im.scanLine(y);
                    for (int x=0; x<glyph_im.width(); ++x) {
                        tex_data[ppl*y + x] = *s;
                        ++s;
                    }
                }
            } else {
                int ppl = rect.Pitch;
                uchar *tex_data = (uchar *) rect.pBits;
                for (int y=0; y<glyph_im.height(); ++y) {
                    uchar *s = (uchar *) glyph_im.scanLine(y);
                    for (int x=0; x<glyph_im.width(); ++x) {
                        tex_data[ppl*y + x] = *s;
                        ++s;
                    }
                }
            }
            font_tex->texture->UnlockRect(0);

            // debug
//             dump_font_texture(font_tex);

            if (font_tex->x_offset + glyph_width > font_tex->width) {
                font_tex->x_offset = 0;
                font_tex->y_offset += glyph_height;
            } else {
                font_tex->x_offset += glyph_width;
            }

            cache->insert(glyphs[i], d3d_glyph);
        }
    }
}

Q_GLOBAL_STATIC(QD3DGlyphCache, qd3d_glyph_cache)

//
// end font caching stuff
//


//
//  D3D image cache stuff
//

// ### keep the GL stuff in mind..
typedef void (*_qt_image_cleanup_hook_64)(qint64);
extern Q_GUI_EXPORT _qt_image_cleanup_hook_64 qt_image_cleanup_hook_64;

static void qd3d_image_cleanup(qint64 key);

class QD3DImage
{
public:
    QD3DImage(IDirect3DDevice9 *device, const QImage &image);
    ~QD3DImage();

    IDirect3DTexture9 *texture;
};

static QList<IDirect3DTexture9 *> qd3d_release_list;

QD3DImage::QD3DImage(IDirect3DDevice9 *device, const QImage &image)
{
    texture = 0;
    Q_ASSERT(device);
    QImage im = image.convertToFormat(QImage::Format_ARGB32);
    if (FAILED(device->CreateTexture(im.width(), im.height(), 1, 0,
                                     D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &texture, 0))) {
        qWarning("QD3DImage(): unable to create Direct3D texture.");
        return;
    }
//     qDebug(" -> created image texture: %p - 0x%08x%08x",texture,uint (image.cacheKey() >> 32),uint (image.cacheKey() & 0xffffffff));
    D3DLOCKED_RECT rect;
    if (FAILED(texture->LockRect(0, &rect, 0, 0))) {
        qDebug() << "QD3DImage: unable to lock texture rect.";
        return;
    }
    DWORD *dst = (DWORD *) rect.pBits;
    DWORD *src = (DWORD *) im.scanLine(0);
    int dst_ppl = rect.Pitch/4;
    int src_ppl = im.bytesPerLine()/4;

    Q_ASSERT(dst_ppl == src_ppl);
    memcpy(dst, src, rect.Pitch*im.height());
    texture->UnlockRect(0);
}

QD3DImage::~QD3DImage()
{
    if (texture)
        qd3d_release_list.append(texture);
}

static int qd3d_cache_limit = 64*1024; // cache ~64 MB worth of textures
typedef QCache<quint64, QD3DImage> QD3DImageCache;

class QD3DImageManager
{
public:
    QD3DImageManager() {
        // ### GL does the same!
        qt_image_cleanup_hook_64 = qd3d_image_cleanup;
        cache.setMaxCost(qd3d_cache_limit);
    }
    ~QD3DImageManager() {
//         qDebug() << "unhooking d3d image cache";
        qt_image_cleanup_hook_64 = 0;
        cache.clear();
    }

    IDirect3DTexture9 *lookup(IDirect3DDevice9 *device, const QImage &image);
    void remove(quint64 key);

private:
    QD3DImageCache cache;
};

IDirect3DTexture9 *QD3DImageManager::lookup(IDirect3DDevice9 *device, const QImage &image)
{
    QD3DImage *tex_image = 0;

    tex_image = cache.object(image.cacheKey());
    if (!tex_image) {
        // to avoid cache thrashing we remove images from the cache
        // that have the same serial no as the cached image, since
        // that image is most likely destoyed already, and we got a
        // stale cache entry
        uint serial = (uint) (image.cacheKey() >> 32);
        QList<quint64> keys = cache.keys();
        for (int i=0; i<keys.size(); ++i) {
            if ((uint)(keys.at(i) >> 32) == serial) {
                cache.remove(keys.at(i));
                break;
            }
        }
//         qDebug(" => cached: %d, adding cache image: 0x%08x%08x",cache.size(), uint (image.cacheKey() >> 32),uint (image.cacheKey() & 0xffffffff));
        // add cache entry
        int cost = image.width()*image.height()*4/1024;
        if (cache.totalCost() + cost > cache.maxCost()) {
            // no room for new entries? kick out half the cached images
            int old_max_cost = cache.maxCost();
            cache.setMaxCost(old_max_cost/2);
            cache.setMaxCost(old_max_cost);
        }
        tex_image = new QD3DImage(device, image);
        cache.insert(image.cacheKey(), tex_image, cost);
//         qDebug() << "==> total cache cost: " << cache.totalCost() << cost;
    }

    return tex_image->texture;
}

void QD3DImageManager::remove(quint64 key)
{
//     QList<quint64> keys = cache.keys();
//     if (keys.contains(key))
//         qDebug() << "entery removed from cache";
    cache.remove(key);
}

Q_GLOBAL_STATIC(QD3DImageManager, qd3d_image_cache)

static void qd3d_image_cleanup(qint64 key)
{
//     qDebug() << "qd3d_image_cleanup:";
//     qDebug("  => key: 0x%08x%08x", (uint) (key >> 32), (uint)(key & 0xffffffff));
    qd3d_image_cache()->remove(key);
}

//
// end D3D image cache stuff
//

class QD3DMaskAllocator {
public:
    QD3DMaskAllocator();

    void reset();
    bool allocate(int w, int h);
    void setSize(int w, int h);

    QD3DMaskPosition mask_position;

private:
    int width;
    int height;

    int mask_offsetX2;
    int mask_offsetY2;
};

class QD3DVertexBuffer : public QTessellator
{
public:
    QD3DVertexBuffer(QDirect3DPaintEnginePrivate *pe);
    ~QD3DVertexBuffer();

    bool needsFlushing() const;
    QD3DMaskPosition allocateMaskPosition(const QRectF &brect, bool *breakbatch);

    void setClipPath(const QPainterPath &path, QD3DBatchItem **item);

    void queueAntialiasedMask(const QPolygonF &poly, QD3DBatchItem **item, const QRectF &brect);
    QRectF queueAliasedMask(const QPainterPath &path, QD3DBatchItem **item, D3DCOLOR color);

    void queueRect(const QRectF &rect, QD3DBatchItem *item, D3DCOLOR color, const QPolygonF &trect);
    void queueRect(const QRectF &rect, QD3DBatchItem *item, D3DCOLOR color);

    void queueTextGlyph(const QRectF &rect, const qreal *tex_coords, QD3DBatchItem *item,
                        D3DCOLOR color);

    void queueAntialiasedLines(const QPainterPath &path, QD3DBatchItem **item, const QRectF &brect);
    void queueAliasedLines(const QLineF *lines, int lineCount, QD3DBatchItem **item, D3DCOLOR color, bool transform);
    void queueAliasedLines(const QPainterPath &path, QD3DBatchItem **item, D3DCOLOR color);

    int drawAntialiasedMask(int offset, int maxoffset);
    void drawAliasedMask(int offset);
    void drawAntialiasedBoundingRect(QD3DBatchItem *item);
    void drawAliasedBoundingRect(QD3DBatchItem *item);
    void drawTextItem(QD3DBatchItem *item);
    void drawAliasedLines(QD3DBatchItem *item);

    void setMaskSize(QSize size);

    void beforeReset();
    void afterReset();

    IDirect3DSurface9 *freeMaskSurface();

    void lock();
    void unlock();

    inline int index() { return m_index; }

#ifdef QT_DEBUG_VERTEXBUFFER_ACCESS
    enum VertexBufferAccess {
        CLEAR   = 0x00,
        READ    = 0x01,
        WRITE   = 0x02
    };
    int accesscontrol[QT_VERTEX_BUF_SIZE];
#endif

private:
    void addTrap(const Trapezoid &trap);
    void tessellate(const QPolygonF &poly);
    inline void lineToStencil(qreal x, qreal y);
    inline void curveToStencil(const QPointF &cp1, const QPointF &cp2, const QPointF &ep);
    QRectF pathToVertexArrays(const QPainterPath &path);

    QDirect3DPaintEnginePrivate *m_pe;

    qreal m_xoffset, m_yoffset;
    int m_startindex;
    int m_index;
    int m_height, m_width;
    LPDIRECT3DVERTEXBUFFER9 m_d3dvbuff;
    vertex *m_vbuff;
    QD3DBatchItem *m_item;
    QRectF m_boundingRect;

    qreal max_x;
    qreal max_y;
    qreal min_x;
    qreal min_y;
    qreal firstx;
    qreal firsty;

    QPointF tess_lastpoint;
    int tess_index;
    QD3DMaskAllocator m_maskallocator;

    bool m_locked;
    IDirect3DTexture9 *m_mask;
    IDirect3DSurface9 *m_maskSurface;
    IDirect3DSurface9 *m_depthStencilSurface;

    D3DCOLOR m_color;
    D3DXMATRIX m_d3dIdentityMatrix;
    bool m_clearmask;
};

QD3DStateManager::QD3DStateManager(LPDIRECT3DDEVICE9 pDevice, ID3DXEffect *effect)
    : m_pDevice(pDevice), m_effect(effect), m_refs(0)
{
    reset();
}

void QD3DStateManager::reset()
{
    m_radgradfd = -1;

    m_pass = -1;
    m_maskchannel = -1;
    m_brushmode = -1;
    m_texture = 0;
    m_xoffset = INT_MAX;
    m_yoffset = INT_MAX;

    m_vertexshader = 0;
    m_pixelshader = 0;

    ZeroMemory(&m_transformation, sizeof(D3DMATRIX));
    ZeroMemory(&m_projection, sizeof(D3DMATRIX));
    ZeroMemory(m_textures, sizeof(LPDIRECT3DBASETEXTURE9) * D3D_STAGE_COUNT);
    FillMemory(m_samplerstates, sizeof(DWORD) * D3D_SAMPLE_STATES * D3D_STAGE_COUNT, 0xFFFFFFFE);
    FillMemory(m_texturestates, sizeof(DWORD) * D3D_TEXTURE_STATES * D3D_STAGE_COUNT, 0xFFFFFFFE);
    FillMemory(m_renderstate, sizeof(DWORD) * D3D_RENDER_STATES, 0xFFFFFFFE);
}

inline void QD3DStateManager::beginPass(int pass)
{
    if (pass != m_pass) {
        if (m_pass != -1)
            m_effect->EndPass();
        m_effect->BeginPass(pass);
        m_pass = pass;
    }
}

inline void QD3DStateManager::endPass()
{
    if (m_pass != -1) {
        m_pass = -1;
        m_effect->EndPass();
    }
}

inline void QD3DStateManager::startStateBlock() {
    m_changed = false;
}

inline void QD3DStateManager::setBrushMode(int mode)
{
    if (mode != m_brushmode) {
        m_effect->SetInt("g_mBrushMode", mode);
        m_brushmode = mode;
        m_changed = true;
    }
}

inline void QD3DStateManager::setTexture(LPDIRECT3DBASETEXTURE9 pTexture, QGradient::Spread spread)
{
    switch(spread) {
        case QGradient::RepeatSpread:
            SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
            SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
            break;
        case QGradient::ReflectSpread:
            SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);
            SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);
            break;
        default:
            SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
            SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
            break;
    };

    if (pTexture != m_texture) {
        m_texture = pTexture;
        m_effect->SetTexture("g_mTexture", pTexture);
        m_changed = true;
    }
}

inline void QD3DStateManager::setTransformation(const D3DXMATRIX *pMatrix)
{
    if (*pMatrix != m_transformation) {
        m_effect->SetMatrix("g_mTransformation", pMatrix);
        m_transformation = *pMatrix;
        m_changed = true;
    }
}

inline void QD3DStateManager::setProjection(const D3DXMATRIX *pMatrix)
{
    if (*pMatrix != m_projection) {
        m_effect->SetMatrix("g_mViewProjection", pMatrix);
        m_projection = *pMatrix;
        m_changed = true;
    }
}

inline void QD3DStateManager::setFocalDistance(const qreal &fd)
{
    if (fd != m_radgradfd) {
        m_effect->SetFloat("g_mFocalDist", fd);
        m_changed = true;
        m_radgradfd = fd;
    }
}

inline void QD3DStateManager::setMaskOffset(qreal x, qreal y)
{
    if (x != m_xoffset || y != m_yoffset) {
        float offset[2] = {x, y};
        m_effect->SetFloatArray("g_mMaskOffset", offset, 2);
        m_xoffset = x;
        m_yoffset = y;
        m_changed = true;
    }
}

inline void QD3DStateManager::setMaskChannel(int channel)
{
    if (m_maskchannel != channel) {
        m_effect->SetIntArray("g_mChannel", m_maskChannels[channel], 4);
        m_maskchannel = channel;
        m_changed = true;
    }
}

inline void QD3DStateManager::endStateBlock()
{
    if (m_changed) {
        m_effect->CommitChanges();
        m_changed = false;
    }
}

STDMETHODIMP QD3DStateManager::QueryInterface(REFIID iid, LPVOID *ppv)
{
    if(iid == IID_IUnknown || iid == IID_ID3DXEffectStateManager)
    {
        *ppv = this;
        ++m_refs;
        return NOERROR;
    }
    *ppv = NULL;
    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) QD3DStateManager::AddRef(void)
{
    return (ULONG)InterlockedIncrement( &m_refs );
}


STDMETHODIMP_(ULONG) QD3DStateManager::Release(void)
{
    if( 0L == InterlockedDecrement( &m_refs ) ) {
        delete this;
        return 0L;
    }

    return m_refs;
}
STDMETHODIMP QD3DStateManager::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix)
{
    return m_pDevice->SetTransform(State, pMatrix);
}

STDMETHODIMP QD3DStateManager::SetMaterial(CONST D3DMATERIAL9 *pMaterial)
{
    return m_pDevice->SetMaterial(pMaterial);
}

STDMETHODIMP QD3DStateManager::SetLight(DWORD Index, CONST D3DLIGHT9 *pLight)
{
    return m_pDevice->SetLight(Index, pLight);
}

STDMETHODIMP QD3DStateManager::LightEnable(DWORD Index, BOOL Enable)
{
    return m_pDevice->LightEnable(Index, Enable);
}

STDMETHODIMP QD3DStateManager::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
    if (State < D3D_RENDER_STATES) {
        if (m_renderstate[State] == Value)
            return S_OK;
        m_renderstate[State] = Value;
    }
    return m_pDevice->SetRenderState(State, Value);
}

STDMETHODIMP QD3DStateManager::SetTexture(DWORD Stage, LPDIRECT3DBASETEXTURE9 pTexture)
{
    if (Stage < D3D_STAGE_COUNT) {
        if (m_textures[Stage] == pTexture)
            return S_OK;
        m_textures[Stage] = pTexture;
    }
    return m_pDevice->SetTexture(Stage, pTexture);
}

STDMETHODIMP QD3DStateManager::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
    if (Stage < D3D_STAGE_COUNT && Type < D3D_TEXTURE_STATES) {
        if (m_texturestates[Stage][Type] == Value)
            return S_OK;
        m_texturestates[Stage][Type] = Value;
    }
    return m_pDevice->SetTextureStageState(Stage, Type, Value);
}

STDMETHODIMP QD3DStateManager::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
    if (Sampler < D3D_STAGE_COUNT && Type < D3D_SAMPLE_STATES) {
        if (m_samplerstates[Sampler][Type] == Value)
            return S_OK;
        m_samplerstates[Sampler][Type] = Value;
    }
    return m_pDevice->SetSamplerState(Sampler, Type, Value);
}

STDMETHODIMP QD3DStateManager::SetNPatchMode(FLOAT NumSegments)
{
    return m_pDevice->SetNPatchMode(NumSegments);
}

STDMETHODIMP QD3DStateManager::SetFVF(DWORD FVF)
{
    return m_pDevice->SetFVF(FVF);
}

STDMETHODIMP QD3DStateManager::SetVertexShader(LPDIRECT3DVERTEXSHADER9 pShader)
{
    if (m_vertexshader == pShader)
        return S_OK;
    m_vertexshader = pShader;
    return m_pDevice->SetVertexShader(pShader);
}

STDMETHODIMP QD3DStateManager::SetVertexShaderConstantF(UINT RegisterIndex, CONST FLOAT *pConstantData, UINT RegisterCount)
{
    return m_pDevice->SetVertexShaderConstantF(RegisterIndex, pConstantData, RegisterCount);
}

STDMETHODIMP QD3DStateManager::SetVertexShaderConstantI(UINT RegisterIndex, CONST INT *pConstantData, UINT RegisterCount)
{
    return m_pDevice->SetVertexShaderConstantI(RegisterIndex, pConstantData, RegisterCount);
}

STDMETHODIMP QD3DStateManager::SetVertexShaderConstantB(UINT RegisterIndex, CONST BOOL *pConstantData, UINT RegisterCount)
{
    return m_pDevice->SetVertexShaderConstantB(RegisterIndex, pConstantData, RegisterCount);
}

STDMETHODIMP QD3DStateManager::SetPixelShader(LPDIRECT3DPIXELSHADER9 pShader)
{
    if (m_pixelshader == pShader)
        return S_OK;
    m_pixelshader = pShader;
    return m_pDevice->SetPixelShader(pShader);
}

STDMETHODIMP QD3DStateManager::SetPixelShaderConstantF(UINT RegisterIndex, CONST FLOAT *pConstantData, UINT RegisterCount)
{
    return m_pDevice->SetPixelShaderConstantF(RegisterIndex, pConstantData, RegisterCount);
}

STDMETHODIMP QD3DStateManager::SetPixelShaderConstantI(UINT RegisterIndex, CONST INT *pConstantData, UINT RegisterCount)
{
    return m_pDevice->SetPixelShaderConstantI(RegisterIndex, pConstantData, RegisterCount);
}

STDMETHODIMP QD3DStateManager::SetPixelShaderConstantB(UINT RegisterIndex, CONST BOOL *pConstantData, UINT RegisterCount)
{
    return m_pDevice->SetPixelShaderConstantB(RegisterIndex, pConstantData, RegisterCount);
}

#define QD3D_GRADIENT_CACHE_SIZE    60
#define QD3D_GRADIENT_PALETTE_SIZE  1024

class QD3DGradientCache
{
    struct CacheInfo
    {
        inline CacheInfo(QGradientStops s, qreal op) :
            stops(s), opacity(op) {}

        IDirect3DTexture9 *texture;
        QGradientStops stops;
        qreal opacity;
    };

    typedef QMultiHash<quint64, CacheInfo> QD3DGradientColorTableHash;

public:
    QD3DGradientCache(LPDIRECT3DDEVICE9 device);
    ~QD3DGradientCache();

    inline IDirect3DTexture9 *getBuffer(const QGradientStops &stops, qreal opacity);

protected:
    inline void generateGradientColorTable(const QGradientStops& s,
                                           uint *colorTable,
                                           int size, qreal opacity) const;
    IDirect3DTexture9 *addCacheElement(quint64 hash_val, const QGradientStops &stops, qreal opacity);
    void cleanCache();

    QD3DGradientColorTableHash cache;
    LPDIRECT3DDEVICE9 m_device;
};

QD3DGradientCache::QD3DGradientCache(LPDIRECT3DDEVICE9 device)
    : m_device(device)
{

}

QD3DGradientCache::~QD3DGradientCache()
{
    cleanCache();
}

inline IDirect3DTexture9 *QD3DGradientCache::getBuffer(const QGradientStops &stops, qreal opacity)
{
    quint64 hash_val = 0;

    for (int i = 0; i < stops.size() && i <= 2; i++)
        hash_val += stops[i].second.rgba();

    QD3DGradientColorTableHash::const_iterator it = cache.constFind(hash_val);

    if (it == cache.constEnd())
        return addCacheElement(hash_val, stops, opacity);
    else {
        do {
            const CacheInfo &cache_info = it.value();
            if (cache_info.stops == stops && cache_info.opacity == opacity) {
                return cache_info.texture;
            }
            ++it;
        } while (it != cache.constEnd() && it.key() == hash_val);
        // an exact match for these stops and opacity was not found, create new cache
        return addCacheElement(hash_val, stops, opacity);
    }
}

void QD3DGradientCache::generateGradientColorTable(const QGradientStops& s, uint *colorTable, int size, qreal opacity) const
{
    int pos = 0;
    qreal fpos = 0.0;
    qreal incr = 1.0 / qreal(size);
    QVector<uint> colors(s.size());

    for (int i = 0; i < s.size(); ++i)
        colors[i] = s[i].second.rgba();

    uint alpha = qRound(opacity * 255);
    while (fpos < s.first().first) {
        colorTable[pos] = ARGB_COMBINE_ALPHA(colors[0], alpha);
        pos++;
        fpos += incr;
    }

    for (int i = 0; i < s.size() - 1; ++i) {
        qreal delta = 1/(s[i+1].first - s[i].first);
        while (fpos < s[i+1].first && pos < size) {
            int dist = int(256 * ((fpos - s[i].first) * delta));
            int idist = 256 - dist;
            uint current_color = ARGB_COMBINE_ALPHA(colors[i], alpha);
            uint next_color = ARGB_COMBINE_ALPHA(colors[i+1], alpha);
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
            colorTable[pos] = INTERPOLATE_PIXEL_256(current_color, idist, next_color, dist);
#else
            uint c = INTERPOLATE_PIXEL_256(current_color, idist, next_color, dist);
            colorTable[pos] = ( (c << 24) & 0xff000000)
                              | ((c >> 24) & 0x000000ff)
                              | ((c << 8) & 0x00ff0000)
                              | ((c >> 8) & 0x0000ff00);
#endif // Q_BYTE_ORDER
            ++pos;
            fpos += incr;
        }
    }
    for (;pos < size; ++pos)
        colorTable[pos] = colors[s.size() - 1];
}

IDirect3DTexture9 *QD3DGradientCache::addCacheElement(quint64 hash_val, const QGradientStops &stops, qreal opacity)
{
    if (cache.size() == QD3D_GRADIENT_CACHE_SIZE) {
        int elem_to_remove = qrand() % QD3D_GRADIENT_CACHE_SIZE;
        uint key = cache.keys()[elem_to_remove];

        // need to call release on each removed cache entry:
        QD3DGradientColorTableHash::const_iterator it = cache.constFind(key);
        do {
            it.value().texture->Release();
        } while (++it != cache.constEnd() && it.key() == key);

        cache.remove(key); // may remove more than 1, but OK
    }

    CacheInfo cache_entry(stops, opacity);
    uint buffer[QD3D_GRADIENT_PALETTE_SIZE];
    generateGradientColorTable(stops, buffer, QD3D_GRADIENT_PALETTE_SIZE, opacity);

    if (FAILED(m_device->CreateTexture(QD3D_GRADIENT_PALETTE_SIZE, 1, 1, 0,
                    D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &cache_entry.texture, 0))) {
        qWarning("QD3DGradientCache::addCacheElement(): unable to create Direct3D texture.");
        return 0;
    }

    D3DLOCKED_RECT rect;
    if (FAILED(cache_entry.texture->LockRect(0, &rect, 0, 0))) {
        qDebug() << "QD3DGradientCache::addCacheElement(): unable to lock texture rect.";
        return 0;
    }
    memcpy(rect.pBits, buffer, rect.Pitch);
    cache_entry.texture->UnlockRect(0);

    return cache.insert(hash_val, cache_entry).value().texture;
}

void QD3DGradientCache::cleanCache()
{
    QD3DGradientColorTableHash::const_iterator it = cache.constBegin();
    for (; it != cache.constEnd(); ++it) {
        const CacheInfo &cache_info = it.value();
        cache_info.texture->Release();
    }
    cache.clear();
}

QD3DWindowManager::QD3DWindowManager() :
    m_status(NoStatus), m_dummy(0), m_device(0), m_pd(0), m_current(0)
{

}

QD3DWindowManager::~QD3DWindowManager()
{
}

void QD3DWindowManager::setPaintDevice(QPaintDevice *pd)
{
    m_status = NoStatus;
    m_pd = pd;
    m_current = 0;

    if (m_device->TestCooperativeLevel() != D3D_OK) {
        m_status = NeedsReseting;
        return;
    }

    m_current = m_swapchains.value(pd, 0);
    QWidget *w = static_cast<QWidget*>(pd);

    if (m_current) {
        if (m_current->size != w->size()) {
            m_swapchains.remove(pd);
            m_current->surface->Release();
            m_current->swapchain->Release();
            delete m_current;
            m_current = 0;
        }
    }

    if (!m_current) {
        m_current = createSwapChain(w);
        updateMaxSize();
    }
}

int QD3DWindowManager::status() const
{
    return m_status;
}

void QD3DWindowManager::reset()
{
    QList<QPaintDevice *> pds = m_swapchains.keys();

    QMap<QPaintDevice *, D3DSwapChain *>::const_iterator i = m_swapchains.constBegin();
    while (i != m_swapchains.constEnd()) {
        i.value()->surface->Release();
        i.value()->swapchain->Release();
        ++i;
    }
    qDeleteAll(m_swapchains.values());
    m_swapchains.clear();

    D3DPRESENT_PARAMETERS params;
    initPresentParameters(&params);
    params.hDeviceWindow = m_dummy;

    HRESULT res = m_device->Reset(&params);
    if (FAILED(res)) {
        switch (res) {
            case D3DERR_DEVICELOST:
                qWarning("QDirect3DPaintEngine: Reset failed (D3DERR_DEVICELOST)");
                break;
            case D3DERR_DRIVERINTERNALERROR:
                qWarning("QDirect3DPaintEngine: Reset failed (D3DERR_DRIVERINTERNALERROR)");
                break;
            case D3DERR_OUTOFVIDEOMEMORY:
                qWarning("QDirect3DPaintEngine: Reset failed (D3DERR_OUTOFVIDEOMEMORY)");
                break;
            default:
                qWarning("QDirect3DPaintEngine: Reset failed");
        };
    }

    for (int i=0; i<pds.count(); ++i) {
        QWidget *w = static_cast<QWidget*>(pds.at(i));
        createSwapChain(w);
    }

    // reset the mask as well
    m_status = MaxSizeChanged;

    setPaintDevice(m_pd);
    updateMaxSize();
}

LPDIRECT3DSURFACE9 QD3DWindowManager::renderTarget()
{
    return m_current ? m_current->surface : 0;
}

LPDIRECT3DSURFACE9 QD3DWindowManager::surface(QPaintDevice *pd)
{
    D3DSwapChain *swapchain = m_swapchains.value(pd, 0);
    return swapchain ? swapchain->surface : 0;
}

LPDIRECT3DSWAPCHAIN9 QD3DWindowManager::swapChain(QPaintDevice *pd)
{
    D3DSwapChain *swapchain = m_swapchains.value(pd, 0);
    return swapchain ? swapchain->swapchain : 0;
}

void QD3DWindowManager::releasePaintDevice(QPaintDevice *pd)
{
    D3DSwapChain *swapchain = m_swapchains.take(pd);

    if (swapchain) {
        swapchain->surface->Release();
        swapchain->swapchain->Release();
        delete swapchain;
        if (swapchain == m_current)
            m_current = 0;
    }
}

LPDIRECT3DDEVICE9 QD3DWindowManager::device()
{
    return m_device;
}

void QD3DWindowManager::cleanup()
{
    QPixmapCache::clear();
    qd3d_glyph_cache()->cleanCache();

    // release doomed textures
    for (int k=0; k<qd3d_release_list.size(); ++k)
        qd3d_release_list.at(k)->Release();
    qd3d_release_list.clear();

    QMap<QPaintDevice *, D3DSwapChain *>::const_iterator i = m_swapchains.constBegin();
    while (i != m_swapchains.constEnd()) {
        i.value()->surface->Release();
        i.value()->swapchain->Release();
        ++i;
    }
    qDeleteAll(m_swapchains.values());

    if (m_device)
        m_device->Release();

    DestroyWindow(m_dummy);
    QString cname(QLatin1String("qt_d3d_dummy"));
    QT_WA({
        UnregisterClass((TCHAR*)cname.utf16(), (HINSTANCE)qWinAppInst());
    } , {
        UnregisterClassA(cname.toLatin1(), (HINSTANCE)qWinAppInst());
    });
}

QSize QD3DWindowManager::maxSize() const
{
    return m_maxSize;
}

extern "C" {
    LRESULT CALLBACK QtWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};

void QD3DWindowManager::init(LPDIRECT3D9 object)
{
    QString cname(QLatin1String("qt_d3d_dummy"));
    uint style = CS_DBLCLKS | CS_SAVEBITS;
    ATOM atom;
    QT_WA({
        WNDCLASS wc;
        wc.style         = style;
        wc.lpfnWndProc   = (WNDPROC)QtWndProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = 0;
        wc.hInstance     = (HINSTANCE)qWinAppInst();
        wc.hIcon         = 0;
        wc.hCursor       = 0;
        wc.hbrBackground = 0;
        wc.lpszMenuName  = 0;
        wc.lpszClassName = (TCHAR*)cname.utf16();
        atom = RegisterClass(&wc);
    } , {
        WNDCLASSA wc;
        wc.style         = style;
        wc.lpfnWndProc   = (WNDPROC)QtWndProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = 0;
        wc.hInstance     = (HINSTANCE)qWinAppInst();
        wc.hIcon         = 0;
        wc.hCursor       = 0;
        wc.hbrBackground = 0;
        wc.lpszMenuName  = 0;
        QByteArray tempArray = cname.toLatin1();
        wc.lpszClassName = tempArray;
        atom = RegisterClassA(&wc);
    });

    QT_WA({
        const TCHAR *className = (TCHAR*)cname.utf16();
        m_dummy = CreateWindow(className, className, 0,
                               0, 0, 1, 1,
                               0, 0, qWinAppInst(), 0);
    } , {
        m_dummy = CreateWindowA(cname.latin1(), cname.toLatin1(), 0,
                                0, 0, 1, 1,
                                0, 0, qWinAppInst(), 0);
    });

    D3DPRESENT_PARAMETERS params;
    initPresentParameters(&params);
    params.hDeviceWindow = m_dummy;

    HRESULT res = object->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, 0,
        D3DCREATE_PUREDEVICE|D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_NOWINDOWCHANGES|D3DCREATE_FPU_PRESERVE,
        &params, &m_device);

    if (FAILED(res) || m_device == 0)
        qWarning("QDirect3DPaintEngine: failed to create Direct3D device (error=0x%x).", res);
}

void QD3DWindowManager::updateMaxSize()
{
    int w = 0, h = 0;
    QMap<QPaintDevice *, D3DSwapChain *>::const_iterator i = m_swapchains.constBegin();
    while (i != m_swapchains.constEnd()) {

        int nw = i.key()->width();
        if (nw > w)
            w = nw;

        int nh = i.key()->height();
        if (nh > h)
            h = nh;

        ++i;
    }

    QSize newsize = QSize(w, h);
    if (newsize != m_maxSize) {
        m_status |= MaxSizeChanged;
        m_maxSize = newsize;
    }
}

void QD3DWindowManager::initPresentParameters(D3DPRESENT_PARAMETERS *params)
{
    ZeroMemory(params, sizeof(D3DPRESENT_PARAMETERS));
    params->Windowed = true;
    params->SwapEffect = D3DSWAPEFFECT_COPY;
    params->BackBufferFormat = D3DFMT_UNKNOWN;
    params->PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    params->Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
}

QD3DWindowManager::D3DSwapChain *QD3DWindowManager::createSwapChain(QWidget *w)
{
    D3DPRESENT_PARAMETERS params;
    initPresentParameters(&params);
    params.hDeviceWindow = w->winId();
    D3DSwapChain *swapchain = new D3DSwapChain();
    swapchain->size = w->size();
    if (FAILED(m_device->CreateAdditionalSwapChain(&params, &swapchain->swapchain)))
        qWarning("QDirect3DPaintEngine: CreateAdditionalSwapChain failed");
    if (FAILED(swapchain->swapchain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &swapchain->surface)))
        qWarning("QDirect3DPaintEngine: GetBackBuffer failed");
    m_swapchains.insert(w, swapchain);
    connect(w, SIGNAL(destroyed(QObject *)), SLOT(cleanupPaintDevice(QObject *)));

    // init with background color
    QColor bg = w->palette().color(QPalette::Background);
    m_device->ColorFill(swapchain->surface, 0, D3DCOLOR_ARGB(bg.alpha(), bg.red(),bg.green(),bg.blue()));

    return swapchain;
}

void QD3DWindowManager::cleanupPaintDevice(QObject *object)
{
    QWidget *w = static_cast<QWidget *>(object);
    releasePaintDevice(w);
}

QD3DMaskAllocator::QD3DMaskAllocator() {
    reset();
}

void QD3DMaskAllocator::reset() {
    mask_position.x = mask_position.y = 0;
    mask_position.channel = 0;
    mask_offsetX2 = mask_offsetY2 = 0;
}

// flow allocate free space in mask texture
bool QD3DMaskAllocator::allocate(int w, int h)
{
    w += 3;
    h += 3;

    if (w > width)
        w = width;
    if (h > height)
        h = height;

    bool flush_mask = false;

    if ((height - mask_offsetY2) >= h && (width - mask_position.x) >= w) {
        mask_position.y = mask_offsetY2;
    } else if ((width - mask_offsetX2) >= w) {
        mask_position.y = 0;
        mask_position.x = mask_offsetX2;
    } else if (mask_position.channel < 3) {
        ++mask_position.channel;
        mask_position.x = mask_position.y = 0;
        mask_offsetX2 = mask_offsetY2 = 0;
    } else {
        reset();
        flush_mask = true;
    }

    int newoffset = mask_position.x + w;
    if (mask_offsetX2 < newoffset)
        mask_offsetX2 = newoffset;
    mask_offsetY2 = (mask_position.y + h);

    return flush_mask;
}

void QD3DMaskAllocator::setSize(int w, int h) {
    width = w;
    height = h;
}

int QD3DStateManager::m_maskChannels[4][4] =
    {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};

QD3DVertexBuffer::QD3DVertexBuffer(QDirect3DPaintEnginePrivate *pe)
    : m_pe(pe), m_d3dvbuff(0), m_maskSurface(0), m_depthStencilSurface(0),
      m_locked(false), m_mask(0), m_startindex(0), m_index(0), m_vbuff(0), m_clearmask(true)
{
    if (FAILED(D3DXMatrixIdentity(&m_d3dIdentityMatrix))) {
        qWarning("QDirect3DPaintEngine: D3DXMatrixIdentity failed");
    }
#ifdef QT_DEBUG_VERTEXBUFFER_ACCESS
    memset(accesscontrol, 0, QT_VERTEX_BUF_SIZE * sizeof(VertexBufferAccess));
#endif

    // create vertex buffer
    afterReset();
}

QD3DVertexBuffer::~QD3DVertexBuffer()
{
    if (m_maskSurface)
        m_maskSurface->Release();

    if (m_mask)
        m_mask->Release();

    if (m_depthStencilSurface)
        m_depthStencilSurface->Release();

    if (m_d3dvbuff)
        m_d3dvbuff->Release();
}

void QD3DVertexBuffer::lock()
{
    if (!m_locked) {
        DWORD lockflags = D3DLOCK_NOOVERWRITE;
        if (m_startindex >= QT_VERTEX_RESET_LIMIT) {
            m_startindex = 0;
            m_index = 0;

#ifdef QT_DEBUG_VERTEXBUFFER_ACCESS
            for (int i=0; i<QT_VERTEX_BUF_SIZE; ++i) {
                if (accesscontrol[i] != (WRITE|READ) && accesscontrol[i] != CLEAR)
                    qDebug() << "Vertex Buffer: Access Error";
            }
            memset(accesscontrol, 0, QT_VERTEX_BUF_SIZE * sizeof(VertexBufferAccess));
#endif

            lockflags = D3DLOCK_DISCARD;
        }

        if (FAILED(m_d3dvbuff->Lock(0, 0, (void**)&m_vbuff, lockflags))) {
            qWarning() << "QDirect3DPaintEngine: unable to lock vertex buffer.";
        }
        m_locked = true;
    }
}

void QD3DVertexBuffer::unlock()
{
    if (m_locked) {
        if (FAILED(m_d3dvbuff->Unlock())) {
            qWarning() << "QDirect3DPaintEngine: unable to unlock vertex buffer.";
        }
        m_locked = false;
    }
}

void QD3DVertexBuffer::setClipPath(const QPainterPath &path, QD3DBatchItem **item)
{
    lock();

    m_item = *item;
    m_item->m_maskpos.x = m_item->m_maskpos.y = 0;
    m_item->m_maskpos.channel = 3;
    m_item->m_info |= QD3DBatchItem::BI_CLIP;

    bool winding = (path.fillRule() == Qt::WindingFill);
    if (winding)
        m_item->m_info |= QD3DBatchItem::BI_WINDING;

    if (!path.isEmpty()) {
        m_item->m_info |= QD3DBatchItem::BI_MASK;
        m_item->m_info &= ~QD3DBatchItem::BI_AA;
        m_color = 0;
        QRectF brect = pathToVertexArrays(path);
        queueRect(brect, m_item, 0);
    }

    *item = m_item;
}



void QD3DVertexBuffer::queueAntialiasedMask(const QPolygonF &poly, QD3DBatchItem **item, const QRectF &brect)
{
    lock();

    m_item = *item;
    m_item->m_info |= QD3DBatchItem::BI_MASK;
    setWinding(m_item->m_info & QD3DBatchItem::BI_WINDING);

    int xoffset = m_item->m_maskpos.x;
    int yoffset = m_item->m_maskpos.y;

    int x = brect.left();
    int y = brect.top();

    m_item->m_xoffset = (xoffset - x) + 1;
    m_item->m_yoffset = (yoffset - y) + 1;

    m_boundingRect = brect;
    tessellate(poly);

    *item = m_item;
}

QRectF QD3DVertexBuffer::queueAliasedMask(const QPainterPath &path, QD3DBatchItem **item, D3DCOLOR color)
{
    lock();

    m_color = color;
    m_item = *item;
    m_item->m_info |= QD3DBatchItem::BI_MASK;

    bool winding = (path.fillRule() == Qt::WindingFill);
    if (winding)
        m_item->m_info |= QD3DBatchItem::BI_WINDING;

    QRectF result = pathToVertexArrays(path);
    *item = m_item;
    return result;
}

// used for drawing aliased transformed rects directly
// don't use for antialiased or masked drawing
void QD3DVertexBuffer::queueRect(const QRectF &rect, QD3DBatchItem *item, D3DCOLOR color, const QPolygonF &trect)
{
    lock();

    qreal zval = (item->m_info & QD3DBatchItem::BI_CLIP) ? 0.0f : 0.5f;
    item->m_info |= QD3DBatchItem::BI_BRECT;

    // if the item does not have a mask, the offset is different
    if (!(item->m_info & QD3DBatchItem::BI_MASK)) {
        item->m_offset = m_index;
        item->m_count = (item->m_info & QD3DBatchItem::BI_AA) ? 0 : -2;
    }

    qreal x1 = rect.left();
    qreal y1 = rect.top();
    qreal x2 = rect.right();
    qreal y2 = rect.bottom();

    QPointF tc = trect.at(0);
    vertex v1 = {D3DXVECTOR3(x1, y1, zval), color,
        tc.x(), tc.y(), 0.f, 0.f,
        0.f   , 0.f   , 0.f, 0.f};

    tc = trect.at(1);
    vertex v2 = {D3DXVECTOR3(x2, y1, zval), color,
        tc.x(), tc.y(), 0.f, 0.f,
        0.f   , 0.f   , 0.f, 0.f};

    tc = trect.at(2);
    vertex v3 = {D3DXVECTOR3(x2, y2, zval), color,
        tc.x(), tc.y(), 0.f, 0.f,
        0.f   , 0.f   , 0.f, 0.f};

    tc = trect.at(3);
    vertex v4 = {D3DXVECTOR3(x1, y2, zval), color,
        tc.x(), tc.y(), 0.f, 0.f,
        0.f   , 0.f   , 0.f, 0.f};

#ifdef QT_DEBUG_VERTEXBUFFER_ACCESS
    for (int i=m_index; i<(m_index + 4); ++i) {
        if ((m_index + 4) > QT_VERTEX_BUF_SIZE)
            qDebug() << "Vertex Buffer: Buffer overflow";
        if (accesscontrol[i] != CLEAR)
            qDebug() << "Vertex Buffer: Access Error";
        accesscontrol[i] |= WRITE;
    }
#endif

    m_vbuff[m_index++] = v1;
    m_vbuff[m_index++] = v2;
    m_vbuff[m_index++] = v3;
    m_vbuff[m_index++] = v4;

    m_startindex = m_index;
}


QD3DMaskPosition QD3DVertexBuffer::allocateMaskPosition(const QRectF &brect, bool *breakbatch)
{
    *breakbatch = m_maskallocator.allocate(brect.width(), brect.height());
    return m_maskallocator.mask_position;
}

void QD3DVertexBuffer::queueRect(const QRectF &rect, QD3DBatchItem *item, D3DCOLOR color)
{
    lock();

    QRectF brect;
    item->m_info |= QD3DBatchItem::BI_BRECT;
    qreal zval = (item->m_info & QD3DBatchItem::BI_CLIP) ? 0.0f : 0.5f;

    if (item->m_info & QD3DBatchItem::BI_AA) {
        int xoffset = item->m_maskpos.x;
        int yoffset = item->m_maskpos.y;

        int x = rect.left();
        int y = rect.top();

        brect = QRectF(x, y, rect.width() + 1, rect.height() + 1);

        item->m_xoffset = (xoffset - x) + 1;
        item->m_yoffset = (yoffset - y) + 1;

        // if the item does not have a mask, the offset is different
        if (!(item->m_info & QD3DBatchItem::BI_MASK)) {
            item->m_offset = m_index;
            item->m_count = 0;
        }
    } else {
        brect = rect;

        if (!(item->m_info & QD3DBatchItem::BI_MASK)) {
            item->m_offset = m_index;
            item->m_count = -2;
        }
    }

    vertex v1 = {D3DXVECTOR3(brect.left(), brect.bottom(), zval), color,
        0.f, 0.f, 0.f, 0.f,
        0.f, 0.f, 0.f, 0.f};
    vertex v2 = {D3DXVECTOR3(brect.left(), brect.top(), zval), color,
        0.f, 0.f, 0.f, 0.f,
        0.f, 0.f, 0.f, 0.f};
    vertex v3 = {D3DXVECTOR3(brect.right(), brect.top(), zval), color,
        0.f, 0.f, 0.f, 0.f,
        0.f, 0.f, 0.f, 0.f};
    vertex v4 = {D3DXVECTOR3(brect.right(), brect.bottom(), zval), color,
        0.f, 0.f, 0.f, 0.f,
        0.f, 0.f, 0.f, 0.f};

#ifdef QT_DEBUG_VERTEXBUFFER_ACCESS
    for (int i=m_index; i<(m_index + 4); ++i) {
        if ((m_index + 4) > QT_VERTEX_BUF_SIZE)
            qDebug() << "Vertex Buffer: Buffer overflow";
        if (accesscontrol[i] != CLEAR)
            qDebug() << "Vertex Buffer: Access Error";
        accesscontrol[i] |= WRITE;
    }
#endif

    m_vbuff[m_index++] = v1;
    m_vbuff[m_index++] = v2;
    m_vbuff[m_index++] = v3;
    m_vbuff[m_index++] = v4;

    m_startindex = m_index;
}

void QD3DVertexBuffer::queueAntialiasedLines(const QPainterPath &path, QD3DBatchItem **item, const QRectF &brect)
{
    lock();

    m_item = *item;
    m_item->m_info |= QD3DBatchItem::BI_MASK;
    setWinding(m_item->m_info & QD3DBatchItem::BI_WINDING);

    int xoffset = m_item->m_maskpos.x;
    int yoffset = m_item->m_maskpos.y;
    int x = brect.left();
    int y = brect.top();

    m_item->m_xoffset = (xoffset - x) + 1;
    m_item->m_yoffset = (yoffset - y) + 1;

    m_boundingRect = brect;

    m_xoffset = (x - xoffset) + 0.5f;
    m_yoffset = (y - yoffset) + 0.5f;

    QPointF last;
    for (int i = 0; i < path.elementCount(); ++i) {
        QPainterPath::Element element = path.elementAt(i);

        //Q_ASSERT(!element.isCurveTo());

        if (element.isLineTo())
            QTessellator::tessellateRect(last, element, m_item->m_width);

        last = element;
    }

    m_item->m_offset = m_startindex;
    m_item->m_count = ( m_index - m_startindex ) / 3;
    m_startindex = m_index;

    *item = m_item;
}

void QD3DVertexBuffer::queueAliasedLines(const QLineF *lines, int lineCount, QD3DBatchItem **item, D3DCOLOR color, bool transform)
{
    lock();

    m_item = *item;
    m_item->m_info |= QD3DBatchItem::BI_FASTLINE;

    for (int i=0; i<lineCount; ++i) {
        QLineF line = transform ? m_pe->m_matrix.map(lines[i]) : lines[i];

        int x1 = line.x1();
        int y1 = line.y1();

        // extend the line one unit in the direction of the line vector
        qreal rx2 = line.x2();
        qreal ry2 = line.y2();
        line.setLength(1.0f);
        int x2 = (rx2 + line.dx());
        int y2 = (ry2 + line.dy());

        vertex v1 = {D3DXVECTOR3(x1, y1, 0.5f), color,
                     0.f, 0.f, 0.f, 0.f,
                     0.f, 0.f, 0.f, 0.f };
        vertex v2 = {D3DXVECTOR3(x2, y2, 0.5f), color,
                     0.f, 0.f, 0.f, 0.f,
                     0.f, 0.f, 0.f, 0.f };

        m_vbuff[m_index++] = v1;
        m_vbuff[m_index++] = v2;

        if (m_index >= (QT_VERTEX_BUF_SIZE - 16)) {
            m_item->m_offset = m_startindex;
            m_item->m_count = ( m_index - m_startindex ) / 2;
            m_startindex = m_index;

            QD3DBatchItem itemcopy = *m_item;
            m_item = m_pe->nextBatchItem();
            *m_item = itemcopy;

            lock();
        }
    }

    m_item->m_offset = m_startindex;
    m_item->m_count = ( m_index - m_startindex ) / 2;
    m_startindex = m_index;

    *item = m_item;
}

void QD3DVertexBuffer::queueTextGlyph(const QRectF &rect, const qreal *tex_coords,
                                      QD3DBatchItem *item, D3DCOLOR color)
{
    lock();

    qreal x1 = rect.left();
    qreal y1 = rect.top();
    qreal x2 = rect.right();
    qreal y2 = rect.bottom();

    vertex v1 = {D3DXVECTOR3(x1, y1, 0.5f), color,
                 tex_coords[0], tex_coords[1], 0.f, 0.f,
                 0.f   , 0.f   , 0.f, 0.f};
    vertex v2 = {D3DXVECTOR3(x2, y1, 0.5f), color,
                 tex_coords[2], tex_coords[1], 0.f, 0.f,
                 0.f   , 0.f   , 0.f, 0.f};
    vertex v3 = {D3DXVECTOR3(x2, y2, 0.5f), color,
                 tex_coords[2], tex_coords[3], 0.f, 0.f,
                 0.f   , 0.f   , 0.f, 0.f};
    vertex v4 = {D3DXVECTOR3(x1, y1, 0.5f), color,
                 tex_coords[0], tex_coords[1], 0.f, 0.f,
                 0.f   , 0.f   , 0.f, 0.f};
    vertex v5 = {D3DXVECTOR3(x2, y2, 0.5f), color,
                 tex_coords[2], tex_coords[3], 0.f, 0.f,
                 0.f   , 0.f   , 0.f, 0.f};
    vertex v6 = {D3DXVECTOR3(x1, y2, 0.5f), color,
                 tex_coords[0], tex_coords[3], 0.f, 0.f,
                 0.f   , 0.f   , 0.f, 0.f};

#ifdef QT_DEBUG_VERTEXBUFFER_ACCESS
    for (int i=m_index; i<(m_index + 6); ++i) {
        if ((m_index + 6) > QT_VERTEX_BUF_SIZE)
            qDebug() << "Vertex Buffer: Buffer overflow";
        if (accesscontrol[i] != CLEAR)
            qDebug() << "Vertex Buffer: Access Error";
        accesscontrol[i] |= WRITE;
    }
#endif

    m_vbuff[m_index++] = v1;
    m_vbuff[m_index++] = v2;
    m_vbuff[m_index++] = v3;
    m_vbuff[m_index++] = v4;
    m_vbuff[m_index++] = v5;
    m_vbuff[m_index++] = v6;

    m_startindex = m_index;
    ++item->m_count;
}

bool QD3DVertexBuffer::needsFlushing() const
{
    return (m_pe->m_batch.m_itemIndex >= QD3D_BATCH_SIZE || m_startindex >= QT_VERTEX_RESET_LIMIT);
}

void QD3DVertexBuffer::setMaskSize(QSize size)
{
    m_width = size.width();
    m_height = size.height();
    m_maskallocator.setSize(m_width, m_height);

    if (m_maskSurface)
        m_maskSurface->Release();

    if (m_mask)
        m_mask->Release();

    if (FAILED(m_pe->m_d3dDevice->CreateTexture(m_width, m_height, 1, D3DUSAGE_RENDERTARGET,
        D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_mask, NULL))) {
            qWarning() << "QDirect3DPaintEngine: CreateTexture() failed.";
    }

    if (m_depthStencilSurface)
        m_depthStencilSurface->Release();

    if (FAILED(m_pe->m_d3dDevice->CreateDepthStencilSurface(m_width, m_height, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0,
                                                   TRUE, &m_depthStencilSurface, NULL))) {
            qWarning() << "QDirect3DPaintEngine: CreateDepthStencilSurface() failed.";
    }

    m_pe->m_d3dDevice->SetDepthStencilSurface(m_depthStencilSurface);

    if (FAILED(m_mask->GetSurfaceLevel(0, &m_maskSurface))) {
        qWarning() << "QDirect3DPaintEngine: GetSurfaceLevel() failed.";
    }

    m_pe->m_d3dDevice->ColorFill(m_maskSurface, 0, D3DCOLOR_ARGB(255,0,0,0));
    D3DXMATRIX projMatrix;
    pD3DXMatrixOrthoOffCenterLH(&projMatrix, 0, m_width, m_height, 0, 0, 1);
    m_pe->m_effect->SetMatrix("g_mMaskProjection", &projMatrix);
    m_pe->m_effect->SetTexture("g_mAAMask", m_mask);
}

void QD3DVertexBuffer::beforeReset()
{
    m_maskallocator.reset();
    m_clearmask = true;

    if (m_maskSurface) {
        m_maskSurface->Release();
        m_maskSurface = 0;
    }

    if (m_mask) {
        m_mask->Release();
        m_mask = 0;
    }

    if (m_depthStencilSurface) {
        m_depthStencilSurface->Release();
        m_depthStencilSurface = 0;
    }

    if (m_d3dvbuff)
        m_d3dvbuff->Release();
}

void QD3DVertexBuffer::afterReset()
{
    if (FAILED(m_pe->m_d3dDevice->CreateVertexBuffer(QT_VERTEX_BUF_SIZE*sizeof(vertex), D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY,
        QD3DFVF_CSVERTEX,
        D3DPOOL_DEFAULT, &m_d3dvbuff, NULL))) {
            qWarning() << "QDirect3DPaintEngine: failed to create vertex buffer.";
    }

    m_pe->m_d3dDevice->SetStreamSource(0, m_d3dvbuff, 0, sizeof(vertex));
    m_pe->m_d3dDevice->SetFVF(QD3DFVF_CSVERTEX);

    m_startindex = 0;
    m_index = 0;
}

IDirect3DSurface9 *QD3DVertexBuffer::freeMaskSurface()
{
     // we need to make sure the mask is cleared when it's used for something else
    m_maskallocator.reset();
    m_clearmask = true;

    return m_maskSurface;
}

int QD3DVertexBuffer::drawAntialiasedMask(int offset, int maxoffset)
{
    int newoffset = offset;
    QD3DBatchItem *item = &(m_pe->m_batch.items[offset]);

    // set mask as render target
    if (FAILED(m_pe->m_d3dDevice->SetRenderTarget(0, m_maskSurface)))
        qWarning() << "QDirect3DPaintEngine: SetRenderTarget failed!";

    if (m_clearmask) {
        m_pe->m_d3dDevice->Clear(0, 0, D3DCLEAR_TARGET,D3DCOLOR_ARGB(0,0,0,0), 0, 0);
        m_clearmask = false;
    }

    // fill the mask
    m_pe->m_effect->BeginPass(PASS_AA_CREATEMASK);
    for (; newoffset<maxoffset; ++newoffset) {
        item = &(m_pe->m_batch.items[newoffset]);
        if (!(item->m_info & QD3DBatchItem::BI_AA) || !(item->m_info & QD3DBatchItem::BI_MASK)) {
            break;
        } else if (item->m_info & QD3DBatchItem::BI_MASKFULL) {
            item->m_info &= ~QD3DBatchItem::BI_MASKFULL;
            m_clearmask = true;
            break;
        }

        m_pe->m_statemanager->startStateBlock();
        if (item->m_info & QD3DBatchItem::BI_MASKSCISSOR) {
            RECT rect;
            QRectF srect = item->m_brect.adjusted(item->m_xoffset, item->m_yoffset,
                item->m_xoffset, item->m_yoffset);
            rect.left = qMax(qRound(srect.left()), 0);
            rect.top = qMax(qRound(srect.top()), 0);
            rect.bottom = qMin(m_height, qRound(srect.bottom()));
            rect.right = qMin(m_width, qRound(srect.right()));
            m_pe->m_d3dDevice->SetScissorRect(&rect);
            m_pe->m_statemanager->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
        }
        m_pe->m_statemanager->setMaskChannel(item->m_maskpos.channel);
        m_pe->m_statemanager->endStateBlock();

#ifdef QT_DEBUG_VERTEXBUFFER_ACCESS
        int vbstart = item->m_offset;
        for (int i=vbstart; i<(vbstart + (item->m_count * 3)); ++i) {
            if (accesscontrol[i] != WRITE)
                qDebug() << "Vertex Buffer: Access Error";
            accesscontrol[i] |= READ;
        }
#endif

        m_pe->m_d3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, item->m_offset, item->m_count);

        if (item->m_info & QD3DBatchItem::BI_MASKSCISSOR) {
            m_pe->m_statemanager->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        }
    }
    m_pe->m_effect->EndPass();

    return newoffset;
}

void QD3DVertexBuffer::drawAliasedMask(int offset)
{
    QD3DBatchItem *item = &(m_pe->m_batch.items[offset]);
    if (item->m_info & QD3DBatchItem::BI_MASK) {
        m_pe->m_effect->BeginPass( (item->m_info & QD3DBatchItem::BI_WINDING) ? PASS_STENCIL_WINDING : PASS_STENCIL_ODDEVEN );
        int prev_stop = 0;
        for (int i=0; i<item->m_pointstops.count(); ++i) {
            int stop = item->m_pointstops.at(i);

#ifdef QT_DEBUG_VERTEXBUFFER_ACCESS
            int vbstart = (item->m_offset + prev_stop);
            for (int j=vbstart; j<(vbstart+(stop - prev_stop)); ++j) {
                if (accesscontrol[j] != WRITE)
                    qDebug() << "Vertex Buffer: Access Error";
                accesscontrol[j] |= READ;
            }
#endif
            m_pe->m_d3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, item->m_offset + prev_stop, (stop - prev_stop) - 2);
            prev_stop = stop;
        }
        m_pe->m_effect->EndPass();
    }
}

void QD3DVertexBuffer::drawTextItem(QD3DBatchItem *item)
{
#ifdef QT_DEBUG_VERTEXBUFFER_ACCESS
    int vbstart = item->m_offset;
    for (int j=vbstart; j<(vbstart + ((item->m_count * 2) * 3)); ++j) {
        if (accesscontrol[j] != WRITE)
            qDebug() << "Vertex Buffer: Access Error";
        accesscontrol[j] |= READ;
    }
#endif
    m_pe->m_d3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, item->m_offset, item->m_count*2);
}

void QD3DVertexBuffer::drawAliasedLines(QD3DBatchItem *item)
{
    if (item->m_info & QD3DBatchItem::BI_TRANSFORM) {
        m_pe->m_statemanager->setTransformation(&item->m_matrix);
    } else {
        m_pe->m_statemanager->setTransformation(&m_pe->m_d3dxidentmatrix);
    }
    int pass = (item->m_info & QD3DBatchItem::BI_MASK) ?
        PASS_STENCIL_ODDEVEN : PASS_STENCIL_NOSTENCILCHECK_DIRECT;
    m_pe->m_effect->BeginPass(pass);

    if (item->m_info & QD3DBatchItem::BI_LINESTRIP) {
        int prev_stop = 0;
        for (int i=0; i<item->m_pointstops.count(); ++i) {
            int stop = item->m_pointstops.at(i);

#ifdef QT_DEBUG_VERTEXBUFFER_ACCESS
            int vbstart = (item->m_offset + prev_stop);
            for (int j=vbstart; j<(vbstart+(stop - prev_stop)); ++j) {
                if (accesscontrol[j] != WRITE)
                    qDebug() << "Vertex Buffer: Access Error";
                accesscontrol[j] |= READ;
            }
#endif
            m_pe->m_d3dDevice->DrawPrimitive(D3DPT_LINESTRIP, item->m_offset + prev_stop, (stop - prev_stop) - 1);
            prev_stop = stop;
        }
    } else {
        m_pe->m_d3dDevice->DrawPrimitive(D3DPT_LINELIST, item->m_offset, item->m_count);
    }
    m_pe->m_effect->EndPass();
}

void QD3DVertexBuffer::drawAntialiasedBoundingRect(QD3DBatchItem *item)
{
    if (item->m_info & QD3DBatchItem::BI_SCISSOR) {
        RECT rect;
        rect.left = qMax(qRound(item->m_brect.left()), 0);
        rect.top = qMax(qRound(item->m_brect.top()), 0);
        rect.bottom = qMin(m_height, qRound(item->m_brect.bottom()));
        rect.right = qMin(m_width, qRound(item->m_brect.right()));
        m_pe->m_d3dDevice->SetScissorRect(&rect);
        m_pe->m_statemanager->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
    }

#ifdef QT_DEBUG_VERTEXBUFFER_ACCESS
    int vbstart = item->m_offset + (item->m_count * 3);
    for (int j=vbstart; j<(vbstart + 4); ++j) {
        if (accesscontrol[j] != WRITE)
            qDebug() << "Vertex Buffer: Access Error";
        accesscontrol[j] |= READ;
    }
#endif

    m_pe->m_d3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, item->m_offset + (item->m_count * 3), 2);

    if (item->m_info & QD3DBatchItem::BI_SCISSOR) {
        m_pe->m_statemanager->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
    }
}

void QD3DVertexBuffer::drawAliasedBoundingRect(QD3DBatchItem *item)
{
#ifdef QT_DEBUG_VERTEXBUFFER_ACCESS
    int vbstart = (item->m_offset + item->m_count + 2);
    for (int j=vbstart; j<(vbstart + 4); ++j) {
        if (accesscontrol[j] != WRITE)
            qDebug() << "Vertex Buffer: Access Error";
        accesscontrol[j] |= READ;
    }
#endif

    m_pe->m_d3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, item->m_offset + item->m_count + 2, 2);
}

void QD3DVertexBuffer::addTrap(const Trapezoid &trap)
{
    qreal topLeftY = Q27Dot5ToDouble(trap.topLeft->y) - m_yoffset;
    qreal topLeftX = Q27Dot5ToDouble(trap.topLeft->x) - m_xoffset;
    qreal topRightY = Q27Dot5ToDouble(trap.topRight->y) - m_yoffset;
    qreal topRightX = Q27Dot5ToDouble(trap.topRight->x) - m_xoffset;
    qreal top = Q27Dot5ToDouble(trap.top) - m_yoffset;
    qreal bottom = Q27Dot5ToDouble(trap.bottom) - m_yoffset;

    Q27Dot5 _h = trap.topLeft->y - trap.bottomLeft->y;
    Q27Dot5 _w = trap.topLeft->x - trap.bottomLeft->x;
    qreal _leftA = (qreal)_w/_h;
    qreal _leftB = topLeftX - _leftA * topLeftY;

    _h = trap.topRight->y - trap.bottomRight->y;
    _w = trap.topRight->x - trap.bottomRight->x;
    qreal _rightA = (qreal)_w/_h;
    qreal _rightB = topRightX - _rightA * topRightY;

    qreal invLeftA = qFuzzyCompare(_leftA, 0.0) ? 0.0 : 1.0 / _leftA;
    qreal invRightA = qFuzzyCompare(_rightA, 0.0) ? 0.0 : 1.0 / _rightA;

    vertex v1 = {D3DXVECTOR3(1.f, top - 1.f, 0.5f), 0.f,
        top, bottom, invLeftA, -invRightA,
        _leftA, _leftB, _rightA, _rightB};
    vertex v2 = {D3DXVECTOR3(0.f, top - 1.f, 0.5f), 0.f,
        top, bottom, invLeftA, -invRightA,
        _leftA, _leftB, _rightA, _rightB};
    vertex v3 = {D3DXVECTOR3(0.f, bottom + 1.f, 0.5f), 0.f,
        top, bottom, invLeftA, -invRightA,
        _leftA, _leftB, _rightA, _rightB};

    vertex v4 = {D3DXVECTOR3(1.f, top - 1.f, 0.5f), 0.f,
        top, bottom, invLeftA, -invRightA,
        _leftA, _leftB, _rightA, _rightB};
    vertex v5 = {D3DXVECTOR3(0.f, bottom + 1.f, 0.5f), 0.f,
        top, bottom, invLeftA, -invRightA,
        _leftA, _leftB, _rightA, _rightB};
    vertex v6 = {D3DXVECTOR3(1.f, bottom + 1.f, 0.5f), 0.f,
        top, bottom, invLeftA, -invRightA,
        _leftA, _leftB, _rightA, _rightB};

#ifdef QT_DEBUG_VERTEXBUFFER_ACCESS
    for (int i=m_index; i<(m_index + 6); ++i) {
        if ((m_index + 6) > QT_VERTEX_BUF_SIZE)
            qDebug() << "Vertex Buffer: Buffer overflow";
        if (accesscontrol[i] != CLEAR)
            qDebug() << "Vertex Buffer: Access Error";
        accesscontrol[i] |= WRITE;
    }
#endif

    m_vbuff[m_index++] = v1;
    m_vbuff[m_index++] = v2;
    m_vbuff[m_index++] = v3;
    m_vbuff[m_index++] = v4;
    m_vbuff[m_index++] = v5;
    m_vbuff[m_index++] = v6;

    // check if buffer is full
    if (m_index >= (QT_VERTEX_BUF_SIZE - 16)) {
        m_item->m_offset = m_startindex;
        m_item->m_count = ( m_index - m_startindex ) / 3;
        m_startindex = m_index;

        QD3DBatchItem itemcopy = *m_item;
        m_item = m_pe->nextBatchItem();
        *m_item = itemcopy;
        m_item->m_info &= ~QD3DBatchItem::BI_MASKFULL;

        lock();
    }
}

void QD3DVertexBuffer::tessellate(const QPolygonF &poly) {
    int xoffset = m_item->m_maskpos.x;
    int yoffset = m_item->m_maskpos.y;

    int x = m_boundingRect.left();
    int y = m_boundingRect.top();
    m_xoffset = (x - xoffset) + 0.5f;
    m_yoffset = (y - yoffset) + 0.5f;

    QTessellator::tessellate(poly.data(), poly.count());

    m_item->m_offset = m_startindex;
    m_item->m_count = ( m_index - m_startindex ) / 3;
    m_startindex = m_index;
}

inline void QD3DVertexBuffer::lineToStencil(qreal x, qreal y)
{
    tess_lastpoint = QPointF(x, y);

#ifdef QT_DEBUG_VERTEXBUFFER_ACCESS
    if (m_index > QT_VERTEX_BUF_SIZE)
        qDebug() << "Vertex Buffer: Buffer overflow";
    if (accesscontrol[m_index] != CLEAR)
        qDebug() << "Vertex Buffer: Access Error";
    accesscontrol[m_index] |= WRITE;
#endif

    vertex v = {D3DXVECTOR3(x , y, 0.5f), m_color,
        0.f, 0.f, 0.f, 0.f,
        0.f, 0.f, 0.f, 0.f};

    m_vbuff[m_index++] = v;
    ++tess_index;

    // check if buffer is full
    if (m_index >= (QT_VERTEX_BUF_SIZE - 16)) {
        int firstindex = m_startindex;
        if (!m_item->m_pointstops.isEmpty())
            firstindex = m_item->m_pointstops.last();

        vertex first = m_vbuff[firstindex];

        // finish current polygon
        m_item->m_pointstops.append(tess_index);
        m_item->m_offset = m_startindex;
        m_startindex = m_index;

        // copy item
        QD3DBatchItem itemcopy = *m_item;
        m_item = m_pe->nextBatchItem();
        *m_item = itemcopy;

        // start new polygon
        lock();
        tess_index = 2;
        m_item->m_pointstops.clear();

#ifdef QT_DEBUG_VERTEXBUFFER_ACCESS
        if (accesscontrol[m_index] != CLEAR)
            qDebug() << "Vertex Buffer: Access Error";
        accesscontrol[m_index] |= WRITE;
#endif

        m_vbuff[m_index++] = first;

#ifdef QT_DEBUG_VERTEXBUFFER_ACCESS
        if (accesscontrol[m_index] != CLEAR)
            qDebug() << "Vertex Buffer: Access Error";
        accesscontrol[m_index] |= WRITE;
#endif

        m_vbuff[m_index++] = v;
    }

    if (x > max_x)
        max_x = x;
    else if (x < min_x)
        min_x = x;
    if (y > max_y)
        max_y = y;
    else if (y < min_y)
        min_y = y;
}

inline void QD3DVertexBuffer::curveToStencil(const QPointF &cp1, const QPointF &cp2, const QPointF &ep)
{
    qreal inverseScale = 0.5f;
    qreal inverseScaleHalf = inverseScale / 2;

    QBezier beziers[32];
    beziers[0] = QBezier::fromPoints(tess_lastpoint, cp1, cp2, ep);
    QBezier *b = beziers;
    while (b >= beziers) {
        // check if we can pop the top bezier curve from the stack
        qreal l = qAbs(b->x4 - b->x1) + qAbs(b->y4 - b->y1);
        qreal d;
        if (l > inverseScale) {
            d = qAbs( (b->x4 - b->x1)*(b->y1 - b->y2) - (b->y4 - b->y1)*(b->x1 - b->x2) )
                + qAbs( (b->x4 - b->x1)*(b->y1 - b->y3) - (b->y4 - b->y1)*(b->x1 - b->x3) );
            d /= l;
        } else {
            d = qAbs(b->x1 - b->x2) + qAbs(b->y1 - b->y2) +
                qAbs(b->x1 - b->x3) + qAbs(b->y1 - b->y3);
        }
        if (d < inverseScaleHalf || b == beziers + 31) {
            // good enough, we pop it off and add the endpoint
            lineToStencil(b->x4, b->y4);
            --b;
        } else {
            // split, second half of the polygon goes lower into the stack
            b->split(b+1, b);
           ++b;
        }
    }
}

QRectF QD3DVertexBuffer::pathToVertexArrays(const QPainterPath &path)
{
    bool line = (m_item->m_info & QD3DBatchItem::BI_FASTLINE);
    const QPainterPath::Element &first = path.elementAt(0);
    firstx = first.x;
    firsty = first.y;
    min_x = max_x = firstx;
    min_y = max_y = firsty;

    tess_index = 0;
    m_item->m_pointstops.clear();
    lineToStencil(firstx, firsty);

    for (int i=1; i<path.elementCount(); ++i) {
        const QPainterPath::Element &e = path.elementAt(i);
        switch (e.type) {
        case QPainterPath::MoveToElement:
            m_item->m_pointstops.append(tess_index);
            lineToStencil(e.x, e.y);
            break;
        case QPainterPath::LineToElement:
            lineToStencil(e.x, e.y);
            break;
        case QPainterPath::CurveToElement:
            curveToStencil(e, path.elementAt(i+1), path.elementAt(i+2));
            i+=2;
            break;
        default:
            break;
        }
    }

    if (!line)
        lineToStencil(firstx, firsty);

    m_item->m_pointstops.append(tess_index);

    m_item->m_offset = m_startindex;
    m_item->m_count = ( m_index - m_startindex ) - 2;
    m_startindex = m_index;

    QRectF result;
    result.setLeft(min_x);
    result.setRight(max_x);
    result.setTop(min_y);
    result.setBottom(max_y);

    if (line)
        result.adjust(0,0,1,1);

    return result;
}

static inline QPainterPath strokeForPath(const QPainterPath &path, const QPen &cpen) {
    QPainterPathStroker stroker;
    if (cpen.style() == Qt::CustomDashLine)
        stroker.setDashPattern(cpen.dashPattern());
    else
        stroker.setDashPattern(cpen.style());

    stroker.setCapStyle(cpen.capStyle());
    stroker.setJoinStyle(cpen.joinStyle());
    stroker.setMiterLimit(cpen.miterLimit());
    stroker.setWidth(cpen.widthF());

    QPainterPath stroke = stroker.createStroke(path);
    stroke.setFillRule(Qt::WindingFill);
    return stroke;
}

QDirect3DPaintEnginePrivate::~QDirect3DPaintEnginePrivate()
{

}

void QDirect3DPaintEnginePrivate::updateClipPath(const QPainterPath &path, Qt::ClipOperation op)
{
    //#### remove me
    QRegion r(path.toFillPolygon().toPolygon(), path.fillRule());
    updateClipRegion(r, op);

/*    if (m_vBuffer->needsFlushing())
        flushBatch();

    if (op == Qt::IntersectClip && !has_clipping)
        op = Qt::ReplaceClip;

    // switch to paths
    if (!has_complex_clipping) {
        m_clipPath = QPainterPath();
        m_clipPath.addRegion(m_clipRegion);
        m_clipRegion = QRegion();
        m_sysClipPath = QPainterPath();
        m_sysClipPath.addRegion(m_sysClipRegion);
        m_sysClipRegion = QRegion();
        has_complex_clipping = true;
    }

    QPainterPath cpath = m_matrix.map(path);

    QD3DBatchItem *item = &m_batch.items[m_batch.m_itemIndex++];
    item->m_info = QD3DBatchItem::BI_COMPLEXCLIP;

    switch (op) {
        case Qt::UniteClip:
            has_clipping = true;
            m_clipPath = m_clipPath.united(cpath);
            break;
        case Qt::ReplaceClip:
            has_clipping = true;
            m_clipPath = cpath;
            break;
        case Qt::NoClip:
            has_complex_clipping = false;
            has_clipping = false;
            item->m_info |= QD3DBatchItem::BI_CLEARCLIP;
            break;
        default: // intersect clip
            has_clipping = true;
            m_clipPath = m_clipPath.intersected(cpath);
            break;
    }

    if (!m_sysClipPath.isEmpty()) {
        item->m_info &= ~QD3DBatchItem::BI_CLEARCLIP;
        if (has_clipping)
            m_clipPath = m_clipPath.intersected(m_sysClipPath);
        else
            m_clipPath = m_sysClipPath;
    }

    // update the aliased clipping mask
    m_vBuffer->setClipPath(m_clipPath, item);

    // update the antialiased clipping mask
    if (m_vBuffer->needsFlushing())
        flushBatch();

    QD3DBatchItem *aaitem = &m_batch.items[m_batch.m_itemIndex++];
    aaitem->m_info = item->m_info|QD3DBatchItem::BI_AA;
    m_vBuffer->setClipPath(m_clipPath, aaitem); */
}

void QDirect3DPaintEnginePrivate::updateClipRegion(const QRegion &clipregion, Qt::ClipOperation op)
{
    if (m_vBuffer->needsFlushing())
        flushBatch();
    if (has_complex_clipping) {
        QPainterPath path;
        path.addRegion(clipregion);
        updateClipPath(path, op);
        return;
    }

    if (op == Qt::IntersectClip && m_clipRegion.isEmpty())
        op = Qt::ReplaceClip;

    QRegion cregion = m_matrix.map(clipregion);

    QD3DBatchItem *item = nextBatchItem();
    item->m_info &= ~QD3DBatchItem::BI_AA;

    switch (op) {
        case Qt::UniteClip:
            m_clipRegion = m_clipRegion.united(cregion);
            break;
        case Qt::ReplaceClip:
            m_clipRegion = cregion;
            break;
        case Qt::NoClip:
            m_clipRegion = QRegion();
            item->m_info |= QD3DBatchItem::BI_CLEARCLIP;
            break;
        default: // intersect clip
            m_clipRegion = m_clipRegion.intersected(cregion);
            break;
    }

    QRegion crgn = m_clipRegion;
    if (!m_sysClipRegion.isEmpty()) {
        item->m_info &= ~QD3DBatchItem::BI_CLEARCLIP;
        if (!crgn.isEmpty())
            crgn = crgn.intersected(m_sysClipRegion);
        else
            crgn = m_sysClipRegion;
    }

    QPainterPath path;
    path.addRegion(crgn);
    m_vBuffer->setClipPath(path, &item);
}

void QDirect3DPaintEnginePrivate::updateFont(const QFont &)
{
}

void QDirect3DPaintEnginePrivate::setRenderTechnique(RenderTechnique technique)
{
    if (m_currentTechnique != technique) {
        if (m_currentTechnique != RT_NoTechnique)
            m_effect->End();

        if (technique == RT_Aliased) {
            m_effect->SetTechnique("Aliased");
            m_effect->Begin(0,D3DXFX_DONOTSAVESTATE);
        } else if (technique == RT_Antialiased) {
            m_effect->SetTechnique("Antialiased");
            m_effect->Begin(0,D3DXFX_DONOTSAVESTATE);
        }
    }

    m_currentTechnique = technique;
}

/*QPolygonF QDirect3DPaintEnginePrivate::transformedRect(const QRectF &brect) const
{
    QPolygonF poly(brect);
    return m_matrix.map(poly);
}

QPolygonF QDirect3DPaintEnginePrivate::calcTextureCoords(const QPolygonF &trect) const
{
    QPolygonF result(4);
    QRectF brect = trect.boundingRect();
    qreal angle = atan(trect.at(0).x() -
}

QPolygonF QDirect3DPaintEnginePrivate::offsetTextureCoords(const QRectF &brect, const QPolygonF &trect) const
{

}*/

QD3DBatchItem *QDirect3DPaintEnginePrivate::nextBatchItem()
{
    if (m_vBuffer->needsFlushing())
        flushBatch();

    QD3DBatchItem *item = &m_batch.items[m_batch.m_itemIndex++];
    item->m_info = m_currentState;
    item->m_cmode = m_cmode;
    return item;
}

qreal calculateAngle(qreal dx, qreal dy)
{
    qreal angle;

    if (qFuzzyCompare(dx, 0.0)) {
        angle = (dy < 0) ? -M_PI/2 : M_PI/2;
    } else {
        angle = atanf(dy/dx);
        if (dx < 0)
            angle += M_PI;
    }

    return angle;
}

QPolygonF QDirect3DPaintEnginePrivate::brushCoordinates(const QRectF &r, bool stroke, qreal *fd) const
{
    QBrush brush;
    QTransform matrix;
    Qt::BrushStyle style;

    if (stroke) {
        brush = m_pen.brush();
        matrix = m_inv_pen_matrix;
        style = m_pen_brush_style;
    } else {
        brush = m_brush;
        matrix = m_inv_brush_matrix;
        style = m_brush_style;
    }

    QPolygonF bpoly;
    switch(style) {
        case Qt::TexturePattern: {
            QTransform totxcoords;
            QRectF adj_brect = r.adjusted(-0.5f, -0.5f, -0.5f, -0.5f);
            totxcoords.scale(1.0f/brush.texture().width(),
                1.0f/brush.texture().height());
            bpoly = matrix.map(QPolygonF(adj_brect));
            bpoly = totxcoords.map(bpoly);
            break; }
        case Qt::LinearGradientPattern: {
            const QLinearGradient *g = static_cast<const QLinearGradient *>(brush.gradient());
            QPointF start = g->start();
            QPointF stop = g->finalStop();
            qreal dx = stop.x() - start.x();
            qreal dy = stop.y() - start.y();
            qreal length = sqrt(dx * dx + dy * dy);
            qreal angle = calculateAngle(dx, dy);
            QTransform totxcoords;
            QRectF adj_brect = r.adjusted(-0.5f, -0.5f, -0.5f, -0.5f);
            totxcoords.scale(1.0f/length, 1.0f/length);
            totxcoords.rotateRadians(-angle);
            totxcoords.translate(-start.x(), -start.y());
            bpoly = matrix.map(QPolygonF(adj_brect));
            bpoly = totxcoords.map(bpoly);
            break; }
        case Qt::ConicalGradientPattern: {
            const QConicalGradient *g = static_cast<const QConicalGradient *>(brush.gradient());
            QPointF center = g->center();
            qreal angle = g->angle();
            QTransform totxcoords;
            totxcoords.rotate(angle);
            totxcoords.translate(-center.x(), -center.y());
            bpoly = matrix.map(QPolygonF(r));
            bpoly = totxcoords.map(bpoly);
            break; }
        case Qt::RadialGradientPattern: {
            const QRadialGradient *g = static_cast<const QRadialGradient *>(brush.gradient());
            QPointF center = g->center();
            QPointF focalpoint = g->focalPoint();
            qreal dx = focalpoint.x() - center.x();
            qreal dy = focalpoint.y() - center.y();
            qreal radius = g->radius();
            *fd = sqrt(dx * dx + dy * dy) / radius;
            qreal angle = calculateAngle(dx, dy);
            QTransform totxcoords;
            totxcoords.scale(1.0f/radius, 1.0f/radius);
            totxcoords.rotateRadians(-angle);
            totxcoords.translate(-center.x(), -center.y());
            bpoly = matrix.map(QPolygonF(r));
            bpoly = totxcoords.map(bpoly);
            break; }
        default:
            bpoly = QPolygonF(4);
    };

    return bpoly;
}

void QDirect3DPaintEnginePrivate::strokeAliasedPath(QPainterPath path, const QRectF &brect, const QTransform &txform)
{
    D3DCOLOR solid_color;
    QD3DBatchItem *item = nextBatchItem();

    if (!txform.isIdentity())
        path = txform.map(path);

    QRectF trect;
    QPolygonF txcoord;

    bool has_complex_brush = false;
    if (m_pen_brush_style != Qt::SolidPattern) {
        has_complex_brush = true;
        item->m_brush = m_pen.brush();
        item->m_info |= QD3DBatchItem::BI_COMPLEXBRUSH;
        item->m_opacity = m_opacity;
        solid_color = m_opacityColor;
    } else {
        solid_color = m_penColor;
    }
    if (has_fast_pen) {
        item->m_info |= QD3DBatchItem::BI_FASTLINE|QD3DBatchItem::BI_LINESTRIP;
        if (m_pen_brush_style == Qt::SolidPattern) {
            m_vBuffer->queueAliasedMask(path, &item, solid_color);
            item->m_info &= ~QD3DBatchItem::BI_MASK; // bypass stencil buffer
            return;
        }
    }

    QRectF txrect = m_vBuffer->queueAliasedMask(path, &item, 0);

    if (has_complex_brush) {
        trect = brect;
        txcoord = brushCoordinates(brect, true, &item->m_distance);
        item->m_info |= QD3DBatchItem::BI_TRANSFORM;
        item->m_matrix = m_d3dxmatrix;
    } else {
        trect = txrect;
        txcoord = QPolygonF(4);
    }

    m_vBuffer->queueRect(trect, item, solid_color, txcoord);
}

void QDirect3DPaintEnginePrivate::fillAliasedPath(QPainterPath path, const QRectF &brect, const QTransform &txform)
{
    D3DCOLOR solid_color;
    QD3DBatchItem *item = nextBatchItem();

    if (!txform.isIdentity())
        path = txform.map(path);

    QRectF trect;
    QPolygonF txcoord;

    bool has_complex_brush = false;
    if (m_brush_style != Qt::SolidPattern) {
        has_complex_brush = true;
        item->m_brush = m_brush;
        item->m_info |= QD3DBatchItem::BI_COMPLEXBRUSH;
        item->m_opacity = m_opacity;
        solid_color = m_opacityColor;
    } else {
        solid_color = m_brushColor;
    }

    QRectF txrect = m_vBuffer->queueAliasedMask(path, &item, 0);

    if (has_complex_brush) {
        trect = brect;
        txcoord = brushCoordinates(brect, false, &item->m_distance);
        item->m_info |= QD3DBatchItem::BI_TRANSFORM;
        item->m_matrix = m_d3dxmatrix;
    } else {
        trect = txrect;
        txcoord = QPolygonF(4);
    }

    m_vBuffer->queueRect(trect, item, solid_color, txcoord);
}

void QDirect3DPaintEnginePrivate::fillAntialiasedPath(const QPainterPath &path, const QRectF &brect,
                                                      const QTransform &txform, bool stroke)
{
    D3DCOLOR solid_color;
    bool winding = (path.fillRule() == Qt::WindingFill);
    QPolygonF poly;
    QRectF txrect;
    QPainterPath tpath;

    if (has_fast_pen && stroke) {
        tpath = txform.map(path);
        txrect = tpath.controlPointRect();
        txrect.adjust(-(m_pen_width/2),-(m_pen_width/2), m_pen_width, m_pen_width); 
    } else {
        poly = path.toFillPolygon(txform);
        txrect = poly.boundingRect();
    }

    // brect = approx. bounding rect before transformation
    // txrect = exact bounding rect after transformation
    // trect = the rectangle to be drawn
    // txcoord = the texture coordinates
    // adj_txrect = adjusted rect to include aliased outline

    bool use_scissor = false;
    if (txrect.left() < 0) {
        txrect.adjust(-txrect.left(),0,0,0);
        use_scissor = true;
    }
    if (txrect.top() < 0) {
        txrect.adjust(0,-txrect.top(),0,0);
        use_scissor = true;
    }

    if (!txrect.isValid())
        return;

    QD3DBatchItem *item = nextBatchItem();

    QRectF adj_txrect = txrect.adjusted(-1,-1,1,1);
    QRectF trect;
    QPolygonF txcoord;

    bool has_complex_brush = false;
    if (stroke) {
        if (m_pen_brush_style != Qt::SolidPattern) {
            has_complex_brush = true;
            item->m_brush = m_pen.brush();
            solid_color = m_opacityColor;
        } else {
            solid_color = m_penColor;
        }
        item->m_width = m_pen_width;
    } else {
        if (m_brush_style != Qt::SolidPattern) {
            has_complex_brush = true;
            item->m_brush = m_brush;
            solid_color = m_opacityColor;
        } else {
            solid_color = m_brushColor;
        }
    }

    qreal focaldist = 0;
    if (has_complex_brush) {
        trect = brect;
        txcoord = brushCoordinates(brect, stroke, &focaldist);
    } else {
        trect = adj_txrect;
        txcoord = QPolygonF(4);
    }

    bool maskfull;
    item->m_maskpos = m_vBuffer->allocateMaskPosition(txrect, &maskfull);
    if (maskfull)
        item->m_info |= QD3DBatchItem::BI_MASKFULL;
    item->m_distance = focaldist;

    if (winding)
        item->m_info |= QD3DBatchItem::BI_WINDING;

    if (has_complex_brush) {
        item->m_info |= QD3DBatchItem::BI_SCISSOR|QD3DBatchItem::BI_COMPLEXBRUSH|
            QD3DBatchItem::BI_TRANSFORM;
        item->m_brect = adj_txrect;
        item->m_matrix = m_d3dxmatrix;
        item->m_opacity = m_opacity;
    }
    if (use_scissor) {
        item->m_info |= QD3DBatchItem::BI_MASKSCISSOR;
        item->m_brect = adj_txrect;
    }

    if (has_fast_pen && stroke) {
        m_vBuffer->queueAntialiasedLines(tpath, &item, txrect);
    } else {
        m_vBuffer->queueAntialiasedMask(poly, &item, txrect);
    }

    m_vBuffer->queueRect(trect, item, solid_color, txcoord);
}

QPainterPath QDirect3DPaintEnginePrivate::strokePathFastPen(const QPainterPath &path)
{
    QPainterPath result;
    QBezier beziers[32];
    for (int i=0; i<path.elementCount(); ++i) {
        const QPainterPath::Element &e = path.elementAt(i);
        switch (e.type) {
        case QPainterPath::MoveToElement:
            result.moveTo(e.x, e.y);
            break;
        case QPainterPath::LineToElement:
            result.lineTo(e.x, e.y);
            break;

        case QPainterPath::CurveToElement:
        {
            QPointF sp = path.elementAt(i-1);
            QPointF cp2 = path.elementAt(i+1);
            QPointF ep = path.elementAt(i+2);
            i+=2;

            qreal inverseScaleHalf = m_invScale / 2;
            beziers[0] = QBezier::fromPoints(sp, e, cp2, ep);
            QBezier *b = beziers;
            while (b >= beziers) {
                // check if we can pop the top bezier curve from the stack
                qreal l = qAbs(b->x4 - b->x1) + qAbs(b->y4 - b->y1);
                qreal d;
                if (l > m_invScale) {
                    d = qAbs( (b->x4 - b->x1)*(b->y1 - b->y2)
                              - (b->y4 - b->y1)*(b->x1 - b->x2) )
                        + qAbs( (b->x4 - b->x1)*(b->y1 - b->y3)
                                - (b->y4 - b->y1)*(b->x1 - b->x3) );
                    d /= l;
                } else {
                    d = qAbs(b->x1 - b->x2) + qAbs(b->y1 - b->y2) +
                        qAbs(b->x1 - b->x3) + qAbs(b->y1 - b->y3);
                }
                if (d < inverseScaleHalf || b == beziers + 31) {
                    // good enough, we pop it off and add the endpoint
                    result.lineTo(b->x4, b->y4);
                    --b;
                } else {
                    // split, second half of the polygon goes lower into the stack
                    b->split(b+1, b);
                    ++b;
                }
            }
        } // case CurveToElement
        default:
            break;
        } // end of switch
    }
    return result;
}

void QDirect3DPaintEnginePrivate::strokePath(const QPainterPath &path, QRectF brect, bool simple)
{
    QTransform txform;
    QPainterPath tpath;

    if (has_fast_pen) {
        if (!simple)
            tpath = strokePathFastPen(path);
        else
            tpath = path;  //already only lines
    } else {
        tpath = strokeForPath(path, m_pen);
    }

    if (tpath.isEmpty())
        return;

    //brect is null if the path is not transformed
    if (brect.isNull())
        txform = m_matrix;

    if (!brect.isNull()) {
        // brect is set when the path is transformed already,
        // this is the case when we have a cosmetic pen.
        brect.adjust(-(m_pen_width/2),-(m_pen_width/2), m_pen_width, m_pen_width);
    }

    if (brect.isNull())
        brect = tpath.controlPointRect();
    brect.adjust(-m_invScale,-m_invScale,m_invScale,m_invScale); //adjust for antialiasing

    if (m_currentState & QD3DBatchItem::BI_AA) {
        fillAntialiasedPath(tpath, brect, txform, true);
    } else {
        strokeAliasedPath(tpath, brect, txform);
    }
}

void QDirect3DPaintEnginePrivate::fillPath(const QPainterPath &path, QRectF brect)
{
    QTransform txform;

    //brect is null if the path is not transformed
    if (brect.isNull())
        txform = m_matrix;

    if (brect.isNull())
        brect = path.controlPointRect();
    brect.adjust(-m_invScale,-m_invScale,m_invScale,m_invScale); //adjust for antialiasing

    if (m_currentState & QD3DBatchItem::BI_AA) {
        fillAntialiasedPath(path, brect, txform, false);
    } else {
        fillAliasedPath(path, brect, txform);
    }
}


bool QDirect3DPaintEnginePrivate::init()
{
#ifdef QT_DEBUG_D3D_CALLS
    qDebug() << "QDirect3DPaintEnginePrivate::init()";
#endif

    m_vBuffer = 0;
    m_gradCache = 0;
    m_dc = 0;
    m_dcsurface = 0;

    m_supports_d3d = false;
    m_currentState = 0;
    m_inScene = false;
    has_fast_pen = false;
    has_pen = false;
    has_brush = false;
    m_penColor = 0;
    m_brushColor = 0;
    m_defaultSurface = 0;
    m_batch.m_itemIndex = 0;
    m_currentTechnique = RT_NoTechnique;

    if (!pDirect3DCreate9) {
        QLibrary d3d_lib(QLatin1String("d3d9.dll"));
        pDirect3DCreate9 = (PFNDIRECT3DCREATE9) d3d_lib.resolve("Direct3DCreate9");
        if (!pDirect3DCreate9) {
            qWarning("QDirect3DPaintEngine: failed to resolve symbols from d3d9.dll.\n"
                     "Make sure you have the DirectX run-time installed.");
            return false;
        }
    }

    if (!pD3DXCreateBuffer || !pD3DXCreateEffect || !pD3DXMatrixOrthoOffCenterLH) {
        QLibrary d3dx_lib(QLatin1String("d3dx9_32.dll"));
        pD3DXCreateBuffer = (PFND3DXCREATEBUFFER) d3dx_lib.resolve("D3DXCreateBuffer");
        pD3DXCreateEffect = (PFND3DXCREATEEFFECT) d3dx_lib.resolve("D3DXCreateEffect");
        pD3DXMatrixOrthoOffCenterLH = (PFND3DXMATRIXORTHOOFFCENTERLH)
                                        d3dx_lib.resolve("D3DXMatrixOrthoOffCenterLH");
        if (!(pD3DXCreateBuffer && pD3DXCreateEffect && pD3DXMatrixOrthoOffCenterLH)) {
            qWarning("QDirect3DPaintEngine: failed to resolve symbols from d3dx9_32.dll.\n"
                     "Make sure you have the DirectX run-time installed.");
            return false;
        }
    }

    if (!m_d3dObject) {
        m_d3dObject = pDirect3DCreate9(D3D_SDK_VERSION);
        if (!m_d3dObject) {
            qWarning("QDirect3DPaintEngine: failed to create Direct3D object.\n"
                     "Direct3D support in Qt will be disabled.");
            return false;
        }
    }

    m_supports_d3d = testCaps();
    if (!m_supports_d3d)
        return false;

    D3DXMatrixIdentity(&m_d3dxidentmatrix);
    m_winManager.init(m_d3dObject);
    m_d3dDevice = m_winManager.device();

    if (!m_d3dDevice)
        return false;

    /* load shaders */
    //QFile file("C:\\depot\\qt\\main\\src\\gui\\painting\\qpaintengine_d3d.fx");
    QFile file(QLatin1String(":/qpaintengine_d3d.fx"));
    QByteArray fxFile;
    if (file.open(QFile::ReadOnly))
        fxFile = file.readAll();

    if (fxFile.size() > 0) {
        LPD3DXBUFFER compout;
        pD3DXCreateBuffer(4096, &compout);
        DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE|D3DXFX_DONOTSAVESTATE;
        if(FAILED(pD3DXCreateEffect(m_d3dDevice, fxFile.constData(), fxFile.size(),
                                      NULL, NULL, dwShaderFlags, NULL, &m_effect, &compout))) {
            qWarning("QDirect3DPaintEngine: failed to compile effect file");
            if (compout)
                qWarning((char *)compout->GetBufferPointer());
            // ### add a fallback for cards that do not support ps 3.0 and vs 3.0 - disable for now
            m_supports_d3d = false;
            return false;
        }
        if (m_effect) {
            m_statemanager = new QD3DStateManager(m_d3dDevice, m_effect);
            m_effect->SetStateManager(m_statemanager);
            m_vBuffer = new QD3DVertexBuffer(this);
            initDevice();
            m_gradCache = new QD3DGradientCache(m_d3dDevice);
        }
    }
    return true;
}

bool QDirect3DPaintEnginePrivate::testCaps()
{
    D3DCAPS9 caps;
    if (FAILED(m_d3dObject->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps)))
        return false;

    if ((caps.PresentationIntervals & D3DPRESENT_INTERVAL_IMMEDIATE)
        && (caps.DevCaps & D3DDEVCAPS_PUREDEVICE)
        && (caps.RasterCaps & D3DPRASTERCAPS_SCISSORTEST)
        && (caps.StencilCaps & D3DSTENCILCAPS_TWOSIDED))
        return true;
#if 0
    qDebug() << "Direct3D caps:";
    qDebug() << "D3DPRESENT_INTERVAL_IMMEDIATE:" << ((caps.PresentationIntervals & D3DPRESENT_INTERVAL_IMMEDIATE) != 0);
    qDebug() << "D3DDEVCAPS_PUREDEVICE:" << ((caps.DevCaps & D3DDEVCAPS_PUREDEVICE) != 0);
    qDebug() << "D3DPRASTERCAPS_SCISSORTEST:" << ((caps.RasterCaps & D3DPRASTERCAPS_SCISSORTEST) != 0);
    qDebug() << "D3DSTENCILCAPS_TWOSIDED:" << ((caps.StencilCaps & D3DSTENCILCAPS_TWOSIDED) != 0);
#endif
    return false;
}

void QDirect3DPaintEnginePrivate::initDevice()
{
    m_statemanager->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    m_statemanager->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
    m_statemanager->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    m_statemanager->SetRenderState(D3DRS_LIGHTING, FALSE);
    m_statemanager->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    m_statemanager->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    m_statemanager->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTALPHA);
}

void QDirect3DPaintEnginePrivate::updatePen(const QPen &pen)
{
#ifdef QT_DEBUG_D3D_CALLS
    qDebug() << "QDirect3DPaintEngine::updatePen";
#endif
    m_pen = pen;
    has_cosmetic_pen = false;
    has_pen = (m_pen.style() != Qt::NoPen);
    if (has_pen) {
        m_pen_brush_style = m_pen.brush().style();
        int a, r, g, b;
        m_pen.color().getRgb(&r, &g, &b, &a);
        m_penColor = D3DCOLOR_ARGB((int)(a * m_opacity),r,g,b);
        has_cosmetic_pen = m_pen.isCosmetic();

        if (m_pen_brush_style != Qt::NoBrush &&
            m_pen_brush_style != Qt::SolidPattern) {
            bool ok;
            m_inv_pen_matrix = m_pen.brush().transform().inverted(&ok);
            if (!ok)
                qWarning() << "QDirect3DPaintEngine: No inverse matix for pen brush matrix.";
        }

        m_pen_width = m_pen.widthF();
        if (m_pen_width == 0.0f)
            m_pen_width = 1.0f;
    }
}

void QDirect3DPaintEnginePrivate::updateBrush(const QBrush &brush)
{
#ifdef QT_DEBUG_D3D_CALLS
    qDebug() << "QDirect3DPaintEngine::updateBrush";
#endif
    m_brush = brush;
    m_brush_style = m_brush.style();
    has_brush = (m_brush_style != Qt::NoBrush);
    if (has_brush) {
        int a, r, g, b;
        m_brush.color().getRgb(&r, &g, &b, &a);
        m_brushColor = D3DCOLOR_ARGB((int)(a * m_opacity),r,g,b);

        if (m_brush_style != Qt::SolidPattern) {
            bool ok;
            m_inv_brush_matrix = (m_brush.transform() * m_brushOrigin).inverted(&ok);
            if (!ok)
                qWarning() << "QDirect3DPaintEngine: No inverse matix for brush matrix.";

            // make sure the texture is loaded as a texture (###)
            if (m_brush_style == Qt::TexturePattern && !m_brush.texture().data->texture)
                m_brush.setTexture(QPixmap::fromImage(m_brush.texture().toImage()));
        }
    }
}

void QDirect3DPaintEnginePrivate::updateTransform(const QTransform &matrix)
{
#ifdef QT_DEBUG_D3D_CALLS
    qDebug() << "QDirect3DPaintEngine::updateTransform";
#endif
    m_d3dxmatrix = D3DXMATRIX(matrix.m11(), matrix.m12(), 0, matrix.m13(),
        matrix.m21(), matrix.m22(), 0, matrix.m23(),
        0, 0, 1, 0,
        matrix.dx(), matrix.dy(), 0, 1);

    m_matrix = matrix;
    m_invScale = qMax(1 / qMax( qMax(qAbs(m_matrix.m11()), qAbs(m_matrix.m22())),
        qMax(qAbs(m_matrix.m12()), qAbs(m_matrix.m21())) ), 0.0001);
    m_txop = matrix.type();
}

int QDirect3DPaintEnginePrivate::flushAntialiased(int offset)
{
    // fills the mask (returns number of items added to the mask)
    int newoffset = m_vBuffer->drawAntialiasedMask(offset, m_batch.m_itemIndex);

    // set the render target to the current output surface
    if (FAILED(m_d3dDevice->SetRenderTarget(0, m_defaultSurface)))
        qWarning() << "QDirect3DPaintEngine: SetRenderTarget failed!";

    // draw the bounding boxes (using the mask generated by drawAntialiasedMask)
    for (int i=offset; i<newoffset; ++i) {
        QD3DBatchItem *item = &(m_batch.items[i]);
        int pass = (item->m_info & QD3DBatchItem::BI_COMPLEXBRUSH) ? PASS_AA_DRAW : PASS_AA_DRAW_DIRECT;
        m_statemanager->beginPass(pass);
        prepareItem(item);
        if (item->m_info & QD3DBatchItem::BI_BRECT)
            m_vBuffer->drawAntialiasedBoundingRect(item);
        cleanupItem(item);
    }

    m_statemanager->endPass();

    return newoffset;
}

bool QDirect3DPaintEnginePrivate::prepareBatch(QD3DBatchItem *item, int offset)
{
    if (item->m_info & QD3DBatchItem::BI_CLIP) {
        setRenderTechnique(RT_Aliased);
        if (item->m_info & QD3DBatchItem::BI_CLEARCLIP) {
            m_d3dDevice->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 0.0f, 0);
            return true;
        }

        m_vBuffer->drawAliasedMask(offset);
        m_d3dDevice->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
        if (item->m_info & QD3DBatchItem::BI_BRECT) {
            m_effect->BeginPass(PASS_STENCIL_CLIP);
            m_vBuffer->drawAliasedBoundingRect(item);
            m_effect->EndPass();
        }

        return true;
    }

    if (item->m_info & QD3DBatchItem::BI_AA) {
        setRenderTechnique(RT_Antialiased);
    } else {
        setRenderTechnique(RT_Aliased);
    }

    return false;
}

void QDirect3DPaintEnginePrivate::prepareItem(QD3DBatchItem *item) {
    // pixmap
    int brushmode = 0;
    m_statemanager->startStateBlock();
    if ((item->m_info & QD3DBatchItem::BI_PIXMAP) || (item->m_info & QD3DBatchItem::BI_IMAGE)) {
        IDirect3DTexture9 *tex = (item->m_info & QD3DBatchItem::BI_PIXMAP) ?
                                 item->m_pixmap.data->texture : item->m_texture;
        m_statemanager->setTexture(tex);
        brushmode = 1;
    }

    if (item->m_info & QD3DBatchItem::BI_AA) {
        m_statemanager->setMaskChannel(item->m_maskpos.channel);
        m_statemanager->setMaskOffset(item->m_xoffset, item->m_yoffset);
    }

    if (item->m_info & QD3DBatchItem::BI_COMPLEXBRUSH) {
        const QBrush brush = item->m_brush;
        switch (brush.style()) {
            case Qt::TexturePattern:
                m_statemanager->setTexture(brush.texture().data->texture, QGradient::RepeatSpread);
                brushmode = 1;
                break;
            case Qt::LinearGradientPattern:
                m_statemanager->setTexture(m_gradCache->
                    getBuffer(brush.gradient()->stops(), item->m_opacity),
                    brush.gradient()->spread());
                brushmode = 2;
                break;
            case Qt::ConicalGradientPattern:
                m_statemanager->setTexture(m_gradCache->
                    getBuffer(brush.gradient()->stops(), item->m_opacity),
                    brush.gradient()->spread());
                brushmode = 3;
                break;
            case Qt::RadialGradientPattern:
                m_statemanager->setTexture(m_gradCache->
                    getBuffer(brush.gradient()->stops(), item->m_opacity),
                    brush.gradient()->spread());
                m_statemanager->setFocalDistance(item->m_distance);
                brushmode = 4;
                break;
        };
    }

    if (item->m_info & QD3DBatchItem::BI_TRANSFORM) {
        m_statemanager->setTransformation(&item->m_matrix);
    } else {
        m_statemanager->setTransformation(&m_d3dxidentmatrix);
    }

    m_statemanager->setBrushMode(brushmode);
    setCompositionMode(item->m_cmode);
    m_statemanager->endStateBlock();
}


void QDirect3DPaintEnginePrivate::releaseDC()
{
    if (m_dc) {
        m_dcsurface->ReleaseDC(m_dc);
        m_dcsurface = 0;
        m_dc = 0;
    }
}


void QDirect3DPaintEnginePrivate::cleanupItem(QD3DBatchItem *item)
{
    if (item->m_info & QD3DBatchItem::BI_PIXMAP)
        item->m_pixmap = QPixmap();
    item->m_brush = QBrush();
}

bool QDirect3DPaintEnginePrivate::isFastRect(const QRectF &rect)
{
    if (m_matrix.type() < QTransform::TxRotate) {
        QRectF r = m_matrix.mapRect(rect);
        return r.topLeft().toPoint() == r.topLeft()
            && r.bottomRight().toPoint() == r.bottomRight();
    }

    return false;
}

void QDirect3DPaintEnginePrivate::setCompositionMode(QPainter::CompositionMode mode)
{
    switch(mode) {
        case QPainter::CompositionMode_SourceOver:
        default:
            m_statemanager->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
            m_statemanager->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    };
}

void QDirect3DPaintEnginePrivate::cleanup()
{
    // clean batch
    for(int i=0; i<QD3D_BATCH_SIZE; ++i) {
        m_batch.items[i].m_brush = QBrush();
        m_batch.items[i].m_pixmap = QPixmap();
    }

    m_winManager.cleanup();

    delete m_gradCache;
    delete m_vBuffer;

    if (m_effect)
        m_effect->Release();

    if (m_d3dObject)
        m_d3dObject->Release();

    m_effect = 0;
    m_d3dObject = 0;
    m_gradCache = 0;
    m_vBuffer = 0;
}

void QDirect3DPaintEnginePrivate::flushAliased(QD3DBatchItem *item, int offset)
{
    m_vBuffer->drawAliasedMask(offset);

    if (item->m_info & QD3DBatchItem::BI_BRECT) {
        int pass = PASS_STENCIL_NOSTENCILCHECK;
        if (item->m_info & QD3DBatchItem::BI_MASK)
            pass = (item->m_info & QD3DBatchItem::BI_COMPLEXBRUSH) ? PASS_STENCIL_DRAW : PASS_STENCIL_DRAW_DIRECT;
        m_effect->BeginPass(pass);
        prepareItem(item);
        m_vBuffer->drawAliasedBoundingRect(item);
        cleanupItem(item);
        m_effect->EndPass();
    }
}

void QDirect3DPaintEnginePrivate::flushText(QD3DBatchItem *item, int)
{
    prepareItem(item);
    m_statemanager->setTexture(item->m_texture);
    m_statemanager->setBrushMode(1);
//     m_statemanager->SetRenderState(D3DRS_BLENDFACTOR, item->m_brush.color().rgba());
    m_effect->BeginPass(cleartype_text ? PASS_CLEARTYPE_TEXT : PASS_TEXT);
    m_vBuffer->drawTextItem(item);
    m_effect->EndPass();
    cleanupItem(item);
}

void QDirect3DPaintEnginePrivate::flushLines(QD3DBatchItem *item, int)
{
    m_vBuffer->drawAliasedLines(item);

    if (item->m_info & QD3DBatchItem::BI_BRECT) {
        int pass = (item->m_info & QD3DBatchItem::BI_COMPLEXBRUSH) ? PASS_STENCIL_DRAW : PASS_STENCIL_DRAW_DIRECT;
        m_effect->BeginPass(pass);
        prepareItem(item);
        m_vBuffer->drawAliasedBoundingRect(item);
        cleanupItem(item);
        m_effect->EndPass();
    }
}

void QDirect3DPaintEnginePrivate::flushBatch()
{
//     static int dbgcounter = 0;
//     ++dbgcounter;
//     qDebug() << " -> flush" << dbgcounter;

    int offset = 0;
    m_vBuffer->unlock();
    releaseDC();

    // iterate over all items in the batch
    while (offset != m_batch.m_itemIndex) {
        QD3DBatchItem *item = &(m_batch.items[offset]);

        if (prepareBatch(item, offset)) {
            ++offset;
            continue;
        }

        if (item->m_info & QD3DBatchItem::BI_FASTLINE) {
            flushLines(item, offset++);
        } else if (item->m_info & QD3DBatchItem::BI_AA) {
            offset = flushAntialiased(offset);
        } else if (item->m_info & QD3DBatchItem::BI_TEXT) {
            flushText(item, offset++);
        } else {
            flushAliased(item, offset++);
        }
    }

    // reset batch
    m_batch.m_itemIndex = 0;

    // release doomed textures
    for (int i=0; i<qd3d_release_list.size(); ++i)
        qd3d_release_list.at(i)->Release();
    qd3d_release_list.clear();
}

QDirect3DPaintEngine::QDirect3DPaintEngine()
    : QPaintEngine(*(new QDirect3DPaintEnginePrivate),
                   PaintEngineFeatures(AllFeatures))
{ }

QDirect3DPaintEngine::~QDirect3DPaintEngine()
{
}

bool QDirect3DPaintEngine::begin(QPaintDevice *device)
{
#ifdef QT_DEBUG_D3D_CALLS
    qDebug() << "QDirect3DPaintEngine::begin";
#endif
    Q_D(QDirect3DPaintEngine);
    setActive(true);

    d->m_winSize = QRect(0, 0, device->width(), device->height()).size();

    d->m_currentState = 0;
    d->m_invScale = 1;
    d->m_opacity = 1.0f;
    d->m_opacityColor = D3DCOLOR_ARGB(255,255,255,255);
    d->m_matrix = QTransform();
    d->m_brushOrigin = QTransform();
    d->m_txop = QTransform::TxNone;
    d->m_d3dxmatrix = d->m_d3dxidentmatrix;
    d->m_cmode = QPainter::CompositionMode_SourceOver;

    Q_ASSERT(device && device->devType() == QInternal::Widget);
    if (d->m_d3dDevice == 0) {
        qWarning() << "QDirect3DPaintEngine: No Device!";
        return false;
    }

    d->cleartype_text = false;
//     QT_WA({
//         UINT result;
//         BOOL ok;
//         ok = SystemParametersInfoW(SPI_GETFONTSMOOTHINGTYPE, 0, &result, 0);
//         if (ok)
//             d->cleartype_text = (result == FE_FONTSMOOTHINGCLEARTYPE);
//     }, {
//         UINT result;
//         BOOL ok;
//         ok = SystemParametersInfoA(SPI_GETFONTSMOOTHINGTYPE, 0, &result, 0);
//         if (ok)
//             d->cleartype_text = (result == FE_FONTSMOOTHINGCLEARTYPE);
//     });

    d->m_winManager.setPaintDevice(device);
    int status = d->m_winManager.status();
    if (status & QD3DWindowManager::NeedsReseting) {
        d->m_effect->OnLostDevice();
        d->m_vBuffer->beforeReset();
        d->m_statemanager->reset();
        d->m_winManager.reset();
        d->m_vBuffer->afterReset();
        d->m_effect->OnResetDevice();
        d->initDevice();
    }

    status = d->m_winManager.status();
    if (status & QD3DWindowManager::MaxSizeChanged) {
        QSize maxsize = d->m_winManager.maxSize();
        d->m_vBuffer->setMaskSize(maxsize);
        int masksize[2] = {maxsize.width(), maxsize.height()};
        d->m_effect->SetIntArray("g_mMaskSize", masksize, 2);

    }

    LPDIRECT3DSURFACE9 newsurface = d->m_winManager.renderTarget();
    if (d->m_defaultSurface != newsurface) {
        d->m_defaultSurface = newsurface;
        if (FAILED(d->m_d3dDevice->SetRenderTarget(0, newsurface)))
            qWarning() << "QDirect3DPaintEngine: SetRenderTarget failed!";
    }


    D3DXMATRIX projMatrix;
    pD3DXMatrixOrthoOffCenterLH(&projMatrix, 0, d->m_winSize.width(), d->m_winSize.height(), 0, 0.0f, 1.0f);
    d->m_statemanager->setProjection(&projMatrix);

    if (!d->m_inScene) {
        if (FAILED(d->m_d3dDevice->BeginScene())) {
            qWarning() << "QDirect3DPaintEngine: BeginScene() failed.";
            return false;
        }
        QWidget *widget = static_cast<QWidget *>(device);
        if (widget->autoFillBackground() == true) {
            QColor color = widget->palette().brush(widget->backgroundRole()).color();
            RECT rect = {0, 0, widget->width(), widget->height()};
            d->m_d3dDevice->ColorFill(d->m_defaultSurface, &rect,
                D3DCOLOR_ARGB(color.alpha(), color.red(), color.green(), color.blue()));
        }
        d->m_inScene = true;
    }

    // set system clip
    d->clipping_enabled = false;
    d->has_complex_clipping = false;

    d->m_sysClipRegion = systemClip();
    QVector<QRect> rects = d->m_sysClipRegion.rects();
    if (rects.count() == 1 && rects.at(0).size() == d->m_winSize)
        d->m_sysClipRegion = QRegion();

    d->updateClipRegion(QRegion(), Qt::NoClip);

    return true;
}

void QDirect3DPaintEngine::drawEllipse(const QRectF &rect)
{
#ifdef QT_DEBUG_D3D_CALLS
    qDebug() << "QDirect3DPaintEngine::drawEllipse (float)";
#endif
    QPaintEngine::drawEllipse(rect);
}

void QDirect3DPaintEngine::drawEllipse(const QRect &rect)
{
#ifdef QT_DEBUG_D3D_CALLS
    qDebug() << "QDirect3DPaintEngine::drawEllipse";
#endif
    QPaintEngine::drawEllipse(rect);
}

void QDirect3DPaintEngine::drawImage(const QRectF &r, const QImage &image, const QRectF &sr,
                                     Qt::ImageConversionFlags)
{
#ifdef QT_DEBUG_D3D_CALLS
    qDebug() << "QDirect3DPaintEngine::drawImage";
#endif
     //drawPixmap(r, QPixmap::fromImage(image, flags), sr);
     //return;

    Q_D(QDirect3DPaintEngine);
    int width = image.width();
    int height = image.height();

    // transform rectangle
    QPolygonF txrect(QRectF((sr.left() + 0.5f) / width, (sr.top() + 0.5f) / height,
                            sr.width() / width, sr.height() / height));

    QD3DBatchItem *item = d->nextBatchItem();
    item->m_info = QD3DBatchItem::BI_IMAGE | QD3DBatchItem::BI_TRANSFORM;
    item->m_texture = qd3d_image_cache()->lookup(d->m_d3dDevice, image);
    item->m_matrix = d->m_d3dxmatrix;
    d->m_vBuffer->queueRect(r, item, d->m_opacityColor, txrect);

    //drawPixmap(rectangle, QPixmap::fromImage(image, flags), sr);
}

void QDirect3DPaintEngine::drawLines(const QLineF *lines, int lineCount)
{
#ifdef QT_DEBUG_D3D_CALLS
    qDebug() << "QDirect3DPaintEngine::drawLines (float)";
#endif
    Q_D(QDirect3DPaintEngine);

    if (!d->has_pen)
        return;

    if (d->has_fast_pen && !(d->m_currentState & QD3DBatchItem::BI_AA)
        && (d->m_pen_brush_style == Qt::SolidPattern)) {
        QD3DBatchItem *item = d->nextBatchItem();
        if (d->m_txop <= QTransform::TxTranslate) {
            item->m_info |= QD3DBatchItem::BI_TRANSFORM;
            item->m_matrix = d->m_d3dxmatrix;
            d->m_vBuffer->queueAliasedLines(lines, lineCount, &item, d->m_penColor, false);
        } else {
            d->m_vBuffer->queueAliasedLines(lines, lineCount, &item, d->m_penColor, true);
        }
    } else {
        QRectF brect;
        QPainterPath path;

        // creates a path with the lines
        path.moveTo(lines[0].x1(), lines[0].y1());
        qreal lastx = lines[0].x2();
        qreal lasty = lines[0].y2();
        path.lineTo(lastx, lasty);

        for (int i=1; i<lineCount; ++i) {
            qreal x = lines[i].x1();
            qreal y = lines[i].y1();
            if (lastx != x || lasty != y) {
                path.moveTo(x, y);
            }
            path.lineTo(lines[i].x2(), lines[i].y2());
        }

        if (d->has_cosmetic_pen) {
            brect = path.controlPointRect();
            path = d->m_matrix.map(path);
        }

        d->strokePath(path, brect, true);
    }
}

void QDirect3DPaintEngine::drawLines(const QLine *lines, int lineCount)
{
#ifdef QT_DEBUG_D3D_CALLS
    qDebug() << "QDirect3DPaintEngine::drawLines";
#endif
    QPaintEngine::drawLines(lines, lineCount);
}

void QDirect3DPaintEngine::drawPath(const QPainterPath &path)
{
#ifdef QT_DEBUG_D3D_CALLS
    qDebug() << "QDirect3DPaintEngine::drawPath";
#endif
    Q_D(QDirect3DPaintEngine);

    if (path.isEmpty())
        return;

    QRectF brect;
    QPainterPath tpath;

    if (d->has_cosmetic_pen) {
        brect = path.controlPointRect();
        tpath = d->m_matrix.map(path);
    } else {
        tpath = path;
    }

    if (d->has_brush)
        d->fillPath(tpath, brect);

    if (d->has_pen)
        d->strokePath(tpath, brect);
}


QPointF QDirect3DPaintEnginePrivate::transformPoint(const QPointF &p, qreal *w) const
{
    (*w) = 1.0f;
    qreal fx = p.x();
    qreal fy = p.y();
    qreal nx = m_matrix.m11()*fx + m_matrix.m21()*fy + m_matrix.m31();
    qreal ny = m_matrix.m12()*fx + m_matrix.m22()*fy + m_matrix.m32();
    if (!m_matrix.isAffine()) {
        *w = m_matrix.m13()*fx + m_matrix.m23()*fy + m_matrix.m33();
        //*w = 1/(*w);
        nx = nx/(*w);
        ny = ny/(*w);
    }
    return QPointF(nx, ny);
}

void QDirect3DPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr)
{
#ifdef QT_DEBUG_D3D_CALLS
    qDebug() << "QDirect3DPaintEngine::drawPixmap";
#endif
    Q_D(QDirect3DPaintEngine);

    if (d->m_vBuffer->needsFlushing())
        d->flushBatch();

    if (!pm.data->texture) {
        QImage im = pm.data->image.convertToFormat(QImage::Format_ARGB32);
        if (FAILED(d->m_d3dDevice->CreateTexture(im.width(), im.height(), 1, 0,
                                                 D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pm.data->texture, 0)))
        {
            qWarning("QDirect3DPaintEngine: unable to create Direct3D texture from pixmap.");
            return;
        }
        D3DLOCKED_RECT rect;
        if (FAILED(pm.data->texture->LockRect(0, &rect, 0, 0))) {
            qDebug() << "QDirect3DPaintEngine: unable to lock texture rect.";
            return;
        }
        DWORD *dst = (DWORD *) rect.pBits;
        DWORD *src = (DWORD *) im.scanLine(0);
        int dst_ppl = rect.Pitch/4;
        int src_ppl = im.bytesPerLine()/4;

        Q_ASSERT(dst_ppl == src_ppl);
        memcpy(dst, src, rect.Pitch*im.height());
        pm.data->texture->UnlockRect(0);
    }

    int width = pm.width();
    int height = pm.height();

    // transform rectangle
    QPolygonF txrect(QRectF((sr.left() + 0.5f) / width, (sr.top() + 0.5f) / height,
        sr.width() / width, sr.height() / height));

    QD3DBatchItem *item = d->nextBatchItem();
    item->m_info = QD3DBatchItem::BI_PIXMAP|QD3DBatchItem::BI_TRANSFORM;
    item->m_pixmap = pm;
    item->m_matrix = d->m_d3dxmatrix;
    d->m_vBuffer->queueRect(r, item, d->m_opacityColor, txrect);
}

void QDirect3DPaintEngine::drawPoints(const QPointF *points, int pointCount)
{
#ifdef QT_DEBUG_D3D_CALLS
    qDebug() << "QDirect3DPaintEngine::drawPoints (float)";
#endif
    QPaintEngine::drawPoints(points, pointCount);
}

void QDirect3DPaintEngine::drawPoints(const QPoint *points, int pointCount)
{
#ifdef QT_DEBUG_D3D_CALLS
    qDebug() << "QDirect3DPaintEngine::drawPoints";
#endif
    QPaintEngine::drawPoints(points, pointCount);
}

void QDirect3DPaintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
#ifdef QT_DEBUG_D3D_CALLS
    qDebug() << "QDirect3DPaintEngine::drawPolygon";
#endif
    Q_D(QDirect3DPaintEngine);

    if (d->has_brush && mode != PolylineMode) {
        QPainterPath path;
        path.setFillRule(mode == WindingMode ? Qt::WindingFill : Qt::OddEvenFill);
        path.moveTo(points[0]);
        for (int i=1; i<pointCount; ++i)
            path.lineTo(points[i]);
        if (path.isEmpty())
            return;
        d->fillPath(path, QRectF());
    }

    if (d->has_pen) {
        QPainterPath path(points[0]);
        for (int i = 1; i < pointCount; ++i)
            path.lineTo(points[i]);
        if (mode != PolylineMode)
            path.lineTo(points[0]);

        if (path.isEmpty())
            return;
        QRectF brect;
        if (d->has_cosmetic_pen) {
            brect = path.controlPointRect();
            path = d->m_matrix.map(path);
        }

        d->strokePath(path, brect);
    }
}

void QDirect3DPaintEngine::drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode)
{
#ifdef QT_DEBUG_D3D_CALLS
    qDebug() << "QDirect3DPaintEngine::drawPolygon";
#endif
    QPaintEngine::drawPolygon(points, pointCount, mode);
}

void QDirect3DPaintEngine::drawRects(const QRectF *rects, int rectCount)
{
    Q_D(QDirect3DPaintEngine);
#ifdef QT_DEBUG_D3D_CALLS
    qDebug() << "QDirect3DPaintEngine::drawRects (float)";
#endif
    for (int i=0; i<rectCount; ++i) {
        if ((d->m_brush_style == Qt::SolidPattern) &&
            (!(d->m_currentState & QD3DBatchItem::BI_AA) || d->isFastRect(rects[i]))) {
            QD3DBatchItem *item = d->nextBatchItem();
            item->m_info |= QD3DBatchItem::BI_TRANSFORM;
            item->m_info &= ~QD3DBatchItem::BI_AA;
            item->m_matrix = d->m_d3dxmatrix;
            d->m_vBuffer->queueRect(rects[i], item, d->m_brushColor, QPolygonF(4));
            if (d->has_pen) {
                QPainterPath path;
                QRectF brect;

                path.addRect(rects[i]);
                if (d->has_cosmetic_pen) {
                    brect = path.controlPointRect();
                    path = d->m_matrix.map(path);
                }

                d->strokePath(path, brect, true);
            }
        } else {
            QPainterPath path;
            QRectF brect;

            path.addRect(rects[i]);
            if (d->has_cosmetic_pen) {
                brect = path.controlPointRect();
                path = d->m_matrix.map(path);
            }

            if (d->has_brush)
                d->fillPath(path, brect);

            if (d->has_pen)
                d->strokePath(path, brect, true);
        }
    }
}

void QDirect3DPaintEngine::drawRects(const QRect *rects, int rectCount)
{
#ifdef QT_DEBUG_D3D_CALLS
    qDebug() << "QDirect3DPaintEngine::drawRects";
#endif
    QPaintEngine::drawRects(rects, rectCount);
}


void QDirect3DPaintEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
    Q_D(QDirect3DPaintEngine);

#ifdef QT_DEBUG_D3D_CALLS
    qDebug() << "QDirect3DPaintEngine::drawTextItem";
#endif
//     if (d->m_matrix.isScaling() || (d->m_pen_brush_style >= Qt::LinearGradientPattern
//                                     && d->m_pen_brush_style <= Qt::ConicalGradientPattern)) {
//         QPaintEngine::drawTextItem(p, textItem);
//         return;
//     }

    const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);
    QVarLengthArray<QFixedPoint> positions;
    QVarLengthArray<glyph_t> glyphs;
    QTransform matrix;
    matrix.translate(p.x(), p.y());
    ti.fontEngine->getGlyphPositions(ti.glyphs, ti.num_glyphs, matrix, ti.flags, glyphs, positions);

    qd3d_glyph_cache()->cacheGlyphs(this, ti, glyphs, d->cleartype_text);
    QD3DFontTexture *font_tex = qd3d_glyph_cache()->fontTexture(ti.fontEngine);

    // ### check buffer size limit
    QD3DBatchItem *item = d->nextBatchItem();
    d->m_vBuffer->lock();

    item->m_info = QD3DBatchItem::BI_TEXT
        | (d->m_currentState & ~QD3DBatchItem::BI_AA) | QD3DBatchItem::BI_TRANSFORM;
    item->m_texture = font_tex->texture;
    item->m_offset = d->m_vBuffer->index();
    item->m_matrix = d->m_d3dxmatrix;
    item->m_count = 0;
    item->m_brush = d->m_pen.brush();

    for (int i=0; i< glyphs.size(); ++i) {
        QD3DGlyphCoord *g = qd3d_glyph_cache()->lookup(ti.fontEngine, glyphs[i]);

        // we don't cache glyphs with no width/height
        if (!g)
            continue;

        // texture coords
        qreal tex_coords[] = { g->x, g->y, g->x + g->width, g->y + g->height };
        QPointF logical_pos(qRound((positions[i].x - g->x_offset).toReal()) - 0.5f,
                            qRound((positions[i].y + g->y_offset).toReal()) - 0.5f);

        QRectF glyph_rect(logical_pos, QSizeF(g->log_width, g->log_height));
        d->m_vBuffer->queueTextGlyph(glyph_rect, tex_coords, item, d->m_penColor);
    }
}

void QDirect3DPaintEngine::drawTiledPixmap(const QRectF &rect, const QPixmap &pixmap, const QPointF &p)
{
#ifdef QT_DEBUG_D3D_CALLS
    qDebug() << "QDirect3DPaintEngine::drawTiledPixmap";
#endif
    QPaintEngine::drawTiledPixmap(rect, pixmap, p);
}

bool QDirect3DPaintEngine::end()
{
    Q_D(QDirect3DPaintEngine);

    d->flushBatch();

    if (d->m_flushOnEnd) {
        QPaintDevice *pdev = paintDevice();
        LPDIRECT3DSWAPCHAIN9 swapchain = swapChain(pdev);


        QWidget *w = 0;
        if (pdev->devType() == QInternal::Widget) {
            w = static_cast<QWidget *>(pdev);
        }

        if (w && swapchain) {
            QRect br = w->rect();
            QRect wbr = br;//.translated(-w->pos());

            RECT destrect;
            destrect.left   = wbr.x();
            destrect.top    = wbr.y();
            destrect.right  = destrect.left + wbr.width();
            destrect.bottom = destrect.top  + wbr.height();

            RECT srcrect;
            srcrect.left    = br.x();// + w->x();
            srcrect.top     = br.y();// + w->y();
            srcrect.right   = wbr.width() + srcrect.left;
            srcrect.bottom  = wbr.height() + srcrect.top;
            int devwidth = w->width();
            int devheight = w->height();

            if (devwidth <= srcrect.right) {
                int diff = srcrect.right - devwidth;
                srcrect.right -= diff;
                destrect.right -= diff;
                if (srcrect.right <= srcrect.left)
                    return false;
            }
            if (devheight <= srcrect.bottom) {
                int diff = srcrect.bottom - devheight;
                srcrect.bottom -= diff;
                destrect.bottom -= diff;
                if (srcrect.bottom <= srcrect.top)
                    return false;
            }

            if (FAILED(swapchain->Present(&srcrect, &destrect, w->winId(), 0, 0)))
                qWarning("QDirect3DPaintEngine: failed to present back buffer.");
        }
    }


    return true;
}

void QDirect3DPaintEngine::updateState(const QPaintEngineState &state)
{
#ifdef QT_DEBUG_D3D_CALLS
    qDebug() << "QDirect3DPaintEngine::updateState";
#endif
    Q_D(QDirect3DPaintEngine);

    bool update_fast_pen = false;
    DirtyFlags flags = state.state();

    if (flags & DirtyOpacity) {
        d->m_opacity = state.opacity();
        if (d->m_opacity > 1.0f)
            d->m_opacity = 1.0f;
        if (d->m_opacity < 0.f)
            d->m_opacity = 0.f;
        uint c = (d->m_opacity * 255);
        d->m_opacityColor = D3DCOLOR_ARGB(c,c,c,c);
        flags |= (DirtyPen | DirtyBrush);
    }

    if (flags & DirtyCompositionMode) {
        d->m_cmode = state.compositionMode();
    }

    if (flags & DirtyTransform) {
        d->updateTransform(state.transform());
        update_fast_pen = true;
    }

    if (flags & DirtyHints) {
        if (state.renderHints() & QPainter::Antialiasing)
            d->m_currentState |= QD3DBatchItem::BI_AA;
        else
            d->m_currentState &= ~QD3DBatchItem::BI_AA;
    }

    if (flags & DirtyFont) {
        d->updateFont(state.font());
    }

    if (state.state() & DirtyClipEnabled) {
        if (state.isClipEnabled() && !d->clipping_enabled) {
            d->clipping_enabled = true;
            if (d->has_complex_clipping)
                d->updateClipPath(painter()->clipPath(), Qt::ReplaceClip);
            else
                d->updateClipRegion(painter()->clipRegion(), Qt::ReplaceClip);
        } else if (!state.isClipEnabled() && d->clipping_enabled) {
            d->clipping_enabled = false;
            if (d->has_complex_clipping)
                d->updateClipPath(QPainterPath(), Qt::NoClip);
            else
                d->updateClipRegion(QRegion(), Qt::NoClip);
        }
    }

    if (flags & DirtyClipRegion) {
        d->updateClipRegion(state.clipRegion(), state.clipOperation());
    }

    if (flags & DirtyClipPath) {
        d->updateClipPath(state.clipPath(), state.clipOperation());
    }

    if (flags & DirtyBrushOrigin) {
        d->m_brushOrigin = QTransform();
        d->m_brushOrigin.translate(-state.brushOrigin().x(),
            -state.brushOrigin().y());
        flags |= DirtyBrush;
    }

    if (flags & DirtyPen) {
        d->updatePen(state.pen());
        update_fast_pen = true;
    }

    if (flags & DirtyBrush)
        d->updateBrush(state.brush());

    if (update_fast_pen && d->has_pen) {
        d->has_fast_pen = ((d->m_pen_width == 0 || (d->m_pen_width <= 1 && d->m_txop <= QTransform::TxTranslate))
             || d->has_cosmetic_pen) && d->m_pen.style() == Qt::SolidLine;
    }
}

void QDirect3DPaintEngine::cleanup()
{
    Q_D(QDirect3DPaintEngine);
    d->cleanup();
}

void QDirect3DPaintEngine::scroll(QPaintDevice *pd, const RECT &srcrect, const RECT &destrect)
{
    Q_D(QDirect3DPaintEngine);
    LPDIRECT3DSURFACE9 srcsurf = d->m_winManager.surface(pd);
    LPDIRECT3DSURFACE9 masksurf = d->m_vBuffer->freeMaskSurface();
    if (FAILED(d->m_d3dDevice->StretchRect(srcsurf, &srcrect, masksurf, &srcrect, D3DTEXF_NONE)))
        qWarning("QDirect3DPaintEngine: StretchRect failed.");
    if (FAILED(d->m_d3dDevice->StretchRect(masksurf, &srcrect, srcsurf, &destrect, D3DTEXF_NONE)))
        qWarning("QDirect3DPaintEngine: StretchRect failed.");
}

LPDIRECT3DSWAPCHAIN9 QDirect3DPaintEngine::swapChain(QPaintDevice *pd)
{
    Q_D(QDirect3DPaintEngine);

    if (d->m_inScene) {
        if (d->m_d3dDevice == 0) {
            qWarning("QDirect3DPaintEngine: No device!");
            return false;
        }

        d->setRenderTechnique(QDirect3DPaintEnginePrivate::RT_NoTechnique);
        if (FAILED(d->m_d3dDevice->EndScene()))
            qWarning("QDirect3DPaintEngine: failed to end scene.");

        d->m_inScene = false;
    }

    return d->m_winManager.swapChain(pd);
}

void QDirect3DPaintEngine::releaseSwapChain(QPaintDevice *pd)
{
    Q_D(QDirect3DPaintEngine);
    d->m_winManager.releasePaintDevice(pd);
}

HDC QDirect3DPaintEngine::getDC() const
{
    QDirect3DPaintEnginePrivate *d = const_cast<QDirect3DPaintEnginePrivate *>(d_func());

    if (!d->m_dc && d->m_defaultSurface) {
        d->m_dcsurface = d->m_defaultSurface;
        if (FAILED(d->m_defaultSurface->GetDC(&d->m_dc)))
            qWarning() << "QDirect3DPaintEngine::getDC() failed!";
    }

    return d->m_dc;
}

void QDirect3DPaintEngine::setFlushOnEnd(bool flushOnEnd)
{
    Q_D(QDirect3DPaintEngine);

    d->m_flushOnEnd = flushOnEnd;
}

bool QDirect3DPaintEngine::hasDirect3DSupport()
{
    Q_D(QDirect3DPaintEngine);
    return d->m_supports_d3d;
}

#include "qpaintengine_d3d.moc"
