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

#include <private/qtextengine_p.h>
#include <qdebug.h>
#include <private/qfontengine_p.h>
#include <private/qmath_p.h>
#include <private/qdrawhelper_p.h>
#include <private/qpaintengine_p.h>
#include "qapplication.h"
#include "qbrush.h"
#include "qgl.h"
#include <private/qgl_p.h>
#include "qmap.h"
#include <private/qpaintengine_opengl_p.h>
#include "qpen.h"
#include "qvarlengtharray.h"
#include <private/qpainter_p.h>
#include <qglpixelbuffer.h>
#include <private/qglpixelbuffer_p.h>
#include <private/qstroker_p.h>
#include <private/qbezier_p.h>

#ifdef Q_OS_MAC
# include <OpenGL/glu.h>
#else
# include <GL/glu.h>
#endif
#include <stdlib.h>

struct QSubpath
{
    int start, end;
    qreal left, right, top, bottom;
    bool processed;
    bool added;


    inline void unite(qreal x, qreal y) {
        if (x < left) left = x;
        else if (x > right) right = x;
        if (y > bottom) bottom = y;
        else if (y < top) top = y;
    }

    inline bool intersects(const QSubpath &s) const
    {
        return (qMax(left, s.left) <= qMin(right, s.right) &&
                qMax(top, s.top) <= qMin(bottom, s.bottom));
    }
};

QDebug operator<<(QDebug s, const QSubpath &p)
{
    s << "Subpath [" << p.start << "-" << p.end << "]"
      << "left:" << p.left
      << "right:" << p.right
      << "top:" << p.top
      << "bottom:" << p.bottom
      << p.processed << p.added;
    return s;
}

#define QREAL_MAX 9e100
#define QREAL_MIN -9e100

void qt_painterpath_split(const QPainterPath &path, QDataBuffer<int> *paths, QDataBuffer<QSubpath> *subpaths)
{
//     qDebug() << "\nqt_painterpath_split";

    // reset the old buffers...
    paths->reset();
    subpaths->reset();

    // Create the set of current subpaths...
    {
        QSubpath *current = 0;
        for (int i=0; i<path.elementCount(); ++i) {
            const QPainterPath::Element &e = path.elementAt(i);
            switch (e.type) {
            case QPainterPath::MoveToElement: {
                if (current)
                    current->end = i-1;
                QSubpath sp = { i, 0, QREAL_MAX, QREAL_MIN, QREAL_MAX, QREAL_MIN, false, false };
                sp.unite(e.x, e.y);
                subpaths->add(sp);
                current = &subpaths->data()[subpaths->size() - 1];
                break;
            }
            case QPainterPath::LineToElement:
                current->unite(e.x, e.y);
                break;
            case QPainterPath::CurveToElement: {
                const QPainterPath::Element &cp2 = path.elementAt(i+1);
                const QPainterPath::Element &ep = path.elementAt(i+2);
                current->unite(e.x, e.y);
                current->unite(cp2.x, cp2.y);
                current->unite(ep.x, ep.y);
                i+=2;
                break;
            }
            default:
                break;
            }
        }
        if (current)
            current->end = path.elementCount() - 1;
    }

//     for (int i=0; i<subpaths->size(); ++i) {
//         qDebug() << subpaths->at(i);
//     }

    // Check which intersect and merge in paths variable
    {
        for (int spi=0; spi<subpaths->size(); ++spi) {
            if (subpaths->at(spi).processed)
                continue;

            paths->add(spi);
            (subpaths->data() + spi)->added = true;

//             qDebug() << " - matching: " << spi << subpaths->at(spi) << paths->size();

            for (int i=paths->size() - 1; i<paths->size(); ++i) {
                QSubpath *s1 = subpaths->data() + paths->at(i);
                s1->processed = true;
//                 qDebug() << "   - checking" << paths->at(i) << *(subpaths->data() + paths->at(i));

                for (int j=spi+1; j<subpaths->size(); ++j) {
                    QSubpath *s2 = subpaths->data() + j;
                    if (s2->processed || s2->added)
                        continue;
//                     qDebug() << "      - against" << j << *s2;
                    if (s1->intersects(*s2)) {
//                         qDebug() << "        - intersects...";
                        s2->added = true;
                        paths->add(j);

                    }
                }
            }

            paths->add(-1);
//             qDebug() << "paths...";
//             for (int i=0; i<paths->size(); ++i)
//                 qDebug() << "  " << paths->at(i);
        }
    }
}

class QGLDrawable {
public:
    QGLDrawable() : widget(0), buffer(0) {}
    inline void setDevice(QPaintDevice *pdev);
    inline void setAutoBufferSwap(bool);
    inline bool autoBufferSwap() const;
    inline void swapBuffers();
    inline void makeCurrent();
    inline QSize size() const;
    inline QGLFormat format() const;
    inline GLuint bindTexture(const QImage &image, GLenum target = GL_TEXTURE_2D, GLint format = GL_RGBA8);
    inline GLuint bindTexture(const QPixmap &pixmap, GLenum target = GL_TEXTURE_2D, GLint format = GL_RGBA8);
    inline QColor backgroundColor() const;

private:
    QGLWidget *widget;
    QGLPixelBuffer *buffer;
};

void QGLDrawable::setDevice(QPaintDevice *pdev)
{
    if (pdev->devType() == QInternal::Widget)
        widget = static_cast<QGLWidget *>(pdev);
    else if (pdev->devType() == QInternal::Pbuffer)
        buffer = static_cast<QGLPixelBuffer *>(pdev);
}

inline void QGLDrawable::setAutoBufferSwap(bool enable)
{
    if (widget)
        widget->setAutoBufferSwap(enable);
}

inline bool QGLDrawable::autoBufferSwap() const
{
    return widget && widget->autoBufferSwap();
}

inline void QGLDrawable::swapBuffers()
{
    if (widget)
        widget->swapBuffers();
}

inline void QGLDrawable::makeCurrent()
{
    if (widget)
        widget->makeCurrent();
    else if (buffer)
        buffer->makeCurrent();
}

inline QSize QGLDrawable::size() const
{
    if (widget)
        return widget->size();
    else if (buffer)
        return buffer->size();
    return QSize();
}

inline QGLFormat QGLDrawable::format() const
{
    if (widget)
        return widget->format();
    else if (buffer)
        return buffer->format();
    return QGLFormat();
}

inline GLuint QGLDrawable::bindTexture(const QImage &image, GLenum target, GLint format)
{
    if (widget)
        return widget->d_func()->glcx->d_func()->bindTexture(image, target, format, true);
    else if (buffer)
        return buffer->d_func()->qctx->d_func()->bindTexture(image, target, format, true);
    return 0;
}

inline GLuint QGLDrawable::bindTexture(const QPixmap &pixmap, GLenum target, GLint format)
{
    if (widget)
        return widget->d_func()->glcx->d_func()->bindTexture(pixmap, target, format, true);
    else if (buffer)
        return buffer->d_func()->qctx->d_func()->bindTexture(pixmap, target, format, true);
    return 0;
}

inline QColor QGLDrawable::backgroundColor() const
{
    if (widget)
        return widget->palette().brush(QPalette::Background).color();
    return QColor();
}

#ifndef CALLBACK // for Windows
#define CALLBACK
#endif

// Need to allocate space for new vertices on intersecting lines and
// they need to be alive until gluTessEndPolygon() has returned
static QList<GLdouble *> vertexStorage;
static void CALLBACK qgl_tess_combine(GLdouble coords[3],
                                      GLdouble *[4],
                                      GLfloat [4], GLdouble **dataOut)
{
    GLdouble *vertex = (GLdouble *) malloc(3 * sizeof(GLdouble));
    vertex[0] = coords[0];
    vertex[1] = coords[1];
    vertex[2] = coords[2];
    *dataOut = vertex;
    vertexStorage.append(vertex);
}

static void CALLBACK qgl_tess_error(GLenum errorCode)
{
    qWarning("QOpenGLPaintEngine: tessellation error: %s", gluErrorString(errorCode));
}

