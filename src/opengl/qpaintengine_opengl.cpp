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
#include <qglpbuffer.h>
#include <private/qglpbuffer_p.h>
#include <private/qmath_p.h>

static void mk_texture(const QGradient& g);
static void mk_color_table(const QGradientStops& s, unsigned int *colorTable, int size);

#ifdef Q_OS_MAC
# include <OpenGL/glu.h>
#else
# include <GL/glu.h>
#endif

#include <stdlib.h>

#ifndef CALLBACK // for Windows
#define CALLBACK
#endif

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

// define QT_GL_NO_CONCAVE_POLYGONS to remove support for drawing
// concave polygons (much faster)

//#define QT_GL_NO_CONCAVE_POLYGONS

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
    QGLPbuffer *buffer;
};

void QGLDrawable::setDevice(QPaintDevice *pdev)
{
    if (pdev->devType() == QInternal::Widget)
        widget = static_cast<QGLWidget *>(pdev);
    else if (pdev->devType() == QInternal::Pbuffer)
        buffer = static_cast<QGLPbuffer *>(pdev);
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


class QOpenGLPaintEnginePrivate : public QPaintEnginePrivate {
    Q_DECLARE_PUBLIC(QOpenGLPaintEngine)
public:
    QOpenGLPaintEnginePrivate()
        : bgmode(Qt::TransparentMode)
        , txop(QPainterPrivate::TxNone) {}

    void setGLPen(const QColor &c) {
        pen_color[0] = c.red();
        pen_color[1] = c.green();
        pen_color[2] = c.blue();
        pen_color[3] = c.alpha();
    }

    void setGLBrush(const QColor &c) {
        brush_color[0] = c.red();
        brush_color[1] = c.green();
        brush_color[2] = c.blue();
        brush_color[3] = c.alpha();
    }

    QPen cpen;
    QBrush cbrush;
    QBrush bgbrush;
    Qt::BGMode bgmode;
    QRegion crgn;
    uint has_clipping : 1;
    uint has_pen : 1;
    uint has_brush : 1;
    uint has_autoswap : 1;

    QMatrix matrix;
    GLubyte pen_color[4];
    GLubyte brush_color[4];
    QPainterPrivate::TransformationCodes txop;
    QGLDrawable drawable;

    GLuint palette_tex;
    GLuint grad_radial;
    GLuint grad_radial_palette_loc;
    GLuint grad_radial_focus_loc;
    GLuint grad_radial_fmp_loc;
    GLuint grad_radial_fmp2_m_radius2_loc;
    GLuint grad_conical;
    GLuint grad_conical_palette_loc;
    GLuint grad_conical_point_loc;
    GLuint grad_conical_angle_loc;
};

/* shader_object extension crap */
typedef void (APIENTRY *pfn_glDeleteObjectARB) (GLhandleARB);
typedef GLhandleARB (APIENTRY *pfn_glGetHandleARB) (GLenum);
typedef void (APIENTRY *pfn_glDetachObjectARB) (GLhandleARB, GLhandleARB);
typedef GLhandleARB (APIENTRY *pfn_glCreateShaderObjectARB) (GLenum);
typedef void (APIENTRY *pfn_glShaderSourceARB) (GLhandleARB, GLsizei, const GLcharARB* *, const GLint *);
typedef void (APIENTRY *pfn_glCompileShaderARB) (GLhandleARB);
typedef GLhandleARB (APIENTRY *pfn_glCreateProgramObjectARB) (void);
typedef void (APIENTRY *pfn_glAttachObjectARB) (GLhandleARB, GLhandleARB);
typedef void (APIENTRY *pfn_glLinkProgramARB) (GLhandleARB);
typedef void (APIENTRY *pfn_glUseProgramObjectARB) (GLhandleARB);
typedef void (APIENTRY *pfn_glValidateProgramARB) (GLhandleARB);
typedef void (APIENTRY *pfn_glUniform1fARB) (GLint, GLfloat);
typedef void (APIENTRY *pfn_glUniform2fARB) (GLint, GLfloat, GLfloat);
typedef void (APIENTRY *pfn_glUniform3fARB) (GLint, GLfloat, GLfloat, GLfloat);
typedef void (APIENTRY *pfn_glUniform4fARB) (GLint, GLfloat, GLfloat, GLfloat, GLfloat);
typedef void (APIENTRY *pfn_glUniform1iARB) (GLint, GLint);
typedef void (APIENTRY *pfn_glUniform2iARB) (GLint, GLint, GLint);
typedef void (APIENTRY *pfn_glUniform3iARB) (GLint, GLint, GLint, GLint);
typedef void (APIENTRY *pfn_glUniform4iARB) (GLint, GLint, GLint, GLint, GLint);
typedef void (APIENTRY *pfn_glUniform1fvARB) (GLint, GLsizei, const GLfloat *);
typedef void (APIENTRY *pfn_glUniform2fvARB) (GLint, GLsizei, const GLfloat *);
typedef void (APIENTRY *pfn_glUniform3fvARB) (GLint, GLsizei, const GLfloat *);
typedef void (APIENTRY *pfn_glUniform4fvARB) (GLint, GLsizei, const GLfloat *);
typedef void (APIENTRY *pfn_glUniform1ivARB) (GLint, GLsizei, const GLint *);
typedef void (APIENTRY *pfn_glUniform2ivARB) (GLint, GLsizei, const GLint *);
typedef void (APIENTRY *pfn_glUniform3ivARB) (GLint, GLsizei, const GLint *);
typedef void (APIENTRY *pfn_glUniform4ivARB) (GLint, GLsizei, const GLint *);
typedef void (APIENTRY *pfn_glUniformMatrix2fvARB) (GLint, GLsizei, GLboolean, const GLfloat *);
typedef void (APIENTRY *pfn_glUniformMatrix3fvARB) (GLint, GLsizei, GLboolean, const GLfloat *);
typedef void (APIENTRY *pfn_glUniformMatrix4fvARB) (GLint, GLsizei, GLboolean, const GLfloat *);
typedef void (APIENTRY *pfn_glGetObjectParameterfvARB) (GLhandleARB, GLenum, GLfloat *);
typedef void (APIENTRY *pfn_glGetObjectParameterivARB) (GLhandleARB, GLenum, GLint *);
typedef void (APIENTRY *pfn_glGetInfoLogARB) (GLhandleARB, GLsizei, GLsizei *, GLcharARB *);
typedef void (APIENTRY *pfn_glGetAttachedObjectsARB) (GLhandleARB, GLsizei, GLsizei *, GLhandleARB *);
typedef GLint (APIENTRY *pfn_glGetUniformLocationARB) (GLhandleARB, const GLcharARB *);
typedef void (APIENTRY *pfn_glGetActiveUniformARB) (GLhandleARB, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLcharARB *);
typedef void (APIENTRY *pfn_glGetUniformfvARB) (GLhandleARB, GLint, GLfloat *);
typedef void (APIENTRY *pfn_glGetUniformivARB) (GLhandleARB, GLint, GLint *);
typedef void (APIENTRY *pfn_glGetShaderSourceARB) (GLhandleARB, GLsizei, GLsizei *, GLcharARB *);

static pfn_glDeleteObjectARB qt_glDeleteObjectARB = 0;
static pfn_glGetHandleARB qt_glGetHandleARB = 0;
static pfn_glDetachObjectARB qt_glDetachObjectARB = 0;
static pfn_glCreateShaderObjectARB qt_glCreateShaderObjectARB = 0;
static pfn_glShaderSourceARB qt_glShaderSourceARB = 0;
static pfn_glCompileShaderARB qt_glCompileShaderARB = 0;
static pfn_glCreateProgramObjectARB qt_glCreateProgramObjectARB = 0;
static pfn_glAttachObjectARB qt_glAttachObjectARB = 0;
static pfn_glLinkProgramARB qt_glLinkProgramARB = 0;
static pfn_glUseProgramObjectARB qt_glUseProgramObjectARB = 0;
static pfn_glValidateProgramARB qt_glValidateProgramARB = 0;
static pfn_glUniform1fARB qt_glUniform1fARB = 0;
static pfn_glUniform2fARB qt_glUniform2fARB = 0;
static pfn_glUniform3fARB qt_glUniform3fARB = 0;
static pfn_glUniform4fARB qt_glUniform4fARB = 0;
static pfn_glUniform1iARB qt_glUniform1iARB = 0;
static pfn_glUniform2iARB qt_glUniform2iARB = 0;
static pfn_glUniform3iARB qt_glUniform3iARB = 0;
static pfn_glUniform4iARB qt_glUniform4iARB = 0;
static pfn_glUniform1fvARB qt_glUniform1fvARB = 0;
static pfn_glUniform2fvARB qt_glUniform2fvARB = 0;
static pfn_glUniform3fvARB qt_glUniform3fvARB = 0;
static pfn_glUniform4fvARB qt_glUniform4fvARB = 0;
static pfn_glUniform1ivARB qt_glUniform1ivARB = 0;
static pfn_glUniform2ivARB qt_glUniform2ivARB = 0;
static pfn_glUniform3ivARB qt_glUniform3ivARB = 0;
static pfn_glUniform4ivARB qt_glUniform4ivARB = 0;
static pfn_glUniformMatrix2fvARB qt_glUniformMatrix2fvARB = 0;
static pfn_glUniformMatrix3fvARB qt_glUniformMatrix3fvARB = 0;
static pfn_glUniformMatrix4fvARB qt_glUniformMatrix4fvARB = 0;
static pfn_glGetObjectParameterfvARB qt_glGetObjectParameterfvARB = 0;
static pfn_glGetObjectParameterivARB qt_glGetObjectParameterivARB = 0;
static pfn_glGetInfoLogARB qt_glGetInfoLogARB = 0;
static pfn_glGetAttachedObjectsARB qt_glGetAttachedObjectsARB = 0;
static pfn_glGetUniformLocationARB qt_glGetUniformLocationARB = 0;
static pfn_glGetActiveUniformARB qt_glGetActiveUniformARB = 0;
static pfn_glGetUniformfvARB qt_glGetUniformfvARB = 0;
static pfn_glGetUniformivARB qt_glGetUniformivARB = 0;
static pfn_glGetShaderSourceARB qt_glGetShaderSourceARB = 0;

/* load function pointers */
static void load_shader_object_ext()
{
    QGLContext cx(QGLFormat::defaultFormat());
#define LOAD_EXT(func) qt_##func = (pfn_##func) cx.getProcAddress( #func )

    LOAD_EXT(glDeleteObjectARB);
    LOAD_EXT(glGetHandleARB);
    LOAD_EXT(glDetachObjectARB);
    LOAD_EXT(glCreateShaderObjectARB);
    LOAD_EXT(glShaderSourceARB);
    LOAD_EXT(glCompileShaderARB);
    LOAD_EXT(glCreateProgramObjectARB);
    LOAD_EXT(glAttachObjectARB);
    LOAD_EXT(glLinkProgramARB);
    LOAD_EXT(glUseProgramObjectARB);
    LOAD_EXT(glValidateProgramARB);
    LOAD_EXT(glUniform1fARB);
    LOAD_EXT(glUniform2fARB);
    LOAD_EXT(glUniform3fARB);
    LOAD_EXT(glUniform4fARB);
    LOAD_EXT(glUniform1iARB);
    LOAD_EXT(glUniform2iARB);
    LOAD_EXT(glUniform3iARB);
    LOAD_EXT(glUniform4iARB);
    LOAD_EXT(glUniform1fvARB);
    LOAD_EXT(glUniform2fvARB);
    LOAD_EXT(glUniform3fvARB);
    LOAD_EXT(glUniform4fvARB);
    LOAD_EXT(glUniform1ivARB);
    LOAD_EXT(glUniform2ivARB);
    LOAD_EXT(glUniform3ivARB);
    LOAD_EXT(glUniform4ivARB);
    LOAD_EXT(glUniformMatrix2fvARB);
    LOAD_EXT(glUniformMatrix3fvARB);
    LOAD_EXT(glUniformMatrix4fvARB);
    LOAD_EXT(glGetObjectParameterfvARB);
    LOAD_EXT(glGetObjectParameterivARB);
    LOAD_EXT(glGetInfoLogARB);
    LOAD_EXT(glGetAttachedObjectsARB);
    LOAD_EXT(glGetUniformLocationARB);
    LOAD_EXT(glGetActiveUniformARB);
    LOAD_EXT(glGetUniformfvARB);
    LOAD_EXT(glGetUniformivARB);
    LOAD_EXT(glGetShaderSourceARB);
}

/* create a shader object from the string sh */
static GLuint load_shader(const char *sh)
{
    GLint status, fs, ps;

    fs = qt_glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
    qt_glShaderSourceARB(fs, 1, &sh, NULL);
    qt_glCompileShaderARB(fs);
    qt_glGetObjectParameterivARB( fs, GL_OBJECT_COMPILE_STATUS_ARB, &status);
    if(!status){	printf("Could not compile the shader!\n"); return 0;}
    ps = qt_glCreateProgramObjectARB();
    qt_glAttachObjectARB(ps,fs);
    qt_glDeleteObjectARB(fs);
    qt_glLinkProgramARB(ps);
    qt_glGetObjectParameterivARB( ps, GL_OBJECT_LINK_STATUS_ARB, &status);
    if(!status){	printf("Could not link the shader!\n"); return 0;}
    qt_glValidateProgramARB(ps);
    qt_glGetObjectParameterivARB( ps, GL_OBJECT_VALIDATE_STATUS_ARB,&status);
    if(!status){	printf("Could not validate the shader!\n"); return 0;}
    return ps;
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

    if((QGLExtensions::glExtensions & QGLExtensions::MirroredRepeat)
       && (QGLExtensions::glExtensions &
           (QGLExtensions::ClampToEdge||QGLExtensions::ClampToBorder)))
    {
        gccaps |= LinearGradientFill;
        glGenTextures(1,&d->palette_tex);
	if(QGLExtensions::glExtensions & QGLExtensions::FragmentShader)
	{
            static char grad_radial_shad[] =
                "uniform sampler1D palette;\n"
                "uniform vec2 focus;\n"
                "uniform vec2 fmp;\n"
                "uniform float fmp2_m_radius2;\n"
                "void main(){\n"
                "	vec2 A = gl_FragCoord.xy - focus;\n"
                "	vec2 B = fmp;\n"
                "	float a = fmp2_m_radius2;\n"
                "	float b = 2.0*dot(A,B);\n"
                "	float c = dot(A,A);\n"
                "	float val = (-b - sqrt(b*b - 4.0*a*c) ) / (2.0*a);\n"
                "	gl_FragColor = texture1D( palette, val );\n"
                "}\n";
            static char grad_conical_shad[] =
                "#define M_PI  3.14159265358979323846\n"
                "uniform sampler1D palette;\n"
                "uniform vec2 point;\n"
                "uniform float angle;\n"
                "void main(){\n"
                "	vec2 A = gl_FragCoord.xy - point;\n"
                "	float val = mod((atan(-A.y, A.x) + angle)/(2.0 * M_PI),1);\n"
                "	gl_FragColor = texture1D( palette, val );\n"
                "}\n";

            gccaps |= (RadialGradientFill|ConicalGradientFill);

            load_shader_object_ext();

            d->grad_radial = load_shader(grad_radial_shad);
            d->grad_radial_palette_loc = qt_glGetUniformLocationARB(d->grad_radial,"palette");
            d->grad_radial_focus_loc = qt_glGetUniformLocationARB(d->grad_radial,"focus");
            d->grad_radial_fmp_loc = qt_glGetUniformLocationARB(d->grad_radial,"fmp");
            d->grad_radial_fmp2_m_radius2_loc = qt_glGetUniformLocationARB(d->grad_radial,
                                                                           "fmp2_m_radius2");

            d->grad_conical = load_shader(grad_conical_shad);
            d->grad_conical_palette_loc = qt_glGetUniformLocationARB(d->grad_conical,"palette");
            d->grad_conical_point_loc = qt_glGetUniformLocationARB(d->grad_conical,"point");
            d->grad_conical_angle_loc = qt_glGetUniformLocationARB(d->grad_conical,"angle");

	}
    }
}

QOpenGLPaintEngine::~QOpenGLPaintEngine()
{
    Q_D(QOpenGLPaintEngine);

    if(QGLExtensions::glExtensions & QGLExtensions::FragmentShader)
    {
        qt_glUseProgramObjectARB(0);
        glBindTexture(GL_TEXTURE_1D,0);

        qt_glDeleteObjectARB(d->grad_radial);
        qt_glDeleteObjectARB(d->grad_conical);
        glDeleteTextures(1,&d->palette_tex);
    }
}

bool QOpenGLPaintEngine::begin(QPaintDevice *pdev)
{
    Q_D(QOpenGLPaintEngine);
    d->drawable.setDevice(pdev);
    d->has_clipping = false;
    d->has_autoswap = d->drawable.autoBufferSwap();
    d->drawable.setAutoBufferSwap(false);
    setActive(true);
    d->drawable.makeCurrent();
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    const QColor &c = d->drawable.backgroundColor();
    glClearColor(c.redF(), c.greenF(), c.blueF(), 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glShadeModel(GL_FLAT);
    QSize sz(d->drawable.size());
    glViewport(0, 0, sz.width(), sz.height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, sz.width(), sz.height(), 0, -999999, 999999);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    setDirty(QPaintEngine::DirtyPen);
    setDirty(QPaintEngine::DirtyBrush);
    glDisable(GL_MULTISAMPLE);
    glDisable(GL_TEXTURE_1D);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);

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
    if (flags & DirtyPen) updatePen(state.pen());
    if (flags & DirtyBrush) updateBrush(state.brush(), state.brushOrigin());
    if (flags & DirtyBackground) updateBackground(state.backgroundMode(), state.backgroundBrush());
    if (flags & DirtyFont) updateFont(state.font());
    if (flags & DirtyTransform) updateMatrix(state.matrix());
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
    if (flags & DirtyClipRegion) updateClipRegion(state.clipRegion(), state.clipOperation());
    if (flags & DirtyHints) updateRenderHints(state.renderHints());
}

void QOpenGLPaintEngine::updatePen(const QPen &pen)
{
    Q_D(QOpenGLPaintEngine);
    d->cpen = pen;
    d->has_pen = (pen.style() != Qt::NoPen);
    d->setGLPen(pen.color());
    glColor4ubv(d->pen_color);
}

void QOpenGLPaintEngine::updateBrush(const QBrush &brush, const QPointF &)
{
    Q_D(QOpenGLPaintEngine);
    /* load the gradient shader */
    if(QGLExtensions::glExtensions & QGLExtensions::FragmentShader) {
        switch (brush.style()) {
	case Qt::LinearGradientPattern:
	{
            const QLinearGradient *g = static_cast<const QLinearGradient *>(brush.gradient());
            float tr[4], f;
            tr[0] = g->finalStop().x() - g->start().x();
            tr[1] = g->finalStop().y() - g->start().y();
            f = 1.0/(tr[0]*tr[0] + tr[1]*tr[1]);
            tr[0] *= f;
            tr[1] *= f;
            tr[2] = 0;
            tr[3] = - (g->start().x()*tr[0] + g->start().y()*tr[1]);

            d->setGLBrush(Qt::white);
            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
            glTexGenfv(GL_S, GL_OBJECT_PLANE, tr);
            glEnable(GL_TEXTURE_GEN_S);
            glEnable(GL_TEXTURE_1D);

            glBindTexture(GL_TEXTURE_1D,d->palette_tex);
            mk_texture(*brush.gradient());
            qt_glUseProgramObjectARB(0);
            break;
	}
	case Qt::RadialGradientPattern:
	{
            const QRadialGradient *g = static_cast<const QRadialGradient *>(brush.gradient());
            float pt[2];
            float f;
            qt_glUseProgramObjectARB(d->grad_radial);
            pt[0] = g->focalPoint().x();
            pt[1] = d->pdev->height() - g->focalPoint().y();
            qt_glUniform2fvARB(d->grad_radial_focus_loc,1,pt);
            pt[0] -= g->center().x();
            pt[1] -= d->pdev->height() - g->center().y();
            qt_glUniform2fvARB(d->grad_radial_fmp_loc,1,pt);
            f = pt[0]*pt[0] + pt[1]*pt[1] - g->radius()*g->radius();
            qt_glUniform1fARB(d->grad_radial_fmp2_m_radius2_loc, f);
            qt_glUniform1iARB(d->grad_radial_palette_loc, 0);

            glBindTexture(GL_TEXTURE_1D,d->palette_tex);
            mk_texture(*brush.gradient());
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_1D);
            break;
	}
	case Qt::ConicalGradientPattern:
	{
            const QConicalGradient *g = static_cast<const QConicalGradient *>(brush.gradient());
            float pt[2];
            qt_glUseProgramObjectARB(d->grad_conical);
            pt[0] = g->center().x();
            pt[1] = d->pdev->height() - g->center().y();
            qt_glUniform2fvARB(d->grad_conical_point_loc,1,pt);
            qt_glUniform1fARB(d->grad_conical_angle_loc, g->angle()* 2 * Q_PI / 360.0);
            qt_glUniform1iARB(d->grad_conical_palette_loc, 0);

            glBindTexture(GL_TEXTURE_1D,d->palette_tex);
            mk_texture(*brush.gradient());
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_1D);
            break;
	}
    	default:
            qt_glUseProgramObjectARB(0);
            glBindTexture(GL_TEXTURE_1D,0);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_1D);
            break;
        }
    }

    d->cbrush = brush;
    d->has_brush = (brush.style() != Qt::NoBrush);
    d->setGLBrush(brush.color());
    glColor4ubv(d->brush_color);
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
    // ### this could be done faster I'm sure...
    for (int i=0; i<rectCount; ++i) {
        QRectF r = rects[i];
        double x = r.x();
        double y = r.y();
        double w = r.width();
        double h = r.height();
        if (d->has_brush) {
            glColor4ubv(d->brush_color);
            glRectd(x, y, x+w, y+h);
        }

        if (d->has_pen) {
            glColor4ubv(d->pen_color);
            glBegin(GL_LINE_LOOP);
            {
                glVertex2d(x, y);
                glVertex2d(x+w, y);
                glVertex2d(x+w, y+h);
                glVertex2d(x, y+h);
            }
            glEnd();
        }
    }
}

