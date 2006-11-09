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

#ifndef QGL_P_H
#define QGL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include "QtOpenGL/qgl.h"
#include "QtOpenGL/qglcolormap.h"
#include "QtCore/qmap.h"
#include "QtCore/qthread.h"
#include "QtCore/qthreadstorage.h"
#include "QtCore/qhash.h"
#include "private/qwidget_p.h"

class QGLContext;
class QGLOverlayWidget;
class QPixmap;
#ifdef Q_WS_MAC
#include <AGL/agl.h>
class QMacWindowChangeEvent;
#endif

#ifdef Q_WS_QWS
#include <GLES/egl.h>
class QGLDirectPainter;
#endif

// extension prototypes
#ifndef Q_WS_MAC
# ifndef APIENTRYP
#   ifdef APIENTRY
#     define APIENTRYP APIENTRY *
#   else
#     define APIENTRY
#     define APIENTRYP *
#   endif
# endif
#else
# define APIENTRY
# define APIENTRYP *
#endif

// ARB_fragment_program
typedef void (APIENTRY *_glProgramStringARB) (GLenum, GLenum, GLsizei, const GLvoid *);
typedef void (APIENTRY *_glBindProgramARB) (GLenum, GLuint);
typedef void (APIENTRY *_glDeleteProgramsARB) (GLsizei, const GLuint *);
typedef void (APIENTRY *_glGenProgramsARB) (GLsizei, GLuint *);
typedef void (APIENTRY *_glProgramLocalParameter4fvARB) (GLenum, GLuint, const GLfloat *);

// GLSL
typedef GLuint (APIENTRY *_glCreateShader) (GLenum);
typedef void (APIENTRY *_glShaderSource) (GLuint, GLsizei, const char **, const GLint *);
typedef void (APIENTRY *_glCompileShader) (GLuint);
typedef void (APIENTRY *_glDeleteShader) (GLuint);

typedef GLuint (APIENTRY *_glCreateProgram) ();
typedef void (APIENTRY *_glAttachShader) (GLuint, GLuint);
typedef void (APIENTRY *_glDetachShader) (GLuint, GLuint);
typedef void (APIENTRY *_glLinkProgram) (GLuint);
typedef void (APIENTRY *_glUseProgram) (GLuint);
typedef void (APIENTRY *_glDeleteProgram) (GLuint);

typedef void (APIENTRY *_glGetShaderInfoLog) (GLuint, GLsizei, GLsizei *, char *);
typedef void (APIENTRY *_glGetProgramiv) (GLuint, GLenum, GLint *);

typedef GLuint (APIENTRY *_glGetUniformLocation) (GLuint, const char*);
typedef void (APIENTRY *_glUniform4fv) (GLint, GLsizei, GLfloat *);
typedef void (APIENTRY *_glUniform3fv) (GLint, GLsizei, GLfloat *);
typedef void (APIENTRY *_glUniform2fv) (GLint, GLsizei, GLfloat *);
typedef void (APIENTRY *_glUniform1fv) (GLint, GLsizei, GLfloat *);
typedef void (APIENTRY *_glUniform1i) (GLint, GLint);

typedef void (APIENTRY *_glActiveStencilFaceEXT) (GLenum );

typedef void (APIENTRY *_glMultiTexCoord4f) (GLenum, GLfloat, GLfloat, GLfloat, GLfloat);
typedef void (APIENTRY *_glActiveTexture) (GLenum);

// EXT_GL_framebuffer_object
typedef GLboolean (APIENTRYP PFNGLISRENDERBUFFEREXTPROC) (GLuint renderbuffer);
typedef void (APIENTRYP PFNGLBINDRENDERBUFFEREXTPROC) (GLenum target, GLuint renderbuffer);
typedef void (APIENTRYP PFNGLDELETERENDERBUFFERSEXTPROC) (GLsizei n, const GLuint *renderbuffers);
typedef void (APIENTRYP PFNGLGENRENDERBUFFERSEXTPROC) (GLsizei n, GLuint *renderbuffers);
typedef void (APIENTRYP PFNGLRENDERBUFFERSTORAGEEXTPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef GLboolean (APIENTRYP PFNGLISFRAMEBUFFEREXTPROC) (GLuint framebuffer);
typedef void (APIENTRYP PFNGLBINDFRAMEBUFFEREXTPROC) (GLenum target, GLuint framebuffer);
typedef void (APIENTRYP PFNGLDELETEFRAMEBUFFERSEXTPROC) (GLsizei n, const GLuint *framebuffers);
typedef void (APIENTRYP PFNGLGENFRAMEBUFFERSEXTPROC) (GLsizei n, GLuint *framebuffers);
typedef GLenum (APIENTRYP PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) (GLenum target);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE1DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget,
                                                           GLuint texture, GLint level);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget,
                                                           GLuint texture, GLint level);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE3DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget,
                                                           GLuint texture, GLint level, GLint zoffset);
