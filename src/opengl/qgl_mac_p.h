#ifndef QGL_MAC_P_H
#define QGL_MAC_P_H
#ifdef QT_DLOPEN_OPENGL
// resolve the GL symbols we use ourselves
bool qt_resolve_gl_symbols(bool = TRUE);
extern "C" {
    // GL symbols
    typedef void (*_glCallLists)( GLsizei n, GLenum type, const GLvoid *lists );
    typedef void (*_glClearColor)( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha );
    typedef void (*_glClearIndex)( GLfloat c );
    typedef void (*_glColor3ub)( GLubyte red, GLubyte green, GLubyte blue );
    typedef void (*_glDeleteLists)( GLuint list, GLsizei range );
    typedef void (*_glDrawBuffer)( GLenum mode );
    typedef void (*_glFlush)( void );
    typedef void (*_glIndexi)( GLint c );
    typedef void (*_glListBase)( GLuint base );
    typedef void (*_glLoadIdentity)( void );
    typedef void (*_glMatrixMode)( GLenum mode );
    typedef void (*_glOrtho)( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val );
    typedef void (*_glPopAttrib)( void );
    typedef void (*_glPopMatrix)( void );
    typedef void (*_glPushAttrib)( GLbitfield mask );
    typedef void (*_glPushMatrix)( void );
    typedef void (*_glRasterPos2i)( GLint x, GLint y );
    typedef void (*_glRasterPos3d)( GLdouble x, GLdouble y, GLdouble z );
    typedef void (*_glReadPixels)( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels );
    typedef void (*_glViewport)( GLint x, GLint y, GLsizei width, GLsizei height );

    // GLX symbols - should be in the GL lib as well
    typedef AGLContext (*_aglCreateContext)(AGLPixelFormat pix, AGLContext share);
    typedef GLboolean (*_aglDestroyContext)(AGLContext ctx);
    typedef GLboolean (*_aglUpdateContext)(AGLContext ctx);
    typedef GLboolean (*_aglSetCurrentContext)(AGLContext ctx);
    typedef AGLContext (*_aglGetCurrentContext)(void);
    typedef AGLPixelFormat (*_aglNextPixelFormat)(AGLPixelFormat pix);
    typedef GLboolean (*_aglDescribePixelFormat)(AGLPixelFormat pix, GLint attrib, GLint *value);
    typedef AGLPixelFormat (*_aglChoosePixelFormat)(const AGLDevice *gdevs, GLint ndev, const GLint *attribs);
    typedef void (*_aglDestroyPixelFormat)(AGLPixelFormat pix);
    typedef GLboolean (*_aglSetDrawable)(AGLContext ctx, AGLDrawable draw);
    typedef GLboolean (*_aglSetOffScreen)(AGLContext ctx, GLsizei width, GLsizei height, GLsizei rowbytes, GLvoid *baseaddr);
    typedef void (*_aglSwapBuffers)(AGLContext ctx);
    typedef GLboolean (*_aglEnable)(AGLContext ctx, GLenum pname);
    typedef GLboolean (*_aglDisable)(AGLContext ctx, GLenum pname);
    typedef GLboolean (*_aglIsEnabled)(AGLContext ctx, GLenum pname);
    typedef GLboolean (*_aglSetInteger)(AGLContext ctx, GLenum pname, const GLint *params);
    typedef GLboolean (*_aglUseFont)(AGLContext ctx, GLint fontID, Style face, GLint size, GLint first, GLint count, GLint base);
    typedef GLenum (*_aglGetError)(void);

    extern _glCallLists qt_glCallLists;
    extern _glClearColor qt_glClearColor;
    extern _glClearIndex qt_glClearIndex;
    extern _glColor3ub qt_glColor3ub;
    extern _glDeleteLists qt_glDeleteLists;
    extern _glDrawBuffer qt_glDrawBuffer;
    extern _glFlush qt_glFlush;
    extern _glIndexi qt_glIndexi;
    extern _glListBase qt_glListBase;
    extern _glLoadIdentity qt_glLoadIdentity;
    extern _glMatrixMode qt_glMatrixMode;
    extern _glOrtho qt_glOrtho;
    extern _glPopAttrib qt_glPopAttrib;
    extern _glPopMatrix qt_glPopMatrix;
    extern _glPushAttrib qt_glPushAttrib;
    extern _glPushMatrix qt_glPushMatrix;
    extern _glRasterPos2i qt_glRasterPos2i;
    extern _glRasterPos3d qt_glRasterPos3d;
    extern _glReadPixels qt_glReadPixels;
    extern _glViewport qt_glViewport;

    extern _aglChoosePixelFormat qt_aglChoosePixelFormat;
    extern _aglCreateContext qt_aglCreateContext;
    extern _aglDescribePixelFormat qt_aglDescribePixelFormat;
    extern _aglDestroyContext qt_aglDestroyContext;
    extern _aglDestroyPixelFormat qt_aglDestroyPixelFormat;
    extern _aglDisable qt_aglDisable;
    extern _aglEnable qt_aglEnable;
    extern _aglGetCurrentContext qt_aglGetCurrentContext;
    extern _aglGetError qt_aglGetError;
    extern _aglIsEnabled qt_aglIsEnabled;
    extern _aglNextPixelFormat qt_aglNextPixelFormat;
    extern _aglSetCurrentContext qt_aglSetCurrentContext;
    extern _aglSetDrawable qt_aglSetDrawable;
    extern _aglSetInteger qt_aglSetInteger;
    extern _aglSetOffScreen qt_aglSetOffScreen;
    extern _aglSwapBuffers qt_aglSwapBuffers;
    extern _aglUpdateContext qt_aglUpdateContext;
    extern _aglUseFont qt_aglUseFont;
}; // extern "C"

