/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcolor_win.cpp#43 $
**
** Implementation of QColor class for Win32
**
** Created : 940112
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qcolor.h"
#include "qapplication.h"

#if defined(_CC_BOOL_DEF_)
#undef	bool
#include <windows.h>
#define bool int
#else
#include <windows.h>
#endif

/*****************************************************************************
  QColor static member functions
 *****************************************************************************/


HANDLE QColor::hpal = 0;			// application global palette

static int  current_alloc_context = 0;


int QColor::maxColors()
{
    static int maxcols = 0;
    if ( maxcols == 0 ) {
	HANDLE hdc = GetDC( 0 );
	if ( GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE )
	    maxcols = GetDeviceCaps( hdc, SIZEPALETTE );
	else
	    maxcols = GetDeviceCaps( hdc, NUMCOLORS );
	ReleaseDC( 0, hdc );
    }
    return maxcols;
}

int QColor::numBitPlanes()
{
    static int planes = 0;
    if ( planes == 0 ) {
	HANDLE hdc = GetDC( 0 );
	planes = GetDeviceCaps( hdc, BITSPIXEL );
	ReleaseDC( 0, hdc );
    }
    return planes;
}


void QColor::initialize()
{
    if ( color_init )
	return;

    color_init = TRUE;
    if ( QApplication::colorSpec() == QApplication::NormalColor )
	return;

    int numCols = maxColors();
    if ( numCols <= 16 || numCols > 256 )	// no need to create palette
	return;

    static struct {
	WORD	     palVersion;
	WORD	     palNumEntries;
	BYTE         palPalEntries[1024];
    } rgb8palette = {
	0x300,
	256, {
	  0,  0,  0,  0,  63,  0,  0,  0, 104,  0,  0,  0, 128,  0,  0,  0,
	171,  0,  0,  0, 200,  0,  0,  0, 229,  0,  0,  0, 255,  0,  0,  0,
	  0, 63,  0,  0,  63, 63,  0,  0, 104, 63,  0,  0, 139, 63,  0,  0,
	171, 63,  0,  0, 200, 63,  0,  0, 229, 63,  0,  0, 255, 63,  0,  0,
	  0,104,  0,  0,  63,104,  0,  0, 104,104,  0,  0, 139,104,  0,  0,
	171,104,  0,  0, 200,104,  0,  0, 229,104,  0,  0, 255,104,  0,  0,
	  0,128,  0,  0,  63,139,  0,  0, 104,139,  0,  0, 128,128,  0,  0,
	171,139,  0,  0, 200,139,  0,  0, 229,139,  0,  0, 255,139,  0,  0,
	  0,171,  0,  0,  63,171,  0,  0, 104,171,  0,  0, 139,171,  0,  0,
	171,171,  0,  0, 200,171,  0,  0, 229,171,  0,  0, 255,171,  0,  0,
	  0,200,  0,  0,  63,200,  0,  0, 104,200,  0,  0, 139,200,  0,  0,
	171,200,  0,  0, 200,200,  0,  0, 229,200,  0,  0, 255,200,  0,  0,
	  0,229,  0,  0,  63,229,  0,  0, 104,229,  0,  0, 139,229,  0,  0,
	171,229,  0,  0, 200,229,  0,  0, 229,229,  0,  0, 255,229,  0,  0,
	  0,255,  0,  0,  63,255,  0,  0, 104,255,  0,  0, 139,255,  0,  0,
	171,255,  0,  0, 200,255,  0,  0, 229,255,  0,  0, 255,255,  0,  0,
	  0,  0,128,  0,  63,  0,116,  0, 104,  0,116,  0, 128,  0,128,  0,
	171,  0,116,  0, 200,  0,116,  0, 229,  0,116,  0, 255,  0,116,  0,
	  0, 63,116,  0,  63, 63,116,  0, 104, 63,116,  0, 139, 63,116,  0,
	171, 63,116,  0, 200, 63,116,  0, 229, 63,116,  0, 255, 63,116,  0,
	  0,104,116,  0,  63,104,116,  0, 104,104,116,  0, 139,104,116,  0,
	171,104,116,  0, 200,104,116,  0, 229,104,116,  0, 255,104,116,  0,
	  0,128,128,  0,  63,139,116,  0, 104,139,116,  0, 128,128,128,  0,
	171,139,116,  0, 200,139,116,  0, 229,139,116,  0, 255,139,116,  0,
	  0,171,116,  0,  63,171,116,  0, 104,171,116,  0, 139,171,116,  0,
	171,171,116,  0, 200,171,116,  0, 229,171,116,  0, 255,171,116,  0,
	  0,200,116,  0,  63,200,116,  0, 104,200,116,  0, 139,200,116,  0,
	171,200,116,  0, 200,200,116,  0, 229,200,116,  0, 255,200,116,  0,
	  0,229,116,  0,  63,229,116,  0, 104,229,116,  0, 139,229,116,  0,
	171,229,116,  0, 200,229,116,  0, 229,229,116,  0, 255,229,116,  0,
	  0,255,116,  0,  63,255,116,  0, 104,255,116,  0, 139,255,116,  0,
	171,255,116,  0, 200,255,116,  0, 229,255,116,  0, 255,255,116,  0,
	  0,  0,191,  0,  63,  0,191,  0, 104,  0,191,  0, 139,  0,191,  0,
	171,  0,191,  0, 200,  0,191,  0, 229,  0,191,  0, 255,  0,191,  0,
	  0, 63,191,  0,  63, 63,191,  0, 104, 63,191,  0, 139, 63,191,  0,
	171, 63,191,  0, 200, 63,191,  0, 229, 63,191,  0, 255, 63,191,  0,
	  0,104,191,  0,  63,104,191,  0, 104,104,191,  0, 139,104,191,  0,
	171,104,191,  0, 200,104,191,  0, 229,104,191,  0, 255,104,191,  0,
	  0,139,191,  0,  63,139,191,  0, 104,139,191,  0, 139,139,191,  0,
	171,139,191,  0, 200,139,191,  0, 229,139,191,  0, 255,139,191,  0,
	  0,171,191,  0,  63,171,191,  0, 104,171,191,  0, 139,171,191,  0,
	160,160,164,  0, 200,171,191,  0, 229,171,191,  0, 255,171,191,  0,
	  0,200,191,  0,  63,200,191,  0, 104,200,191,  0, 139,200,191,  0,
	171,200,191,  0, 192,192,192,  0, 229,200,191,  0, 255,200,191,  0,
	  0,229,191,  0,  63,229,191,  0, 104,229,191,  0, 139,229,191,  0,
	171,229,191,  0, 192,220,192,  0, 229,229,191,  0, 255,229,191,  0,
	  0,255,191,  0,  63,255,191,  0, 104,255,191,  0, 139,255,191,  0,
	171,255,191,  0, 200,255,191,  0, 229,255,191,  0, 255,255,191,  0,
	  0,  0,255,  0,  63,  0,255,  0, 104,  0,255,  0, 139,  0,255,  0,
	171,  0,255,  0, 200,  0,255,  0, 229,  0,255,  0, 255,  0,255,  0,
	  0, 63,255,  0,  63, 63,255,  0, 104, 63,255,  0, 139, 63,255,  0,
	171, 63,255,  0, 200, 63,255,  0, 229, 63,255,  0, 255, 63,255,  0,
	  0,104,255,  0,  63,104,255,  0, 104,104,255,  0, 139,104,255,  0,
	171,104,255,  0, 200,104,255,  0, 229,104,255,  0, 255,104,255,  0,
	  0,139,255,  0,  63,139,255,  0, 104,139,255,  0, 139,139,255,  0,
	171,139,255,  0, 200,139,255,  0, 229,139,255,  0, 255,139,255,  0,
	  0,171,255,  0,  63,171,255,  0, 104,171,255,  0, 139,171,255,  0,
	171,171,255,  0, 200,171,255,  0, 229,171,255,  0, 255,171,255,  0,
	  0,200,255,  0,  63,200,255,  0, 104,200,255,  0, 139,200,255,  0,
	166,202,240,  0, 200,200,255,  0, 229,200,255,  0, 255,200,255,  0,
	  0,229,255,  0,  63,229,255,  0, 104,229,255,  0, 139,229,255,  0,
	171,229,255,  0, 200,229,255,  0, 229,229,255,  0, 255,251,240,  0,
	  0,255,255,  0,  63,255,255,  0, 104,255,255,  0, 139,255,255,  0,
	171,255,255,  0, 200,255,255,  0, 229,255,255,  0, 255,255,255,  0 } };

    hpal = CreatePalette( (LOGPALETTE*)&rgb8palette );

    ((QColor*)(&black))->   alloc();
    ((QColor*)(&white))->   alloc();
    ((QColor*)(&::red))->   alloc();
    ((QColor*)(&::green))-> alloc();
    ((QColor*)(&::blue))->  alloc();
}