typedef void (APIENTRYP PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) (GLenum target, GLenum attachment, GLenum renderbuffertarget,
                                                              GLuint renderbuffer);
typedef void (APIENTRYP PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC) (GLenum target, GLenum attachment, GLenum pname,
                                                                          GLint *params);
typedef void (APIENTRYP PFNGLGENERATEMIPMAPEXTPROC) (GLenum target);

class QGLFormatPrivate
{
public:
    QGLFormatPrivate() {
        opts = QGL::DoubleBuffer | QGL::DepthBuffer | QGL::Rgba | QGL::DirectRendering | QGL::StencilBuffer;
        pln = 0;
        depthSize = accumSize = stencilSize = redSize = greenSize = blueSize = alphaSize = -1;
        numSamples = -1;
        swapInterval = -1;
    }
    QGL::FormatOptions opts;
    int pln;
    int depthSize;
    int accumSize;
    int stencilSize;
    int redSize;
    int greenSize;
    int blueSize;
    int alphaSize;
    int numSamples;
    int swapInterval;
};

class QGLWidgetPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QGLWidget)
public:
    QGLWidgetPrivate() : QWidgetPrivate() {}
    ~QGLWidgetPrivate() {}

    void init(QGLContext *context, const QGLWidget* shareWidget);
    void initContext(QGLContext *context, const QGLWidget* shareWidget);
    bool renderCxPm(QPixmap *pixmap);
    void cleanupColormaps();

    QGLContext *glcx;
    bool autoSwap;

    QGLColormap cmap;
    QMap<QString, int> displayListCache;

#if defined(Q_WS_WIN)
    void updateColormap();
    QGLContext *olcx;
#elif defined(Q_WS_X11)
    QGLOverlayWidget *olw;
#elif defined(Q_WS_MAC)
    QGLContext *olcx;
    void updatePaintDevice();
#if (MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_5)
    QMacWindowChangeEvent *watcher;
#endif
#elif defined(Q_WS_QWS)
    QGLDirectPainter *directPainter;
    void resizeHandler(const QSize &);
    void render(const QRegion&);
#endif
};