struct QGLUTesselatorCleanupHandler
{
    inline QGLUTesselatorCleanupHandler() { qgl_tess = gluNewTess(); }
    inline ~QGLUTesselatorCleanupHandler() { gluDeleteTess(qgl_tess); }
    GLUtesselator *qgl_tess;
};

Q_GLOBAL_STATIC(QGLUTesselatorCleanupHandler, tessHandler)

class QOpenGLPaintEnginePrivate : public QPaintEnginePrivate {
    Q_DECLARE_PUBLIC(QOpenGLPaintEngine)
public:
    QOpenGLPaintEnginePrivate()
        : bgmode(Qt::TransparentMode)
        , has_fast_pen(false)
        , txop(QPainterPrivate::TxNone)
        , inverseScale(1)
        , moveToCount(0)
        , dashStroker(0)
        , stroker(0)
        , tessVector(20000)
        , shader_dev(0)
        , grad_palette(0)
        {}

    inline void setGLPen(const QColor &c) {
        pen_color[0] = c.red();
        pen_color[1] = c.green();
        pen_color[2] = c.blue();
        pen_color[3] = c.alpha();
    }

    inline void setGLBrush(const QColor &c) {
        brush_color[0] = c.red();
        brush_color[1] = c.green();
        brush_color[2] = c.blue();
        brush_color[3] = c.alpha();
    }

    inline void setGradientOps(Qt::BrushStyle style);
    void generateGradientColorTable(const QGradientStops& s,
                                    unsigned int *colorTable, int size);
    void createGradientPaletteTexture(const QGradient& g);

    void updateGradient(const QBrush &brush);
    void beginPath(QPaintEngine::PolygonDrawMode mode);
    void endPath();
    inline void moveTo(const QPointF &p);
    inline void lineTo(const QPointF &p);
    inline void curveTo(const QPointF &cp1, const QPointF &cp2, const QPointF &ep);

    QPen cpen;
    QBrush cbrush;
    QBrush bgbrush;
    Qt::BGMode bgmode;
    QRegion crgn;
    Qt::BrushStyle brush_style;
    Qt::BrushStyle pen_brush_style;

    uint has_clipping : 1;
    uint has_pen : 1;
    uint has_brush : 1;
    uint has_autoswap : 1;
    uint has_fast_pen : 1;

    QMatrix matrix;
    GLubyte pen_color[4];
    GLubyte brush_color[4];
    QPainterPrivate::TransformationCodes txop;
    QGLDrawable drawable;

    qreal inverseScale;

    int moveToCount;
    QStroker basicStroker;
    QDashStroker *dashStroker;
    QStrokerOps *stroker;
    QPointF path_start;

    QDataBuffer<QSubpath> subpath_buffer;
    QDataBuffer<int> int_buffer;
    QDataBuffer<GLdouble> tessVector;

    QPaintDevice *shader_dev;
    GLuint grad_palette;

    GLuint radial_frag_program;
    GLuint conical_frag_program;
};

void QOpenGLPaintEnginePrivate::beginPath(QPaintEngine::PolygonDrawMode mode)
{
    Q_ASSERT(mode != QPaintEngine::PolylineMode);

    tessVector.reset();
    moveToCount = 0;

    GLUtesselator *qgl_tess = tessHandler()->qgl_tess;
    gluTessProperty(qgl_tess, GLU_TESS_WINDING_RULE,
                    mode == QPaintEngine::WindingMode
                    ? GLU_TESS_WINDING_NONZERO
                    : GLU_TESS_WINDING_ODD);
    gluTessBeginPolygon(qgl_tess, NULL);
    gluTessBeginContour(qgl_tess);
}

void QOpenGLPaintEnginePrivate::endPath()
{
    // rewind to first position...
    lineTo(path_start);
//     printf("endPath() -> tessVector = %d, vertexStorage = %d\n", tessVector.size(), vertexStorage.size());

    GLUtesselator *qgl_tess = tessHandler()->qgl_tess;
    gluTessEndContour(qgl_tess);
    gluTessEndPolygon(qgl_tess);

    for (int i=0; i<vertexStorage.size(); ++i)
        free(vertexStorage.at(i));
    vertexStorage.clear();
}

inline void QOpenGLPaintEnginePrivate::moveTo(const QPointF &p)
{
    if (moveToCount == 0) {
        // store rewind position
        path_start = p;
    } else if (moveToCount >= 2) {
        // rewind to first position...
        lineTo(path_start);
    }

    ++moveToCount;
    lineTo(p);
}

inline void QOpenGLPaintEnginePrivate::lineTo(const QPointF &p)
{
    GLUtesselator *qgl_tess = tessHandler()->qgl_tess;
    tessVector.add(p.x());
    tessVector.add(p.y());
    tessVector.add(0);
    gluTessVertex(qgl_tess, tessVector.data() + tessVector.size() - 3, tessVector.data() + tessVector.size() - 3);
}

inline void QOpenGLPaintEnginePrivate::curveTo(const QPointF &cp1, const QPointF &cp2, const QPointF &ep)
{
    qreal x1 = tessVector.at(tessVector.size() - 3);
    qreal y1 = tessVector.at(tessVector.size() - 2);

    qreal inverseScaleHalf = inverseScale / 2;

    QBezier beziers[32];
    beziers[0] = QBezier::fromPoints(QPointF(x1, y1), cp1, cp2, ep);
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
            lineTo(QPointF(b->x4, b->y4));
            --b;
        } else {
            // split, second half of the polygon goes lower into the stack
            b->split(b+1, b);
            ++b;
        }
    }
}


static void strokeMoveTo(qfixed x, qfixed y, void *data)
{
    ((QOpenGLPaintEnginePrivate *) data)->moveTo(QPointF(qt_fixed_to_real(x),
                                                         qt_fixed_to_real(y)));
}


static void strokeLineTo(qfixed x, qfixed y, void *data)
{
    ((QOpenGLPaintEnginePrivate *) data)->lineTo(QPointF(qt_fixed_to_real(x),
                                                         qt_fixed_to_real(y)));
}


static void strokeCurveTo(qfixed c1x, qfixed c1y,
                   qfixed c2x, qfixed c2y,
                   qfixed ex, qfixed ey,
                   void *data)
{
    ((QOpenGLPaintEnginePrivate *) data)->curveTo(QPointF(qt_fixed_to_real(c1x),
                                                          qt_fixed_to_real(c1y)),
                                                  QPointF(qt_fixed_to_real(c2x),
                                                          qt_fixed_to_real(c2y)),
                                                  QPointF(qt_fixed_to_real(ex),
                                                          qt_fixed_to_real(ey)));
}

#ifndef APIENTRY
# define APIENTRY
#endif

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#ifndef GL_IBM_texture_mirrored_repeat
#define GL_MIRRORED_REPEAT_IBM            0x8370
#endif

#ifndef GL_SGIS_generate_mipmap
#define GL_GENERATE_MIPMAP_SGIS           0x8191
#define GL_GENERATE_MIPMAP_HINT_SGIS      0x8192
#endif

// ARB_fragment_program extension protos
#ifndef GL_FRAGMENT_PROGRAM_ARB
#define GL_FRAGMENT_PROGRAM_ARB           0x8804
#define GL_PROGRAM_FORMAT_ASCII_ARB       0x8875
#endif

typedef void (APIENTRY *_glProgramStringARB) (GLenum, GLenum, GLsizei, const GLvoid *);
typedef void (APIENTRY *_glBindProgramARB) (GLenum, GLuint);
typedef void (APIENTRY *_glDeleteProgramsARB) (GLsizei, const GLuint *);
typedef void (APIENTRY *_glGenProgramsARB) (GLsizei, GLuint *);
typedef void (APIENTRY *_glProgramLocalParameter4fvARB) (GLenum, GLuint, const GLfloat *);

