/**********************************************************************
** $Id: //depot/qt/main/src/kernel/qpsprinter.cpp#38 $
**
** Implementation of QPSPrinter class
**
** Created : 941003
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#include "qpsprn.h"
#include "qpainter.h"
#include "qpaintdc.h"
#include "qimage.h"
#include "qdatetm.h"

#include "qstring.h"
#include "qdict.h"
#include "qregexp.h"

#include "qfile.h"
#include "qbuffer.h"

RCSTAG("$Id: //depot/qt/main/src/kernel/qpsprinter.cpp#38 $");

#if !defined(QT_HEADER_PS)
     // produced from qpshdr.txt
static char *ps_header =
"%\n"
"% $Id: //depot/qt/main/src/kernel/qpsprinter.cpp#38 $\n"
"%\n"
"% Postscript routines for QPSPrinter class\n"
"%\n"
"% Author  : Eirik Eng\n"
"% Created : 940920\n"
"%\n"
"% Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.\n"
"%\n"
"\n"
"/D  {bind def} bind def\n"
"/ED {exch def} D\n"
"/LT {lineto} D\n"
"/MT {moveto} D\n"
"/S  {stroke} D\n"
"/SW {setlinewidth} D\n"
"/CP {closepath} D\n"
"/RL {rlineto} D\n"
"/NP {newpath} D\n"
"/CM {currentmatrix} D\n"
"/SM {setmatrix} D\n"
"/TR {translate} D\n"
"/SRGB {setrgbcolor} D\n"
"/SC {aload pop SRGB} D\n"
"/GS {gsave} D\n"
"/GR {grestore} D\n"
"\n"
"[ % iso 8859-1\n"
" /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef\n"
" /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef\n"
" /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef\n"
" /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef\n"
" /space /exclam /quotedbl /numbersign /dollar /percent /ampersand /quoteright\n"
" /parenleft /parenright /asterisk /plus /comma /hyphen /period /slash\n"
" /zero /one /two /three /four /five /six /seven\n"
" /eight /nine /colon /semicolon /less /equal /greater /question\n"
" /at /A /B /C /D /E /F /G\n"
" /H /I /J /K /L /M /N /O\n"
" /P /Q /R /S /T /U /V /W\n"
" /X /Y /Z /bracketleft /backslash /bracketright /asciicircum /underscore\n"
" /quoteleft /a /b /c /d /e /f /g\n"
" /h /i /j /k /l /m /n /o\n"
" /p /q /r /s /t /u /v /w\n"
" /x /y /z /braceleft /bar /braceright /asciitilde /.notdef\n"
" /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef\n"
" /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef\n"
" /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef\n"
" /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef\n"
" /space /exclamdown /cent /sterling /currency /yen /brokenbar /section\n"
" % switch to four per line starting with 0250\n"
" /dieresis /copyright /ordfeminine /guillemotleft\n"
" /logicalnot /hyphen /registered /macron\n"
" /degree /plusminus /twosuperior /threesuperior\n"
" /acute /mu /paragraph /periodcentered\n"
" /cedilla /onesuperior /ordmasculine /guillemotright\n"
" /onequarter /onehalf /threequarters /questiondown\n"
" % 0300 here\n"
" /Agrave /Aacute /Acircumflex /Atilde\n"
" /Adieresis /Aring /AE /Ccedilla\n"
" /Egrave /Eacute /Ecircumflex /Edieresis\n"
" /Igrave /Iacute /Icircumflex /Idieresis\n"
" /Eth /Ntilde /Ograve /Oacute\n"
" /Ocircumflex /Otilde /Odieresis /multiply\n"
" /Oslash /Ugrave /Uacute /Ucircumflex\n"
" /Udieresis /Yacute /Thorn /germandbls\n"
" % 0340\n"
" /agrave /aacute /acircumflex /atilde\n"
" /adieresis /aring /ae /ccedilla\n"
" /egrave /eacute /ecircumflex /edieresis\n"
" /igrave /iacute /icircumflex /idieresis\n"
" /eth /ntilde /ograve /oacute\n"
" /ocircumflex /otilde /odieresis /divide\n"
" /oslash /ugrave /uacute /ucircumflex\n"
" /udieresis /yacute /thorn /ydieresis\n"
"] /iso88591 exch def\n"
"\n"
"/BSt 0 def			% brush style\n"
"/LWi 1 def			% line width\n"
"/PSt 1 def			% pen style\n"
"/Cx  0 def			% current x position\n"
"/Cy  0 def			% current y position\n"
"/WFi false def			% winding fill\n"
"/OMo false def			% opaque mode (not transparent)\n"
"\n"
"/BCol  [ 1 1 1 ] def		% brush color\n"
"/PCol  [ 0 0 0 ] def		% pen color\n"
"/BkCol [ 1 1 1 ] def		% background color\n"
"\n"
"/nS 0 def			% number of saved painter states\n"
"\n"
"\n"
"/QS {				% stroke command\n"
"    PSt 0 ne			% != NO_PEN\n"
"    { LWi SW			% set line width\n"
"      GS\n"
"      PCol SC			% set pen color\n"
"      true GPS 0 setdash S	% draw line pattern\n"
"      OMo PSt 1 ne and		% opaque mode and not solid line?\n"
"      { GR BkCol SC\n"
"	false GPS dup 0 get setdash S	% fill in opaque pattern\n"
"      }\n"
"      { GR } ifelse\n"
"    } if\n"
"} D\n"
"\n"
"/QF {				% fill command\n"
"    GS\n"
"    BSt 2 ge BSt 8 le and	% dense pattern?\n"
"    { BDArr BSt 2 sub get setgray fill } if\n"
"    BSt 9 ge BSt 14 le and	% fill pattern?\n"
"    { BF } if\n"
"    BSt 1 eq			% solid brush?\n"
"    { BCol SC WFi { fill } { eofill } ifelse } if\n"
"    GR\n"
"} D\n"
"\n"
"/PF {				% polygon fill command\n"
"    GS\n"
"    BSt 2 ge BSt 8 le and	% dense pattern?\n"
"    { BDArr BSt 2 sub get setgray WFi { fill } { eofill } ifelse } if\n"
"    BSt 9 ge BSt 14 le and	% fill pattern?\n"
"    { BF } if\n"
"    BSt 1 eq			% solid brush?\n"
"    { BCol SC WFi { fill } { eofill } ifelse } if\n"
"    GR\n"
"} D\n"
"\n"
"/BDArr[				% Brush dense patterns:\n"
"    0.94 0.88 0.63 0.50 0.37 0.12 0.6\n"
"] def\n"
"\n"
"/ArcDict 6 dict def\n"
"ArcDict begin\n"
"    /tmp matrix def\n"
"end\n"
"\n"
"/ARC {				% Generic ARC function [ X Y W H ang1 ang2 ]\n"
"    ArcDict begin\n"
"    /ang2 ED /ang1 ED /h ED /w ED /y ED /x ED\n"
"    tmp CM pop\n"
"    x w 2 div add y h 2 div add TR\n"
"    1 h w div neg scale\n"
"    ang2 0 ge\n"
"    {0 0 w 2 div ang1 ang1 ang2 add arc }\n"
"    {0 0 w 2 div ang1 ang1 ang2 add arcn} ifelse\n"
"    tmp SM\n"
"    end\n"
"} D\n"
"\n"
"\n"
"/QI {\n"
"    /savedContext save def\n"
"    clippath pathbbox\n"
"    3 index /PageX ED\n"
"    0 index /PageY ED\n"
"    3 2 roll\n"
"    exch\n"
"    sub neg /PageH ED\n"
"    sub neg /PageW ED\n"
"\n"
"    PageX PageY TR\n"
"    1 -1 scale\n"
"    /defM matrix CM def		% default transformation matrix\n"
"    /Cx  0 def			% reset current x position\n"
"    /Cy  0 def			% reset current y position\n"
"    255 255 255 BC\n"
"    /OMo false def\n"
"    1 0 0 0 0 PE\n"
"    0 0 0 0 B\n"
"} D\n"
"\n"
"/QP {				% show page\n"
"    savedContext restore\n"
"    showpage\n"
"} D\n"
"\n"
"\n"
"/P {				% PDC_DRAWPOINT [x y]\n"
"    NP\n"
"    MT\n"
"    0.5 0.5 rmoveto\n"
"    0  -1 RL\n"
"    -1	0 RL\n"
"    0	1 RL\n"
"    CP\n"
"    PCol SC\n"
"    fill\n"
"} D\n"
"\n"
"/M {				% PDC_MOVETO [x y]\n"
"    /Cy ED /Cx ED\n"
"} D\n"
"\n"
"/L {				% PDC_LINETO [x y]\n"
"    NP\n"
"    Cx Cy MT\n"
"    /Cy ED /Cx ED\n"
"    Cx Cy LT\n"
"    QS\n"
"} D\n"
"\n"
"/DL {				% PDC_DRAWLINE [x0 y0 x1 y1]\n"
"    4 2 roll\n"
"    NP\n"
"    MT\n"
"    LT\n"
"    QS\n"
"} D\n"
"\n"
"/RDict 4 dict def\n"
"/R {				% PDC_DRAWRECT [x y w h]\n"
"    RDict begin\n"
"    /h ED /w ED /y ED /x ED\n"
"    NP\n"
"    x y MT\n"
"    0 h RL\n"
"    w 0 RL\n"
"    0 h neg RL\n"
"    CP\n"
"    QF\n"
"    QS\n"
"    end\n"
"} D\n"
"\n"
"/RRDict 6 dict def\n"
"/RR {				% PDC_DRAWROUNDRECT [x y w h xr yr]\n"
"    RRDict begin\n"
"    /yr ED /xr ED /h ED /w ED /y ED /x ED\n"
"    xr 0 le yr 0 le or\n"
"    {x y w h R}	     % Do rectangle if one of rounding values is less than 0.\n"
"    {xr 100 ge yr 100 ge or\n"
"	{x y w h E}  % Do ellipse if both rounding values are larger than 100\n"
"	{\n"
"	 /rx xr w mul 200 div def\n"
"	 /ry yr h mul 200 div def\n"
"	 /rx2 rx 2 mul def\n"
"	 /ry2 ry 2 mul def\n"
"	 NP\n"
"	 x rx add y MT\n"
"	 x w add rx2 sub y		 rx2 ry2 90  -90 ARC\n"
"	 x w add rx2 sub y h add ry2 sub rx2 ry2 0   -90 ARC\n"
"	 x		 y h add ry2 sub rx2 ry2 270 -90 ARC\n"
"	 x		 y		 rx2 ry2 180 -90 ARC\n"
"	 CP\n"
"	 QF\n"
"	 QS\n"
"	} ifelse\n"
"    } ifelse\n"
"    end\n"
"} D\n"
"\n"
"\n"
"/EDict 5 dict def\n"
"EDict begin\n"
"/tmp matrix def\n"
"end\n"
"/E {				% PDC_DRAWELLIPSE [x y w h]\n"
"    EDict begin\n"
"    /h ED /w ED /y ED /x ED\n"
"    tmp CM pop\n"
"    x w 2 div add y h 2 div add translate\n"
"    1 h w div scale\n"
"    NP\n"
"    0 0 w 2 div 0 360 arc\n"
"    tmp SM\n"
"    QF\n"
"    QS\n"
"    end\n"
"} D\n"
"\n"
"\n"
"/A {				% PDC_DRAWARC [x y w h ang1 ang2]\n"
"    16 div exch 16 div exch\n"
"    NP\n"
"    ARC\n"
"    QS\n"
"} D\n"
"\n"
"\n"
"/PieDict 6 dict def\n"
"/PIE {				% PDC_DRAWPIE [x y w h ang1 ang2]\n"
"    PieDict begin\n"
"    /ang2 ED /ang1 ED /h ED /w ED /y ED /x ED\n"
"    NP\n"
"    x w 2 div add y h 2 div add MT\n"
"    x y w h ang1 16 div ang2 16 div ARC\n"
"    CP\n"
"    QF\n"
"    QS\n"
"    end\n"
"} D\n"
"\n"
"/CH {				% PDC_DRAWCHORD [x y w h ang1 ang2]\n"
"    16 div exch 16 div exch\n"
"    NP\n"
"    ARC\n"
"    CP\n"
"    QF\n"
"    QS\n"
"} D\n"
"\n"
"\n"
"/BZ {				% PDC_DRAWQUADBEZIER [3 points]\n"
"    curveto\n"
"    QS\n"
"} D\n"
"\n"
"\n"
"/CRGB {				% Compute RGB [R G B] => R/255 G/255 B/255\n"
"    255 div 3 1 roll\n"
"    255 div 3 1 roll\n"
"    255 div 3 1 roll\n"
"} D\n"
"\n"
"\n"
"/SV {				% Save painter state\n"
"    BSt LWi PSt Cx Cy WFi OMo BCol PCol BkCol\n"
"    /nS nS 1 add def\n"
"    GS\n"
"} D\n"
"\n"
"/RS {				% Restore painter state\n"
"    nS 0 gt\n"
"    { GR\n"
"      /BkCol ED /PCol ED /BCol ED /OMo ED /WFi ED\n"
"      /Cy ED /Cx ED /PSt ED /LWi ED /BSt ED\n"
"      /nS nS 1 sub def\n"
"    } if\n"
"\n"
"} D\n"
"\n"
"/BC {				% PDC_SETBKCOLOR [R G B]\n"
"    CRGB\n"
"    BkCol astore pop\n"
"} D\n"
"\n"
"/B {				% PDC_SETBRUSH [style R G B]\n"
"    CRGB\n"
"    BCol astore pop\n"
"    /BSt ED\n"
"} D\n"
"\n"
"/PE {				% PDC_SETPEN [style width R G B]\n"
"    CRGB\n"
"    PCol astore pop\n"
"    /LWi ED\n"
"    /PSt ED\n"
"    LWi 0 eq { 0.3 /LWi ED } if\n"
"} D\n"
"\n"
"/ST {				% SET TRANSFORM [matrix]\n"
"    defM setmatrix\n"
"    concat\n"
"} D\n"
"\n"
"\n"
"% use MF like this make /F114 a 12 point font, preferably Univers, but\n"
"% Helvetica if Univers is not available and Courier if all else fails:\n"
"%\n"
"% /114 [ 12 0 0 -12 0 0 ] [ /Univers /Helvetica ] MF\n"
"\n"
"/F /Courier def\n"
"/MF {				% make font [ newname matrix fontlist ]\n"
"  /F /Courier def\n"
"  {\n"
"    dup FontDirectory exch known\n"
"    {\n"
"      /F exch def\n"
"      exit\n"
"    } {\n"
"      pop\n"
"    } ifelse\n"
"  } forall\n"
"  F findfont dup length dict begin {\n"
"    1 index /FID ne {\n"
"      def\n"
"    } {\n"
"      pop pop\n"
"    } ifelse\n"
"  } forall\n"
"  /Encoding iso88591 def\n"
"  currentdict\n"
"  end\n"
"  2 index exch definefont\n"
"  exch makefont\n"
"  definefont pop\n"
"} D\n"
"\n"
"\n"
"/SF {				% PDC_SETFONT [ fontname ]\n"
"  findfont setfont\n"
"} D\n"
"\n"
"\n"
"% isn't this important enough to try to avoid the SC?\n"
"\n"
"/T {				% PDC_DRAWTEXT [x y string]\n"
"    3 1 roll\n"
"    MT				% !!!! Uff\n"
"    PCol SC			% set pen/text color\n"
"    show\n"
"} D\n"
"\n"
"\n"
"/BFDict 2 dict def\n"
"/BF {				% brush fill\n"
"    BSt 9 ge BSt 14 le and	% valid brush pattern?\n"
"    {\n"
"     BFDict begin\n"
"     GS\n"
"     WFi { clip } { eoclip } ifelse\n"
"     defM SM\n"
"     pathbbox			% left upper right lower\n"
"     3 index 3 index translate\n"
"     4 2 roll			% right lower left upper\n"
"     3 2 roll			% right left upper lower\n"
"     exch			% left right lower upper\n"
"     sub /h ED\n"
"     sub /w ED\n"
"     OMo {\n"
"	  NP\n"
"	  0 0 MT\n"
"	  0 h RL\n"
"	  w 0 RL\n"
"	  0 h neg RL\n"
"	  CP\n"
"	  BkCol SC\n"
"	  fill\n"
"     } if\n"
"     BCol SC\n"
"     0.3 SW\n"
"     BSt 9 eq BSt 11 eq or	% horiz or cross pattern\n"
"     { 0 4 h			% draw horiz lines !!! alignment\n"
"       { NP dup 0 exch MT w exch LT S } for\n"
"     } if\n"
"     BSt 10 eq BSt 11 eq or	% vert or cross pattern\n"
"     { 0 4 w			% draw vert lines !!! alignment\n"
"       { NP dup 0 MT h LT S } for\n"
"     } if\n"
"     BSt 12 eq BSt 14 eq or	% F-diag or diag cross\n"
"     { w h gt\n"
"       { 0 6 w h add\n"
"	{ NP dup h MT h sub 0 LT S } for }\n"
"       { 0 6 w h add\n"
"	 { NP dup w exch MT w add 0 exch LT S } for } ifelse\n"
"     } if\n"
"     BSt 13 eq BSt 14 eq or	% B-diag or diag cross\n"
"     { w h gt\n"
"       { 0 6 w h add\n"
"	 { NP dup 0 MT h sub h LT S } for }\n"
"       { 0 6 w h add\n"
"	 { NP dup 0 exch MT w add w exch LT S } for } ifelse\n"
"     } if\n"
"     GR\n"
"     end\n"
"    } if\n"
"} D\n"
"\n"
"/LArr[					% Pen styles:\n"
"    []		     []			%   solid line\n"
"    [ 10 3 ]	     [ 3 10 ]		%   dash line\n"
"    [ 3 3 ]	     [ 3 3 ]		%   dot line\n"
"    [ 5 3 3 3 ]	     [ 3 5 3 3 ]	%   dash dot line\n"
"    [ 5 3 3 3 3 3 ]  [ 3 5 3 3 3 3 ]	%   dash dot dot line\n"
"] def\n"
"\n"
"%\n"
"% Returns the line pattern (from pen style PSt).\n"
"%\n"
"% Argument:\n"
"%   bool pattern\n"
"%	true : draw pattern\n"
"%	false: fill pattern\n"
"%\n"
"\n"
"/GPS {\n"
"  PSt 1 ge PSt 5 le and			% valid pen pattern?\n"
"    { { LArr PSt 1 sub 2 mul get }	% draw pattern\n"
"      { LArr PSt 2 mul 1 sub get } ifelse   % opaque pattern\n"
"    }\n"
"    { [] } ifelse			% out of range => solid line\n"
"} D\n"
"\n"
"%%EndProlog";
#endif


QPSPrinter::QPSPrinter( QPrinter *prt )
    : QPaintDevice( PDT_PRINTER | PDF_EXTDEV )
{
    printer = prt;
    device = 0;
}

// some hacky variables that may be deleted at any time.  used to save
// on wear and tear in the printer's font loading

static int fontNameNumber = 0;
static QDict<QString> * fontNames = 0;

QPSPrinter::~QPSPrinter()
{
    delete fontNames;
    fontNames = 0;
    fontNameNumber = 0;
}


//
// Sets a new font for PostScript
//

static void ps_setFont( QTextStream *s, const QFont *f, QString *fonts )
{
    if ( f->rawMode() ) {
	QFont fnt( "Helvetica", 12 );
	ps_setFont( s, &fnt, fonts );
	return;
    }
    if ( f->pointSize() == 0 ) {
#if defined(CHECK_RANGE)
	warning( "QPrinter: Cannot set a font with zero point size." );
#endif
	return;
    }

    QString family = f->family();
    QString ps;
    int	 weight = f->weight();
    bool italic = f->italic();
    
    family = family.lower();
    if ( family == "courier" ) {
	ps = "/Courier ";
    } else if ( family == "helvetica" ) {
	ps = "/Helvetica ";
    } else if ( family == "times" ) {
	ps = "/Times ";
    } else if ( family == "charter" ) {
	ps = "/CharterBT /Times ";
    } else if ( family == "palatino" ) {
	ps = "/Palatino /Garamond /AGaramond /Times ";
    } else if ( family == "garamond" ) {
	ps = "/AGaramond /Garamond /Palatino /Times ";
    } else if ( family == "baskerville" ) {
	ps = "/Baskerville /Palatino /Times ";
    } else if ( family == "new century schoolbook" ) {
	ps = "/NewCenturySchlbk /Garamond /AGaramond /Times ";
    } else if ( family == "gillsans" ) {
	ps = "/GillSans /Univers /Helvetica ";
    } else if ( family == "univers" ) {
	ps = "/Univers /GillSans /Helvetica ";
    } else if ( family == "lucida" ) {
	ps = "/Lucida /AvantGarde /Helvetica ";
    } else if ( family == "lucidabright" ) {
	// ### don't know the name of lucida bright as Type 1 font
	ps = "/Lucida /Palatino /Times ";
    } else if ( family == "lucidatypewriter" ) {
	// ### don't know the name of lucida typewriter as Type 1 font
	ps = "/Courier ";
    } else if ( family == "utopia" ) {
	ps = "/Utopia /Garamond /AGaramond /Times ";
    } else if ( family == "terminal" ) {
	ps = "/Courier "; // more?
    } else if ( family == "symbol" ) {
	ps = "/Symbol ";
    } else {
	ps = "/Courier ";
    }

    // next, modify these sh***y irregular names
    if ( weight >= QFont::Bold && italic ) {
	ps.replace( QRegExp( "/Times " ), "/Times-BoldItalic " );
	ps.replace( QRegExp( "/Charter " ), "/CharterBT-BoldItalic " );
	ps.replace( QRegExp( "/Palatino " ), "/Palatino-BoldItalic " );
	ps.replace( QRegExp( "/Garamond " ), "/Garamond-BoldItalic " );
	ps.replace( QRegExp( "/AGaramond " ), "/Garamond-BoldItalic " );
	ps.replace( QRegExp( "/Palatino " ), "/Palatino-BoldItalic " );
	ps.replace( QRegExp( "/Baskerville " ), "/Baskerville-BoldItalic " );
	ps.replace( QRegExp( "/NewCenturySchlbk " ),
		    "/NewCenturySchlbk-BoldItalic " );
	ps.replace( QRegExp( "/GillSans " ), "/GillSans-BoldItalic " );
	ps.replace( QRegExp( "/Univers " ), "/Univers-BoldItalic " );
	ps.replace( QRegExp( "/Helvetica " ), "/Helvetica-BoldOBlique " );
	ps.replace( QRegExp( "/Lucida " ), "/Lucida-BoldItalic " );
	ps.replace( QRegExp( "/Utopia " ), "/Utopia-BoldItalic " );
    } else if ( weight >= QFont::Bold ) {
	ps.replace( QRegExp( "/Times " ), "/Times-Bold " );
	ps.replace( QRegExp( "/CharterBT " ), "/CharterBT-Bold " );
	ps.replace( QRegExp( "/Palatino " ), "/Palatino-Bold " );
	ps.replace( QRegExp( "/Garamond " ), "/Garamond-Bold " );
	ps.replace( QRegExp( "/AGaramond " ), "/AGaramond-Bold " );
	ps.replace( QRegExp( "/Palatino " ), "/Palatino-Bold " );
	ps.replace( QRegExp( "/Baskerville " ), "/Baskerville-Bold " );
	ps.replace( QRegExp( "/NewCenturySchlbk " ),
		    "/NewCenturySchlbk-Bold " );
	ps.replace( QRegExp( "/GillSans " ), "/GillSans-Bold " );
	ps.replace( QRegExp( "/Univers " ), "/Univers-Bold " );
	ps.replace( QRegExp( "/Helvetica " ), "/Helvetica-Bold " );
	ps.replace( QRegExp( "/Lucida " ), "/Lucida-Bold " );
	ps.replace( QRegExp( "/Utopia " ), "/Utopia-Bold " );
    } else if ( weight >= QFont::DemiBold && italic ) {
	ps.replace( QRegExp( "/AGaramond " ), "/AGaramond-SemiBoldItalic " );
	ps.replace( QRegExp( "/AvantGarde " ), "/AvantGarde-DemiOblique " );
    } else if ( weight >= QFont::DemiBold ) {
	ps.replace( QRegExp( "/AGaramond " ), "/AGaramond-SemiBold " );
	ps.replace( QRegExp( "/AvantGarde " ), "/AvantGarde-Demi " );
    } else if ( italic ) {
	ps.replace( QRegExp( "/Times " ), "/Times-Italic " );
	ps.replace( QRegExp( "/CharterBT " ), "/CharterBT-Italic " );
	ps.replace( QRegExp( "/Palatino " ), "/Palatino-Italic " );
	ps.replace( QRegExp( "/Garamond " ), "/Garamond-Italic " );
	ps.replace( QRegExp( "/AGaramond " ), "/AGaramond-Italic " );
	ps.replace( QRegExp( "/Palatino " ), "/Palatino-Italic " );
	ps.replace( QRegExp( "/Baskerville " ),
		    "/Baskerville-Normal-Italic " );
	ps.replace( QRegExp( "/NewCenturySchlbk " ),
		    "/NewCenturySchlbk-Italic " );
	ps.replace( QRegExp( "/GillSans " ), "/GillSans-Italic " );
	ps.replace( QRegExp( "/Univers " ), "/Univers-Italic " );
	ps.replace( QRegExp( "/Helvetica " ), "/Helvetica-OBlique " );
	ps.replace( QRegExp( "/Lucida " ), "/Lucida-Italic " );
	ps.replace( QRegExp( "/AvantGarde " ), "/AvantGarde-BookOblique " );
	ps.replace( QRegExp( "/Utopia " ), "/Utopia-Italic " );
    } else if ( weight <= QFont::Light && italic ) {
	ps.replace( QRegExp( "/Garamond " ), "/Garamond-LightItalic " );
	ps.replace( QRegExp( "/GillSans " ), "/GillSans-LightItalic " );
	ps.replace( QRegExp( "/Univers " ), "/Univers-LightItalic " );
    } else if ( weight <= QFont::Light ) {
	ps.replace( QRegExp( "/Garamond " ), "/Garamond-Light " );
	ps.replace( QRegExp( "/GillSans " ), "/GillSans-Light " );
	ps.replace( QRegExp( "/Univers " ), "/Univers-Light " );
    }

    ps.replace( QRegExp( "/Times " ), "/Times-Roman " );
    ps.replace( QRegExp( "/CharterBT " ), "/CharterBT-Roman " );
    ps.replace( QRegExp( "/Palatino " ), "/Palatino-Roman " );
    ps.replace( QRegExp( "/Garamond " ), "/Garamond-Regular " );
    ps.replace( QRegExp( "/Garamond " ), "/Garamond-Regular " );
    ps.replace( QRegExp( "/AGaramond " ), "/AGaramond-Regular " );
    ps.replace( QRegExp( "/Palatino " ), "/Palatino-Regular " );
    ps.replace( QRegExp( "/Baskerville " ), "/Baskerville-Normal " );
    ps.replace( QRegExp( "/NewCenturySchlbk " ),
		"/NewCenturySchlbk-Roman " );
    //ps.replace( QRegExp( "/GillSans " ), "/GillSans " );
    //ps.replace( QRegExp( "/Univers " ), "/Univers " );
    //ps.replace( QRegExp( "/Helvetica " ), "/Helvetica " );
    //ps.replace( QRegExp( "/Lucida " ), "/Lucida " );
    ps.replace( QRegExp( "/AvantGarde " ), "/AvantGarde-Book " );
    ps.replace( QRegExp( "/Utopia " ), "/Utopia-Regular " );
    
    QString key;
    key.sprintf( "%p %s %d", s, ps.data(), f->pointSize() );
    QString fontName;
    if ( fontNames ) {
	QString * tmp = fontNames->find( key );
	if ( tmp )
	fontName = *tmp;
    } else {
	fontNames = new QDict<QString>( 31 );
	fontNames->setAutoDelete( TRUE );
    }

    if ( fontName.isEmpty() ) {
	QString fontMatrix;
	fontMatrix.sprintf( " [ %d 0 0 -%d 0 0 ] [ ",
			    f->pointSize(), f->pointSize() );
	fontName.sprintf( "/F%d", ++fontNameNumber );
	*s << fontName << fontMatrix << ps << "] MF\n";
	fontNames->insert( key, &(fontName.copy()) );
    }
    *s << fontName << " SF\n";

    // change "/Palatino-Roman /Times-Roman " to "Times-Roman "
    ps.replace( QRegExp( "^.*/" ), "" );
    if ( !fonts->contains(ps) )
	*fonts += ps;
}