class QGLContextPrivate
{
    Q_DECLARE_PUBLIC(QGLContext)
public:
    explicit QGLContextPrivate(QGLContext *context) : q_ptr(context)
    {
#ifdef Q_WS_WIN
        qt_glProgramStringARB = 0;
        qt_glBindProgramARB = 0;
        qt_glDeleteProgramsARB = 0;
        qt_glGenProgramsARB = 0;
        qt_glProgramLocalParameter4fvARB = 0;

        qt_glCreateShader = 0;
        qt_glShaderSource = 0;
        qt_glCompileShader = 0;
        qt_glDeleteShader = 0;

        qt_glCreateProgram = 0;
        qt_glAttachShader = 0;
        qt_glDetachShader = 0;
        qt_glLinkProgram = 0;
        qt_glUseProgram = 0;
        qt_glDeleteProgram = 0;

        qt_glGetShaderInfoLog = 0;
        qt_glGetProgramiv = 0;

        qt_glGetUniformLocation = 0;
        qt_glUniform4fv = 0;
        qt_glUniform3fv = 0;
        qt_glUniform2fv = 0;
        qt_glUniform1fv = 0;
        qt_glUniform1i = 0;

        qt_glActiveStencilFaceEXT = 0;

        qt_glMultiTexCoord4f = 0;
        qt_glActiveTexture = 0;

        qt_glIsRenderbufferEXT = 0;
        qt_glBindRenderbufferEXT = 0;
        qt_glDeleteRenderbuffersEXT = 0;
        qt_glGenRenderbuffersEXT = 0;
        qt_glRenderbufferStorageEXT = 0;
        qt_glGetRenderbufferParameterivEXT = 0;
        qt_glIsFramebufferEXT = 0;
        qt_glBindFramebufferEXT = 0;
        qt_glDeleteFramebuffersEXT = 0;
        qt_glGenFramebuffersEXT = 0;
        qt_glCheckFramebufferStatusEXT = 0;
        qt_glFramebufferTexture1DEXT = 0;
        qt_glFramebufferTexture2DEXT = 0;
        qt_glFramebufferTexture3DEXT = 0;
        qt_glFramebufferRenderbufferEXT = 0;
        qt_glGetFramebufferAttachmentParameterivEXT = 0;
        qt_glGenerateMipmapEXT = 0;
#endif
    }
    ~QGLContextPrivate() {}
    GLuint bindTexture(const QImage &image, GLenum target, GLint format, const QString &key,
                       qint64 qt_id, bool clean = false);
    GLuint bindTexture(const QPixmap &pixmap, GLenum target, GLint format, bool clean);
    GLuint bindTexture(const QImage &image, GLenum target, GLint format, bool clean);
    bool textureCacheLookup(const QString &key, GLuint *id, qint64 *qt_id);
    void init(QPaintDevice *dev, const QGLFormat &format);
    QImage convertToGLFormat(const QImage &image, bool force_premul, GLenum texture_format);

#if defined(Q_WS_WIN)
    HGLRC rc;
    HDC dc;
    WId        win;
    int pixelFormatId;
    QGLCmap* cmap;
    HBITMAP hbitmap;
    HDC hbitmap_hdc;
#elif defined(Q_WS_X11) || defined(Q_WS_MAC)
    void* vi;
    void* cx;
#if defined(Q_WS_X11)
    quint32 gpm;
#endif
#if defined(Q_WS_MAC)
    bool update;
    AGLPixelFormat tryFormat(const QGLFormat &format);
#endif
#elif defined(Q_WS_QWS)
    EGLDisplay dpy;
    EGLContext cx;
    EGLConfig  config;
    EGLSurface surface;
#endif
    QGLFormat glFormat;
    QGLFormat reqFormat;

    uint valid : 1;
    uint sharing : 1;
    uint initDone : 1;
    uint crWin : 1;
    QPaintDevice *paintDevice;
    QColor transpColor;
    QGLContext *q_ptr;

#ifdef Q_WS_WIN
    _glProgramStringARB qt_glProgramStringARB;
    _glBindProgramARB qt_glBindProgramARB;
    _glDeleteProgramsARB qt_glDeleteProgramsARB;
    _glGenProgramsARB qt_glGenProgramsARB;
    _glProgramLocalParameter4fvARB qt_glProgramLocalParameter4fvARB;

    // GLSL definitions
    _glCreateShader qt_glCreateShader;
    _glShaderSource qt_glShaderSource;
    _glCompileShader qt_glCompileShader;
    _glDeleteShader qt_glDeleteShader;

    _glCreateProgram qt_glCreateProgram;
    _glAttachShader qt_glAttachShader;
    _glDetachShader qt_glDetachShader;
    _glLinkProgram qt_glLinkProgram;
    _glUseProgram qt_glUseProgram;
    _glDeleteProgram qt_glDeleteProgram;

    _glGetShaderInfoLog qt_glGetShaderInfoLog;
    _glGetProgramiv qt_glGetProgramiv;

    _glGetUniformLocation qt_glGetUniformLocation;
    _glUniform4fv qt_glUniform4fv;
    _glUniform3fv qt_glUniform3fv;
    _glUniform2fv qt_glUniform2fv;
    _glUniform1fv qt_glUniform1fv;
    _glUniform1i qt_glUniform1i;

    _glActiveStencilFaceEXT qt_glActiveStencilFaceEXT;

    _glMultiTexCoord4f qt_glMultiTexCoord4f;
    _glActiveTexture qt_glActiveTexture;