static _glProgramStringARB qt_glProgramStringARB = 0;
static _glBindProgramARB qt_glBindProgramARB = 0;
static _glDeleteProgramsARB qt_glDeleteProgramsARB = 0;
static _glGenProgramsARB qt_glGenProgramsARB = 0;
static _glProgramLocalParameter4fvARB qt_glProgramLocalParameter4fvARB = 0;

#define glProgramStringARB qt_glProgramStringARB
#define glBindProgramARB qt_glBindProgramARB
#define glDeleteProgramsARB qt_glDeleteProgramsARB
#define glGenProgramsARB qt_glGenProgramsARB
#define glProgramLocalParameter4fvARB qt_glProgramLocalParameter4fvARB

/*  radial fragment program
    parameter: 0 = inv_matrix
               1 = inv_matrix_offset
               2 = fmp
               4 = radius
*/
static const char *const radial_program =
    "!!ARBfp1.0"
    "PARAM c[5] = { program.local[0..2],"
    "             { 2, 4 },"
    "             program.local[4] };"
    "TEMP R0;"
    "MUL R0.zw, c[0].xyxy, fragment.position.xyxy;"
    "ADD R0.z, R0, R0.w;"
    "MUL R0.xy, c[0].zwzw, fragment.position;"
    "ADD R0.w, R0.x, R0.y;"
    "ADD R0.xy, R0.zwzw, c[1];"
    "MUL R0.zw, R0.xyxy, R0.xyxy;"
    "MUL R0.xy, R0, c[2];"
    "ADD R0.x, R0, R0.y;"
    "ADD R0.z, R0, R0.w;"
    "MUL R0.z, c[4].x, -R0;"
    "MUL R0.y, R0.z, c[3];"
    "MUL R0.x, R0, c[3];"
    "MAD R0.y, R0.x, R0.x, -R0;"
    "MOV R0.z, c[3].x;"
    "RSQ R0.y, R0.y;"
    "MUL R0.z, c[4].x, R0;"
    "RCP R0.y, R0.y;"
    "RCP R0.z, R0.z;"
    "ADD R0.x, -R0, R0.y;"
    "MUL R0.x, R0, R0.z;"
    "TEX result.color, R0, texture[0], 1D;"
    "END"
    "\0";

/*  radial fragment program
    parameter: 0 = inv_matrix
               1 = inv_matrix_offset
               4 = angle
*/
static const char *const conical_program =
    "!!ARBfp1.0"
    "PARAM c[6] = { program.local[0..1],"
    "               { 0.0020000001, 9.9999997e-10, 0.1963, 0.9817 },"
    "               { 2.3561945, 0.78539819, -1, 1 },"
    "               program.local[4],"
    "               { 0.15915494 } };"
    "TEMP R0;"
    "TEMP R1;"
    "MUL R0.zw, c[0].xyxy, fragment.position.xyxy;"
    "ADD R0.z, R0, R0.w;"
    "MUL R0.xy, c[0].zwzw, fragment.position;"
    "ADD R0.w, R0.x, R0.y;"
    "ADD R0.xy, R0.zwzw, c[1];"
    "ABS R0.w, R0.x;"
    "ABS R0.z, R0.y;"
    "ADD R0.z, R0, -R0.w;"
    "ADD R0.w, R0.y, c[2].x;"
    "ABS R0.z, R0;"
    "CMP R0.y, -R0.z, R0, R0.w;"
    "ABS R0.z, -R0.y;"
    "ADD R0.z, R0, c[2].y;"
    "ADD R0.w, R0.x, R0.z;"
    "ADD R1.x, R0.z, -R0;"
    "RCP R1.x, R1.x;"
    "RCP R1.y, R0.w;"
    "MUL R0.w, R0, R1.x;"
    "ADD R0.z, R0.x, -R0;"
    "MUL R0.z, R0, R1.y;"
    "CMP R0.z, R0.x, R0.w, R0;"
    "MUL R0.w, R0.z, R0.z;"
    "MOV R1.x, c[3].y;"
    "CMP R0.y, -R0, c[3].z, c[3].w;"
    "MAD R0.w, R0, c[2].z, -c[2];"
    "CMP R0.x, R0, c[3], R1;"
    "MAD R0.x, R0.w, R0.z, R0;"
    "MAD R0.x, R0, R0.y, c[4];"
    "MUL R0.x, R0, c[5];"
    "FLR R0.y, R0.x;"
    "ADD R0.x, R0, -R0.y;"
    "TEX result.color, R0, texture[0], 1D;"
    "END"
    "\0";

static bool qt_resolve_frag_program_extensions()
{
    static int resolved = false;

    if (resolved && qt_glProgramStringARB)
        return true;
    else if (resolved)
        return false;

    QGLContext cx(QGLFormat::defaultFormat());

    // ARB_fragment_program
    qt_glProgramStringARB = (_glProgramStringARB) cx.getProcAddress("glProgramStringARB");
    qt_glBindProgramARB = (_glBindProgramARB) cx.getProcAddress("glBindProgramARB");
    qt_glDeleteProgramsARB = (_glDeleteProgramsARB) cx.getProcAddress("glDeleteProgramsARB");
    qt_glGenProgramsARB = (_glGenProgramsARB) cx.getProcAddress("glGenProgramsARB");
    qt_glProgramLocalParameter4fvARB = (_glProgramLocalParameter4fvARB) cx.getProcAddress("glProgramLocalParameter4fvARB");

    resolved = qt_glProgramLocalParameter4fvARB ? true : false;
    return resolved;
}

void QOpenGLPaintEnginePrivate::generateGradientColorTable(const QGradientStops& s, unsigned int *colorTable, int size)
{
    int pos = 0;
    qreal fpos = 0.0;
    qreal incr = 1.0 / qreal(size);
    QVector<unsigned int> colors(s.size());

    for (int i = 0; i < s.size(); ++i)
        colors[i] = s[i].second.rgba();

    while (fpos < s.first().first) {
        colorTable[pos] = colors[0];
        pos++;
        fpos += incr;
    }

    for (int i = 0; i < s.size() - 1; ++i) {
        qreal delta = 1/(s[i+1].first - s[i].first);
        while (fpos < s[i+1].first && pos < size) {
            int dist = int(255 * ((fpos - s[i].first) * delta));
            int idist = 255 - dist;
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
            colorTable[pos] = INTERPOLATE_PIXEL_256(colors[i], idist, colors[i+1], dist);
#else
	    uint c = INTERPOLATE_PIXEL_256(colors[i], idist, colors[i+1], dist);
	    colorTable[pos] = ((c << 24) & 0xff000000)
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

void QOpenGLPaintEnginePrivate::createGradientPaletteTexture(const QGradient& g)
{
    const int PAL_SIZE = 1024;
    unsigned int palbuf[PAL_SIZE];
    generateGradientColorTable(g.stops(), palbuf, PAL_SIZE);

    if (g.spread() == QGradient::RepeatSpread || g.type() == QGradient::ConicalGradient)
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    else if (g.spread() == QGradient::ReflectSpread)
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT_IBM);
    else
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    if (!(QGLExtensions::glExtensions & QGLExtensions::GenerateMipmap))
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    else if (g.type() == QGradient::ConicalGradient || g.spread() == QGradient::PadSpread) {
        // disable mipmaps for pad gradients and conical gradients
        glTexParameteri(GL_TEXTURE_1D, GL_GENERATE_MIPMAP_SGIS, GL_FALSE);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_1D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, PAL_SIZE, 0, GL_BGRA, GL_UNSIGNED_BYTE, palbuf);
}


inline void QOpenGLPaintEnginePrivate::setGradientOps(Qt::BrushStyle style)
{
    if (style == Qt::RadialGradientPattern || style == Qt::ConicalGradientPattern) {
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_1D);
        glEnable(GL_FRAGMENT_PROGRAM_ARB);
    } else if (style == Qt::LinearGradientPattern) {
        glDisable(GL_FRAGMENT_PROGRAM_ARB);
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_1D);
    } else {
        glDisable(GL_FRAGMENT_PROGRAM_ARB);
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_1D);
    }
}

