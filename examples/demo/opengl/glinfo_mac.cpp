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

#include <qgl.h>
#include "glinfo.h"

#include "qstring.h"
#include <agl.h>
#include <gl.h>
#include <Carbon/Carbon.h>

static QString bitsToModes(int bits)
{
    QString s;
    if ((bits & AGL_RGB8_BIT) || (bits & AGL_BGR233_BIT) || (bits & AGL_RGB332_BIT) )
	s.sprintf("    8 RGB bit/pixel: %s%s%s\n",
		  (bits & AGL_RGB8_BIT) ? "RGB=7:0 - " : "",
		  (bits & AGL_BGR233_BIT) ? "B=7:6, G=5:3, R=2:0 - " : "",
		  (bits & AGL_RGB332_BIT) ? "R=7:5, G=4:2, B=1:0" : "" );
    if ((bits & AGL_RGB8_A8_BIT) || (bits & AGL_BGR233_A8_BIT) || (bits & AGL_RGB332_A8_BIT) )
	s.sprintf("%s    8-8 ARGB bit/pixel: %s%s%s\n", s.latin1(),
		  (bits & AGL_RGB8_A8_BIT) ? "A=7:0, RGB=7:0 - " : "",
		  (bits & AGL_BGR233_A8_BIT) ? "A=7:0, B=7:6, G=5:3, R=2:0 - " : "",
		  (bits & AGL_RGB332_A8_BIT) ? "A=7:0, R=7:5, G=4:2, B=1:0" : "");
    if ((bits & AGL_RGB444_BIT) || (bits & AGL_RGB555_BIT) || (bits & AGL_RGB565_BIT) )
	s.sprintf("%s    16 RGB bit/pixel: %s%s%s\n", s.latin1(),
		  (bits & AGL_RGB444_BIT) ? "R=11:8, G=7:4, B=3:0 - " : "",
		  (bits & AGL_RGB555_BIT) ? "R=14:10, G=9:5, B=4:0 - " : "",
		  (bits & AGL_RGB565_BIT) ? "R=15:11, G=10:5, B=4:0" : "" );
    if ((bits & AGL_ARGB4444_BIT) || (bits & AGL_ARGB1555_BIT))
	s.sprintf("%s    16 ARGB bit/pixel: %s%s\n", s.latin1(),
		  (bits & AGL_ARGB4444_BIT) ? "A=15:12, R=11:8, G=7:4, B=3:0 - " : "",
		  (bits & AGL_ARGB1555_BIT) ? "A=15, R=14:10, G=9:5, B=4:0" : "");
    if ((bits & AGL_RGB444_A8_BIT) || (bits & AGL_RGB555_A8_BIT) || (bits & AGL_RGB565_A8_BIT) )
	s.sprintf("%s    8-16 ARGB bit/pixel: %s%s%s\n", s.latin1(),
		  (bits & AGL_RGB444_A8_BIT) ? "A=7:0, R=11:8, G=7:4, B=3:0 - " : "",
		  (bits & AGL_RGB555_A8_BIT) ? "A=7:0, R=14:10, G=9:5, B=4:0 - " : "",
		  (bits & AGL_RGB565_A8_BIT) ? "A=7:0, R=15:11, G=10:5, B=4:0" : "");
    if ((bits & AGL_RGB888_BIT) || (bits & AGL_RGB101010_BIT))
	s.sprintf("%s    32 RGB bit/pixel: %s%s\n", s.latin1(),
		  (bits & AGL_RGB888_BIT) ? "R=23:16, G=15:8, B=7:0 - " : "",
		  (bits & AGL_RGB101010_BIT) ? "R=29:20, G=19:10, B=9:0" : "" );
    if ((bits & AGL_ARGB8888_BIT) || (bits & AGL_ARGB2101010_BIT))
	s.sprintf("%s    32 ARGB bit/pixel: %s%s\n", s.latin1(),
		  (bits & AGL_ARGB8888_BIT) ? "A=31:24, R=23:16, G=15:8, B=7:0 - " : "",
		  (bits & AGL_ARGB2101010_BIT) ? "A=31:30, R=29:20, G=19:10, B=9:0" : "" );
    if ((bits & AGL_RGB888_A8_BIT) || (bits & AGL_RGB101010_A8_BIT))
	s.sprintf("%s    8-32 ARGB bit/pixel: %s%s\n", s.latin1(),
		  (bits & AGL_RGB888_A8_BIT) ? "A=7:0, R=23:16, G=15:8, B=7:0 - " : "",
	      (bits & AGL_RGB101010_A8_BIT) ? "A=7:0, R=29:20, G=19:10, B=9:0" : "");
    if (bits & AGL_RGB121212_BIT)
	s += "    48 RGB bit/pixel: R=35:24, G=23:12, B=11:0\n";
    if (bits & AGL_ARGB12121212_BIT)
	s += "    48 ARGB bit/pixel: A=47:36, R=35:24, G=23:12, B=11:0\n";
    if (bits & AGL_RGB161616_BIT)
	s += "    64 RGB bit/pixel: R=47:32, G=31:16, B=15:0\n";
    if (bits & AGL_ARGB16161616_BIT)
	s += "    64 ARGB bit/pixel: A=63:48, R=47:32, G=31:16, B=15:0\n";
    if (bits & AGL_INDEX8_BIT)
	s += "    8 bit color index: Yes\n";
    if (bits & AGL_INDEX16_BIT)
	s += "    16 bit color index: Yes\n";
    return s;
}


