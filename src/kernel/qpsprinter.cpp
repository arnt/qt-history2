/**********************************************************************
** $Id: //depot/qt/main/src/kernel/qpsprinter.cpp#108 $
**
** Implementation of QPSPrinter class
**
** Created : 941003
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qpsprinter_p.h"

#ifndef QT_NO_PRINTER

#include "qpainter.h"
#include "qpaintdevicemetrics.h"
#include "qimage.h"
#include "qdatetime.h"

#include "qstring.h"
#include "qdict.h"

#include "qfile.h"
#include "qbuffer.h"
#include "qintdict.h"
#include "qtextcodec.h"

#include <ctype.h>
#if defined(Q_OS_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif

// http://partners.adobe.com/asn/developer/technotes.html has lots of
// information relevant to this file.

// Note: this is comment-stripped and word-wrapped later.
// Note: stripHeader() constrains the postscript used in this prolog.
// only function/variable declarations are currently allowed, and
// stripHeader knows a bit about the names of some functions.

//
// #### improve build process and qembed the result, saving RAM and ROM
//

static const char * const ps_header[] = {
"/d  /def load def",
"/D  {bind d} bind d",
"/d2 {dup dup} D",
"/B  {0 d2} D",
"/W  {255 d2} D",
"/ED {exch d} D",
"/D0 {0 ED} D",
"/LT {lineto} D",
"/MT {moveto} D",
"/ND /.notdef d",
"/S  {stroke} D",
"/F  {setfont} D",
"/SW {setlinewidth} D",
"/CP {closepath} D",
"/RL {rlineto} D",
"/NP {newpath} D",
"/CM {currentmatrix} D",
"/SM {setmatrix} D",
"/TR {translate} D",
"/SRGB {setrgbcolor} D",
"/SC {aload pop SRGB} D",
"/GS {gsave} D",
"/GR {grestore} D",
"",
"/BSt 0 d",				// brush style
"/LWi 1 d",				// line width
"/PSt 1 d",				// pen style
"/Cx  0 d",				// current x position
"/Cy  0 d",				// current y position
"/WFi false d",			// winding fill
"/OMo false d",			// opaque mode (not transparent)
"",
"/BCol  [ 1 1 1 ] d",			// brush color
"/PCol  [ 0 0 0 ] d",			// pen color
"/BkCol [ 1 1 1 ] d",			// background color
"",
"/nS 0 d",				// number of saved painter states
"",
"/LArr[",					// Pen styles:
"    []		     []",			//   solid line
"    [ 10 3 ]	     [ 3 10 ]",			//   dash line
"    [ 3 3 ]	     [ 3 3 ]",			//   dot line
"    [ 5 3 3 3 ]	     [ 3 5 3 3 ]",	//   dash dot line
"    [ 5 3 3 3 3 3 ]  [ 3 5 3 3 3 3 ]",		//   dash dot dot line
"] d",
"",
"",//
"",// Returns the line pattern (from pen style PSt).
"",//
"",// Argument:
"",//   bool pattern
"",//	true : draw pattern
"",//	false: fill pattern
"",//
"",
"/GPS {",
"  PSt 1 ge PSt 5 le and",			// valid pen pattern?
"    { { LArr PSt 1 sub 2 mul get }",		// draw pattern
"      { LArr PSt 2 mul 1 sub get } ifelse",    // opaque pattern
"    }",
"    { [] } ifelse",				// out of range => solid line
"} D",
"",
"/QS {",				// stroke command
"    PSt 0 ne",				// != NO_PEN
"    { LWi SW",				// set line width
"      GS",
"      PCol SC",			// set pen color
"      true GPS 0 setdash S",		// draw line pattern
"      OMo PSt 1 ne and",		// opaque mode and not solid line?
"      { GR BkCol SC",
"	false GPS dup 0 get setdash S",	// fill in opaque pattern
"      }",
"      { GR } ifelse",
"    } if",
"} D",
"",
"/BDArr[",				// Brush dense patterns:
"    0.06 0.12 0.37 0.50 0.63 0.88 0.94",
"] d",
"",

"", // read 28 bits and leave them on tos
"/r28 {",
"  ", // skip past whitespace and read one character
"  { currentfile read pop ",
"    dup 32 gt { exit } if",
"    pop",
"  } loop",
"  ", // read three more
"  3 {",
"    currentfile read pop",
"  } repeat",
"  ", // make an accumulator
"  0",
"  ", // for each character, shift the accumulator and add in the character
"  4 {",
"    7 bitshift exch",
"    dup 128 gt { 84 sub } if 42 sub 127 and",
"    add",
"  } repeat",
"} D",
"",
"", // read some bits and leave them on tos
"/rA 0 d ", // accumulator
"/rL 0 d ", // bits left
"",
"", // takes number of bits, leaves number
"/rB {",
"  rL 0 eq {",
"    ", // if we have nothing, let's get something
"    /rA r28 d",
"    /rL 28 d",
"  } if",
"  dup rL gt {",
"    ", // if we don't have enough, take what we have and get more
"    rA exch rL sub rL exch",
"    /rA 0 d /rL 0 d",
"    rB exch bitshift add",
"  } {",
"    ", // else take some of what we have
"    dup rA 16#fffffff 3 -1 roll bitshift not and exch",
"    ", // ... and update rL and rA
"    dup rL exch sub /rL ED",
"    neg rA exch bitshift /rA ED",
"  } ifelse",
"} D",
"",
"", // uncompresses from currentfile until the string on the stack is full;
"", // leaves the string there.  assumes that nothing could conceivably go
"", // wrong.
"/rC {",
"  /rL 0 d",
"  0",
"  {", // string pos
"    dup 2 index length ge { exit } if",
"    1 rB",
"    1 eq {", // compressed
"      3 rB", // string pos bits
"      dup 4 ge {",
"        dup rB", // string pos bits extra
"        1 index 5 ge {",
"          1 index 6 ge {",
"            1 index 7 ge {",
"              64 add",
"            } if",
"            32 add",
"          } if",
"          16 add",
"        } if",
"        4 add",
"	exch pop",
"      } if",
"      1 add 3 mul",
"      ", // string pos length
"      exch 10 rB 1 add 3 mul",
"      ", // string length pos dist
"      {",
"	dup 3 index lt {",
"	  dup",
"	} {",
"	  2 index",
"	} ifelse", // string length pos dist length-this-time
"	4 index 3 index 3 index sub 2 index getinterval",
"	5 index 4 index 3 -1 roll putinterval",
"	dup 4 -1 roll add 3 1 roll",
"	4 -1 roll exch sub ",
"	dup 0 eq { exit } if",
"	3 1 roll",
"      } loop", // string pos dist length
"      pop pop",
"    } {", // uncompressed
"      3 rB 1 add 3 mul",
"      {",
"	2 copy 8 rB put 1 add",
"      } repeat",
"    } ifelse",
"  } loop",
"  pop",
"} D",
"",
"/sl D0", // slow implementation, but strippable by stripHeader
"/QCIF D0 /Bcomp D0 /Bycomp D0 /QCIstr1 D0 /QCIstr1 D0 /QCIindex D0",
"/QCI {", // as colorimage but without the last two arguments
"  /colorimage where {",
"    pop",
"    false 3 colorimage",
"  }{", // the hard way, based on PD code by John Walker <kelvin@autodesk.com>
"    /QCIF ED",
"    /mat ED",
"    /Bcomp ED",
"    /h ED",
"    /w ED",
"    /Bycomp Bcomp 7 add 8 idiv d",
"    w h Bcomp mat",
"    /QCIstr2 sl length 3 idiv string d",
"    { Function exec",
"      /QCIstr1 ED",
"      0 1 QCIstr1 length 3 idiv 1 sub {",
"        /QCIindex ED",
"        /x QCIindex 3 mul d",
"        QCIstr2 QCIindex",
"        QCIstr1 x       get 0.30 mul",
"        QCIstr1 x 1 add get 0.59 mul",
"        QCIstr1 x 2 add get 0.11 mul",
"        add add cvi",
"        put",
"      } for",
"      QCIstr2",
"    }",
"    image",
"  } ifelse",
"} D",
"",
"/BF {",				// brush fill
"   GS",
"   BSt 1 eq",				// solid brush?
"   {",
"     BCol SC"
"     WFi { fill } { eofill } ifelse",
"   } if",
"   BSt 2 ge BSt 8 le and",		// dense pattern?
"   {",
"     BDArr BSt 2 sub get setgray fill"
"   } if",
"   BSt 9 ge BSt 14 le and",		// brush pattern?
"   {",
"     WFi { clip } { eoclip } ifelse",
"     defM SM",
"     pathbbox",			// left upper right lower
"     3 index 3 index translate",
"     4 2 roll",			// right lower left upper
"     3 2 roll",			// right left upper lower
"     exch",				// left right lower upper
"     sub /h ED",
"     sub /w ED",
"     OMo {",
"	  NP",
"	  0 0 MT",
"	  0 h RL",
"	  w 0 RL",
"	  0 h neg RL",
"	  CP",
"	  BkCol SC",
"	  fill",
"     } if",
"     BCol SC",
"     0.3 SW",
"     NP",
"     BSt 9 eq BSt 11 eq or",		// horiz or cross pattern
"     { 0 4 h",
"       { dup 0 exch MT w exch LT } for",
"     } if",
"     BSt 10 eq BSt 11 eq or",		// vert or cross pattern
"     { 0 4 w",
"       { dup 0 MT h LT } for",
"     } if",
"     BSt 12 eq BSt 14 eq or",		// F-diag or diag cross
"     { w h gt",
"       { 0 6 w h add",
"	  { dup 0 MT h sub h LT } for"
"       } { 0 6 w h add",
"         { dup 0 exch MT w sub w exch LT } for"
"       } ifelse",
"     } if",
"     BSt 13 eq BSt 14 eq or",		// B-diag or diag cross
"     { w h gt",
"       { 0 6 w h add",
"	  { dup h MT h sub 0 LT } for",
"       } { 0 6 w h add",
"	  { dup w exch MT w sub 0 exch LT } for"
"       } ifelse",
"     } if",
"     S",
"   } if",
"   BSt 24 eq",				// CustomPattern
"     ",
"   {",
"   } if",
"   GR",
"} D",
"",
"",
"", // for arc
"/mat matrix d",
"/ang1 D0 /ang2 D0",
"/w D0 /h D0",
"/x D0 /y D0",
"",
"/ARC {",				// Generic ARC function [ X Y W H ang1 ang2 ]
"    /ang2 ED /ang1 ED /h ED /w ED /y ED /x ED",
"    mat CM pop",
"    x w 2 div add y h 2 div add TR",
"    1 h w div neg scale",
"    ang2 0 ge",
"    {0 0 w 2 div ang1 ang1 ang2 add arc }",
"    {0 0 w 2 div ang1 ang1 ang2 add arcn} ifelse",
"    mat SM",
"} D",
"",
"/defM D0",
"",
"/C D0",
"",
"/P {",					// PdcDrawPoint [x y]
"    NP",
"    MT",
"    0.5 0.5 rmoveto",
"    0  -1 RL",
"    -1	0 RL",
"    0	1 RL",
"    CP",
"    PCol SC",
"    fill",
"} D",
"",
"/M {",					// PdcMoveTo [x y]
"    /Cy ED /Cx ED",
"} D",
"",
"/L {",					// PdcLineTo [x y]
"    NP",
"    Cx Cy MT",
"    /Cy ED /Cx ED",
"    Cx Cy LT",
"    QS",
"} D",
"",
"/DL {",				// PdcDrawLine [x1 y1 x0 y0]
"    NP",
"    MT",
"    LT",
"    QS",
"} D",
"",
"/HL {",				// PdcDrawLine [x1 y x0]
"    1 index DL",
"} D",
"",
"/VL {",				// PdcDrawLine [x y1 y0]
"    2 index exch DL",
"} D",
"",
"/R {",					// PdcDrawRect [x y w h]
"    /h ED /w ED /y ED /x ED",
"    NP",
"    x y MT",
"    0 h RL",
"    w 0 RL",
"    0 h neg RL",
"    CP",
"    BF",
"    QS",
"} D",
"",
"/ACR {",					// add clip rect
"    /h ED /w ED /y ED /x ED",
"    x y MT",
"    0 h RL",
"    w 0 RL",
"    0 h neg RL",
"    CP",
"} D",
"",
"/CLSTART {",				// clipping start
"    /clipTmp matrix CM d",		// save current matrix
"    defM SM",				// Page default matrix
"    NP",
"} D",
"",
"/CLEND {",				// clipping end
"    clip",
"    NP",
"    clipTmp SM",			// restore the current matrix
"} D",
"",
"/CLO {",				// clipping off
"    GR",				// restore top of page state
"    GS",				// save it back again
"    defM SM",				// set coordsys (defensive progr.)
"} D",
"",
"/xr D0 /yr D0",
"/rx D0 /ry D0 /rx2 D0 /ry2 D0",
"",
"/RR {",				// PdcDrawRoundRect [x y w h xr yr]
"    /yr ED /xr ED /h ED /w ED /y ED /x ED",
"    xr 0 le yr 0 le or",
"    {x y w h R}",		    // Do rect if one of rounding values is less than 0.
"    {xr 100 ge yr 100 ge or",
"	{x y w h E}",			 // Do ellipse if both rounding values are larger than 100
"	{",
"	 /rx xr w mul 200 div d",
"	 /ry yr h mul 200 div d",
"	 /rx2 rx 2 mul d",
"	 /ry2 ry 2 mul d",
"	 NP",
"	 x rx add y MT",
"	 x		 y		 rx2 ry2 180 -90",
"	 x		 y h add ry2 sub rx2 ry2 270 -90",
"	 x w add rx2 sub y h add ry2 sub rx2 ry2 0   -90",
"	 x w add rx2 sub y		 rx2 ry2 90  -90",
"	 ARC ARC ARC ARC",
"	 CP",
"	 BF",
"	 QS",
"	} ifelse",
"    } ifelse",
"} D",
"",
"",
"/E {",				// PdcDrawEllipse [x y w h]
"    /h ED /w ED /y ED /x ED",
"    mat CM pop",
"    x w 2 div add y h 2 div add translate",
"    1 h w div scale",
"    NP",
"    0 0 w 2 div 0 360 arc",
"    mat SM",
"    BF",
"    QS",
"} D",
"",
"",
"/A {",				// PdcDrawArc [x y w h ang1 ang2]
"    16 div exch 16 div exch",
"    NP",
"    ARC",
"    QS",
"} D",
"",
"",
"/PIE {",				// PdcDrawPie [x y w h ang1 ang2]
"    /ang2 ED /ang1 ED /h ED /w ED /y ED /x ED",
"    NP",
"    x w 2 div add y h 2 div add MT",
"    x y w h ang1 16 div ang2 16 div ARC",
"    CP",
"    BF",
"    QS",
"} D",
"",
"/CH {",				// PdcDrawChord [x y w h ang1 ang2]
"    16 div exch 16 div exch",
"    NP",
"    ARC",
"    CP",
"    BF",
"    QS",
"} D",
"",
"",
"/BZ {",				// PdcDrawCubicBezier [4 points]
"    curveto",
"    QS",
"} D",
"",
"",
"/CRGB {",				// Compute RGB [R G B] => R/255 G/255 B/255
"    255 div 3 1 roll",
"    255 div 3 1 roll",
"    255 div 3 1 roll",
"} D",
"",
"",
"/SV {",				// Save painter state
"    BSt LWi PSt Cx Cy WFi OMo BCol PCol BkCol",
"    /nS nS 1 add d",
"    GS",
"} D",
"",
"/RS {",				// Restore painter state
"    nS 0 gt",
"    { GR",
"      /BkCol ED /PCol ED /BCol ED /OMo ED /WFi ED",
"      /Cy ED /Cx ED /PSt ED /LWi ED /BSt ED",
"      /nS nS 1 sub d",
"    } if",
"",
"} D",
"",
"/BC {",				// PdcSetBkColor [R G B]
"    CRGB",
"    BkCol astore pop",
"} D",
"",
"/BR {",				// PdcSetBrush [style R G B]
"    CRGB",
"    BCol astore pop",
"    /BSt ED",
"} D",
"",
"/WB {",				// set white solid brush
"    1 W BR",
"} D",
"",
"/NB {",				// set nobrush
"    0 B BR",
"} D",
"",
"/PE {", // PdcSetPen [style width R G B Cap Join]
"    setlinejoin setlinecap"
"    CRGB",
"    PCol astore pop",
"    /LWi ED",
"    /PSt ED",
"    LWi 0 eq { 0.25 /LWi ED } if", // ### 3.0 remove this line
"} D",
"",
"/P1 {",				// PdcSetPen [R G B]
"    1 0 5 2 roll 0 0 PE",
"} D",
"",
"/ST {",				// SET TRANSFORM [matrix]
"    defM setmatrix",
"    concat",
"} D",
"",
"/MF {",				// newname encoding fontname
"  findfont dup length dict begin",
"  {",
"    1 index /FID ne",
"    {d}{pop pop}ifelse",
"  } forall",
"  /Encoding ED currentdict end",
"  definefont pop",
"} D",
"",
"/DF {",				// newname pointsize fontmame
"  findfont",
"  /FONTSIZE 3 -1 roll d [ FONTSIZE 0 0 FONTSIZE -1 mul 0 0 ] makefont",
"  d",
"} D",
"",
"",
"/ty 0 d",
"/Y {",
"    /ty ED",
"} D",
"",
"/Tl {", // draw underline/strikeout line: () w x y ->Tl-> () w x
"    NP 1 index exch MT",
"    1 index 0 rlineto QS",
"} D",
"",
"/T {",					// PdcDrawText2 [string fm.width x]
"    PCol SC", // really need to kill these SCs
"    ty MT",
"    1 index", // string cwidth string
"    dup length exch", // string cwidth length string
"    stringwidth pop", // string cwidth length pwidth
"    3 -1 roll", // string length pwidth cwidth
"    exch sub exch div", // string extraperchar
"    exch 0 exch", // extraperchar 0 string
"    ashow",
"} D",
"",
"/QI {",
"    /C save d",
"    defM setmatrix",			// default transformation matrix
"    /Cx  0 d",				// reset current x position
"    /Cy  0 d",				// reset current y position
"    /OMo false d",
"    GS",
"} D",
"",
"/QP {",				// show page
"    GR",
"    C restore",
"    showpage",
"} D",
0 };



// the next table is derived from a list provided by Adobe on its web
// server: http://partners.adobe.com/asn/developer/typeforum/glyphlist.txt

// the start of the header comment:
//
// Name:          Adobe Glyph List
// Table version: 1.2
// Date:          22 Oct 1998
//
// Description:
//
//   The Adobe Glyph List (AGL) list relates Unicode values (UVs) to glyph
//   names, and should be used only as described in the document "Unicode and
//   Glyph Names," at
//   http://www.adobe.com/asn/developer/typeforum/unicodegn.html .


static const struct {
    Q_UINT16 u;
    const char * g;
} unicodetoglyph[] = {
    // grep '^[0-9A-F][0-9A-F][0-9A-F][0-9A-F];' < /tmp/glyphlist.txt | sed -e 's/;/, "/' -e 's-;-" },  // -' -e 's/^/    { 0x/' | sort
    { 0x0020, "space" },  // SPACE
    { 0x0021, "exclam" },  // EXCLAMATION MARK
    { 0x0022, "quotedbl" },  // QUOTATION MARK
    { 0x0023, "numbersign" },  // NUMBER SIGN
    { 0x0024, "dollar" },  // DOLLAR SIGN
    { 0x0025, "percent" },  // PERCENT SIGN
    { 0x0026, "ampersand" },  // AMPERSAND
    { 0x0027, "quotesingle" },  // APOSTROPHE
    { 0x0028, "parenleft" },  // LEFT PARENTHESIS
    { 0x0029, "parenright" },  // RIGHT PARENTHESIS
    { 0x002A, "asterisk" },  // ASTERISK
    { 0x002B, "plus" },  // PLUS SIGN
    { 0x002C, "comma" },  // COMMA
    { 0x002D, "hyphen" },  // HYPHEN-MINUS
    { 0x002E, "period" },  // FULL STOP
    { 0x002F, "slash" },  // SOLIDUS
    { 0x0030, "zero" },  // DIGIT ZERO
    { 0x0031, "one" },  // DIGIT ONE
    { 0x0032, "two" },  // DIGIT TWO
    { 0x0033, "three" },  // DIGIT THREE
    { 0x0034, "four" },  // DIGIT FOUR
    { 0x0035, "five" },  // DIGIT FIVE
    { 0x0036, "six" },  // DIGIT SIX
    { 0x0037, "seven" },  // DIGIT SEVEN
    { 0x0038, "eight" },  // DIGIT EIGHT
    { 0x0039, "nine" },  // DIGIT NINE
    { 0x003A, "colon" },  // COLON
    { 0x003B, "semicolon" },  // SEMICOLON
    { 0x003C, "less" },  // LESS-THAN SIGN
    { 0x003D, "equal" },  // EQUALS SIGN
    { 0x003E, "greater" },  // GREATER-THAN SIGN
    { 0x003F, "question" },  // QUESTION MARK
    { 0x0040, "at" },  // COMMERCIAL AT
    { 0x0041, "A" },  // LATIN CAPITAL LETTER A
    { 0x0042, "B" },  // LATIN CAPITAL LETTER B
    { 0x0043, "C" },  // LATIN CAPITAL LETTER C
    { 0x0044, "D" },  // LATIN CAPITAL LETTER D
    { 0x0045, "E" },  // LATIN CAPITAL LETTER E
    { 0x0046, "F" },  // LATIN CAPITAL LETTER F
    { 0x0047, "G" },  // LATIN CAPITAL LETTER G
    { 0x0048, "H" },  // LATIN CAPITAL LETTER H
    { 0x0049, "I" },  // LATIN CAPITAL LETTER I
    { 0x004A, "J" },  // LATIN CAPITAL LETTER J
    { 0x004B, "K" },  // LATIN CAPITAL LETTER K
    { 0x004C, "L" },  // LATIN CAPITAL LETTER L
    { 0x004D, "M" },  // LATIN CAPITAL LETTER M
    { 0x004E, "N" },  // LATIN CAPITAL LETTER N
    { 0x004F, "O" },  // LATIN CAPITAL LETTER O
    { 0x0050, "P" },  // LATIN CAPITAL LETTER P
    { 0x0051, "Q" },  // LATIN CAPITAL LETTER Q
    { 0x0052, "R" },  // LATIN CAPITAL LETTER R
    { 0x0053, "S" },  // LATIN CAPITAL LETTER S
    { 0x0054, "T" },  // LATIN CAPITAL LETTER T
    { 0x0055, "U" },  // LATIN CAPITAL LETTER U
    { 0x0056, "V" },  // LATIN CAPITAL LETTER V
    { 0x0057, "W" },  // LATIN CAPITAL LETTER W
    { 0x0058, "X" },  // LATIN CAPITAL LETTER X
    { 0x0059, "Y" },  // LATIN CAPITAL LETTER Y
    { 0x005A, "Z" },  // LATIN CAPITAL LETTER Z
    { 0x005B, "bracketleft" },  // LEFT SQUARE BRACKET
    { 0x005C, "backslash" },  // REVERSE SOLIDUS
    { 0x005D, "bracketright" },  // RIGHT SQUARE BRACKET
    { 0x005E, "asciicircum" },  // CIRCUMFLEX ACCENT
    { 0x005F, "underscore" },  // LOW LINE
    { 0x0060, "grave" },  // GRAVE ACCENT
    { 0x0061, "a" },  // LATIN SMALL LETTER A
    { 0x0062, "b" },  // LATIN SMALL LETTER B
    { 0x0063, "c" },  // LATIN SMALL LETTER C
    { 0x0064, "d" },  // LATIN SMALL LETTER D
    { 0x0065, "e" },  // LATIN SMALL LETTER E
    { 0x0066, "f" },  // LATIN SMALL LETTER F
    { 0x0067, "g" },  // LATIN SMALL LETTER G
    { 0x0068, "h" },  // LATIN SMALL LETTER H
    { 0x0069, "i" },  // LATIN SMALL LETTER I
    { 0x006A, "j" },  // LATIN SMALL LETTER J
    { 0x006B, "k" },  // LATIN SMALL LETTER K
    { 0x006C, "l" },  // LATIN SMALL LETTER L
    { 0x006D, "m" },  // LATIN SMALL LETTER M
    { 0x006E, "n" },  // LATIN SMALL LETTER N
    { 0x006F, "o" },  // LATIN SMALL LETTER O
    { 0x0070, "p" },  // LATIN SMALL LETTER P
    { 0x0071, "q" },  // LATIN SMALL LETTER Q
    { 0x0072, "r" },  // LATIN SMALL LETTER R
    { 0x0073, "s" },  // LATIN SMALL LETTER S
    { 0x0074, "t" },  // LATIN SMALL LETTER T
    { 0x0075, "u" },  // LATIN SMALL LETTER U
    { 0x0076, "v" },  // LATIN SMALL LETTER V
    { 0x0077, "w" },  // LATIN SMALL LETTER W
    { 0x0078, "x" },  // LATIN SMALL LETTER X
    { 0x0079, "y" },  // LATIN SMALL LETTER Y
    { 0x007A, "z" },  // LATIN SMALL LETTER Z
    { 0x007B, "braceleft" },  // LEFT CURLY BRACKET
    { 0x007C, "bar" },  // VERTICAL LINE
    { 0x007D, "braceright" },  // RIGHT CURLY BRACKET
    { 0x007E, "asciitilde" },  // TILDE
    { 0x00A0, "space" },  // NO-BREAK SPACE;Duplicate
    { 0x00A1, "exclamdown" },  // INVERTED EXCLAMATION MARK
    { 0x00A2, "cent" },  // CENT SIGN
    { 0x00A3, "sterling" },  // POUND SIGN
    { 0x00A4, "currency" },  // CURRENCY SIGN
    { 0x00A5, "yen" },  // YEN SIGN
    { 0x00A6, "brokenbar" },  // BROKEN BAR
    { 0x00A7, "section" },  // SECTION SIGN
    { 0x00A8, "dieresis" },  // DIAERESIS
    { 0x00A9, "copyright" },  // COPYRIGHT SIGN
    { 0x00AA, "ordfeminine" },  // FEMININE ORDINAL INDICATOR
    { 0x00AB, "guillemotleft" },  // LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
    { 0x00AC, "logicalnot" },  // NOT SIGN
    { 0x00AD, "hyphen" },  // SOFT HYPHEN;Duplicate
    { 0x00AE, "registered" },  // REGISTERED SIGN
    { 0x00AF, "macron" },  // MACRON
    { 0x00B0, "degree" },  // DEGREE SIGN
    { 0x00B1, "plusminus" },  // PLUS-MINUS SIGN
    { 0x00B2, "twosuperior" },  // SUPERSCRIPT TWO
    { 0x00B3, "threesuperior" },  // SUPERSCRIPT THREE
    { 0x00B4, "acute" },  // ACUTE ACCENT
    { 0x00B5, "mu" },  // MICRO SIGN
    { 0x00B6, "paragraph" },  // PILCROW SIGN
    { 0x00B7, "periodcentered" },  // MIDDLE DOT
    { 0x00B8, "cedilla" },  // CEDILLA
    { 0x00B9, "onesuperior" },  // SUPERSCRIPT ONE
    { 0x00BA, "ordmasculine" },  // MASCULINE ORDINAL INDICATOR
    { 0x00BB, "guillemotright" }, // RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
    { 0x00BC, "onequarter" },  // VULGAR FRACTION ONE QUARTER
    { 0x00BD, "onehalf" },  // VULGAR FRACTION ONE HALF
    { 0x00BE, "threequarters" },  // VULGAR FRACTION THREE QUARTERS
    { 0x00BF, "questiondown" },  // INVERTED QUESTION MARK
    { 0x00C0, "Agrave" },  // LATIN CAPITAL LETTER A WITH GRAVE
    { 0x00C1, "Aacute" },  // LATIN CAPITAL LETTER A WITH ACUTE
    { 0x00C2, "Acircumflex" },  // LATIN CAPITAL LETTER A WITH CIRCUMFLEX
    { 0x00C3, "Atilde" },  // LATIN CAPITAL LETTER A WITH TILDE
    { 0x00C4, "Adieresis" },  // LATIN CAPITAL LETTER A WITH DIAERESIS
    { 0x00C5, "Aring" },  // LATIN CAPITAL LETTER A WITH RING ABOVE
    { 0x00C6, "AE" },  // LATIN CAPITAL LETTER AE
    { 0x00C7, "Ccedilla" },  // LATIN CAPITAL LETTER C WITH CEDILLA
    { 0x00C8, "Egrave" },  // LATIN CAPITAL LETTER E WITH GRAVE
    { 0x00C9, "Eacute" },  // LATIN CAPITAL LETTER E WITH ACUTE
    { 0x00CA, "Ecircumflex" },  // LATIN CAPITAL LETTER E WITH CIRCUMFLEX
    { 0x00CB, "Edieresis" },  // LATIN CAPITAL LETTER E WITH DIAERESIS
    { 0x00CC, "Igrave" },  // LATIN CAPITAL LETTER I WITH GRAVE
    { 0x00CD, "Iacute" },  // LATIN CAPITAL LETTER I WITH ACUTE
    { 0x00CE, "Icircumflex" },  // LATIN CAPITAL LETTER I WITH CIRCUMFLEX
    { 0x00CF, "Idieresis" },  // LATIN CAPITAL LETTER I WITH DIAERESIS
    { 0x00D0, "Eth" },  // LATIN CAPITAL LETTER ETH
    { 0x00D1, "Ntilde" },  // LATIN CAPITAL LETTER N WITH TILDE
    { 0x00D2, "Ograve" },  // LATIN CAPITAL LETTER O WITH GRAVE
    { 0x00D3, "Oacute" },  // LATIN CAPITAL LETTER O WITH ACUTE
    { 0x00D4, "Ocircumflex" },  // LATIN CAPITAL LETTER O WITH CIRCUMFLEX
    { 0x00D5, "Otilde" },  // LATIN CAPITAL LETTER O WITH TILDE
    { 0x00D6, "Odieresis" },  // LATIN CAPITAL LETTER O WITH DIAERESIS
    { 0x00D7, "multiply" },  // MULTIPLICATION SIGN
    { 0x00D8, "Oslash" },  // LATIN CAPITAL LETTER O WITH STROKE
    { 0x00D9, "Ugrave" },  // LATIN CAPITAL LETTER U WITH GRAVE
    { 0x00DA, "Uacute" },  // LATIN CAPITAL LETTER U WITH ACUTE
    { 0x00DB, "Ucircumflex" },  // LATIN CAPITAL LETTER U WITH CIRCUMFLEX
    { 0x00DC, "Udieresis" },  // LATIN CAPITAL LETTER U WITH DIAERESIS
    { 0x00DD, "Yacute" },  // LATIN CAPITAL LETTER Y WITH ACUTE
    { 0x00DE, "Thorn" },  // LATIN CAPITAL LETTER THORN
    { 0x00DF, "germandbls" },  // LATIN SMALL LETTER SHARP S
    { 0x00E0, "agrave" },  // LATIN SMALL LETTER A WITH GRAVE
    { 0x00E1, "aacute" },  // LATIN SMALL LETTER A WITH ACUTE
    { 0x00E2, "acircumflex" },  // LATIN SMALL LETTER A WITH CIRCUMFLEX
    { 0x00E3, "atilde" },  // LATIN SMALL LETTER A WITH TILDE
    { 0x00E4, "adieresis" },  // LATIN SMALL LETTER A WITH DIAERESIS
    { 0x00E5, "aring" },  // LATIN SMALL LETTER A WITH RING ABOVE
    { 0x00E6, "ae" },  // LATIN SMALL LETTER AE
    { 0x00E7, "ccedilla" },  // LATIN SMALL LETTER C WITH CEDILLA
    { 0x00E8, "egrave" },  // LATIN SMALL LETTER E WITH GRAVE
    { 0x00E9, "eacute" },  // LATIN SMALL LETTER E WITH ACUTE
    { 0x00EA, "ecircumflex" },  // LATIN SMALL LETTER E WITH CIRCUMFLEX
    { 0x00EB, "edieresis" },  // LATIN SMALL LETTER E WITH DIAERESIS
    { 0x00EC, "igrave" },  // LATIN SMALL LETTER I WITH GRAVE
    { 0x00ED, "iacute" },  // LATIN SMALL LETTER I WITH ACUTE
    { 0x00EE, "icircumflex" },  // LATIN SMALL LETTER I WITH CIRCUMFLEX
    { 0x00EF, "idieresis" },  // LATIN SMALL LETTER I WITH DIAERESIS
    { 0x00F0, "eth" },  // LATIN SMALL LETTER ETH
    { 0x00F1, "ntilde" },  // LATIN SMALL LETTER N WITH TILDE
    { 0x00F2, "ograve" },  // LATIN SMALL LETTER O WITH GRAVE
    { 0x00F3, "oacute" },  // LATIN SMALL LETTER O WITH ACUTE
    { 0x00F4, "ocircumflex" },  // LATIN SMALL LETTER O WITH CIRCUMFLEX
    { 0x00F5, "otilde" },  // LATIN SMALL LETTER O WITH TILDE
    { 0x00F6, "odieresis" },  // LATIN SMALL LETTER O WITH DIAERESIS
    { 0x00F7, "divide" },  // DIVISION SIGN
    { 0x00F8, "oslash" },  // LATIN SMALL LETTER O WITH STROKE
    { 0x00F9, "ugrave" },  // LATIN SMALL LETTER U WITH GRAVE
    { 0x00FA, "uacute" },  // LATIN SMALL LETTER U WITH ACUTE
    { 0x00FB, "ucircumflex" },  // LATIN SMALL LETTER U WITH CIRCUMFLEX
    { 0x00FC, "udieresis" },  // LATIN SMALL LETTER U WITH DIAERESIS
    { 0x00FD, "yacute" },  // LATIN SMALL LETTER Y WITH ACUTE
    { 0x00FE, "thorn" },  // LATIN SMALL LETTER THORN
    { 0x00FF, "ydieresis" },  // LATIN SMALL LETTER Y WITH DIAERESIS
    { 0x0100, "Amacron" },  // LATIN CAPITAL LETTER A WITH MACRON
    { 0x0101, "amacron" },  // LATIN SMALL LETTER A WITH MACRON
    { 0x0102, "Abreve" },  // LATIN CAPITAL LETTER A WITH BREVE
    { 0x0103, "abreve" },  // LATIN SMALL LETTER A WITH BREVE
    { 0x0104, "Aogonek" },  // LATIN CAPITAL LETTER A WITH OGONEK
    { 0x0105, "aogonek" },  // LATIN SMALL LETTER A WITH OGONEK
    { 0x0106, "Cacute" },  // LATIN CAPITAL LETTER C WITH ACUTE
    { 0x0107, "cacute" },  // LATIN SMALL LETTER C WITH ACUTE
    { 0x0108, "Ccircumflex" },  // LATIN CAPITAL LETTER C WITH CIRCUMFLEX
    { 0x0109, "ccircumflex" },  // LATIN SMALL LETTER C WITH CIRCUMFLEX
    { 0x010A, "Cdotaccent" },  // LATIN CAPITAL LETTER C WITH DOT ABOVE
    { 0x010B, "cdotaccent" },  // LATIN SMALL LETTER C WITH DOT ABOVE
    { 0x010C, "Ccaron" },  // LATIN CAPITAL LETTER C WITH CARON
    { 0x010D, "ccaron" },  // LATIN SMALL LETTER C WITH CARON
    { 0x010E, "Dcaron" },  // LATIN CAPITAL LETTER D WITH CARON
    { 0x010F, "dcaron" },  // LATIN SMALL LETTER D WITH CARON
    { 0x0110, "Dcroat" },  // LATIN CAPITAL LETTER D WITH STROKE
    { 0x0111, "dcroat" },  // LATIN SMALL LETTER D WITH STROKE
    { 0x0112, "Emacron" },  // LATIN CAPITAL LETTER E WITH MACRON
    { 0x0113, "emacron" },  // LATIN SMALL LETTER E WITH MACRON
    { 0x0114, "Ebreve" },  // LATIN CAPITAL LETTER E WITH BREVE
    { 0x0115, "ebreve" },  // LATIN SMALL LETTER E WITH BREVE
    { 0x0116, "Edotaccent" },  // LATIN CAPITAL LETTER E WITH DOT ABOVE
    { 0x0117, "edotaccent" },  // LATIN SMALL LETTER E WITH DOT ABOVE
    { 0x0118, "Eogonek" },  // LATIN CAPITAL LETTER E WITH OGONEK
    { 0x0119, "eogonek" },  // LATIN SMALL LETTER E WITH OGONEK
    { 0x011A, "Ecaron" },  // LATIN CAPITAL LETTER E WITH CARON
    { 0x011B, "ecaron" },  // LATIN SMALL LETTER E WITH CARON
    { 0x011C, "Gcircumflex" },  // LATIN CAPITAL LETTER G WITH CIRCUMFLEX
    { 0x011D, "gcircumflex" },  // LATIN SMALL LETTER G WITH CIRCUMFLEX
    { 0x011E, "Gbreve" },  // LATIN CAPITAL LETTER G WITH BREVE
    { 0x011F, "gbreve" },  // LATIN SMALL LETTER G WITH BREVE
    { 0x0120, "Gdotaccent" },  // LATIN CAPITAL LETTER G WITH DOT ABOVE
    { 0x0121, "gdotaccent" },  // LATIN SMALL LETTER G WITH DOT ABOVE
    { 0x0122, "Gcommaaccent" },  // LATIN CAPITAL LETTER G WITH CEDILLA
    { 0x0123, "gcommaaccent" },  // LATIN SMALL LETTER G WITH CEDILLA
    { 0x0124, "Hcircumflex" },  // LATIN CAPITAL LETTER H WITH CIRCUMFLEX
    { 0x0125, "hcircumflex" },  // LATIN SMALL LETTER H WITH CIRCUMFLEX
    { 0x0126, "Hbar" },  // LATIN CAPITAL LETTER H WITH STROKE
    { 0x0127, "hbar" },  // LATIN SMALL LETTER H WITH STROKE
    { 0x0128, "Itilde" },  // LATIN CAPITAL LETTER I WITH TILDE
    { 0x0129, "itilde" },  // LATIN SMALL LETTER I WITH TILDE
    { 0x012A, "Imacron" },  // LATIN CAPITAL LETTER I WITH MACRON
    { 0x012B, "imacron" },  // LATIN SMALL LETTER I WITH MACRON
    { 0x012C, "Ibreve" },  // LATIN CAPITAL LETTER I WITH BREVE
    { 0x012D, "ibreve" },  // LATIN SMALL LETTER I WITH BREVE
    { 0x012E, "Iogonek" },  // LATIN CAPITAL LETTER I WITH OGONEK
    { 0x012F, "iogonek" },  // LATIN SMALL LETTER I WITH OGONEK
    { 0x0130, "Idotaccent" },  // LATIN CAPITAL LETTER I WITH DOT ABOVE
    { 0x0131, "dotlessi" },  // LATIN SMALL LETTER DOTLESS I
    { 0x0132, "IJ" },  // LATIN CAPITAL LIGATURE IJ
    { 0x0133, "ij" },  // LATIN SMALL LIGATURE IJ
    { 0x0134, "Jcircumflex" },  // LATIN CAPITAL LETTER J WITH CIRCUMFLEX
    { 0x0135, "jcircumflex" },  // LATIN SMALL LETTER J WITH CIRCUMFLEX
    { 0x0136, "Kcommaaccent" },  // LATIN CAPITAL LETTER K WITH CEDILLA
    { 0x0137, "kcommaaccent" },  // LATIN SMALL LETTER K WITH CEDILLA
    { 0x0138, "kgreenlandic" },  // LATIN SMALL LETTER KRA
    { 0x0139, "Lacute" },  // LATIN CAPITAL LETTER L WITH ACUTE
    { 0x013A, "lacute" },  // LATIN SMALL LETTER L WITH ACUTE
    { 0x013B, "Lcommaaccent" },  // LATIN CAPITAL LETTER L WITH CEDILLA
    { 0x013C, "lcommaaccent" },  // LATIN SMALL LETTER L WITH CEDILLA
    { 0x013D, "Lcaron" },  // LATIN CAPITAL LETTER L WITH CARON
    { 0x013E, "lcaron" },  // LATIN SMALL LETTER L WITH CARON
    { 0x013F, "Ldot" },  // LATIN CAPITAL LETTER L WITH MIDDLE DOT
    { 0x0140, "ldot" },  // LATIN SMALL LETTER L WITH MIDDLE DOT
    { 0x0141, "Lslash" },  // LATIN CAPITAL LETTER L WITH STROKE
    { 0x0142, "lslash" },  // LATIN SMALL LETTER L WITH STROKE
    { 0x0143, "Nacute" },  // LATIN CAPITAL LETTER N WITH ACUTE
    { 0x0144, "nacute" },  // LATIN SMALL LETTER N WITH ACUTE
    { 0x0145, "Ncommaaccent" },  // LATIN CAPITAL LETTER N WITH CEDILLA
    { 0x0146, "ncommaaccent" },  // LATIN SMALL LETTER N WITH CEDILLA
    { 0x0147, "Ncaron" },  // LATIN CAPITAL LETTER N WITH CARON
    { 0x0148, "ncaron" },  // LATIN SMALL LETTER N WITH CARON
    { 0x0149, "napostrophe" },  // LATIN SMALL LETTER N PRECEDED BY APOSTROPHE
    { 0x014A, "Eng" },  // LATIN CAPITAL LETTER ENG
    { 0x014B, "eng" },  // LATIN SMALL LETTER ENG
    { 0x014C, "Omacron" },  // LATIN CAPITAL LETTER O WITH MACRON
    { 0x014D, "omacron" },  // LATIN SMALL LETTER O WITH MACRON
    { 0x014E, "Obreve" },  // LATIN CAPITAL LETTER O WITH BREVE
    { 0x014F, "obreve" },  // LATIN SMALL LETTER O WITH BREVE
    { 0x0150, "Ohungarumlaut" },  // LATIN CAPITAL LETTER O WITH DOUBLE ACUTE
    { 0x0151, "ohungarumlaut" },  // LATIN SMALL LETTER O WITH DOUBLE ACUTE
    { 0x0152, "OE" },  // LATIN CAPITAL LIGATURE OE
    { 0x0153, "oe" },  // LATIN SMALL LIGATURE OE
    { 0x0154, "Racute" },  // LATIN CAPITAL LETTER R WITH ACUTE
    { 0x0155, "racute" },  // LATIN SMALL LETTER R WITH ACUTE
    { 0x0156, "Rcommaaccent" },  // LATIN CAPITAL LETTER R WITH CEDILLA
    { 0x0157, "rcommaaccent" },  // LATIN SMALL LETTER R WITH CEDILLA
    { 0x0158, "Rcaron" },  // LATIN CAPITAL LETTER R WITH CARON
    { 0x0159, "rcaron" },  // LATIN SMALL LETTER R WITH CARON
    { 0x015A, "Sacute" },  // LATIN CAPITAL LETTER S WITH ACUTE
    { 0x015B, "sacute" },  // LATIN SMALL LETTER S WITH ACUTE
    { 0x015C, "Scircumflex" },  // LATIN CAPITAL LETTER S WITH CIRCUMFLEX
    { 0x015D, "scircumflex" },  // LATIN SMALL LETTER S WITH CIRCUMFLEX
    { 0x015E, "Scedilla" },  // LATIN CAPITAL LETTER S WITH CEDILLA
    { 0x015F, "scedilla" },  // LATIN SMALL LETTER S WITH CEDILLA
    { 0x0160, "Scaron" },  // LATIN CAPITAL LETTER S WITH CARON
    { 0x0161, "scaron" },  // LATIN SMALL LETTER S WITH CARON
    { 0x0162, "Tcommaaccent" },  // LATIN CAPITAL LETTER T WITH CEDILLA
    { 0x0163, "tcommaaccent" },  // LATIN SMALL LETTER T WITH CEDILLA
    { 0x0164, "Tcaron" },  // LATIN CAPITAL LETTER T WITH CARON
    { 0x0165, "tcaron" },  // LATIN SMALL LETTER T WITH CARON
    { 0x0166, "Tbar" },  // LATIN CAPITAL LETTER T WITH STROKE
    { 0x0167, "tbar" },  // LATIN SMALL LETTER T WITH STROKE
    { 0x0168, "Utilde" },  // LATIN CAPITAL LETTER U WITH TILDE
    { 0x0169, "utilde" },  // LATIN SMALL LETTER U WITH TILDE
    { 0x016A, "Umacron" },  // LATIN CAPITAL LETTER U WITH MACRON
    { 0x016B, "umacron" },  // LATIN SMALL LETTER U WITH MACRON
    { 0x016C, "Ubreve" },  // LATIN CAPITAL LETTER U WITH BREVE
    { 0x016D, "ubreve" },  // LATIN SMALL LETTER U WITH BREVE
    { 0x016E, "Uring" },  // LATIN CAPITAL LETTER U WITH RING ABOVE
    { 0x016F, "uring" },  // LATIN SMALL LETTER U WITH RING ABOVE
    { 0x0170, "Uhungarumlaut" },  // LATIN CAPITAL LETTER U WITH DOUBLE ACUTE
    { 0x0171, "uhungarumlaut" },  // LATIN SMALL LETTER U WITH DOUBLE ACUTE
    { 0x0172, "Uogonek" },  // LATIN CAPITAL LETTER U WITH OGONEK
    { 0x0173, "uogonek" },  // LATIN SMALL LETTER U WITH OGONEK
    { 0x0174, "Wcircumflex" },  // LATIN CAPITAL LETTER W WITH CIRCUMFLEX
    { 0x0175, "wcircumflex" },  // LATIN SMALL LETTER W WITH CIRCUMFLEX
    { 0x0176, "Ycircumflex" },  // LATIN CAPITAL LETTER Y WITH CIRCUMFLEX
    { 0x0177, "ycircumflex" },  // LATIN SMALL LETTER Y WITH CIRCUMFLEX
    { 0x0178, "Ydieresis" },  // LATIN CAPITAL LETTER Y WITH DIAERESIS
    { 0x0179, "Zacute" },  // LATIN CAPITAL LETTER Z WITH ACUTE
    { 0x017A, "zacute" },  // LATIN SMALL LETTER Z WITH ACUTE
    { 0x017B, "Zdotaccent" },  // LATIN CAPITAL LETTER Z WITH DOT ABOVE
    { 0x017C, "zdotaccent" },  // LATIN SMALL LETTER Z WITH DOT ABOVE
    { 0x017D, "Zcaron" },  // LATIN CAPITAL LETTER Z WITH CARON
    { 0x017E, "zcaron" },  // LATIN SMALL LETTER Z WITH CARON
    { 0x017F, "longs" },  // LATIN SMALL LETTER LONG S
    { 0x0192, "florin" },  // LATIN SMALL LETTER F WITH HOOK
    { 0x01A0, "Ohorn" },  // LATIN CAPITAL LETTER O WITH HORN
    { 0x01A1, "ohorn" },  // LATIN SMALL LETTER O WITH HORN
    { 0x01AF, "Uhorn" },  // LATIN CAPITAL LETTER U WITH HORN
    { 0x01B0, "uhorn" },  // LATIN SMALL LETTER U WITH HORN
    { 0x01E6, "Gcaron" },  // LATIN CAPITAL LETTER G WITH CARON
    { 0x01E7, "gcaron" },  // LATIN SMALL LETTER G WITH CARON
    { 0x01FA, "Aringacute" },  // LATIN CAPITAL LETTER A WITH RING ABOVE AND ACUTE
    { 0x01FB, "aringacute" },  // LATIN SMALL LETTER A WITH RING ABOVE AND ACUTE
    { 0x01FC, "AEacute" },  // LATIN CAPITAL LETTER AE WITH ACUTE
    { 0x01FD, "aeacute" },  // LATIN SMALL LETTER AE WITH ACUTE
    { 0x01FE, "Oslashacute" },  // LATIN CAPITAL LETTER O WITH STROKE AND ACUTE
    { 0x01FF, "oslashacute" },  // LATIN SMALL LETTER O WITH STROKE AND ACUTE
    { 0x0218, "Scommaaccent" },  // LATIN CAPITAL LETTER S WITH COMMA BELOW
    { 0x0219, "scommaaccent" },  // LATIN SMALL LETTER S WITH COMMA BELOW
    { 0x021A, "Tcommaaccent" },  // LATIN CAPITAL LETTER T WITH COMMA BELOW;Duplicate
    { 0x021B, "tcommaaccent" },  // LATIN SMALL LETTER T WITH COMMA BELOW;Duplicate
    { 0x02BC, "afii57929" },  // MODIFIER LETTER APOSTROPHE
    { 0x02BD, "afii64937" },  // MODIFIER LETTER REVERSED COMMA
    { 0x02C6, "circumflex" },  // MODIFIER LETTER CIRCUMFLEX ACCENT
    { 0x02C7, "caron" },  // CARON
    { 0x02C9, "macron" },  // MODIFIER LETTER MACRON;Duplicate
    { 0x02D8, "breve" },  // BREVE
    { 0x02D9, "dotaccent" },  // DOT ABOVE
    { 0x02DA, "ring" },  // RING ABOVE
    { 0x02DB, "ogonek" },  // OGONEK
    { 0x02DC, "tilde" },  // SMALL TILDE
    { 0x02DD, "hungarumlaut" },  // DOUBLE ACUTE ACCENT
    { 0x0300, "gravecomb" },  // COMBINING GRAVE ACCENT
    { 0x0301, "acutecomb" },  // COMBINING ACUTE ACCENT
    { 0x0303, "tildecomb" },  // COMBINING TILDE
    { 0x0309, "hookabovecomb" },  // COMBINING HOOK ABOVE
    { 0x0323, "dotbelowcomb" },  // COMBINING DOT BELOW
    { 0x0384, "tonos" },  // GREEK TONOS
    { 0x0385, "dieresistonos" },  // GREEK DIALYTIKA TONOS
    { 0x0386, "Alphatonos" },  // GREEK CAPITAL LETTER ALPHA WITH TONOS
    { 0x0387, "anoteleia" },  // GREEK ANO TELEIA
    { 0x0388, "Epsilontonos" },  // GREEK CAPITAL LETTER EPSILON WITH TONOS
    { 0x0389, "Etatonos" },  // GREEK CAPITAL LETTER ETA WITH TONOS
    { 0x038A, "Iotatonos" },  // GREEK CAPITAL LETTER IOTA WITH TONOS
    { 0x038C, "Omicrontonos" },  // GREEK CAPITAL LETTER OMICRON WITH TONOS
    { 0x038E, "Upsilontonos" },  // GREEK CAPITAL LETTER UPSILON WITH TONOS
    { 0x038F, "Omegatonos" },  // GREEK CAPITAL LETTER OMEGA WITH TONOS
    { 0x0390, "iotadieresistonos" },  // GREEK SMALL LETTER IOTA WITH DIALYTIKA AND TONOS
    { 0x0391, "Alpha" },  // GREEK CAPITAL LETTER ALPHA
    { 0x0392, "Beta" },  // GREEK CAPITAL LETTER BETA
    { 0x0393, "Gamma" },  // GREEK CAPITAL LETTER GAMMA
    { 0x0394, "Delta" },  // GREEK CAPITAL LETTER DELTA;Duplicate
    { 0x0395, "Epsilon" },  // GREEK CAPITAL LETTER EPSILON
    { 0x0396, "Zeta" },  // GREEK CAPITAL LETTER ZETA
    { 0x0397, "Eta" },  // GREEK CAPITAL LETTER ETA
    { 0x0398, "Theta" },  // GREEK CAPITAL LETTER THETA
    { 0x0399, "Iota" },  // GREEK CAPITAL LETTER IOTA
    { 0x039A, "Kappa" },  // GREEK CAPITAL LETTER KAPPA
    { 0x039B, "Lambda" },  // GREEK CAPITAL LETTER LAMDA
    { 0x039C, "Mu" },  // GREEK CAPITAL LETTER MU
    { 0x039D, "Nu" },  // GREEK CAPITAL LETTER NU
    { 0x039E, "Xi" },  // GREEK CAPITAL LETTER XI
    { 0x039F, "Omicron" },  // GREEK CAPITAL LETTER OMICRON
    { 0x03A0, "Pi" },  // GREEK CAPITAL LETTER PI
    { 0x03A1, "Rho" },  // GREEK CAPITAL LETTER RHO
    { 0x03A3, "Sigma" },  // GREEK CAPITAL LETTER SIGMA
    { 0x03A4, "Tau" },  // GREEK CAPITAL LETTER TAU
    { 0x03A5, "Upsilon" },  // GREEK CAPITAL LETTER UPSILON
    { 0x03A6, "Phi" },  // GREEK CAPITAL LETTER PHI
    { 0x03A7, "Chi" },  // GREEK CAPITAL LETTER CHI
    { 0x03A8, "Psi" },  // GREEK CAPITAL LETTER PSI
    { 0x03A9, "Omega" },  // GREEK CAPITAL LETTER OMEGA;Duplicate
    { 0x03AA, "Iotadieresis" },  // GREEK CAPITAL LETTER IOTA WITH DIALYTIKA
    { 0x03AB, "Upsilondieresis" },  // GREEK CAPITAL LETTER UPSILON WITH DIALYTIKA
    { 0x03AC, "alphatonos" },  // GREEK SMALL LETTER ALPHA WITH TONOS
    { 0x03AD, "epsilontonos" },  // GREEK SMALL LETTER EPSILON WITH TONOS
    { 0x03AE, "etatonos" },  // GREEK SMALL LETTER ETA WITH TONOS
    { 0x03AF, "iotatonos" },  // GREEK SMALL LETTER IOTA WITH TONOS
    { 0x03B0, "upsilondieresistonos" },  // GREEK SMALL LETTER UPSILON WITH DIALYTIKA AND TONOS
    { 0x03B1, "alpha" },  // GREEK SMALL LETTER ALPHA
    { 0x03B2, "beta" },  // GREEK SMALL LETTER BETA
    { 0x03B3, "gamma" },  // GREEK SMALL LETTER GAMMA
    { 0x03B4, "delta" },  // GREEK SMALL LETTER DELTA
    { 0x03B5, "epsilon" },  // GREEK SMALL LETTER EPSILON
    { 0x03B6, "zeta" },  // GREEK SMALL LETTER ZETA
    { 0x03B7, "eta" },  // GREEK SMALL LETTER ETA
    { 0x03B8, "theta" },  // GREEK SMALL LETTER THETA
    { 0x03B9, "iota" },  // GREEK SMALL LETTER IOTA
    { 0x03BA, "kappa" },  // GREEK SMALL LETTER KAPPA
    { 0x03BB, "lambda" },  // GREEK SMALL LETTER LAMDA
    { 0x03BC, "mu" },  // GREEK SMALL LETTER MU;Duplicate
    { 0x03BD, "nu" },  // GREEK SMALL LETTER NU
    { 0x03BE, "xi" },  // GREEK SMALL LETTER XI
    { 0x03BF, "omicron" },  // GREEK SMALL LETTER OMICRON
    { 0x03C0, "pi" },  // GREEK SMALL LETTER PI
    { 0x03C1, "rho" },  // GREEK SMALL LETTER RHO
    { 0x03C2, "sigma1" },  // GREEK SMALL LETTER FINAL SIGMA
    { 0x03C3, "sigma" },  // GREEK SMALL LETTER SIGMA
    { 0x03C4, "tau" },  // GREEK SMALL LETTER TAU
    { 0x03C5, "upsilon" },  // GREEK SMALL LETTER UPSILON
    { 0x03C6, "phi" },  // GREEK SMALL LETTER PHI
    { 0x03C7, "chi" },  // GREEK SMALL LETTER CHI
    { 0x03C8, "psi" },  // GREEK SMALL LETTER PSI
    { 0x03C9, "omega" },  // GREEK SMALL LETTER OMEGA
    { 0x03CA, "iotadieresis" },  // GREEK SMALL LETTER IOTA WITH DIALYTIKA
    { 0x03CB, "upsilondieresis" },  // GREEK SMALL LETTER UPSILON WITH DIALYTIKA
    { 0x03CC, "omicrontonos" },  // GREEK SMALL LETTER OMICRON WITH TONOS
    { 0x03CD, "upsilontonos" },  // GREEK SMALL LETTER UPSILON WITH TONOS
    { 0x03CE, "omegatonos" },  // GREEK SMALL LETTER OMEGA WITH TONOS
    { 0x03D1, "theta1" },  // GREEK THETA SYMBOL
    { 0x03D2, "Upsilon1" },  // GREEK UPSILON WITH HOOK SYMBOL
    { 0x03D5, "phi1" },  // GREEK PHI SYMBOL
    { 0x03D6, "omega1" },  // GREEK PI SYMBOL
    { 0x0401, "afii10023" },  // CYRILLIC CAPITAL LETTER IO
    { 0x0402, "afii10051" },  // CYRILLIC CAPITAL LETTER DJE
    { 0x0403, "afii10052" },  // CYRILLIC CAPITAL LETTER GJE
    { 0x0404, "afii10053" },  // CYRILLIC CAPITAL LETTER UKRAINIAN IE
    { 0x0405, "afii10054" },  // CYRILLIC CAPITAL LETTER DZE
    { 0x0406, "afii10055" },  // CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I
    { 0x0407, "afii10056" },  // CYRILLIC CAPITAL LETTER YI
    { 0x0408, "afii10057" },  // CYRILLIC CAPITAL LETTER JE
    { 0x0409, "afii10058" },  // CYRILLIC CAPITAL LETTER LJE
    { 0x040A, "afii10059" },  // CYRILLIC CAPITAL LETTER NJE
    { 0x040B, "afii10060" },  // CYRILLIC CAPITAL LETTER TSHE
    { 0x040C, "afii10061" },  // CYRILLIC CAPITAL LETTER KJE
    { 0x040E, "afii10062" },  // CYRILLIC CAPITAL LETTER SHORT U
    { 0x040F, "afii10145" },  // CYRILLIC CAPITAL LETTER DZHE
    { 0x0410, "afii10017" },  // CYRILLIC CAPITAL LETTER A
    { 0x0411, "afii10018" },  // CYRILLIC CAPITAL LETTER BE
    { 0x0412, "afii10019" },  // CYRILLIC CAPITAL LETTER VE
    { 0x0413, "afii10020" },  // CYRILLIC CAPITAL LETTER GHE
    { 0x0414, "afii10021" },  // CYRILLIC CAPITAL LETTER DE
    { 0x0415, "afii10022" },  // CYRILLIC CAPITAL LETTER IE
    { 0x0416, "afii10024" },  // CYRILLIC CAPITAL LETTER ZHE
    { 0x0417, "afii10025" },  // CYRILLIC CAPITAL LETTER ZE
    { 0x0418, "afii10026" },  // CYRILLIC CAPITAL LETTER I
    { 0x0419, "afii10027" },  // CYRILLIC CAPITAL LETTER SHORT I
    { 0x041A, "afii10028" },  // CYRILLIC CAPITAL LETTER KA
    { 0x041B, "afii10029" },  // CYRILLIC CAPITAL LETTER EL
    { 0x041C, "afii10030" },  // CYRILLIC CAPITAL LETTER EM
    { 0x041D, "afii10031" },  // CYRILLIC CAPITAL LETTER EN
    { 0x041E, "afii10032" },  // CYRILLIC CAPITAL LETTER O
    { 0x041F, "afii10033" },  // CYRILLIC CAPITAL LETTER PE
    { 0x0420, "afii10034" },  // CYRILLIC CAPITAL LETTER ER
    { 0x0421, "afii10035" },  // CYRILLIC CAPITAL LETTER ES
    { 0x0422, "afii10036" },  // CYRILLIC CAPITAL LETTER TE
    { 0x0423, "afii10037" },  // CYRILLIC CAPITAL LETTER U
    { 0x0424, "afii10038" },  // CYRILLIC CAPITAL LETTER EF
    { 0x0425, "afii10039" },  // CYRILLIC CAPITAL LETTER HA
    { 0x0426, "afii10040" },  // CYRILLIC CAPITAL LETTER TSE
    { 0x0427, "afii10041" },  // CYRILLIC CAPITAL LETTER CHE
    { 0x0428, "afii10042" },  // CYRILLIC CAPITAL LETTER SHA
    { 0x0429, "afii10043" },  // CYRILLIC CAPITAL LETTER SHCHA
    { 0x042A, "afii10044" },  // CYRILLIC CAPITAL LETTER HARD SIGN
    { 0x042B, "afii10045" },  // CYRILLIC CAPITAL LETTER YERU
    { 0x042C, "afii10046" },  // CYRILLIC CAPITAL LETTER SOFT SIGN
    { 0x042D, "afii10047" },  // CYRILLIC CAPITAL LETTER E
    { 0x042E, "afii10048" },  // CYRILLIC CAPITAL LETTER YU
    { 0x042F, "afii10049" },  // CYRILLIC CAPITAL LETTER YA
    { 0x0430, "afii10065" },  // CYRILLIC SMALL LETTER A
    { 0x0431, "afii10066" },  // CYRILLIC SMALL LETTER BE
    { 0x0432, "afii10067" },  // CYRILLIC SMALL LETTER VE
    { 0x0433, "afii10068" },  // CYRILLIC SMALL LETTER GHE
    { 0x0434, "afii10069" },  // CYRILLIC SMALL LETTER DE
    { 0x0435, "afii10070" },  // CYRILLIC SMALL LETTER IE
    { 0x0436, "afii10072" },  // CYRILLIC SMALL LETTER ZHE
    { 0x0437, "afii10073" },  // CYRILLIC SMALL LETTER ZE
    { 0x0438, "afii10074" },  // CYRILLIC SMALL LETTER I
    { 0x0439, "afii10075" },  // CYRILLIC SMALL LETTER SHORT I
    { 0x043A, "afii10076" },  // CYRILLIC SMALL LETTER KA
    { 0x043B, "afii10077" },  // CYRILLIC SMALL LETTER EL
    { 0x043C, "afii10078" },  // CYRILLIC SMALL LETTER EM
    { 0x043D, "afii10079" },  // CYRILLIC SMALL LETTER EN
    { 0x043E, "afii10080" },  // CYRILLIC SMALL LETTER O
    { 0x043F, "afii10081" },  // CYRILLIC SMALL LETTER PE
    { 0x0440, "afii10082" },  // CYRILLIC SMALL LETTER ER
    { 0x0441, "afii10083" },  // CYRILLIC SMALL LETTER ES
    { 0x0442, "afii10084" },  // CYRILLIC SMALL LETTER TE
    { 0x0443, "afii10085" },  // CYRILLIC SMALL LETTER U
    { 0x0444, "afii10086" },  // CYRILLIC SMALL LETTER EF
    { 0x0445, "afii10087" },  // CYRILLIC SMALL LETTER HA
    { 0x0446, "afii10088" },  // CYRILLIC SMALL LETTER TSE
    { 0x0447, "afii10089" },  // CYRILLIC SMALL LETTER CHE
    { 0x0448, "afii10090" },  // CYRILLIC SMALL LETTER SHA
    { 0x0449, "afii10091" },  // CYRILLIC SMALL LETTER SHCHA
    { 0x044A, "afii10092" },  // CYRILLIC SMALL LETTER HARD SIGN
    { 0x044B, "afii10093" },  // CYRILLIC SMALL LETTER YERU
    { 0x044C, "afii10094" },  // CYRILLIC SMALL LETTER SOFT SIGN
    { 0x044D, "afii10095" },  // CYRILLIC SMALL LETTER E
    { 0x044E, "afii10096" },  // CYRILLIC SMALL LETTER YU
    { 0x044F, "afii10097" },  // CYRILLIC SMALL LETTER YA
    { 0x0451, "afii10071" },  // CYRILLIC SMALL LETTER IO
    { 0x0452, "afii10099" },  // CYRILLIC SMALL LETTER DJE
    { 0x0453, "afii10100" },  // CYRILLIC SMALL LETTER GJE
    { 0x0454, "afii10101" },  // CYRILLIC SMALL LETTER UKRAINIAN IE
    { 0x0455, "afii10102" },  // CYRILLIC SMALL LETTER DZE
    { 0x0456, "afii10103" },  // CYRILLIC SMALL LETTER BYELORUSSIAN-UKRAINIAN I
    { 0x0457, "afii10104" },  // CYRILLIC SMALL LETTER YI
    { 0x0458, "afii10105" },  // CYRILLIC SMALL LETTER JE
    { 0x0459, "afii10106" },  // CYRILLIC SMALL LETTER LJE
    { 0x045A, "afii10107" },  // CYRILLIC SMALL LETTER NJE
    { 0x045B, "afii10108" },  // CYRILLIC SMALL LETTER TSHE
    { 0x045C, "afii10109" },  // CYRILLIC SMALL LETTER KJE
    { 0x045E, "afii10110" },  // CYRILLIC SMALL LETTER SHORT U
    { 0x045F, "afii10193" },  // CYRILLIC SMALL LETTER DZHE
    { 0x0462, "afii10146" },  // CYRILLIC CAPITAL LETTER YAT
    { 0x0463, "afii10194" },  // CYRILLIC SMALL LETTER YAT
    { 0x0472, "afii10147" },  // CYRILLIC CAPITAL LETTER FITA
    { 0x0473, "afii10195" },  // CYRILLIC SMALL LETTER FITA
    { 0x0474, "afii10148" },  // CYRILLIC CAPITAL LETTER IZHITSA
    { 0x0475, "afii10196" },  // CYRILLIC SMALL LETTER IZHITSA
    { 0x0490, "afii10050" },  // CYRILLIC CAPITAL LETTER GHE WITH UPTURN
    { 0x0491, "afii10098" },  // CYRILLIC SMALL LETTER GHE WITH UPTURN
    { 0x04D9, "afii10846" },  // CYRILLIC SMALL LETTER SCHWA
    { 0x05B0, "afii57799" },  // HEBREW POINT SHEVA
    { 0x05B1, "afii57801" },  // HEBREW POINT HATAF SEGOL
    { 0x05B2, "afii57800" },  // HEBREW POINT HATAF PATAH
    { 0x05B3, "afii57802" },  // HEBREW POINT HATAF QAMATS
    { 0x05B4, "afii57793" },  // HEBREW POINT HIRIQ
    { 0x05B5, "afii57794" },  // HEBREW POINT TSERE
    { 0x05B6, "afii57795" },  // HEBREW POINT SEGOL
    { 0x05B7, "afii57798" },  // HEBREW POINT PATAH
    { 0x05B8, "afii57797" },  // HEBREW POINT QAMATS
    { 0x05B9, "afii57806" },  // HEBREW POINT HOLAM
    { 0x05BB, "afii57796" },  // HEBREW POINT QUBUTS
    { 0x05BC, "afii57807" },  // HEBREW POINT DAGESH OR MAPIQ
    { 0x05BD, "afii57839" },  // HEBREW POINT METEG
    { 0x05BE, "afii57645" },  // HEBREW PUNCTUATION MAQAF
    { 0x05BF, "afii57841" },  // HEBREW POINT RAFE
    { 0x05C0, "afii57842" },  // HEBREW PUNCTUATION PASEQ
    { 0x05C1, "afii57804" },  // HEBREW POINT SHIN DOT
    { 0x05C2, "afii57803" },  // HEBREW POINT SIN DOT
    { 0x05C3, "afii57658" },  // HEBREW PUNCTUATION SOF PASUQ
    { 0x05D0, "afii57664" },  // HEBREW LETTER ALEF
    { 0x05D1, "afii57665" },  // HEBREW LETTER BET
    { 0x05D2, "afii57666" },  // HEBREW LETTER GIMEL
    { 0x05D3, "afii57667" },  // HEBREW LETTER DALET
    { 0x05D4, "afii57668" },  // HEBREW LETTER HE
    { 0x05D5, "afii57669" },  // HEBREW LETTER VAV
    { 0x05D6, "afii57670" },  // HEBREW LETTER ZAYIN
    { 0x05D7, "afii57671" },  // HEBREW LETTER HET
    { 0x05D8, "afii57672" },  // HEBREW LETTER TET
    { 0x05D9, "afii57673" },  // HEBREW LETTER YOD
    { 0x05DA, "afii57674" },  // HEBREW LETTER FINAL KAF
    { 0x05DB, "afii57675" },  // HEBREW LETTER KAF
    { 0x05DC, "afii57676" },  // HEBREW LETTER LAMED
    { 0x05DD, "afii57677" },  // HEBREW LETTER FINAL MEM
    { 0x05DE, "afii57678" },  // HEBREW LETTER MEM
    { 0x05DF, "afii57679" },  // HEBREW LETTER FINAL NUN
    { 0x05E0, "afii57680" },  // HEBREW LETTER NUN
    { 0x05E1, "afii57681" },  // HEBREW LETTER SAMEKH
    { 0x05E2, "afii57682" },  // HEBREW LETTER AYIN
    { 0x05E3, "afii57683" },  // HEBREW LETTER FINAL PE
    { 0x05E4, "afii57684" },  // HEBREW LETTER PE
    { 0x05E5, "afii57685" },  // HEBREW LETTER FINAL TSADI
    { 0x05E6, "afii57686" },  // HEBREW LETTER TSADI
    { 0x05E7, "afii57687" },  // HEBREW LETTER QOF
    { 0x05E8, "afii57688" },  // HEBREW LETTER RESH
    { 0x05E9, "afii57689" },  // HEBREW LETTER SHIN
    { 0x05EA, "afii57690" },  // HEBREW LETTER TAV
    { 0x05F0, "afii57716" },  // HEBREW LIGATURE YIDDISH DOUBLE VAV
    { 0x05F1, "afii57717" },  // HEBREW LIGATURE YIDDISH VAV YOD
    { 0x05F2, "afii57718" },  // HEBREW LIGATURE YIDDISH DOUBLE YOD
    { 0x060C, "afii57388" },  // ARABIC COMMA
    { 0x061B, "afii57403" },  // ARABIC SEMICOLON
    { 0x061F, "afii57407" },  // ARABIC QUESTION MARK
    { 0x0621, "afii57409" },  // ARABIC LETTER HAMZA
    { 0x0622, "afii57410" },  // ARABIC LETTER ALEF WITH MADDA ABOVE
    { 0x0623, "afii57411" },  // ARABIC LETTER ALEF WITH HAMZA ABOVE
    { 0x0624, "afii57412" },  // ARABIC LETTER WAW WITH HAMZA ABOVE
    { 0x0625, "afii57413" },  // ARABIC LETTER ALEF WITH HAMZA BELOW
    { 0x0626, "afii57414" },  // ARABIC LETTER YEH WITH HAMZA ABOVE
    { 0x0627, "afii57415" },  // ARABIC LETTER ALEF
    { 0x0628, "afii57416" },  // ARABIC LETTER BEH
    { 0x0629, "afii57417" },  // ARABIC LETTER TEH MARBUTA
    { 0x062A, "afii57418" },  // ARABIC LETTER TEH
    { 0x062B, "afii57419" },  // ARABIC LETTER THEH
    { 0x062C, "afii57420" },  // ARABIC LETTER JEEM
    { 0x062D, "afii57421" },  // ARABIC LETTER HAH
    { 0x062E, "afii57422" },  // ARABIC LETTER KHAH
    { 0x062F, "afii57423" },  // ARABIC LETTER DAL
    { 0x0630, "afii57424" },  // ARABIC LETTER THAL
    { 0x0631, "afii57425" },  // ARABIC LETTER REH
    { 0x0632, "afii57426" },  // ARABIC LETTER ZAIN
    { 0x0633, "afii57427" },  // ARABIC LETTER SEEN
    { 0x0634, "afii57428" },  // ARABIC LETTER SHEEN
    { 0x0635, "afii57429" },  // ARABIC LETTER SAD
    { 0x0636, "afii57430" },  // ARABIC LETTER DAD
    { 0x0637, "afii57431" },  // ARABIC LETTER TAH
    { 0x0638, "afii57432" },  // ARABIC LETTER ZAH
    { 0x0639, "afii57433" },  // ARABIC LETTER AIN
    { 0x063A, "afii57434" },  // ARABIC LETTER GHAIN
    { 0x0640, "afii57440" },  // ARABIC TATWEEL
    { 0x0641, "afii57441" },  // ARABIC LETTER FEH
    { 0x0642, "afii57442" },  // ARABIC LETTER QAF
    { 0x0643, "afii57443" },  // ARABIC LETTER KAF
    { 0x0644, "afii57444" },  // ARABIC LETTER LAM
    { 0x0645, "afii57445" },  // ARABIC LETTER MEEM
    { 0x0646, "afii57446" },  // ARABIC LETTER NOON
    { 0x0647, "afii57470" },  // ARABIC LETTER HEH
    { 0x0648, "afii57448" },  // ARABIC LETTER WAW
    { 0x0649, "afii57449" },  // ARABIC LETTER ALEF MAKSURA
    { 0x064A, "afii57450" },  // ARABIC LETTER YEH
    { 0x064B, "afii57451" },  // ARABIC FATHATAN
    { 0x064C, "afii57452" },  // ARABIC DAMMATAN
    { 0x064D, "afii57453" },  // ARABIC KASRATAN
    { 0x064E, "afii57454" },  // ARABIC FATHA
    { 0x064F, "afii57455" },  // ARABIC DAMMA
    { 0x0650, "afii57456" },  // ARABIC KASRA
    { 0x0651, "afii57457" },  // ARABIC SHADDA
    { 0x0652, "afii57458" },  // ARABIC SUKUN
    { 0x0660, "afii57392" },  // ARABIC-INDIC DIGIT ZERO
    { 0x0661, "afii57393" },  // ARABIC-INDIC DIGIT ONE
    { 0x0662, "afii57394" },  // ARABIC-INDIC DIGIT TWO
    { 0x0663, "afii57395" },  // ARABIC-INDIC DIGIT THREE
    { 0x0664, "afii57396" },  // ARABIC-INDIC DIGIT FOUR
    { 0x0665, "afii57397" },  // ARABIC-INDIC DIGIT FIVE
    { 0x0666, "afii57398" },  // ARABIC-INDIC DIGIT SIX
    { 0x0667, "afii57399" },  // ARABIC-INDIC DIGIT SEVEN
    { 0x0668, "afii57400" },  // ARABIC-INDIC DIGIT EIGHT
    { 0x0669, "afii57401" },  // ARABIC-INDIC DIGIT NINE
    { 0x066A, "afii57381" },  // ARABIC PERCENT SIGN
    { 0x066D, "afii63167" },  // ARABIC FIVE POINTED STAR
    { 0x0679, "afii57511" },  // ARABIC LETTER TTEH
    { 0x067E, "afii57506" },  // ARABIC LETTER PEH
    { 0x0686, "afii57507" },  // ARABIC LETTER TCHEH
    { 0x0688, "afii57512" },  // ARABIC LETTER DDAL
    { 0x0691, "afii57513" },  // ARABIC LETTER RREH
    { 0x0698, "afii57508" },  // ARABIC LETTER JEH
    { 0x06A4, "afii57505" },  // ARABIC LETTER VEH
    { 0x06AF, "afii57509" },  // ARABIC LETTER GAF
    { 0x06BA, "afii57514" },  // ARABIC LETTER NOON GHUNNA
    { 0x06D2, "afii57519" },  // ARABIC LETTER YEH BARREE
    { 0x06D5, "afii57534" },  // ARABIC LETTER AE
    { 0x1E80, "Wgrave" },  // LATIN CAPITAL LETTER W WITH GRAVE
    { 0x1E81, "wgrave" },  // LATIN SMALL LETTER W WITH GRAVE
    { 0x1E82, "Wacute" },  // LATIN CAPITAL LETTER W WITH ACUTE
    { 0x1E83, "wacute" },  // LATIN SMALL LETTER W WITH ACUTE
    { 0x1E84, "Wdieresis" },  // LATIN CAPITAL LETTER W WITH DIAERESIS
    { 0x1E85, "wdieresis" },  // LATIN SMALL LETTER W WITH DIAERESIS
    { 0x1EF2, "Ygrave" },  // LATIN CAPITAL LETTER Y WITH GRAVE
    { 0x1EF3, "ygrave" },  // LATIN SMALL LETTER Y WITH GRAVE
    { 0x200C, "afii61664" },  // ZERO WIDTH NON-JOINER
    { 0x200D, "afii301" },  // ZERO WIDTH JOINER
    { 0x200E, "afii299" },  // LEFT-TO-RIGHT MARK
    { 0x200F, "afii300" },  // RIGHT-TO-LEFT MARK
    { 0x2012, "figuredash" },  // FIGURE DASH
    { 0x2013, "endash" },  // EN DASH
    { 0x2014, "emdash" },  // EM DASH
    { 0x2015, "afii00208" },  // HORIZONTAL BAR
    { 0x2017, "underscoredbl" },  // DOUBLE LOW LINE
    { 0x2018, "quoteleft" },  // LEFT SINGLE QUOTATION MARK
    { 0x2019, "quoteright" },  // RIGHT SINGLE QUOTATION MARK
    { 0x201A, "quotesinglbase" },  // SINGLE LOW-9 QUOTATION MARK
    { 0x201B, "quotereversed" },  // SINGLE HIGH-REVERSED-9 QUOTATION MARK
    { 0x201C, "quotedblleft" },  // LEFT DOUBLE QUOTATION MARK
    { 0x201D, "quotedblright" },  // RIGHT DOUBLE QUOTATION MARK
    { 0x201E, "quotedblbase" },  // DOUBLE LOW-9 QUOTATION MARK
    { 0x2020, "dagger" },  // DAGGER
    { 0x2021, "daggerdbl" },  // DOUBLE DAGGER
    { 0x2022, "bullet" },  // BULLET
    { 0x2024, "onedotenleader" },  // ONE DOT LEADER
    { 0x2025, "twodotenleader" },  // TWO DOT LEADER
    { 0x2026, "ellipsis" },  // HORIZONTAL ELLIPSIS
    { 0x202C, "afii61573" },  // POP DIRECTIONAL FORMATTING
    { 0x202D, "afii61574" },  // LEFT-TO-RIGHT OVERRIDE
    { 0x202E, "afii61575" },  // RIGHT-TO-LEFT OVERRIDE
    { 0x2030, "perthousand" },  // PER MILLE SIGN
    { 0x2032, "minute" },  // PRIME
    { 0x2033, "second" },  // DOUBLE PRIME
    { 0x2039, "guilsinglleft" },  // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
    { 0x203A, "guilsinglright" },  // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
    { 0x203C, "exclamdbl" },  // DOUBLE EXCLAMATION MARK
    { 0x2044, "fraction" },  // FRACTION SLASH
    { 0x2070, "zerosuperior" },  // SUPERSCRIPT ZERO
    { 0x2074, "foursuperior" },  // SUPERSCRIPT FOUR
    { 0x2075, "fivesuperior" },  // SUPERSCRIPT FIVE
    { 0x2076, "sixsuperior" },  // SUPERSCRIPT SIX
    { 0x2077, "sevensuperior" },  // SUPERSCRIPT SEVEN
    { 0x2078, "eightsuperior" },  // SUPERSCRIPT EIGHT
    { 0x2079, "ninesuperior" },  // SUPERSCRIPT NINE
    { 0x207D, "parenleftsuperior" },  // SUPERSCRIPT LEFT PARENTHESIS
    { 0x207E, "parenrightsuperior" },  // SUPERSCRIPT RIGHT PARENTHESIS
    { 0x207F, "nsuperior" },  // SUPERSCRIPT LATIN SMALL LETTER N
    { 0x2080, "zeroinferior" },  // SUBSCRIPT ZERO
    { 0x2081, "oneinferior" },  // SUBSCRIPT ONE
    { 0x2082, "twoinferior" },  // SUBSCRIPT TWO
    { 0x2083, "threeinferior" },  // SUBSCRIPT THREE
    { 0x2084, "fourinferior" },  // SUBSCRIPT FOUR
    { 0x2085, "fiveinferior" },  // SUBSCRIPT FIVE
    { 0x2086, "sixinferior" },  // SUBSCRIPT SIX
    { 0x2087, "seveninferior" },  // SUBSCRIPT SEVEN
    { 0x2088, "eightinferior" },  // SUBSCRIPT EIGHT
    { 0x2089, "nineinferior" },  // SUBSCRIPT NINE
    { 0x208D, "parenleftinferior" },  // SUBSCRIPT LEFT PARENTHESIS
    { 0x208E, "parenrightinferior" },  // SUBSCRIPT RIGHT PARENTHESIS
    { 0x20A1, "colonmonetary" },  // COLON SIGN
    { 0x20A3, "franc" },  // FRENCH FRANC SIGN
    { 0x20A4, "lira" },  // LIRA SIGN
    { 0x20A7, "peseta" },  // PESETA SIGN
    { 0x20AA, "afii57636" },  // NEW SHEQEL SIGN
    { 0x20AB, "dong" },  // DONG SIGN
    { 0x20AC, "Euro" },  // EURO SIGN
    { 0x2105, "afii61248" },  // CARE OF
    { 0x2111, "Ifraktur" },  // BLACK-LETTER CAPITAL I
    { 0x2113, "afii61289" },  // SCRIPT SMALL L
    { 0x2116, "afii61352" },  // NUMERO SIGN
    { 0x2118, "weierstrass" },  // SCRIPT CAPITAL P
    { 0x211C, "Rfraktur" },  // BLACK-LETTER CAPITAL R
    { 0x211E, "prescription" },  // PRESCRIPTION TAKE
    { 0x2122, "trademark" },  // TRADE MARK SIGN
    { 0x2126, "Omega" },  // OHM SIGN
    { 0x212E, "estimated" },  // ESTIMATED SYMBOL
    { 0x2135, "aleph" },  // ALEF SYMBOL
    { 0x2153, "onethird" },  // VULGAR FRACTION ONE THIRD
    { 0x2154, "twothirds" },  // VULGAR FRACTION TWO THIRDS
    { 0x215B, "oneeighth" },  // VULGAR FRACTION ONE EIGHTH
    { 0x215C, "threeeighths" },  // VULGAR FRACTION THREE EIGHTHS
    { 0x215D, "fiveeighths" },  // VULGAR FRACTION FIVE EIGHTHS
    { 0x215E, "seveneighths" },  // VULGAR FRACTION SEVEN EIGHTHS
    { 0x2190, "arrowleft" },  // LEFTWARDS ARROW
    { 0x2191, "arrowup" },  // UPWARDS ARROW
    { 0x2192, "arrowright" },  // RIGHTWARDS ARROW
    { 0x2193, "arrowdown" },  // DOWNWARDS ARROW
    { 0x2194, "arrowboth" },  // LEFT RIGHT ARROW
    { 0x2195, "arrowupdn" },  // UP DOWN ARROW
    { 0x21A8, "arrowupdnbse" },  // UP DOWN ARROW WITH BASE
    { 0x21B5, "carriagereturn" },  // DOWNWARDS ARROW WITH CORNER LEFTWARDS
    { 0x21D0, "arrowdblleft" },  // LEFTWARDS DOUBLE ARROW
    { 0x21D1, "arrowdblup" },  // UPWARDS DOUBLE ARROW
    { 0x21D2, "arrowdblright" },  // RIGHTWARDS DOUBLE ARROW
    { 0x21D3, "arrowdbldown" },  // DOWNWARDS DOUBLE ARROW
    { 0x21D4, "arrowdblboth" },  // LEFT RIGHT DOUBLE ARROW
    { 0x2200, "universal" },  // FOR ALL
    { 0x2202, "partialdiff" },  // PARTIAL DIFFERENTIAL
    { 0x2203, "existential" },  // THERE EXISTS
    { 0x2205, "emptyset" },  // EMPTY SET
    { 0x2206, "Delta" },  // INCREMENT
    { 0x2207, "gradient" },  // NABLA
    { 0x2208, "element" },  // ELEMENT OF
    { 0x2209, "notelement" },  // NOT AN ELEMENT OF
    { 0x220B, "suchthat" },  // CONTAINS AS MEMBER
    { 0x220F, "product" },  // N-ARY PRODUCT
    { 0x2211, "summation" },  // N-ARY SUMMATION
    { 0x2212, "minus" },  // MINUS SIGN
    { 0x2215, "fraction" },  // DIVISION SLASH;Duplicate
    { 0x2217, "asteriskmath" },  // ASTERISK OPERATOR
    { 0x2219, "periodcentered" },  // BULLET OPERATOR;Duplicate
    { 0x221A, "radical" },  // SQUARE ROOT
    { 0x221D, "proportional" },  // PROPORTIONAL TO
    { 0x221E, "infinity" },  // INFINITY
    { 0x221F, "orthogonal" },  // RIGHT ANGLE
    { 0x2220, "angle" },  // ANGLE
    { 0x2227, "logicaland" },  // LOGICAL AND
    { 0x2228, "logicalor" },  // LOGICAL OR
    { 0x2229, "intersection" },  // INTERSECTION
    { 0x222A, "union" },  // UNION
    { 0x222B, "integral" },  // INTEGRAL
    { 0x2234, "therefore" },  // THEREFORE
    { 0x223C, "similar" },  // TILDE OPERATOR
    { 0x2245, "congruent" },  // APPROXIMATELY EQUAL TO
    { 0x2248, "approxequal" },  // ALMOST EQUAL TO
    { 0x2260, "notequal" },  // NOT EQUAL TO
    { 0x2261, "equivalence" },  // IDENTICAL TO
    { 0x2264, "lessequal" },  // LESS-THAN OR EQUAL TO
    { 0x2265, "greaterequal" },  // GREATER-THAN OR EQUAL TO
    { 0x2282, "propersubset" },  // SUBSET OF
    { 0x2283, "propersuperset" },  // SUPERSET OF
    { 0x2284, "notsubset" },  // NOT A SUBSET OF
    { 0x2286, "reflexsubset" },  // SUBSET OF OR EQUAL TO
    { 0x2287, "reflexsuperset" },  // SUPERSET OF OR EQUAL TO
    { 0x2295, "circleplus" },  // CIRCLED PLUS
    { 0x2297, "circlemultiply" },  // CIRCLED TIMES
    { 0x22A5, "perpendicular" },  // UP TACK
    { 0x22C5, "dotmath" },  // DOT OPERATOR
    { 0x2302, "house" },  // HOUSE
    { 0x2310, "revlogicalnot" },  // REVERSED NOT SIGN
    { 0x2320, "integraltp" },  // TOP HALF INTEGRAL
    { 0x2321, "integralbt" },  // BOTTOM HALF INTEGRAL
    { 0x2329, "angleleft" },  // LEFT-POINTING ANGLE BRACKET
    { 0x232A, "angleright" },  // RIGHT-POINTING ANGLE BRACKET
    { 0x2500, "SF100000" },  // BOX DRAWINGS LIGHT HORIZONTAL
    { 0x2502, "SF110000" },  // BOX DRAWINGS LIGHT VERTICAL
    { 0x250C, "SF010000" },  // BOX DRAWINGS LIGHT DOWN AND RIGHT
    { 0x2510, "SF030000" },  // BOX DRAWINGS LIGHT DOWN AND LEFT
    { 0x2514, "SF020000" },  // BOX DRAWINGS LIGHT UP AND RIGHT
    { 0x2518, "SF040000" },  // BOX DRAWINGS LIGHT UP AND LEFT
    { 0x251C, "SF080000" },  // BOX DRAWINGS LIGHT VERTICAL AND RIGHT
    { 0x2524, "SF090000" },  // BOX DRAWINGS LIGHT VERTICAL AND LEFT
    { 0x252C, "SF060000" },  // BOX DRAWINGS LIGHT DOWN AND HORIZONTAL
    { 0x2534, "SF070000" },  // BOX DRAWINGS LIGHT UP AND HORIZONTAL
    { 0x253C, "SF050000" },  // BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL
    { 0x2550, "SF430000" },  // BOX DRAWINGS DOUBLE HORIZONTAL
    { 0x2551, "SF240000" },  // BOX DRAWINGS DOUBLE VERTICAL
    { 0x2552, "SF510000" },  // BOX DRAWINGS DOWN SINGLE AND RIGHT DOUBLE
    { 0x2553, "SF520000" },  // BOX DRAWINGS DOWN DOUBLE AND RIGHT SINGLE
    { 0x2554, "SF390000" },  // BOX DRAWINGS DOUBLE DOWN AND RIGHT
    { 0x2555, "SF220000" },  // BOX DRAWINGS DOWN SINGLE AND LEFT DOUBLE
    { 0x2556, "SF210000" },  // BOX DRAWINGS DOWN DOUBLE AND LEFT SINGLE
    { 0x2557, "SF250000" },  // BOX DRAWINGS DOUBLE DOWN AND LEFT
    { 0x2558, "SF500000" },  // BOX DRAWINGS UP SINGLE AND RIGHT DOUBLE
    { 0x2559, "SF490000" },  // BOX DRAWINGS UP DOUBLE AND RIGHT SINGLE
    { 0x255A, "SF380000" },  // BOX DRAWINGS DOUBLE UP AND RIGHT
    { 0x255B, "SF280000" },  // BOX DRAWINGS UP SINGLE AND LEFT DOUBLE
    { 0x255C, "SF270000" },  // BOX DRAWINGS UP DOUBLE AND LEFT SINGLE
    { 0x255D, "SF260000" },  // BOX DRAWINGS DOUBLE UP AND LEFT
    { 0x255E, "SF360000" },  // BOX DRAWINGS VERTICAL SINGLE AND RIGHT DOUBLE
    { 0x255F, "SF370000" },  // BOX DRAWINGS VERTICAL DOUBLE AND RIGHT SINGLE
    { 0x2560, "SF420000" },  // BOX DRAWINGS DOUBLE VERTICAL AND RIGHT
    { 0x2561, "SF190000" },  // BOX DRAWINGS VERTICAL SINGLE AND LEFT DOUBLE
    { 0x2562, "SF200000" },  // BOX DRAWINGS VERTICAL DOUBLE AND LEFT SINGLE
    { 0x2563, "SF230000" },  // BOX DRAWINGS DOUBLE VERTICAL AND LEFT
    { 0x2564, "SF470000" },  // BOX DRAWINGS DOWN SINGLE AND HORIZONTAL DOUBLE
    { 0x2565, "SF480000" },  // BOX DRAWINGS DOWN DOUBLE AND HORIZONTAL SINGLE
    { 0x2566, "SF410000" },  // BOX DRAWINGS DOUBLE DOWN AND HORIZONTAL
    { 0x2567, "SF450000" },  // BOX DRAWINGS UP SINGLE AND HORIZONTAL DOUBLE
    { 0x2568, "SF460000" },  // BOX DRAWINGS UP DOUBLE AND HORIZONTAL SINGLE
    { 0x2569, "SF400000" },  // BOX DRAWINGS DOUBLE UP AND HORIZONTAL
    { 0x256A, "SF540000" },  // BOX DRAWINGS VERTICAL SINGLE AND HORIZONTAL DOUBLE
    { 0x256B, "SF530000" },  // BOX DRAWINGS VERTICAL DOUBLE AND HORIZONTAL SINGLE
    { 0x256C, "SF440000" },  // BOX DRAWINGS DOUBLE VERTICAL AND HORIZONTAL
    { 0x2580, "upblock" },  // UPPER HALF BLOCK
    { 0x2584, "dnblock" },  // LOWER HALF BLOCK
    { 0x2588, "block" },  // FULL BLOCK
    { 0x258C, "lfblock" },  // LEFT HALF BLOCK
    { 0x2590, "rtblock" },  // RIGHT HALF BLOCK
    { 0x2591, "ltshade" },  // LIGHT SHADE
    { 0x2592, "shade" },  // MEDIUM SHADE
    { 0x2593, "dkshade" },  // DARK SHADE
    { 0x25A0, "filledbox" },  // BLACK SQUARE
    { 0x25A1, "H22073" },  // WHITE SQUARE
    { 0x25AA, "H18543" },  // BLACK SMALL SQUARE
    { 0x25AB, "H18551" },  // WHITE SMALL SQUARE
    { 0x25AC, "filledrect" },  // BLACK RECTANGLE
    { 0x25B2, "triagup" },  // BLACK UP-POINTING TRIANGLE
    { 0x25BA, "triagrt" },  // BLACK RIGHT-POINTING POINTER
    { 0x25BC, "triagdn" },  // BLACK DOWN-POINTING TRIANGLE
    { 0x25C4, "triaglf" },  // BLACK LEFT-POINTING POINTER
    { 0x25CA, "lozenge" },  // LOZENGE
    { 0x25CB, "circle" },  // WHITE CIRCLE
    { 0x25CF, "H18533" },  // BLACK CIRCLE
    { 0x25D8, "invbullet" },  // INVERSE BULLET
    { 0x25D9, "invcircle" },  // INVERSE WHITE CIRCLE
    { 0x25E6, "openbullet" },  // WHITE BULLET
    { 0x263A, "smileface" },  // WHITE SMILING FACE
    { 0x263B, "invsmileface" },  // BLACK SMILING FACE
    { 0x263C, "sun" },  // WHITE SUN WITH RAYS
    { 0x2640, "female" },  // FEMALE SIGN
    { 0x2642, "male" },  // MALE SIGN
    { 0x2660, "spade" },  // BLACK SPADE SUIT
    { 0x2663, "club" },  // BLACK CLUB SUIT
    { 0x2665, "heart" },  // BLACK HEART SUIT
    { 0x2666, "diamond" },  // BLACK DIAMOND SUIT
    { 0x266A, "musicalnote" },  // EIGHTH NOTE
    { 0x266B, "musicalnotedbl" },  // BEAMED EIGHTH NOTES
    { 0xF6BE, "dotlessj" },  // LATIN SMALL LETTER DOTLESS J
    { 0xF6BF, "LL" },  // LATIN CAPITAL LETTER LL
    { 0xF6C0, "ll" },  // LATIN SMALL LETTER LL
    { 0xF6C1, "Scedilla" },  // LATIN CAPITAL LETTER S WITH CEDILLA;Duplicate
    { 0xF6C2, "scedilla" },  // LATIN SMALL LETTER S WITH CEDILLA;Duplicate
    { 0xF6C3, "commaaccent" },  // COMMA BELOW
    { 0xF6C4, "afii10063" },  // CYRILLIC SMALL LETTER GHE VARIANT
    { 0xF6C5, "afii10064" },  // CYRILLIC SMALL LETTER BE VARIANT
    { 0xF6C6, "afii10192" },  // CYRILLIC SMALL LETTER DE VARIANT
    { 0xF6C7, "afii10831" },  // CYRILLIC SMALL LETTER PE VARIANT
    { 0xF6C8, "afii10832" },  // CYRILLIC SMALL LETTER TE VARIANT
    { 0xF6C9, "Acute" },  // CAPITAL ACUTE ACCENT
    { 0xF6CA, "Caron" },  // CAPITAL CARON
    { 0xF6CB, "Dieresis" },  // CAPITAL DIAERESIS
    { 0xF6CC, "DieresisAcute" },  // CAPITAL DIAERESIS ACUTE ACCENT
    { 0xF6CD, "DieresisGrave" },  // CAPITAL DIAERESIS GRAVE ACCENT
    { 0xF6CE, "Grave" },  // CAPITAL GRAVE ACCENT
    { 0xF6CF, "Hungarumlaut" },  // CAPITAL DOUBLE ACUTE ACCENT
    { 0xF6D0, "Macron" },  // CAPITAL MACRON
    { 0xF6D1, "cyrBreve" },  // CAPITAL CYRILLIC BREVE
    { 0xF6D2, "cyrFlex" },  // CAPITAL CYRILLIC CIRCUMFLEX
    { 0xF6D3, "dblGrave" },  // CAPITAL DOUBLE GRAVE ACCENT
    { 0xF6D4, "cyrbreve" },  // CYRILLIC BREVE
    { 0xF6D5, "cyrflex" },  // CYRILLIC CIRCUMFLEX
    { 0xF6D6, "dblgrave" },  // DOUBLE GRAVE ACCENT
    { 0xF6D7, "dieresisacute" },  // DIAERESIS ACUTE ACCENT
    { 0xF6D8, "dieresisgrave" },  // DIAERESIS GRAVE ACCENT
    { 0xF6D9, "copyrightserif" },  // COPYRIGHT SIGN SERIF
    { 0xF6DA, "registerserif" },  // REGISTERED SIGN SERIF
    { 0xF6DB, "trademarkserif" },  // TRADE MARK SIGN SERIF
    { 0xF6DC, "onefitted" },  // PROPORTIONAL DIGIT ONE
    { 0xF6DD, "rupiah" },  // RUPIAH SIGN
    { 0xF6DE, "threequartersemdash" },  // THREE QUARTERS EM DASH
    { 0xF6DF, "centinferior" },  // SUBSCRIPT CENT SIGN
    { 0xF6E0, "centsuperior" },  // SUPERSCRIPT CENT SIGN
    { 0xF6E1, "commainferior" },  // SUBSCRIPT COMMA
    { 0xF6E2, "commasuperior" },  // SUPERSCRIPT COMMA
    { 0xF6E3, "dollarinferior" },  // SUBSCRIPT DOLLAR SIGN
    { 0xF6E4, "dollarsuperior" },  // SUPERSCRIPT DOLLAR SIGN
    { 0xF6E5, "hypheninferior" },  // SUBSCRIPT HYPHEN-MINUS
    { 0xF6E6, "hyphensuperior" },  // SUPERSCRIPT HYPHEN-MINUS
    { 0xF6E7, "periodinferior" },  // SUBSCRIPT FULL STOP
    { 0xF6E8, "periodsuperior" },  // SUPERSCRIPT FULL STOP
    { 0xF6E9, "asuperior" },  // SUPERSCRIPT LATIN SMALL LETTER A
    { 0xF6EA, "bsuperior" },  // SUPERSCRIPT LATIN SMALL LETTER B
    { 0xF6EB, "dsuperior" },  // SUPERSCRIPT LATIN SMALL LETTER D
    { 0xF6EC, "esuperior" },  // SUPERSCRIPT LATIN SMALL LETTER E
    { 0xF6ED, "isuperior" },  // SUPERSCRIPT LATIN SMALL LETTER I
    { 0xF6EE, "lsuperior" },  // SUPERSCRIPT LATIN SMALL LETTER L
    { 0xF6EF, "msuperior" },  // SUPERSCRIPT LATIN SMALL LETTER M
    { 0xF6F0, "osuperior" },  // SUPERSCRIPT LATIN SMALL LETTER O
    { 0xF6F1, "rsuperior" },  // SUPERSCRIPT LATIN SMALL LETTER R
    { 0xF6F2, "ssuperior" },  // SUPERSCRIPT LATIN SMALL LETTER S
    { 0xF6F3, "tsuperior" },  // SUPERSCRIPT LATIN SMALL LETTER T
    { 0xF6F4, "Brevesmall" },  // SMALL CAPITAL BREVE
    { 0xF6F5, "Caronsmall" },  // SMALL CAPITAL CARON
    { 0xF6F6, "Circumflexsmall" },  // SMALL CAPITAL MODIFIER LETTER CIRCUMFLEX ACCENT
    { 0xF6F7, "Dotaccentsmall" },  // SMALL CAPITAL DOT ABOVE
    { 0xF6F8, "Hungarumlautsmall" },  // SMALL CAPITAL DOUBLE ACUTE ACCENT
    { 0xF6F9, "Lslashsmall" },  // LATIN SMALL CAPITAL LETTER L WITH STROKE
    { 0xF6FA, "OEsmall" },  // LATIN SMALL CAPITAL LIGATURE OE
    { 0xF6FB, "Ogoneksmall" },  // SMALL CAPITAL OGONEK
    { 0xF6FC, "Ringsmall" },  // SMALL CAPITAL RING ABOVE
    { 0xF6FD, "Scaronsmall" },  // LATIN SMALL CAPITAL LETTER S WITH CARON
    { 0xF6FE, "Tildesmall" },  // SMALL CAPITAL SMALL TILDE
    { 0xF6FF, "Zcaronsmall" },  // LATIN SMALL CAPITAL LETTER Z WITH CARON
    { 0xF721, "exclamsmall" },  // SMALL CAPITAL EXCLAMATION MARK
    { 0xF724, "dollaroldstyle" },  // OLDSTYLE DOLLAR SIGN
    { 0xF726, "ampersandsmall" },  // SMALL CAPITAL AMPERSAND
    { 0xF730, "zerooldstyle" },  // OLDSTYLE DIGIT ZERO
    { 0xF731, "oneoldstyle" },  // OLDSTYLE DIGIT ONE
    { 0xF732, "twooldstyle" },  // OLDSTYLE DIGIT TWO
    { 0xF733, "threeoldstyle" },  // OLDSTYLE DIGIT THREE
    { 0xF734, "fouroldstyle" },  // OLDSTYLE DIGIT FOUR
    { 0xF735, "fiveoldstyle" },  // OLDSTYLE DIGIT FIVE
    { 0xF736, "sixoldstyle" },  // OLDSTYLE DIGIT SIX
    { 0xF737, "sevenoldstyle" },  // OLDSTYLE DIGIT SEVEN
    { 0xF738, "eightoldstyle" },  // OLDSTYLE DIGIT EIGHT
    { 0xF739, "nineoldstyle" },  // OLDSTYLE DIGIT NINE
    { 0xF73F, "questionsmall" },  // SMALL CAPITAL QUESTION MARK
    { 0xF760, "Gravesmall" },  // SMALL CAPITAL GRAVE ACCENT
    { 0xF761, "Asmall" },  // LATIN SMALL CAPITAL LETTER A
    { 0xF762, "Bsmall" },  // LATIN SMALL CAPITAL LETTER B
    { 0xF763, "Csmall" },  // LATIN SMALL CAPITAL LETTER C
    { 0xF764, "Dsmall" },  // LATIN SMALL CAPITAL LETTER D
    { 0xF765, "Esmall" },  // LATIN SMALL CAPITAL LETTER E
    { 0xF766, "Fsmall" },  // LATIN SMALL CAPITAL LETTER F
    { 0xF767, "Gsmall" },  // LATIN SMALL CAPITAL LETTER G
    { 0xF768, "Hsmall" },  // LATIN SMALL CAPITAL LETTER H
    { 0xF769, "Ismall" },  // LATIN SMALL CAPITAL LETTER I
    { 0xF76A, "Jsmall" },  // LATIN SMALL CAPITAL LETTER J
    { 0xF76B, "Ksmall" },  // LATIN SMALL CAPITAL LETTER K
    { 0xF76C, "Lsmall" },  // LATIN SMALL CAPITAL LETTER L
    { 0xF76D, "Msmall" },  // LATIN SMALL CAPITAL LETTER M
    { 0xF76E, "Nsmall" },  // LATIN SMALL CAPITAL LETTER N
    { 0xF76F, "Osmall" },  // LATIN SMALL CAPITAL LETTER O
    { 0xF770, "Psmall" },  // LATIN SMALL CAPITAL LETTER P
    { 0xF771, "Qsmall" },  // LATIN SMALL CAPITAL LETTER Q
    { 0xF772, "Rsmall" },  // LATIN SMALL CAPITAL LETTER R
    { 0xF773, "Ssmall" },  // LATIN SMALL CAPITAL LETTER S
    { 0xF774, "Tsmall" },  // LATIN SMALL CAPITAL LETTER T
    { 0xF775, "Usmall" },  // LATIN SMALL CAPITAL LETTER U
    { 0xF776, "Vsmall" },  // LATIN SMALL CAPITAL LETTER V
    { 0xF777, "Wsmall" },  // LATIN SMALL CAPITAL LETTER W
    { 0xF778, "Xsmall" },  // LATIN SMALL CAPITAL LETTER X
    { 0xF779, "Ysmall" },  // LATIN SMALL CAPITAL LETTER Y
    { 0xF77A, "Zsmall" },  // LATIN SMALL CAPITAL LETTER Z
    { 0xF7A1, "exclamdownsmall" },  // SMALL CAPITAL INVERTED EXCLAMATION MARK
    { 0xF7A2, "centoldstyle" },  // OLDSTYLE CENT SIGN
    { 0xF7A8, "Dieresissmall" },  // SMALL CAPITAL DIAERESIS
    { 0xF7AF, "Macronsmall" },  // SMALL CAPITAL MACRON
    { 0xF7B4, "Acutesmall" },  // SMALL CAPITAL ACUTE ACCENT
    { 0xF7B8, "Cedillasmall" },  // SMALL CAPITAL CEDILLA
    { 0xF7BF, "questiondownsmall" },  // SMALL CAPITAL INVERTED QUESTION MARK
    { 0xF7E0, "Agravesmall" },  // LATIN SMALL CAPITAL LETTER A WITH GRAVE
    { 0xF7E1, "Aacutesmall" },  // LATIN SMALL CAPITAL LETTER A WITH ACUTE
    { 0xF7E2, "Acircumflexsmall" },  // LATIN SMALL CAPITAL LETTER A WITH CIRCUMFLEX
    { 0xF7E3, "Atildesmall" },  // LATIN SMALL CAPITAL LETTER A WITH TILDE
    { 0xF7E4, "Adieresissmall" },  // LATIN SMALL CAPITAL LETTER A WITH DIAERESIS
    { 0xF7E5, "Aringsmall" },  // LATIN SMALL CAPITAL LETTER A WITH RING ABOVE
    { 0xF7E6, "AEsmall" },  // LATIN SMALL CAPITAL LETTER AE
    { 0xF7E7, "Ccedillasmall" },  // LATIN SMALL CAPITAL LETTER C WITH CEDILLA
    { 0xF7E8, "Egravesmall" },  // LATIN SMALL CAPITAL LETTER E WITH GRAVE
    { 0xF7E9, "Eacutesmall" },  // LATIN SMALL CAPITAL LETTER E WITH ACUTE
    { 0xF7EA, "Ecircumflexsmall" },  // LATIN SMALL CAPITAL LETTER E WITH CIRCUMFLEX
    { 0xF7EB, "Edieresissmall" },  // LATIN SMALL CAPITAL LETTER E WITH DIAERESIS
    { 0xF7EC, "Igravesmall" },  // LATIN SMALL CAPITAL LETTER I WITH GRAVE
    { 0xF7ED, "Iacutesmall" },  // LATIN SMALL CAPITAL LETTER I WITH ACUTE
    { 0xF7EE, "Icircumflexsmall" },  // LATIN SMALL CAPITAL LETTER I WITH CIRCUMFLEX
    { 0xF7EF, "Idieresissmall" },  // LATIN SMALL CAPITAL LETTER I WITH DIAERESIS
    { 0xF7F0, "Ethsmall" },  // LATIN SMALL CAPITAL LETTER ETH
    { 0xF7F1, "Ntildesmall" },  // LATIN SMALL CAPITAL LETTER N WITH TILDE
    { 0xF7F2, "Ogravesmall" },  // LATIN SMALL CAPITAL LETTER O WITH GRAVE
    { 0xF7F3, "Oacutesmall" },  // LATIN SMALL CAPITAL LETTER O WITH ACUTE
    { 0xF7F4, "Ocircumflexsmall" },  // LATIN SMALL CAPITAL LETTER O WITH CIRCUMFLEX
    { 0xF7F5, "Otildesmall" },  // LATIN SMALL CAPITAL LETTER O WITH TILDE
    { 0xF7F6, "Odieresissmall" },  // LATIN SMALL CAPITAL LETTER O WITH DIAERESIS
    { 0xF7F8, "Oslashsmall" },  // LATIN SMALL CAPITAL LETTER O WITH STROKE
    { 0xF7F9, "Ugravesmall" },  // LATIN SMALL CAPITAL LETTER U WITH GRAVE
    { 0xF7FA, "Uacutesmall" },  // LATIN SMALL CAPITAL LETTER U WITH ACUTE
    { 0xF7FB, "Ucircumflexsmall" },  // LATIN SMALL CAPITAL LETTER U WITH CIRCUMFLEX
    { 0xF7FC, "Udieresissmall" },  // LATIN SMALL CAPITAL LETTER U WITH DIAERESIS
    { 0xF7FD, "Yacutesmall" },  // LATIN SMALL CAPITAL LETTER Y WITH ACUTE
    { 0xF7FE, "Thornsmall" },  // LATIN SMALL CAPITAL LETTER THORN
    { 0xF7FF, "Ydieresissmall" },  // LATIN SMALL CAPITAL LETTER Y WITH DIAERESIS
    { 0xF8E5, "radicalex" },  // RADICAL EXTENDER
    { 0xF8E6, "arrowvertex" },  // VERTICAL ARROW EXTENDER
    { 0xF8E7, "arrowhorizex" },  // HORIZONTAL ARROW EXTENDER
    { 0xF8E8, "registersans" },  // REGISTERED SIGN SANS SERIF
    { 0xF8E9, "copyrightsans" },  // COPYRIGHT SIGN SANS SERIF
    { 0xF8EA, "trademarksans" },  // TRADE MARK SIGN SANS SERIF
    { 0xF8EB, "parenlefttp" },  // LEFT PAREN TOP
    { 0xF8EC, "parenleftex" },  // LEFT PAREN EXTENDER
    { 0xF8ED, "parenleftbt" },  // LEFT PAREN BOTTOM
    { 0xF8EE, "bracketlefttp" },  // LEFT SQUARE BRACKET TOP
    { 0xF8EF, "bracketleftex" },  // LEFT SQUARE BRACKET EXTENDER
    { 0xF8F0, "bracketleftbt" },  // LEFT SQUARE BRACKET BOTTOM
    { 0xF8F1, "bracelefttp" },  // LEFT CURLY BRACKET TOP
    { 0xF8F2, "braceleftmid" },  // LEFT CURLY BRACKET MID
    { 0xF8F3, "braceleftbt" },  // LEFT CURLY BRACKET BOTTOM
    { 0xF8F4, "braceex" },  // CURLY BRACKET EXTENDER
    { 0xF8F5, "integralex" },  // INTEGRAL EXTENDER
    { 0xF8F6, "parenrighttp" },  // RIGHT PAREN TOP
    { 0xF8F7, "parenrightex" },  // RIGHT PAREN EXTENDER
    { 0xF8F8, "parenrightbt" },  // RIGHT PAREN BOTTOM
    { 0xF8F9, "bracketrighttp" },  // RIGHT SQUARE BRACKET TOP
    { 0xF8FA, "bracketrightex" },  // RIGHT SQUARE BRACKET EXTENDER
    { 0xF8FB, "bracketrightbt" },  // RIGHT SQUARE BRACKET BOTTOM
    { 0xF8FC, "bracerighttp" },  // RIGHT CURLY BRACKET TOP
    { 0xF8FD, "bracerightmid" },  // RIGHT CURLY BRACKET MID
    { 0xF8FE, "bracerightbt" },  // RIGHT CURLY BRACKET BOTTOM
    { 0xFB00, "ff" },  // LATIN SMALL LIGATURE FF
    { 0xFB01, "fi" },  // LATIN SMALL LIGATURE FI
    { 0xFB02, "fl" },  // LATIN SMALL LIGATURE FL
    { 0xFB03, "ffi" },  // LATIN SMALL LIGATURE FFI
    { 0xFB04, "ffl" },  // LATIN SMALL LIGATURE FFL
    { 0xFB1F, "afii57705" },  // HEBREW LIGATURE YIDDISH YOD YOD PATAH
    { 0xFB2A, "afii57694" },  // HEBREW LETTER SHIN WITH SHIN DOT
    { 0xFB2B, "afii57695" },  // HEBREW LETTER SHIN WITH SIN DOT
    { 0xFB35, "afii57723" },  // HEBREW LETTER VAV WITH DAGESH
    { 0xFB4B, "afii57700" },  // HEBREW LETTER VAV WITH HOLAM
    // end of stuff from glyphlist.txt
    { 0xFFFF, 0 }
};


static const struct {
    QFont::CharSet cs;
    uint mib;
} unicodevalues[] = {
    { QFont::ISO_8859_1, 4 },
#ifndef QT_NO_TEXTCODEC
    { QFont::ISO_8859_2, 5 },
    { QFont::ISO_8859_3, 6 },
    { QFont::ISO_8859_4, 7 },
    { QFont::ISO_8859_5, 8 },
    { QFont::KOI8U, 2088 },
    { QFont::KOI8R, 2084 },
    { QFont::ISO_8859_6, 82 },
    { QFont::ISO_8859_7, 10 },
    { QFont::ISO_8859_8, 85 },
    { QFont::ISO_8859_10, 13 },
    { QFont::ISO_8859_11, 2259 }, // aka tis620
    // ### don't have any MIB for -12: { QFont::ISO_8859_12, 0 },
    { QFont::ISO_8859_13, 109 },
    { QFont::ISO_8859_14, 110 },
    { QFont::ISO_8859_15, 111 },
    // makeFixedStrings() below assumes that this is last
    { QFont::ISO_8859_9, 12 }
#define unicodevalues_LAST QFont::ISO_8859_9
#else
#define unicodevalues_LAST QFont::ISO_8859_1
#endif
};




static QString *fixed_ps_header = 0;
static QIntDict<QString> * font_vectors = 0;

static void cleanup()
{
    delete fixed_ps_header;
    fixed_ps_header = 0;
    delete font_vectors;
    font_vectors = 0;
}


static QString wordwrap( const QString & s )
{
    QString result;

    int ip; // input pointer
    int oll; // output line length
    int cils, cols; // canidate input line start, -o-
    bool needws; // need to insert ws before next character
    bool havews; // have just inserted something like ws

    ip = 0;
    oll = 0;
    cils = cols = 0;
    havews = FALSE;
    needws = FALSE;
    while( ip < (int)s.length() ) {
	if ( oll > 79 && cils > 0 ) {
	    result.truncate( cols );
	    ip = cils;
	    result += '\n';
	    oll = 0;
	    cils = 0;
	    havews = TRUE;
	}
	if ( havews && oll > 0 ) {
	    cils = ip;
	    cols = result.length();
	}
	if ( isspace( s[ip] ) ) {
	    if ( !havews )
		needws = TRUE;
	    cils = ip+1;
	    cols = result.length();
	} else if ( s[ip] == '/'  || s[ip] == '{' || s[ip] == '}' ||
		    s[ip] == '[' || s[ip] == ']' ) {
	    havews = s[ip] != '/';
	    needws = FALSE;
	    cils = ip;
	    cols = result.length();
	    result += s[ip];
	    oll++;
	} else {
	    if ( needws ) {
		cols = result.length();
		cils = ip;
		result += ' ';
		oll++;
		needws = FALSE;
	    }
	    result += s[ip];
	    oll++;
	    havews = FALSE;
	}
	ip++;
    }
    return result;
}


static void makeFixedStrings()
{
    if ( fixed_ps_header )
	return;
    qAddPostRoutine( cleanup );

    fixed_ps_header = new QString;
    const char * const * headerLine = ps_header;
    while ( *headerLine ) {
	fixed_ps_header->append( QString::fromLatin1(*headerLine++) );
	fixed_ps_header->append( '\n' );
    }

    *fixed_ps_header = wordwrap( *fixed_ps_header );

    // fonts.
    font_vectors = new QIntDict<QString>( 17 );
    font_vectors->setAutoDelete( TRUE );

    int i = 0;
    int k;
    int l = 0; // unicode to glyph cursor
    QString vector;
    QString glyphname;
    QString unicodestring;
    do {
	vector.sprintf( "/FE%d [", (int)unicodevalues[i].cs );
	glyphname = "";
	l = 0;
	// the first 128 positions are the same always
	for( k=0; k<128; k++ ) {
	    while( unicodetoglyph[l].u < k )
		l++;
	    if ( unicodetoglyph[l].u == k )
		glyphname = QString::fromLatin1(unicodetoglyph[l].g);
	    else
		glyphname = QString::fromLatin1("ND");
	    vector += QString::fromLatin1(" /");
	    vector += glyphname;
	}
	// the next 128 are particular to each encoding
#ifndef QT_NO_TEXTCODEC
	QTextCodec * codec;
	codec = QTextCodec::codecForMib( (int)unicodevalues[i].mib );
	for( k=128; k<256; k++ ) {
	    int value = 0xFFFD;
	    uchar as8bit[2];
	    as8bit[0] = k;
	    as8bit[1] = '\0';
	    if ( codec ) {
		value = codec->toUnicode( (char*) as8bit, 1 )[0].unicode();
	    }
	    if ( value == 0xFFFD ) {
		glyphname = QString::fromLatin1("ND");
	    } else {
		if ( l && unicodetoglyph[l].u > value )
		    l = 0;
		while( unicodetoglyph[l].u < value )
		    l++;
		if ( unicodetoglyph[l].u == value )
		    glyphname = QString::fromLatin1(unicodetoglyph[l].g);
		else
		    glyphname = QString::fromLatin1("ND");
	    }
	    vector += QString::fromLatin1(" /");
	    vector += glyphname;
	}
#endif
	vector += QString::fromLatin1(" ] d");
	vector = wordwrap( vector );
	font_vectors->insert( (int)(unicodevalues[i].cs),
			      new QString( vector ) );
    } while ( unicodevalues[i++].cs != unicodevalues_LAST );
}


struct QPSPrinterPrivate {
    QPSPrinterPrivate( int filedes )
	: buffer( 0 ), realDevice( 0 ), fd( filedes ), savedImage( 0 ),
	  dirtypen( FALSE ), dirtybrush( FALSE ), currentFontCodec( 0 ),
	  fm( 0 ), textY( 0 )
    {
	headerFontNames.setAutoDelete( TRUE );
	pageFontNames.setAutoDelete( TRUE );
	headerEncodings.setAutoDelete( FALSE );
	pageEncodings.setAutoDelete( FALSE );
    }

    QBuffer * buffer;
    int pagesInBuffer;
    QIODevice * realDevice;
    int fd;
    QDict<QString> headerFontNames;
    QDict<QString> pageFontNames;
    QIntDict<void> headerEncodings;
    QIntDict<void> pageEncodings;
    int headerFontNumber;
    int pageFontNumber;
    QBuffer * fontBuffer;
    QTextStream fontStream;
    bool dirtyClipping;
    bool firstClipOnPage;
    QRect boundingBox;
    QImage * savedImage;
    QPen cpen;
    QBrush cbrush;
    bool dirtypen;
    bool dirtybrush;
    QTextCodec * currentFontCodec;
    QFontMetrics * fm;
    int textY;
    QFont currentUsed;
    QFont currentSet;
};



QPSPrinter::QPSPrinter( QPrinter *prt, int fd )
    : QPaintDevice( QInternal::Printer | QInternal::ExternalDevice )
{
    printer = prt;
    d = new QPSPrinterPrivate( fd );
}


QPSPrinter::~QPSPrinter()
{
    if ( d->fd >= 0 )
#if defined(Q_OS_WIN32)
	::_close( d->fd );
#else
	::close( d->fd );
#endif
    delete d;
}


static const struct {
    const char * input;
    const char * roman;
    const char * italic;
    const char * bold;
    const char * boldItalic;
    const char * light;
    const char * lightItalic;
} postscriptFontNames[] = {
    { "arial", "Arial", 0, 0, 0, 0, 0 },
    { "avantgarde", "AvantGarde-Book", 0, 0, 0, 0, 0 },
    { "charter", "CharterBT-Roman", 0, 0, 0, 0, 0 },
    { "courier", "Courier", 0, 0, 0, 0, 0 },
    { "garamond", "Garamond-Regular", 0, 0, 0, 0, 0 },
    { "gillsans", "GillSans", 0, 0, 0, 0, 0 },
    { "helvetica",
      "Helvetica", "Helvetica-Oblique",
      "Helvetica-Bold", "Helvetica-BoldOblique",
      "Helvetica", "Helvetica-Oblique" },
    { "new century schoolbook", "NewCenturySchlbk-Roman", 0, 0, 0, 0, 0 },
    { "symbol", "Symbol", "Symbol", "Symbol", "Symbol", "Symbol", "Symbol" },
    { "terminal", "Courier", 0, 0, 0, 0, 0 },
    { "times new roman", "TimesNewRoman", 0, 0, 0, 0, 0 },
    { "utopia", "Utopia-Regular", 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0 }
};


void QPSPrinter::setFont( const QFont & f )
{
    if ( f.rawMode() ) {
	QFont fnt( QString::fromLatin1("Helvetica"), 12 );
	setFont( fnt );
	return;
    }
    if ( f.pointSize() == 0 ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QPrinter: Cannot set a font with zero point size." );
#endif
	return;
    }

    if ( !fixed_ps_header )
	makeFixedStrings();

    QString family = f.family();
    QString ps;
    int	 weight = f.weight();
    bool italic = f.italic();

    family = family.lower();

    int i;

    // try to make a "good" postscript name
    ps = family.simplifyWhiteSpace();
    i = 0;
    while( (unsigned int)i < ps.length() ) {
	if ( i == 0 || ps[i-1] == ' ' ) {
	    ps[i] = ps[i].upper();
	    if ( i )
		ps.remove( i-1, 1 );
	    else
		i++;
	} else {
	    i++;
	}
    }

    // see if the table has a better name
    i = 0;
    while( postscriptFontNames[i].input &&
	   QString::fromLatin1(postscriptFontNames[i].input) != family )
	i++;
    if ( postscriptFontNames[i].roman ) {
	ps = QString::fromLatin1(postscriptFontNames[i].roman);
	int p = ps.find( '-' );
	if ( p > -1 )
	    ps.truncate( p );
    }

    // get the right modification, or build something
    if ( weight >= QFont::Bold && italic ) {
	if ( postscriptFontNames[i].boldItalic )
	    ps = QString::fromLatin1(postscriptFontNames[i].boldItalic);
	else
	    ps.append( QString::fromLatin1("-BoldItalic") );
    } else if ( weight >= QFont::Bold ) {
	if ( postscriptFontNames[i].bold )
	    ps = QString::fromLatin1(postscriptFontNames[i].bold);
	else
	    ps.append( QString::fromLatin1("-Bold") );
    } else if ( weight >= QFont::DemiBold && italic ) {
	if ( postscriptFontNames[i].italic )
	    ps = QString::fromLatin1(postscriptFontNames[i].italic);
	else
	    ps.append( QString::fromLatin1("-Italic") );
    } else if ( weight <= QFont::Light && italic ) {
	if ( postscriptFontNames[i].lightItalic )
	    ps = QString::fromLatin1(postscriptFontNames[i].lightItalic);
	else
	    ps.append( QString::fromLatin1("-LightItalic") );
    } else if ( weight <= QFont::Light ) {
	if ( postscriptFontNames[i].light )
	    ps = QString::fromLatin1(postscriptFontNames[i].light);
	else
	    ps.append( QString::fromLatin1("-Light") );
    } else if ( italic ) {
	if ( postscriptFontNames[i].italic )
	    ps = QString::fromLatin1(postscriptFontNames[i].italic);
	else
	    ps.append( QString::fromLatin1("-Italic") );
    } else {
	if ( postscriptFontNames[i].roman )
	    ps = QString::fromLatin1(postscriptFontNames[i].roman);
	else
	    ps.append( QString::fromLatin1("-Roman") );
    }

    QString key;
    int cs = (int)f.charSet();
    if ( cs == QFont::AnyCharSet ) {
	QIntDictIterator<void> it( d->headerEncodings );
	if ( it.current() )
	    cs = it.currentKey();
	else
	    cs = QFont::Latin1;
    }

    key.sprintf( "%s %d %d", ps.ascii(), f.pointSize(), cs );
    QString * tmp;
    tmp = d->headerFontNames.find( key );
    if ( !tmp && !d->buffer )
	tmp = d->pageFontNames.find( key );

    QString fontName;
    if ( tmp )
	fontName = *tmp;

    if ( fontName.isEmpty() ) {
	QString key2;
	key2.sprintf( "%s %d", ps.ascii(), cs );
	tmp = d->headerFontNames.find( key );

	QString fontEncoding;
	fontEncoding.sprintf( " FE%d", cs );
	if ( d->buffer ) {
	    if ( !d->headerEncodings.find( cs ) ) {
		QString * vector = font_vectors->find( cs );
		if ( vector ) {
		    d->fontStream << *vector << "\n";
		    d->headerEncodings.insert( cs, (void*)42 );
		} else {
		    d->fontStream << "% wanted font encoding "
				  << cs << "\n";
		}
	    }
	    if ( tmp ) {
		fontName = *tmp;
	    } else {
		fontName.sprintf( "/F%d", ++d->headerFontNumber );
		d->fontStream << fontName << fontEncoding << "/"
			      << ps << " MF\n";
		d->headerFontNames.insert( key2, new QString( fontName ) );
	    }
	    ++d->headerFontNumber;
	    d->fontStream << "/F" << d->headerFontNumber << " "
			  << f.pointSize() << fontName << " DF\n";
	    fontName.sprintf( "F%d", d->headerFontNumber );
	    d->headerFontNames.insert( key, new QString( fontName ) );
	} else {
	    if ( !d->headerEncodings.find( cs ) &&
		 !d->pageEncodings.find( cs ) ) {
		QString * vector = font_vectors->find( cs );
		if ( !vector )
		    vector = font_vectors->find( QFont::Latin1 );
		stream << *vector << "\n";
		d->pageEncodings.insert( cs, (void*)42 );
	    }
	    if ( !tmp )
		tmp = d->pageFontNames.find( key );
	    if ( tmp ) {
		fontName = *tmp;
	    } else {
		fontName.sprintf( "/F%d", ++d->pageFontNumber );
		stream << fontName << fontEncoding << "/" << ps << " MF\n";
		d->pageFontNames.insert( key2, new QString( fontName ) );
	    }
	    ++d->pageFontNumber;
	    stream << "/F" << d->pageFontNumber << " "
		   << f.pointSize() << fontName << " DF\n";
	    fontName.sprintf( "F%d", d->pageFontNumber );
	    d->pageFontNames.insert( key, new QString( fontName ) );
	}
    }
    stream << fontName << " F\n";

    ps.append( ' ' );
    ps.prepend( ' ' );
    if ( !fontsUsed.contains( ps ) )
	fontsUsed += ps;

    QTextCodec * codec = 0;
#ifndef QT_NO_TEXTCODEC
    i = 0;
    do {
	if ( unicodevalues[i].cs == f.charSet() )
	    codec = QTextCodec::codecForMib( unicodevalues[i++].mib );
    } while( codec == 0 && unicodevalues[i++].cs != unicodevalues_LAST );
#endif
    d->currentFontCodec = codec;
}


static void ps_r7( QTextStream& stream, const char * s, int l )
{
    int i = 0;
    uchar line[79];
    int col = 0;

    while( i < l ) {
	line[col++] = s[i++];
	if ( col >= 76 ) {
	    line[col++] = '\n';
	    line[col++] = '\0';
	    stream << (const char *)line;
	    col = 0;
	}
    }
    if ( col > 0 ) {
	while( (col&3) != 0 )
	    line[col++] = '%'; // use a comment as padding
	line[col++] = '\n';
	line[col++] = '\0';
	stream << (const char *)line;
    }
}


static const int quoteSize = 3; // 1-8 pixels
static const int maxQuoteLength = 4+16+32+64+128; // magic extended quote
static const int quoteReach = 10; // ... 1-1024 pixels back
static const int tableSize = 1024; // 2 ** quoteReach;

static const int hashSize = 29;

static const int None = INT_MAX;


static void emitBits( QByteArray & out, int & byte, int & bit,
		      int numBits, uint data )
{
    int b = 0;
    uint d = data;
    while( b < numBits ) {
	if ( bit == 0 )
	    out[byte] = 0;
	if ( d & 1 )
	    out[byte] = (uchar)out[byte] | ( 1 << bit );
	d = d >> 1;
	b++;
	bit++;
	if ( bit > 6 ) {
	    bit = 0;
	    byte++;
	    if ( byte == (int)out.size() )
		out.resize( byte*2 );
	}
    }
}


QByteArray compress( const QImage & image ) {
    int size = image.width()*image.height();
    int pastPixel[tableSize];
    int mostRecentPixel[hashSize];
    QRgb *pixel = new QRgb[size+1];

    int i = 0;
    if ( image.depth() == 8 ) {
	for( int y=0; y < image.height(); y++ ) {
	    uchar * s = image.scanLine( y );
	    for( int x=0; x < image.width(); x++ )
		pixel[i++] = ( image.color( s[x] ) ) & RGB_MASK;
	}
    } else {
	for( int y=0; y < image.height(); y++ ) {
	    QRgb * s = (QRgb*)(image.scanLine( y ));
	    for( int x=0; x < image.width(); x++ )
		pixel[i++] = (*s++) & RGB_MASK;
	}
    }

    pixel[size] = 0;

    /* this compression function emits blocks of data, where each
       block is an unquoted series of pixels, or a quote from earlier
       pixels. if the six-letter string "banana" were a six-pixel
       image, it would be an nquoted "ban" block followed by a 3-pixel
       quote from -2.  note that the final "a" is then copied from the
       second "a", which is copied from the first "a" in the same copy
       operation.

       the scanning for quotable blocks uses a cobol-like loop and a
       hash table: we know how many pixels we need to quote, hash the
       first and last pixel we need, and then go backwards in time
       looking for some spot where those pixels of those two colours
       occur at the right distance from each other.

       when we find a spot, we'll try a string-compare of all the
       intervening pixels. we only do a maximum of 128 both-ends
       compares or 64 full-string compares. it's more important to be
       fast than get the ultimate in compression. */

    for( i=0; i < hashSize; i++ )
	mostRecentPixel[i] = None;
    int index = 0;
    int emittedUntil = 0;
    QByteArray out( 49 );
    int outOffset = 0;
    int outBit = 0;
    /* we process pixels serially, emitting as necessary/possible. */
    while( index <= size ) {
	/* bestCandidate is the start of the best known quote area and
           bestLength its length. */
	int bestCandidate = None;
	int bestLength = 0;
	/* pick up the most recent pixels of the two colours we want,
           and mark the pixel we're lookin at as being the new most
           recent pixel in its chain. */
	i = index % tableSize;
	int h = pixel[index] % hashSize;
	/* start and end point to the stard and end of the current
           candidate are. they may or may not point to something that
           actually can be quoted. */
	int start, end;
	start = end = pastPixel[i] = mostRecentPixel[h];
	mostRecentPixel[h] = index;
	/* if our first candidate quote is unusable, or we don't need
	   to quote because we've already emitted something for this
	   pixel, just skip. */
	if ( start < index - tableSize || index >= size ||
	     emittedUntil > index)
	    start = end = None;
	/* attempts count how much work we've done trying to find
           something quotable for this pixel. */
	int attempts = 0;
	/* scan for suitable quote candidates: not too far back, and
	   if we've found one that's as big as it can possibly get,
	   we don't look for more. */
	while( start != None && end != None &&
	       bestLength < maxQuoteLength &&
	       start >= index - tableSize &&
	       end >= index - tableSize + bestLength ) {
	    /* scan backwards, looking for something good enough to
	       try a (slow) string comparison. we maintain indexes to
	       the start and the end of the quote candidate here. */
	    while( start != None && end != None &&
		   ( pixel[start] != pixel[index] ||
		     pixel[end] != pixel[index+bestLength] ) ) {
		if ( attempts++ > 128 ) {
		    start = None;
		} else if ( pixel[end] % hashSize ==
			    pixel[index+bestLength] % hashSize ) {
		    /* we move the area along the end index' chain. */
		    end = pastPixel[end%tableSize];
		    start = end - bestLength;
		} else if ( pixel[start] % hashSize ==
			    pixel[index] % hashSize ) {
		    /* ... or along the start index' chain. */
		    start = pastPixel[start%tableSize];
		    end = start + bestLength;
		} else {
#if 0
		    /* this should never happen: both the start and
		       the end pointers ran off their tracks. */
		    qDebug( "oops! %06x %06x %06x %06x %5d %5d %5d %d",
			    pixel[start], pixel[end],
			    pixel[index], pixel[index+bestLength],
			    start, end, index, bestLength );
#endif
		    /* but if it should happen, no problem. we'll just
		       say we found nothing, and the compression will
		       be a bit worse. */
		    start = None;
		}
		/* if we've moved either index too far to use the
		   quote candidate, let's just give up here. there's
		   also a guard against "start" insanity. */
		if ( start < index - tableSize || start < 0 || start >= index )
		    start = None;
		if ( end < index - tableSize + bestLength || end < bestLength )
		    end = None;
	    }
	    /* ok, now start and end point to an area of suitable
	       length whose first and last points match, or one/both
	       is/are set to None. */
	    if ( start != None && end != None ) {
		/* slow string compare... */
		int length = 0;
		while( length < maxQuoteLength &&
		       index+length < size &&
		       pixel[start+length] == pixel[index+length] )
		    length++;
		/* if we've found something that overlaps the index
		   point, maybe we can move the quote point back?  if
		   we're copying 10 pixels from 8 pixels back (an
		   overlap of 2), that'll be faster than copying from
		   4 pixels back (an overlap of 6). */
		if ( start + length > index && length > 0 ) {
		    int d = index-start;
		    int equal = TRUE;
		    while( equal && start + length > index &&
			   start > d && start-d >= index-tableSize ) {
			int i = 0;
			while( equal && i < d ) {
			    if( pixel[start+i] != pixel[start+i-d] )
				equal = FALSE;
			    i++;
			}
			if ( equal )
			    start -= d;
		    }
		}
		/* if what we have is longer than the best previous
		   candidate, we'll use this one. */
		if ( length > bestLength ) {
		    attempts = 0;
		    bestCandidate = start;
		    bestLength = length;
		    if ( length < maxQuoteLength && index + length < size )
			end = mostRecentPixel[pixel[index+length]%hashSize];
		} else {
		    /* and if it isn't, we'll try some more. but we'll
		       count each string compare extra, since they're
		       so expensive. */
		    attempts += 2;
		    if ( attempts > 128 ) {
			start = None;
		    } else if ( pastPixel[start%tableSize] + bestLength <
				pastPixel[end%tableSize] ) {
			start = pastPixel[start%tableSize];
			end = start + bestLength;
		    } else {
			end = pastPixel[end%tableSize];
			start = end - bestLength;
		    }
		}
		/* again, if we can't make use of the current quote
		   candidate, we don't try any more. */
		if ( start < index - tableSize )
		    start = None;
		if ( end < index - tableSize + bestLength )
		    end = None;
	    }
	}
	/* at this point, bestCandidate is a candidate of bestLength
	   length, or else it's None. if we have such a candidate, or
	   we're at the end, we have to emit all unquoted data. */
	if ( index == size || bestCandidate != None ) {
	    /* we need a double loop, because there's a maximum length
	       on the "unquoted data" section. */
	    while( emittedUntil < index ) {
		int l = QMIN( 8, index - emittedUntil );
		emitBits( out, outOffset, outBit,
			  1, 0 );
		emitBits( out, outOffset, outBit,
			  quoteSize, l-1 );
		while( l-- ) {
		    emitBits( out, outOffset, outBit,
			      8, qRed( pixel[emittedUntil] ) );
		    emitBits( out, outOffset, outBit,
			      8, qGreen( pixel[emittedUntil] ) );
		    emitBits( out, outOffset, outBit,
			      8, qBlue( pixel[emittedUntil] ) );
		    emittedUntil++;
		}
	    }
	}
	/* if we have some quoted data we can output, do it. */
	if ( bestCandidate != None ) {
	    emitBits( out, outOffset, outBit,
		      1, 1 );
	    if ( bestLength < 5 ) {
		emitBits( out, outOffset, outBit,
			  quoteSize, bestLength - 1 );
	    } else if ( bestLength - 4 <= 16 ) {
		emitBits( out, outOffset, outBit,
			  quoteSize, 4 );
		emitBits( out, outOffset, outBit,
			  4, bestLength - 1 - 4 );
	    } else if ( bestLength - 4 - 16 <= 32 ) {
		emitBits( out, outOffset, outBit,
			  quoteSize, 5 );
		emitBits( out, outOffset, outBit,
			  5, bestLength - 1 - 4 - 16 );
	    } else if ( bestLength - 4 - 16 - 32 <= 64 ) {
		emitBits( out, outOffset, outBit,
			  quoteSize, 6 );
		emitBits( out, outOffset, outBit,
			  6, bestLength - 1 - 4 - 16 - 32 );
	    } else /* if ( bestLength - 4 - 16 - 32 - 64 <= 128 ) */ {
		emitBits( out, outOffset, outBit,
			  quoteSize, 7 );
		emitBits( out, outOffset, outBit,
			  7, bestLength - 1 - 4 - 16 - 32 - 64 );
	    }
	    emitBits( out, outOffset, outBit,
		      quoteReach, index - bestCandidate - 1 );
	    emittedUntil += bestLength;
	}
	index++;
    }
    /* we've output all the data; time to clean up and finish off the
       last characters. */
    if ( outBit )
	outOffset++;
    out.truncate( outOffset );
    i = 0;
    /* we have to make sure the date is encoded in a stylish way :) */
    while( i < outOffset ) {
	uchar c = out[i];
	c += 42;
	if ( c > 'Z' && ( c != 't' || i == 0 || out[i-1] != 'Q' ) )
	    c += 84;
	out[i] = c;
	i++;
    }
    delete [] pixel;
    return out;
}


