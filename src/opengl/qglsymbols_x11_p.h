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

#ifndef QGLSYMBOLS_X11_P_H
#define QGLSYMBOLS_X11_P_H

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

// GL protos

extern "C" {
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
typedef void (*_glPixelStorei)( GLenum pname, GLint param );
typedef void (*_glBitmap)( GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove,
			   const GLubyte *bitmap );
typedef void (*_glDrawPixels)( GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels );
typedef void (*_glNewList)( GLuint list, GLenum mode );
typedef void (*_glGetFloatv)( GLenum pname, GLfloat *params );
typedef void (*_glGetIntegerv)( GLenum pname, GLint *params );
typedef void (*_glEndList)( void );

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
extern _glPixelStorei qt_glPixelStorei;
extern _glBitmap qt_glBitmap;
extern _glDrawPixels qt_glDrawPixels;
extern _glNewList qt_glNewList;
extern _glGetFloatv qt_glGetFloatv;
extern _glGetIntegerv qt_glGetIntegerv;
extern _glEndList qt_glEndList;

// GLX protos
#ifndef QGL_NO_GLX_VISIBILITY
typedef GLXFBConfig* (*_glXChooseFBConfig) (Display *dpy, int screen, const int *attrib_list, int *nelements);
typedef int (*_glXGetFBConfigAttrib) (Display *dpy, GLXFBConfig config, int attribute, int *value);
typedef GLXPbuffer (*_glXCreatePbuffer) (Display *dpy, GLXFBConfig config, const int *attrib_list);
typedef void (*_glXDestroyPbuffer) (Display *dpy, GLXPbuffer pbuf);
typedef GLXContext (*_glXCreateNewContext) (Display *dpy, GLXFBConfig config, int render_type, GLXContext share_list, Bool direct);
typedef Bool (*_glXMakeContextCurrent) (Display *dpy, GLXDrawable draw, GLXDrawable read, GLXContext ctx);

extern _glXChooseFBConfig qt_glXChooseFBConfig;
extern _glXCreateNewContext qt_glXCreateNewContext;
extern _glXCreatePbuffer qt_glXCreatePbuffer;
extern _glXDestroyPbuffer qt_glXDestroyPbuffer;
extern _glXGetFBConfigAttrib qt_glXGetFBConfigAttrib;
extern _glXMakeContextCurrent qt_glXMakeContextCurrent;

typedef XVisualInfo* (*_glXChooseVisual)(Display *dpy, int screen, int *attribList);
typedef GLXContext (*_glXCreateContext)(Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct);
typedef GLXPixmap (*_glXCreateGLXPixmap)(Display *dpy, XVisualInfo *vis, Pixmap pixmap);
typedef void (*_glXDestroyContext)(Display *dpy, GLXContext ctx);
typedef void (*_glXDestroyGLXPixmap)(Display *dpy, GLXPixmap pix);
typedef const char* (*_glXGetClientString)(Display *dpy, int name );
typedef int (*_glXGetConfig)(Display *dpy, XVisualInfo *vis, int attrib, int *value);
typedef Bool (*_glXIsDirect)(Display *dpy, GLXContext ctx);
typedef Bool (*_glXMakeCurrent)(Display *dpy, GLXDrawable drawable, GLXContext ctx);
typedef Bool (*_glXQueryExtension)(Display *dpy, int *errorBase, int *eventBase);
typedef const char* (*_glXQueryExtensionsString)(Display *dpy, int screen);
typedef const char* (*_glXQueryServerString)(Display *dpy, int screen, int name);
typedef void (*_glXSwapBuffers)(Display *dpy, GLXDrawable drawable);
typedef void (*_glXUseXFont)(Font font, int first, int count, int listBase);
typedef void (*_glXWaitX)(void);

extern _glXChooseVisual qt_glXChooseVisual;
extern _glXCreateContext qt_glXCreateContext;
extern _glXCreateGLXPixmap qt_glXCreateGLXPixmap;
extern _glXDestroyContext qt_glXDestroyContext;
extern _glXDestroyGLXPixmap qt_glXDestroyGLXPixmap;
extern _glXGetClientString qt_glXGetClientString;
extern _glXGetConfig qt_glXGetConfig;
extern _glXIsDirect qt_glXIsDirect;
extern _glXMakeCurrent qt_glXMakeCurrent;
extern _glXQueryExtension qt_glXQueryExtension;
extern _glXQueryExtensionsString qt_glXQueryExtensionsString;
extern _glXQueryServerString qt_glXQueryServerString;
extern _glXSwapBuffers qt_glXSwapBuffers;
extern _glXUseXFont qt_glXUseXFont;
extern _glXWaitX qt_glXWaitX;
#endif // QGL_NO_GLX_VISIBILITY
};

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
#define glPixelStorei qt_glPixelStorei
#define glBitmap qt_glBitmap
#define glDrawPixels qt_glDrawPixels
#define glNewList qt_glNewList
#define glGetFloatv qt_glGetFloatv
#define glGetIntegerv qt_glGetIntegerv
#define glEndList qt_glEndList

#define glXChooseFBConfig qt_glXChooseFBConfig
#define glXCreateNewContext qt_glXCreateNewContext
#define glXCreatePbuffer qt_glXCreatePbuffer
#define glXDestroyPbuffer qt_glXDestroyPbuffer
#define glXGetFBConfigAttrib qt_glXGetFBConfigAttrib
#define glXMakeContextCurrent qt_glXMakeContextCurrent

#define glXChooseVisual qt_glXChooseVisual
#define glXCreateContext qt_glXCreateContext
#define glXCreateGLXPixmap qt_glXCreateGLXPixmap
#define glXDestroyContext qt_glXDestroyContext
#define glXDestroyGLXPixmap qt_glXDestroyGLXPixmap
#define glXGetClientString qt_glXGetClientString
#define glXGetConfig qt_glXGetConfig
#define glXIsDirect qt_glXIsDirect
#define glXMakeCurrent qt_glXMakeCurrent
#define glXQueryExtension qt_glXQueryExtension
#define glXQueryExtensionsString qt_glXQueryExtensionsString
#define glXQueryServerString qt_glXQueryServerString
#define glXSwapBuffers qt_glXSwapBuffers
#define glXUseXFont qt_glXUseXFont
#define glXWaitX qt_glXWaitX

#define QGL_GLX_1_X  1
#define QGL_GLX_1_3  2
extern int qt_resolve_gl_symbols();

#endif // QGLSYMBOLS_X11_P_H