#define glCallLists qt_glCallLists
#define glClearColor qt_glClearColor
#define glClearIndex qt_glClearIndex
#define glColor3ub qt_glColor3ub
#define glDeleteLists qt_glDeleteLists
#define glDrawBuffer qt_glDrawBuffer
#define glFlush qt_glFlush
#define glIndexi qt_glIndexi
#define glListBase qt_glListBase
#define glLoadIdentity qt_glLoadIdentity
#define glMatrixMode qt_glMatrixMode
#define glOrtho qt_glOrtho
#define glPopAttrib qt_glPopAttrib
#define glPopMatrix qt_glPopMatrix
#define glPushAttrib qt_glPushAttrib
#define glPushMatrix qt_glPushMatrix
#define glRasterPos2i qt_glRasterPos2i
#define glRasterPos3d qt_glRasterPos3d
#define glReadPixels qt_glReadPixels
#define glViewport qt_glViewport

#define aglChoosePixelFormat qt_aglChoosePixelFormat
#define aglCreateContext qt_aglCreateContext
#define aglDescribePixelFormat qt_aglDescribePixelFormat
#define aglDestroyContext qt_aglDestroyContext
#define aglDestroyPixelFormat qt_aglDestroyPixelFormat
#define aglDisable qt_aglDisable
#define aglEnable qt_aglEnable
#define aglGetCurrentContext qt_aglGetCurrentContext
#define aglGetError qt_aglGetError
#define aglIsEnabled qt_aglIsEnabled
#define aglNextPixelFormat qt_aglNextPixelFormat
#define aglSetCurrentContext qt_aglSetCurrentContext
#define aglSetDrawable qt_aglSetDrawable
#define aglSetInteger qt_aglSetInteger
#define aglSetOffScreen qt_aglSetOffScreen
#define aglSwapBuffers qt_aglSwapBuffers
#define aglUpdateContext qt_aglUpdateContext
#define aglUseFont qt_aglUseFont

#else
inline bool qt_resolve_gl_symbols(bool = TRUE) { return TRUE; }
#endif // QT_DLOPEN_OPENGL
#endif // QGL_P_H