#undef XCOORD
#undef YCOORD
#undef WIDTH
#undef HEIGHT
#undef POINT
#undef RECT
#undef INT_ARG

#define XCOORD(x)	(float)(x)
#define YCOORD(y)	(float)(y)
#define WIDTH(w)	(float)(w)
#define HEIGHT(h)	(float)(h)

#define POINT(index)	XCOORD(p[index].point->x()) << ' ' <<		\
			YCOORD(p[index].point->y()) << ' '
#define RECT(index)	XCOORD(p[index].rect->normalize().x())  << ' ' <<     \
			YCOORD(p[index].rect->normalize().y())  << ' ' <<     \
			WIDTH (p[index].rect->normalize().width()) << ' ' <<  \
			HEIGHT(p[index].rect->normalize().height()) << ' '
#define INT_ARG(index)	p[index].ival << ' '

static char returnbuffer[13];
static const char * color( const QColor &c, QPrinter * printer )
{
    if ( c == Qt::black )
	qstrcpy( returnbuffer, "B " );
    else if ( c == Qt::white )
	qstrcpy( returnbuffer, "W " );
    else if ( c.red() == c.green() && c.red() == c.blue() )
	sprintf( returnbuffer, "%d d2 ", c.red() );
    else if ( printer->colorMode() == QPrinter::GrayScale )
	sprintf( returnbuffer, "%d d2 ",
		 qGray( c.red(), c.green(),c.blue() ) );
    else
	sprintf( returnbuffer, "%d %d %d ",
		 c.red(), c.green(), c.blue() );
    return returnbuffer;
}