    PFNGLISRENDERBUFFEREXTPROC qt_glIsRenderbufferEXT;
    PFNGLBINDRENDERBUFFEREXTPROC qt_glBindRenderbufferEXT;
    PFNGLDELETERENDERBUFFERSEXTPROC qt_glDeleteRenderbuffersEXT;
    PFNGLGENRENDERBUFFERSEXTPROC qt_glGenRenderbuffersEXT;
    PFNGLRENDERBUFFERSTORAGEEXTPROC qt_glRenderbufferStorageEXT;
    PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC qt_glGetRenderbufferParameterivEXT;
    PFNGLISFRAMEBUFFEREXTPROC qt_glIsFramebufferEXT;
    PFNGLBINDFRAMEBUFFEREXTPROC qt_glBindFramebufferEXT;
    PFNGLDELETEFRAMEBUFFERSEXTPROC qt_glDeleteFramebuffersEXT;
    PFNGLGENFRAMEBUFFERSEXTPROC qt_glGenFramebuffersEXT;
    PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC qt_glCheckFramebufferStatusEXT;
    PFNGLFRAMEBUFFERTEXTURE1DEXTPROC qt_glFramebufferTexture1DEXT;
    PFNGLFRAMEBUFFERTEXTURE2DEXTPROC qt_glFramebufferTexture2DEXT;
    PFNGLFRAMEBUFFERTEXTURE3DEXTPROC qt_glFramebufferTexture3DEXT;
    PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC qt_glFramebufferRenderbufferEXT;
    PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC qt_glGetFramebufferAttachmentParameterivEXT;
    PFNGLGENERATEMIPMAPEXTPROC qt_glGenerateMipmapEXT;
#endif // Q_WS_WIN
};

// GL extension definitions
class QGLExtensions {
public:
    enum Extension {
        TextureRectangle        = 0x00000001,
        SampleBuffers           = 0x00000002,
        GenerateMipmap          = 0x00000004,
        TextureCompression      = 0x00000008,
        FragmentProgram         = 0x00000010,
        MirroredRepeat          = 0x00000020,
        FramebufferObject       = 0x00000040,
        StencilTwoSide          = 0x00000080,
        StencilWrap             = 0x00000100,
        PackedDepthStencil      = 0x00000200
    };
    Q_DECLARE_FLAGS(Extensions, Extension)

    static Extensions glExtensions;
    static void init(); // sys dependent
    static void init_extensions(); // general: called by init()
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QGLExtensions::Extensions)

/* NV_texture_rectangle */
#ifndef GL_NV_texture_rectangle
#define GL_TEXTURE_RECTANGLE_NV           0x84F5
#define GL_TEXTURE_BINDING_RECTANGLE_NV   0x84F6
#define GL_PROXY_TEXTURE_RECTANGLE_NV     0x84F7
#define GL_MAX_RECTANGLE_TEXTURE_SIZE_NV  0x84F8
#endif

#ifndef GL_BGRA
#define GL_BGRA 0x80E1
#endif

struct QGLThreadContext {
    QGLContext *context;
};
extern QThreadStorage<QGLThreadContext *> qgl_context_storage;

typedef QMultiHash<const QGLContext *, const QGLContext *> QGLSharingHash;
class QGLShareRegister
{
public:
    QGLShareRegister() {}
    ~QGLShareRegister() { reg.clear(); }

    bool checkSharing(const QGLContext *context1, const QGLContext *context2, const QGLContext * skip=0) {
        if (context1 == context2)
            return true;
        QList<const QGLContext *> shares = reg.values(context1);
        for (int k=0; k<shares.size(); ++k) {
            const QGLContext *ctx = shares.at(k);
            if (ctx == skip) // avoid an indirect circular loop (infinite recursion)
                continue;
            if (ctx == context2)
                return true;
            if (checkSharing(ctx, context2, context1))
                return true;
        }
        return false;
    }

    void addShare(const QGLContext *context, const QGLContext *share) {
        reg.insert(context, share); // context sharing works both ways
        reg.insert(share, context);
    }

    void removeShare(const QGLContext *context) {
        QGLSharingHash::iterator it = reg.begin();
        while (it != reg.end()) {
            if (it.key() == context || it.value() == context)
                it = reg.erase(it);
            else
                ++it;
        }
    }

private:
    QGLSharingHash reg;
};

