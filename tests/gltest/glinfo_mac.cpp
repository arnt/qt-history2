/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "glinfo.h"

#include "qstring.h"
#include <agl.h>
#include <gl.h>


GLInfo::GLInfo(QWidget* parent, const char* name)
    : QGLWidget(parent, name)
{
    infotext = new QString("GLTest:\n");
    viewlist = new QStringList();
 
};

QString GLInfo::getText()
{
    int   i;
    char* s;
    char  t[80];
    char* p;
    
    makeCurrent();
    return *infotext;

    infotext->sprintf("%sdisplay: N/A\n"
		      "server agl vendor string: N/A\n"
		      "server agl version string: N/A\n"
		      "server agl extensions (AGL_): N/A\n"
		      "client agl version: N/A\n"
		      "client agl extensions (AGL_): none\n"
		      "OpenGL vendor string: %s\n", (const char*)*infotext, 
		      ((const char*)glGetString(GL_VENDOR)));
    infotext->sprintf("%sOpenGL renderer string: %s\n", (const char*)*infotext, 
		      glGetString(GL_RENDERER));
    infotext->sprintf("%sOpenGL version string: %s\n", (const char*)*infotext, 
		      glGetString(GL_VERSION));
    infotext->sprintf("%sOpenGL extensions (GL_): \n", (const char*)*infotext);
    i = 0;
    s = (char*)glGetString(GL_EXTENSIONS);
	t[79] = '\0';
    while(*s) {
		t[i++] = *s;
		if(*s == ' ') {
			if (*(s+1) != '\0') {
				t[i-1] = ',';
				t[i] = ' ';
				p = &t[i++];
			} else {       // zoinks! last one terminated in a space! //
				t[i-1] = '\0';
			}
		}
		if(i > 80 - 5) {
			*p = t[i] = '\0';
			infotext->sprintf("%s    %s\n", (const char*)*infotext, t);
			p++;
			i = strlen(p);
			strcpy(t, p);
		}
		s++;
	}
    t[i] = '\0';
    infotext->sprintf("%s    %s.", (const char*)*infotext, t);
}

QStringList GLInfo::getViewList()
{
  return *viewlist;
}