static const char * psCap( Qt::PenCapStyle p )
{
    if ( p == Qt::SquareCap )
	return "2 ";
    else if ( p == Qt::RoundCap )
	return "1 ";
    return "0 ";
}


static const char * psJoin( Qt::PenJoinStyle p ) {
    if ( p == Qt::BevelJoin )
	return "2 ";
    else if ( p == Qt::RoundJoin )
	return "1 ";
    return "0 ";
}


bool QPSPrinter::cmd( int c , QPainter *paint, QPDevCmdParam *p )
{
    if ( c == PdcBegin ) {		// start painting
	d->pagesInBuffer = 0;
	d->buffer = new QBuffer();
	d->buffer->open( IO_WriteOnly );
	stream.setEncoding( QTextStream::Latin1 );
	stream.setDevice( d->buffer );
	d->fontBuffer = new QBuffer();
	d->fontBuffer->open( IO_WriteOnly );
	d->fontStream.setEncoding( QTextStream::Latin1 );
	d->fontStream.setDevice( d->fontBuffer );
	d->headerFontNumber = 0;
	pageCount           = 1;		// initialize state
	dirtyMatrix         = TRUE;
	d->dirtyClipping    = TRUE;
	dirtyNewPage        = TRUE;
	d->firstClipOnPage  = TRUE;
	d->boundingBox = QRect( 0, 0, -1, -1 );
	fontsUsed = QString::fromLatin1("");

	d->fm = new QFontMetrics( paint->fontMetrics() );

	stream << "%%Page: " << pageCount << ' ' << pageCount << endl
	       << "QI\n";
	return TRUE;
    }

    if ( c == PdcEnd ) {			// painting done
	bool pageCountAtEnd = (d->buffer == 0);
	if ( !pageCountAtEnd )
	    emitHeader( TRUE );
	stream << "QP\n"
	       << "%%Trailer\n";
	if ( pageCountAtEnd )
	    stream << "%%Pages: " << pageCount << "\n%%DocumentFonts: "
		   << fontsUsed.simplifyWhiteSpace() << '\n';
	stream.unsetDevice();
	d->realDevice->close();
	if ( d->fd >= 0 )
	    ::close( d->fd );
	d->fd = -1;
	delete d->realDevice;
	d->realDevice = 0;
	delete d->fm;
    }

    if ( c >= PdcDrawFirst && c <= PdcDrawLast ) {
	if ( !paint )
	    return FALSE; // sanity
	if ( dirtyMatrix )
	    matrixSetup( paint );
	if ( dirtyNewPage )
	    newPageSetup( paint );
	if ( d->dirtyClipping )	// Must be after matrixSetup and newPageSetup
	    clippingSetup( paint );
	if ( d->dirtypen ) {
	    // we special-case for narrow solid lines with the default
	    // cap and join styles
	    if ( d->cpen.style() == Qt::SolidLine && d->cpen.width() == 0 &&
		 d->cpen.capStyle() == Qt::FlatCap &&
		 d->cpen.joinStyle() == Qt::MiterJoin )
		stream << color( d->cpen.color(), printer ) << "P1\n";
	    else
		stream << (int)d->cpen.style() << ' ' << d->cpen.width()
		       << ' ' << color( d->cpen.color(), printer )
		       << psCap( d->cpen.capStyle() )
		       << psJoin( d->cpen.joinStyle() ) << "PE\n";
	    d->dirtypen = FALSE;
	}
	if ( d->dirtybrush ) {
	    // we special-case for nobrush and solid white, since
	    // those are the two most common brushes
	    if ( d->cbrush.style() == Qt::NoBrush )
		stream << "NB\n";
	    else if ( d->cbrush.style() == Qt::SolidPattern &&
		      d->cbrush.color() == Qt::white )
		stream << "WB\n";
	    else
		stream << (int)d->cbrush.style() << ' '
		       << color( d->cbrush.color(), printer ) << "BR\n";
	    d->dirtybrush = FALSE;
	}
    }

    switch( c ) {
    case PdcDrawPoint:
	stream << POINT(0) << "P\n";
	break;
    case PdcMoveTo:
	stream << POINT(0) << "M\n";
	break;
    case PdcLineTo:
	stream << POINT(0) << "L\n";
	break;
    case PdcDrawLine:
	if ( p[0].point->y() == p[1].point->y() )
	    stream << POINT(1) << p[0].point->x() << " HL\n";
	else if ( p[0].point->x() == p[1].point->x() )
	    stream << POINT(1) << p[0].point->y() << " VL\n";
	else
	    stream << POINT(1) << POINT(0) << "DL\n";
	break;
    case PdcDrawRect:
	stream << RECT(0) << "R\n";
	break;
    case PdcDrawRoundRect:
	stream << RECT(0) << INT_ARG(1) << INT_ARG(2) << "RR\n";
	break;
    case PdcDrawEllipse:
	stream << RECT(0) << "E\n";
	break;
    case PdcDrawArc:
	stream << RECT(0) << INT_ARG(1) << INT_ARG(2) << "A\n";
	break;
    case PdcDrawPie:
	stream << RECT(0) << INT_ARG(1) << INT_ARG(2) << "PIE\n";
	break;
    case PdcDrawChord:
	stream << RECT(0) << INT_ARG(1) << INT_ARG(2) << "CH\n";
	break;
    case PdcDrawLineSegments:
	if ( p[0].ptarr->size() > 0 ) {
	    QPointArray a = *p[0].ptarr;
	    QPoint pt;
	    stream << "NP\n";
	    for ( int i=0; i<(int)a.size(); i+=2 ) {
		pt = a.point( i );
		stream << XCOORD(pt.x()) << ' '
		       << YCOORD(pt.y()) << " MT\n";
		pt = a.point( i+1 );
		stream << XCOORD(pt.x()) << ' '
		       << YCOORD(pt.y()) << " LT\n";
	    }
	    stream << "QS\n";
	}
	break;
    case PdcDrawPolyline:
	if ( p[0].ptarr->size() > 1 ) {
	    QPointArray a = *p[0].ptarr;
	    QPoint pt = a.point( 0 );
	    stream << "NP\n"
		   << XCOORD(pt.x()) << ' ' << YCOORD(pt.y()) << " MT\n";
	    for ( int i=1; i<(int)a.size(); i++ ) {
		pt = a.point( i );
		stream << XCOORD(pt.x()) << ' '
		       << YCOORD(pt.y()) << " LT\n";
	    }
	    stream << "QS\n";
	}
	break;
    case PdcDrawPolygon:
	if ( p[0].ptarr->size() > 2 ) {
	    QPointArray a = *p[0].ptarr;
	    if ( p[1].ival )
		stream << "/WFi true d\n";
	    QPoint pt = a.point(0);
	    stream << "NP\n";
	    stream << XCOORD(pt.x()) << ' '
		   << YCOORD(pt.y()) << " MT\n";
	    for( int i=1; i<(int)a.size(); i++) {
		pt = a.point( i );
		stream << XCOORD(pt.x()) << ' '
		       << YCOORD(pt.y()) << " LT\n";
	    }
	    stream << "CP BF QS\n";
	    if ( p[1].ival )
		stream << "/WFi false d\n";
	}
	break;
    case PdcDrawCubicBezier:
	if ( p[0].ptarr->size() == 4 ) {
	    stream << "NP\n";
	    QPointArray a = *p[0].ptarr;
	    stream << XCOORD(a[0].x()) << ' '
		   << YCOORD(a[0].y()) << " MT ";
	    for ( int i=1; i<4; i++ ) {
		stream << XCOORD(a[i].x()) << ' '
		       << YCOORD(a[i].y()) << ' ';
	    }
	    stream << "BZ\n";
	}
	break;
    case PdcDrawText2:
	if ( p[1].str->length() > 0 ) {
	    QCString tmpC;
#ifndef QT_NO_TEXTCODEC
	    if ( d->currentFontCodec )
		tmpC = d->currentFontCodec->fromUnicode( *p[1].str );
	    else // #### should use 16-bit stuff here
#endif
		tmpC=p[1].str->local8Bit();
	    uint spaces = 0;
	    while( spaces < tmpC.length() && tmpC[(int)spaces] == ' ' )
		spaces++;
	    if ( spaces )
		tmpC = tmpC.mid( spaces, tmpC.length() );
	    while ( tmpC.length() > 0 && isspace(tmpC[(int)tmpC.length()-1]) )
		tmpC.truncate( tmpC.length()-1 );
	    char *tmp = new char[tmpC.length()*2 + 2];
#if defined(QT_CHECK_NULL)
	    Q_CHECK_PTR( tmp );
#endif
	    const char* from = (const char*)tmpC;

	    // first scan to see whether it'll look good with just ()
	    int parenlevel = 0;
	    bool parensOK = TRUE;
	    while( from && *from ) {
		if ( *from == '(' )
		    parenlevel++;
		else if ( *from == ')' )
		    parenlevel--;
		if ( parenlevel < 0 )
		    parensOK = FALSE;
		from++;
	    }
	    if ( parenlevel != 0 )
		parensOK = FALSE;

	    // then scan again, outputting the stuff
	    from = (const char *)tmpC;
	    char * to = tmp;
	    while ( from && *from ) {
		if ( *from == '\\' ||
		     ( !parensOK && ( *from == '(' || *from == ')' ) ) )
		    *to++ = '\\'; // escape special chars if necessary
		*to++ = *from++;
	    }
	    *to = '\0';

	    if ( qstrlen( tmp ) > 0 ) {
		if ( d->currentSet != d->currentUsed ) {
		    d->currentUsed = d->currentSet;
		    setFont( d->currentSet );
		}
		int x = p[0].point->x();
		if ( spaces > 0 )
		    x += spaces * d->fm->width( ' ' );
		int y = p[0].point->y();
		if ( y != d->textY || d->textY == 0 )
		    stream << y << " Y";
		d->textY = y;
		int w = d->fm->width( tmpC );
		stream << "(" << tmp << ")" << w << " " << x;
		if ( paint->font().underline() )
		    stream << ' ' << y + d->fm->underlinePos() << " Tl";
		if ( paint->font().strikeOut() )
		    stream << ' ' << y + d->fm->strikeOutPos() << " Tl";
		stream << " T\n";
	    }
	    if ( tmp )
		delete [] tmp;
	}
	break;
    case PdcDrawText2Formatted:
	return FALSE;			// uses QPainter instead
    case PdcDrawPixmap: {
	if ( p[1].pixmap->isNull() )
	    break;
	QPoint pnt = *(p[0].point);
	QImage img;
	img = *(p[1].pixmap);
	drawImage( paint, pnt, img );
	break;
    }
    case PdcDrawImage: {
	if ( p[1].image->isNull() )
	    break;
	QPoint pnt = *(p[0].point);
	QImage img = *(p[1].image);
	drawImage( paint, pnt, img );
	break;
    }
    case PdcSetBkColor:
	stream << color( *(p[0].color), printer ) << "BC\n";
	break;
    case PdcSetBkMode:
	if ( p[0].ival == Qt::TransparentMode )
	    stream << "/OMo false d\n";
	else
	    stream << "/OMo true d\n";
	break;
    case PdcSetROP:
#if defined(QT_CHECK_RANGE)
	if ( p[0].ival != Qt::CopyROP )
	    qWarning( "QPrinter: Raster operation setting not supported" );
#endif
	break;
    case PdcSetBrushOrigin:
	break;
    case PdcSetFont:
	d->currentSet = *(p[0].font);
	// turn these off - they confuse the 'avoid font change' logic
	d->currentSet.setUnderline( FALSE );
	d->currentSet.setStrikeOut( FALSE );
	break;
    case PdcSetPen:
	if ( d->cpen != *(p[0].pen) ) {
	    d->dirtypen = TRUE;
	    d->cpen = *(p[0].pen);
	}
	break;
    case PdcSetBrush:
	if ( p[0].brush->style() == Qt::CustomPattern ) {
#if defined(QT_CHECK_RANGE)
	    qWarning( "QPrinter: Pixmap brush not supported" );
#endif
	    return FALSE;
	}
	if ( d->cbrush != *(p[0].brush) ) {
	    d->dirtybrush = TRUE;
	    d->cbrush = *(p[0].brush);
	}
	break;
    case PdcSetTabStops:
    case PdcSetTabArray:
	return FALSE;
    case PdcSetUnit:
	break;
    case PdcSetVXform:
    case PdcSetWindow:
    case PdcSetViewport:
    case PdcSetWXform:
    case PdcSetWMatrix:
    case PdcRestoreWMatrix:
	dirtyMatrix = TRUE;
	break;
    case PdcSetClip:
	d->dirtyClipping = TRUE;
	break;
    case PdcSetClipRegion:
	d->dirtyClipping = TRUE;
	break;
    case NewPage:
	pageCount++;
	stream << "QP\n%%Page: "
	       << pageCount << ' ' << pageCount
	       << "\nQI\n";
	dirtyNewPage       = TRUE;
	d->dirtyClipping   = TRUE;
	d->firstClipOnPage = TRUE;
	delete d->savedImage;
	d->savedImage = 0;
	break;
    case AbortPrinting:
	break;
    default:
	break;
    }
    return TRUE;
}


