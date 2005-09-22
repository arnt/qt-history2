#include <qdebug.h>
#include <qlibrary.h>
#include <private/qt_x11_p.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include "qglsymbols_x11_p.h"

extern "C" {
_glCallLists qt_glCallLists;
_glClearColor qt_glClearColor;
_glClearIndex qt_glClearIndex;
_glColor3ub qt_glColor3ub;
_glDeleteLists qt_glDeleteLists;
_glDrawBuffer qt_glDrawBuffer;
_glFlush qt_glFlush;
_glIndexi qt_glIndexi;
_glListBase qt_glListBase;
_glLoadIdentity qt_glLoadIdentity;
_glMatrixMode qt_glMatrixMode;
_glOrtho qt_glOrtho;
_glPopAttrib qt_glPopAttrib;
_glPopMatrix qt_glPopMatrix;
_glPushAttrib qt_glPushAttrib;
_glPushMatrix qt_glPushMatrix;
_glRasterPos2i qt_glRasterPos2i;
_glRasterPos3d qt_glRasterPos3d;
_glReadPixels qt_glReadPixels;
_glViewport qt_glViewport;
_glPixelStorei qt_glPixelStorei;
_glBitmap qt_glBitmap;
_glDrawPixels qt_glDrawPixels;
_glNewList qt_glNewList;
_glGetFloatv qt_glGetFloatv;
_glGetIntegerv qt_glGetIntegerv;
_glEndList qt_glEndList;

_glXChooseVisual qt_glXChooseVisual;
_glXCreateContext qt_glXCreateContext;
_glXCreateGLXPixmap qt_glXCreateGLXPixmap;
_glXDestroyContext qt_glXDestroyContext;
_glXDestroyGLXPixmap qt_glXDestroyGLXPixmap;
_glXGetClientString qt_glXGetClientString;
_glXGetConfig qt_glXGetConfig;
_glXIsDirect qt_glXIsDirect;
_glXMakeCurrent qt_glXMakeCurrent;
_glXQueryExtension qt_glXQueryExtension;
_glXQueryExtensionsString qt_glXQueryExtensionsString;
_glXQueryServerString qt_glXQueryServerString;
_glXSwapBuffers qt_glXSwapBuffers;
_glXUseXFont qt_glXUseXFont;
_glXWaitX qt_glXWaitX;

_glXChooseFBConfig qt_glXChooseFBConfig;
_glXCreateNewContext qt_glXCreateNewContext;
_glXCreatePbuffer qt_glXCreatePbuffer;
_glXDestroyPbuffer qt_glXDestroyPbuffer;
_glXGetFBConfigAttrib qt_glXGetFBConfigAttrib;
_glXMakeContextCurrent qt_glXMakeContextCurrent;
};

