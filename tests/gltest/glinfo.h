/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef GLINFO_H
#define GLINFO_H

#include <qstring.h>
#include <qstringlist.h>
#include <qwidget.h>
#include <qgl.h>
#include <qdialog.h>
class QTextView;
class QListView;

#if defined(Q_WS_WIN)
#include <qt_windows.h>
#endif

#if defined(Q_WS_MAC)
#include <qt_mac.h>
#ifndef QMAC_OPENGL_DOUBLEBUFFER
#define QMAC_OPENGL_DOUBLEBUFFER
#endif
#endif

#if defined(Q_WS_X11)
//#include <qt_x11.h>

struct visual_attribs {
    /* X visual attribs */
    int id;
    int klass;
    int depth;
    int redMask, greenMask, blueMask;
    int colormapSize;
    int bitsPerRGB;
 
    /* GL visual attribs */
    int supportsGL;
    int transparent;
    int bufferSize;
    int level;
    int rgba;
    int doubleBuffer;
    int stereo;
    int auxBuffers;
    int redSize, greenSize, blueSize, alphaSize;
    int depthSize;
    int stencilSize;
    int accumRedSize, accumGreenSize, accumBlueSize, accumAlphaSize;
    int numSamples, numMultisample;
    int visualCaveat;
};
#endif

class GLInfo : public QDialog
{
    Q_OBJECT

public:
    GLInfo(QWidget* parent = 0, const char* name = 0);
    QString getText();
    QStringList getViewList();

protected:
    QString *infotext;
    QStringList *viewlist;
    QTextView* infoView;
    QListView* infoList;
    QGLWidget *glw;
#if defined(Q_WS_X11)
    QWidget *parent;
    void print_screen_info(Display *dpy, int scrnum);
    void print_extension_list(const char *ext);
    void print_visual_info(Display *dpy, int scrnum);
#elif defined(Q_WS_WIN)
    void VisualInfo( HDC hDC );
#endif
};

#endif