void QOpenGLPaintEngine::drawPoints(const QPointF *points, int pointCount)
{
    Q_D(QOpenGLPaintEngine);
    GLfloat pen_width = d->cpen.widthF();
    if (pen_width > 1 || (pen_width > 0 && d->txop > QPainterPrivate::TxTranslate)) {
        const QPointF *end = points + pointCount;
        while (points < end) {
            QPainterPath path;
            path.moveTo(*points);
            path.lineTo(points->x() + 0.001, points->y());
            drawPath(path);
            ++points;
        }
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

    GLfloat pen_width = d->cpen.widthF();
    if (pen_width > 1 || (pen_width > 0 && d->txop > QPainterPrivate::TxTranslate)) {
        QPainterPath path(lines[0].p1());
        path.lineTo(lines[0].p2());
        for (int i = 1; i < lineCount; ++lines) {
            path.lineTo(lines[i].p1());
            path.lineTo(lines[i].p2());
        }
        drawPath(path);
        return;
    }
    glColor4ubv(d->pen_color);
    glBegin(GL_LINES);
    {
        for (int i = 0; i < lineCount; ++i) {
            glVertex2d(lines[i].x1(), lines[i].y1());
            glVertex2d(lines[i].x2(), lines[i].y2());
        }
    }
    glEnd();
}

// Need to allocate space for new vertices on intersecting lines and
// they need to be alive until gluTessEndPolygon() has returned
static QList<GLdouble *> vertexStorage;
static void CALLBACK qgl_tess_combine(GLdouble coords[3],
				      GLdouble *[4],
				      GLfloat [4], GLdouble **dataOut)
{
    GLdouble *vertex;
    vertex = (GLdouble *) malloc(3 * sizeof(GLdouble));
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

static void qgl_draw_poly(const QPointF *points, int pointCount, bool winding = false)
{
#ifndef QT_GL_NO_CONCAVE_POLYGONS
    GLUtesselator *qgl_tess = tessHandler()->qgl_tess;
    gluTessProperty(qgl_tess, GLU_TESS_WINDING_RULE,
                    winding ? GLU_TESS_WINDING_NONZERO : GLU_TESS_WINDING_ODD);
    QVarLengthArray<GLdouble> v(pointCount*3);
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
    gluTessBeginPolygon(qgl_tess, NULL);
    {
	gluTessBeginContour(qgl_tess);
	{
	    for (int i = 0; i < pointCount; ++i) {
		v[i*3] = points[i].x();
		v[i*3+1] = points[i].y();
		v[i*3+2] = 0.0;
		gluTessVertex(qgl_tess, &v[i*3], &v[i*3]);
	    }
	}
	gluTessEndContour(qgl_tess);
    }
    gluTessEndPolygon(qgl_tess);
    // clean up after the qgl_tess_combine callback
    for (int i=0; i < vertexStorage.size(); ++i)
	free(vertexStorage[i]);
    vertexStorage.clear();
#else
    glBegin(GL_POLYGON);
    {
        for (int i = 0; i < pointCount; ++i)
	    glVertex2d(points[i].x(), points[i].y());
    }
    glEnd();
#endif
}

void QOpenGLPaintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    Q_D(QOpenGLPaintEngine);
    if(!pointCount)
        return;
    glColor4ubv(d->brush_color);
    if (d->has_brush && mode != PolylineMode)
        qgl_draw_poly(points, pointCount, mode == QPaintEngine::WindingMode);
    if (d->has_pen) {
        glColor4ubv(d->pen_color);
        double x1 = points[0].x();
        double y1 = points[0].y();
        double x2 = points[pointCount - 1].x();
        double y2 = points[pointCount - 1].y();

        glBegin(GL_LINE_STRIP);
        {
            for (int i = 0; i < pointCount; ++i)
                glVertex2d(points[i].x(), points[i].y());
            if (mode != PolylineMode && !(x1 == x2 && y1 == y2))
                glVertex2d(x1, y1);
        }
        glEnd();
    }
}

void QOpenGLPaintEngine::drawPath(const QPainterPath &path)
{
    Q_D(QOpenGLPaintEngine);

    if (path.isEmpty())
        return;

    if (d->has_brush) {
        QPolygonF poly = path.toFillPolygon();
        bool had_pen = d->has_pen;
        QPen old_pen = d->cpen;
        d->has_pen = false;
        d->cpen.setStyle(Qt::NoPen);
        drawPolygon(poly.data(), poly.size(),
                    path.fillRule() == Qt::OddEvenFill ? OddEvenMode : WindingMode);
        d->has_pen = had_pen;
        d->cpen = old_pen;
    }

    if (d->has_pen) {
        QPainterPathStroker stroker;
        stroker.setDashPattern(d->cpen.dashPattern());
        stroker.setCapStyle(d->cpen.capStyle());
        stroker.setJoinStyle(d->cpen.joinStyle());
        stroker.setMiterLimit(d->cpen.miterLimit());
        QPainterPath stroke;
        qreal width = d->cpen.widthF();
        QPolygonF poly;
        if (width == 0) {
            stroker.setWidth(1);
            stroke = stroker.createStroke(path * d->matrix);
            if (stroke.isEmpty())
                return;
            poly = stroke.toFillPolygon();
        } else {
            stroker.setWidth(width);
            stroker.setCurveThreshold( 1 / (2 * 10 * d->matrix.m11() * d->matrix.m22()));
            stroke = stroker.createStroke(path);
            if (stroke.isEmpty())
                return;
            poly = stroke.toFillPolygon(d->matrix);
        }

        bool had_pen = d->has_pen;
        bool had_brush = d->has_brush;
        QBrush old_brush = d->cbrush;
        d->has_pen = false;
        d->has_brush = true;
        d->setGLBrush(d->cpen.brush().color());

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        drawPolygon(poly.data(), poly.size(), WindingMode);
        glPopMatrix();

        d->has_pen = had_pen;
        d->has_brush = had_brush;
        d->cbrush = old_brush;
        d->setGLBrush(d->cbrush.color());
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

void QOpenGLPaintEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
    Q_D(QOpenGLPaintEngine);
    glPushAttrib(GL_CURRENT_BIT);
    glDisable(GL_TEXTURE_1D);
    glDisable(GL_TEXTURE_GEN_S);
    qt_glUseProgramObjectARB(0);
    QImage img((int)textItem.width(),
               (int)(textItem.ascent() + textItem.descent()),
               QImage::Format_ARGB32_Premultiplied);
    img.fill(0);
    QPainter painter(&img);
    painter.setPen(d->cpen);
    painter.setBrush(d->cbrush);
    painter.drawTextItem(QPoint(0, (int)(textItem.ascent())), textItem);
    painter.end();
    drawImage(QRectF(p.x(), p.y()-(textItem.ascent()), img.width(), img.height()),
              img,
              QRectF(0, 0, img.width(), img.height()),
              Qt::AutoColor);
    glPopAttrib();
    if (d->cbrush.style() == Qt::ConicalGradientPattern)
        qt_glUseProgramObjectARB(d->grad_conical);
    else if (d->cbrush.style() == Qt::RadialGradientPattern)
        qt_glUseProgramObjectARB(d->grad_radial);
}

/* fill the array colorTable with the gradient */
static void mk_color_table(const QGradientStops& s, unsigned int *colorTable, int size)
{
    int pos = 0;
    qreal fpos = 0.0;
    qreal incr = 1.0 / qreal(size);
    QVector<unsigned int> colors(s.size());

    /* init the array of converted colors */
    for(int i=0;i<s.size();i++)
    	colors[i] = s[i].second.rgba();

    while(fpos<s.first().first)
    {
	colorTable[pos] = colors[0];
    	pos++;
	fpos += incr;
    }

    for(int i=0;i<s.size()-1;i++)
    {
	/* do things intelligently, only one division per stop */
	qreal delta = 1/(s[i+1].first - s[i].first);

	while(fpos<s[i+1].first && pos<size)
	{
	    int dist = int(255*((fpos-s[i].first)*delta));
	    int idist = 255 - dist;
	    colorTable[pos] = INTERPOLATE_PIXEL_256(colors[i],
                                                    idist, colors[i+1], dist);
	    pos++;
	    fpos += incr;
	}
    }

    for(;pos<size;pos++)
	colorTable[pos] = colors[s.size()-1];
}

/* load the gradient as the current 1D texture */
static void mk_texture(const QGradient& g)
{
#define PAL_SIZE 1024
    unsigned int palbuf[PAL_SIZE];
    mk_color_table(g.stops(),palbuf,PAL_SIZE);

    if(g.spread() == QGradient::RepeatSpread || g.type() == QGradient::ConicalGradient)
        glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    else if(g.spread() == QGradient::ReflectSpread)
        glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_WRAP_S,GL_MIRRORED_REPEAT_IBM);
    /* use CLAMP_TO_BORDER if possible */
    else if(QGLExtensions::glExtensions & QGLExtensions::ClampToBorder)
        glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_BORDER_SGIS);
    else
        glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE_SGIS);

    /* mipmaps do not work well with conical gradient, and are probably not worth */
    if(! (QGLExtensions::glExtensions & QGLExtensions::GenerateMipmap) )
        glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    else if( (g.type() == QGradient::ConicalGradient) ||
             ( (g.spread() == QGradient::PadSpread) &&
               !(QGLExtensions::glExtensions & QGLExtensions::ClampToBorder)))
    {
        /* if we are using CLAMP_TO_EDGE (in place of CLAMP_TO_BORDER)
           disable mipmaps for pad gradients, or else they will look weird */
        glTexParameteri(GL_TEXTURE_1D,GL_GENERATE_MIPMAP_SGIS,GL_FALSE);
        glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_1D,GL_GENERATE_MIPMAP_SGIS,GL_TRUE);
        glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    }
    glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA,PAL_SIZE,0,GL_BGRA,GL_UNSIGNED_BYTE,palbuf);


}