void QColor::cleanup()
{
    if ( hpal ) {				// delete application global
	DeleteObject( hpal );			//   palette
	hpal = 0;
    }
    color_init = FALSE;
}


uint QColor::realizePal( QWidget *widget )
{
    if ( !hpal )				// not using palette
	return 0;
    HDC hdc = GetDC( widget->winId() );
    HPALETTE hpalT = SelectPalette( hdc, hpal, FALSE );
    uint i = RealizePalette( hdc );
    UpdateColors( hdc );
    SelectPalette( hdc, hpalT, FALSE );
    ReleaseDC( widget->winId(), hdc );
    return i;
}


/*****************************************************************************
  QColor member functions
 *****************************************************************************/

uint QColor::alloc()
{
    if ( (rgbVal & RGB_INVALID) || !color_init ) {
	rgbVal = 0;				// invalid color or state
	pix = 0;
    } else {
	rgbVal &= RGB_MASK;
	pix = hpal ? PALETTEINDEX(GetNearestPaletteIndex(hpal,rgbVal)) :rgbVal;
    }
    return pix;
}


/****************************************************************************
** Color lookup based on a name.  The color names have been borrowed from X.
*****************************************************************************/

#if !defined(NO_COLORNAMES)

#include <stdlib.h>

#undef RGB
#define RGB(r,g,b) (r+g*256+b*65536)

const int rgbTblSize = 657;