QOpenGLPaintEngine::QOpenGLPaintEngine()
    : QPaintEngine(*(new QOpenGLPaintEnginePrivate),
                   PaintEngineFeatures(AllFeatures
                                       & ~(LinearGradientFill
                                           | RadialGradientFill
                                           | ConicalGradientFill
                                           | PatternBrush)))
{
    Q_D(QOpenGLPaintEngine);
    d->basicStroker.setMoveToHook(strokeMoveTo);
    d->basicStroker.setLineToHook(strokeLineTo);
    d->basicStroker.setCubicToHook(strokeCurveTo);

    GLUtesselator *qgl_tess = tessHandler()->qgl_tess;

#ifdef Q_WS_MAC  // This removes warnings.
    gluTessCallback(qgl_tess, GLU_TESS_BEGIN, reinterpret_cast<GLvoid (CALLBACK *)(...)>(&glBegin));
    gluTessCallback(qgl_tess, GLU_TESS_VERTEX,
                    reinterpret_cast<GLvoid (CALLBACK *)(...)>(&glVertex3dv));
    gluTessCallback(qgl_tess, GLU_TESS_END, reinterpret_cast<GLvoid (CALLBACK *)(...)>(&glEnd));
    gluTessCallback(qgl_tess, GLU_TESS_COMBINE,
                    reinterpret_cast<GLvoid (CALLBACK *)(...)>(&qgl_tess_combine));
    gluTessCallback(qgl_tess, GLU_TESS_ERROR,
                    reinterpret_cast<GLvoid (CALLBACK *)(...)>(&qgl_tess_error));
#else
    gluTessCallback(qgl_tess, GLU_TESS_BEGIN, reinterpret_cast<GLvoid (CALLBACK *)()>(&glBegin));
    gluTessCallback(qgl_tess, GLU_TESS_VERTEX,
                    reinterpret_cast<GLvoid (CALLBACK *)()>(&glVertex3dv));
    gluTessCallback(qgl_tess, GLU_TESS_END, reinterpret_cast<GLvoid (CALLBACK *)()>(&glEnd));
    gluTessCallback(qgl_tess, GLU_TESS_COMBINE,
                    reinterpret_cast<GLvoid (CALLBACK *)()>(&qgl_tess_combine));
    gluTessCallback(qgl_tess, GLU_TESS_ERROR,
                    reinterpret_cast<GLvoid (CALLBACK *) ()>(&qgl_tess_error));
#endif

    if (QGLExtensions::glExtensions & QGLExtensions::FragmentProgram)
        qt_resolve_frag_program_extensions();
}

QOpenGLPaintEngine::~QOpenGLPaintEngine()
{
    Q_D(QOpenGLPaintEngine);
    if (d->dashStroker)
        delete d->dashStroker;
}

bool QOpenGLPaintEngine::begin(QPaintDevice *pdev)
{
    Q_D(QOpenGLPaintEngine);
    d->drawable.setDevice(pdev);
    d->has_clipping = false;
    d->has_fast_pen = false;
    d->has_autoswap = d->drawable.autoBufferSwap();
    d->drawable.setAutoBufferSwap(false);
    d->inverseScale = 1;
    setActive(true);
    d->drawable.makeCurrent();

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_MULTISAMPLE);
    glDisable(GL_TEXTURE_1D);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_RECTANGLE_NV);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glShadeModel(GL_FLAT);

    const QColor &c = d->drawable.backgroundColor();
    glClearColor(c.redF(), c.greenF(), c.blueF(), 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);
    QSize sz(d->drawable.size());
    glViewport(0, 0, sz.width(), sz.height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, sz.width(), sz.height(), 0, -999999, 999999);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    if ((QGLExtensions::glExtensions & QGLExtensions::MirroredRepeat)
        && (pdev != d->shader_dev))
    {
        if (d->shader_dev) {
            glBindTexture(GL_TEXTURE_1D, 0);
            glDeleteTextures(1, &d->grad_palette);
            if (QGLExtensions::glExtensions & QGLExtensions::FragmentProgram) {
                glDeleteProgramsARB(1, &d->radial_frag_program);
                glDeleteProgramsARB(1, &d->conical_frag_program);
            }
        }
        d->shader_dev = pdev;
        gccaps |= LinearGradientFill;
        glGenTextures(1, &d->grad_palette);

        if (QGLExtensions::glExtensions & QGLExtensions::FragmentProgram) {
            glGenProgramsARB(1, &d->radial_frag_program);
            glGenProgramsARB(1, &d->conical_frag_program);

            glGetError(); // reset the error state
            glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, d->radial_frag_program);
            glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
                               strlen(radial_program), (const GLbyte *) radial_program);
            if (glGetError() == GL_NO_ERROR)
                gccaps |= RadialGradientFill;
            else
                qWarning() << "QOpenGLPaintEngine: Unable to use radial gradient fragment shader.";

            glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, d->conical_frag_program);
            glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
                               strlen(conical_program), (const GLbyte *) conical_program);
            if (glGetError() == GL_NO_ERROR)
                gccaps |= ConicalGradientFill;
            else
                qWarning() << "QOpenGLPaintEngine: Unable to use conical gradient fragment shader.";
        }
    }

    setDirty(QPaintEngine::DirtyPen);
    setDirty(QPaintEngine::DirtyBrush);
    return true;
}

bool QOpenGLPaintEngine::end()
{
    Q_D(QOpenGLPaintEngine);
    glPopAttrib();
    glFlush();
    d->drawable.swapBuffers();
    d->drawable.setAutoBufferSwap(d->has_autoswap);
    setActive(false);
    return true;
}

void QOpenGLPaintEngine::updateState(const QPaintEngineState &state)
{
    QPaintEngine::DirtyFlags flags = state.state();

    bool update_fast_pen = false;

    if (flags & DirtyTransform) {
        update_fast_pen = true;
        updateMatrix(state.matrix());
    }

    if (flags & DirtyPen) {
        update_fast_pen = true;
        updatePen(state.pen());
    }

    if (flags & DirtyBrush) {
        updateBrush(state.brush(), state.brushOrigin());
    }

    if (flags & DirtyBackground) {
        updateBackground(state.backgroundMode(), state.backgroundBrush());
    }

    if (flags & DirtyFont) {
        updateFont(state.font());
    }

    if (state.state() & DirtyClipEnabled) {
        if (state.isClipEnabled())
            updateClipRegion(painter()->clipRegion(), Qt::ReplaceClip);
        else
            updateClipRegion(QRegion(), Qt::NoClip);
    }

    if (flags & DirtyClipPath) {
        updateClipRegion(QRegion(state.clipPath().toFillPolygon().toPolygon(),
                                 state.clipPath().fillRule()),
                         state.clipOperation());
    }

    if (flags & DirtyClipRegion) {
        updateClipRegion(state.clipRegion(), state.clipOperation());
    }

    if (flags & DirtyHints) {
        updateRenderHints(state.renderHints());
    }

    if (update_fast_pen) {
        Q_D(QOpenGLPaintEngine);
        qreal pen_width = d->cpen.widthF();
        d->has_fast_pen =
            (pen_width == 0 || (pen_width == 1 && d->txop <= QPainterPrivate::TxTranslate))
            && d->cpen.style() == Qt::SolidLine
            && d->cpen.isSolid();
    }
}