static void hexOut( QTextStream &stream, int i )
{
    if ( i < 0x10 )
	stream << '0';
    stream << i;
}

static void ps_dumpTransparentBitmapData( QTextStream &stream, QImage &img )
{
    stream.setf( QTextStream::hex, QTextStream::basefield );

    int width  = img.width();
    int height = img.height();
    int numBytes = (width + 7)/8;
    uchar *scanLine;
    int x,y;
    int count = -1;
    for( y = 0 ; y < height ; y++ ) {
	scanLine = img.scanLine(y);
	for( x = 0 ; x < numBytes ; x++ ) {
	    hexOut( stream, scanLine[x] );
	    if ( !(count++ % 66) )
		stream << '\n';
	}
    }
    if ( --count % 66 )
	stream << '\n';

    stream.setf( QTextStream::dec, QTextStream::basefield );
}

static void ps_dumpPixmapData( QTextStream &stream, QImage img,
			       QColor fgCol, QColor bgCol )
{
    stream.setf( QTextStream::hex, QTextStream::basefield );

    if ( img.depth() == 1 ) {
	img = img.convertDepth( 8 );
	if ( img.color(0) == 0 ) {			// black
	    img.setColor( 0, fgCol.rgb() );
	    img.setColor( 1, bgCol.rgb() );
	} else {
	    img.setColor( 0, bgCol.rgb() );
	    img.setColor( 1, fgCol.rgb() );
	}
    }

    int width  = img.width();
    int height = img.height();
    int pixWidth = img.depth() == 8 ? 1 : 4;
    uchar *scanLine;
    uint cval;
    int x,y;
    int count = -1;
    for( y = 0 ; y < height ; y++ ) {
	scanLine = img.scanLine(y);
	for( x = 0 ; x < width ; x++ ) {
	    if ( pixWidth == 1 ) {
		cval = img.color( scanLine[x] );
	    } else {
		cval = ((QRgb*) scanLine)[x];
	    }
	    hexOut( stream, qRed(cval) );
	    hexOut( stream, qGreen(cval) );
	    hexOut( stream, qBlue(cval) );
	}
	if ( !(count++ % 11) )
	    stream << '\n';
    }
    if ( --count % 11 )
	stream << '\n';

    stream.setf( QTextStream::dec, QTextStream::basefield );
}

