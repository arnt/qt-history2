/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

/****************************************************************************
**
** This is the info widget for unix.
** Most of the code was borrowed from Brian Pauls glxinfo.c
**
****************************************************************************/

#include "glinfo.h"

#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <qstring.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qtextview.h>
#include <qpushbutton.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
 
#ifndef GLX_NONE_EXT
#define GLX_NONE_EXT  0x8000
#endif


void GLInfo::print_extension_list(const char *ext)
{
    const char *indentString = "    ";
    const int indent = 4;
    const int max = 79;
    int width, i, j;
 
    if (!ext || !ext[0])
        return;
 
    width = indent;
    infotext->sprintf("%s%s", (const char*)*infotext, indentString);
    i = j = 0;
    while (1) {
        if (ext[j] == ' ' || ext[j] == 0) {
	    /* found end of an extension name */
	    const int len = j - i;
	    if (width + len > max) {
                /* start a new line */
	        infotext->sprintf("%s\n", (const char*)*infotext);
		width = indent;
		infotext->sprintf("%s%s", (const char*)*infotext, indentString);
	    }
	    /* print the extension name between ext[i] and ext[j] */
	    while (i < j) {
	        infotext->sprintf("%s%c", (const char*)*infotext, ext[i]);
		i++;
	    }
	    /* either we're all done, or we'll continue with next extension */
	    width += len + 1;
	    if (ext[j] == 0) {
	        break;
	    }
	    else {
	        i++;
		j++;
		if (ext[j] == 0)
		    break; 
		infotext->sprintf("%s, ", (const char*)*infotext);
		width += 2;
	    }
	}   
	j++;
    }
    infotext->sprintf("%s\n", (const char*)*infotext);
}

void GLInfo::print_screen_info(Display *dpy, int scrnum)
{
    glw->makeCurrent();

    const char *serverVendor = glXQueryServerString(dpy, scrnum, GLX_VENDOR);
    const char *serverVersion = glXQueryServerString(dpy, scrnum, GLX_VERSION);
    const char *serverExtensions = glXQueryServerString(dpy, scrnum, GLX_EXTENSIONS);
    const char *clientVendor = glXGetClientString(dpy, GLX_VENDOR);
    const char *clientVersion = glXGetClientString(dpy, GLX_VERSION);
    const char *clientExtensions = glXGetClientString(dpy, GLX_EXTENSIONS);
    const char *glxExtensions = glXQueryExtensionsString(dpy, scrnum);
    const char *glVendor = (const char *) glGetString(GL_VENDOR);
    const char *glRenderer = (const char *) glGetString(GL_RENDERER);
    const char *glVersion = (const char *) glGetString(GL_VERSION);
    const char *glExtensions = (const char *) glGetString(GL_EXTENSIONS);
    const char *gluVersion = (const char *) gluGetString((GLenum)GLU_VERSION);
    const char *gluExtensions = (const char *) gluGetString((GLenum)GLU_EXTENSIONS);

    infotext->sprintf("%sdirect rendering: %s\n", (const char*)*infotext, 
		      glw->format().directRendering() ? "Yes" : "No");
    infotext->sprintf("%sserver glx vendor string: %s\n", (const char*)*infotext, serverVendor);
    infotext->sprintf("%sserver glx version string: %s\n", (const char*)*infotext, serverVersion);
    infotext->sprintf("%sserver glx extensions:\n", (const char*)*infotext);
    print_extension_list(serverExtensions);
    infotext->sprintf("%sclient glx vendor string: %s\n", (const char*)*infotext, clientVendor);
    infotext->sprintf("%sclient glx version string: %s\n", (const char*)*infotext, clientVersion);
    infotext->sprintf("%sclient glx extensions:\n", (const char*)*infotext);
    print_extension_list(clientExtensions);
    infotext->sprintf("%sGLX extensions:\n", (const char*)*infotext);
    print_extension_list(glxExtensions);
    infotext->sprintf("%sOpenGL vendor string: %s\n", (const char*)*infotext, glVendor);
    infotext->sprintf("%sOpenGL renderer string: %s\n", (const char*)*infotext, glRenderer);
    infotext->sprintf("%sOpenGL version string: %s\n", (const char*)*infotext, glVersion);
    infotext->sprintf("%sOpenGL extensions:\n", (const char*)*infotext);
    print_extension_list(glExtensions);
    infotext->sprintf("%sglu version: %s\n", (const char*)*infotext, gluVersion);
    infotext->sprintf("%sglu extensions:\n", (const char*)*infotext);
    print_extension_list(gluExtensions);

   
};