void QOpenGLPaintEnginePrivate::updateGradient(const QBrush &brush)
{
    bool has_mirrored_repeat = QGLExtensions::glExtensions & QGLExtensions::MirroredRepeat;
    bool has_frag_program = QGLExtensions::glExtensions & QGLExtensions::FragmentProgram;
    Qt::BrushStyle style = brush.style();

    if (has_mirrored_repeat && style == Qt::LinearGradientPattern) {
        const QLinearGradient *g = static_cast<const QLinearGradient *>(brush.gradient());
        float tr[4], f;
        tr[0] = g->finalStop().x() - g->start().x();
        tr[1] = g->finalStop().y() - g->start().y();
        f = 1.0 / (tr[0]*tr[0] + tr[1]*tr[1]);
        tr[0] *= f;
        tr[1] *= f;
        tr[2] = 0;
        tr[3] = -(g->start().x()*tr[0] + g->start().y()*tr[1]);
        setGLBrush(Qt::white);
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
        glTexGenfv(GL_S, GL_OBJECT_PLANE, tr);

        glBindTexture(GL_TEXTURE_1D, grad_palette);
        createGradientPaletteTexture(*brush.gradient());
    } else if (has_frag_program) {
        if (style == Qt::RadialGradientPattern) {
            const QRadialGradient *g = static_cast<const QRadialGradient *>(brush.gradient());
            QMatrix translate(1, 0, 0, 1, -g->focalPoint().x(), -g->focalPoint().y());
            QMatrix gl_to_qt(1, 0, 0, -1, 0, pdev->height());
            QMatrix inv_matrix = gl_to_qt * matrix.inverted() * translate;

            glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, radial_frag_program);

            float pt[4] = {inv_matrix.dx(), inv_matrix.dy(), 0.f, 0.f};
            float inv[4] = {inv_matrix.m11(), inv_matrix.m21(),
                            inv_matrix.m12(), inv_matrix.m22()};
            glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 0, inv); // inv_matrix
            glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 1, pt); // inv_matrix_offset

            pt[0] = g->center().x() - g->focalPoint().x();
            pt[1] = g->center().y() - g->focalPoint().y();
            glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 2, pt); // fmp

            float f[4] = {-pt[0]*pt[0] - pt[1]*pt[1] + g->radius()*g->radius(), 0.f, 0.f, 0.f};
            glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 4, f); // fmp2_m_radius2

            glBindTexture(GL_TEXTURE_1D, grad_palette);
            createGradientPaletteTexture(*brush.gradient());
        } else if (style == Qt::ConicalGradientPattern) {
            const QConicalGradient *g = static_cast<const QConicalGradient *>(brush.gradient());
            QMatrix translate(1, 0, 0, 1, -g->center().x(), -g->center().y());
            QMatrix gl_to_qt(1, 0, 0, -1, 0, pdev->height());
            QMatrix inv_matrix = gl_to_qt * matrix.inverted() * translate;

            glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, conical_frag_program);

            float pt[4] = {inv_matrix.dx(), inv_matrix.dy(), 0.f, 0.f};
            float inv[4] = {inv_matrix.m11(), inv_matrix.m21(),
                            inv_matrix.m12(), inv_matrix.m22()};
            glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 0, inv); // inv_matrix
            glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 1, pt); // inv_matrix_offset

            float angle[4] = {-(g->angle() * 2 * Q_PI) / 360.0, 0.f, 0.f, 0.f};
            glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 4, angle); // angle

            glBindTexture(GL_TEXTURE_1D, grad_palette);
            createGradientPaletteTexture(*brush.gradient());
        }
    }
    glDisable(GL_TEXTURE_1D);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_FRAGMENT_PROGRAM_ARB);
}


void QOpenGLPaintEngine::updatePen(const QPen &pen)
{
    Q_D(QOpenGLPaintEngine);
    Qt::PenStyle pen_style = pen.style();
    d->pen_brush_style = pen.brush().style();
    d->cpen = pen;
    d->has_pen = (pen_style != Qt::NoPen);


    if (d->pen_brush_style >= Qt::LinearGradientPattern
        && d->pen_brush_style <= Qt::ConicalGradientPattern) {
        d->updateGradient(pen.brush());
        d->setGLPen(Qt::white);
    } else {
        d->setGLPen(pen.color());
        glColor4ubv(d->pen_color);
    }

    d->basicStroker.setJoinStyle(pen.joinStyle());
    d->basicStroker.setCapStyle(pen.capStyle());
    d->basicStroker.setMiterLimit(pen.miterLimit());
    qreal penWidth = pen.widthF();
    if (penWidth == 0)
        d->basicStroker.setStrokeWidth(1);
    else
        d->basicStroker.setStrokeWidth(penWidth);

    if (pen_style == Qt::SolidLine) {
        d->stroker = &d->basicStroker;
    } else if (pen_style != Qt::NoPen) {
        if (!d->dashStroker)
            d->dashStroker = new QDashStroker(&d->basicStroker);
        d->dashStroker->setDashPattern(pen.dashPattern());
        d->stroker = d->dashStroker;
    } else {
        d->stroker = 0;
    }
}

void QOpenGLPaintEngine::updateBrush(const QBrush &brush, const QPointF &)
{
    Q_D(QOpenGLPaintEngine);
    d->cbrush = brush;
    d->brush_style = brush.style();
    d->has_brush = (d->brush_style != Qt::NoBrush);

    if (d->brush_style >= Qt::LinearGradientPattern
        && d->brush_style <= Qt::ConicalGradientPattern) {
        d->updateGradient(brush);
    } else {
        d->setGLBrush(brush.color());
        glColor4ubv(d->brush_color);
    }
}

void QOpenGLPaintEngine::updateFont(const QFont &)
{
}

void QOpenGLPaintEngine::updateBackground(Qt::BGMode bgMode, const QBrush &bgBrush)
{
    Q_D(QOpenGLPaintEngine);
    const QColor &c = bgBrush.color();
    glClearColor(c.redF(), c.greenF(), c.blueF(), 1.0);
    d->bgmode = bgMode;
    d->bgbrush = bgBrush;
}

void QOpenGLPaintEngine::updateMatrix(const QMatrix &mtx)
{
    Q_D(QOpenGLPaintEngine);

    d->matrix = mtx;
    GLdouble mat[4][4];

    mat[0][0] = mtx.m11();
    mat[0][1] = mtx.m12();
    mat[0][2] = 0;
    mat[0][3] = 0;

    mat[1][0] = mtx.m21();
    mat[1][1] = mtx.m22();
    mat[1][2] = 0;
    mat[1][3] = 0;

    mat[2][0] = 0;
    mat[2][1] = 0;
    mat[2][2] = 1;
    mat[2][3] = 0;

    mat[3][0] = mtx.dx();
    mat[3][1] = mtx.dy();
    mat[3][2] = 0;
    mat[3][3] = 1;

    if (mtx.m12() != 0 || mtx.m21() != 0)
        d->txop = QPainterPrivate::TxRotShear;
    else if (mtx.m11() != 1 || mtx.m22() != 1)
        d->txop = QPainterPrivate::TxScale;
    else if (mtx.dx() != 0 || mtx.dy() != 0)
        d->txop = QPainterPrivate::TxTranslate;
    else
        d->txop = QPainterPrivate::TxNone;

    // 1/10000 == 0.0001, so we have good enough res to cover curves
    // that span the entire widget...
    d->inverseScale = qMax(1 / qMax( qMax(qAbs(mtx.m11()), qAbs(mtx.m22())),
                                     qMax(qAbs(mtx.m12()), qAbs(mtx.m21())) ),
                           0.0001);

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(&mat[0][0]);
}