extern QGLShareRegister* qgl_share_reg();

// OpenGL constants
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

// Stencil wrap and two-side defines
#ifndef GL_STENCIL_TEST_TWO_SIDE_EXT
#define GL_STENCIL_TEST_TWO_SIDE_EXT 0x8910
#endif
#ifndef GL_INCR_WRAP_EXT
#define GL_INCR_WRAP_EXT 0x8507
#endif
#ifndef GL_DECR_WRAP_EXT
#define GL_DECR_WRAP_EXT 0x8508
#endif

#ifndef GL_TEXTURE0
#define GL_TEXTURE0 0x84C0
#endif

#ifndef GL_TEXTURE1
#define GL_TEXTURE1 0x84C1
#endif

#ifndef GL_EXT_framebuffer_object
#define GL_INVALID_FRAMEBUFFER_OPERATION_EXT                    0x0506
#define GL_MAX_RENDERBUFFER_SIZE_EXT                            0x84E8
#define GL_FRAMEBUFFER_BINDING_EXT                              0x8CA6
#define GL_RENDERBUFFER_BINDING_EXT                             0x8CA7
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT               0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT               0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT             0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT     0x8CD3
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT        0x8CD4
#define GL_FRAMEBUFFER_COMPLETE_EXT                             0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT                0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT        0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT      0x8CD8
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT                0x8CD9
#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT                   0x8CDA
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT               0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT               0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED_EXT                          0x8CDD
#define GL_MAX_COLOR_ATTACHMENTS_EXT                            0x8CDF
#define GL_COLOR_ATTACHMENT0_EXT                                0x8CE0
#define GL_COLOR_ATTACHMENT1_EXT                                0x8CE1
#define GL_COLOR_ATTACHMENT2_EXT                                0x8CE2
#define GL_COLOR_ATTACHMENT3_EXT                                0x8CE3
#define GL_COLOR_ATTACHMENT4_EXT                                0x8CE4
#define GL_COLOR_ATTACHMENT5_EXT                                0x8CE5
#define GL_COLOR_ATTACHMENT6_EXT                                0x8CE6
#define GL_COLOR_ATTACHMENT7_EXT                                0x8CE7
#define GL_COLOR_ATTACHMENT8_EXT                                0x8CE8
#define GL_COLOR_ATTACHMENT9_EXT                                0x8CE9
#define GL_COLOR_ATTACHMENT10_EXT                               0x8CEA
#define GL_COLOR_ATTACHMENT11_EXT                               0x8CEB
#define GL_COLOR_ATTACHMENT12_EXT                               0x8CEC
#define GL_COLOR_ATTACHMENT13_EXT                               0x8CED
#define GL_COLOR_ATTACHMENT14_EXT                               0x8CEE
#define GL_COLOR_ATTACHMENT15_EXT                               0x8CEF
#define GL_DEPTH_ATTACHMENT_EXT                                 0x8D00
#define GL_STENCIL_ATTACHMENT_EXT                               0x8D20
#define GL_FRAMEBUFFER_EXT                                      0x8D40
#define GL_RENDERBUFFER_EXT                                     0x8D41
#define GL_RENDERBUFFER_WIDTH_EXT                               0x8D42
#define GL_RENDERBUFFER_HEIGHT_EXT                              0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT_EXT                     0x8D44
#define GL_STENCIL_INDEX_EXT                                    0x8D45
#define GL_STENCIL_INDEX1_EXT                                   0x8D46
#define GL_STENCIL_INDEX4_EXT                                   0x8D47
#define GL_STENCIL_INDEX8_EXT                                   0x8D48
#define GL_STENCIL_INDEX16_EXT                                  0x8D49
#define GL_RENDERBUFFER_RED_SIZE_EXT                            0x8D50
#define GL_RENDERBUFFER_GREEN_SIZE_EXT                          0x8D51
#define GL_RENDERBUFFER_BLUE_SIZE_EXT                           0x8D52
#define GL_RENDERBUFFER_ALPHA_SIZE_EXT                          0x8D53
#define GL_RENDERBUFFER_DEPTH_SIZE_EXT                          0x8D54
#define GL_RENDERBUFFER_STENCIL_SIZE_EXT                        0x8D55
#endif