const char * visual_class_name(int cls)
{
    switch (cls) {
    case StaticColor:
        return "StaticColor";
    case PseudoColor:
        return "PseudoColor";
    case StaticGray:
        return "StaticGray";
    case GrayScale:
        return "GrayScale";
    case TrueColor:
        return "TrueColor";
    case DirectColor:
        return "DirectColor";
    default:
        return "";
   }
}


void get_visual_attribs(Display *dpy, XVisualInfo *vInfo,
			struct visual_attribs *attribs)
{ 
    memset(attribs, 0, sizeof(struct visual_attribs));
 
    attribs->id = vInfo->visualid;
    attribs->klass = vInfo->c_class;
    attribs->depth = vInfo->depth;
    attribs->redMask = vInfo->red_mask;
    attribs->greenMask = vInfo->green_mask;
    attribs->blueMask = vInfo->blue_mask;
    attribs->colormapSize = vInfo->colormap_size;
    attribs->bitsPerRGB = vInfo->bits_per_rgb;
 
    if (glXGetConfig(dpy, vInfo, GLX_USE_GL, &attribs->supportsGL) != 0)
        return;
    glXGetConfig(dpy, vInfo, GLX_BUFFER_SIZE, &attribs->bufferSize);
    glXGetConfig(dpy, vInfo, GLX_LEVEL, &attribs->level);
    glXGetConfig(dpy, vInfo, GLX_RGBA, &attribs->rgba);
    glXGetConfig(dpy, vInfo, GLX_DOUBLEBUFFER, &attribs->doubleBuffer);
    glXGetConfig(dpy, vInfo, GLX_STEREO, &attribs->stereo);
    glXGetConfig(dpy, vInfo, GLX_AUX_BUFFERS, &attribs->auxBuffers);
    glXGetConfig(dpy, vInfo, GLX_RED_SIZE, &attribs->redSize);
    glXGetConfig(dpy, vInfo, GLX_GREEN_SIZE, &attribs->greenSize);
    glXGetConfig(dpy, vInfo, GLX_BLUE_SIZE, &attribs->blueSize);
    glXGetConfig(dpy, vInfo, GLX_ALPHA_SIZE, &attribs->alphaSize);
    glXGetConfig(dpy, vInfo, GLX_DEPTH_SIZE, &attribs->depthSize);
    glXGetConfig(dpy, vInfo, GLX_STENCIL_SIZE, &attribs->stencilSize);
    glXGetConfig(dpy, vInfo, GLX_ACCUM_RED_SIZE, &attribs->accumRedSize);
    glXGetConfig(dpy, vInfo, GLX_ACCUM_GREEN_SIZE, &attribs->accumGreenSize);
    glXGetConfig(dpy, vInfo, GLX_ACCUM_BLUE_SIZE, &attribs->accumBlueSize);
    glXGetConfig(dpy, vInfo, GLX_ACCUM_ALPHA_SIZE, &attribs->accumAlphaSize);
    
    /* transparent pixel value not implemented yet */
    attribs->transparent = 0;
 
    /* multisample tests not implemented yet */
    attribs->numSamples = 0;
    attribs->numMultisample = 0;
 
#if defined(GLX_EXT_visual_rating)
    const char *ext = glXQueryExtensionsString(dpy, vInfo->screen);
    if (ext && strstr(ext, "GLX_EXT_visual_rating")) {
        glXGetConfig(dpy, vInfo, GLX_VISUAL_CAVEAT_EXT, &attribs->visualCaveat);
    }
    else {
        attribs->visualCaveat = GLX_NONE_EXT;
    }
#else
    attribs->visualCaveat = 0;
#endif
}


void GLInfo::print_visual_info(Display *dpy, int scrnum)
{
    QString str;
    XVisualInfo *visuals, temp;
    int numVisuals;
    long mask;
    int i;
 
    /* get list of all visuals on this screen */
    temp.screen = scrnum;
    mask = VisualScreenMask;
    visuals = XGetVisualInfo(dpy, mask, &temp, &numVisuals);
 
    for (i = 0; i < numVisuals; i++) {
        struct visual_attribs attribs;
	get_visual_attribs(dpy, &visuals[i], &attribs);
	str.sprintf("0x%2x %d %s %d %d %d %s %d %d %d %d %d %d"
		     " %d %d %d %d %d %d %d %d %d",
		     attribs.id,
		     attribs.depth,
		     visual_class_name(attribs.klass),
		     attribs.transparent,
		     attribs.bufferSize,
		     attribs.level,
		     attribs.rgba ? "rgba" : "ci",
		     attribs.doubleBuffer,
		     attribs.stereo,
		     attribs.redSize, attribs.greenSize,
		     attribs.blueSize, attribs.alphaSize,
		     attribs.auxBuffers,
		     attribs.depthSize,
		     attribs.stencilSize,
		     attribs.accumRedSize, attribs.accumGreenSize,
		     attribs.accumBlueSize, attribs.accumAlphaSize,
		     attribs.numSamples, attribs.numMultisample
		     );
	viewlist->append(str);
    }

 
    XFree(visuals);
}

