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

#include "qgl.h"
#include "qgl_p.h"

#include "qmap.h"
#include "qapplication.h"
#include "qdesktopwidget.h"
#include "qpixmap.h"
#include "qhash.h"
#include <private/qfontengine_p.h>
#include <private/qt_x11_p.h>

#define INT8  dummy_INT8
#define INT32 dummy_INT32
#include <GL/glx.h>
#undef  INT8
#undef  INT32
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/Xmu/StdCmap.h>

// POSIX Large File Support redefines truncate -> truncate64
#if defined(truncate)
# undef truncate
#endif

extern Drawable qt_x11Handle(const QPaintDevice *pd);
extern const QX11Info *qt_x11Info(const QPaintDevice *pd);

/*
  The choose_cmap function is internal and used by QGLWidget::setContext()
  and GLX (not Windows).  If the application can't find any sharable
  colormaps, it must at least create as few colormaps as possible.  The
  dictionary solution below ensures only one colormap is created per visual.
  Colormaps are also deleted when the application terminates.
*/

struct CMapEntry {
    CMapEntry();
    ~CMapEntry();

    Colormap                cmap;
    bool                alloc;
    XStandardColormap        scmap;
};

CMapEntry::CMapEntry()
{
    cmap = 0;
    alloc = false;
    scmap.colormap = 0;
}

CMapEntry::~CMapEntry()
{
    if (alloc)
        XFreeColormap(X11->display, cmap);
}


typedef QHash<int, CMapEntry *> CMapEntryHash;
Q_GLOBAL_STATIC(CMapEntryHash, cmap_hash)

static bool mesa_gl = false;
static bool first_time = true;

typedef QHash<int, QMap<int, QRgb> > GLCMapHash;
Q_GLOBAL_STATIC(GLCMapHash, qglcmap_hash)

static void cleanup_cmaps()
{
    CMapEntryHash *hash = cmap_hash();
    QHash<int, CMapEntry *>::ConstIterator it = hash->constBegin();
    while (it != hash->constEnd()) {
        delete it.value();
        ++it;
    }
    hash->clear();
}

static Colormap choose_cmap(Display *dpy, XVisualInfo *vi)
{
    if (first_time) {
        const char *v = glXQueryServerString(dpy, vi->screen, GLX_VERSION);
        if (v)
            mesa_gl = (strstr(v, "Mesa") != 0);
        qAddPostRoutine(cleanup_cmaps);
        first_time = false;
    }

    CMapEntryHash *hash = cmap_hash();
    CMapEntryHash::ConstIterator it = hash->find((long) vi->visualid + (vi->screen * 256));
    if (it != hash->constEnd())
        return it.value()->cmap; // found colormap for visual

    CMapEntry *x = new CMapEntry();

    XStandardColormap *c;
    int n, i;

    // qDebug("Choosing cmap for vID %0x", vi->visualid);

    if (vi->visualid ==
         XVisualIDFromVisual((Visual *) QX11Info::appVisual(vi->screen))) {
        // qDebug("Using x11AppColormap");
        return QX11Info::appColormap(vi->screen);
    }

    if (mesa_gl) {                                // we're using MesaGL
        Atom hp_cmaps = XInternAtom(dpy, "_HP_RGB_SMOOTH_MAP_LIST", true);
        if (hp_cmaps && vi->visual->c_class == TrueColor && vi->depth == 8) {
            if (XGetRGBColormaps(dpy,RootWindow(dpy,vi->screen),&c,&n,
                                  hp_cmaps)) {
                i = 0;
                while (i < n && x->cmap == 0) {
                    if (c[i].visualid == vi->visual->visualid) {
                        x->cmap = c[i].colormap;
                        x->scmap = c[i];
                        //qDebug("Using HP_RGB scmap");

                    }
                    i++;
                }
                XFree((char *)c);
            }
        }
    }
#if !defined(Q_OS_SOLARIS)
    if (!x->cmap) {
        if (XmuLookupStandardColormap(dpy,vi->screen,vi->visualid,vi->depth,
                                       XA_RGB_DEFAULT_MAP,false,true)) {
            if (XGetRGBColormaps(dpy,RootWindow(dpy,vi->screen),&c,&n,
                                  XA_RGB_DEFAULT_MAP)) {
                i = 0;
                while (i < n && x->cmap == 0) {
                    if (c[i].visualid == vi->visualid) {
                        x->cmap = c[i].colormap;
                        x->scmap = c[i];
                        //qDebug("Using RGB_DEFAULT scmap");
                    }
                    i++;
                }
                XFree((char *)c);
            }
        }
    }
#endif
    if (!x->cmap) {                                // no shared cmap found
        x->cmap = XCreateColormap(dpy, RootWindow(dpy,vi->screen), vi->visual,
                                   AllocNone);
        x->alloc = true;
        // qDebug("Allocating cmap");
    }

    // associate cmap with visualid
    hash->insert((long) vi->visualid + (vi->screen * 256), x);
    return x->cmap;
}