GLInfo::GLInfo()
{
    GLint major, minor;
    aglGetVersion(&major, &minor);
    infotext.sprintf("AGL Version: %d.%d\n\n", major, minor);

    GLint val = 0;
    QGLWidget gl((QWidget *) 0);
    GLint rid;
    AGLRendererInfo rf = aglQueryRendererInfo(NULL, 0), tf;
    do {
	aglDescribeRenderer(rf, AGL_RENDERER_ID, &rid);
	GLint attribs[] = {AGL_RGBA, AGL_RENDERER_ID, rid, AGL_NONE};
	AGLPixelFormat pf = aglChoosePixelFormat(NULL, 0, attribs);
	AGLContext glc = aglCreateContext(pf, NULL);
	aglSetDrawable(glc, GetWindowPort((WindowPtr) gl.handle()));
	aglSetCurrentContext(glc);
	infotext.sprintf("%sVendor string: %s\n"
			 "Renderer string: %s\n"
			 "Version string: %s\n"
			 "Renderer ID: 0x%08x\n", infotext.latin1(),
			 glGetString(GL_VENDOR),
			 glGetString(GL_RENDERER),
			 glGetString(GL_VERSION),
			 rid);
	aglDescribeRenderer(rf, AGL_OFFSCREEN, &val );
	infotext += QString("Off Screen: %1\n").arg(val ? "Yes" : "No");
	aglDescribeRenderer(rf, AGL_FULLSCREEN, &val );
	infotext += QString("Full Screen: %1\n").arg(val ? "Yes" : "No");
	aglDescribeRenderer(rf, AGL_WINDOW, &val );
	infotext += QString("Window: %1\n").arg(val ? "Yes" : "No");
	aglDescribeRenderer(rf, AGL_ACCELERATED, &val );
	infotext += QString("Hardware Accelerated: %1\n").arg(val ? "Yes" : "No");
	aglDescribeRenderer(rf, AGL_BACKING_STORE, &val );
	infotext += QString("Backing Store: %1\n").arg(val ? "Yes" : "No");
	aglDescribeRenderer(rf, AGL_ROBUST, &val );
	infotext += QString("Robust: %1\n").arg(val ? "Yes" : "No");
	aglDescribeRenderer(rf, AGL_MP_SAFE, &val );
	infotext += QString("MP Safe: %1\n").arg(val ? "Yes" : "No");
	aglDescribeRenderer(rf, AGL_COMPLIANT, &val );
	infotext += QString("Compliant: %1\n").arg(val ? "Yes" : "No");
	aglDescribeRenderer(rf, AGL_MULTISCREEN, &val );
	infotext += QString("Multi Screen: %1\n").arg(val ? "Yes" : "No");
	aglDescribeRenderer(rf, AGL_BUFFER_MODES, &val );
	infotext += QString("Buffer Modes: %1%2%3%4\n").arg(val & AGL_MONOSCOPIC_BIT ? "Monoscopic, " : "").arg(val & AGL_STEREOSCOPIC_BIT ? "Stereoscopic, " : "").arg(val & AGL_SINGLEBUFFER_BIT ? "Single Buffer, " : "").arg(val & AGL_DOUBLEBUFFER_BIT ? "Double Buffer" : "");
	aglDescribeRenderer(rf, AGL_MIN_LEVEL, &val );
	infotext += QString("Min Level: %1\n").arg(val);
	aglDescribeRenderer(rf, AGL_MAX_LEVEL, &val );
	infotext += QString("Max Level: %1\n").arg(val);
	aglDescribeRenderer(rf, AGL_COLOR_MODES, &val );
	infotext.sprintf("%sColor Modes: 0x%08x\n", infotext.latin1(), val);
	infotext += bitsToModes(val);
	aglDescribeRenderer(rf, AGL_ACCUM_MODES, &val );
	infotext.sprintf("%sAccum Modes: 0x%08x\n", infotext.latin1(), val);
	infotext += bitsToModes(val);
	infotext += "OpenGL extensions:\n    ";
	infotext += QString((const char *) glGetString(GL_EXTENSIONS)).replace(' ', "\n    ") + "\n";

	aglSetCurrentContext(NULL);
	aglSetDrawable(glc, NULL);
	aglDestroyContext(glc);
	aglDestroyPixelFormat(pf);
	tf = aglNextRendererInfo( rf );
	aglDestroyRendererInfo( rf );
	rf = tf;
    } while (rf != NULL);
}

QString GLInfo::info()
{
    return infotext;
}