GLInfo::GLInfo(QWidget* parent, const char* name)
    : QDialog(parent, name)
{
    glw = new QGLWidget( this );
    
    infotext = new QString("GLTest:\n");
    viewlist = new QStringList();
    char *displayName = NULL;
    Display *dpy;
    int numScreens, scrnum;
     
    dpy = this->x11Display();
    if (!dpy) {
        qDebug("Error: unable to open display %s\n", displayName);
    }
 
    numScreens = ScreenCount(dpy);
    infotext->append("name of display: ");
    infotext->append(DisplayString(dpy));
    infotext->append("\n");
    
    for (scrnum = 0; scrnum < numScreens; scrnum++) {
	print_screen_info(dpy, scrnum);
	print_visual_info(dpy, scrnum);
	if (scrnum + 1 < numScreens)
	    printf("\n\n");
    }
  
    QVBoxLayout *layout = new QVBoxLayout(this);
    infoView = new QTextView( this, "infoView" );
    layout->addWidget( infoView, 7 );
 
    infoList = new QListView( this, "infoList" );
    infoList->addColumn( trUtf8( "Vis ID", "" ) );
    infoList->addColumn( trUtf8( "Vis Depth", "" ) );
    infoList->addColumn( trUtf8( "Visual Type", "" ) );
    infoList->addColumn( trUtf8( "Transparent", "" ) );
    infoList->addColumn( trUtf8( "buff size", "" ) );
    infoList->addColumn( trUtf8( "level", "" ) );
    infoList->addColumn( trUtf8( "render type", "" ) );
    infoList->addColumn( trUtf8( "DB", "" ) );
    infoList->addColumn( trUtf8( "stereo", "" ) );
    infoList->addColumn( trUtf8( "R sz", "" ) );
    infoList->addColumn( trUtf8( "G sz", "" ) );
    infoList->addColumn( trUtf8( "B sz", "" ) );
    infoList->addColumn( trUtf8( "A sz", "" ) );
    infoList->addColumn( trUtf8( "aux buff", "" ) );
    infoList->addColumn( trUtf8( "depth", "" ) );
    infoList->addColumn( trUtf8( "stencil", "" ) );
    infoList->addColumn( trUtf8( "R accum", "" ) );
    infoList->addColumn( trUtf8( "G accum", "" ) );
    infoList->addColumn( trUtf8( "B accum", "" ) );
    infoList->addColumn( trUtf8( "A accum", "" ) );
    infoList->addColumn( trUtf8( "MS num", "" ) );
    infoList->addColumn( trUtf8( "MS bufs", "" ) );
    infoList->setSelectionMode( QListView::Extended );
    infoList->setAllColumnsShowFocus( TRUE );
    layout->addWidget( infoList, 10 );
    
    QHBoxLayout *buttonLayout = new QHBoxLayout( this );
    layout->addLayout( buttonLayout );
    buttonLayout->addStretch();
    
    QPushButton *ok = new QPushButton( "Ok", this );
    connect( ok, SIGNAL( clicked() ), this, SLOT( accept() ));
    buttonLayout->addWidget( ok );
    
    int i;
    QListViewItem *item;
    QStringList list, listItem;
    infoView->setText(*infotext);
    for ( QStringList::Iterator it = viewlist->begin(); it != viewlist->end(); ++it ) {
        i = 0;
        item = new QListViewItem(infoList);
        listItem = QStringList::split(" ", (*it).latin1());
        for ( QStringList::Iterator ti = listItem.begin(); ti != listItem.end(); ++ti ) {
            item->setText(i, (*ti).latin1());
            i++;
        }
        infoList->insertItem(item);
    }
};

QString GLInfo::getText()
{
  return *infotext;
}

QStringList GLInfo::getViewList()
{
  return *viewlist;
}