#undef XCOORD
#undef YCOORD
#undef WIDTH
#undef HEIGHT
#undef POINT
#undef RECT
#undef INT_ARG
#undef COLOR

#define XCOORD(x)	(float)(x)
#define YCOORD(y)	(float)(y)
#define WIDTH(w)	(float)(w)
#define HEIGHT(h)	(float)(h)

#define POINT(index)	XCOORD(p[index].point->x()) << ' ' <<		\
			YCOORD(p[index].point->y()) << ' '
#define RECT(index)	XCOORD(p[index].rect->x())  << ' ' <<		\
			YCOORD(p[index].rect->y())  << ' ' <<		\
			WIDTH (p[index].rect->width())	<< ' ' <<	\
			HEIGHT(p[index].rect->height()) << ' '
#define INT_ARG(index)	p[index].ival << ' '
#define COLOR(x)	(x).red()   << ' ' <<	\
			(x).green() << ' ' <<	\
			(x).blue()  << ' '


bool QPSPrinter::cmd( int c , QPainter *paint, QPDevCmdParam *p )
{
    if ( c == PDC_SETDEV ) {			// preset io device
	device = p->device;
	epsf = TRUE;
    } else if ( c == PDC_BEGIN ) {		// start painting
	if (epsf) {
	    // ... ##### handle things differently
	}
	pageCount    = 1;			// initialize state
	dirtyMatrix  = TRUE;
	dirtyNewPage = FALSE;			// setup done by QPainter
	                                        // for the first page.
	fontsUsed    = "";
	const char *title   = printer->docName();
	const char *creator = printer->creator();
	if ( !title )				// default document name
	    title = "Unknown";
	if ( !creator )				// default creator
	    creator = "Qt";
	stream.setDevice( device );
	stream << "%!PS-Adobe-1.0\n";		// write document header
	stream << "%%Creator: " << creator << '\n';
	stream << "%%Title: "	<< title   << '\n';
	stream << "%%CreationDate:" << QDateTime::currentDateTime().toString()
	  << '\n';
	stream << "%%Pages: (atend)\n";
	stream << "%%DocumentFonts: (atend)\n";
	stream << "%%EndComments\n\n";
	if ( printer->numCopies() > 1 )
	    stream << "/#copies " << printer->numCopies() << " def\n";

#if defined(QT_HEADER_PS)
	QFile f( "/usr/lib/qpshdr.txt" );	// read predefined PS header
	if ( !f.open(IO_ReadOnly|IO_Raw) )
	    fatal( "Cannot open /usr/lib/qpshdr.txt" );
	QByteArray a(f.size());
	f.readBlock( a.data(), f.size() );
	f.close();
	stream.writeRawBytes( a.data(), a.size() );
#else
	stream << ps_header;
#endif
	stream << "\n%%Page: " << pageCount << ' ' << pageCount << endl;
	stream << "QI\n";
	orientationSetup();
	return TRUE;
    }

    if ( c == PDC_END ) {			// painting done
	stream << "QP\n";
	stream << "%%Trailer\n";
	stream << "%%Pages: " << pageCount << '\n';
	stream << "%%DocumentFonts: " << fontsUsed << '\n';
	if (!epsf) // is it our device to close?
	    device->close();
	stream.unsetDevice();
    }

    if ( c >= PDC_DRAW_FIRST && c <= PDC_DRAW_LAST ) {
	if ( dirtyMatrix )
	    matrixSetup( paint );
	if ( dirtyNewPage )
	    newPageSetup( paint );
    }

    switch( c ) {
	case PDC_DRAWPOINT:
	    stream << POINT(0) << "P\n";
	    break;
	case PDC_MOVETO:
	    stream << POINT(0) << "M\n";
	    break;
	case PDC_LINETO:
	    stream << POINT(0) << "L\n";
	    break;
	case PDC_DRAWLINE:
	    stream << POINT(0) << POINT(1) << "DL\n";
	    break;
	case PDC_DRAWRECT:
	    stream << RECT(0) << "R\n";
	    break;
	case PDC_DRAWROUNDRECT:
	    stream << RECT(0) << INT_ARG(1) << INT_ARG(2) << "RR\n";
	    break;
	case PDC_DRAWELLIPSE:
	    stream << RECT(0) << "E\n";
	    break;
	case PDC_DRAWARC:
	    stream << RECT(0) << INT_ARG(1) << INT_ARG(2) << "A\n";
	    break;
	case PDC_DRAWPIE:
	    stream << RECT(0) << INT_ARG(1) << INT_ARG(2) << "PIE\n";
	    break;
	case PDC_DRAWCHORD:
	    stream << RECT(0) << INT_ARG(1) << INT_ARG(2) << "CH\n";
	    break;
	case PDC_DRAWLINESEGS:
	    if ( p[0].ptarr->size() > 0 ) {
		QPointArray a = *p[0].ptarr;
		QPoint pt;
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
	case PDC_DRAWPOLYLINE:
	    if ( p[0].ptarr->size() > 1 ) {
		QPointArray a = *p[0].ptarr;
		QPoint pt = a.point( 0 );
		stream << XCOORD(pt.x()) << ' ' << YCOORD(pt.y()) << " MT\n";
		for ( int i=1; i<(int)a.size(); i++ ) {
		    pt = a.point( i );
		    stream << XCOORD(pt.x()) << ' '
			   << YCOORD(pt.y()) << " LT\n";
		}
		stream << "QS\n";
	    }
	    break;
	case PDC_DRAWPOLYGON:
	    if ( p[0].ptarr->size() > 2 ) {
		QPointArray a = *p[0].ptarr;
		if ( p[1].ival )
		    stream << "/WFi true def\n";
		QPoint pt = a.point(0);
		stream << "NP\n";
		stream << XCOORD(pt.x()) << ' '
		       << YCOORD(pt.y()) << " MT\n";
		for( int i=1; i<(int)a.size(); i++) {
		    pt = a.point( i );
		    stream << XCOORD(pt.x()) << ' '
			   << YCOORD(pt.y()) << " LT\n";
		}
		stream << "CP PF QS\n";
		if ( p[1].ival )
		    stream << "/WFi false def\n";
	    }
	    break;
	case PDC_DRAWQUADBEZIER:
	    if ( p[0].ptarr->size() == 4 ) {
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
	case PDC_DRAWTEXT:
	    if ( p[1].str && strlen( p[1].str ) ) {
		char * tmp = new char[ strlen( p[1].str ) * 2 + 2 ];
#if defined(CHECK_NULL)
		CHECK_PTR( tmp );
#endif
		const char * from = p[1].str;
		char * to = tmp;
		while ( *from ) {
		    if ( *from == '\\' || *from == '(' || *from == ')' )
			*to++ = '\\';		// escape special chars
		    *to++ = *from++;
		}
		*to = '\0';
		stream << POINT(0) << "(" << tmp << ") T\n";
		delete [] tmp;
	    }
	    break;
	case PDC_DRAWTEXTFRMT:;
	    return FALSE;			// uses Qt instead
	case PDC_DRAWPIXMAP: {
	    if ( p[1].pixmap->isNull() )
		break;
	    QPoint pnt = *(p[0].point);
	    stream << pnt.x() << " " << pnt.y() << " TR\n";
	    QImage img;
	    img = *(p[1].pixmap);
	    bool mask  = FALSE;
	    int width  = img.width();
	    int height = img.height();

	    QColor fgCol = paint->pen().color();
	    QColor bgCol = paint->backgroundColor();
	    if ( mask )
		stream << COLOR(fgCol) << "CRGB SRGB\n";
	    stream << "/sl " << (mask ? (width + 7)/8 : width*3)
		   << " string def\n";
	    stream << width << " " << height;
	    if ( !mask )
		stream << " 8 ";
	    stream << "[1 0 0 1 0 0] { currentfile sl readhexstring pop }\n";
	    if ( mask ) {
		stream << "imagemask\n";
//		QColor fgCol = paint->pen().color();
		ps_dumpTransparentBitmapData( stream, img );
	    } else {
		stream << "false 3 colorimage\n";
		ps_dumpPixmapData( stream, img, fgCol, bgCol );
	    }
	    stream << -pnt.x() << " " << -pnt.y() << " TR\n";
	    break;
	}
	case PDC_SAVE:
	    stream << "SV\n";
	    break;
	case PDC_RESTORE:
	    stream << "RS\n";
	    break;
	case PDC_SETBKCOLOR:
	    stream << COLOR(*(p[0].color)) << "BC\n";
	    break;
	case PDC_SETBKMODE:
	    stream << "/OMo ";
	    if ( p[0].ival == TransparentMode )
		stream << "false";
	    else
		stream << "true";
	    stream << " def\n";
	    break;
	case PDC_SETROP:
#if defined(DEBUG)
	    if ( p[0].ival != CopyROP )
		debug( "QPrinter: Raster operation setting not supported" );
#endif
	    break;
	case PDC_SETBRUSHORIGIN:
	    break;
	case PDC_SETFONT:
	    ps_setFont( &stream, p[0].font, &fontsUsed );
	    break;
	case PDC_SETPEN:
	    stream << (int)p[0].pen->style() << ' ' << p[0].pen->width()
		   << ' ' << COLOR(p[0].pen->color()) << "PE\n";
	    break;
	case PDC_SETBRUSH:
	    if ( p[0].brush->style() == CustomPattern ) {
#if defined(DEBUG)
		warning( "QPrinter: Pixmap brush not supported" );
#endif
		return FALSE;
	    }
	    stream << (int)p[0].brush->style()	 << ' '
		   << COLOR(p[0].brush->color()) << "B\n";
	    break;
	case PDC_SETTABSTOPS:
	case PDC_SETTABARRAY:
	    return FALSE;
	case PDC_SETUNIT:
	    break;
	case PDC_SETVXFORM:
	case PDC_SETWINDOW:
	case PDC_SETVIEWPORT:
	case PDC_SETWXFORM:
	case PDC_SETWMATRIX:
	    dirtyMatrix = TRUE;
	    break;
	case PDC_SETCLIP:
#if defined(DEBUG)
	    debug( "QPrinter: Clipping not supported" );
#endif
	    break;
	case PDC_SETCLIPRGN:
#if defined(DEBUG)
	    debug( "QPrinter: Clipping not supported" );
#endif
	    break;
	case PDC_PRT_NEWPAGE:
	    stream << "QP\n";
	    pageCount++;
	    stream << "\n%%Page: " << pageCount << ' ' << pageCount << endl;
	    stream << "QI\n";
	    dirtyNewPage = TRUE;
	    orientationSetup();
	    break;
	case PDC_PRT_ABORT:
	    break;
	default:
	    break;
    }
    return TRUE;
}

void QPSPrinter::matrixSetup( QPainter *paint )
{
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
    stream << "[ "
	   << tmp.m11() << ' ' << tmp.m12() << ' '
	   << tmp.m21() << ' ' << tmp.m22() << ' '
	   << tmp.dx()	<< ' ' << tmp.dy()  << " ] ST\n";
    dirtyMatrix = FALSE;
}

void QPSPrinter::orientationSetup()
{
    if ( printer->orientation() == QPrinter::Landscape ) {
	stream << "PageW 0 TR 90 rotate\n";
	stream << "/defM matrix CM def\n";
    }

}

void QPSPrinter::newPageSetup( QPainter *paint )
{
    QPDevCmdParam param[1];
    QPen   defaultPen;			// default drawing tools
    QBrush defaultBrush;

    param[0].color = &paint->backgroundColor();
    if ( *param[0].color != white )
	cmd( PDC_SETBKCOLOR, paint, param );

    param[0].ival = paint->backgroundMode();
    if (param[0].ival != TransparentMode )
	cmd( PDC_SETBKMODE, paint, param );

    param[0].font = &paint->font();
    cmd( PDC_SETFONT, paint, param );

    param[0].pen = &paint->pen();
    if (*param[0].pen != defaultPen )
	cmd( PDC_SETPEN, paint,param );

    param[0].brush = &paint->brush();
    if (*param[0].brush != defaultBrush )
	cmd( PDC_SETBRUSH, paint, param);

    if ( paint->hasViewXForm() || paint->hasWorldXForm() )
	matrixSetup( paint );

    dirtyNewPage = FALSE;

    delete fontNames;
    fontNames = 0;
    fontNameNumber = 0;
}