#ifndef GL_EXT_packed_depth_stencil
#define GL_DEPTH_STENCIL_EXT                                    0x84F9
#define GL_UNSIGNED_INT_24_8_EXT                                0x84FA
#define GL_DEPTH24_STENCIL8_EXT                                 0x88F0
#define GL_TEXTURE_STENCIL_SIZE_EXT                             0x88F1
#endif

// ### hm. should be part of the GL 1.2 spec..
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE                  0x812F
#endif

#if defined(Q_WS_X11) || defined(Q_WS_MAC) || defined(Q_WS_QWS)

extern _glProgramStringARB qt_glProgramStringARB;
extern _glBindProgramARB qt_glBindProgramARB;
extern _glDeleteProgramsARB qt_glDeleteProgramsARB;
extern _glGenProgramsARB qt_glGenProgramsARB;
extern _glProgramLocalParameter4fvARB qt_glProgramLocalParameter4fvARB;

extern _glActiveStencilFaceEXT qt_glActiveStencilFaceEXT;

extern _glMultiTexCoord4f qt_glMultiTexCoord4f;
extern _glActiveTexture qt_glActiveTexture;

#define glProgramStringARB qt_glProgramStringARB
#define glBindProgramARB qt_glBindProgramARB
#define glDeleteProgramsARB qt_glDeleteProgramsARB
#define glGenProgramsARB qt_glGenProgramsARB
#define glProgramLocalParameter4fvARB qt_glProgramLocalParameter4fvARB

#define glActiveStencilFaceEXT qt_glActiveStencilFaceEXT

#define glMultiTexCoord4f qt_glMultiTexCoord4f
#define glActiveTexture qt_glActiveTexture

extern PFNGLISRENDERBUFFEREXTPROC qt_glIsRenderbufferEXT;
extern PFNGLBINDRENDERBUFFEREXTPROC qt_glBindRenderbufferEXT;
extern PFNGLDELETERENDERBUFFERSEXTPROC qt_glDeleteRenderbuffersEXT;
extern PFNGLGENRENDERBUFFERSEXTPROC qt_glGenRenderbuffersEXT;
extern PFNGLRENDERBUFFERSTORAGEEXTPROC qt_glRenderbufferStorageEXT;
extern PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC qt_glGetRenderbufferParameterivEXT;
extern PFNGLISFRAMEBUFFEREXTPROC qt_glIsFramebufferEXT;
extern PFNGLBINDFRAMEBUFFEREXTPROC qt_glBindFramebufferEXT;
extern PFNGLDELETEFRAMEBUFFERSEXTPROC qt_glDeleteFramebuffersEXT;
extern PFNGLGENFRAMEBUFFERSEXTPROC qt_glGenFramebuffersEXT;
extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC qt_glCheckFramebufferStatusEXT;
extern PFNGLFRAMEBUFFERTEXTURE1DEXTPROC qt_glFramebufferTexture1DEXT;
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC qt_glFramebufferTexture2DEXT;
extern PFNGLFRAMEBUFFERTEXTURE3DEXTPROC qt_glFramebufferTexture3DEXT;
extern PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC qt_glFramebufferRenderbufferEXT;
extern PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC qt_glGetFramebufferAttachmentParameterivEXT;
extern PFNGLGENERATEMIPMAPEXTPROC qt_glGenerateMipmapEXT;

#define glIsRenderbufferEXT qt_glIsRenderbufferEXT
#define glBindRenderbufferEXT qt_glBindRenderbufferEXT
#define glDeleteRenderbuffersEXT qt_glDeleteRenderbuffersEXT
#define glGenRenderbuffersEXT qt_glGenRenderbuffersEXT
#define glRenderbufferStorageEXT qt_glRenderbufferStorageEXT
#define glGetRenderbufferParameterivEXT qt_glGetRenderbufferParameterivEXT
#define glIsFramebufferEXT qt_glIsFramebufferEXT
#define glBindFramebufferEXT qt_glBindFramebufferEXT
#define glDeleteFramebuffersEXT qt_glDeleteFramebuffersEXT
#define glGenFramebuffersEXT qt_glGenFramebuffersEXT
#define glCheckFramebufferStatusEXT qt_glCheckFramebufferStatusEXT
#define glFramebufferTexture1DEXT qt_glFramebufferTexture1DEXT
#define glFramebufferTexture2DEXT qt_glFramebufferTexture2DEXT
#define glFramebufferTexture3DEXT qt_glFramebufferTexture3DEXT
#define glFramebufferRenderbufferEXT qt_glFramebufferRenderbufferEXT
#define glGetFramebufferAttachmentParameterivEXT qt_glGetFramebufferAttachmentParameterivEXT
#define glGenerateMipmapEXT qt_glGenerateMipmapEXT