void QOpenGLPaintEngine::updateClipRegion(const QRegion &clipRegion, Qt::ClipOperation op)
{
    Q_D(QOpenGLPaintEngine);
    QGLFormat f = d->drawable.format();
    bool useStencilBuffer = f.stencil();
    bool useDepthBuffer = f.depth() && !useStencilBuffer;

    // clipping is only supported when a stencil or depth buffer is
    // available
    if (!useStencilBuffer && !useDepthBuffer)
        return;

    if (op == Qt::NoClip) {
        d->has_clipping = false;
        d->crgn = QRegion();
        glDisable(useStencilBuffer ? GL_STENCIL_TEST : GL_DEPTH_TEST);
        return;
    }

    QRegion region = clipRegion * d->matrix;
    switch (op) {
    case Qt::IntersectClip:
        if (d->has_clipping) {
            d->crgn &= region;
            break;
        }
        // fall through
    case Qt::ReplaceClip:
        d->crgn = region;
        break;
    case Qt::UniteClip:
        d->crgn |= region;
        break;
    default:
        break;
    }

    if (useStencilBuffer) {
        glClearStencil(0x0);
        glClear(GL_STENCIL_BUFFER_BIT);
        glClearStencil(0x1);
    } else {
        glClearDepth(0x0);
        glClear(GL_DEPTH_BUFFER_BIT);
        glDepthMask(true);
        glClearDepth(0x1);
    }

    const QVector<QRect> rects = d->crgn.rects();
    glEnable(GL_SCISSOR_TEST);
    for (int i = 0; i < rects.size(); ++i) {
        glScissor(rects.at(i).left(), d->drawable.size().height() - rects.at(i).bottom(),
                  rects.at(i).width(), rects.at(i).height());
        glClear(useStencilBuffer ? GL_STENCIL_BUFFER_BIT : GL_DEPTH_BUFFER_BIT);
    }
    glDisable(GL_SCISSOR_TEST);

    if (useStencilBuffer) {
        glStencilFunc(GL_EQUAL, 0x1, 0x1);
        glEnable(GL_STENCIL_TEST);
    } else {
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_DEPTH_TEST);
    }
    d->has_clipping = true;
}

void QOpenGLPaintEngine::updateRenderHints(QPainter::RenderHints hints)
{
    if (hints & QPainter::Antialiasing)
        glEnable(GL_MULTISAMPLE);
    else
        glDisable(GL_MULTISAMPLE);
}

void QOpenGLPaintEngine::drawRects(const QRectF *rects, int rectCount)
{
    Q_D(QOpenGLPaintEngine);

    for (int i=0; i<rectCount; ++i) {
        QRectF r = rects[i];

        qreal left = r.left();
        qreal right = r.right();
        qreal top = r.top();
        qreal bottom = r.bottom();

        if (d->has_brush) {
            d->setGradientOps(d->brush_style);
            glColor4ubv(d->brush_color);
            glRectd(left, top, right, bottom);
        }

        if (d->has_pen) {
            d->setGradientOps(d->pen_brush_style);
            glColor4ubv(d->pen_color);
            if (d->has_fast_pen) {
                glColor4ubv(d->pen_color);
                glBegin(GL_LINE_STRIP);
                {
                    glVertex2d(left, top);
                    glVertex2d(right, top);
                    glVertex2d(right, bottom);
                    glVertex2d(left, bottom);
                    glVertex2d(left, top);
                }
                glEnd();
            } else {
                d->beginPath(QPaintEngine::WindingMode);
                d->stroker->begin(d);
                d->stroker->moveTo(qt_real_to_fixed(left), qt_real_to_fixed(top));
                d->stroker->lineTo(qt_real_to_fixed(right), qt_real_to_fixed(top));
                d->stroker->lineTo(qt_real_to_fixed(right), qt_real_to_fixed(bottom));
                d->stroker->lineTo(qt_real_to_fixed(left), qt_real_to_fixed(bottom));
                d->stroker->lineTo(qt_real_to_fixed(left), qt_real_to_fixed(top));
                d->stroker->end();
                d->endPath();
            }
        }
    }
}

void QOpenGLPaintEngine::drawPoints(const QPointF *points, int pointCount)
{
    Q_D(QOpenGLPaintEngine);
    d->setGradientOps(d->pen_brush_style);

    GLfloat pen_width = d->cpen.widthF();
    if (pen_width > 1 || (pen_width > 0 && d->txop > QPainterPrivate::TxTranslate)) {
        QPaintEngine::drawPoints(points, pointCount);
        return;
    }

    glBegin(GL_POINTS);
    {
        for (int i=0; i<pointCount; ++i)
            glVertex2d(points[i].x(), points[i].y());
    }
    glEnd();
}

void QOpenGLPaintEngine::drawLines(const QLineF *lines, int lineCount)
{
    Q_D(QOpenGLPaintEngine);
    if (d->has_pen) {
        d->setGradientOps(d->pen_brush_style);
        if (d->has_fast_pen) {
            glColor4ubv(d->pen_color);
            glBegin(GL_LINES);
            {
                for (int i = 0; i < lineCount; ++i) {
                    glVertex2d(lines[i].x1(), lines[i].y1());
                    glVertex2d(lines[i].x2(), lines[i].y2());
                }
            }
            glEnd();
        } else {
            glColor4ubv(d->pen_color);
            for (int i=0; i<lineCount; ++i) {
                const QLineF &l = lines[i];
                d->beginPath(QPaintEngine::WindingMode);
                d->stroker->begin(d);
                d->stroker->moveTo(qt_real_to_fixed(l.x1()), qt_real_to_fixed(l.y1()));
                d->stroker->lineTo(qt_real_to_fixed(l.x2()), qt_real_to_fixed(l.y2()));
                d->stroker->end();
                d->endPath();
            }
        }
    }
}


void QOpenGLPaintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    Q_D(QOpenGLPaintEngine);
    if(pointCount < 2)
        return;

    if (d->has_brush && mode != PolylineMode) {
        d->setGradientOps(d->brush_style);
        if (mode == ConvexMode) {
            glBegin(GL_TRIANGLE_FAN); {
                for (int i=0; i<pointCount; ++i)
                    glVertex2d(points[i].x(), points[i].y());
            }
            glEnd();
        } else {
            glColor4ubv(d->brush_color);
            d->beginPath(mode);
            d->moveTo(points[0]);
            for (int i=1; i<pointCount; ++i)
                d->lineTo(points[i]);
            d->endPath();
        }
    }

    if (d->has_pen) {
        d->setGradientOps(d->pen_brush_style);
        glColor4ubv(d->pen_color);
        if (d->has_fast_pen) {
            glBegin(GL_LINE_STRIP); {
                for (int i=0; i<pointCount; ++i)
                    glVertex2d(points[i].x(), points[i].y());
                if (mode != PolylineMode)
                    glVertex2d(points[0].x(), points[0].y());
            }
            glEnd();
        } else {
            d->beginPath(WindingMode);
            d->stroker->strokePolygon(points, pointCount, mode != PolylineMode, d, QMatrix());
            d->endPath();
        }
    }
}