struct RGBData {
    uint  value;
    char *name;
} rgbTbl[] = {
  { RGB(240,248,255),	"aliceblue" },
  { RGB(250,235,215),	"antiquewhite" },
  { RGB(255,239,219),	"antiquewhite1" },
  { RGB(238,223,204),	"antiquewhite2" },
  { RGB(205,192,176),	"antiquewhite3" },
  { RGB(139,131,120),	"antiquewhite4" },
  { RGB(127,255,212),	"aquamarine" },
  { RGB(127,255,212),	"aquamarine1" },
  { RGB(118,238,198),	"aquamarine2" },
  { RGB(102,205,170),	"aquamarine3" },
  { RGB( 69,139,116),	"aquamarine4" },
  { RGB(240,255,255),	"azure" },
  { RGB(240,255,255),	"azure1" },
  { RGB(224,238,238),	"azure2" },
  { RGB(193,205,205),	"azure3" },
  { RGB(131,139,139),	"azure4" },
  { RGB(245,245,220),	"beige" },
  { RGB(255,228,196),	"bisque" },
  { RGB(255,228,196),	"bisque1" },
  { RGB(238,213,183),	"bisque2" },
  { RGB(205,183,158),	"bisque3" },
  { RGB(139,125,107),	"bisque4" },
  { RGB(  0,  0,  0),	"black" },
  { RGB(255,235,205),	"blanchedalmond" },
  { RGB(  0,  0,255),	"blue" },
  { RGB(  0,  0,255),	"blue1" },
  { RGB(  0,  0,238),	"blue2" },
  { RGB(  0,  0,205),	"blue3" },
  { RGB(  0,  0,139),	"blue4" },
  { RGB(138, 43,226),	"blueviolet" },
  { RGB(165, 42, 42),	"brown" },
  { RGB(255, 64, 64),	"brown1" },
  { RGB(238, 59, 59),	"brown2" },
  { RGB(205, 51, 51),	"brown3" },
  { RGB(139, 35, 35),	"brown4" },
  { RGB(222,184,135),	"burlywood" },
  { RGB(255,211,155),	"burlywood1" },
  { RGB(238,197,145),	"burlywood2" },
  { RGB(205,170,125),	"burlywood3" },
  { RGB(139,115, 85),	"burlywood4" },
  { RGB( 95,158,160),	"cadetblue" },
  { RGB(152,245,255),	"cadetblue1" },
  { RGB(142,229,238),	"cadetblue2" },
  { RGB(122,197,205),	"cadetblue3" },
  { RGB( 83,134,139),	"cadetblue4" },
  { RGB(127,255,  0),	"chartreuse" },
  { RGB(127,255,  0),	"chartreuse1" },
  { RGB(118,238,  0),	"chartreuse2" },
  { RGB(102,205,  0),	"chartreuse3" },
  { RGB( 69,139,  0),	"chartreuse4" },
  { RGB(210,105, 30),	"chocolate" },
  { RGB(255,127, 36),	"chocolate1" },
  { RGB(238,118, 33),	"chocolate2" },
  { RGB(205,102, 29),	"chocolate3" },
  { RGB(139, 69, 19),	"chocolate4" },
  { RGB(255,127, 80),	"coral" },
  { RGB(255,114, 86),	"coral1" },
  { RGB(238,106, 80),	"coral2" },
  { RGB(205, 91, 69),	"coral3" },
  { RGB(139, 62, 47),	"coral4" },
  { RGB(100,149,237),	"cornflowerblue" },
  { RGB(255,248,220),	"cornsilk" },
  { RGB(255,248,220),	"cornsilk1" },
  { RGB(238,232,205),	"cornsilk2" },
  { RGB(205,200,177),	"cornsilk3" },
  { RGB(139,136,120),	"cornsilk4" },
  { RGB(  0,255,255),	"cyan" },
  { RGB(  0,255,255),	"cyan1" },
  { RGB(  0,238,238),	"cyan2" },
  { RGB(  0,205,205),	"cyan3" },
  { RGB(  0,139,139),	"cyan4" },
  { RGB(  0,  0,139),	"darkblue" },
  { RGB(  0,139,139),	"darkcyan" },
  { RGB(184,134, 11),	"darkgoldenrod" },
  { RGB(255,185, 15),	"darkgoldenrod1" },
  { RGB(238,173, 14),	"darkgoldenrod2" },
  { RGB(205,149, 12),	"darkgoldenrod3" },
  { RGB(139,101,  8),	"darkgoldenrod4" },
  { RGB(169,169,169),	"darkgray" },
  { RGB(  0,100,  0),	"darkgreen" },
  { RGB(169,169,169),	"darkgrey" },
  { RGB(189,183,107),	"darkkhaki" },
  { RGB(139,  0,139),	"darkmagenta" },
  { RGB( 85,107, 47),	"darkolivegreen" },
  { RGB(202,255,112),	"darkolivegreen1" },
  { RGB(188,238,104),	"darkolivegreen2" },
  { RGB(162,205, 90),	"darkolivegreen3" },
  { RGB(110,139, 61),	"darkolivegreen4" },
  { RGB(255,140,  0),	"darkorange" },
  { RGB(255,127,  0),	"darkorange1" },
  { RGB(238,118,  0),	"darkorange2" },
  { RGB(205,102,  0),	"darkorange3" },
  { RGB(139, 69,  0),	"darkorange4" },
  { RGB(153, 50,204),	"darkorchid" },
  { RGB(191, 62,255),	"darkorchid1" },
  { RGB(178, 58,238),	"darkorchid2" },
  { RGB(154, 50,205),	"darkorchid3" },
  { RGB(104, 34,139),	"darkorchid4" },
  { RGB(139,  0,  0),	"darkred" },
  { RGB(233,150,122),	"darksalmon" },
  { RGB(143,188,143),	"darkseagreen" },
  { RGB(193,255,193),	"darkseagreen1" },
  { RGB(180,238,180),	"darkseagreen2" },
  { RGB(155,205,155),	"darkseagreen3" },
  { RGB(105,139,105),	"darkseagreen4" },
  { RGB( 72, 61,139),	"darkslateblue" },
  { RGB( 47, 79, 79),	"darkslategray" },
  { RGB(151,255,255),	"darkslategray1" },
  { RGB(141,238,238),	"darkslategray2" },
  { RGB(121,205,205),	"darkslategray3" },
  { RGB( 82,139,139),	"darkslategray4" },
  { RGB( 47, 79, 79),	"darkslategrey" },
  { RGB(  0,206,209),	"darkturquoise" },
  { RGB(148,  0,211),	"darkviolet" },
  { RGB(255, 20,147),	"deeppink" },
  { RGB(255, 20,147),	"deeppink1" },
  { RGB(238, 18,137),	"deeppink2" },
  { RGB(205, 16,118),	"deeppink3" },
  { RGB(139, 10, 80),	"deeppink4" },
  { RGB(  0,191,255),	"deepskyblue" },
  { RGB(  0,191,255),	"deepskyblue1" },
  { RGB(  0,178,238),	"deepskyblue2" },
  { RGB(  0,154,205),	"deepskyblue3" },
  { RGB(  0,104,139),	"deepskyblue4" },
  { RGB(105,105,105),	"dimgray" },
  { RGB(105,105,105),	"dimgrey" },
  { RGB( 30,144,255),	"dodgerblue" },
  { RGB( 30,144,255),	"dodgerblue1" },
  { RGB( 28,134,238),	"dodgerblue2" },
  { RGB( 24,116,205),	"dodgerblue3" },
  { RGB( 16, 78,139),	"dodgerblue4" },
  { RGB(178, 34, 34),	"firebrick" },
  { RGB(255, 48, 48),	"firebrick1" },
  { RGB(238, 44, 44),	"firebrick2" },
  { RGB(205, 38, 38),	"firebrick3" },
  { RGB(139, 26, 26),	"firebrick4" },
  { RGB(255,250,240),	"floralwhite" },
  { RGB( 34,139, 34),	"forestgreen" },
  { RGB(220,220,220),	"gainsboro" },
  { RGB(248,248,255),	"ghostwhite" },
  { RGB(255,215,  0),	"gold" },
  { RGB(255,215,  0),	"gold1" },
  { RGB(238,201,  0),	"gold2" },
  { RGB(205,173,  0),	"gold3" },
  { RGB(139,117,  0),	"gold4" },
  { RGB(218,165, 32),	"goldenrod" },
  { RGB(255,193, 37),	"goldenrod1" },
  { RGB(238,180, 34),	"goldenrod2" },
  { RGB(205,155, 29),	"goldenrod3" },
  { RGB(139,105, 20),	"goldenrod4" },
  { RGB(190,190,190),	"gray" },
  { RGB(  0,  0,  0),	"gray0" },
  { RGB(  3,  3,  3),	"gray1" },
  { RGB( 26, 26, 26),	"gray10" },
  { RGB(255,255,255),	"gray100" },
  { RGB( 28, 28, 28),	"gray11" },
  { RGB( 31, 31, 31),	"gray12" },
  { RGB( 33, 33, 33),	"gray13" },
  { RGB( 36, 36, 36),	"gray14" },
  { RGB( 38, 38, 38),	"gray15" },
  { RGB( 41, 41, 41),	"gray16" },
  { RGB( 43, 43, 43),	"gray17" },
  { RGB( 46, 46, 46),	"gray18" },
  { RGB( 48, 48, 48),	"gray19" },
  { RGB(  5,  5,  5),	"gray2" },
  { RGB( 51, 51, 51),	"gray20" },
  { RGB( 54, 54, 54),	"gray21" },
  { RGB( 56, 56, 56),	"gray22" },
  { RGB( 59, 59, 59),	"gray23" },
  { RGB( 61, 61, 61),	"gray24" },
  { RGB( 64, 64, 64),	"gray25" },
  { RGB( 66, 66, 66),	"gray26" },
  { RGB( 69, 69, 69),	"gray27" },
  { RGB( 71, 71, 71),	"gray28" },
  { RGB( 74, 74, 74),	"gray29" },
  { RGB(  8,  8,  8),	"gray3" },
  { RGB( 77, 77, 77),	"gray30" },
  { RGB( 79, 79, 79),	"gray31" },
  { RGB( 82, 82, 82),	"gray32" },
  { RGB( 84, 84, 84),	"gray33" },
  { RGB( 87, 87, 87),	"gray34" },
  { RGB( 89, 89, 89),	"gray35" },
  { RGB( 92, 92, 92),	"gray36" },
  { RGB( 94, 94, 94),	"gray37" },
  { RGB( 97, 97, 97),	"gray38" },
  { RGB( 99, 99, 99),	"gray39" },
  { RGB( 10, 10, 10),	"gray4" },
  { RGB(102,102,102),	"gray40" },
  { RGB(105,105,105),	"gray41" },
  { RGB(107,107,107),	"gray42" },
  { RGB(110,110,110),	"gray43" },
  { RGB(112,112,112),	"gray44" },
  { RGB(115,115,115),	"gray45" },
  { RGB(117,117,117),	"gray46" },
  { RGB(120,120,120),	"gray47" },
  { RGB(122,122,122),	"gray48" },
  { RGB(125,125,125),	"gray49" },
  { RGB( 13, 13, 13),	"gray5" },
  { RGB(127,127,127),	"gray50" },
  { RGB(130,130,130),	"gray51" },
  { RGB(133,133,133),	"gray52" },
  { RGB(135,135,135),	"gray53" },
  { RGB(138,138,138),	"gray54" },
  { RGB(140,140,140),	"gray55" },
  { RGB(143,143,143),	"gray56" },
  { RGB(145,145,145),	"gray57" },
  { RGB(148,148,148),	"gray58" },
  { RGB(150,150,150),	"gray59" },
  { RGB( 15, 15, 15),	"gray6" },
  { RGB(153,153,153),	"gray60" },
  { RGB(156,156,156),	"gray61" },
  { RGB(158,158,158),	"gray62" },
  { RGB(161,161,161),	"gray63" },
  { RGB(163,163,163),	"gray64" },
  { RGB(166,166,166),	"gray65" },
  { RGB(168,168,168),	"gray66" },
  { RGB(171,171,171),	"gray67" },
  { RGB(173,173,173),	"gray68" },
  { RGB(176,176,176),	"gray69" },
  { RGB( 18, 18, 18),	"gray7" },
  { RGB(179,179,179),	"gray70" },
  { RGB(181,181,181),	"gray71" },
  { RGB(184,184,184),	"gray72" },
  { RGB(186,186,186),	"gray73" },
  { RGB(189,189,189),	"gray74" },
  { RGB(191,191,191),	"gray75" },
  { RGB(194,194,194),	"gray76" },
  { RGB(196,196,196),	"gray77" },
  { RGB(199,199,199),	"gray78" },
  { RGB(201,201,201),	"gray79" },
  { RGB( 20, 20, 20),	"gray8" },
  { RGB(204,204,204),	"gray80" },
  { RGB(207,207,207),	"gray81" },
  { RGB(209,209,209),	"gray82" },
  { RGB(212,212,212),	"gray83" },
  { RGB(214,214,214),	"gray84" },
  { RGB(217,217,217),	"gray85" },
  { RGB(219,219,219),	"gray86" },
  { RGB(222,222,222),	"gray87" },
  { RGB(224,224,224),	"gray88" },
  { RGB(227,227,227),	"gray89" },
  { RGB( 23, 23, 23),	"gray9" },
  { RGB(229,229,229),	"gray90" },
  { RGB(232,232,232),	"gray91" },
  { RGB(235,235,235),	"gray92" },
  { RGB(237,237,237),	"gray93" },
  { RGB(240,240,240),	"gray94" },
  { RGB(242,242,242),	"gray95" },
  { RGB(245,245,245),	"gray96" },
  { RGB(247,247,247),	"gray97" },
  { RGB(250,250,250),	"gray98" },
  { RGB(252,252,252),	"gray99" },
  { RGB(  0,255,  0),	"green" },
  { RGB(  0,255,  0),	"green1" },
  { RGB(  0,238,  0),	"green2" },
  { RGB(  0,205,  0),	"green3" },
  { RGB(  0,139,  0),	"green4" },
  { RGB(173,255, 47),	"greenyellow" },
  { RGB(190,190,190),	"grey" },
  { RGB(  0,  0,  0),	"grey0" },
  { RGB(  3,  3,  3),	"grey1" },
  { RGB( 26, 26, 26),	"grey10" },
  { RGB(255,255,255),	"grey100" },
  { RGB( 28, 28, 28),	"grey11" },
  { RGB( 31, 31, 31),	"grey12" },
  { RGB( 33, 33, 33),	"grey13" },
  { RGB( 36, 36, 36),	"grey14" },
  { RGB( 38, 38, 38),	"grey15" },
  { RGB( 41, 41, 41),	"grey16" },
  { RGB( 43, 43, 43),	"grey17" },
  { RGB( 46, 46, 46),	"grey18" },
  { RGB( 48, 48, 48),	"grey19" },
  { RGB(  5,  5,  5),	"grey2" },
  { RGB( 51, 51, 51),	"grey20" },
  { RGB( 54, 54, 54),	"grey21" },
  { RGB( 56, 56, 56),	"grey22" },
  { RGB( 59, 59, 59),	"grey23" },
  { RGB( 61, 61, 61),	"grey24" },
  { RGB( 64, 64, 64),	"grey25" },
  { RGB( 66, 66, 66),	"grey26" },
  { RGB( 69, 69, 69),	"grey27" },
  { RGB( 71, 71, 71),	"grey28" },
  { RGB( 74, 74, 74),	"grey29" },
  { RGB(  8,  8,  8),	"grey3" },
  { RGB( 77, 77, 77),	"grey30" },
  { RGB( 79, 79, 79),	"grey31" },
  { RGB( 82, 82, 82),	"grey32" },
  { RGB( 84, 84, 84),	"grey33" },
  { RGB( 87, 87, 87),	"grey34" },
  { RGB( 89, 89, 89),	"grey35" },
  { RGB( 92, 92, 92),	"grey36" },
  { RGB( 94, 94, 94),	"grey37" },
  { RGB( 97, 97, 97),	"grey38" },
  { RGB( 99, 99, 99),	"grey39" },
  { RGB( 10, 10, 10),	"grey4" },
  { RGB(102,102,102),	"grey40" },
  { RGB(105,105,105),	"grey41" },
  { RGB(107,107,107),	"grey42" },
  { RGB(110,110,110),	"grey43" },
  { RGB(112,112,112),	"grey44" },
  { RGB(115,115,115),	"grey45" },
  { RGB(117,117,117),	"grey46" },
  { RGB(120,120,120),	"grey47" },
  { RGB(122,122,122),	"grey48" },
  { RGB(125,125,125),	"grey49" },
  { RGB( 13, 13, 13),	"grey5" },
  { RGB(127,127,127),	"grey50" },
  { RGB(130,130,130),	"grey51" },
  { RGB(133,133,133),	"grey52" },
  { RGB(135,135,135),	"grey53" },
  { RGB(138,138,138),	"grey54" },
  { RGB(140,140,140),	"grey55" },
  { RGB(143,143,143),	"grey56" },
  { RGB(145,145,145),	"grey57" },
  { RGB(148,148,148),	"grey58" },
  { RGB(150,150,150),	"grey59" },
  { RGB( 15, 15, 15),	"grey6" },
  { RGB(153,153,153),	"grey60" },
  { RGB(156,156,156),	"grey61" },
  { RGB(158,158,158),	"grey62" },
  { RGB(161,161,161),	"grey63" },
  { RGB(163,163,163),	"grey64" },
  { RGB(166,166,166),	"grey65" },
  { RGB(168,168,168),	"grey66" },
  { RGB(171,171,171),	"grey67" },
  { RGB(173,173,173),	"grey68" },
  { RGB(176,176,176),	"grey69" },
  { RGB( 18, 18, 18),	"grey7" },
  { RGB(179,179,179),	"grey70" },
  { RGB(181,181,181),	"grey71" },
  { RGB(184,184,184),	"grey72" },
  { RGB(186,186,186),	"grey73" },
  { RGB(189,189,189),	"grey74" },
  { RGB(191,191,191),	"grey75" },
  { RGB(194,194,194),	"grey76" },
  { RGB(196,196,196),	"grey77" },
  { RGB(199,199,199),	"grey78" },
  { RGB(201,201,201),	"grey79" },
  { RGB( 20, 20, 20),	"grey8" },
  { RGB(204,204,204),	"grey80" },
  { RGB(207,207,207),	"grey81" },
  { RGB(209,209,209),	"grey82" },
  { RGB(212,212,212),	"grey83" },
  { RGB(214,214,214),	"grey84" },
  { RGB(217,217,217),	"grey85" },
  { RGB(219,219,219),	"grey86" },
  { RGB(222,222,222),	"grey87" },
  { RGB(224,224,224),	"grey88" },
  { RGB(227,227,227),	"grey89" },
  { RGB( 23, 23, 23),	"grey9" },
  { RGB(229,229,229),	"grey90" },
  { RGB(232,232,232),	"grey91" },
  { RGB(235,235,235),	"grey92" },
  { RGB(237,237,237),	"grey93" },
  { RGB(240,240,240),	"grey94" },
  { RGB(242,242,242),	"grey95" },
  { RGB(245,245,245),	"grey96" },
  { RGB(247,247,247),	"grey97" },
  { RGB(250,250,250),	"grey98" },
  { RGB(252,252,252),	"grey99" },
  { RGB(240,255,240),	"honeydew" },
  { RGB(240,255,240),	"honeydew1" },
  { RGB(224,238,224),	"honeydew2" },
  { RGB(193,205,193),	"honeydew3" },
  { RGB(131,139,131),	"honeydew4" },
  { RGB(255,105,180),	"hotpink" },
  { RGB(255,110,180),	"hotpink1" },
  { RGB(238,106,167),	"hotpink2" },
  { RGB(205, 96,144),	"hotpink3" },
  { RGB(139, 58, 98),	"hotpink4" },
  { RGB(205, 92, 92),	"indianred" },
  { RGB(255,106,106),	"indianred1" },
  { RGB(238, 99, 99),	"indianred2" },
  { RGB(205, 85, 85),	"indianred3" },
  { RGB(139, 58, 58),	"indianred4" },
  { RGB(255,255,240),	"ivory" },
  { RGB(255,255,240),	"ivory1" },
  { RGB(238,238,224),	"ivory2" },
  { RGB(205,205,193),	"ivory3" },
  { RGB(139,139,131),	"ivory4" },
  { RGB(240,230,140),	"khaki" },
  { RGB(255,246,143),	"khaki1" },
  { RGB(238,230,133),	"khaki2" },
  { RGB(205,198,115),	"khaki3" },
  { RGB(139,134, 78),	"khaki4" },
  { RGB(230,230,250),	"lavender" },
  { RGB(255,240,245),	"lavenderblush" },
  { RGB(255,240,245),	"lavenderblush1" },
  { RGB(238,224,229),	"lavenderblush2" },
  { RGB(205,193,197),	"lavenderblush3" },
  { RGB(139,131,134),	"lavenderblush4" },
  { RGB(124,252,  0),	"lawngreen" },
  { RGB(255,250,205),	"lemonchiffon" },
  { RGB(255,250,205),	"lemonchiffon1" },
  { RGB(238,233,191),	"lemonchiffon2" },
  { RGB(205,201,165),	"lemonchiffon3" },
  { RGB(139,137,112),	"lemonchiffon4" },
  { RGB(173,216,230),	"lightblue" },
  { RGB(191,239,255),	"lightblue1" },
  { RGB(178,223,238),	"lightblue2" },
  { RGB(154,192,205),	"lightblue3" },
  { RGB(104,131,139),	"lightblue4" },
  { RGB(240,128,128),	"lightcoral" },
  { RGB(224,255,255),	"lightcyan" },
  { RGB(224,255,255),	"lightcyan1" },
  { RGB(209,238,238),	"lightcyan2" },
  { RGB(180,205,205),	"lightcyan3" },
  { RGB(122,139,139),	"lightcyan4" },
  { RGB(238,221,130),	"lightgoldenrod" },
  { RGB(255,236,139),	"lightgoldenrod1" },
  { RGB(238,220,130),	"lightgoldenrod2" },
  { RGB(205,190,112),	"lightgoldenrod3" },
  { RGB(139,129, 76),	"lightgoldenrod4" },
  { RGB(250,250,210),	"lightgoldenrodyellow" },
  { RGB(211,211,211),	"lightgray" },
  { RGB(144,238,144),	"lightgreen" },
  { RGB(211,211,211),	"lightgrey" },
  { RGB(255,182,193),	"lightpink" },
  { RGB(255,174,185),	"lightpink1" },
  { RGB(238,162,173),	"lightpink2" },
  { RGB(205,140,149),	"lightpink3" },
  { RGB(139, 95,101),	"lightpink4" },
  { RGB(255,160,122),	"lightsalmon" },
  { RGB(255,160,122),	"lightsalmon1" },
  { RGB(238,149,114),	"lightsalmon2" },
  { RGB(205,129, 98),	"lightsalmon3" },
  { RGB(139, 87, 66),	"lightsalmon4" },
  { RGB( 32,178,170),	"lightseagreen" },
  { RGB(135,206,250),	"lightskyblue" },
  { RGB(176,226,255),	"lightskyblue1" },
  { RGB(164,211,238),	"lightskyblue2" },
  { RGB(141,182,205),	"lightskyblue3" },
  { RGB( 96,123,139),	"lightskyblue4" },
  { RGB(132,112,255),	"lightslateblue" },
  { RGB(119,136,153),	"lightslategray" },
  { RGB(119,136,153),	"lightslategrey" },
  { RGB(176,196,222),	"lightsteelblue" },
  { RGB(202,225,255),	"lightsteelblue1" },
  { RGB(188,210,238),	"lightsteelblue2" },
  { RGB(162,181,205),	"lightsteelblue3" },
  { RGB(110,123,139),	"lightsteelblue4" },
  { RGB(255,255,224),	"lightyellow" },
  { RGB(255,255,224),	"lightyellow1" },
  { RGB(238,238,209),	"lightyellow2" },
  { RGB(205,205,180),	"lightyellow3" },
  { RGB(139,139,122),	"lightyellow4" },
  { RGB( 50,205, 50),	"limegreen" },
  { RGB(250,240,230),	"linen" },
  { RGB(255,  0,255),	"magenta" },
  { RGB(255,  0,255),	"magenta1" },
  { RGB(238,  0,238),	"magenta2" },
  { RGB(205,  0,205),	"magenta3" },
  { RGB(139,  0,139),	"magenta4" },
  { RGB(176, 48, 96),	"maroon" },
  { RGB(255, 52,179),	"maroon1" },
  { RGB(238, 48,167),	"maroon2" },
  { RGB(205, 41,144),	"maroon3" },
  { RGB(139, 28, 98),	"maroon4" },
  { RGB(102,205,170),	"mediumaquamarine" },
  { RGB(  0,  0,205),	"mediumblue" },
  { RGB(186, 85,211),	"mediumorchid" },
  { RGB(224,102,255),	"mediumorchid1" },
  { RGB(209, 95,238),	"mediumorchid2" },
  { RGB(180, 82,205),	"mediumorchid3" },
  { RGB(122, 55,139),	"mediumorchid4" },
  { RGB(147,112,219),	"mediumpurple" },
  { RGB(171,130,255),	"mediumpurple1" },
  { RGB(159,121,238),	"mediumpurple2" },
  { RGB(137,104,205),	"mediumpurple3" },
  { RGB( 93, 71,139),	"mediumpurple4" },
  { RGB( 60,179,113),	"mediumseagreen" },
  { RGB(123,104,238),	"mediumslateblue" },
  { RGB(  0,250,154),	"mediumspringgreen" },
  { RGB( 72,209,204),	"mediumturquoise" },
  { RGB(199, 21,133),	"mediumvioletred" },
  { RGB( 25, 25,112),	"midnightblue" },
  { RGB(245,255,250),	"mintcream" },
  { RGB(255,228,225),	"mistyrose" },
  { RGB(255,228,225),	"mistyrose1" },
  { RGB(238,213,210),	"mistyrose2" },
  { RGB(205,183,181),	"mistyrose3" },
  { RGB(139,125,123),	"mistyrose4" },
  { RGB(255,228,181),	"moccasin" },
  { RGB(255,222,173),	"navajowhite" },
  { RGB(255,222,173),	"navajowhite1" },
  { RGB(238,207,161),	"navajowhite2" },
  { RGB(205,179,139),	"navajowhite3" },
  { RGB(139,121, 94),	"navajowhite4" },
  { RGB(  0,  0,128),	"navy" },
  { RGB(  0,  0,128),	"navyblue" },
  { RGB(253,245,230),	"oldlace" },
  { RGB(107,142, 35),	"olivedrab" },
  { RGB(192,255, 62),	"olivedrab1" },
  { RGB(179,238, 58),	"olivedrab2" },
  { RGB(154,205, 50),	"olivedrab3" },
  { RGB(105,139, 34),	"olivedrab4" },
  { RGB(255,165,  0),	"orange" },
  { RGB(255,165,  0),	"orange1" },
  { RGB(238,154,  0),	"orange2" },
  { RGB(205,133,  0),	"orange3" },
  { RGB(139, 90,  0),	"orange4" },
  { RGB(255, 69,  0),	"orangered" },
  { RGB(255, 69,  0),	"orangered1" },
  { RGB(238, 64,  0),	"orangered2" },
  { RGB(205, 55,  0),	"orangered3" },
  { RGB(139, 37,  0),	"orangered4" },
  { RGB(218,112,214),	"orchid" },
  { RGB(255,131,250),	"orchid1" },
  { RGB(238,122,233),	"orchid2" },
  { RGB(205,105,201),	"orchid3" },
  { RGB(139, 71,137),	"orchid4" },
  { RGB(238,232,170),	"palegoldenrod" },
  { RGB(152,251,152),	"palegreen" },
  { RGB(154,255,154),	"palegreen1" },
  { RGB(144,238,144),	"palegreen2" },
  { RGB(124,205,124),	"palegreen3" },
  { RGB( 84,139, 84),	"palegreen4" },
  { RGB(175,238,238),	"paleturquoise" },
  { RGB(187,255,255),	"paleturquoise1" },
  { RGB(174,238,238),	"paleturquoise2" },
  { RGB(150,205,205),	"paleturquoise3" },
  { RGB(102,139,139),	"paleturquoise4" },
  { RGB(219,112,147),	"palevioletred" },
  { RGB(255,130,171),	"palevioletred1" },
  { RGB(238,121,159),	"palevioletred2" },
  { RGB(205,104,137),	"palevioletred3" },
  { RGB(139, 71, 93),	"palevioletred4" },
  { RGB(255,239,213),	"papayawhip" },
  { RGB(255,218,185),	"peachpuff" },
  { RGB(255,218,185),	"peachpuff1" },
  { RGB(238,203,173),	"peachpuff2" },
  { RGB(205,175,149),	"peachpuff3" },
  { RGB(139,119,101),	"peachpuff4" },
  { RGB(205,133, 63),	"peru" },
  { RGB(255,192,203),	"pink" },
  { RGB(255,181,197),	"pink1" },
  { RGB(238,169,184),	"pink2" },
  { RGB(205,145,158),	"pink3" },
  { RGB(139, 99,108),	"pink4" },
  { RGB(221,160,221),	"plum" },
  { RGB(255,187,255),	"plum1" },
  { RGB(238,174,238),	"plum2" },
  { RGB(205,150,205),	"plum3" },
  { RGB(139,102,139),	"plum4" },
  { RGB(176,224,230),	"powderblue" },
  { RGB(160, 32,240),	"purple" },
  { RGB(155, 48,255),	"purple1" },
  { RGB(145, 44,238),	"purple2" },
  { RGB(125, 38,205),	"purple3" },
  { RGB( 85, 26,139),	"purple4" },
  { RGB(255,  0,  0),	"red" },
  { RGB(255,  0,  0),	"red1" },
  { RGB(238,  0,  0),	"red2" },
  { RGB(205,  0,  0),	"red3" },
  { RGB(139,  0,  0),	"red4" },
  { RGB(188,143,143),	"rosybrown" },
  { RGB(255,193,193),	"rosybrown1" },
  { RGB(238,180,180),	"rosybrown2" },
  { RGB(205,155,155),	"rosybrown3" },
  { RGB(139,105,105),	"rosybrown4" },
  { RGB( 65,105,225),	"royalblue" },
  { RGB( 72,118,255),	"royalblue1" },
  { RGB( 67,110,238),	"royalblue2" },
  { RGB( 58, 95,205),	"royalblue3" },
  { RGB( 39, 64,139),	"royalblue4" },
  { RGB(139, 69, 19),	"saddlebrown" },
  { RGB(250,128,114),	"salmon" },
  { RGB(255,140,105),	"salmon1" },
  { RGB(238,130, 98),	"salmon2" },
  { RGB(205,112, 84),	"salmon3" },
  { RGB(139, 76, 57),	"salmon4" },
  { RGB(244,164, 96),	"sandybrown" },
  { RGB( 46,139, 87),	"seagreen" },
  { RGB( 84,255,159),	"seagreen1" },
  { RGB( 78,238,148),	"seagreen2" },
  { RGB( 67,205,128),	"seagreen3" },
  { RGB( 46,139, 87),	"seagreen4" },
  { RGB(255,245,238),	"seashell" },
  { RGB(255,245,238),	"seashell1" },
  { RGB(238,229,222),	"seashell2" },
  { RGB(205,197,191),	"seashell3" },
  { RGB(139,134,130),	"seashell4" },
  { RGB(160, 82, 45),	"sienna" },
  { RGB(255,130, 71),	"sienna1" },
  { RGB(238,121, 66),	"sienna2" },
  { RGB(205,104, 57),	"sienna3" },
  { RGB(139, 71, 38),	"sienna4" },
  { RGB(135,206,235),	"skyblue" },
  { RGB(135,206,255),	"skyblue1" },
  { RGB(126,192,238),	"skyblue2" },
  { RGB(108,166,205),	"skyblue3" },
  { RGB( 74,112,139),	"skyblue4" },
  { RGB(106, 90,205),	"slateblue" },
  { RGB(131,111,255),	"slateblue1" },
  { RGB(122,103,238),	"slateblue2" },
  { RGB(105, 89,205),	"slateblue3" },
  { RGB( 71, 60,139),	"slateblue4" },
  { RGB(112,128,144),	"slategray" },
  { RGB(198,226,255),	"slategray1" },
  { RGB(185,211,238),	"slategray2" },
  { RGB(159,182,205),	"slategray3" },
  { RGB(108,123,139),	"slategray4" },
  { RGB(112,128,144),	"slategrey" },
  { RGB(255,250,250),	"snow" },
  { RGB(255,250,250),	"snow1" },
  { RGB(238,233,233),	"snow2" },
  { RGB(205,201,201),	"snow3" },
  { RGB(139,137,137),	"snow4" },
  { RGB(  0,255,127),	"springgreen" },
  { RGB(  0,255,127),	"springgreen1" },
  { RGB(  0,238,118),	"springgreen2" },
  { RGB(  0,205,102),	"springgreen3" },
  { RGB(  0,139, 69),	"springgreen4" },
  { RGB( 70,130,180),	"steelblue" },
  { RGB( 99,184,255),	"steelblue1" },
  { RGB( 92,172,238),	"steelblue2" },
  { RGB( 79,148,205),	"steelblue3" },
  { RGB( 54,100,139),	"steelblue4" },
  { RGB(210,180,140),	"tan" },
  { RGB(255,165, 79),	"tan1" },
  { RGB(238,154, 73),	"tan2" },
  { RGB(205,133, 63),	"tan3" },
  { RGB(139, 90, 43),	"tan4" },
  { RGB(216,191,216),	"thistle" },
  { RGB(255,225,255),	"thistle1" },
  { RGB(238,210,238),	"thistle2" },
  { RGB(205,181,205),	"thistle3" },
  { RGB(139,123,139),	"thistle4" },
  { RGB(255, 99, 71),	"tomato" },
  { RGB(255, 99, 71),	"tomato1" },
  { RGB(238, 92, 66),	"tomato2" },
  { RGB(205, 79, 57),	"tomato3" },
  { RGB(139, 54, 38),	"tomato4" },
  { RGB( 64,224,208),	"turquoise" },
  { RGB(  0,245,255),	"turquoise1" },
  { RGB(  0,229,238),	"turquoise2" },
  { RGB(  0,197,205),	"turquoise3" },
  { RGB(  0,134,139),	"turquoise4" },
  { RGB(238,130,238),	"violet" },
  { RGB(208, 32,144),	"violetred" },
  { RGB(255, 62,150),	"violetred1" },
  { RGB(238, 58,140),	"violetred2" },
  { RGB(205, 50,120),	"violetred3" },
  { RGB(139, 34, 82),	"violetred4" },
  { RGB(245,222,179),	"wheat" },
  { RGB(255,231,186),	"wheat1" },
  { RGB(238,216,174),	"wheat2" },
  { RGB(205,186,150),	"wheat3" },
  { RGB(139,126,102),	"wheat4" },
  { RGB(255,255,255),	"white" },
  { RGB(245,245,245),	"whitesmoke" },
  { RGB(255,255,  0),	"yellow" },
  { RGB(255,255,  0),	"yellow1" },
  { RGB(238,238,  0),	"yellow2" },
  { RGB(205,205,  0),	"yellow3" },
  { RGB(139,139,  0),	"yellow4" },
  { RGB(154,205, 50),	"yellowgreen" } };