struct TransColor
{
    VisualID        vis;
    int                screen;
    long        color;
};

static QVector<TransColor> trans_colors;
static int trans_colors_init = false;


static void find_trans_colors()
{
    struct OverlayProp {
        long  visual;
        long  type;
        long  value;
        long  layer;
    };

    trans_colors_init = true;

    Display* appDisplay = X11->display;

    int scr;
    int lastsize = 0;
    for (scr = 0; scr < ScreenCount(appDisplay); scr++) {
        QWidget* rootWin = QApplication::desktop()->screen(scr);
        if (!rootWin)
            return;                                        // Should not happen
        Atom overlayVisualsAtom = XInternAtom(appDisplay,
                                               "SERVER_OVERLAY_VISUALS", True);
        if (overlayVisualsAtom == XNone)
            return;                                        // Server has no overlays

        Atom actualType;
        int actualFormat;
        ulong nItems;
        ulong bytesAfter;
        OverlayProp* overlayProps = 0;
        int res = XGetWindowProperty(appDisplay, rootWin->winId(),
                                      overlayVisualsAtom, 0, 10000, False,
                                      overlayVisualsAtom, &actualType,
                                      &actualFormat, &nItems, &bytesAfter,
                                      (uchar**)&overlayProps);

        if (res != Success || actualType != overlayVisualsAtom
             || actualFormat != 32 || nItems < 4 || !overlayProps)
            return;                                        // Error reading property

        int numProps = nItems / 4;
        trans_colors.resize(lastsize + numProps);
        int j = lastsize;
        for (int i = 0; i < numProps; i++) {
            if (overlayProps[i].type == 1) {
                trans_colors[j].vis = (VisualID)overlayProps[i].visual;
                trans_colors[j].screen = scr;
                trans_colors[j].color = (int)overlayProps[i].value;
                j++;
            }
        }
        XFree(overlayProps);
        lastsize = j;
        trans_colors.resize(lastsize);
    }
}


/*****************************************************************************
  QGLFormat UNIX/GLX-specific code
 *****************************************************************************/

bool QGLFormat::hasOpenGL()
{
    return glXQueryExtension(X11->display, 0, 0) != 0;
}


bool QGLFormat::hasOpenGLOverlays()
{
    if (!trans_colors_init)
        find_trans_colors();
    return trans_colors.size() > 0;
}

/*****************************************************************************
  QGLContext UNIX/GLX-specific code
 *****************************************************************************/

bool QGLContext::chooseContext(const QGLContext* shareContext)
{
    const QX11Info *xinfo = qt_x11Info(d->paintDevice);

    Display* disp = xinfo->display();
    vi = chooseVisual();
    if (!vi)
        return false;

    if (deviceIsPixmap() &&
         (((XVisualInfo*)vi)->depth != xinfo->depth() ||
          ((XVisualInfo*)vi)->screen != xinfo->screen()))
    {
        XFree(vi);
        XVisualInfo appVisInfo;
        memset(&appVisInfo, 0, sizeof(XVisualInfo));
        appVisInfo.visualid = XVisualIDFromVisual((Visual *) xinfo->visual());
        appVisInfo.screen = xinfo->screen();
        int nvis;
        vi = XGetVisualInfo(disp, VisualIDMask | VisualScreenMask, &appVisInfo, &nvis);
        if (!vi)
            return false;

        int useGL;
        glXGetConfig(disp, (XVisualInfo*)vi, GLX_USE_GL, &useGL);
        if (!useGL)
            return false;        //# Chickening out already...
    }
    int res;
    glXGetConfig(disp, (XVisualInfo*)vi, GLX_LEVEL, &res);
    glFormat.setPlane(res);
    glXGetConfig(disp, (XVisualInfo*)vi, GLX_DOUBLEBUFFER, &res);
    glFormat.setDoubleBuffer(res);
    glXGetConfig(disp, (XVisualInfo*)vi, GLX_DEPTH_SIZE, &res);
    glFormat.setDepth(res);
    glXGetConfig(disp, (XVisualInfo*)vi, GLX_RGBA, &res);
    glFormat.setRgba(res);
    glXGetConfig(disp, (XVisualInfo*)vi, GLX_ALPHA_SIZE, &res);
    glFormat.setAlpha(res);
    glXGetConfig(disp, (XVisualInfo*)vi, GLX_ACCUM_RED_SIZE, &res);
    glFormat.setAccum(res);
    glXGetConfig(disp, (XVisualInfo*)vi, GLX_STENCIL_SIZE, &res);
    glFormat.setStencil(res);
    glXGetConfig(disp, (XVisualInfo*)vi, GLX_STEREO, &res);
    glFormat.setStereo(res);

    Bool direct = format().directRendering() ? True : False;

    if (shareContext &&
         (!shareContext->isValid() || !shareContext->cx)) {
            qWarning("QGLContext::chooseContext(): Cannot share with invalid context");
            shareContext = 0;
    }

    // 1. Sharing between rgba and color-index will give wrong colors.
    // 2. Contexts cannot be shared btw. direct/non-direct renderers.
    // 3. Pixmaps cannot share contexts that are set up for direct rendering.
    if (shareContext && (format().rgba() != shareContext->format().rgba() ||
                          (deviceIsPixmap() &&
                           glXIsDirect(disp, (GLXContext)shareContext->cx))))
        shareContext = 0;

    cx = 0;
    if (shareContext) {
        cx = glXCreateContext(disp, (XVisualInfo *)vi,
                               (GLXContext)shareContext->cx, direct);
        if (cx)
            d->sharing = true;
    }
    if (!cx)
        cx = glXCreateContext(disp, (XVisualInfo *)vi, NULL, direct);
    if (!cx)
        return false;
    glFormat.setDirectRendering(glXIsDirect(disp, (GLXContext)cx));
    if (deviceIsPixmap()) {
#if defined(GLX_MESA_pixmap_colormap) && defined(QGL_USE_MESA_EXT)
        gpm = glXCreateGLXPixmapMESA(disp, (XVisualInfo *)vi,
                                      qt_x11Handle(d->paintDevice),
                                      choose_cmap(disp, (XVisualInfo *)vi));
#else
        gpm = (Q_UINT32)glXCreateGLXPixmap(disp, (XVisualInfo *)vi,
                                            qt_x11Handle(d->paintDevice));
#endif
        if (!gpm)
            return false;
    }
    return true;
}