// ### deal with ColorMode GrayScale here.
void QPSPrinter::drawImage( QPainter *paint, const QPoint &pnt,
			    const QImage &img )
{
    int width  = img.width();
    int height = img.height();

    if ( img.isNull() )
	return;

    if ( width * height > 21830 ) { // 65535/3, tolerance for broken printers
	int images, subheight;
	images = ( width * height + 21829 ) / 21830;
	subheight = ( height + images-1 ) / images;
	while ( subheight * width > 21830 ) {
	    images++;
	    subheight = ( height + images-1 ) / images;
	}
	int y = 0;
	while( y < height ) {
	    drawImage( paint, QPoint( pnt.x(), pnt.y()+y ),
		       img.copy( 0, y, width, QMIN( subheight, height-y ) ) );
	    y += subheight;
	}
    } else {
	if ( pnt.x() || pnt.y() )
	    stream << pnt.x() << " " << pnt.y() << " TR\n";
	stream << "/sl " << width*3*height << " string d\n";
	stream << "sl rC\n";
	QByteArray out;
	if ( img.depth() < 8 )
	    out = compress( img.convertDepth( 8 ) );
	else if ( img.depth() > 8 && img.depth() < 24 )
	    out = compress( img.convertDepth( 24 ) );
	else
	    out = compress( img );
	ps_r7( stream, out, out.size() );
	stream << "pop\n";
	stream << width << ' ' << height << " 8[1 0 0 1 0 0]{sl}QCI\n";
	if ( pnt.x() || pnt.y() )
	    stream << -pnt.x() << " " << -pnt.y() << " TR\n";
    }
}