static int rgb_cmp( const void *d1, const void *d2 )
{
    return stricmp( ((RGBData *)d1)->name, ((RGBData *)d2)->name );
}

static uint get_rgb_val( const char *name )
{
    RGBData x;
    x.name = (char *)name;
    RGBData *r = (RGBData*)bsearch((char*)&x, (char*)rgbTbl, rgbTblSize,
				   sizeof(RGBData), rgb_cmp);
    return r ? r->value : RGB_INVALID;
}

#else

static uint get_rgb_val( const char * )
{
    return RGB_INVALID;
}

#endif // NO_COLORNAMES


void QColor::setSystemNamedColor( const char *name )
{
    if ( !color_init ) {
#if defined(CHECK_STATE)
	warning( "QColor::setSystemNamedColor: Cannot perform this operation "
		 "because QApplication does not exist" );
#endif
	alloc();				// makes the color black
	return;
    }
    rgbVal = get_rgb_val( name );
    if ( lalloc ) {
	rgbVal |= RGB_DIRTY;			// alloc later
	pix = 0;
    } else {
	alloc();				// alloc now
    }
}


#define MAX_CONTEXTS 16
static int  context_stack[MAX_CONTEXTS];
static int  context_ptr = 0;

static void init_context_stack()
{
    static bool did_init = FALSE;
    if ( !did_init ) {
	did_init = TRUE;
	context_stack[0] = current_alloc_context = 0;
    }
}


int QColor::enterAllocContext()
{
    static context_seq_no = 0;
    init_context_stack();
    if ( context_ptr+1 == MAX_CONTEXTS ) {
#if defined(CHECK_STATE)
	warning( "QColor::enterAllocContext: Context stack overflow" );
#endif
	return 0;
    }
    current_alloc_context = context_stack[++context_ptr] = ++context_seq_no;
    return current_alloc_context;
}


void QColor::leaveAllocContext()
{
    init_context_stack();
    if ( context_ptr == 0 ) {
#if defined(CHECK_STATE)
	warning( "QColor::leaveAllocContext: Context stack underflow" );
#endif
	return;
    }
    current_alloc_context = context_stack[--context_ptr];
}


int QColor::currentAllocContext()
{
    return current_alloc_context;
}


void QColor::destroyAllocContext( int )
{
    init_context_stack();
}