/*!
  <strong>X11 only</strong>: This virtual function tries to find a
  visual that matches the format, reducing the demands if the original
  request cannot be met.

  The algorithm for reducing the demands of the format is quite
  simple-minded, so override this method in your subclass if your
  application has spcific requirements on visual selection.

  \sa chooseContext()
*/

void *QGLContext::chooseVisual()
{
    static int bufDepths[] = { 8, 4, 2, 1 };        // Try 16, 12 also?
    //todo: if pixmap, also make sure that vi->depth == pixmap->depth
    void* vis = 0;
    int i = 0;
    bool fail = false;
    QGLFormat fmt = format();
    bool tryDouble = !fmt.doubleBuffer();  // Some GL impl's only have double
    bool triedDouble = false;
    while(!fail && !(vis = tryVisual(fmt, bufDepths[i]))) {
        if (!fmt.rgba() && bufDepths[i] > 1) {
            i++;
            continue;
        }
        if (tryDouble) {
            fmt.setDoubleBuffer(true);
            tryDouble = false;
            triedDouble = true;
            continue;
        }
        else if (triedDouble) {
            fmt.setDoubleBuffer(false);
            triedDouble = false;
        }
        if (fmt.stereo()) {
            fmt.setStereo(false);
            continue;
        }
        if (fmt.accum()) {
            fmt.setAccum(false);
            continue;
        }
        if (fmt.stencil()) {
            fmt.setStencil(false);
            continue;
        }
        if (fmt.alpha()) {
            fmt.setAlpha(false);
            continue;
        }
        if (fmt.depth()) {
            fmt.setDepth(false);
            continue;
        }
        if (fmt.doubleBuffer()) {
            fmt.setDoubleBuffer(false);
            continue;
        }
        fail = true;
    }
    glFormat = fmt;
    return vis;
}


/*!

  \internal

  <strong>X11 only</strong>: This virtual function chooses a visual
  that matches the OpenGL \link format() format\endlink. Reimplement this
  function in a subclass if you need a custom visual.

  \sa chooseContext()
*/