void QPSPrinter::matrixSetup( QPainter *paint )
{
#ifndef QT_NO_TRANSFORMATIONS
    QWMatrix tmp;
    if ( paint->hasViewXForm() ) {
	QRect viewport = paint->viewport();
	QRect window   = paint->window();
	tmp.translate( viewport.x(), viewport.y() );
	tmp.scale( 1.0 * viewport.width()  / window.width(),
		   1.0 * viewport.height() / window.height() );
	tmp.translate( -window.x(), -window.y() );
    }
    if ( paint->hasWorldXForm() ) {
	tmp = paint->worldMatrix() * tmp;
    }
    stream << "["
	   << tmp.m11() << ' ' << tmp.m12() << ' '
	   << tmp.m21() << ' ' << tmp.m22() << ' '
	   << tmp.dx()	<< ' ' << tmp.dy()
	   << "]ST\n";
#else
    QPoint p(0,0);
    p = paint->xForm(p);
    stream << "["
	   << 0 << ' ' << 0 << ' '
	   << 0 << ' ' << 0 << ' '
	   << p.x()    << ' ' << p.y()
	   << "]ST\n";
#endif
    dirtyMatrix = FALSE;
}

void QPSPrinter::orientationSetup()
{
    if ( printer->orientation() == QPrinter::Landscape )
	stream << "QLS\n";
}