extern bool qt_resolve_framebufferobject_extensions(QGLContext *ctx);

#elif defined(Q_WS_WIN)

#define glProgramStringARB qt_glctx_get_dptr(ctx)->qt_glProgramStringARB
#define glBindProgramARB qt_glctx_get_dptr(ctx)->qt_glBindProgramARB
#define glDeleteProgramsARB qt_glctx_get_dptr(ctx)->qt_glDeleteProgramsARB
#define glGenProgramsARB qt_glctx_get_dptr(ctx)->qt_glGenProgramsARB
#define glProgramLocalParameter4fvARB qt_glctx_get_dptr(ctx)->qt_glProgramLocalParameter4fvARB

#define glActiveStencilFaceEXT qt_glctx_get_dptr(ctx)->qt_glActiveStencilFaceEXT

#define glMultiTexCoord4f qt_glctx_get_dptr(ctx)->qt_glMultiTexCoord4f
#define glActiveTexture qt_glctx_get_dptr(ctx)->qt_glActiveTexture

#define glIsRenderbufferEXT qt_glctx_get_dptr(ctx)->qt_glIsRenderbufferEXT
#define glBindRenderbufferEXT qt_glctx_get_dptr(ctx)->qt_glBindRenderbufferEXT
#define glDeleteRenderbuffersEXT qt_glctx_get_dptr(ctx)->qt_glDeleteRenderbuffersEXT
#define glGenRenderbuffersEXT qt_glctx_get_dptr(ctx)->qt_glGenRenderbuffersEXT
#define glRenderbufferStorageEXT qt_glctx_get_dptr(ctx)->qt_glRenderbufferStorageEXT
#define glGetRenderbufferParameterivEXT qt_glctx_get_dptr(ctx)->qt_glGetRenderbufferParameterivEXT
#define glIsFramebufferEXT qt_glctx_get_dptr(ctx)->qt_glIsFramebufferEXT
#define glBindFramebufferEXT qt_glctx_get_dptr(ctx)->qt_glBindFramebufferEXT
#define glDeleteFramebuffersEXT qt_glctx_get_dptr(ctx)->qt_glDeleteFramebuffersEXT
#define glGenFramebuffersEXT qt_glctx_get_dptr(ctx)->qt_glGenFramebuffersEXT
#define glCheckFramebufferStatusEXT qt_glctx_get_dptr(ctx)->qt_glCheckFramebufferStatusEXT
#define glFramebufferTexture1DEXT qt_glctx_get_dptr(ctx)->qt_glFramebufferTexture1DEXT
#define glFramebufferTexture2DEXT qt_glctx_get_dptr(ctx)->qt_glFramebufferTexture2DEXT
#define glFramebufferTexture3DEXT qt_glctx_get_dptr(ctx)->qt_glFramebufferTexture3DEXT
#define glFramebufferRenderbufferEXT qt_glctx_get_dptr(ctx)->qt_glFramebufferRenderbufferEXT
#define glGetFramebufferAttachmentParameterivEXT qt_glctx_get_dptr(ctx)->qt_glGetFramebufferAttachmentParameterivEXT
#define glGenerateMipmapEXT qt_glctx_get_dptr(ctx)->qt_glGenerateMipmapEXT

extern bool qt_resolve_framebufferobject_extensions(QGLContext *ctx);

#endif

bool qt_resolve_version_1_3_functions(QGLContext *ctx);
bool qt_resolve_stencil_face_extension(QGLContext *ctx);
bool qt_resolve_frag_program_extensions(QGLContext *ctx);

#endif // QGL_P_H