void *QGLContext::tryVisual(const QGLFormat& f, int bufDepth)
{
    int spec[40];
    int i = 0;
    spec[i++] = GLX_LEVEL;
    spec[i++] = f.plane();
    const QX11Info *xinfo = qt_x11Info(d->paintDevice);

#if defined(GLX_VERSION_1_1) && defined(GLX_EXT_visual_info)
    static bool useTranspExt = false;
    static bool useTranspExtChecked = false;
    if (f.plane() && !useTranspExtChecked && d->paintDevice) {
        QByteArray estr(glXQueryExtensionsString(xinfo->display(), xinfo->screen()));
        useTranspExt = estr.contains("GLX_EXT_visual_info");
        //# (A bit simplistic; that could theoretically be a substring)
        if (useTranspExt) {
            QByteArray cstr(glXGetClientString(xinfo->display(), GLX_VENDOR));
            useTranspExt = !cstr.contains("Xi Graphics"); // bug workaround
            if (useTranspExt) {
                // bug workaround - some systems (eg. FireGL) refuses to return an overlay
                // visual if the GLX_TRANSPARENT_TYPE_EXT attribute is specfied, even if
                // the implementation supports transparent overlays
                int tmpSpec[] = { GLX_LEVEL, f.plane(), GLX_TRANSPARENT_TYPE_EXT,
                                  f.rgba() ? GLX_TRANSPARENT_RGB_EXT : GLX_TRANSPARENT_INDEX_EXT,
                                  XNone };
                XVisualInfo * vinf = glXChooseVisual(xinfo->display(), xinfo->screen(), tmpSpec);
                if (!vinf) {
                    useTranspExt = false;
                }
            }
        }

        useTranspExtChecked = true;
    }
    if (f.plane() && useTranspExt) {
        // Required to avoid non-transparent overlay visual(!) on some systems
        spec[i++] = GLX_TRANSPARENT_TYPE_EXT;
        spec[i++] = f.rgba() ? GLX_TRANSPARENT_RGB_EXT : GLX_TRANSPARENT_INDEX_EXT;
    }
#endif

    if (f.doubleBuffer())
        spec[i++] = GLX_DOUBLEBUFFER;
    if (f.depth()) {
        spec[i++] = GLX_DEPTH_SIZE;
        spec[i++] = 1;
    }
    if (f.stereo()) {
        spec[i++] = GLX_STEREO;
    }
    if (f.stencil()) {
        spec[i++] = GLX_STENCIL_SIZE;
        spec[i++] = 1;
    }
    if (f.rgba()) {
        spec[i++] = GLX_RGBA;
        spec[i++] = GLX_RED_SIZE;
        spec[i++] = 1;
        spec[i++] = GLX_GREEN_SIZE;
        spec[i++] = 1;
        spec[i++] = GLX_BLUE_SIZE;
        spec[i++] = 1;
        if (f.alpha()) {
            spec[i++] = GLX_ALPHA_SIZE;
            spec[i++] = 1;
        }
        if (f.accum()) {
            spec[i++] = GLX_ACCUM_RED_SIZE;
            spec[i++] = 1;
            spec[i++] = GLX_ACCUM_GREEN_SIZE;
            spec[i++] = 1;
            spec[i++] = GLX_ACCUM_BLUE_SIZE;
            spec[i++] = 1;
            if (f.alpha()) {
                spec[i++] = GLX_ACCUM_ALPHA_SIZE;
                spec[i++] = 1;
            }
        }
    }
    else {
        spec[i++] = GLX_BUFFER_SIZE;
        spec[i++] = bufDepth;
    }

    spec[i] = XNone;
    return glXChooseVisual(xinfo->display(), xinfo->screen(), spec);
}


void QGLContext::reset()
{
    if (!d->valid)
        return;
    const QX11Info *xinfo = qt_x11Info(d->paintDevice);
    doneCurrent();
    if (gpm)
        glXDestroyGLXPixmap(xinfo->display(), (GLXPixmap)gpm);
    gpm = 0;
    glXDestroyContext(xinfo->display(), (GLXContext)cx);
    if (vi)
        XFree(vi);
    vi = 0;
    cx = 0;
    d->crWin = false;
    d->sharing = false;
    d->valid = false;
    d->transpColor = QColor();
    d->initDone = false;
}


void QGLContext::makeCurrent()
{
    if (!d->valid) {
        qWarning("QGLContext::makeCurrent(): Cannot make invalid context current.");
        return;
    }
    const QX11Info *xinfo = qt_x11Info(d->paintDevice);
    bool ok = true;
    if (deviceIsPixmap())
        ok = glXMakeCurrent(xinfo->display(), (GLXPixmap)gpm, (GLXContext)cx);

    else
        ok = glXMakeCurrent(xinfo->display(), ((QWidget *)d->paintDevice)->winId(),
                             (GLXContext)cx);
    if (!ok)
        qWarning("QGLContext::makeCurrent(): Failed.");
    if (ok)
        currentCtx = this;
}

void QGLContext::doneCurrent()
{
    glXMakeCurrent(qt_x11Info(d->paintDevice)->display(), 0, 0);
    currentCtx = 0;
}


void QGLContext::swapBuffers() const
{
    if (!d->valid)
        return;
    if (!deviceIsPixmap())
        glXSwapBuffers(qt_x11Info(d->paintDevice)->display(),
		       static_cast<QWidget *>(d->paintDevice)->winId());
}

QColor QGLContext::overlayTransparentColor() const
{
    if (isValid()) {
        if (!trans_colors_init)
            find_trans_colors();

        VisualID myVisualId = ((XVisualInfo*)vi)->visualid;
        int myScreen = ((XVisualInfo*)vi)->screen;
        for (int i = 0; i < (int)trans_colors.size(); i++) {
            if (trans_colors[i].vis == myVisualId &&
                 trans_colors[i].screen == myScreen) {
                XColor col;
                col.pixel = trans_colors[i].color;
                col.red = col.green = col.blue = 0;
                col.flags = 0;
                Display *dpy = qt_x11Info(d->paintDevice)->display();
                if (col.pixel > (uint) ((XVisualInfo *)vi)->colormap_size - 1)
                    col.pixel = ((XVisualInfo *)vi)->colormap_size - 1;
                XQueryColor(dpy, choose_cmap(dpy, (XVisualInfo *) vi), &col);
                uchar r = (uchar)((col.red / 65535.0) * 255.0 + 0.5);
                uchar g = (uchar)((col.green / 65535.0) * 255.0 + 0.5);
                uchar b = (uchar)((col.blue / 65535.0) * 255.0 + 0.5);
                return QColor(qRgb(r,g,b));
            }
        }
    }
    return QColor();                // Invalid color
}