static QString stripHeader( const QString & header, const char * data,
			    int len, bool useFonts )
{
    // first pass: find and mark all identifiers
    QDict<int> ids( 257 );
    ids.setAutoDelete( TRUE );

    int i=0;
    int size = header.length();
    int * used = new int[size];
    for( i=0; i<size; i++ )
	used[i] = 0;
    i=0;
    used[0] = 0x10000000;
    while( i < size ) {
	while( i < size && header[i] != '/' )
	    i++;
	if ( header[i] == '/' && isalpha( header[i+1].latin1() ) ) {
	    i++;
	    int j=i;
	    while( j < size && isalnum( header[j].latin1() ) )
		j++;
	    char id[10];
	    strncpy( id, header.ascii() + i, j-i );
	    id[j-i] = '\0';
	    while( j < size && isspace( header[j].latin1() ) )
		j++;
	    if ( header[j] == '{' ) {
		j++;
		int k, l;
		l = 1;
		while( l && j < size ) {
		    while( j < size && !isalpha( header[j].latin1() ) ) {
			if ( header[j] == '{' )
			    l++;
			else if ( header[j] == '}' )
			    l--;
			j++;
		    }
		    if ( l ) {
			k = j;
			while( j < size && isalnum( header[j].latin1() ) )
			    j++;
			if( j - k < 10 ) {
			    char id2[10];
			    strncpy( id2, header.ascii() + k, j-k );
			    id2[j-k] = '\0';
			    int * offs = ids.find( id2 );
			    if ( offs )
				used[k] = *offs;
			}
		    }
		}
	    } else if ( header[j] == '[' ) {
		// handle array defintions
		int l=0;
		do {
		    if ( header[j] == '[' )
			l++;
		    else if ( header[j] == ']' )
			l--;
		    j++;
		} while( j < size && l );
	    } else if ( header[j] == 'D' && header[j+1] == '0' &&
			!isalnum( header[j+2].latin1() ) ) {
		ids.insert( id, new int(i-1) );
		used[i-1] = 0x20000000;
		int *tmp = ids.find( "D0" );
		if ( tmp )
		    used[j] = *tmp;
		j = j+2;
	    } else if ( header[j] == 'E' && header[j+1] == 'D' &&
			!isalnum( header[j+2].latin1() ) ) {
		ids.insert( id, new int(i-1) );
		used[i-1] = 0x20000000;
		int *tmp = ids.find( "ED" );
		if ( tmp )
		    used[j] = *tmp;
		j = j+2;
	    } else {
		// handle other variables.
		while( j < size && !isspace( header[j].latin1() ) )
		    j++;
		while( j < size && isspace( header[j].latin1() ) )
		    j++;
		// hack: then skip dict
		if ( !qstrncmp( header.ascii()+j, "dict", 4 ) &&
		     !isalnum( header[j+4].latin1() ) )
		    j += 4;
		// worse: string
		if ( !qstrncmp( header.ascii()+j, "string", 6 ) &&
		     !isalnum( header[j+6].latin1() ) )
		    j += 6;
	    }
	    while( j < size && isspace( header[j].latin1() ) )
		j++;
	    if ( header[j] == 'D' && !isalnum( header[j+1].latin1() ) ) {
		ids.insert( id, new int(i-1) );
		used[i-1] = 0x20000000;
		i = j+1;
	    } else if ( !qstrncmp( header.ascii()+j, "d", 1 ) &&
			!isalnum( header[j+1].latin1() ) ) {
		ids.insert( id, new int(i-1) );
		used[i-1] = 0x20000000;
		i = j+1;
	    } else {
		i = j;
	    }
	} else {
	    i++;
	}
    }

    // second pass: mark the identifiers used in the document

    // we know CM, QI and QP are used
    int * tmp;
    if ( (tmp = ids.take("CM")) != 0 ) {
	used[*tmp] = 0x10000000;
	delete tmp;
    }
    if ( (tmp = ids.take("QI")) != 0 ) {
	used[*tmp] = 0x10000000;
	delete tmp;
    }
    if ( (tmp = ids.take("QP")) != 0 ) {
	used[*tmp] = 0x10000000;
	delete tmp;
    }

    // we speed this up a little by hand-hacking DF/MF/ND support
    if ( useFonts ) {
	if ( (tmp = ids.take("DF")) != 0 ) {
	    used[*tmp] = 0x10000000;
	    delete tmp;
	}
	if ( (tmp = ids.take("MF")) != 0 ) {
	    used[*tmp] = 0x10000000;
	    delete tmp;
	}
	if ( (tmp = ids.take("ND")) != 0 ) {
	    used[*tmp] = 0x10000000;
	    delete tmp;
	}
    }

    char id[10];
    i = 0;
    while( i < len ) {
	while( i < len && !isalpha( data[i] ) ) {
	    if ( data[i++] == '(' ) { // it's a string, skip it
		int level = 1;
		do {
		    if ( data[i] == '(' )
			level++;
		    else if ( data[i] == ')' )
			level--;
		    else if ( data[i] == '\\' )
			i++;
		    i++;
		} while( i < len && level > 0 );
	    }
	}
	if ( i < len && isalpha( data[i] ) ) {
	    int j=0;
	    while( isalnum( data[i+j] ) )
		j++;
	    if( j < 9 ) {
		// all identifiers we care about are <9 chars long
		strncpy( id, data + i, j );
		id[j] = '\0';
		int * offs = ids.take( id );
		if ( offs )
		    used[*offs] = 0x10000000;
		delete offs;
	    }
	    i += j;
	}
    }

    // third pass: mark the identifiers used in the used parts of the header

    i = size-1;
    while( i > 0 ) {
	// find beginning of function
	int j = i;
	while( j && used[j] < 0x10000000 )
	    j--;
	if ( used[j] == 0x10000000 ) {
	    // this function is used.
	    while( i > j ) {
		if ( used[i] )
		    used[used[i]] = 0x10000000;
		i--;
	    }
	}
	i = j-1;
    }

    // fourth pass: make the new header

    i=0;
    QString result;
    while( i < size ) {
	bool c = used[i] == 0x10000000;
	if ( c && result.length() )
	    result += '\n';
	do {
	    if ( c )
		result += header[i];
	    i++;
	} while ( i < size && used[i] < 0x10000000 );
    }
    delete[] used;
    result = wordwrap( result );
    return result;
}