int qt_resolve_gl_symbols()
{
    static int resolve_lvl = 0;
    static int resolved = false;

    if (resolved)
        return resolve_lvl;

    QLibrary gl("GL");

    qt_glCallLists = (_glCallLists) gl.resolve("glCallLists");
    if (!qt_glCallLists) { // if this fails the rest will surely fail
        qFatal("Unable to resolve GL/GLX symbols - please check your GL library installation.");
	return resolve_lvl;
    }

    resolve_lvl = QGL_GLX_1_X;

    // GLX >= 1.0
    qt_glClearColor = (_glClearColor) gl.resolve("glClearColor");
    qt_glClearIndex = (_glClearIndex) gl.resolve("glClearIndex");
    qt_glColor3ub = (_glColor3ub) gl.resolve("glColor3ub");
    qt_glDeleteLists = (_glDeleteLists) gl.resolve("glDeleteLists");
    qt_glDrawBuffer = (_glDrawBuffer) gl.resolve("glDrawBuffer");
    qt_glFlush = (_glFlush) gl.resolve("glFlush");
    qt_glIndexi = (_glIndexi) gl.resolve("glIndexi");
    qt_glListBase = (_glListBase) gl.resolve("glListBase");
    qt_glLoadIdentity = (_glLoadIdentity) gl.resolve("glLoadIdentity");
    qt_glMatrixMode = (_glMatrixMode) gl.resolve("glMatrixMode");
    qt_glOrtho = (_glOrtho) gl.resolve("glOrtho");
    qt_glPopAttrib = (_glPopAttrib) gl.resolve("glPopAttrib");
    qt_glPopMatrix = (_glPopMatrix) gl.resolve("glPopMatrix");
    qt_glPushAttrib = (_glPushAttrib) gl.resolve("glPushAttrib");
    qt_glPushMatrix = (_glPushMatrix) gl.resolve("glPushMatrix");
    qt_glRasterPos2i = (_glRasterPos2i) gl.resolve("glRasterPos2i");
    qt_glRasterPos3d = (_glRasterPos3d) gl.resolve("glRasterPos3d");
    qt_glReadPixels = (_glReadPixels) gl.resolve("glReadPixels");
    qt_glViewport = (_glViewport) gl.resolve("glViewport");
    qt_glPixelStorei = (_glPixelStorei) gl.resolve("glPixelStorei");
    qt_glBitmap = (_glBitmap) gl.resolve("glBitmap");
    qt_glDrawPixels = (_glDrawPixels) gl.resolve("glDrawPixels");
    qt_glNewList = (_glNewList) gl.resolve("glNewList");
    qt_glGetFloatv = (_glGetFloatv) gl.resolve("glGetFloatv");
    qt_glGetIntegerv = (_glGetIntegerv) gl.resolve("glGetIntegerv");
    qt_glEndList = (_glEndList) gl.resolve("glEndList");

    qt_glXChooseVisual = (_glXChooseVisual) gl.resolve("glXChooseVisual");
    qt_glXCreateContext = (_glXCreateContext) gl.resolve("glXCreateContext");
    qt_glXCreateGLXPixmap = (_glXCreateGLXPixmap) gl.resolve("glXCreateGLXPixmap");
    qt_glXDestroyContext = (_glXDestroyContext) gl.resolve("glXDestroyContext");
    qt_glXDestroyGLXPixmap = (_glXDestroyGLXPixmap) gl.resolve("glXDestroyGLXPixmap");
    qt_glXGetClientString = (_glXGetClientString) gl.resolve("glXGetClientString");
    qt_glXGetConfig = (_glXGetConfig) gl.resolve("glXGetConfig");
    qt_glXIsDirect = (_glXIsDirect) gl.resolve("glXIsDirect");
    qt_glXMakeCurrent = (_glXMakeCurrent) gl.resolve("glXMakeCurrent");
    qt_glXQueryExtension = (_glXQueryExtension) gl.resolve("glXQueryExtension");
    qt_glXQueryExtensionsString = (_glXQueryExtensionsString) gl.resolve("glXQueryExtensionsString");
    qt_glXQueryServerString = (_glXQueryServerString) gl.resolve("glXQueryServerString");
    qt_glXSwapBuffers = (_glXSwapBuffers) gl.resolve("glXSwapBuffers");
    qt_glXUseXFont = (_glXUseXFont) gl.resolve("glXUseXFont");
    qt_glXWaitX = (_glXWaitX) gl.resolve("glXWaitX");


    // GLX >= 1.3
    qt_glXChooseFBConfig = (_glXChooseFBConfig) gl.resolve("glXChooseFBConfig");
    qt_glXCreateNewContext = (_glXCreateNewContext) gl.resolve("glXCreateNewContext");
    qt_glXCreatePbuffer = (_glXCreatePbuffer) gl.resolve("glXCreatePbuffer");
    qt_glXDestroyPbuffer = (_glXDestroyPbuffer) gl.resolve("glXDestroyPbuffer");
    qt_glXGetFBConfigAttrib = (_glXGetFBConfigAttrib) gl.resolve("glXGetFBConfigAttrib");
    qt_glXMakeContextCurrent = (_glXMakeContextCurrent) gl.resolve("glXMakeContextCurrent");
    if (qt_glXMakeContextCurrent)
        resolve_lvl = QGL_GLX_1_3;

    resolved = true;
    return resolve_lvl;
}