uint QGLContext::colorIndex(const QColor& c) const
{
    int screen = ((XVisualInfo *)vi)->screen;
    if (isValid()) {
        if (format().plane()
             && c.pixel(screen) == overlayTransparentColor().pixel(screen))
            return c.pixel(screen);                // Special; don't look-up
        if (((XVisualInfo*)vi)->visualid ==
             XVisualIDFromVisual((Visual *) QX11Info::appVisual(screen)))
            return c.pixel(screen);                // We're using QColor's cmap

        XVisualInfo *info = (XVisualInfo *) vi;
        CMapEntryHash *hash = cmap_hash();
        CMapEntryHash::ConstIterator it = hash->find((long) info->visualid + (info->screen * 256));
        CMapEntry *x = 0;
        if (it != hash->constEnd())
            x = it.value();
        if (x && !x->alloc) {                // It's a standard colormap
            int rf = (int)(((float)c.red() * (x->scmap.red_max+1))/256.0);
            int gf = (int)(((float)c.green() * (x->scmap.green_max+1))/256.0);
            int bf = (int)(((float)c.blue() * (x->scmap.blue_max+1))/256.0);
            uint p = x->scmap.base_pixel
                     + (rf * x->scmap.red_mult)
                     + (gf * x->scmap.green_mult)
                     + (bf * x->scmap.blue_mult);
            return p;
        } else {
            QMap<int, QRgb> &cmap = (*qglcmap_hash())[(long)info->visualid];

            // already in the map?
            QRgb target = c.rgb();
            QMap<int, QRgb>::Iterator it = cmap.begin();
            for (; it != cmap.end(); ++it) {
                if ((*it) == target)
                    return it.key();
            }

            // need to alloc color
            unsigned long plane_mask[2];
            unsigned long color_map_entry;
            if (!XAllocColorCells (QX11Info::display(), x->cmap, true, plane_mask, 0,
                                   &color_map_entry, 1))
                return c.pixel(screen);

            XColor col;
            col.flags = DoRed | DoGreen | DoBlue;
            col.pixel = color_map_entry;
            col.red   = (ushort)((qRed(c.rgb()) / 255.0) * 65535.0 + 0.5);
            col.green = (ushort)((qGreen(c.rgb()) / 255.0) * 65535.0 + 0.5);
            col.blue  = (ushort)((qBlue(c.rgb()) / 255.0) * 65535.0 + 0.5);
            XStoreColor(QX11Info::display(), x->cmap, &col);

            cmap.insert(color_map_entry, target);
            return color_map_entry;
        }
    }
    return 0;
}