void QPSPrinter::emitHeader( bool finished )
{
    QString title = printer->docName();
    QString creator = printer->creator();
    if ( !creator )				// default creator
	creator = QString::fromLatin1("Qt " QT_VERSION_STR);
    d->realDevice = new QFile();
    (void)((QFile *)d->realDevice)->open( IO_WriteOnly, d->fd );
    stream.setDevice( d->realDevice );
    stream << "%!PS-Adobe-1.0";
    if ( finished && pageCount == 1 && printer->numCopies() == 1 &&
	 printer->fullPage() ) {
	QPaintDeviceMetrics m( printer );
	if ( !d->boundingBox.isValid() )
	    d->boundingBox.setRect( 0, 0, m.width(), m.height() );
	if ( printer->orientation() == QPrinter::Landscape )
	    stream << " EPSF-3.0\n%%BoundingBox: "
		   << m.height() - d->boundingBox.bottom() << " " // llx
		   << m.width() - d->boundingBox.right() << " " // lly
		   << m.height() - d->boundingBox.top() << " " // urx
		   << m.width() - d->boundingBox.left();// ury
	else
	    stream << " EPSF-3.0\n%%BoundingBox: "
		   << d->boundingBox.left() << " "
		   << m.height() - d->boundingBox.bottom() - 1 << " "
		   << d->boundingBox.right() + 1 << " "
		   << m.height() - d->boundingBox.top();
    }
    stream << "\n%%Creator: " << creator;
    if ( !!title )
	stream << "\n%%Title: " << title;
    stream << "\n%%CreationDate: " << QDateTime::currentDateTime().toString();
    stream << "\n%%Orientation: ";
    if ( printer->orientation() == QPrinter::Landscape )
	stream << "Landscape";
    else
	stream << "Portrait";
    if ( finished )
	stream << "\n%%Pages: " << pageCount << "\n%%DocumentFonts: "
	       << fontsUsed.simplifyWhiteSpace();
    else
	stream << "\n%%Pages: (atend)"
	       << "\n%%DocumentFonts: (atend)";
    stream << "\n%%EndComments\n\n";

    if ( printer->numCopies() > 1 )
	stream << "/#copies " << printer->numCopies() << " def\n";

    if ( !fixed_ps_header )
	makeFixedStrings();

    const char * prologLicense = "% Prolog copyright 1994-2000 Trolltech. "
				 "You may copy this prolog in any way\n"
				 "% that is directly related to this "
				 "document. For other use of this prolog,\n"
				 "% see your licensing agreement for Qt.\n";

    if ( finished ) {
	QString r( stripHeader( *fixed_ps_header,
				d->buffer->buffer().data(),
				d->buffer->buffer().size(),
				d->fontBuffer->buffer().size() > 0 ) );
	stream << prologLicense << r << "\n";
    } else {
	stream << prologLicense << *fixed_ps_header << "\n";
    }

    if ( !printer->fullPage() )
	stream << "% lazy-margin hack: QPrinter::setFullPage(FALSE)\n"
	       << printer->margins().width() << " "
	       << printer->margins().height() << " translate\n";
    if ( printer->orientation() == QPrinter::Portrait ) {
	QPaintDeviceMetrics m( printer );
	stream << "% " << m.widthMM() << "*" << m.heightMM()
	       << "mm (portrait)\n0 " << m.height()
	       << " translate 1 -1 scale/defM matrix CM d\n";
    } else {
	QPaintDeviceMetrics m( printer );
	stream << "% " << m.heightMM() << "*" << m.widthMM()
	       << " mm (landscape)\n90 rotate 1 -1 scale/defM matrix CM d\n";
    }

    if ( d->fontBuffer->buffer().size() ) {
	if ( pageCount == 1 || finished )
	    stream << "% Fonts and encodings used\n";
	else
	    stream << "% Fonts and encodings used on pages 1-"
		   << pageCount << "\n";
	stream.writeRawBytes( d->fontBuffer->buffer().data(),
			      d->fontBuffer->buffer().size() );
    }
    stream << "%%EndProlog\n";
    stream.writeRawBytes( d->buffer->buffer().data(),
			  d->buffer->buffer().size() );

    delete d->buffer;
    d->buffer = 0;
    d->fontStream.unsetDevice();
    delete d->fontBuffer;
    d->fontBuffer = 0;
}


