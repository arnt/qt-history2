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

#include <qglpixelbuffer.h>
#include <qgl.h>
#include <private/qgl_p.h>

#include <qglpixelbuffer_p.h>

#include <qimage.h>
#include <qdebug.h>

/* WGL_WGLEXT_PROTOTYPES */
typedef const char * (WINAPI * PFNWGLGETEXTENSIONSSTRINGARBPROC) (HDC hdc);
typedef HPBUFFERARB (WINAPI * PFNWGLCREATEPBUFFERARBPROC) (HDC hDC, int iPixelFormat, int iWidth, int iHeight, const int *piAttribList);
typedef HDC (WINAPI * PFNWGLGETPBUFFERDCARBPROC) (HPBUFFERARB hPbuffer);
typedef int (WINAPI * PFNWGLRELEASEPBUFFERDCARBPROC) (HPBUFFERARB hPbuffer, HDC hDC);
typedef BOOL (WINAPI * PFNWGLDESTROYPBUFFERARBPROC) (HPBUFFERARB hPbuffer);
typedef BOOL (WINAPI * PFNWGLQUERYPBUFFERARBPROC) (HPBUFFERARB hPbuffer, int iAttribute, int *piValue);
typedef BOOL (WINAPI * PFNWGLGETPIXELFORMATATTRIBIVARBPROC) (HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, int *piValues);
typedef BOOL (WINAPI * PFNWGLGETPIXELFORMATATTRIBFVARBPROC) (HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, FLOAT *pfValues);
typedef BOOL (WINAPI * PFNWGLCHOOSEPIXELFORMATARBPROC) (HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
typedef BOOL (WINAPI * PFNWGLBINDTEXIMAGEARBPROC) (HPBUFFERARB hPbuffer, int iBuffer);
typedef BOOL (WINAPI * PFNWGLRELEASETEXIMAGEARBPROC) (HPBUFFERARB hPbuffer, int iBuffer);
typedef BOOL (WINAPI * PFNWGLSETPBUFFERATTRIBARBPROC) (HPBUFFERARB hPbuffer, const int * piAttribList);

PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB = 0;
PFNWGLCREATEPBUFFERARBPROC wglCreatePbufferARB = 0;
PFNWGLGETPBUFFERDCARBPROC wglGetPbufferDCARB = 0;
PFNWGLRELEASEPBUFFERDCARBPROC wglReleasePbufferDCARB = 0;
PFNWGLDESTROYPBUFFERARBPROC wglDestroyPbufferARB = 0;
PFNWGLQUERYPBUFFERARBPROC wglQueryPbufferARB = 0;
PFNWGLGETPIXELFORMATATTRIBIVARBPROC wglGetPixelFormatAttribivARB = 0;
PFNWGLGETPIXELFORMATATTRIBFVARBPROC wglGetPixelFormatAttribfvARB = 0;
PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = 0;
PFNWGLBINDTEXIMAGEARBPROC wglBindTexImageARB = 0;
PFNWGLRELEASETEXIMAGEARBPROC wglReleaseTexImageARB = 0;
PFNWGLSETPBUFFERATTRIBARBPROC wglSetPbufferAttribARB = 0;

#ifndef WGL_ARB_pbuffer
#define WGL_DRAW_TO_PBUFFER_ARB        0x202D
#define WGL_MAX_PBUFFER_PIXELS_ARB     0x202E
#define WGL_MAX_PBUFFER_WIDTH_ARB      0x202F
#define WGL_MAX_PBUFFER_HEIGHT_ARB     0x2030
#define WGL_PBUFFER_LARGEST_ARB        0x2033
#define WGL_PBUFFER_WIDTH_ARB          0x2034
#define WGL_PBUFFER_HEIGHT_ARB         0x2035
#define WGL_PBUFFER_LOST_ARB           0x2036
#endif

#ifndef WGL_ARB_pixel_format
#define WGL_NUMBER_PIXEL_FORMATS_ARB   0x2000
#define WGL_DRAW_TO_WINDOW_ARB         0x2001
#define WGL_DRAW_TO_BITMAP_ARB         0x2002
#define WGL_ACCELERATION_ARB           0x2003
#define WGL_NEED_PALETTE_ARB           0x2004
#define WGL_NEED_SYSTEM_PALETTE_ARB    0x2005
#define WGL_SWAP_LAYER_BUFFERS_ARB     0x2006
#define WGL_SWAP_METHOD_ARB            0x2007
#define WGL_NUMBER_OVERLAYS_ARB        0x2008
#define WGL_NUMBER_UNDERLAYS_ARB       0x2009
#define WGL_TRANSPARENT_ARB            0x200A
#define WGL_TRANSPARENT_RED_VALUE_ARB  0x2037
#define WGL_TRANSPARENT_GREEN_VALUE_ARB 0x2038
#define WGL_TRANSPARENT_BLUE_VALUE_ARB 0x2039
#define WGL_TRANSPARENT_ALPHA_VALUE_ARB 0x203A
#define WGL_TRANSPARENT_INDEX_VALUE_ARB 0x203B
#define WGL_SHARE_DEPTH_ARB            0x200C
#define WGL_SHARE_STENCIL_ARB          0x200D
#define WGL_SHARE_ACCUM_ARB            0x200E
#define WGL_SUPPORT_GDI_ARB            0x200F
#define WGL_SUPPORT_OPENGL_ARB         0x2010
#define WGL_DOUBLE_BUFFER_ARB          0x2011
#define WGL_STEREO_ARB                 0x2012
#define WGL_PIXEL_TYPE_ARB             0x2013
#define WGL_COLOR_BITS_ARB             0x2014
#define WGL_RED_BITS_ARB               0x2015
#define WGL_RED_SHIFT_ARB              0x2016
#define WGL_GREEN_BITS_ARB             0x2017
#define WGL_GREEN_SHIFT_ARB            0x2018
#define WGL_BLUE_BITS_ARB              0x2019
#define WGL_BLUE_SHIFT_ARB             0x201A
#define WGL_ALPHA_BITS_ARB             0x201B
#define WGL_ALPHA_SHIFT_ARB            0x201C
#define WGL_ACCUM_BITS_ARB             0x201D
#define WGL_ACCUM_RED_BITS_ARB         0x201E
#define WGL_ACCUM_GREEN_BITS_ARB       0x201F
#define WGL_ACCUM_BLUE_BITS_ARB        0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB       0x2021
#define WGL_DEPTH_BITS_ARB             0x2022
#define WGL_STENCIL_BITS_ARB           0x2023
#define WGL_AUX_BUFFERS_ARB            0x2024
#define WGL_NO_ACCELERATION_ARB        0x2025
#define WGL_GENERIC_ACCELERATION_ARB   0x2026
#define WGL_FULL_ACCELERATION_ARB      0x2027
#define WGL_SWAP_EXCHANGE_ARB          0x2028
#define WGL_SWAP_COPY_ARB              0x2029
#define WGL_SWAP_UNDEFINED_ARB         0x202A
#define WGL_TYPE_RGBA_ARB              0x202B
#define WGL_TYPE_COLORINDEX_ARB        0x202C
#endif

#ifndef WGL_ARB_render_texture
#define WGL_BIND_TO_TEXTURE_RGB_ARB        0x2070
#define WGL_BIND_TO_TEXTURE_RGBA_ARB       0x2071
#define WGL_TEXTURE_FORMAT_ARB             0x2072
#define WGL_TEXTURE_TARGET_ARB             0x2073
#define WGL_MIPMAP_TEXTURE_ARB             0x2074
#define WGL_TEXTURE_RGB_ARB                0x2075
#define WGL_TEXTURE_RGBA_ARB               0x2076
#define WGL_NO_TEXTURE_ARB                 0x2077
#define WGL_TEXTURE_CUBE_MAP_ARB           0x2078
#define WGL_TEXTURE_1D_ARB                 0x2079
#define WGL_TEXTURE_2D_ARB                 0x207A
#define WGL_MIPMAP_LEVEL_ARB               0x207B
#define WGL_CUBE_MAP_FACE_ARB              0x207C
#define WGL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB 0x207D
#define WGL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB 0x207E
#define WGL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB 0x207F
#define WGL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB 0x2080
#define WGL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB 0x2081
#define WGL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB 0x2082
#define WGL_FRONT_LEFT_ARB                 0x2083
#define WGL_FRONT_RIGHT_ARB                0x2084
#define WGL_BACK_LEFT_ARB                  0x2085
#define WGL_BACK_RIGHT_ARB                 0x2086
#define WGL_AUX0_ARB                       0x2087
#define WGL_AUX1_ARB                       0x2088
#define WGL_AUX2_ARB                       0x2089
#define WGL_AUX3_ARB                       0x208A
#define WGL_AUX4_ARB                       0x208B
#define WGL_AUX5_ARB                       0x208C
#define WGL_AUX6_ARB                       0x208D
#define WGL_AUX7_ARB                       0x208E
#define WGL_AUX8_ARB                       0x208F
#define WGL_AUX9_ARB                       0x2090
#endif

bool qt_resolve_pbuffer_extensions();
QGLFormat pfiToQGLFormat(HDC hdc, int pfi);

QGLPixelBuffer::QGLPixelBuffer(const QSize &size, const QGLFormat &f, QGLWidget *shareWidget)
    : d_ptr(new QGLPixelBufferPrivate)
{
    Q_D(QGLPixelBuffer);
    if (!qt_resolve_pbuffer_extensions())
        return;

    d->dc = GetDC(d->dmy.winId());
    Q_ASSERT(d->dc);

    int attribs[40];
    int i = 0;
    attribs[i++] = WGL_SUPPORT_OPENGL_ARB;
    attribs[i++] = TRUE;
    attribs[i++] = WGL_DRAW_TO_PBUFFER_ARB;
    attribs[i++] = TRUE;
    attribs[i++] = WGL_BIND_TO_TEXTURE_RGBA_ARB;
    attribs[i++] = TRUE;
    attribs[i++] = WGL_COLOR_BITS_ARB;
    attribs[i++] = 32;
    attribs[i++] = WGL_DOUBLE_BUFFER_ARB;
    attribs[i++] = FALSE;
    if (f.stereo()) {
	attribs[i++] = WGL_STEREO_ARB;
	attribs[i++] = TRUE;
    }
    if (f.depth()) {
	attribs[i++] = WGL_DEPTH_BITS_ARB;
	attribs[i++] = f.depthBufferSize() == -1 ? 24 : f.depthBufferSize();
    }
    if (f.alpha()) {
	attribs[i++] = WGL_ALPHA_BITS_ARB;
	attribs[i++] = f.alphaBufferSize() == -1 ? 8 : f.alphaBufferSize();
    }
    if (f.accum()) {
	attribs[i++] = WGL_ACCUM_BITS_ARB;
	attribs[i++] = f.accumBufferSize() == -1 ? 16 : f.accumBufferSize();
    }
    if (f.stencil()) {
	attribs[i++] = WGL_STENCIL_BITS_ARB;
	attribs[i++] = f.stencilBufferSize() == -1 ? 8 : f.stencilBufferSize();
    }
    // sample buffers doesn't work in conjunction with the render_texture extension
    // so igonre that for now
    // if (f.sampleBuffers()) {
    // 	   attribs[i++] = WGL_SAMPLE_BUFFERS_ARB;
    // 	   attribs[i++] = 1;
    // 	   attribs[i++] = WGL_SAMPLES_ARB;
    // 	   attribs[i++] = f.samples() == -1 ? 16 : f.samples();
    // }
    attribs[i] = 0;

    // Find pbuffer capable pixel format.
    unsigned int num_formats = 0;
    int pixel_format;
    wglChoosePixelFormatARB(d->dc, attribs, 0, 1, &pixel_format, &num_formats);
    if (num_formats == 0) {
	qWarning("QGLPixelBuffer: Unable to find a pixel format with pbuffer  - giving up.");
	ReleaseDC(d->dmy.winId(), d->dc);
	return;
    }
    d->format = pfiToQGLFormat(d->dc, pixel_format);

    // NB! The below ONLY works if the width/height are powers of 2.
    // Set some pBuffer attributes so that we can use this pBuffer as
    // a 2D RGBA texture target.
    int pb_attribs[] = {WGL_TEXTURE_FORMAT_ARB, WGL_TEXTURE_RGBA_ARB,
			WGL_TEXTURE_TARGET_ARB, WGL_TEXTURE_2D_ARB,
			0};

    d->pbuf = wglCreatePbufferARB(d->dc, pixel_format, size.width(), size.height(), pb_attribs);
    if(!d->pbuf) {
	qWarning("QGLPixelBuffer: Unable to create pbuffer [w=%d, h=%d] - giving up.", size.width(), size.height());
	ReleaseDC(d->dmy.winId(), d->dc);
	return;
    }

    ReleaseDC(d->dmy.winId(), d->dc);
    d->dc = wglGetPbufferDCARB(d->pbuf);
    d->ctx = wglCreateContext(d->dc);

    if (!d->dc || !d->ctx) {
	qWarning("QGLPixelBuffer: Unable to create pbuffer context - giving up.");
	return;
    }

    HGLRC share_ctx = shareWidget ? shareWidget->d_func()->glcx->d_func()->rc : 0;
    if (share_ctx && !wglShareLists(share_ctx, d->ctx))
        qWarning("QGLPixelBuffer: Unable to share display lists - with share widget.");

    int width, height;
    wglQueryPbufferARB(d->pbuf, WGL_PBUFFER_WIDTH_ARB, &width);
    wglQueryPbufferARB(d->pbuf, WGL_PBUFFER_HEIGHT_ARB, &height);
    d->size = QSize(width, height);
    d->invalid = false;
    d->qctx = new QGLContext(f);
    d->qctx->d_func()->sharing = (shareWidget != 0);
    d->qctx->d_func()->paintDevice = this;
}

QGLPixelBuffer::~QGLPixelBuffer()
{
    Q_D(QGLPixelBuffer);
    wglMakeCurrent(d->dc, 0);
    wglDeleteContext(d->ctx);
    if (!d->invalid) {
        wglReleasePbufferDCARB(d->pbuf, d->dc);
        wglDestroyPbufferARB(d->pbuf);
    }
    delete d->qctx;
    delete d_ptr;
}

bool QGLPixelBuffer::bindToDynamicTexture(GLuint texture_id)
{
    Q_D(QGLPixelBuffer);
    if (d->invalid)
	return false;
    glBindTexture(GL_TEXTURE_2D, texture_id);
    return wglBindTexImageARB(d->pbuf, WGL_FRONT_LEFT_ARB);
}

void QGLPixelBuffer::releaseFromDynamicTexture()
{
    Q_D(QGLPixelBuffer);
    if (d->invalid)
	return;
    wglReleaseTexImageARB(d->pbuf, WGL_FRONT_LEFT_ARB);
}

bool QGLPixelBuffer::makeCurrent()
{
    Q_D(QGLPixelBuffer);
    if (d->invalid)
        return false;
    return wglMakeCurrent(d->dc, d->ctx);
}

bool QGLPixelBuffer::doneCurrent()
{
    Q_D(QGLPixelBuffer);
    if (d->invalid)
        return false;
    return wglMakeCurrent(d->dc, 0);
}

bool QGLPixelBuffer::hasOpenGLPbuffers()
{
    return qt_resolve_pbuffer_extensions();
}

bool qt_resolve_pbuffer_extensions()
{
    static bool resolved = false;
    if (resolved && wglBindTexImageARB)
	return true;
    else if (resolved)
        return false;

    QGLWidget dummy;
    dummy.makeCurrent();

    wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)
				wglGetProcAddress("wglGetExtensionsStringARB");
    QString extensions;
    if (!wglGetExtensionsStringARB)
	return false;

    extensions = wglGetExtensionsStringARB(wglGetCurrentDC());

    if (!extensions.contains("WGL_ARB_pbuffer")
	|| !extensions.contains("WGL_ARB_render_texture")
	|| !extensions.contains("WGL_ARB_pixel_format")) {
        resolved = true;
	return false;
    }

    wglCreatePbufferARB = (PFNWGLCREATEPBUFFERARBPROC) wglGetProcAddress("wglCreatePbufferARB");
    wglGetPbufferDCARB = (PFNWGLGETPBUFFERDCARBPROC) wglGetProcAddress("wglGetPbufferDCARB");
    wglReleasePbufferDCARB = (PFNWGLRELEASEPBUFFERDCARBPROC) wglGetProcAddress("wglReleasePbufferDCARB");
    wglDestroyPbufferARB = (PFNWGLDESTROYPBUFFERARBPROC) wglGetProcAddress("wglDestroyPbufferARB");
    wglQueryPbufferARB = (PFNWGLQUERYPBUFFERARBPROC) wglGetProcAddress("wglQueryPbufferARB");

    wglGetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)
				   wglGetProcAddress("wglGetPixelFormatAttribivARB");
    wglGetPixelFormatAttribfvARB = (PFNWGLGETPIXELFORMATATTRIBFVARBPROC)
				   wglGetProcAddress("wglGetPixelFormatAttribfvARB");
    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)
			      wglGetProcAddress("wglChoosePixelFormatARB");


    wglBindTexImageARB = (PFNWGLBINDTEXIMAGEARBPROC) wglGetProcAddress("wglBindTexImageARB");
    wglReleaseTexImageARB = (PFNWGLRELEASETEXIMAGEARBPROC) wglGetProcAddress("wglReleaseTexImageARB");
    wglSetPbufferAttribARB = (PFNWGLSETPBUFFERATTRIBARBPROC) wglGetProcAddress("wglSetPbufferAttribARB");

    resolved = true;
    if (!wglBindTexImageARB)
        return false;
    return resolved;
}