#ifndef QT_NO_XFT
/*! \internal
    This is basically a substitute for glxUseXFont() which can only
    handle XLFD fonts. This version relies on XFT v2 to render the
    glyphs, but it works with all fonts that XFT2 provides - both
    antialiased and aliased bitmap and outline fonts.
*/
void qgl_use_font(QFontEngineXft *engine, int first, int count, int listBase)
{
    GLfloat color[4];
    glGetFloatv(GL_CURRENT_COLOR, color);

    // save the pixel unpack state
    GLint gl_swapbytes, gl_lsbfirst, gl_rowlength, gl_skiprows, gl_skippixels, gl_alignment;
    glGetIntegerv (GL_UNPACK_SWAP_BYTES, &gl_swapbytes);
    glGetIntegerv (GL_UNPACK_LSB_FIRST, &gl_lsbfirst);
    glGetIntegerv (GL_UNPACK_ROW_LENGTH, &gl_rowlength);
    glGetIntegerv (GL_UNPACK_SKIP_ROWS, &gl_skiprows);
    glGetIntegerv (GL_UNPACK_SKIP_PIXELS, &gl_skippixels);
    glGetIntegerv (GL_UNPACK_ALIGNMENT, &gl_alignment);

    glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
    glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    Bool antialiased = False;
    XftPatternGetBool(engine->pattern(), XFT_ANTIALIAS, 0, &antialiased);
    FT_Face face = engine->freetypeFace();

    // start generating font glyphs
    for (int i = first; i < count; ++i) {
        int list = listBase + i;
        GLfloat x0, y0, dx, dy;

        FT_Error err;

        err = FT_Load_Glyph(face, FT_Get_Char_Index(face, i), FT_LOAD_DEFAULT);
        if (err) {
            qDebug("failed loading glyph %d from font", i);
            Q_ASSERT(!err);
        }
        err = FT_Render_Glyph(face->glyph, (antialiased ? ft_render_mode_normal
                                            : ft_render_mode_mono));
        if (err) {
            qDebug("failed rendering glyph %d from font", i);
            Q_ASSERT(!err);
        }

        FT_Bitmap bm = face->glyph->bitmap;
        x0 = face->glyph->metrics.horiBearingX >> 6;
        y0 = (face->glyph->metrics.height - face->glyph->metrics.horiBearingY) >> 6;
        dx = face->glyph->metrics.horiAdvance >> 6;
        dy = 0;
        int sz = bm.pitch * bm.rows;
        uint *aa_glyph = 0;
        uchar *ua_glyph = 0;

        if (antialiased)
            aa_glyph = new uint[sz];
        else
            ua_glyph = new uchar[sz];

        // convert to GL format
        for (int y = 0; y < bm.rows; ++y) {
            for (int x = 0; x < bm.pitch; ++x) {
                int c1 = y*bm.pitch + x;
                int c2 = (bm.rows - y - 1) > 0 ? (bm.rows-y-1)*bm.pitch + x : x;
                if (antialiased) {
                    aa_glyph[c1] = (int(color[0]*255) << 24)
                                   | (int(color[1]*255) << 16)
                                   | (int(color[2]*255) << 8) | bm.buffer[c2];
                } else {
                    ua_glyph[c1] = bm.buffer[c2];
                }
            }
        }

        glNewList(list, GL_COMPILE);
        if (antialiased) {
            // calling glBitmap() is just a trick to move the current
            // raster pos, since glGet*() won't work in display lists
            glBitmap(0, 0, 0, 0, x0, -y0, 0);
            glDrawPixels(bm.pitch, bm.rows, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, aa_glyph);
            glBitmap(0, 0, 0, 0, dx-x0, y0, 0);
        } else {
            glBitmap(bm.pitch*8, bm.rows, -x0, y0, dx, dy, ua_glyph);
        }
        glEndList();
        antialiased ? delete[] aa_glyph : delete[] ua_glyph;
    }

    // restore pixel unpack settings
    glPixelStorei(GL_UNPACK_SWAP_BYTES, gl_swapbytes);
    glPixelStorei(GL_UNPACK_LSB_FIRST, gl_lsbfirst);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, gl_rowlength);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, gl_skiprows);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, gl_skippixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, gl_alignment);
}
#endif

void QGLContext::generateFontDisplayLists(const QFont & fnt, int listBase)
{
    QFont f(fnt);
    QFontEngine *engine = f.d->engineForScript(QFont::Latin);

#ifndef QT_NO_XFT
    if(engine->type() == QFontEngine::Xft) {
        qgl_use_font((QFontEngineXft *) engine, 0, 256, listBase);
        return;
    }
#endif
    // glXUseXFont() only works with XLFD font structures and a few GL
    // drivers crash if 0 is passed as the font handle
    f.setStyleStrategy(QFont::OpenGLCompatible);
    if (f.handle() && engine->type() == QFontEngine::XLFD)
        glXUseXFont((Font) f.handle(), 0, 256, listBase);
}

/*****************************************************************************
  QGLOverlayWidget (Internal overlay class for X11)
 *****************************************************************************/

#define d d_func()
#define q q_func()

class QGLOverlayWidget : public QGLWidget
{
    Q_OBJECT
public:
    QGLOverlayWidget(const QGLFormat& format, QGLWidget* parent,
                      const char* name=0, const QGLWidget* shareWidget=0);

protected:
    void                initializeGL();
    void                paintGL();
    void                resizeGL(int w, int h);

private:
    QGLWidget*                realWidget;

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QGLOverlayWidget(const QGLOverlayWidget&);
    QGLOverlayWidget&        operator=(const QGLOverlayWidget&);
#endif
};


QGLOverlayWidget::QGLOverlayWidget(const QGLFormat& format, QGLWidget* parent,
                                    const char* name,
                                    const QGLWidget* shareWidget)
    : QGLWidget(format, parent, name, shareWidget ? shareWidget->d->olw : 0)
{
    realWidget = parent;
}



void QGLOverlayWidget::initializeGL()
{
    QColor transparentColor = context()->overlayTransparentColor();
    if (transparentColor.isValid())
        qglClearColor(transparentColor);
    else
        qWarning("QGLOverlayWidget::initializeGL(): Could not get transparent color");
    realWidget->initializeOverlayGL();
}


void QGLOverlayWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    realWidget->resizeOverlayGL(w, h);
}


void QGLOverlayWidget::paintGL()
{
    realWidget->paintOverlayGL();
}

#undef Bool
#include "qgl_x11.moc"

/*****************************************************************************
  QGLWidget UNIX/GLX-specific code
 *****************************************************************************/