void QPSPrinter::newPageSetup( QPainter *paint )
{
    if ( d->buffer &&
	 ( d->pagesInBuffer++ > 32 ||
	   ( d->pagesInBuffer > 4 && d->buffer->size() > 262144 ) ) )
	emitHeader( FALSE );

    if ( !d->buffer ) {
	d->pageEncodings.clear();
	d->pageFontNames.clear();
    }

    resetDrawingTools( paint );
    dirtyNewPage      = FALSE;
    d->pageFontNumber = d->headerFontNumber;
}


/* Called whenever a restore has been done. Currently done at the top of a
  new page and whenever clipping is turned off. */
void QPSPrinter::resetDrawingTools( QPainter *paint )
{
    QPDevCmdParam param[1];
    QPen   defaultPen;			// default drawing tools
    QBrush defaultBrush;

    param[0].color = &paint->backgroundColor();
    if ( *param[0].color != Qt::white )
	cmd( PdcSetBkColor, paint, param );

    param[0].ival = paint->backgroundMode();
    if (param[0].ival != Qt::TransparentMode )
	cmd( PdcSetBkMode, paint, param );

    d->currentUsed = d->currentSet;
    setFont( d->currentSet );

    param[0].brush = &paint->brush();
    if (*param[0].brush != defaultBrush )
	cmd( PdcSetBrush, paint, param);

    d->dirtypen = TRUE;
    d->dirtybrush = TRUE;

    if ( paint->hasViewXForm() || paint->hasWorldXForm() )
	matrixSetup( paint );
}


static void putRect( QTextStream &stream, const QRect &r )
{
    stream << r.x() << " "
	   << r.y() << " "
	   << r.width() << " "
	   << r.height() << " ";
}


void QPSPrinter::setClippingOff( QPainter *paint )
{
	stream << "CLO\n";		// clipping off, includes a restore
	resetDrawingTools( paint );     // so drawing tools must be reset
}


void QPSPrinter::clippingSetup( QPainter *paint )
{
    if ( paint->hasClipping() ) {
	if ( !d->firstClipOnPage )
	    setClippingOff( paint );
	const QRegion rgn = paint->clipRegion();
	QArray<QRect> rects = rgn.rects();
	int i;
	stream<< "CLSTART\n";		// start clipping
	for( i = 0 ; i < (int)rects.size() ; i++ ) {
	    putRect( stream, rects[i] );
	    stream << "ACR\n";		// add clip rect
	    if ( pageCount == 1 )
		d->boundingBox = d->boundingBox.unite( rects[i] );
	}
	stream << "CLEND\n";		// end clipping
	d->firstClipOnPage = FALSE;
    } else {
	if ( !d->firstClipOnPage )	// no need to turn off if first on page
	    setClippingOff( paint );
	// if we're painting without clipping, the bounding box must
	// be everything.  NOTE: this assumes that this function is
	// only ever called when something is to be painted.
	QPaintDeviceMetrics m( printer );
	if ( !d->boundingBox.isValid() )
	    d->boundingBox.setRect( 0, 0, m.width(), m.height() );
    }
    d->dirtyClipping = FALSE;
}


#endif