void QOpenGLPaintEngine::drawPath(const QPainterPath &path)
{
    Q_D(QOpenGLPaintEngine);

    if (path.isEmpty())
        return;

    if (d->has_brush) {
        d->setGradientOps(d->brush_style);
        // Don't split "simple" paths...
        if (path.elementCount() > 32) {
            qt_painterpath_split(path, &d->int_buffer, &d->subpath_buffer);
            glColor4ubv(d->brush_color);
            for (int i=0; i<d->int_buffer.size(); ++i) {
                d->beginPath(path.fillRule() == Qt::WindingFill
                             ? QPaintEngine::WindingMode
                             : QPaintEngine::OddEvenMode);
                for (; d->int_buffer.at(i) != -1; ++i) {
                    const QSubpath &sp = d->subpath_buffer.at(d->int_buffer.at(i));
                    for (int j=sp.start; j<=sp.end; ++j) {
                        const QPainterPath::Element &e = path.elementAt(j);
                        switch (e.type) {
                        case QPainterPath::MoveToElement:
                            d->moveTo(e);
                            break;
                        case QPainterPath::LineToElement:
                            d->lineTo(e);
                            break;
                        case QPainterPath::CurveToElement:
                            d->curveTo(e, path.elementAt(j+1), path.elementAt(j+2));
                            j+=2;
                            break;
                        default:
                            break;
                        }
                    }
                }
                d->endPath();
            }
        } else {
            glColor4ubv(d->brush_color);
            d->beginPath(path.fillRule() == Qt::WindingFill
                         ? QPaintEngine::WindingMode
                         : QPaintEngine::OddEvenMode);
            for (int i=0; i<path.elementCount(); ++i) {
                const QPainterPath::Element &e = path.elementAt(i);
                switch (e.type) {
                case QPainterPath::MoveToElement:
                    d->moveTo(e);
                    break;
                case QPainterPath::LineToElement:
                    d->lineTo(e);
                    break;
                case QPainterPath::CurveToElement:
                    d->curveTo(e, path.elementAt(i+1), path.elementAt(i+2));
                    i+=2;
                    break;
                default:
                    break;
                }
            }
            d->endPath();
        }
    }

    if (d->has_pen) {
        glColor4ubv(d->pen_color);
        d->setGradientOps(d->pen_brush_style);
        if (d->has_fast_pen) {
            QBezier beziers[32];
            for (int i=0; i<path.elementCount(); ++i) {
                const QPainterPath::Element &e = path.elementAt(i);
                switch (e.type) {
                case QPainterPath::MoveToElement:
                    if (i != 0)
                        glEnd(); // GL_LINE_STRIP
                    glBegin(GL_LINE_STRIP);
                    glVertex2d(e.x, e.y);
                    break;
                case QPainterPath::LineToElement:
                    glVertex2d(e.x, e.y);
                    break;

                case QPainterPath::CurveToElement:
                    {
                        QPointF sp = path.elementAt(i-1);
                        QPointF cp2 = path.elementAt(i+1);
                        QPointF ep = path.elementAt(i+2);
                        i+=2;

                        qreal inverseScaleHalf = d->inverseScale / 2;
                        beziers[0] = QBezier::fromPoints(sp, e, cp2, ep);
                        QBezier *b = beziers;
                        while (b >= beziers) {
                            // check if we can pop the top bezier curve from the stack
                            qreal l = qAbs(b->x4 - b->x1) + qAbs(b->y4 - b->y1);
                            qreal d;
                            if (l > d_func()->inverseScale) {
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
                                glVertex2d(b->x4, b->y4);
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
            glEnd(); // GL_LINE_STRIP
        } else {
            d->beginPath(WindingMode);
            d->stroker->strokePath(path, d, QMatrix());
            d->endPath();
        }
    }
}

void QOpenGLPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr)
{
    Q_D(QOpenGLPaintEngine);
    if (pm.depth() == 1) {
        QPixmap tpx(pm.size());
        tpx.fill(d->bgbrush.color());
        QPainter p(&tpx);
        p.setPen(d->cpen);
        p.drawPixmap(0, 0, pm);
        p.end();
        drawPixmap(r, tpx, sr);
        return;
    }
    GLenum target = QGLExtensions::glExtensions & QGLExtensions::TextureRectangle
                    ? GL_TEXTURE_RECTANGLE_NV
                    : GL_TEXTURE_2D;
    if (r.size() != pm.size())
        target = GL_TEXTURE_2D;
    d->drawable.bindTexture(pm, target);

    drawTextureRect(pm.width(), pm.height(), r, sr, target);
}

void QOpenGLPaintEngine::drawTiledPixmap(const QRectF &r, const QPixmap &pm, const QPointF &)
{
    Q_D(QOpenGLPaintEngine);
    d->drawable.bindTexture(pm);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glPushAttrib(GL_CURRENT_BIT);
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);

    GLdouble tc_w = r.width()/pm.width();
    GLdouble tc_h = r.height()/pm.height();

    // Rotate the texture so that it is aligned correctly and the
    // wrapping is done correctly
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glRotated(180.0, 0.0, 1.0, 0.0);
    glRotated(180.0, 0.0, 0.0, 1.0);
    glBegin(GL_QUADS);
    {
        glTexCoord2d(0.0, 0.0);
        glVertex2d(r.x(), r.y());

        glTexCoord2d(tc_w, 0.0);
        glVertex2d(r.x()+r.width(), r.y());

        glTexCoord2d(tc_w, tc_h);
        glVertex2d(r.x()+r.width(), r.y()+r.height());

        glTexCoord2d(0.0, tc_h);
        glVertex2d(r.x(), r.y()+r.height());
    }
    glEnd();
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
    glPopAttrib();
}

void QOpenGLPaintEngine::drawImage(const QRectF &r, const QImage &image, const QRectF &sr,
                                   Qt::ImageConversionFlags)
{
    Q_D(QOpenGLPaintEngine);
    GLenum target = QGLExtensions::glExtensions & QGLExtensions::TextureRectangle
                    ? GL_TEXTURE_RECTANGLE_NV
                    : GL_TEXTURE_2D;
    if (r.size() != image.size())
        target = GL_TEXTURE_2D;
    d->drawable.bindTexture(image, target);
    drawTextureRect(image.width(), image.height(), r, sr, target);
}

void QOpenGLPaintEngine::drawTextureRect(int tx_width, int tx_height, const QRectF &r,
                                         const QRectF &sr, GLenum target)
{
    glPushAttrib(GL_CURRENT_BIT);
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glEnable(target);

    glBegin(GL_QUADS);
    {
        qreal x1, x2, y1, y2;
        if (target == GL_TEXTURE_2D) {
            x1 = sr.x() / tx_width;
            x2 = x1 + sr.width() / tx_width;
            y1 = 1.0 - ((sr.y() / tx_height) + (sr.height() / tx_height));
            y2 = 1.0 - (sr.y() / tx_height);
        } else {
            x1 = sr.x();
            x2 = sr.width();
            y1 = sr.y();
            y2 = sr.height();
        }

        glTexCoord2d(x1, y2);
        glVertex2d(r.x(), r.y());

        glTexCoord2d(x2, y2);
        glVertex2d(r.x()+r.width(), r.y());

        glTexCoord2d(x2, y1);
        glVertex2d(r.x()+r.width(), r.y()+r.height());

        glTexCoord2d(x1, y1);
        glVertex2d(r.x(), r.y()+r.height());
    }
    glEnd();

    glDisable(target);
    glPopAttrib();
}

#ifdef Q_WS_WIN
HDC
#else
Qt::HANDLE
#endif
QOpenGLPaintEngine::handle() const
{
    return 0;
}

static const int x_margin = 1;
static const int y_margin = 0;
static QImage *glyphs_m0 = 0;
static int next_x = x_margin;
static int next_y = y_margin;
static GLuint glyph_tex = 0;

struct GlyphTexture {
    qreal x; // glyph texture coords
    qreal y;
    qreal width;
    qreal height;
};
static QHash<QChar, GlyphTexture*> glyphHash;

void QOpenGLPaintEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
#if 1
    Q_D(QOpenGLPaintEngine);
    bool updateGlyphTexture = false;
    const int scale = 1;

//    GLenum target = GL_TEXTURE_2D;
    GLenum target = GL_TEXTURE_RECTANGLE_NV;

    // add the glyphs used to the glyph texture cache
    const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);
    for (int i=0; i< ti.num_chars; ++i) {
        if (ti.chars[i] == ' ')
            continue;
        QHash<QChar, GlyphTexture *>::const_iterator it = glyphHash.find(ti.chars[i]);
        GlyphTexture *tex;
        if (it == glyphHash.end()) {
            glyph_metrics_t gm = ti.fontEngine->boundingBox(ti.glyphs[i].glyph);
            // render new glyph and put it in the cache
            QPixmap pm0((qRound(gm.width.toReal())+2)*scale, (qRound(ti.ascent.toReal() + ti.descent.toReal())+2)*scale);
            pm0.fill(Qt::transparent);
            QPainter p(&pm0);
            p.setFont(painter()->font());
            p.scale(scale, scale);
            p.setPen(Qt::black);
            p.drawText(0, qRound(ti.ascent.toReal()), ti.chars[i]);
            p.end();

            if (glyphs_m0 == 0) {
                glyphs_m0 = new QImage(1024, 1024, QImage::Format_ARGB32);
                glyphs_m0->fill(0);
            }

            if (next_x + pm0.width() + x_margin > glyphs_m0->width()) {
                next_x = x_margin;
                next_y += pm0.height() + x_margin;
            }

            tex = new GlyphTexture;
//             tex->x = next_x;
//             tex->y = next_y;
//             tex->width = pm0.width()/scale;
//             tex->height = pm0.height()/scale;

            if (target == GL_TEXTURE_2D) {
                tex->x = float(next_x) / glyphs_m0->width();
                tex->y = float(next_y) / glyphs_m0->height();
                tex->width = float(pm0.width()/scale) / glyphs_m0->width();
                tex->height = float(pm0.height()/scale) / glyphs_m0->height();
//                 x1 = float(it.value()->x) / glyphs_m0->width();
//                 y1 = float(it.value()->y) / glyphs_m0->height();
//                 x2 = x1 + float(it.value()->width*scale) / glyphs_m0->width();
//                 y2 = y1 + float(it.value()->height*scale) / glyphs_m0->height();
            } else {
                tex->x = next_x;
                tex->y = next_y;
                tex->width = pm0.width();
                tex->height = pm0.height();
//                 x1 = it.value()->x;
//                 y1 = it.value()->y;
//                 x2 = x1 + it.value()->width*scale;
//                 y2 = y1 + it.value()->height*scale;
            }


            QPainter ip(glyphs_m0);
            ip.drawPixmap(next_x, next_y, pm0);
            ip.end();

            if (next_x + pm0.width() + x_margin > glyphs_m0->width()) {
                next_x = x_margin;
                next_y += pm0.height() + y_margin;
            } else {
                next_x += pm0.width() + x_margin;
            }

            glyphHash.insert(ti.chars[i], tex);
            updateGlyphTexture = true;
        }
    }

    // regen texture if it needs to be updated

    if (updateGlyphTexture) {
        glDeleteTextures(1, &glyph_tex);
        glGenTextures(1, &glyph_tex);

        int i = 0;
        uchar tex_data[1024*1024*2];
        for (int y=0; y<glyphs_m0->height(); ++y) {
            uint *s = (uint *) glyphs_m0->scanLine(y);
            for (int x=0; x<glyphs_m0->width(); ++x) {
                tex_data[i] = 0xff;
                tex_data[i+1] = qAlpha(*s);
                ++s;
                i += 2;
            }
        }
        glBindTexture(target, glyph_tex);

        glTexImage2D(target, 0, GL_LUMINANCE8_ALPHA8, 1024, 1024, 0,
                     GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, &tex_data);

//         gluBuild2DMipmaps(target, GL_LUMINANCE8_ALPHA8, 1024, 1024,
//                           GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, &tex_data);

#if 0   // filtering
        glTexParameterf(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#else
        glTexParameterf(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//_MIPMAP_NEAREST);
        glTexParameterf(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif
    }
//      else {
//          // all in
//          glyphs_m0->save("m0.png", "PNG");
//          glyphs_m1->save("m1.png", "PNG");
//      }

    glBindTexture(target, glyph_tex);
    glColor4ubv(d->pen_color);
    glEnable(target);

    // do the actual drawing
    glBegin(GL_QUADS);
    {
        for (int i=0; i< ti.num_chars; ++i) {
            if (ti.chars[i] == ' ')
                continue;
            QHash<QChar, GlyphTexture *>::const_iterator it = glyphHash.find(ti.chars[i]);
            Q_ASSERT(it != glyphHash.end());
            qreal x1, x2, y1, y2;
            x1 = it.value()->x;
            y1 = it.value()->y;
            x2 = x1 + it.value()->width;
            y2 = y1 + it.value()->height;
//             if (target == GL_TEXTURE_2D) {
//                 x1 = float(it.value()->x) / glyphs_m0->width();
//                 y1 = float(it.value()->y) / glyphs_m0->height();
//                 x2 = x1 + float(it.value()->width*scale) / glyphs_m0->width();
//                 y2 = y1 + float(it.value()->height*scale) / glyphs_m0->height();
//             } else {
//                 x1 = it.value()->x;
//                 y1 = it.value()->y;
//                 x2 = x1 + it.value()->width*scale;
//                 y2 = y1 + it.value()->height*scale;
//             }
            QMatrix inv = d->matrix;
            inv.translate(p.x(), p.y());
            inv = inv.inverted();

            QPointF logical_pos(inv.map(QPointF(ti.deviceGlyphPositions[i].x.toReal(),
                                                ti.deviceGlyphPositions[i].y.toReal())));
            QPointF offset(p - QPointF(0, ti.ascent.toReal()));
            if (d->txop > QPainterPrivate::TxTranslate) // ### not really sure were this is coming from...
                offset -= d->matrix.map(p);

            QRectF r(logical_pos + offset,
//                      QSize(qRound(it.value()->width*glyphs_m0->width()),
//                           qRound(it.value()->height*glyphs_m0->height())));
                     QSize(qRound(it.value()->width),
                           qRound(it.value()->height)));
            glTexCoord2d(x1, y1);
            glVertex2d(r.x(), r.y());

            glTexCoord2d(x2, y1);
            glVertex2d(r.x()+r.width(), r.y());

            glTexCoord2d(x2, y2);
            glVertex2d(r.x()+r.width(), r.y()+r.height());

            glTexCoord2d(x1, y2);
            glVertex2d(r.x(), r.y()+r.height());
        }
    }
    glEnd();
#endif
// < -- Working stuff start
// struct GlyphTexture {
//     QImage *glyph;
// };
// static QHash<QChar, GlyphTexture*> glyphHash;
// #if 1
//     Q_D(QOpenGLPaintEngine);
//     QString str = textItem.text();
//     const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);
//     for (int i=0; i< str.length(); ++i) {
//         if (str.at(i) == ' ')
//             continue;
//         QHash<QChar, GlyphTexture *>::const_iterator it = glyphHash.find(str.at(i));
//         GlyphTexture *tex;
//         if (it == glyphHash.end()) {
//             glyph_metrics_t gm = ti.fontEngine->boundingBox(ti.glyphs[i].glyph);
//             // render new glyph and insert
//             QPixmap pm(gm.width.toInt()+2, (int)(textItem.ascent() + textItem.descent())+2);
//             pm.fill(Qt::transparent);//QColor(Qt::white));
//             QPainter p(&pm);
//             p.translate(1, 1);
//             p.setPen(d->cpen);
//             p.drawText(0, qRound(textItem.ascent()), str.at(i));
//             p.end();
//             GlyphTexture *gt = new GlyphTexture;
//             gt->glyph = new QImage(pm.toImage().convertToFormat(QImage::Format_ARGB32));
//             glyphHash.insert(str.at(i), gt);
//             tex = gt;
//         } else {
//             tex = it.value();
//         }
//         QPointF pt(ti.deviceGlyphPositions[i].x.toReal(), ti.deviceGlyphPositions[i].y.toReal());
//         drawImage(QRectF(pt.x(), pt.y(), tex->glyph->width(), tex->glyph->height()),
//                   *tex->glyph,
//                   QRectF(0, 0, tex->glyph->width(), tex->glyph->height()),
//                   Qt::AutoColor);
//     }
// #endif
// -- > Working stuff -- end
// #ifdef Q_WS_WIN
//     Q_D(QOpenGLPaintEngine);
//     // the image fallback is both faster and nicer looking on windows...
//     if (d->txop <= QPainterPrivate::TxTranslate) {
//         QImage img((int)textItem.width(),
//                    (int)(textItem.ascent() + textItem.descent()),
//                    QImage::Format_ARGB32_Premultiplied);
//         img.fill(0);
//         QPainter painter(&img);
//         painter.setPen(d->cpen);
//         painter.setBrush(d->cbrush);
//         painter.drawTextItem(QPoint(0, (int)(textItem.ascent())), textItem);
//         painter.end();
//         drawImage(QRectF(p.x(), p.y()-(textItem.ascent()), img.width(), img.height()),
//                   img,
//                   QRectF(0, 0, img.width(), img.height()),
//                   Qt::AutoColor);
//     } else
// #endif
//     {
//        QPaintEngine::drawTextItem(p, textItem);
//     }
}