void QGLWidget::init(QGLContext *context, const QGLWidget *shareWidget)
{
    d->glcx = 0;
    d->olw = 0;
    d->autoSwap = true;
    if (!context->device())
        context->setDevice(this);

    if (shareWidget)
        setContext(context, shareWidget->context());
    else
        setContext(context);
    setAttribute(Qt::WA_NoSystemBackground, true);

    if (isValid() && context->format().hasOverlay()) {
        QString olwName = objectName();
        olwName += "-QGL_internal_overlay_widget";
        d->olw = new QGLOverlayWidget(QGLFormat::defaultOverlayFormat(),
                                       this, 0, shareWidget);
        d->olw->setObjectName(olwName);
        if (d->olw->isValid()) {
            d->olw->setAutoBufferSwap(false);
            d->olw->setFocusProxy(this);
        }
        else {
            delete d->olw;
            d->olw = 0;
            d->glcx->glFormat.setOverlay(false);
        }
    }
}

/*! \reimp */
bool QGLWidget::event(QEvent *e)
{
    return QWidget::event(e);
}


void QGLWidget::setMouseTracking(bool enable)
{
    if (d->olw)
        d->olw->setMouseTracking(enable);
    QWidget::setMouseTracking(enable);
}


void QGLWidget::resizeEvent(QResizeEvent *)
{
    if (!isValid())
        return;
    makeCurrent();
    if (!d->glcx->initialized())
        glInit();
    glXWaitX();
    resizeGL(width(), height());
    if (d->olw)
        d->olw->setGeometry(rect());
}

const QGLContext* QGLWidget::overlayContext() const
{
    if (d->olw)
        return d->olw->context();
    else
        return 0;
}


void QGLWidget::makeOverlayCurrent()
{
    if (d->olw)
        d->olw->makeCurrent();
}


void QGLWidget::updateOverlayGL()
{
    if (d->olw)
        d->olw->updateGL();
}

void QGLWidget::setContext(QGLContext *context,
                            const QGLContext* shareContext,
                            bool deleteOldContext)
{
    if (context == 0) {
        qWarning("QGLWidget::setContext: Cannot set null context");
        return;
    }
    if (!context->deviceIsPixmap() && context->device() != this) {
        qWarning("QGLWidget::setContext: Context must refer to this widget");
        return;
    }

    if (d->glcx)
        d->glcx->doneCurrent();
    QGLContext* oldcx = d->glcx;
    d->glcx = context;

    bool createFailed = false;
    if (!d->glcx->isValid()) {
        if (!d->glcx->create(shareContext ? shareContext : oldcx))
            createFailed = true;
    }
    if (createFailed) {
        if (deleteOldContext)
            delete oldcx;
        return;
    }

    if (d->glcx->windowCreated() || d->glcx->deviceIsPixmap()) {
        if (deleteOldContext)
            delete oldcx;
        return;
    }

    bool visible = isVisible();
    if (visible)
        hide();

    XVisualInfo *vi = (XVisualInfo*)d->glcx->vi;
    XSetWindowAttributes a;

    a.colormap = choose_cmap(QX11Info::display(), vi);        // find best colormap
    a.background_pixel = palette().color(backgroundRole()).pixel(vi->screen);
    a.border_pixel = QColor(Qt::black).pixel(vi->screen);
    Window p = RootWindow(X11->display, vi->screen);
    if (parentWidget())
        p = parentWidget()->winId();

    Window w = XCreateWindow(X11->display, p,  x(), y(), width(), height(),
                              0, vi->depth, InputOutput,  vi->visual,
                              CWBackPixel|CWBorderPixel|CWColormap, &a);

    Window *cmw;
    Window *cmwret;
    int count;
    if (XGetWMColormapWindows(X11->display, topLevelWidget()->winId(),
                                &cmwret, &count)) {
        cmw = new Window[count+1];
        memcpy((char *)cmw, (char *)cmwret, sizeof(Window)*count);
        XFree((char *)cmwret);
        int i;
        for (i=0; i<count; i++) {
            if (cmw[i] == winId()) {                // replace old window
                cmw[i] = w;
                break;
            }
        }
        if (i >= count)                        // append new window
            cmw[count++] = w;
    } else {
        count = 1;
        cmw = new Window[count];
        cmw[0] = w;
    }

#if defined(GLX_MESA_release_buffers) && defined(QGL_USE_MESA_EXT)
    if (oldcx && oldcx->windowCreated())
        glXReleaseBuffersMESA(X11->display, winId());
#endif
    if (deleteOldContext)
        delete oldcx;
    oldcx = 0;

    create(w);

    XSetWMColormapWindows(X11->display, topLevelWidget()->winId(), cmw,
                           count);
    delete [] cmw;

    // calling QWidget::create() will always result in a new paint
    // engine being created - get rid of it and replace it with our
    // own

    if (visible)
        show();
    XFlush(X11->display);
    d->glcx->setWindowCreated(true);
}


bool QGLWidget::renderCxPm(QPixmap* pm)
{
    if (((XVisualInfo*)d->glcx->vi)->depth != pm->depth())
        return false;

    GLXPixmap glPm;
#if defined(GLX_MESA_pixmap_colormap) && defined(QGL_USE_MESA_EXT)
    glPm = glXCreateGLXPixmapMESA(X11->display,
                                   (XVisualInfo*)d->glcx->vi,
                                   (Pixmap)pm->handle(),
                                   choose_cmap(pm->X11->display,
                                                (XVisualInfo*)d->glcx->vi));
#else
    glPm = (Q_UINT32)glXCreateGLXPixmap(X11->display,
                                         (XVisualInfo*)d->glcx->vi,
                                         (Pixmap)pm->handle());
#endif

    if (!glXMakeCurrent(X11->display, glPm, (GLXContext)d->glcx->cx)) {
        glXDestroyGLXPixmap(X11->display, glPm);
        return false;
    }

    glDrawBuffer(GL_FRONT);
    if (!d->glcx->initialized())
        glInit();
    resizeGL(pm->width(), pm->height());
    paintGL();
    glFlush();
    makeCurrent();
    glXDestroyGLXPixmap(X11->display, glPm);
    resizeGL(width(), height());
    return true;
}

const QGLColormap & QGLWidget::colormap() const
{
    return d->cmap;
}

/*\internal
  Store color values in the given colormap.
*/
static void qStoreColors(QWidget * tlw, Colormap cmap,
                          const QGLColormap & cols)
{
    Q_UNUSED(tlw);
    XColor c;
    QRgb color;

    for (int i = 0; i < cols.size(); i++) {
        color = cols.entryRgb(i);
        c.pixel = i;
        c.red   = (ushort)((qRed(color) / 255.0) * 65535.0 + 0.5);
        c.green = (ushort)((qGreen(color) / 255.0) * 65535.0 + 0.5);
        c.blue  = (ushort)((qBlue(color) / 255.0) * 65535.0 + 0.5);
        c.flags = DoRed | DoGreen | DoBlue;
        XStoreColor(X11->display, cmap, &c);
    }
}

/*\internal
  Check whether the given visual supports dynamic colormaps or not.
*/
static bool qCanAllocColors(QWidget * w)
{
    bool validVisual = false;
    int  numVisuals;
    long mask;
    XVisualInfo templ;
    XVisualInfo * visuals;
    VisualID id = XVisualIDFromVisual((Visual *) w->topLevelWidget()->x11Info().visual());

    mask = VisualScreenMask;
    templ.screen = w->x11Info().screen();
    visuals = XGetVisualInfo(X11->display, mask, &templ, &numVisuals);

    for (int i = 0; i < numVisuals; i++) {
        if (visuals[i].visualid == id) {
            switch (visuals[i].c_class) {
                case TrueColor:
                case StaticColor:
                case StaticGray:
                case XGrayScale:
                    validVisual = false;
                    break;
                case DirectColor:
                case PseudoColor:
                    validVisual = true;
                    break;
            }
            break;
        }
    }
    XFree(visuals);

    if (!validVisual)
        return false;
    return true;
}


void QGLWidget::setColormap(const QGLColormap & c)
{
    QWidget * tlw = topLevelWidget(); // must return a valid widget

    d->cmap = c;
    if (!d->cmap.handle())
        return;

    if (!qCanAllocColors(this)) {
        qWarning("QGLWidget::setColormap: Cannot create a read/write "
                  "colormap for this visual");
        return;
    }

    // If the child GL widget is not of the same visual class as the
    // toplevel widget we will get in trouble..
    Window wid = tlw->winId();
    Visual * vis = (Visual *) tlw->x11Info().visual();;
    VisualID cvId = XVisualIDFromVisual((Visual *) x11Info().visual());
    VisualID tvId = XVisualIDFromVisual((Visual *) tlw->x11Info().visual());
    if (cvId != tvId) {
        wid = winId();
        vis = (Visual *) x11Info().visual();
    }

    if (!d->cmap.handle()) // allocate a cmap if necessary
        d->cmap.setHandle(XCreateColormap(X11->display, wid, vis, AllocAll));

    qStoreColors(this, (Colormap) d->cmap.handle(), c);
    XSetWindowColormap(X11->display, wid, (Colormap) d->cmap.handle());

    // tell the wm that this window has a special colormap
    Window * cmw;
    Window * cmwret;
    int count;
    if (XGetWMColormapWindows(X11->display, tlw->winId(), &cmwret, &count))
    {
        cmw = new Window[count+1];
        memcpy((char *) cmw, (char *) cmwret, sizeof(Window) * count);
        XFree((char *) cmwret);
        int i;
        for (i = 0; i < count; i++) {
            if (cmw[i] == winId()) {
                break;
            }
        }
        if (i >= count)   // append new window only if not in the list
            cmw[count++] = winId();
    } else {
        count = 1;
        cmw = new Window[count];
        cmw[0] = winId();
    }
    XSetWMColormapWindows(X11->display, tlw->winId(), cmw, count);
    delete [] cmw;
}

/*! \internal
  Free up any allocated colormaps. This fn is only called for
  top-level widgets.
*/
void QGLWidget::cleanupColormaps()
{
    if (!d->cmap.handle()) {
        return;
    } else {
        XFreeColormap(X11->display, (Colormap) d->cmap.handle());
        d->cmap.setHandle(0);
    }
}

