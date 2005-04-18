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

#include "qplatformdefs.h"

#include <private/qprintengine_ps_p.h>
#include <private/qpainter_p.h>
#include <private/qfontengine_p.h>
#include <private/qpaintengine_p.h>

// <X11/Xlib.h> redefines Status -> int
#if defined(Status)
# undef Status
#endif

#ifndef QT_NO_PRINTER

#undef Q_PRINTER_USE_TYPE42

#include "qprinter.h"
#include "qpainter.h"
#include "qapplication.h"
#include "qpixmap.h"
#include "qimage.h"
#include "qdatetime.h"
#include "qstring.h"
#include "qbytearray.h"
#include "qhash.h"
#include "qbuffer.h"
#include "qfile.h"
#include "qtextcodec.h"
#include "qsettings.h"
#include "qmap.h"
#include "qfontdatabase.h"
#include "qregexp.h"
#include "qbitmap.h"
#include "qregion.h"
#include <private/qunicodetables_p.h>
#include <private/qpainterpath_p.h>
#include <qdebug.h>

#if defined(Q_OS_WIN32)
#include <io.h>
#ifdef Q_PRINTER_USE_TYPE42
#include <stdlib.h>
#endif
#else
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#endif

#if defined (Q_WS_X11) || defined (Q_WS_QWS)
#include <qtextlayout.h>
#endif

#ifdef Q_WS_X11
#include <qx11info_x11.h>
#include <private/qt_x11_p.h>
#endif

static bool qt_gen_epsf = false;

void qt_generate_epsf(bool b)
{
    qt_gen_epsf = b;
}

static const char *const ps_header =
"/d/def load def/D{bind d}bind d/d2{dup dup}D/ED{exch d}D/D0{0 ED}D/LT\n"
"{lineto}D/MT{moveto}D/S{stroke}D/F{setfont}D/SW{setlinewidth}D/CP{closepath}\n"
"D/RL{rlineto}D/NP{newpath}D/CM{currentmatrix}D/SM{setmatrix}D/TR{translate}D\n"
"/SD{setdash}D/SC{aload pop setrgbcolor}D/CR{currentfile read pop}D/i{index}D\n"
"/bs{bitshift}D/scs{setcolorspace}D/DB{dict dup begin}D/DE{end d}D/ie{ifelse}\n"
"D/BSt 0 d/LWi 1 d/PSt 1 d/WFi false d/OMo false d/BCol[1 1 1]d/PCol[0 0 0]d\n"
"/BkCol[1 1 1]d/BDArr[0.94 0.88 0.63 0.50 0.37 0.12 0.06]d/defM matrix d\n"
"/level3{/languagelevel where{pop languagelevel 3 ge}{false}ie}D/GPS{PSt 1 ge\n"
"PSt 5 le and{{LArr PSt 1 sub 2 mul get}{LArr PSt 2 mul 1 sub get}ie}{[]}ie}D\n"
"/QS{PSt 0 ne{gsave LWi SW true GPS 0 SD S OMo PSt 1 ne and{BkCol SC false\n"
"GPS dup 0 get SD S}if grestore}if}D/sl D0/QCIgray D0/QCIcolor D0/QCIindex D0\n"
"/QCI{/colorimage where{pop false 3 colorimage}{exec/QCIcolor ED/QCIgray\n"
"QCIcolor length 3 idiv string d 0 1 QCIcolor length 3 idiv 1 sub{/QCIindex\n"
"ED/x QCIindex 3 mul d QCIgray QCIindex QCIcolor x get 0.30 mul QCIcolor x 1\n"
"add get 0.59 mul QCIcolor x 2 add get 0.11 mul add add cvi put}for QCIgray\n"
"image}ie}D/di{gsave TR 1 i 1 eq{false eq{pop true 3 1 roll 4 i 4 i false 4 i\n"
"4 i imagemask BkCol SC imagemask}{pop false 3 1 roll imagemask}ie}{dup false\n"
"ne{level3}{false}ie{/ma ED 8 eq{/dc[0 1]d/DeviceGray}{/dc[0 1 0 1 0 1]d\n"
"/DeviceRGB}ie scs/im ED/mt ED/h ED/w ED/id <</ImageType 1/Width w/Height h\n"
"/ImageMatrix mt/DataSource im/BitsPerComponent 8/Decode dc >> d/md <<\n"
"/ImageType 1/Width w/Height h/ImageMatrix mt/DataSource ma/BitsPerComponent\n"
"1/Decode[0 1]>> d <</ImageType 3/DataDict id/MaskDict md/InterleaveType 3 >>\n"
"image}{pop 8 4 1 roll 8 eq{image}{QCI}ie}ie}ie grestore}d/SetLinGrad{level3{\n"
"/c2 ED/c1 ED/y2 ED/x2 ED/y1 ED/x1 ED/LinGrad <</PatternType 2/Shading <<\n"
"/ShadingType 2/ColorSpace[/DeviceRGB]/Coords[x1 y1 x2 y2]/Extend[true true]\n"
"/Function <</FunctionType 2/Domain[0 1]/C0 c1/C1 c2/N 1 >> >> >> matrix\n"
"makepattern d}{pop}ie/BSt 15 d}D/BF{gsave BSt 1 eq{BCol SC WFi{fill}{eofill}\n"
"ie}if BSt 2 ge BSt 8 le and{BDArr BSt 2 sub get/sc ED BCol{1. exch sub sc\n"
"mul 1. exch sub}forall 3 array astore SC WFi{fill}{eofill}ie}if BSt 9 ge BSt\n"
"14 le and{WFi{clip}{eoclip}ie defM SM pathbbox 3 i 3 i TR 4 2 roll 3 2 roll\n"
"exch sub/h ED sub/w ED OMo{NP 0 0 MT 0 h RL w 0 RL 0 h neg RL CP BkCol SC\n"
"fill}if BCol SC 0.3 SW NP BSt 9 eq BSt 11 eq or{0 4 h{dup 0 exch MT w exch\n"
"LT}for}if BSt 10 eq BSt 11 eq or{0 4 w{dup 0 MT h LT}for}if BSt 12 eq BSt 14\n"
"eq or{w h gt{0 6 w h add{dup 0 MT h sub h LT}for}{0 6 w h add{dup 0 exch MT\n"
"w sub w exch LT}for}ie}if BSt 13 eq BSt 14 eq or{w h gt{0 6 w h add{dup h MT\n"
"h sub 0 LT}for}{0 6 w h add{dup w exch MT w sub 0 exch LT}for}ie}if S}if BSt\n"
"15 eq{level3{LinGrad setpattern WFi{fill}{eofill}ie}if}if BSt 24 eq{}if\n"
"grestore}D/mat matrix d/ang1 D0/ang2 D0/w D0/h D0/x D0/y D0/ARC{/ang2 ED\n"
"/ang1 ED/h ED/w ED/y ED/x ED mat CM pop x w 2 div add y h 2 div add TR 1 h w\n"
"div neg scale ang2 0 ge{0 0 w 2 div ang1 ang1 ang2 add arc}{0 0 w 2 div ang1\n"
"ang1 ang2 add arcn}ie mat SM}D/C D0/P{NP MT 0.5 0.5 rmoveto 0 -1 RL -1 0 RL\n"
"0 1 RL CP fill}D/DL{NP MT LT QS}D/R{/h ED/w ED/y ED/x ED NP x y MT 0 h RL w\n"
"0 RL 0 h neg RL CP BF QS}D/xr D0/yr D0/rx D0/ry D0/rx2 D0/ry2 D0/E{/h ED/w\n"
"ED/y ED/x ED mat CM pop x w 2 div add y h 2 div add TR 1 h w div scale NP 0\n"
"0 w 2 div 0 360 arc mat SM BF QS}D/BC{/BkCol ED}D/BR{/BCol ED/BSt ED}D/NB{0\n"
"[0 0 0]BR}D/PE{setlinejoin setlinecap/PCol ED/LWi ED/PSt ED LWi 0 eq{0.25\n"
"/LWi ED}if PCol SC}D/P1{1 0 3 2 roll 0 0 PE}D/ST{defM SM concat}D/MF{true\n"
"exch true exch{exch pop exch pop dup 0 get dup findfont dup/FontName get 3\n"
"-1 roll eq{exit}if}forall exch dup 1 get/fxscale ED 2 get/fslant ED exch\n"
"/fencoding ED[fxscale 0 fslant 1 0 0]makefont fencoding false eq{}{dup\n"
"maxlength dict begin{1 i/FID ne{def}{pop pop}ifelse}forall/Encoding\n"
"fencoding d currentdict end}ie definefont pop}D/MFEmb{findfont dup length\n"
"dict begin{1 i/FID ne{d}{pop pop}ifelse}forall/Encoding ED currentdict end\n"
"definefont pop}D/DF{findfont/fs 3 -1 roll d[fs 0 0 fs -1 mul 0 0]makefont d}\n"
"D/ty 0 d/Y{/ty ED}D/Tl{gsave SW NP 1 i exch MT 1 i 0 RL S grestore}D/XYT{ty\n"
"MT/xyshow where{pop pop xyshow}{exch pop 1 i dup length 2 div exch\n"
"stringwidth pop 3 -1 roll exch sub exch div exch 0 exch ashow}ie}D/AT{ty MT\n"
"1 i dup length 2 div exch stringwidth pop 3 -1 roll exch sub exch div exch 0\n"
"exch ashow}D/QI{/C save d pageinit/OMo false d}D/QP{C restore showpage}D/SPD\n"
"{/setpagedevice where{<< 3 1 roll >> setpagedevice}{pop pop}ie}D/CLS{gsave\n"
"NP}D/ACR{/h ED/w ED/y ED/x ED x y MT 0 h RL w 0 RL 0 h neg RL CP}D/CLO{\n"
"grestore}D\n";

static const char * const agl =
".notdef\0space\0exclam\0quotedbl\0numbersign\0dollar\0percent\0ampersand\0"
"quotesingle\0parenleft\0parenright\0asterisk\0plus\0comma\0hyphen\0period\0"
"slash\0zero\0one\0two\0three\0four\0five\0six\0seven\0eight\0nine\0colon\0"
"semicolon\0less\0equal\0greater\0question\0at\0A\0B\0C\0D\0E\0F\0G\0H\0I\0J\0"
"K\0L\0M\0N\0O\0P\0Q\0R\0S\0T\0U\0V\0W\0X\0Y\0Z\0bracketleft\0backslash\0"
"bracketright\0asciicircum\0underscore\0grave\0a\0b\0c\0d\0e\0f\0g\0h\0i\0j\0"
"k\0l\0m\0n\0o\0p\0q\0r\0s\0t\0u\0v\0w\0x\0y\0z\0braceleft\0bar\0braceright\0"
"asciitilde\0space\0exclamdown\0cent\0sterling\0currency\0yen\0brokenbar\0"
"section\0dieresis\0copyright\0ordfeminine\0guillemotleft\0logicalnot\0"
"hyphen\0registered\0macron\0degree\0plusminus\0twosuperior\0threesuperior\0"
"acute\0mu\0paragraph\0periodcentered\0cedilla\0onesuperior\0ordmasculine\0"
"guillemotright\0onequarter\0onehalf\0threequarters\0questiondown\0Agrave\0"
"Aacute\0Acircumflex\0Atilde\0Adieresis\0Aring\0AE\0Ccedilla\0Egrave\0Eacute\0"
"Ecircumflex\0Edieresis\0Igrave\0Iacute\0Icircumflex\0Idieresis\0Eth\0Ntilde\0"
"Ograve\0Oacute\0Ocircumflex\0Otilde\0Odieresis\0multiply\0Oslash\0Ugrave\0"
"Uacute\0Ucircumflex\0Udieresis\0Yacute\0Thorn\0germandbls\0agrave\0aacute\0"
"acircumflex\0atilde\0adieresis\0aring\0ae\0ccedilla\0egrave\0eacute\0"
"ecircumflex\0edieresis\0igrave\0iacute\0icircumflex\0idieresis\0eth\0ntilde\0"
"ograve\0oacute\0ocircumflex\0otilde\0odieresis\0divide\0oslash\0ugrave\0"
"uacute\0ucircumflex\0udieresis\0yacute\0thorn\0ydieresis\0Amacron\0amacron\0"
"Abreve\0abreve\0Aogonek\0aogonek\0Cacute\0cacute\0Ccircumflex\0ccircumflex\0"
"Cdotaccent\0cdotaccent\0Ccaron\0ccaron\0Dcaron\0dcaron\0Dcroat\0dcroat\0"
"Emacron\0emacron\0Ebreve\0ebreve\0Edotaccent\0edotaccent\0Eogonek\0eogonek\0"
"Ecaron\0ecaron\0Gcircumflex\0gcircumflex\0Gbreve\0gbreve\0Gdotaccent\0"
"gdotaccent\0Gcommaaccent\0gcommaaccent\0Hcircumflex\0hcircumflex\0Hbar\0"
"hbar\0Itilde\0itilde\0Imacron\0imacron\0Ibreve\0ibreve\0Iogonek\0iogonek\0"
"Idotaccent\0dotlessi\0IJ\0ij\0Jcircumflex\0jcircumflex\0Kcommaaccent\0"
"kcommaaccent\0kgreenlandic\0Lacute\0lacute\0Lcommaaccent\0lcommaaccent\0"
"Lcaron\0lcaron\0Ldot\0ldot\0Lslash\0lslash\0Nacute\0nacute\0Ncommaaccent\0"
"ncommaaccent\0Ncaron\0ncaron\0napostrophe\0Eng\0eng\0Omacron\0omacron\0"
"Obreve\0obreve\0Ohungarumlaut\0ohungarumlaut\0OE\0oe\0Racute\0racute\0"
"Rcommaaccent\0rcommaaccent\0Rcaron\0rcaron\0Sacute\0sacute\0Scircumflex\0"
"scircumflex\0Scedilla\0scedilla\0Scaron\0scaron\0Tcommaaccent\0tcommaaccent\0"
"Tcaron\0tcaron\0Tbar\0tbar\0Utilde\0utilde\0Umacron\0umacron\0Ubreve\0"
"ubreve\0Uring\0uring\0Uhungarumlaut\0uhungarumlaut\0Uogonek\0uogonek\0"
"Wcircumflex\0wcircumflex\0Ycircumflex\0ycircumflex\0Ydieresis\0Zacute\0"
"zacute\0Zdotaccent\0zdotaccent\0Zcaron\0zcaron\0longs\0florin\0Ohorn\0ohorn\0"
"Uhorn\0uhorn\0Gcaron\0gcaron\0Aringacute\0aringacute\0AEacute\0aeacute\0"
"Oslashacute\0oslashacute\0Scommaaccent\0scommaaccent\0Tcommaaccent\0"
"tcommaaccent\0afii57929\0afii64937\0circumflex\0caron\0macron\0breve\0"
"dotaccent\0ring\0ogonek\0tilde\0hungarumlaut\0gravecomb\0acutecomb\0"
"tildecomb\0hookabovecomb\0dotbelowcomb\0tonos\0dieresistonos\0Alphatonos\0"
"anoteleia\0Epsilontonos\0Etatonos\0Iotatonos\0Omicrontonos\0Upsilontonos\0"
"Omegatonos\0iotadieresistonos\0Alpha\0Beta\0Gamma\0Delta\0Epsilon\0Zeta\0"
"Eta\0Theta\0Iota\0Kappa\0Lambda\0Mu\0Nu\0Xi\0Omicron\0Pi\0Rho\0Sigma\0Tau\0"
"Upsilon\0Phi\0Chi\0Psi\0Omega\0Iotadieresis\0Upsilondieresis\0alphatonos\0"
"epsilontonos\0etatonos\0iotatonos\0upsilondieresistonos\0alpha\0beta\0gamma\0"
"delta\0epsilon\0zeta\0eta\0theta\0iota\0kappa\0lambda\0mu\0nu\0xi\0omicron\0"
"pi\0rho\0sigma1\0sigma\0tau\0upsilon\0phi\0chi\0psi\0omega\0iotadieresis\0"
"upsilondieresis\0omicrontonos\0upsilontonos\0omegatonos\0theta1\0Upsilon1\0"
"phi1\0omega1\0afii10023\0afii10051\0afii10052\0afii10053\0afii10054\0"
"afii10055\0afii10056\0afii10057\0afii10058\0afii10059\0afii10060\0afii10061\0"
"afii10062\0afii10145\0afii10017\0afii10018\0afii10019\0afii10020\0afii10021\0"
"afii10022\0afii10024\0afii10025\0afii10026\0afii10027\0afii10028\0afii10029\0"
"afii10030\0afii10031\0afii10032\0afii10033\0afii10034\0afii10035\0afii10036\0"
"afii10037\0afii10038\0afii10039\0afii10040\0afii10041\0afii10042\0afii10043\0"
"afii10044\0afii10045\0afii10046\0afii10047\0afii10048\0afii10049\0afii10065\0"
"afii10066\0afii10067\0afii10068\0afii10069\0afii10070\0afii10072\0afii10073\0"
"afii10074\0afii10075\0afii10076\0afii10077\0afii10078\0afii10079\0afii10080\0"
"afii10081\0afii10082\0afii10083\0afii10084\0afii10085\0afii10086\0afii10087\0"
"afii10088\0afii10089\0afii10090\0afii10091\0afii10092\0afii10093\0afii10094\0"
"afii10095\0afii10096\0afii10097\0afii10071\0afii10099\0afii10100\0afii10101\0"
"afii10102\0afii10103\0afii10104\0afii10105\0afii10106\0afii10107\0afii10108\0"
"afii10109\0afii10110\0afii10193\0afii10146\0afii10194\0afii10147\0afii10195\0"
"afii10148\0afii10196\0afii10050\0afii10098\0afii10846\0afii57799\0afii57801\0"
"afii57800\0afii57802\0afii57793\0afii57794\0afii57795\0afii57798\0afii57797\0"
"afii57806\0afii57796\0afii57807\0afii57839\0afii57645\0afii57841\0afii57842\0"
"afii57804\0afii57803\0afii57658\0afii57664\0afii57665\0afii57666\0afii57667\0"
"afii57668\0afii57669\0afii57670\0afii57671\0afii57672\0afii57673\0afii57674\0"
"afii57675\0afii57676\0afii57677\0afii57678\0afii57679\0afii57680\0afii57681\0"
"afii57682\0afii57683\0afii57684\0afii57685\0afii57686\0afii57687\0afii57688\0"
"afii57689\0afii57690\0afii57716\0afii57717\0afii57718\0afii57388\0afii57403\0"
"afii57407\0afii57409\0afii57410\0afii57411\0afii57412\0afii57413\0afii57414\0"
"afii57415\0afii57416\0afii57417\0afii57418\0afii57419\0afii57420\0afii57421\0"
"afii57422\0afii57423\0afii57424\0afii57425\0afii57426\0afii57427\0afii57428\0"
"afii57429\0afii57430\0afii57431\0afii57432\0afii57433\0afii57434\0afii57440\0"
"afii57441\0afii57442\0afii57443\0afii57444\0afii57445\0afii57446\0afii57470\0"
"afii57448\0afii57449\0afii57450\0afii57451\0afii57452\0afii57453\0afii57454\0"
"afii57455\0afii57456\0afii57457\0afii57458\0afii57392\0afii57393\0afii57394\0"
"afii57395\0afii57396\0afii57397\0afii57398\0afii57399\0afii57400\0afii57401\0"
"afii57381\0afii63167\0afii57511\0afii57506\0afii57507\0afii57512\0afii57513\0"
"afii57508\0afii57505\0afii57509\0afii57514\0afii57519\0afii57534\0Wgrave\0"
"wgrave\0Wacute\0wacute\0Wdieresis\0wdieresis\0Ygrave\0ygrave\0afii61664\0"
"afii301\0afii299\0afii300\0figuredash\0endash\0emdash\0afii00208\0"
"underscoredbl\0quoteleft\0quoteright\0quotesinglbase\0quotereversed\0"
"quotedblleft\0quotedblright\0quotedblbase\0dagger\0daggerdbl\0bullet\0"
"onedotenleader\0twodotenleader\0ellipsis\0afii61573\0afii61574\0afii61575\0"
"perthousand\0minute\0second\0guilsinglleft\0guilsinglright\0exclamdbl\0"
"fraction\0zerosuperior\0foursuperior\0fivesuperior\0sixsuperior\0"
"sevensuperior\0eightsuperior\0ninesuperior\0parenleftsuperior\0"
"parenrightsuperior\0nsuperior\0zeroinferior\0oneinferior\0twoinferior\0"
"threeinferior\0fourinferior\0fiveinferior\0sixinferior\0seveninferior\0"
"eightinferior\0nineinferior\0parenleftinferior\0parenrightinferior\0"
"colonmonetary\0franc\0lira\0peseta\0afii57636\0dong\0Euro\0afii61248\0"
"Ifraktur\0afii61289\0afii61352\0weierstrass\0Rfraktur\0prescription\0"
"trademark\0Omega\0estimated\0aleph\0onethird\0twothirds\0oneeighth\0"
"threeeighths\0fiveeighths\0seveneighths\0arrowleft\0arrowup\0arrowright\0"
"arrowdown\0arrowboth\0arrowupdn\0arrowupdnbse\0carriagereturn\0arrowdblleft\0"
"arrowdblup\0arrowdblright\0arrowdbldown\0arrowdblboth\0universal\0"
"partialdiff\0existential\0emptyset\0Delta\0gradient\0element\0notelement\0"
"suchthat\0product\0summation\0minus\0fraction\0asteriskmath\0periodcentered\0"
"radical\0proportional\0infinity\0orthogonal\0angle\0logicaland\0logicalor\0"
"intersection\0union\0integral\0therefore\0similar\0congruent\0approxequal\0"
"notequal\0equivalence\0lessequal\0greaterequal\0propersubset\0"
"propersuperset\0notsubset\0reflexsubset\0reflexsuperset\0circleplus\0"
"circlemultiply\0perpendicular\0dotmath\0house\0revlogicalnot\0integraltp\0"
"integralbt\0angleleft\0angleright\0SF100000\0SF110000\0SF010000\0SF030000\0"
"SF020000\0SF040000\0SF080000\0SF090000\0SF060000\0SF070000\0SF050000\0"
"SF430000\0SF240000\0SF510000\0SF520000\0SF390000\0SF220000\0SF210000\0"
"SF250000\0SF500000\0SF490000\0SF380000\0SF280000\0SF270000\0SF260000\0"
"SF360000\0SF370000\0SF420000\0SF190000\0SF200000\0SF230000\0SF470000\0"
"SF480000\0SF410000\0SF450000\0SF460000\0SF400000\0SF540000\0SF530000\0"
"SF440000\0upblock\0dnblock\0block\0lfblock\0rtblock\0ltshade\0shade\0"
"dkshade\0filledbox\0H22073\0H18543\0H18551\0filledrect\0triagup\0triagrt\0"
"triagdn\0triaglf\0lozenge\0circle\0H18533\0invbullet\0invcircle\0openbullet\0"
"smileface\0invsmileface\0sun\0female\0male\0spade\0club\0heart\0diamond\0"
"musicalnote\0musicalnotedbl\0copyrightserif\0registerserif\0trademarkserif\0"
"radicalex\0arrowvertex\0arrowhorizex\0registersans\0copyrightsans\0"
"trademarksans\0parenlefttp\0parenleftex\0parenleftbt\0bracketlefttp\0"
"bracketleftex\0bracketleftbt\0bracelefttp\0braceleftmid\0braceleftbt\0"
"braceex\0integralex\0parenrighttp\0parenrightex\0parenrightbt\0"
"bracketrighttp\0bracketrightex\0bracketrightbt\0bracerighttp\0bracerightmid\0"
"bracerightbt\0ff\0fi\0fl\0ffi\0ffl\0afii57705\0afii57694\0afii57695\0"
;

static const struct { quint16 u; quint16 index; } unicodetoglyph[] = {
    {0x0000, 0}, {0x0020, 8}, {0x0021, 14}, {0x0022, 21},
    {0x0023, 30}, {0x0024, 41}, {0x0025, 48}, {0x0026, 56},
    {0x0027, 66}, {0x0028, 78}, {0x0029, 88}, {0x002A, 99},
    {0x002B, 108}, {0x002C, 113}, {0x002D, 119}, {0x002E, 126},
    {0x002F, 133}, {0x0030, 139}, {0x0031, 144}, {0x0032, 148},
    {0x0033, 152}, {0x0034, 158}, {0x0035, 163}, {0x0036, 168},
    {0x0037, 172}, {0x0038, 178}, {0x0039, 184}, {0x003A, 189},
    {0x003B, 195}, {0x003C, 205}, {0x003D, 210}, {0x003E, 216},
    {0x003F, 224}, {0x0040, 233}, {0x0041, 236}, {0x0042, 238},
    {0x0043, 240}, {0x0044, 242}, {0x0045, 244}, {0x0046, 246},
    {0x0047, 248}, {0x0048, 250}, {0x0049, 252}, {0x004A, 254},
    {0x004B, 256}, {0x004C, 258}, {0x004D, 260}, {0x004E, 262},
    {0x004F, 264}, {0x0050, 266}, {0x0051, 268}, {0x0052, 270},
    {0x0053, 272}, {0x0054, 274}, {0x0055, 276}, {0x0056, 278},
    {0x0057, 280}, {0x0058, 282}, {0x0059, 284}, {0x005A, 286},
    {0x005B, 288}, {0x005C, 300}, {0x005D, 310}, {0x005E, 323},
    {0x005F, 335}, {0x0060, 346}, {0x0061, 352}, {0x0062, 354},
    {0x0063, 356}, {0x0064, 358}, {0x0065, 360}, {0x0066, 362},
    {0x0067, 364}, {0x0068, 366}, {0x0069, 368}, {0x006A, 370},
    {0x006B, 372}, {0x006C, 374}, {0x006D, 376}, {0x006E, 378},
    {0x006F, 380}, {0x0070, 382}, {0x0071, 384}, {0x0072, 386},
    {0x0073, 388}, {0x0074, 390}, {0x0075, 392}, {0x0076, 394},
    {0x0077, 396}, {0x0078, 398}, {0x0079, 400}, {0x007A, 402},
    {0x007B, 404}, {0x007C, 414}, {0x007D, 418}, {0x007E, 429},
    {0x00A0, 440}, {0x00A1, 446}, {0x00A2, 457}, {0x00A3, 462},
    {0x00A4, 471}, {0x00A5, 480}, {0x00A6, 484}, {0x00A7, 494},
    {0x00A8, 502}, {0x00A9, 511}, {0x00AA, 521}, {0x00AB, 533},
    {0x00AC, 547}, {0x00AD, 558}, {0x00AE, 565}, {0x00AF, 576},
    {0x00B0, 583}, {0x00B1, 590}, {0x00B2, 600}, {0x00B3, 612},
    {0x00B4, 626}, {0x00B5, 632}, {0x00B6, 635}, {0x00B7, 645},
    {0x00B8, 660}, {0x00B9, 668}, {0x00BA, 680}, {0x00BB, 693},
    {0x00BC, 708}, {0x00BD, 719}, {0x00BE, 727}, {0x00BF, 741},
    {0x00C0, 754}, {0x00C1, 761}, {0x00C2, 768}, {0x00C3, 780},
    {0x00C4, 787}, {0x00C5, 797}, {0x00C6, 803}, {0x00C7, 806},
    {0x00C8, 815}, {0x00C9, 822}, {0x00CA, 829}, {0x00CB, 841},
    {0x00CC, 851}, {0x00CD, 858}, {0x00CE, 865}, {0x00CF, 877},
    {0x00D0, 887}, {0x00D1, 891}, {0x00D2, 898}, {0x00D3, 905},
    {0x00D4, 912}, {0x00D5, 924}, {0x00D6, 931}, {0x00D7, 941},
    {0x00D8, 950}, {0x00D9, 957}, {0x00DA, 964}, {0x00DB, 971},
    {0x00DC, 983}, {0x00DD, 993}, {0x00DE, 1000}, {0x00DF, 1006},
    {0x00E0, 1017}, {0x00E1, 1024}, {0x00E2, 1031}, {0x00E3, 1043},
    {0x00E4, 1050}, {0x00E5, 1060}, {0x00E6, 1066}, {0x00E7, 1069},
    {0x00E8, 1078}, {0x00E9, 1085}, {0x00EA, 1092}, {0x00EB, 1104},
    {0x00EC, 1114}, {0x00ED, 1121}, {0x00EE, 1128}, {0x00EF, 1140},
    {0x00F0, 1150}, {0x00F1, 1154}, {0x00F2, 1161}, {0x00F3, 1168},
    {0x00F4, 1175}, {0x00F5, 1187}, {0x00F6, 1194}, {0x00F7, 1204},
    {0x00F8, 1211}, {0x00F9, 1218}, {0x00FA, 1225}, {0x00FB, 1232},
    {0x00FC, 1244}, {0x00FD, 1254}, {0x00FE, 1261}, {0x00FF, 1267},
    {0x0100, 1277}, {0x0101, 1285}, {0x0102, 1293}, {0x0103, 1300},
    {0x0104, 1307}, {0x0105, 1315}, {0x0106, 1323}, {0x0107, 1330},
    {0x0108, 1337}, {0x0109, 1349}, {0x010A, 1361}, {0x010B, 1372},
    {0x010C, 1383}, {0x010D, 1390}, {0x010E, 1397}, {0x010F, 1404},
    {0x0110, 1411}, {0x0111, 1418}, {0x0112, 1425}, {0x0113, 1433},
    {0x0114, 1441}, {0x0115, 1448}, {0x0116, 1455}, {0x0117, 1466},
    {0x0118, 1477}, {0x0119, 1485}, {0x011A, 1493}, {0x011B, 1500},
    {0x011C, 1507}, {0x011D, 1519}, {0x011E, 1531}, {0x011F, 1538},
    {0x0120, 1545}, {0x0121, 1556}, {0x0122, 1567}, {0x0123, 1580},
    {0x0124, 1593}, {0x0125, 1605}, {0x0126, 1617}, {0x0127, 1622},
    {0x0128, 1627}, {0x0129, 1634}, {0x012A, 1641}, {0x012B, 1649},
    {0x012C, 1657}, {0x012D, 1664}, {0x012E, 1671}, {0x012F, 1679},
    {0x0130, 1687}, {0x0131, 1698}, {0x0132, 1707}, {0x0133, 1710},
    {0x0134, 1713}, {0x0135, 1725}, {0x0136, 1737}, {0x0137, 1750},
    {0x0138, 1763}, {0x0139, 1776}, {0x013A, 1783}, {0x013B, 1790},
    {0x013C, 1803}, {0x013D, 1816}, {0x013E, 1823}, {0x013F, 1830},
    {0x0140, 1835}, {0x0141, 1840}, {0x0142, 1847}, {0x0143, 1854},
    {0x0144, 1861}, {0x0145, 1868}, {0x0146, 1881}, {0x0147, 1894},
    {0x0148, 1901}, {0x0149, 1908}, {0x014A, 1920}, {0x014B, 1924},
    {0x014C, 1928}, {0x014D, 1936}, {0x014E, 1944}, {0x014F, 1951},
    {0x0150, 1958}, {0x0151, 1972}, {0x0152, 1986}, {0x0153, 1989},
    {0x0154, 1992}, {0x0155, 1999}, {0x0156, 2006}, {0x0157, 2019},
    {0x0158, 2032}, {0x0159, 2039}, {0x015A, 2046}, {0x015B, 2053},
    {0x015C, 2060}, {0x015D, 2072}, {0x015E, 2084}, {0x015F, 2093},
    {0x0160, 2102}, {0x0161, 2109}, {0x0162, 2116}, {0x0163, 2129},
    {0x0164, 2142}, {0x0165, 2149}, {0x0166, 2156}, {0x0167, 2161},
    {0x0168, 2166}, {0x0169, 2173}, {0x016A, 2180}, {0x016B, 2188},
    {0x016C, 2196}, {0x016D, 2203}, {0x016E, 2210}, {0x016F, 2216},
    {0x0170, 2222}, {0x0171, 2236}, {0x0172, 2250}, {0x0173, 2258},
    {0x0174, 2266}, {0x0175, 2278}, {0x0176, 2290}, {0x0177, 2302},
    {0x0178, 2314}, {0x0179, 2324}, {0x017A, 2331}, {0x017B, 2338},
    {0x017C, 2349}, {0x017D, 2360}, {0x017E, 2367}, {0x017F, 2374},
    {0x0192, 2380}, {0x01A0, 2387}, {0x01A1, 2393}, {0x01AF, 2399},
    {0x01B0, 2405}, {0x01E6, 2411}, {0x01E7, 2418}, {0x01FA, 2425},
    {0x01FB, 2436}, {0x01FC, 2447}, {0x01FD, 2455}, {0x01FE, 2463},
    {0x01FF, 2475}, {0x0218, 2487}, {0x0219, 2500}, {0x021A, 2513},
    {0x021B, 2526}, {0x02BC, 2539}, {0x02BD, 2549}, {0x02C6, 2559},
    {0x02C7, 2570}, {0x02C9, 2576}, {0x02D8, 2583}, {0x02D9, 2589},
    {0x02DA, 2599}, {0x02DB, 2604}, {0x02DC, 2611}, {0x02DD, 2617},
    {0x0300, 2630}, {0x0301, 2640}, {0x0303, 2650}, {0x0309, 2660},
    {0x0323, 2674}, {0x0384, 2687}, {0x0385, 2693}, {0x0386, 2707},
    {0x0387, 2718}, {0x0388, 2728}, {0x0389, 2741}, {0x038A, 2750},
    {0x038C, 2760}, {0x038E, 2773}, {0x038F, 2786}, {0x0390, 2797},
    {0x0391, 2815}, {0x0392, 2821}, {0x0393, 2826}, {0x0394, 2832},
    {0x0395, 2838}, {0x0396, 2846}, {0x0397, 2851}, {0x0398, 2855},
    {0x0399, 2861}, {0x039A, 2866}, {0x039B, 2872}, {0x039C, 2879},
    {0x039D, 2882}, {0x039E, 2885}, {0x039F, 2888}, {0x03A0, 2896},
    {0x03A1, 2899}, {0x03A3, 2903}, {0x03A4, 2909}, {0x03A5, 2913},
    {0x03A6, 2921}, {0x03A7, 2925}, {0x03A8, 2929}, {0x03A9, 2933},
    {0x03AA, 2939}, {0x03AB, 2952}, {0x03AC, 2968}, {0x03AD, 2979},
    {0x03AE, 2992}, {0x03AF, 3001}, {0x03B0, 3011}, {0x03B1, 3032},
    {0x03B2, 3038}, {0x03B3, 3043}, {0x03B4, 3049}, {0x03B5, 3055},
    {0x03B6, 3063}, {0x03B7, 3068}, {0x03B8, 3072}, {0x03B9, 3078},
    {0x03BA, 3083}, {0x03BB, 3089}, {0x03BC, 3096}, {0x03BD, 3099},
    {0x03BE, 3102}, {0x03BF, 3105}, {0x03C0, 3113}, {0x03C1, 3116},
    {0x03C2, 3120}, {0x03C3, 3127}, {0x03C4, 3133}, {0x03C5, 3137},
    {0x03C6, 3145}, {0x03C7, 3149}, {0x03C8, 3153}, {0x03C9, 3157},
    {0x03CA, 3163}, {0x03CB, 3176}, {0x03CC, 3192}, {0x03CD, 3205},
    {0x03CE, 3218}, {0x03D1, 3229}, {0x03D2, 3236}, {0x03D5, 3245},
    {0x03D6, 3250}, {0x0401, 3257}, {0x0402, 3267}, {0x0403, 3277},
    {0x0404, 3287}, {0x0405, 3297}, {0x0406, 3307}, {0x0407, 3317},
    {0x0408, 3327}, {0x0409, 3337}, {0x040A, 3347}, {0x040B, 3357},
    {0x040C, 3367}, {0x040E, 3377}, {0x040F, 3387}, {0x0410, 3397},
    {0x0411, 3407}, {0x0412, 3417}, {0x0413, 3427}, {0x0414, 3437},
    {0x0415, 3447}, {0x0416, 3457}, {0x0417, 3467}, {0x0418, 3477},
    {0x0419, 3487}, {0x041A, 3497}, {0x041B, 3507}, {0x041C, 3517},
    {0x041D, 3527}, {0x041E, 3537}, {0x041F, 3547}, {0x0420, 3557},
    {0x0421, 3567}, {0x0422, 3577}, {0x0423, 3587}, {0x0424, 3597},
    {0x0425, 3607}, {0x0426, 3617}, {0x0427, 3627}, {0x0428, 3637},
    {0x0429, 3647}, {0x042A, 3657}, {0x042B, 3667}, {0x042C, 3677},
    {0x042D, 3687}, {0x042E, 3697}, {0x042F, 3707}, {0x0430, 3717},
    {0x0431, 3727}, {0x0432, 3737}, {0x0433, 3747}, {0x0434, 3757},
    {0x0435, 3767}, {0x0436, 3777}, {0x0437, 3787}, {0x0438, 3797},
    {0x0439, 3807}, {0x043A, 3817}, {0x043B, 3827}, {0x043C, 3837},
    {0x043D, 3847}, {0x043E, 3857}, {0x043F, 3867}, {0x0440, 3877},
    {0x0441, 3887}, {0x0442, 3897}, {0x0443, 3907}, {0x0444, 3917},
    {0x0445, 3927}, {0x0446, 3937}, {0x0447, 3947}, {0x0448, 3957},
    {0x0449, 3967}, {0x044A, 3977}, {0x044B, 3987}, {0x044C, 3997},
    {0x044D, 4007}, {0x044E, 4017}, {0x044F, 4027}, {0x0451, 4037},
    {0x0452, 4047}, {0x0453, 4057}, {0x0454, 4067}, {0x0455, 4077},
    {0x0456, 4087}, {0x0457, 4097}, {0x0458, 4107}, {0x0459, 4117},
    {0x045A, 4127}, {0x045B, 4137}, {0x045C, 4147}, {0x045E, 4157},
    {0x045F, 4167}, {0x0462, 4177}, {0x0463, 4187}, {0x0472, 4197},
    {0x0473, 4207}, {0x0474, 4217}, {0x0475, 4227}, {0x0490, 4237},
    {0x0491, 4247}, {0x04D9, 4257}, {0x05B0, 4267}, {0x05B1, 4277},
    {0x05B2, 4287}, {0x05B3, 4297}, {0x05B4, 4307}, {0x05B5, 4317},
    {0x05B6, 4327}, {0x05B7, 4337}, {0x05B8, 4347}, {0x05B9, 4357},
    {0x05BB, 4367}, {0x05BC, 4377}, {0x05BD, 4387}, {0x05BE, 4397},
    {0x05BF, 4407}, {0x05C0, 4417}, {0x05C1, 4427}, {0x05C2, 4437},
    {0x05C3, 4447}, {0x05D0, 4457}, {0x05D1, 4467}, {0x05D2, 4477},
    {0x05D3, 4487}, {0x05D4, 4497}, {0x05D5, 4507}, {0x05D6, 4517},
    {0x05D7, 4527}, {0x05D8, 4537}, {0x05D9, 4547}, {0x05DA, 4557},
    {0x05DB, 4567}, {0x05DC, 4577}, {0x05DD, 4587}, {0x05DE, 4597},
    {0x05DF, 4607}, {0x05E0, 4617}, {0x05E1, 4627}, {0x05E2, 4637},
    {0x05E3, 4647}, {0x05E4, 4657}, {0x05E5, 4667}, {0x05E6, 4677},
    {0x05E7, 4687}, {0x05E8, 4697}, {0x05E9, 4707}, {0x05EA, 4717},
    {0x05F0, 4727}, {0x05F1, 4737}, {0x05F2, 4747}, {0x060C, 4757},
    {0x061B, 4767}, {0x061F, 4777}, {0x0621, 4787}, {0x0622, 4797},
    {0x0623, 4807}, {0x0624, 4817}, {0x0625, 4827}, {0x0626, 4837},
    {0x0627, 4847}, {0x0628, 4857}, {0x0629, 4867}, {0x062A, 4877},
    {0x062B, 4887}, {0x062C, 4897}, {0x062D, 4907}, {0x062E, 4917},
    {0x062F, 4927}, {0x0630, 4937}, {0x0631, 4947}, {0x0632, 4957},
    {0x0633, 4967}, {0x0634, 4977}, {0x0635, 4987}, {0x0636, 4997},
    {0x0637, 5007}, {0x0638, 5017}, {0x0639, 5027}, {0x063A, 5037},
    {0x0640, 5047}, {0x0641, 5057}, {0x0642, 5067}, {0x0643, 5077},
    {0x0644, 5087}, {0x0645, 5097}, {0x0646, 5107}, {0x0647, 5117},
    {0x0648, 5127}, {0x0649, 5137}, {0x064A, 5147}, {0x064B, 5157},
    {0x064C, 5167}, {0x064D, 5177}, {0x064E, 5187}, {0x064F, 5197},
    {0x0650, 5207}, {0x0651, 5217}, {0x0652, 5227}, {0x0660, 5237},
    {0x0661, 5247}, {0x0662, 5257}, {0x0663, 5267}, {0x0664, 5277},
    {0x0665, 5287}, {0x0666, 5297}, {0x0667, 5307}, {0x0668, 5317},
    {0x0669, 5327}, {0x066A, 5337}, {0x066D, 5347}, {0x0679, 5357},
    {0x067E, 5367}, {0x0686, 5377}, {0x0688, 5387}, {0x0691, 5397},
    {0x0698, 5407}, {0x06A4, 5417}, {0x06AF, 5427}, {0x06BA, 5437},
    {0x06D2, 5447}, {0x06D5, 5457}, {0x1E80, 5467}, {0x1E81, 5474},
    {0x1E82, 5481}, {0x1E83, 5488}, {0x1E84, 5495}, {0x1E85, 5505},
    {0x1EF2, 5515}, {0x1EF3, 5522}, {0x200C, 5529}, {0x200D, 5539},
    {0x200E, 5547}, {0x200F, 5555}, {0x2012, 5563}, {0x2013, 5574},
    {0x2014, 5581}, {0x2015, 5588}, {0x2017, 5598}, {0x2018, 5612},
    {0x2019, 5622}, {0x201A, 5633}, {0x201B, 5648}, {0x201C, 5662},
    {0x201D, 5675}, {0x201E, 5689}, {0x2020, 5702}, {0x2021, 5709},
    {0x2022, 5719}, {0x2024, 5726}, {0x2025, 5741}, {0x2026, 5756},
    {0x202C, 5765}, {0x202D, 5775}, {0x202E, 5785}, {0x2030, 5795},
    {0x2032, 5807}, {0x2033, 5814}, {0x2039, 5821}, {0x203A, 5835},
    {0x203C, 5850}, {0x2044, 5860}, {0x2070, 5869}, {0x2074, 5882},
    {0x2075, 5895}, {0x2076, 5908}, {0x2077, 5920}, {0x2078, 5934},
    {0x2079, 5948}, {0x207D, 5961}, {0x207E, 5979}, {0x207F, 5998},
    {0x2080, 6008}, {0x2081, 6021}, {0x2082, 6033}, {0x2083, 6045},
    {0x2084, 6059}, {0x2085, 6072}, {0x2086, 6085}, {0x2087, 6097},
    {0x2088, 6111}, {0x2089, 6125}, {0x208D, 6138}, {0x208E, 6156},
    {0x20A1, 6175}, {0x20A3, 6189}, {0x20A4, 6195}, {0x20A7, 6200},
    {0x20AA, 6207}, {0x20AB, 6217}, {0x20AC, 6222}, {0x2105, 6227},
    {0x2111, 6237}, {0x2113, 6246}, {0x2116, 6256}, {0x2118, 6266},
    {0x211C, 6278}, {0x211E, 6287}, {0x2122, 6300}, {0x2126, 6310},
    {0x212E, 6316}, {0x2135, 6326}, {0x2153, 6332}, {0x2154, 6341},
    {0x215B, 6351}, {0x215C, 6361}, {0x215D, 6374}, {0x215E, 6386},
    {0x2190, 6399}, {0x2191, 6409}, {0x2192, 6417}, {0x2193, 6428},
    {0x2194, 6438}, {0x2195, 6448}, {0x21A8, 6458}, {0x21B5, 6471},
    {0x21D0, 6486}, {0x21D1, 6499}, {0x21D2, 6510}, {0x21D3, 6524},
    {0x21D4, 6537}, {0x2200, 6550}, {0x2202, 6560}, {0x2203, 6572},
    {0x2205, 6584}, {0x2206, 6593}, {0x2207, 6599}, {0x2208, 6608},
    {0x2209, 6616}, {0x220B, 6627}, {0x220F, 6636}, {0x2211, 6644},
    {0x2212, 6654}, {0x2215, 6660}, {0x2217, 6669}, {0x2219, 6682},
    {0x221A, 6697}, {0x221D, 6705}, {0x221E, 6718}, {0x221F, 6727},
    {0x2220, 6738}, {0x2227, 6744}, {0x2228, 6755}, {0x2229, 6765},
    {0x222A, 6778}, {0x222B, 6784}, {0x2234, 6793}, {0x223C, 6803},
    {0x2245, 6811}, {0x2248, 6821}, {0x2260, 6833}, {0x2261, 6842},
    {0x2264, 6854}, {0x2265, 6864}, {0x2282, 6877}, {0x2283, 6890},
    {0x2284, 6905}, {0x2286, 6915}, {0x2287, 6928}, {0x2295, 6943},
    {0x2297, 6954}, {0x22A5, 6969}, {0x22C5, 6983}, {0x2302, 6991},
    {0x2310, 6997}, {0x2320, 7011}, {0x2321, 7022}, {0x2329, 7033},
    {0x232A, 7043}, {0x2500, 7054}, {0x2502, 7063}, {0x250C, 7072},
    {0x2510, 7081}, {0x2514, 7090}, {0x2518, 7099}, {0x251C, 7108},
    {0x2524, 7117}, {0x252C, 7126}, {0x2534, 7135}, {0x253C, 7144},
    {0x2550, 7153}, {0x2551, 7162}, {0x2552, 7171}, {0x2553, 7180},
    {0x2554, 7189}, {0x2555, 7198}, {0x2556, 7207}, {0x2557, 7216},
    {0x2558, 7225}, {0x2559, 7234}, {0x255A, 7243}, {0x255B, 7252},
    {0x255C, 7261}, {0x255D, 7270}, {0x255E, 7279}, {0x255F, 7288},
    {0x2560, 7297}, {0x2561, 7306}, {0x2562, 7315}, {0x2563, 7324},
    {0x2564, 7333}, {0x2565, 7342}, {0x2566, 7351}, {0x2567, 7360},
    {0x2568, 7369}, {0x2569, 7378}, {0x256A, 7387}, {0x256B, 7396},
    {0x256C, 7405}, {0x2580, 7414}, {0x2584, 7422}, {0x2588, 7430},
    {0x258C, 7436}, {0x2590, 7444}, {0x2591, 7452}, {0x2592, 7460},
    {0x2593, 7466}, {0x25A0, 7474}, {0x25A1, 7484}, {0x25AA, 7491},
    {0x25AB, 7498}, {0x25AC, 7505}, {0x25B2, 7516}, {0x25BA, 7524},
    {0x25BC, 7532}, {0x25C4, 7540}, {0x25CA, 7548}, {0x25CB, 7556},
    {0x25CF, 7563}, {0x25D8, 7570}, {0x25D9, 7580}, {0x25E6, 7590},
    {0x263A, 7601}, {0x263B, 7611}, {0x263C, 7624}, {0x2640, 7628},
    {0x2642, 7635}, {0x2660, 7640}, {0x2663, 7646}, {0x2665, 7651},
    {0x2666, 7657}, {0x266A, 7665}, {0x266B, 7677}, {0xF6D9, 7692},
    {0xF6DA, 7707}, {0xF6DB, 7721}, {0xF8E5, 7736}, {0xF8E6, 7746},
    {0xF8E7, 7758}, {0xF8E8, 7771}, {0xF8E9, 7784}, {0xF8EA, 7798},
    {0xF8EB, 7812}, {0xF8EC, 7824}, {0xF8ED, 7836}, {0xF8EE, 7848},
    {0xF8EF, 7862}, {0xF8F0, 7876}, {0xF8F1, 7890}, {0xF8F2, 7902},
    {0xF8F3, 7915}, {0xF8F4, 7927}, {0xF8F5, 7935}, {0xF8F6, 7946},
    {0xF8F7, 7959}, {0xF8F8, 7972}, {0xF8F9, 7985}, {0xF8FA, 8000},
    {0xF8FB, 8015}, {0xF8FC, 8030}, {0xF8FD, 8043}, {0xF8FE, 8057},
    {0xFB00, 8070}, {0xFB01, 8073}, {0xFB02, 8076}, {0xFB03, 8079},
    {0xFB04, 8083}, {0xFB1F, 8087}, {0xFB2A, 8097}, {0xFB2B, 8107},
    {0xFB35, 8117}, {0xFB4B, 8127}, {0xFFFF, 8137}
};





#define MM(n) int((n * 720 + 127) / 254)
#define IN(n) int(n * 72)

struct PaperSize {
    int width, height;
};

static const PaperSize paperSizes[QPrinter::NPageSize] =
{
    {  MM(210), MM(297) },      // A4
    {  MM(176), MM(250) },      // B5
    {  IN(8.5), IN(11) },       // Letter
    {  IN(8.5), IN(14) },       // Legal
    {  IN(7.5), IN(10) },       // Executive
    {  MM(841), MM(1189) },     // A0
    {  MM(594), MM(841) },      // A1
    {  MM(420), MM(594) },      // A2
    {  MM(297), MM(420) },      // A3
    {  MM(148), MM(210) },      // A5
    {  MM(105), MM(148) },      // A6
    {  MM(74), MM(105)},        // A7
    {  MM(52), MM(74) },        // A8
    {  MM(37), MM(52) },        // A9
    {  MM(1000), MM(1414) },    // B0
    {  MM(707), MM(1000) },     // B1
    {  MM(31), MM(44) },        // B10
    {  MM(500), MM(707) },      // B2
    {  MM(353), MM(500) },      // B3
    {  MM(250), MM(353) },      // B4
    {  MM(125), MM(176) },      // B6
    {  MM(88), MM(125) },       // B7
    {  MM(62), MM(88) },        // B8
    {  MM(44), MM(62) },        // B9
    {  MM(162),    MM(229) },   // C5E
    {  IN(4.125),  IN(9.5) },   // Comm10E
    {  MM(110),    MM(220) },   // DLE
    {  IN(8.5),    IN(13) },    // Folio
    {  IN(17),     IN(11) },    // Ledger
    {  IN(11),     IN(17) }     // Tabloid
};



// ---------------------------------------------------------------------
// postscript font substitution dictionary. We assume every postscript printer has at least
// Helvetica, Times, Courier and Symbol

#if defined (Q_WS_WIN) && defined (Q_CC_MSVC)
#  pragma warning(disable: 4305)
#endif

struct psfont {
    const char *psname;
    qreal slant;
    qreal xscale;
};

static const psfont Arial[] = {
    {"Arial", 0, 84.04 },
    { "Arial-Italic", 0, 84.04 },
    { "Arial-Bold", 0, 88.65 },
    { "Arial-BoldItalic", 0, 88.65 }
};

static const psfont AvantGarde[] = {
    { "AvantGarde-Book", 0, 87.43 },
    { "AvantGarde-BookOblique", 0, 88.09 },
    { "AvantGarde-Demi", 0, 88.09 },
    { "AvantGarde-DemiOblique", 0, 87.43 },
};

static const psfont Bookman [] = {
    { "Bookman-Light", 0, 93.78 },
    { "Bookman-LightItalic", 0, 91.42 },
    { "Bookman-Demi", 0, 99.86 },
    { "Bookman-DemiItalic", 0, 101.54 }
};

static const psfont Charter [] = {
    { "CharterBT-Roman", 0, 84.04 },
    { "CharterBT-Italic", 0.0, 81.92 },
    { "CharterBT-Bold", 0, 88.99 },
    { "CharterBT-BoldItalic", 0.0, 88.20 }
};

static const psfont Courier [] = {
    { "Courier", 0, 100. },
    { "Courier-Oblique", 0, 100. },
    { "Courier-Bold", 0, 100. },
    { "Courier-BoldOblique", 0, 100. }
};

static const psfont Garamond [] = {
    { "Garamond-Antiqua", 0, 78.13 },
    { "Garamond-Kursiv", 0, 78.13 },
    { "Garamond-Halbfett", 0, 78.13 },
    { "Garamond-KursivHalbfett", 0, 78.13 }
};

static const psfont GillSans [] = { // estimated value for xstretch
    { "GillSans", 0, 82 },
    { "GillSans-Italic", 0, 82 },
    { "GillSans-Bold", 0, 82 },
    { "GillSans-BoldItalic", 0, 82 }
};

static const psfont Helvetica [] = {
    { "Helvetica", 0, 84.04 },
    { "Helvetica-Oblique", 0, 84.04 },
    { "Helvetica-Bold", 0, 88.65 },
    { "Helvetica-BoldOblique", 0, 88.65 }
};

static const psfont Letter [] = {
    { "LetterGothic", 0, 83.32 },
    { "LetterGothic-Italic", 0, 83.32 },
    { "LetterGothic-Bold", 0, 83.32 },
    { "LetterGothic-Bold", 0.2, 83.32 }
};

static const psfont LucidaSans [] = {
    { "LucidaSans", 0, 94.36 },
    { "LucidaSans-Oblique", 0, 94.36 },
    { "LucidaSans-Demi", 0, 98.10 },
    { "LucidaSans-DemiOblique", 0, 98.08 }
};

static const psfont LucidaSansTT [] = {
    { "LucidaSans-Typewriter", 0, 100.50 },
    { "LucidaSans-TypewriterOblique", 0, 100.50 },
    { "LucidaSans-TypewriterBold", 0, 100.50 },
    { "LucidaSans-TypewriterBoldOblique", 0, 100.50 }
};

static const psfont LucidaBright [] = {
    { "LucidaBright", 0, 93.45 },
    { "LucidaBright-Italic", 0, 91.98 },
    { "LucidaBright-Demi", 0, 96.22 },
    { "LucidaBright-DemiItalic", 0, 96.98 }
};

static const psfont Palatino [] = {
    { "Palatino-Roman", 0, 82.45 },
    { "Palatino-Italic", 0, 76.56 },
    { "Palatino-Bold", 0, 83.49 },
    { "Palatino-BoldItalic", 0, 81.51 }
};

static const psfont Symbol [] = {
    { "Symbol", 0, 82.56 },
    { "Symbol", 0.2, 82.56 },
    { "Symbol", 0, 82.56 },
    { "Symbol", 0.2, 82.56 }
};

static const psfont Tahoma [] = {
    { "Tahoma", 0, 83.45 },
    { "Tahoma", 0.2, 83.45 },
    { "Tahoma-Bold", 0, 95.59 },
    { "Tahoma-Bold", 0.2, 95.59 }
};

static const psfont Times [] = {
    { "Times-Roman", 0, 82.45 },
    { "Times-Italic", 0, 82.45 },
    { "Times-Bold", 0, 82.45 },
    { "Times-BoldItalic", 0, 82.45 }
};

static const psfont Verdana [] = {
    { "Verdana", 0, 96.06 },
    { "Verdana-Italic", 0, 96.06 },
    { "Verdana-Bold", 0, 107.12 },
    { "Verdana-BoldItalic", 0, 107.10 }
};

static const psfont Utopia [] = {
    { "Utopia-Regular", 0, 84.70 },
    { "Utopia-Regular", 0.2, 84.70 },
    { "Utopia-Bold", 0, 88.01 },
    { "Utopia-Bold", 0.2, 88.01 }
};

static const psfont * const SansSerifReplacements[] = {
    Helvetica, 0
        };
static const psfont * const SerifReplacements[] = {
    Times, 0
        };
static const psfont * const FixedReplacements[] = {
    Courier, 0
        };
static const psfont * const TahomaReplacements[] = {
    Verdana, AvantGarde, Helvetica, 0
        };
static const psfont * const VerdanaReplacements[] = {
    Tahoma, AvantGarde, Helvetica, 0
        };

static const struct {
    const char * input; // spaces are stripped in here, and everything lowercase
    const psfont * ps;
    const psfont *const * replacements;
} postscriptFonts [] = {
    { "arial", Arial, SansSerifReplacements },
    { "arialmt", Arial, SansSerifReplacements },
    { "arialunicodems", Arial, SansSerifReplacements },
    { "avantgarde", AvantGarde, SansSerifReplacements },
    { "bookman", Bookman, SerifReplacements },
    { "charter", Charter, SansSerifReplacements },
    { "bitstreamcharter", Charter, SansSerifReplacements },
    { "bitstreamcyberbit", Times, SerifReplacements },
    { "courier", Courier, 0 },
    { "couriernew", Courier, 0 },
    { "fixed", Courier, 0 },
    { "garamond", Garamond, SerifReplacements },
    { "gillsans", GillSans, SansSerifReplacements },
    { "helvetica", Helvetica, 0 },
    { "letter", Letter, FixedReplacements },
    { "lucida", LucidaSans, SansSerifReplacements },
    { "lucidasans", LucidaSans, SansSerifReplacements },
    { "lucidabright", LucidaBright, SerifReplacements },
    { "lucidasanstypewriter", LucidaSansTT, FixedReplacements },
    { "luciduxsans", LucidaSans, SansSerifReplacements },
    { "luciduxserif", LucidaBright, SerifReplacements },
    { "luciduxmono", LucidaSansTT, FixedReplacements },
    { "palatino", Palatino, SerifReplacements },
    { "symbol", Symbol, 0 },
    { "tahoma", Tahoma, TahomaReplacements },
    { "terminal", Courier, 0 },
    { "times", Times, 0 },
    { "timesnewroman", Times, 0 },
    { "verdana", Verdana, VerdanaReplacements },
    { "utopia", Utopia, SerifReplacements },
    { 0, 0, 0 }
};


// ------------------------------End of static data ----------------------------------

// make sure DSC comments are not longer than 255 chars per line.
static QString wrapDSC(const QString &str)
{
    QString dsc = str.simplified();
    const int wrapAt = 254;
    QString wrapped;
    if (dsc.length() < wrapAt)
        wrapped = dsc;
    else {
        wrapped = dsc.left(wrapAt);
        QString tmp = dsc.mid(wrapAt);
        while (tmp.length() > wrapAt-3) {
            wrapped += "\n%%+" + tmp.left(wrapAt-3);
            tmp = tmp.mid(wrapAt-3);
        }
        wrapped += "\n%%+" + tmp;
    }
    return wrapped + "\n";
}

static QString toString(const qreal num)
{
    return QString::number( num, 'f', 3 );
}

// ----------------------------- Internal class declarations -----------------------------

class QPSPrintEngineFont;

class QPSPrintEnginePrivate : public QPaintEnginePrivate {
public:
    QPSPrintEnginePrivate(QPrinter::PrinterMode m);
    ~QPSPrintEnginePrivate();

    void orientationSetup();
    void emitHeader(bool finished);
    void setFont(QFontEngine *fe);
    void drawImage(qreal x, qreal y, qreal w, qreal h, const QImage &img, const QImage &mask);
    void flushPage(bool last = false);
    QRect paperRect() const;
    QRect pageRect() const;

    QPrinter   *printer;
    int         pageCount;
    bool        epsf;
    QString     fontsUsed;

    // outstream is the stream the build up pages are copied to. It points to buffer
    // at the start, and is reset to use the outDevice after emitHeader has been called.
    QTextStream outStream;

    // stores the descriptions of the first pages. outStream operates on this buffer
    // until we call emitHeader
    QBuffer *buffer;
    int pagesInBuffer;

    // the device the output is in the end streamed to.
    QIODevice * outDevice;
    int fd;

    // buffer for the current page. Needed because we might have page fonts.
    QBuffer *pageBuffer;
    QTextStream pageStream;

    QHash<QString, QString> headerFontNames;
    QHash<QString, QString> pageFontNames;
    QHash<QString, QPSPrintEngineFont *> fonts;
    QPSPrintEngineFont *currentPSFont;
    QFontEngine *currentFont;
    bool firstPage;
    int headerFontNumber;
    int pageFontNumber;
    QBuffer * fontBuffer;
    QTextStream fontStream;
    bool clipOn;
    QRect boundingBox;
    QPen cpen;
    QBrush cbrush;
    QColor bkColor;
    Qt::BGMode bkMode;
//     QFontMetrics fm;
//     QFont currentUsed;
    qreal scale;

    bool embedFonts;
    QStringList fontpath;
    bool        collate;
    int         copies;
    QString printerName;
    QString outputFileName;
    QString selectionOption;
    QString printProgram;
    QString docName;
    QString creator;
    QPrinter::Orientation orientation;
    QPrinter::PageSize pageSize;
    QPrinter::PageOrder pageOrder;
    int resolution;
    QPrinter::ColorMode colorMode;
    bool fullPage;
    QPrinter::PaperSource paperSource;
    QPrinter::PrinterState printerState;
};


class QPSPrintEngineFont {
public:
    QPSPrintEngineFont(QFontEngine *fe);
    virtual ~QPSPrintEngineFont();
    virtual QString postScriptFontName() { return psname; }
    virtual QString defineFont(QTextStream &stream, const QString &ps, const QString &key,
                             QPSPrintEnginePrivate *ptr, int pixelSize);
    virtual void download(QTextStream& s, bool global);
    virtual void drawText(QTextStream &stream, QPSPrintEnginePrivate *d, const QPointF &p, const QTextItemInt &ti);
    virtual unsigned short mapUnicode(unsigned short unicode);
    void downloadMapping(QTextStream &s, bool global);
    QString glyphName(unsigned short glyphindex, bool *glyphSet = 0);
    virtual void restore();

    virtual unsigned short unicode_for_glyph(int glyphindex) { return glyphindex; }
    virtual unsigned short glyph_for_unicode(unsigned short unicode) { return unicode; }
    unsigned short insertIntoSubset(unsigned short unicode);
    virtual bool embedded() { return false; }

    bool operator == (const QPSPrintEngineFont &other) {
        return other.psname == psname;
    }
    inline void setSymbol() { symbol = true; }

    QFontEngine *fontEngine() const { return fe; }

protected:
    QString psname;
    QStringList replacementList;
    QFontEngine *fe;

    QMap<unsigned short, unsigned short> subset;      // unicode subset in the global font
    QMap<unsigned short, unsigned short> page_subset; // subset added in this page
    int subsetCount;
    int pageSubsetCount;
    bool global_dict;
    bool downloaded;
    bool symbol;
};

QPSPrintEngineFont::~QPSPrintEngineFont()
{
}

// ------------------- end of class declarations ---------------------------

// --------------------------------------------------------------
//   beginning of font related methods
// --------------------------------------------------------------


static int getPsFontType(const QFontEngine *fe)
{
    int weight = fe->fontDef.weight;
    bool italic = fe->fontDef.style != QFont::StyleNormal;

    int type = 0; // used to look up in the psname array
    // get the right modification, or build something
    if (weight > QFont::Normal && italic)
        type = 3;
    else if (weight > QFont::Normal)
        type = 2;
    else if (italic)
        type = 1;
    return type;
}

static int addPsFontNameExtension(const QFontEngine *fe, QString &ps, const psfont *psf = 0)
{
    int type = getPsFontType(fe);

    if (psf) {
        ps = QLatin1String(psf[type].psname);
    } else {
        switch (type) {
        case 1:
            ps.append(QLatin1String("-Italic"));
            break;
        case 2:
            ps.append(QLatin1String("-Bold"));
            break;
        case 3:
            ps.append(QLatin1String("-BoldItalic"));
            break;
        case 0:
        default:
            break;
        }
    }
    return type;
}

static QString makePSFontName(const QFontEngine *fe, int *listpos = 0, int *ftype = 0)
{
  QString ps;
  int i;

  QString family = fe->fontDef.family.toLower();

  // try to make a "good" postscript name
  ps = family.simplified();
  i = 0;
  while(i < ps.length()) {
    if (i != 0 && ps[i] == '[') {
      if (ps[i-1] == ' ')
        ps.truncate (i-1);
      else
        ps.truncate (i);
      break;
    }
    if (i == 0 || ps[i-1] == ' ') {
      ps[i] = ps[i].toUpper();
      if (i)
        ps.remove(i-1, 1);
      else
        i++;
    } else {
      i++;
    }
  }

  if (ps.isEmpty())
      ps = "Helvetica";

  // see if the table has a better name
  i = 0;
  QString lowerName = ps.toLower();
  while(postscriptFonts[i].input &&
         postscriptFonts[i].input != lowerName)
    i++;
  const psfont *psf = postscriptFonts[i].ps;

  int type = addPsFontNameExtension(fe, ps, psf);

  if (listpos)
      *listpos = i;
  if (ftype)
      *ftype = type;
  return ps;
}

static void appendReplacements(QStringList &list, const psfont * const * replacements, int type, qreal xscale = 100.)
{
    // iterate through the replacement fonts
    while (*replacements) {
        const psfont *psf = *replacements;
        QString ps = "[/" + QLatin1String(psf[type].psname) + " " +
                     toString(xscale / psf[type].xscale) + " " +
                     toString(psf[type].slant) + "]";
        list.append(ps);
        ++replacements;
    }
}

static QStringList makePSFontNameList(const QFontEngine *fe, const QString &psname = QString::null, bool useNameForLookup = false)
{
    int i;
    int type;
    QStringList list;
    QString ps = psname;

    if (!ps.isEmpty() && !useNameForLookup) {
        QString best = "[/" + ps + " 1.0 0.0]";
        list.append(best);
    }

    ps = makePSFontName(fe, &i, &type);

    const psfont *psf = postscriptFonts[i].ps;
    const psfont * const * replacements = postscriptFonts[i].replacements;
    qreal xscale = 100;
    if (psf) {
        // xscale for the "right" font is always 1. We scale the replacements...
        xscale = psf->xscale;
        ps = "[/" + QLatin1String(psf[type].psname) + " 1.0 " +
             toString(psf[type].slant) + "]";
    } else {
        ps = "[/" + ps + " 1.0 0.0]";
        // only add default replacement fonts in case this font was unknown.
        if (fe->fontDef.fixedPitch) {
            replacements = FixedReplacements;
        } else {
            replacements = SansSerifReplacements;
            // 100 is courier, but most fonts are not as wide as courier. Using 100
            // here would make letters overlap for some fonts. This value is empirical.
            xscale = 83;
        }
    }
    list.append(ps);

    if (replacements)
        appendReplacements(list, replacements, type, xscale);
    return list;
}

static void emitPSFontNameList(QTextStream &s, const QString &psname, const QStringList &list)
{
    s << "/" << psname << "List [\n";
    s << list.join("\n  ");
    s << "\n] d\n";
}


// ========================== FONT CLASSES  ===============


QPSPrintEngineFont::QPSPrintEngineFont(QFontEngine *fontEngine)
    : fe(fontEngine)
{
    global_dict = false;
    downloaded  = false;
    symbol = false;
    // map 0 to .notdef
    subset.insert(0, 0);
    subsetCount = 1;
    pageSubsetCount = 0;
}

unsigned short QPSPrintEngineFont::insertIntoSubset(unsigned short u)
{
    unsigned short retval = 0;
    if (subset.find(u) == subset.end()) {
        if (!downloaded) { // we need to add to the page subset
            subset.insert(u, subsetCount); // mark it as used
            //printf("GLOBAL SUBSET ADDED %04x = %04x\n",u, subsetCount);
            retval = subsetCount;
            subsetCount++;
        } else if (page_subset.find(u) == page_subset.end()) {
            page_subset.insert(u, pageSubsetCount); // mark it as used
            //printf("PAGE SUBSET ADDED %04x = %04x\n",u, pageSubsetCount);
            retval = pageSubsetCount + (subsetCount/256 + 1) * 256;
            pageSubsetCount++;
        }
    } else {
        qWarning("QPSPrintEngineFont::internal error");
    }
    return retval;
}

void QPSPrintEngineFont::restore()
{
    page_subset.clear();
    pageSubsetCount = 0;
    //qDebug("restore for font %s\n",psname.latin1());
}

static inline const char *toHex(uchar u)
{
    static char hexVal[3];
    int i = 1;
    while (i >= 0) {
        ushort hex = (u & 0x000f);
        if (hex < 0x0a)
            hexVal[i] = '0'+hex;
        else
            hexVal[i] = 'A'+(hex-0x0a);
        u = u >> 4;
        i--;
    }
    hexVal[2] = '\0';
    return hexVal;
}

static const char *toHex(ushort u)
{
    static char hexVal[5];
    int i = 3;
    while (i >= 0) {
        ushort hex = (u & 0x000f);
        if (hex < 0x0a)
            hexVal[i] = '0'+hex;
        else
            hexVal[i] = 'A'+(hex-0x0a);
        u = u >> 4;
        i--;
    }
    hexVal[4] = '\0';
    return hexVal;
}

void QPSPrintEngineFont::drawText(QTextStream &stream, QPSPrintEnginePrivate *d, const QPointF &p, const QTextItemInt &ti)
{
    qreal x = p.x();
    qreal y = p.y();
    stream << y << " Y";

    int len = ti.num_chars;

    stream << "<";
    if (ti.flags & QTextItem::RightToLeft) {
        for (int i = len-1; i >=0; i--)
            stream << toHex(mapUnicode(ti.chars[i].unicode()));
    } else {
        for (int i = 0; i < len; i++)
            stream << toHex(mapUnicode(ti.chars[i].unicode()));
    }
    stream << ">";

    stream << ti.width << " " << x;

    if (ti.flags & QTextItem::Underline)
        stream << ' ' << y + d->currentFont->underlinePosition() + d->currentFont->lineThickness()
               << " " << d->currentFont->lineThickness() << " Tl";
    if (ti.flags & QTextItem::StrikeOut)
        stream << ' ' << y + d->currentFont->ascent()/3.
               << " " << d->currentFont->lineThickness() << " Tl";
    stream << " AT\n";

}


QString QPSPrintEngineFont::defineFont(QTextStream &stream, const QString &ps,
                                              const QString &key, QPSPrintEnginePrivate *ptr, int pixelSize)
{
    QString fontName;
    fontName.sprintf("/%s-Uni", ps.toLatin1().data());

    if (ptr->buffer) {
        ++ptr->headerFontNumber;
        ptr->fontStream << "/F" << ptr->headerFontNumber << " "
                      << pixelSize << fontName << " DF\n";
        fontName.sprintf("F%d", ptr->headerFontNumber);
        ptr->headerFontNames.insert(key, fontName);
    } else {
        ++ptr->pageFontNumber;
        stream << "/F" << ptr->pageFontNumber << " "
               << pixelSize << fontName << " DF\n";
        fontName.sprintf("F%d", ptr->pageFontNumber);
        ptr->pageFontNames.insert(key, fontName);
    }
    return fontName;
}

unsigned short QPSPrintEngineFont::mapUnicode(unsigned short unicode)
{
    QMap<unsigned short, unsigned short>::iterator res;
    res = subset.find(unicode);
    unsigned short offset = 0;
    bool found = false;
    if (res != subset.end()) {
        found = true;
    } else {
        if (downloaded) {
            res = page_subset.find(unicode);
            offset = (subsetCount/256 + 1) * 256;
            if (res != page_subset.end())
                found = true;
        }
    }
    if (!found) {
        return insertIntoSubset(unicode);
    }
    //qDebug("mapping unicode %x to %x", unicode, offset+*res);
    return offset + *res;
}

// This map is used for symbol fonts to get the correct glyph names for the latin range
static const unsigned short symbol_map[0x100] = {
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
    0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
    0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
    0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
    0x0020, 0x0021, 0x2200, 0x0023, 0x2203, 0x0025, 0x0026, 0x220b,
    0x0028, 0x0029, 0x2217, 0x002b, 0x002c, 0x2212, 0x002e, 0x002f,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
    0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,

    0x2245, 0x0391, 0x0392, 0x03a7, 0x0394, 0x0395, 0x03a6, 0x0393,
    0x0397, 0x0399, 0x03d1, 0x039a, 0x039b, 0x039c, 0x039d, 0x039f,
    0x03a0, 0x0398, 0x03a1, 0x03a3, 0x03a4, 0x03a5, 0x03c2, 0x03a9,
    0x039e, 0x03a8, 0x0396, 0x005b, 0x2234, 0x005d, 0x22a5, 0x005f,
    0xf8e5, 0x03b1, 0x03b2, 0x03c7, 0x03b4, 0x03b5, 0x03c6, 0x03b3,
    0x03b7, 0x03b9, 0x03d5, 0x03ba, 0x03bb, 0x03bc, 0x03bd, 0x03bf,
    0x03c0, 0x03b8, 0x03c1, 0x03c3, 0x03c4, 0x03c5, 0x03d6, 0x03c9,
    0x03be, 0x03c8, 0x03b6, 0x007b, 0x007c, 0x007d, 0x223c, 0x007f,

    0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087,
    0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
    0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097,
    0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f,
    0x20ac, 0x03d2, 0x2023, 0x2264, 0x2044, 0x221e, 0x0192, 0x2263,
    0x2666, 0x2665, 0x2660, 0x2194, 0x2190, 0x2191, 0x2192, 0x2193,
    0x00b0, 0x00b1, 0x2033, 0x2265, 0x00d7, 0x221d, 0x2202, 0x2022,
    0x00f7, 0x2260, 0x2261, 0x2248, 0x2026, 0xf8e6, 0xf8e7, 0x21b5,

    0x2135, 0x2111, 0x211c, 0x2118, 0x2297, 0x2295, 0x2205, 0x2229,
    0x222a, 0x2283, 0x2287, 0x2284, 0x2282, 0x2286, 0x2208, 0x2209,
    0x2220, 0x2207, 0xf6da, 0xf6d9, 0xf6db, 0x220f, 0x221a, 0x22c5,
    0x00ac, 0x2227, 0x2228, 0x21d4, 0x21d0, 0x21d1, 0x21d2, 0x21d3,
    0x25ca, 0x2329, 0xf8e8, 0xf8e9, 0xf8ea, 0x2211, 0xf8eb, 0xf8ec,
    0xf8ed, 0xf8ee, 0xf8ef, 0xf8f0, 0xf8f1, 0xf8f2, 0xf8f3, 0xf8f4,
    0x0000, 0x232a, 0x222b, 0x2320, 0xf8f5, 0x2321, 0xf8f6, 0xf8f7,
    0xf8f8, 0xf8f9, 0xf8fa, 0xf8fb, 0xf8fc, 0xf8fd, 0xf8fe, 0x0000,
};

QString QPSPrintEngineFont::glyphName(unsigned short glyphindex, bool *glyphSet)
{
    QString glyphname;
    int l = 0;
    unsigned short unicode = unicode_for_glyph(glyphindex);
    if (symbol && unicode < 0x100) {
        // map from latin1 to symbol
        unicode = symbol_map[unicode];
    }
    if (!unicode && glyphindex) {
        glyphname = "gl";
        glyphname += toHex(glyphindex);
    } else {
        while(unicodetoglyph[l].u < unicode)
            l++;
        if (unicodetoglyph[l].u == unicode) {
            glyphname = agl + unicodetoglyph[l].index;
            if (glyphSet) {
                int other = 0;
                switch (unicode) {
                    // some glyph names are duplicate in postscript. Make sure we give the
                    // duplicate a different name to avoid infinite recursion
                case 0x0394:
                    other = 0x2206;
                    break;
                case 0x03a9:
                    other = 0x2126;
                    break;
                case 0x0162:
                    other = 0x021a;
                    break;
                case 0x2215:
                    other = 0x2044;
                    break;
                case 0x00ad:
                    other = 0x002d;
                    break;
                case 0x02c9:
                    other = 0x00af;
                    break;
                case 0x03bc:
                    other = 0x00b5;
                    break;
                case 0x2219:
                    other = 0x00b7;
                    break;
                case 0x00a0:
                    other = 0x0020;
                    break;
                case 0x0163:
                    other = 0x021b;
                    break;
                default:
                    break;
                }
                if (other) {
                    int oglyph = glyph_for_unicode(other);
                    if(oglyph && oglyph != glyphindex && glyphSet[oglyph]) {
                        glyphname = "uni";
                        glyphname += toHex(unicode);
                    }
                }
            }
        } else {
            glyphname = "uni";
            glyphname += toHex(unicode);
        }
    }
    return glyphname;
}

void QPSPrintEngineFont::download(QTextStream &s, bool global)
{
    //printf("defining mapping for printer font %s\n",psname.latin1());
    downloadMapping(s, global);
}

void QPSPrintEngineFont::downloadMapping(QTextStream &s, bool global)
{
    uchar rangeOffset = 0;
    uchar numRanges = (uchar)(subsetCount/256 + 1);
    uchar range;
    QMap<unsigned short, unsigned short> *subsetDict = &subset;
    if (!global) {
        rangeOffset = numRanges;
        numRanges = pageSubsetCount/256 + 1;
        subsetDict = &page_subset;
    }
    // build up inverse table
    unsigned short *inverse = new unsigned short[numRanges * 256];
    memset(inverse, 0, numRanges * 256 * sizeof(unsigned short));

    QMap<unsigned short, unsigned short>::iterator it;
    for (it = subsetDict->begin(); it != subsetDict->end(); ++it) {
        const unsigned short &mapped = *it;
        inverse[mapped] = it.key();
    }

    QString vector;
    QString glyphname;

    for (range=0; range < numRanges; range++) {
        //printf("outputting range %04x\n",range*256);
        vector = "%% Font Page ";
        vector += toHex((uchar)(range + rangeOffset));
        vector += "\n/";
        vector += psname;
        vector += "-ENC-";
        vector += toHex((uchar)(range + rangeOffset));
        vector += " [\n";

        QString line;
        for(int k=0; k<256; k++) {
            int c = range*256 + k;
            unsigned short glyph = inverse[c];
            glyphname = glyphName(glyph);
            if (line.length() + glyphname.length() > 76) {
                vector += line;
                vector += "\n";
                line = "";
            }
            line += "/" + glyphname;
        }
        vector += line;
        vector += "] def\n";
        s << vector;
    }

    delete [] inverse;

    // DEFINE BASE FONTS

    for (range=0; range < numRanges; range++) {
        s << "/"
          << psname
          << "-Uni-"
          << toHex((uchar)(range + rangeOffset))
          << " "
          << psname
          << "-ENC-"
          << toHex((uchar)(range + rangeOffset));
        if (embedded()) {
            s << " /"
              << psname
              << " MFEmb\n";
        } else {
            s << " " << psname << "List"
              << " MF\n";
        }
    }

    // === write header ===
    //   int VMMin;
    //   int VMMax;

    s << wrapDSC("%%BeginFont: " + psname)
      << "%!PS-AdobeFont-1.0 Composite Font\n"
      << wrapDSC("%%FontName: " + psname + "-Uni")
      << "%%Creator: Composite font created by Qt\n";

    /* Start the dictionary which will eventually */
    /* become the font. */
    s << "25 dict begin\n"
        "/FontName /"
      << psname
      << "-Uni def\n"
        "/PaintType 0 def\n";

    // This is concatenated with the base fonts, so it should perform
    // no transformation. Sivan
    s << "/FontMatrix[1 0 0 1 0 0]def\n"
        "/FontType 0 def\n"

        // now come composite font structures
        // FMapTypes:
        // 2: 8/8, 8 bits select the font, 8 the glyph

        "/FMapType 2 def\n"

        // The encoding in a composite font is used for indirection.
        // Every char is split into a font-number and a character-selector.
        // PostScript prints glyph number character-selector from the font
        // FDepVector[Encoding[font-number]].

        "/Encoding [";
    for (range=0; range < rangeOffset + numRanges; range++) {
        if (range % 16 == 0)
            s << "\n";
        else
            s << " ";
        s << range;
    }
    s << "]def\n"

        // Descendent fonts

        "/FDepVector [\n";
    for (range=0; range < rangeOffset + numRanges; range++) {
        s << "/"
          << psname
          << "-Uni-"
          << toHex(range)
          << " findfont\n";
    }
    s << "]def\n"

        // === trailer ===

        "FontName currentdict end definefont pop\n"
        "%%EndFont\n";
}

#ifdef QT_HAVE_FREETYPE

class QPSPrintEngineFontFT : public QPSPrintEngineFont {
public:
    QPSPrintEngineFontFT(QFontEngine *f);
    virtual void download(QTextStream& s, bool global);
    virtual void drawText(QTextStream &stream, QPSPrintEnginePrivate *d, const QPointF &p, const QTextItemInt &ti);
    ~QPSPrintEngineFontFT();

    virtual bool embedded() { return true; }
private:
    FT_Face face;
    QVector<ushort> glyph2uni; // to speed up lookups

    void uni2glyphSetup();
    void charproc(int glyph, QTextStream& s);
    unsigned short unicode_for_glyph(int glyphindex);
    unsigned short glyph_for_unicode(unsigned short unicode);
};

static FT_Face ft_face(QFontEngine *engine)
{
    if (engine->type() == QFontEngine::Multi) {
        // #### HACK
        QFontEngineMulti *mf = static_cast<QFontEngineMulti *>(engine);
        engine = mf->engine(0);
    }
#ifdef Q_WS_X11
    if (engine->type() == QFontEngine::Freetype) {
        QFontEngineFT *ft = static_cast<QFontEngineFT *>(engine);
        return ft->non_locked_face();
    }
#endif
#ifdef Q_WS_QWS
    if (engine->type() == QFontEngine::Freetype) {
        QFontEngineFT *ft = static_cast<QFontEngineFT *>(engine);
        return ft->face;
    }
#endif
    return 0;
}

QPSPrintEngineFontFT::QPSPrintEngineFontFT(QFontEngine *f)
    : QPSPrintEngineFont(f)
{
    face = ft_face(f);
    Q_ASSERT(face);

    psname = FT_Get_Postscript_Name(face);
}

void QPSPrintEngineFontFT::download(QTextStream& s, bool global)
{
    //qDebug("downloading ttf font %s", psname.latin1());
    //qDebug("target type=%d", target_type);
    global_dict = global;
    QMap<unsigned short, unsigned short> *subsetDict = &subset;
    if (!global)
        subsetDict = &page_subset;

    downloaded  = true;

    emitPSFontNameList(s, psname, replacementList);

    // === write header ===

#if 0
    s << wrapDSC("%%BeginFont: " + FullName);
#endif
    s << "%!PS-Adobe-3.0 Resource-Font\n";

#if 0
    if(!Copyright.isEmpty()) {
        s << wrapDSC("%%Copyright: " + Copyright);
    }
#endif

    s << "%%Creator: Converted from TrueType by Qt\n";

#if 0
    /* If VM usage information is available, print it. */
    if(target_type == 42 && post_table)
    {
        int VMMin = (int)getULONG(post_table + 16);
        int VMMax = (int)getULONG(post_table + 20);
        if(VMMin > 0 && VMMax > 0)
            s << "%%VMUsage: " << VMMin << " " << VMMax << "\n";
    }
#endif

    /* Start the dictionary which will eventually */
    /* become the font. */
    s << "25 dict begin\n"

        "/_m{moveto}D\n"
        "/_l{lineto}D\n"
        "/_cl{closepath eofill}D\n"
        "/_c{curveto}D\n"
        "/_sc{7 -1 roll{setcachedevice}{pop pop pop pop pop pop}ifelse}D\n"
        "/_e{exec}D\n"

        "/FontName /"
      << psname
      << " d\n"
        "/PaintType 0 d\n"
        "/FontMatrix[" << 1./face->units_per_EM << " 0 0 " << 1./face->units_per_EM << " 0 0]d\n"

        "/FontBBox["
      << face->bbox.xMin
      << " "
      << face->bbox.yMin
      << " "
      << face->bbox.xMax
      << " "
      << face->bbox.yMax
      << "]d\n"

        "/FontType 3 d\n"

    // === write encoding ===

        "/Encoding StandardEncoding d\n";

    // === write fontinfo dict ===

#if 0
    /* We create a sub dictionary named "FontInfo" where we */
    /* store information which though it is not used by the */
    /* interpreter, is useful to some programs which will */
    /* be printing with the font. */
    s << "/FontInfo <<\n";

    /* These names come from the TrueType font's "name" table. */
    s << "/FamilyName (";
    s << FamilyName;
    s << ") def\n";

    s << "/FullName (";
    s << FullName;
    s << ") def\n";

    s << "/Notice (";
    s << Copyright;
    s << " ";
    s << Trademark;
    s << ") def\n";

    /* This information is not quite correct. */
    s << "/Weight (";
    s << Style;
    s << ") def\n";

    /* Some fonts have this as "version". */
    s << "/Version (";
    s << Version;
    s << ") def\n";

    /* Some information from the "post" table. */
    if (post_table) {
        Fixed ItalicAngle = getFixed(post_table + 4);
        s << "/ItalicAngle ";
        s << ItalicAngle.whole;
        s << ".";
        s << ItalicAngle.fraction;
        s << " def\n";

        s << "/isFixedPitch ";
        s << (getULONG(post_table + 12) ? "true" : "false");
        s << " def\n";

        s << "/UnderlinePosition ";
        s << (int)getFWord(post_table + 8);
        s << " def\n";

        s << "/UnderlineThickness ";
        s << (int)getFWord(post_table + 10);
        s << " def\n";
    }
    s << ">> readonly d\n";
#endif

    bool glyphset[65536];
    for(int c=0; c < 65536; c++)
        glyphset[c] = false;
    glyphset[0] = true; // always output .notdef

    QMap<unsigned short, unsigned short>::iterator it;
    for(it = subsetDict->begin(); it != subsetDict->end(); ++it)
        glyphset[it.key()] = true;
    int nGlyphs = 0;
    for(int c=0; c < 65536; c++)
        if (glyphset[c]) nGlyphs++;

    s << "/CharStrings ";
    s << nGlyphs;
    s << " dict dup begin\n";

    // Emmit one key-value pair for each glyph.
    for(int x=0; x < 65536; x++) {
        if (!glyphset[x]) continue;

        //qDebug("emitting charproc for glyph %d, name=%s", x, glyphName(x).latin1());
        s << "/";
        s << glyphName(x);
        s << "{";
        charproc(x, s);
        s << "}def\n";     /* "} bind def" */
    }

    s << "end readonly def\n"

        // === trailer ===

        "\n"

        "/BuildGlyph\n"
        " {exch begin\n"              /* start font dictionary */
        " CharStrings exch\n"
        " 2 copy known not{pop /.notdef}if\n"
        " true 3 1 roll get exec\n"
        " end}d\n"

        "\n"

//     /* This procedure is for compatibility with */
//     /* level 1 interpreters. */
//      "/BuildChar {\n";
//      " 1 index /Encoding get exch get\n";
//     s << " 1 index /BuildGlyph get exec\n";
//     s << "}_d\n";

        "\n";

    s << "FontName currentdict end definefont pop\n";

    downloadMapping(s, global);
    s << "%%EndFont\n";
}

void QPSPrintEngineFontFT::drawText(QTextStream &stream, QPSPrintEnginePrivate *d, const QPointF &p, const QTextItemInt &ti)
{
    qreal x = p.x();
    qreal y = p.y();
    stream << y << " Y";

    QByteArray xyarray;
    qreal xo = 0;
    qreal yo = 0;

    QGlyphLayout *glyphs = ti.glyphs;

    int len;
    len = ti.num_glyphs;

    stream << "<";
    if (ti.flags & QTextItem::RightToLeft) {
        for (int i = len-1; i >=0; i--) {
            // map unicode is not really the correct name, as we map glyphs, but we also download glyphs, so this works
            unsigned short glyph;
            glyph = glyphs[i].glyph;
            stream << toHex(mapUnicode(glyph));
            if (i != len-1) {
                xyarray += QByteArray::number(xo + glyphs[i].offset.x() + glyphs[i+1].advance.x());
                xyarray += " ";
                xyarray += QByteArray::number(yo + glyphs[i].offset.y());
                xyarray += " ";
                xo = -glyphs[i].offset.x();
                yo = -glyphs[i].offset.y();
            }
        }
    } else {
        for (int i = 0; i < len; i++) {
            // map unicode is not really the correct name, as we map glyphs, but we also download glyphs, so this works
            unsigned short glyph;
            glyph = glyphs[i].glyph;
            stream << toHex(mapUnicode(glyph));
            if (i) {
                xyarray += QByteArray::number(xo + glyphs[i].offset.x() + glyphs[i-1].advance.x());
                xyarray += " ";
                xyarray += QByteArray::number(yo + glyphs[i].offset.y());
                xyarray += " ";
                xo = -glyphs[i].offset.x();
                yo = -glyphs[i].offset.y();
            }
        }
    }
    stream << ">";

    stream << "[" << xyarray << "0 0]"
           << ti.width << " " << x;

    if (ti.flags & QTextItem::Underline)
        stream << ' ' << y + d->currentFont->underlinePosition() + d->currentFont->lineThickness()
               << " " << d->currentFont->lineThickness() << " Tl";
    if (ti.flags & QTextItem::StrikeOut)
        stream << ' ' << y + d->currentFont->ascent()/3.
               << " " << d->currentFont->lineThickness() << " Tl";
    stream << " XYT\n";

}

QPSPrintEngineFontFT::~QPSPrintEngineFontFT()
{
}

void QPSPrintEngineFontFT::uni2glyphSetup()
{
    glyph2uni.resize(face->num_glyphs);
    for (int i = 0; i < face->num_glyphs; ++i)
        glyph2uni[i] = 0;

    for (int i = 0; i < 0xffff; ++i) {
        int g = FT_Get_Char_Index(face, i);
        if (g != 0)
            glyph2uni[g] = i;
    }
}

unsigned short QPSPrintEngineFontFT::unicode_for_glyph(int glyphindex)
{
    if (glyph2uni.isEmpty())
        uni2glyphSetup();
    return glyph2uni[glyphindex];
}

unsigned short QPSPrintEngineFontFT::glyph_for_unicode(unsigned short unicode)
{
    return FT_Get_Char_Index(face, unicode);
}

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_IMAGE_H
#include FT_BBOX_H

struct decompose_data {
    qreal x;
    qreal y;
    QTextStream *s;
};

static int move_to(FT_Vector *to, void *data)
{
    decompose_data *d = (decompose_data *)data;
    *d->s << to->x
          << " "
          << to->y
          << " _m\n";

    d->x = to->x;
    d->y = to->y;
    return 0;
}

static int line_to(FT_Vector *to, void *data)
{
    decompose_data *d = (decompose_data *)data;
    *d->s << to->x
          << " "
          << to->y
          << " _l\n";

    d->x = to->x;
    d->y = to->y;
    return 0;
}

static int conic_to(FT_Vector *control, FT_Vector *to, void *data)
{
    decompose_data *d = (decompose_data *)data;
    double cx[2], cy[2];

    cx[0] = (2*control->x + d->x)/3.;
    cy[0] = (2*control->y + d->y)/3.;
    cx[1] = (to->x + 2*control->x)/3.;
    cy[1] = (to->y + 2*control->y)/3.;

    *d->s << cx[0]
          << " "
          << cy[0]
          << " "
          << cx[1]
          << " "
          << cy[1]
          << " "
          << to->x
          << " "
          << to->y
          << " _c\n";

    d->x = to->x;
    d->y = to->y;
    return 0;
}

static int cubic_to(FT_Vector *control1, FT_Vector *control2, FT_Vector *to, void *data)
{
    decompose_data *d = (decompose_data *)data;

    *d->s << control1->x
          << " "
          << control1->y
          << " "
          << control2->x
          << " "
          << control2->y
          << " "
          << to->x
          << " "
          << to->y
          << " _c\n";

    d->x = to->x;
    d->y = to->y;
    return 0;
}



void QPSPrintEngineFontFT::charproc(int glyph, QTextStream& s)
{
    FT_Load_Glyph(face, glyph, FT_LOAD_NO_SCALE);

#ifdef DEBUG_TRUETYPE
    s << "% tt_type3_charproc for ";
    s << glyph;
    s << "\n";
#endif

    FT_Outline_Funcs outline_funcs;
    outline_funcs.move_to = move_to;
    outline_funcs.line_to = line_to;
    outline_funcs.conic_to = conic_to;
    outline_funcs.cubic_to = cubic_to;
    outline_funcs.shift = 0;
    outline_funcs.delta = 0;

    decompose_data d;
    d.x = 0;
    d.y = 0;
    d.s = &s;

    FT_BBox bounds;
    FT_Outline_Get_BBox(&face->glyph->outline, &bounds);

    s << face->glyph->metrics.horiAdvance
      << " "
      << 0
      << " "
      << bounds.xMin
      << " "
      << bounds.yMin
      << " "
      << bounds.xMax
      << " "
      << bounds.yMax
      << " _sc\n";

    FT_Outline_Decompose(&face->glyph->outline, &outline_funcs, &d);
    s << "_cl";
}
#endif

// ================== AFontFileNotFound ============



class QPSPrintEngineFontNotFound
    : public QPSPrintEngineFont {
public:
    QPSPrintEngineFontNotFound(QFontEngine* f);
    virtual void    download(QTextStream& s, bool global);
};

QPSPrintEngineFontNotFound::QPSPrintEngineFontNotFound(QFontEngine* f)
    : QPSPrintEngineFont(f)
{
    psname = makePSFontName(f);
    replacementList = makePSFontNameList(f);
}

void QPSPrintEngineFontNotFound::download(QTextStream& s, bool)
{
    //qDebug("downloading not found font %s", psname.latin1());
    emitPSFontNameList(s, psname, replacementList);
    s << "% No embeddable font for "
      << psname
      << " found\n";
    QPSPrintEngineFont::download(s, true);
}

#ifndef QT_NO_TEXTCODEC
// =================== A font file for asian ============

class QPSPrintEngineFontAsian
    : public QPSPrintEngineFont {
public:
    QPSPrintEngineFontAsian(QFontEngine *f)
        : QPSPrintEngineFont(f), codec(0) {}
    void download(QTextStream& s, bool global);
    QString defineFont(QTextStream &stream, const QString &ps, const QString &key,
                        QPSPrintEnginePrivate *d, int pixelSize);
    void drawText(QTextStream &stream, QPSPrintEnginePrivate *d, const QPointF &p, const QTextItemInt &ti);

    QString makePSFontName(const QFontEngine *f, int type) const;
    virtual QString extension() const = 0;

    QTextCodec *codec;
};

QString QPSPrintEngineFontAsian::makePSFontName(const QFontEngine *f, int type) const
{
    QString ps;
    int i;

    QString family = f->fontDef.family.toLower();

    // try to make a "good" postscript name
    ps = family.simplified();
    i = 0;
    while(i < ps.length()) {
        if (i != 0 && ps[i] == '[') {
            if (ps[i-1] == ' ')
                ps.truncate (i-1);
            else
                ps.truncate (i);
            break;
        }
        if (i == 0 || ps[i-1] == ' ') {
            ps[i] = ps[i].toUpper();
            if (i)
                ps.remove(i-1, 1);
            else
                i++;
        } else {
            i++;
        }
    }

    switch (type) {
    case 1:
        ps.append(QLatin1String("-Italic"));
        break;
    case 2:
        ps.append(QLatin1String("-Bold"));
        break;
    case 3:
        ps.append(QLatin1String("-BoldItalic"));
        break;
    case 0:
    default:
        break;
    }

    ps += extension();

    return ps;
}


QString QPSPrintEngineFontAsian::defineFont(QTextStream &stream, const QString &ps,
                                         const QString &key, QPSPrintEnginePrivate *d, int pixelSize)
{
    QString fontName;
    QString fontName2;

    QString tmp = d->headerFontNames.value(ps, QString::null);

    if (d->buffer) {
        if (!tmp.isNull()) {
            fontName = tmp;
        } else {
            fontName.sprintf("F%d", ++d->headerFontNumber);
            d->fontStream << "/" << fontName << " false " << ps << "List MF\n";
            d->headerFontNames.insert(ps, fontName);
        }
        fontName2.sprintf("F%d", ++d->headerFontNumber);
        d->fontStream << "/" << fontName2 << " "
                      << pixelSize << "/" << fontName << " DF\n";
        d->headerFontNames.insert(key, fontName2);
    } else {
        if (!tmp.isNull()) {
            fontName = tmp;
        } else {
            fontName.sprintf("F%d", ++d->pageFontNumber);
            stream << "/" << fontName << " false " << ps << "List MF\n";
            d->pageFontNames.insert(ps, fontName);
        }
        fontName2.sprintf("F%d", ++d->pageFontNumber);
        stream << "/" << fontName2 << " "
               << pixelSize << "/" << fontName << " DF\n";
        d->pageFontNames.insert(key, fontName2);
    }
    return fontName2;
}


void QPSPrintEngineFontAsian::download(QTextStream& s, bool)
{
    //qDebug("downloading asian font %s", psname.latin1());
    s << "% Asian postscript font requested. Using "
      << psname << endl;
    emitPSFontNameList(s, psname, replacementList);
}


void QPSPrintEngineFontAsian::drawText(QTextStream &, QPSPrintEnginePrivate *,
                                       const QPointF &, const QTextItemInt &)
{
    // ###
#if 0
    int len = engine->length(item);
    QScriptItem &si = engine->items[item];

    int x = p.x();
    int y = p.y();
    stream << y << " Y";

    QString mdf;
    if (ps->font.underline())
        mdf += " " + QString().setNum(y + d->fm.underlinePos() + d->fm.lineWidth()) +
               " " + toString(d->fm.lineWidth()) + " Tl";
    if (ps->font.strikeOut())
        mdf += " " + QString().setNum(y + d->fm.strikeOutPos()) +
               " " + toString(d->fm.lineWidth()) + " Tl";
    QByteArray mb;
    QByteArray out;
    QString dummy(QChar(0x20));

    if (si.analysis.bidiLevel % 2) {
        for (int i = len-1; i >= 0; i--) {
            QChar ch = text.unicode()[i];
            if (!ch.row()) {
                ; // ignore, we should never get here anyway
            } else {
                if (codec) {
                    dummy[0] = ch;
                    mb = codec->fromUnicode(dummy);
                } else
                    mb = "  ";

                for (int j = 0; j < mb.size (); j++) {
                    if (mb.at(j) == '(' || mb.at(j) == ')' || mb.at(j) == '\\')
                        out += "\\";
                    out += mb.at(j);
                }
            }
        }
    } else {
        for (int i = 0; i < len; i++) {
            QChar ch = text.unicode()[i];
            if (!ch.row()) {
                ; // ignore, we should never get here anyway
            } else {
                if (codec) {
                    dummy[0] = ch;
                    mb = codec->fromUnicode(dummy);
                } else
                    mb = "  ";

                for (int j = 0; j < mb.size(); j++) {
                    if (mb.at(j) == '(' || mb.at(j) == ')' || mb.at(j) == '\\')
                        out += "\\";
                    out += mb.at(j);
                }
            }
        }
    }
    stream << "(" << out << ")" << si.width.toDouble() << " " << x << mdf << " AT\n";
#endif
}

// ----------- Japanese --------------

static const psfont Japanese1 [] = {
    { "Ryumin-Light-H", 0, 100. },
    { "Ryumin-Light-H", 0.2, 100. },
    { "GothicBBB-Medium-H", 0, 100. },
    { "GothicBBB-Medium-H", 0.2, 100. }
};

static const psfont Japanese1a [] = {
    { "GothicBBB-Medium-H", 0, 100. },
    { "GothicBBB-Medium-H", 0.2, 100. },
    { "Ryumin-Light-H", 0, 100. },
    { "Ryumin-Light-H", 0.2, 100. }
};

static const psfont Japanese2 [] = {
    { "GothicBBB-Medium-H", 0, 100. },
    { "GothicBBB-Medium-H", 0.2, 100. },
    { "GothicBBB-Medium-H", 0, 100. },
    { "GothicBBB-Medium-H", 0.2, 100. }
};

static const psfont Japanese2a [] = {
    { "Ryumin-Light-H", 0, 100. },
    { "Ryumin-Light-H", 0.2, 100. },
    { "Ryumin-Light-H", 0, 100. },
    { "Ryumin-Light-H", 0.2, 100. }
};


// Wadalab fonts

static const psfont WadaMin [] = {
    { "WadaMin-Regular-H", 0, 100. },
    { "WadaMin-Regular-H", 0.2, 100. },
    { "WadaMin-Bold-H", 0, 100. },
    { "WadaMin-Bold-H", 0.2, 100. }
};

static const psfont WadaGo [] = {
    { "WadaMaruGo-Regular-H", 0, 100. },
    { "WadaMaruGo-Regular-H", 0.2, 100. },
    { "WadaGo-Bold-H", 0, 100. },
    { "WadaGo-Bold-H", 0.2, 100. }
};

// Adobe Wadalab

static const psfont WadaGoAdobe [] = {
    { "WadaMaruGo-RegularH-Hojo-H", 0, 100. },
    { "WadaMaruGo-RegularH-Hojo-H", 0.2, 100. },
    { "WadaMaruGo-RegularH-Hojo-H", 0, 100. },
    { "WadaMaruGo-RegularH-Hojo-H", 0.2, 100. },
};
static const psfont WadaMinAdobe [] = {
    { "WadaMin-RegularH-Hojo-H", 0, 100. },
    { "WadaMin-RegularH-Hojo-H", 0.2, 100. },
    { "WadaMin-RegularH-Hojo-H", 0, 100. },
    { "WadaMin-RegularH-Hojo-H", 0.2, 100. },
};


static const psfont * const Japanese1Replacements[] = {
    Japanese1, Japanese1a, WadaMin, WadaGo, WadaMinAdobe, WadaGoAdobe, 0
};
static const psfont * const Japanese2Replacements[] = {
    Japanese2, Japanese2a, WadaMin, WadaGo, WadaMinAdobe, WadaGoAdobe, 0
};

class QPSPrintEngineFontJapanese
  : public QPSPrintEngineFontAsian {
public:
      QPSPrintEngineFontJapanese(QFontEngine* f);
      virtual QString extension() const;
};

QPSPrintEngineFontJapanese::QPSPrintEngineFontJapanese(QFontEngine* f)
    : QPSPrintEngineFontAsian(f)
{
    codec = QTextCodec::codecForMib(63); // jisx0208.1983-0

    int type = getPsFontType(f);
    psname = makePSFontName(f, type);
    QString best = "[/" + psname + " 1.0 0.0]";
    replacementList.append(best);

    const psfont *const *replacements = (psname.contains("Helvetica") ? Japanese2Replacements : Japanese1Replacements);
    appendReplacements(replacementList, replacements, type);
}

QString QPSPrintEngineFontJapanese::extension() const
{
    return "-H";
}

// ----------- Korean --------------

// sans serif
static const psfont SMGothic [] = {
    { "SMGothic-Medium-KSC-EUC-H", 0, 100. },
    { "SMGothic-Medium-KSC-EUC-H", 0.2, 100. },
    { "SMGothic-DemiBold-KSC-EUC-H", 0, 100. },
    { "SMGothic-DemiBold-KSC-EUC-H", 0.2, 100. }
};

// serif
static const psfont MKai [] = {
    { "MingMT-Light-KSC-EUC-H", 0, 100. },
    { "MingMT-Light-KSC-EUC-H", 0.2, 100. },
    { "MKai-Medium-KSC-EUC-H", 0, 100. },
    { "MKai-Medium-KSC-EUC-H", 0.2, 100. },
};


static const psfont Munhwa [] = {
    { "Munhwa-Regular-KSC-EUC-H", 0, 100. },
    { "Munhwa-Regular-KSC-EUC-H", 0.2, 100. },
    { "Munhwa-Bold-KSC-EUC-H", 0, 100. },
    { "Munhwa-Bold-KSC-EUC-H", 0.2, 100. }
};

static const psfont MunhwaGothic [] = {
    { "MunhwaGothic-Regular-KSC-EUC-H", 0, 100. },
    { "MunhwaGothic-Regular-KSC-EUC-H", 0.2, 100. },
    { "MunhwaGothic-Bold-KSC-EUC-H", 0, 100. },
    { "MunhwaGothic-Bold-KSC-EUC-H", 0.2, 100. }
};

static const psfont MunhwaGungSeo [] = {
    { "MunhwaGungSeo-Light-KSC-EUC-H", 0, 100. },
    { "MunhwaGungSeo-Light-KSC-EUC-H", 0.2, 100. },
    { "MunhwaGungSeo-Bold-KSC-EUC-H", 0, 100. },
    { "MunhwaGungSeo-Bold-KSC-EUC-H", 0.2, 100. }
};

static const psfont MunhwaGungSeoHeulim [] = {
    { "MunhwaGungSeoHeulim-Light-KSC-EUC-H", 0, 100. },
    { "MunhwaGungSeoHeulim-Light-KSC-EUC-H", 0.2, 100. },
    { "MunhwaGungSeoHeulim-Bold-KSC-EUC-H", 0, 100. },
    { "MunhwaGungSeoHeulim-Bold-KSC-EUC-H", 0.2, 100. }
};

static const psfont MunhwaHoonMin [] = {
    { "MunhwaHoonMin-Regular-KSC-EUC-H", 0, 100. },
    { "MunhwaHoonMin-Regular-KSC-EUC-H", 0.2, 100. },
    { "MunhwaHoonMin-Regular-KSC-EUC-H", 0, 100. },
    { "MunhwaHoonMin-Regular-KSC-EUC-H", 0.2, 100. }
};

static const psfont BaekmukGulim [] = {
    { "Baekmuk-Gulim-KSC-EUC-H", 0, 100. },
    { "Baekmuk-Gulim-KSC-EUC-H", 0.2, 100. },
    { "Baekmuk-Gulim-KSC-EUC-H", 0, 100. },
    { "Baekmuk-Gulim-KSC-EUC-H", 0.2, 100. }
};



static const psfont * const KoreanReplacements[] = {
    BaekmukGulim, SMGothic, Munhwa, MunhwaGothic, MKai, MunhwaGungSeo,
    MunhwaGungSeoHeulim, MunhwaHoonMin, Helvetica, 0
};

class QPSPrintEngineFontKorean
    : public QPSPrintEngineFontAsian {
public:
    QPSPrintEngineFontKorean(QFontEngine* f);
    QString extension() const;
};

QPSPrintEngineFontKorean::QPSPrintEngineFontKorean(QFontEngine* f)
    : QPSPrintEngineFontAsian(f)
{
    codec = QTextCodec::codecForMib(38); // eucKR
    int type = getPsFontType(f);
    psname = makePSFontName(f, type);
    QString best = "[/" + psname + " 1.0 0.0]";
    replacementList.append(best);
    appendReplacements(replacementList, KoreanReplacements, type);
}

QString QPSPrintEngineFontKorean::extension() const
{
    return "-KSC-EUC-H";
}
// ----------- traditional chinese ------------

// Arphic Public License Big5 TrueType fonts (on Debian and CLE and others)
static const psfont ShanHeiSun [] = {
    { "ShanHeiSun-Light-ETen-B5-H", 0, 100. },
    { "ShanHeiSun-Light-ETen-B5-H", 0.2, 100. },
    { "ShanHeiSun-Light-ETen-B5-H", 0, 100. },
    { "ShanHeiSun-Light-ETen-B5-H", 0.2, 100. },
};
static const psfont ZenKai [] = {
    { "ZenKai-Medium-ETen-B5-H", 0, 100. },
    { "ZenKai-Medium-Italic-ETen-B5-H", 0.2, 100. },
    { "ZenKai-Medium-Bold-ETen-B5-H", 0, 100. },
    { "ZenKai-Medium-BoldItalic-ETen-B5-H", 0.2, 100. },
};

// Fonts on Turbolinux
static const psfont SongB5 [] = {
    { "B5-MSung-Light-ETen-B5-H", 0, 100. },
    { "B5-MSung-Italic-ETen-B5-H", 0, 100. },
    { "B5-MSung-Bold-ETen-B5-H", 0, 100. },
    { "B5-MSung-BoldItalic-ETen-B5-H", 0, 100. },
};
static const psfont KaiB5 [] = {
    { "B5-MKai-Medium-ETen-B5-H", 0, 100. },
    { "B5-MKai-Italic-ETen-B5-H", 0, 100. },
    { "B5-MKai-Bold-ETen-B5-H", 0, 100. },
    { "B5-MKai-BoldItalic-ETen-B5-H", 0, 100. },
};
static const psfont HeiB5 [] = {
    { "B5-MHei-Medium-ETen-B5-H", 0, 100. },
    { "B5-MHei-Italic-ETen-B5-H", 0, 100. },
    { "B5-MHei-Bold-ETen-B5-H", 0, 100. },
    { "B5-MHei-BoldItalic-ETen-B5-H", 0, 100. },
};
static const psfont FangSongB5 [] = {
    { "B5-CFangSong-Light-ETen-B5-H", 0, 100. },
    { "B5-CFangSong-Italic-ETen-B5-H", 0, 100. },
    { "B5-CFangSong-Bold-ETen-B5-H", 0, 100. },
    { "B5-CFangSong-BoldItalic-ETen-B5-H", 0, 100. },
};

// Arphic fonts on Thiz Linux
static const psfont LinGothic [] = {
    { "LinGothic-Light-ETen-B5-H", 0, 100. },
    { "LinGothic-Light-Italic-ETen-B5-H", 0.2, 100. },
    { "LinGothic-Light-Bold-ETen-B5-H", 0, 100. },
    { "LinGothic-Light-BoldItalic-ETen-B5-H", 0.2, 100. },
};
static const psfont YenRound [] = {
    { "YenRound-Light-ETen-B5-H", 0, 100. },
    { "YenRound-Light-Italic-ETen-B5-H", 0.2, 100. },
    { "YenRound-Light-Bold-ETen-B5-H", 0, 100. },
    { "YenRound-Light-BoldItalic-ETen-B5-H", 0.2, 100. },
};

static const psfont MingB5 [] = {
    { "Ming-Light-ETen-B5-H", 0, 100. },
    { "Ming-Light-Italic-ETen-B5-H", 0.2, 100. },
    { "Ming-Light-Bold-ETen-B5-H", 0, 100. },
    { "Ming-Light-BoldItalic-ETen-B5-H", 0.2, 100. },
};

// Microsoft's Ming/Sung font?
static const psfont MSung [] = {
    { "MSung-Light-ETenms-B5-H", 0, 100. },
    { "MSung-Light-ETenms-B5-H", 0.2, 100. },
    { "MSung-Light-ETenms-B5-H", 0, 100. },
    { "MSung-Light-ETenms-B5-H", 0.2, 100. },
};
// "Standard Sung/Ming" font by Taiwan Ministry of Education
static const psfont MOESung [] = {
    { "MOESung-Regular-B5-H", 0, 100. },
    { "MOESung-Regular-B5-H", 0.2, 100. },
    { "MOESung-Regular-B5-H", 0, 100. },
    { "MOESung-Regular-B5-H", 0.2, 100. },
};

static const psfont MOEKai [] = {
    { "MOEKai-Regular-B5-H", 0, 100. },
    { "MOEKai-Regular-B5-H", 0.2, 100. },
    { "MOEKai-Regular-B5-H", 0, 100. },
    { "MOEKai-Regular-B5-H", 0.2, 100. },
};

static const psfont * const TraditionalReplacements[] = {
    MOESung, SongB5, ShanHeiSun, MingB5, MSung, FangSongB5, KaiB5, ZenKai, HeiB5,
    LinGothic, YenRound, MOEKai, Helvetica, 0
        };

class QPSPrintEngineFontTraditionalChinese
  : public QPSPrintEngineFontAsian {
public:
      QPSPrintEngineFontTraditionalChinese(QFontEngine* f);
      QString extension() const;
};

QPSPrintEngineFontTraditionalChinese::QPSPrintEngineFontTraditionalChinese(QFontEngine* f)
    : QPSPrintEngineFontAsian(f)
{
    codec = QTextCodec::codecForMib(2026); // Big5-0
    int type = getPsFontType(f);
    psname = makePSFontName(f, type);
    QString best = "[/" + psname + " 1.0 0.0]";
    replacementList.append(best);
    appendReplacements(replacementList, TraditionalReplacements, type);
}

QString QPSPrintEngineFontTraditionalChinese::extension() const
{
    return "-ETen-B5-H";
}

// ----------- simplified chinese ------------

// GB18030 fonts on Turbolinux
static const psfont SongGBK2K [] = {
    { "MSung-Light-GBK2K-H", 0, 100. },
    { "MSung-Italic-GBK2K-H", 0, 100. },
    { "MSung-Bold-GBK2K-H", 0, 100. },
    { "MSung-BoldItalic-GBK2K-H", 0, 100. },
};
static const psfont KaiGBK2K [] = {
    { "MKai-Medium-GBK2K-H", 0, 100. },
    { "MKai-Italic-GBK2K-H", 0, 100. },
    { "MKai-Bold-GBK2K-H", 0, 100. },
    { "MKai-BoldItalic-GBK2K-H", 0, 100. },
};
static const psfont HeiGBK2K [] = {
    { "MHei-Medium-GBK2K-H", 0, 100. },
    { "MHei-Italic-GBK2K-H", 0, 100. },
    { "MHei-Bold-GBK2K-H", 0, 100. },
    { "MHei-BoldItalic-GBK2K-H", 0, 100. },
};
static const psfont FangSongGBK2K [] = {
    { "CFangSong-Light-GBK2K-H", 0, 100. },
    { "CFangSong-Italic-GBK2K-H", 0, 100. },
    { "CFangSong-Bold-GBK2K-H", 0, 100. },
    { "CFangSong-BoldItalic-GBK2K-H", 0, 100. },
};

static const psfont Simplified [] = {
    { "MSung-Light-GBK-EUC-H", 0, 100. },
    { "MSung-Light-GBK-EUC-H", 0.2, 100. },
    { "MKai-Medium-GBK-EUC-H", 0, 100. },
    { "MKai-Medium-GBK-EUC-H", 0.2, 100. },
};

static const psfont MSungGBK [] = {
    { "MSung-Light-GBK-EUC-H", 0, 100. },
    { "MSung-Light-GBK-EUC-H", 0.2, 100. },
    { "MSung-Light-GBK-EUC-H", 0, 100. },
    { "MSung-Light-GBK-EUC-H", 0.2, 100. },
};

static const psfont FangSong [] = {
    { "CFangSong-Light-GBK-EUC-H", 0, 100. },
    { "CFangSong-Light-GBK-EUC-H", 0.2, 100. },
    { "CFangSong-Light-GBK-EUC-H", 0, 100. },
    { "CFangSong-Light-GBK-EUC-H", 0.2, 100. },
};

// Arphic Public License GB2312 TrueType fonts (on Debian and CLE and others)
static const psfont BousungEG [] = {
    { "BousungEG-Light-GB-GB-EUC-H", 0, 100. },
    { "BousungEG-Light-GB-GB-EUC-H", 0.2, 100. },
    { "BousungEG-Light-GB-Bold-GB-EUC-H", 0, 100. },
    { "BousungEG-Light-GB-Bold-GB-EUC-H", 0.2, 100. },
};
static const psfont GBZenKai [] = {
    { "GBZenKai-Medium-GB-GB-EUC-H", 0, 100. },
    { "GBZenKai-Medium-GB-GB-EUC-H", 0.2, 100. },
    { "GBZenKai-Medium-GB-Bold-GB-EUC-H", 0, 100. },
    { "GBZenKai-Medium-GB-Bold-GB-EUC-H", 0.2, 100. },
};

static const psfont * const SimplifiedReplacements[] = {
    SongGBK2K, FangSongGBK2K, KaiGBK2K, HeiGBK2K,
    Simplified, MSungGBK, FangSong, BousungEG, GBZenKai, Helvetica, 0
        };
#if 0
static const psfont * const SongGBK2KReplacements[] = {
    SongGBK2K, MSungGBK, BousungEG, Helvetica, 0
        };
#endif
static const psfont * const FangSongGBK2KReplacements[] = {
    FangSongGBK2K, FangSong, Courier, 0
        };
static const psfont * const KaiGBK2KReplacements[] = {
    KaiGBK2K, GBZenKai, Times, 0
        };
static const psfont * const HeiGBK2KReplacements[] = {
    HeiGBK2K, LucidaSans, 0
        };

class QPSPrintEngineFontSimplifiedChinese
  : public QPSPrintEngineFontAsian {
public:
      QPSPrintEngineFontSimplifiedChinese(QFontEngine* f);
      QString extension() const;
};

QPSPrintEngineFontSimplifiedChinese::QPSPrintEngineFontSimplifiedChinese(QFontEngine* f)
    : QPSPrintEngineFontAsian(f)
{
    codec = QTextCodec::codecForMib(114); // GB18030
    int type = getPsFontType(f);
    QString family = f->fontDef.family.toLower();
    if(family.contains("kai",Qt::CaseInsensitive)) {
        psname = KaiGBK2K[type].psname;
        appendReplacements(replacementList, KaiGBK2KReplacements, type);
    } else if(family.contains("fangsong",Qt::CaseInsensitive)) {
        psname = FangSongGBK2K[type].psname;
        appendReplacements(replacementList, FangSongGBK2KReplacements, type);
    } else if(family.contains("hei",Qt::CaseInsensitive)) {
        psname = HeiGBK2K[type].psname;
        appendReplacements(replacementList, HeiGBK2KReplacements, type);
    } else {
        psname = SongGBK2K[type].psname;
        appendReplacements(replacementList, SimplifiedReplacements, type);
    }
    //qDebug("simplified chinese: fontname is %s, psname=%s", f.family().latin1(), psname.latin1());
}

QString QPSPrintEngineFontSimplifiedChinese::extension() const
{
    return "-GBK2K-H";
}

#endif

// ================= END OF PS FONT METHODS ============

#if defined(Q_WS_X11) && defined(QT_NO_FONTCONFIG)
static QStringList fontPath()
{
    // append qsettings fontpath
    QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
    settings.beginGroup(QLatin1String("Qt"));

    QStringList fontpath;

    int npaths;
    char** font_path;
    font_path = XGetFontPath(X11->display, &npaths);
    bool xfsconfig_read = false;
    for (int i=0; i<npaths; i++) {
        // If we're using xfs, append font paths from /etc/X11/fs/config
        // can't hurt, and chances are we'll get all fonts that way.
        if (((font_path[i])[0] != '/') && !xfsconfig_read) {
            // We're using xfs -> read its config
            bool finished = false;
            QFile f("/etc/X11/fs/config");
            if (!f.exists())
                f.setFileName("/usr/X11R6/lib/X11/fs/config");
            if (!f.exists())
                f.setFileName("/usr/X11/lib/X11/fs/config");
            if (f.exists()) {
                f.open(QIODevice::ReadOnly);
                while (f.error()==QFile::NoError && !finished) {
                    QString fs = f.readLine(1024);
                    fs=fs.trimmed();
                    if (fs.left(9)=="catalogue" && fs.contains('=')) {
                        fs = fs.mid(fs.indexOf('=') + 1).trimmed();
                        bool end = false;
                        while (f.error()==QFile::NoError && !end) {
                            if (fs[int(fs.length())-1] == ',')
                                fs = fs.left(fs.length()-1);
                            else
                                end = true;
                            if (fs[0] != '#' && !fs.contains(":unscaled"))
                                fontpath += fs;
                            fs = f.readLine(1024);
                            fs=fs.trimmed();
                            if (fs.isEmpty())
                                end = true;
                        }
                        finished = true;
                    }
                }
                f.close();
            }
            xfsconfig_read = true;
        } else if (!strstr(font_path[i], ":unscaled")) {
            // Fonts paths marked :unscaled are always bitmapped fonts
            // -> we can as well ignore them now and save time
            fontpath += font_path[i];
        }
    }
    XFreeFontPath(font_path);

    // append qsettings fontpath
    QStringList fp = settings.value(QLatin1String("fontPath")).toStringList();
    if (!fp.isEmpty())
        fontpath += fp;

    return fontpath;
}

static QString fontFile(const QStringList &fontpath, const QByteArray &xname)
{
    QByteArray searchname = xname.toLower();
    QString fontfilename;
    for (QStringList::ConstIterator it = fontpath.constBegin(); it != fontpath.constEnd(); ++it) {
        if ((*it).left(1) != "/")
            continue; // not a path name, a font server
        QString fontmapname;
        int num = 0;
        // search font.dir and font.scale for the right file
        while (num < 2) {
            if (num == 0)
                fontmapname = (*it) + "/fonts.scale";
            else
                fontmapname = (*it) + "/fonts.dir";
            ++num;
            //qWarning(fontmapname);
            QFile fontmap(fontmapname);
            if (!fontmap.open(QIODevice::ReadOnly))
                continue;
            while (!fontmap.atEnd()) {
                QByteArray mapping = fontmap.readLine();
                // fold to lower (since X folds to lowercase)
                //qWarning(xfontname);
                //qWarning(mapping);
                if (!mapping.toLower().contains(searchname))
                    continue;
                int index = mapping.indexOf(' ');
                QString ffn = mapping.mid(0,index);
                // remove the most common bitmap formats
                if(ffn.contains(".pcf") || ffn.contains(".bdf") || ffn.contains(".spd") || ffn.contains(".phont"))
                    continue;
                fontfilename = (*it) + QString("/") + ffn;
                if (QFile::exists(fontfilename)) {
                    // ############ check if scalable
                    goto end;
                }
                fontfilename = QString();
            }
            fontmap.close();
        }
    }
end:
    return fontfilename;
}
#endif

QPSPrintEnginePrivate::QPSPrintEnginePrivate(QPrinter::PrinterMode m)
    : buffer(0), outDevice(0), fd(-1), pageBuffer(0), fontBuffer(0), clipOn(false),
      bkMode(Qt::TransparentMode),
      collate(false), copies(1), orientation(QPrinter::Portrait),
      pageSize(QPrinter::A4), pageOrder(QPrinter::FirstPageFirst), colorMode(QPrinter::GrayScale),
      printerState(QPrinter::Idle)
{
    currentFont = 0;
    currentPSFont = 0;

    firstPage = true;
    resolution = 72;
    if (m == QPrinter::HighResolution)
        resolution = 1200;
#ifdef Q_WS_X11
    else if (m == QPrinter::ScreenResolution)
        resolution = QX11Info::appDpiY();
#endif

    printer = 0;

    scale = 1.;

    QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
    settings.beginGroup(QLatin1String("Qt"));
    embedFonts = settings.value(QLatin1String("embedFonts"), true).toBool();
#if defined(Q_WS_X11) && defined(QT_NO_FONTCONFIG)
    if (embedFonts && !X11->use_xrender)
        fontpath = ::fontPath();
#endif
}

QPSPrintEnginePrivate::~QPSPrintEnginePrivate()
{
    delete pageBuffer;
    QHash<QString, QPSPrintEngineFont *>::ConstIterator it = fonts.constBegin();
    while (it != fonts.constEnd()) {
        delete it.value();
        ++it;
    }
}

void QPSPrintEnginePrivate::setFont(QFontEngine *fe)
{
    Q_ASSERT(fe);
    if (currentFont == fe && currentPSFont)
        return;
    currentFont = fe;

    QString fontKey;

    QFontEngine::Type fontType = fe->type();

#ifdef QT_HAVE_FREETYPE
    if (embedFonts) {
#ifdef Q_WS_X11
#ifndef QT_NO_FONTCONFIG
        if (fontType == QFontEngine::Multi) {
            // ##### hack to get the multi engine sort of working
            QFontEngineMulti *mf = static_cast<QFontEngineMulti *>(fe);
            fe = mf->engine(0);
            fontType = fe->type();
        }
        if (X11->use_xrender && fontType == QFontEngine::Freetype && FT_IS_SCALABLE(ft_face(fe))) {
            FcPattern *pattern = static_cast<QFontEngineFT *>(fe)->pattern();
            FcChar8 *filename = 0;
            FcPatternGetString (pattern, FC_FILE, 0, &filename);
            //qDebug("filename for font is '%s'", filename);
            if (filename)
                fontKey = QString::fromLocal8Bit((const char *)filename);
        }
#else // QT_NO_FONTCONFIG
        if (fontType == QFontEngine::XLFD) {
            QString rawName = fe->name();
            int index = rawName.indexOf('-');
            if (index == 0) {
                // this is an XLFD font name
                for (int i=0; i < 6; i++) {
                    index = rawName.indexOf('-',index+1);
                }
                fontKey = rawName.mid(0,index);
                if (fontKey.endsWith("*"))
                    fontKey.chop(1);
            }
            if (!fontKey.isEmpty())
                fontKey = ::fontFile(fontpath, fontKey);
        }
#endif
#endif
        // ######## add embedded here
    }
#endif

    if (fontKey.isEmpty()) {
        fontKey = QLatin1String("NonEmbed:") + makePSFontName(fe);
    }
    Q_ASSERT(!fontKey.isEmpty());
    currentPSFont = fonts.value(fontKey);

    if (!currentPSFont) {
#ifdef QT_HAVE_FREETYPE
        if (!fontKey.startsWith(QLatin1String("NonEmbed:"))) {
            currentPSFont = new QPSPrintEngineFontFT(fe);
        } else
#endif
        {
            // ### add Han handling
            currentPSFont = new QPSPrintEngineFontNotFound(fe);
        }
        if (currentPSFont->postScriptFontName() == "Symbol")
            currentPSFont->setSymbol();

        // this is needed to make sure we don't get the same postscriptname twice
        for (QHash<QString, QPSPrintEngineFont *>::ConstIterator it = fonts.constBegin(); it != fonts.constEnd(); ++it) {
            if (*(*it) == *currentPSFont) {
                qWarning("Post script driver: font already in dict");
                delete currentPSFont;
                currentPSFont = *it;
            }
        }
        fonts[fontKey] = currentPSFont;
    }


#if 0
    // map some scripts to something more useful
    if (script == QFont::Han) {
        QTextCodec *lc = QTextCodec::codecForLocale();
        switch(lc->mibEnum()) {
        case 36: // KS C 5601
        case 38: // EUC KR
            script = QFont::Hangul;
            break;

        case 57: // gb2312.1980-0
        case 113: // GBK
        case -113: // gbk-0
        case 114: // GB18030
        case -114: // gb18030-0
        case 2025: // GB2312
        case 2026: // Big5
        case -2026: // Big5-HKSCS
        case 2101: // big5-0, big5.eten-0
        case -2101: // big5hkscs-0, hkscs-1
            break;

        case 16: // JIS7
        case 17: // SJIS
        case 18: // EUC JP
        case 63: // JIS X 0208
        default:
            script = QFont::Hiragana;
            break;
        }
    } else if (script == QFont::Katakana)
        script = QFont::Hiragana;
    else if (script == QFont::Bopomofo)
        script = QFont::Han;

    {
        // non embedding engine
        if (script == QFont::Hiragana)
            p = new QPSPrintEngineFontJapanese(engine);
        else if (script == QFont::Hangul)
            p = new QPSPrintEngineFontKorean(engine);
        else if (script == QFont::Han) {
            QTextCodec *lc = QTextCodec::codecForLocale();
            switch(lc->mibEnum()) {
            case 2025: // GB2312
            case 57: // gb2312.1980-0
            case 113: // GBK
            case -113: // gbk-0
            case 114: // GB18030
            case -114: // gb18030-0
                p = new QPSPrintEngineFontSimplifiedChinese(engine);
                break;
            case 2026: // Big5
            case -2026: // big5-0, big5.eten-0
            case 2101: // Big5-HKSCS
            case -2101: // big5hkscs-0, hkscs-1
                p = new QPSPrintEngineFontTraditionalChinese(engine);
                break;
            default:
                p = new QPSPrintEngineFontJapanese(engine);
            }
        } else {
            //qDebug("didnt find font for %s", xfontname.latin1());
            p = new QPSPrintEngineFontNotFound(engine);
        }
        break;
    }
#endif

    QString ps = currentPSFont->postScriptFontName();

    QString s = ps;
    s.append(' ');
    s.prepend(' ');

    QString key = fontKey + '/' + toString(fe->fontDef.pixelSize);
    QString tmp = !buffer ? pageFontNames.value(key, QString::null) : headerFontNames.value(key, QString::null);

    QString fontName;
    if (!tmp.isNull())
        fontName = tmp;

    if (fontName.isEmpty())
        fontName = currentPSFont->defineFont(pageStream, ps, key, this, fe->fontDef.pixelSize);

    pageStream << fontName << " F\n";

    ps.append(' ');
    ps.prepend(' ');
    if (!fontsUsed.contains(ps))
        fontsUsed += ps;
}


static void ps_r7(QTextStream& stream, const char * s, int l)
{
    int i = 0;
    uchar line[79];
    int col = 0;

    while(i < l) {
        line[col++] = s[i++];
        if (col >= 76) {
            line[col++] = '\n';
            line[col++] = '\0';
            stream << (const char *)line;
            col = 0;
        }
    }
    if (col > 0) {
        while((col&3) != 0)
            line[col++] = '%'; // use a comment as padding
        line[col++] = '\n';
        line[col++] = '\0';
        stream << (const char *)line;
    }
}

enum {
    quoteSize = 3, // 1-8 pixels
    maxQuoteLength = 4+16+32+64+128+256, // magic extended quote
    quoteReach = 10, // ... 1-1024 pixels back
    tableSize = 1024, // 2 ** quoteReach;
    numAttempts = 128,

    hashSize = 71,

    None = INT_MAX
};

static QByteArray runlengthEncode(const QByteArray &input)
{
    if (!input.length())
        return input;

    const char *data = input.constData();

    QByteArray out;
    int start = 0;
    char last = *data;

    enum State {
        Undef,
        Equal,
        Diff
    };
    State state = Undef;

    int i = 1;
    while (1) {
        bool flush = false;
        switch(state) {
        case Undef:
            state = (last == data[i]) ? Equal : Diff;
            break;
        case Equal:
            if (data[i] != last)
                flush = true;
            break;
        case Diff:
            if (data[i] == last) {
                --i;
                flush = true;
            }
        }
        if (flush || i == input.size() - 1 || i - start == 128) {
            int size = i - start;
            if (state == Equal) {
                out.append((char)(uchar)(257-size));
                out.append(last);
            } else {
                out.append((char)(uchar)size-1);
                while (start < i)
                    out.append(data[start++]);
            }
            state = Undef;
            start = i;
            if (i == input.size() - 1)
                break;
        }
        last = data[i];
        ++i;
    };
    out.append((char)128);
    return out;
}

static QByteArray ascii85Encode(const QByteArray &input)
{
    int isize = input.size()/4*4;
    QByteArray output;
    output.resize(input.size()*5/4+7);
    char *out = output.data();
    const uchar *in = (const uchar *)input.constData();
    for (int i = 0; i < isize; i += 4) {
        uint val = (((uint)in[i])<<24) + (((uint)in[i+1])<<16) + (((uint)in[i+2])<<8) + (uint)in[i+3];
        if (val == 0) {
            *out = 'z';
            ++out;
        } else {
            char base[5];
            base[4] = val % 85;
            val /= 85;
            base[3] = val % 85;
            val /= 85;
            base[2] = val % 85;
            val /= 85;
            base[1] = val % 85;
            val /= 85;
            base[0] = val % 85;
            *(out++) = base[0] + '!';
            *(out++) = base[1] + '!';
            *(out++) = base[2] + '!';
            *(out++) = base[3] + '!';
            *(out++) = base[4] + '!';
        }
    }
    //write the last few bytes
    int remaining = input.size() - isize;
    uint val = 0;
    for (int i = isize; i < input.size(); ++i)
        val = val << 8 + in[i];
    val <<= 8*(4-remaining);
    char base[5];
    base[4] = val % 85;
    val /= 85;
    base[3] = val % 85;
    val /= 85;
    base[2] = val % 85;
    val /= 85;
    base[1] = val % 85;
    val /= 85;
    base[0] = val % 85;
    for (int i = 0; i < remaining+1; ++i)
        *(out++) = base[i] + '!';
    *(out++) = '~';
    *(out++) = '>';
    output.resize(out-output.data());
    return output;
}

static QByteArray compress(const QImage & image, bool gray) {
    int width = image.width();
    int height = image.height();
    int depth = image.depth();
    int size = width*height;

    if (depth == 1)
        size = (width+7)/8*height;
    else if (!gray)
        size = size*3;

    QByteArray pixelData;
    pixelData.resize(size+1);
    uchar *pixel = (uchar *)pixelData.data();
    int i = 0;
    if (depth == 1) {
        QImage::Format format = image.format();
        memset(pixel, 0xff, size);
        for(int y=0; y < height; y++) {
            const uchar * s = image.scanLine(y);
            for(int x=0; x < width; x++) {
                // need to copy bit for bit...
                bool b = (format == QImage::Format_MonoLSB) ?
                         (*(s + (x >> 3)) >> (x & 7)) & 1 :
                         (*(s + (x >> 3)) << (x & 7)) & 0x80 ;
                if (b)
                    pixel[i >> 3] ^= (0x80 >> (i & 7));
                i++;
            }
            // we need to align to 8 bit here
            i = (i+7) & 0xffffff8;
        }
    } else if (depth == 8) {
        for(int y=0; y < height; y++) {
            const uchar * s = image.scanLine(y);
            for(int x=0; x < width; x++) {
                QRgb rgb = image.color(s[x]);
                if (gray) {
                    pixel[i] = (unsigned char) qGray(rgb);
                    i++;
                } else {
                    pixel[i] = (unsigned char) qRed(rgb);
                    pixel[i+1] = (unsigned char) qGreen(rgb);
                    pixel[i+2] = (unsigned char) qBlue(rgb);
                    i += 3;
                }
            }
        }
    } else {
        bool alpha = image.format() != QImage::Format_RGB32;
        for(int y=0; y < height; y++) {
            QRgb * s = (QRgb*)(image.scanLine(y));
            for(int x=0; x < width; x++) {
                QRgb rgb = (*s++);
                if (alpha && qAlpha(rgb) < 0x40) // 25% alpha, convert to white -
                    rgb = qRgb(0xff, 0xff, 0xff);
                if (gray) {
                    pixel[i] = (unsigned char) qGray(rgb);
                    i++;
                } else {
                    pixel[i] = (unsigned char) qRed(rgb);
                    pixel[i+1] = (unsigned char) qGreen(rgb);
                    pixel[i+2] = (unsigned char) qBlue(rgb);
                    i += 3;
                }
            }
        }
    }

    pixel[size] = 0;

    QByteArray runlength = runlengthEncode(pixelData);
    QByteArray outarr = ascii85Encode(runlength);
    return outarr;
}

#undef POINT
#undef RECT
#undef INT_ARG

#define POINT(p) p.x() << ' ' << p.y() << ' '
#define RECT(r) r.x()  << ' ' << r.y()  << ' ' <<     \
                        r.width() << ' ' << r.height() << ' '
#define INT_ARG(x)  x << ' '

static QByteArray color(const QColor &c)
{
    QByteArray retval;
    retval += '[';
    retval += QByteArray::number(c.red()/255.);
    retval += ' ';
    retval += QByteArray::number(c.green()/255.);
    retval += ' ';
    retval += QByteArray::number(c.blue()/255.);
    retval += ']';
    return retval;
}


static const char * psCap(Qt::PenCapStyle p)
{
    if (p == Qt::SquareCap)
        return "2 ";
    else if (p == Qt::RoundCap)
        return "1 ";
    return "0 ";
}


static const char * psJoin(Qt::PenJoinStyle p) {
    if (p == Qt::BevelJoin)
        return "2 ";
    else if (p == Qt::RoundJoin)
        return "1 ";
    return "0 ";
}



void QPSPrintEnginePrivate::drawImage(qreal x, qreal y, qreal w, qreal h,
                                  const QImage &img, const QImage &mask)
{
    if (!w || !h || img.isNull()) return;

    int width  = img.width();
    int height = img.height();
    qreal scaleX = width/w;
    qreal scaleY = height/h;

    bool gray = (colorMode == QPrinter::GrayScale) ||
                img.allGray();
    int splitSize = 21830 * (gray ? 3 : 1);
    if (width * height > splitSize) { // 65535/3, tolerance for broken printers
        int images, subheight;
        images = (width * height + splitSize - 1) / splitSize;
        subheight = (height + images-1) / images;
        while (subheight * width > splitSize) {
            images++;
            subheight = (height + images-1) / images;
        }
        int suby = 0;
        while(suby < height) {
            drawImage(x, y + suby/scaleY, w, qMin(subheight, height-suby)/scaleY,
                      img.copy(0, suby, width, qMin(subheight, height-suby)),
                      mask.isNull() ? mask : mask.copy(0, suby, width, qMin(subheight, height-suby)));
            suby += subheight;
        }
    } else {
        QByteArray out;
        int size = 0;
        const char *bits;

        if (!mask.isNull()) {
            out = ::compress(mask, true);
            size = (width+7)/8*height;
            pageStream << "/mask currentfile/ASCII85Decode filter/RunLengthDecode filter "
                       << size << " string readstring\n";
            ps_r7( pageStream, out, out.size() );
            pageStream << " pop d\n";
        }
        if (img.depth() == 1) {
            size = (width+7)/8*height;
            bits = "1 ";
        } else if (gray) {
            size = width*height;
            bits = "8 ";
        } else {
            size = width*height*3;
            bits = "24 ";
        }

        out = ::compress(img, gray);
        pageStream << "/sl currentfile/ASCII85Decode filter/RunLengthDecode filter "
                       << size << " string readstring\n";
        ps_r7( pageStream, out, out.size() );
        pageStream << " pop d\n";
        pageStream << width << ' ' << height << "[" << scaleX << " 0 0 " << scaleY << " 0 0]sl "
                   << bits << (!mask.isNull() ? "mask " : "false ")
                   << x << ' ' << y << " di\n";
    }
}


void QPSPrintEnginePrivate::orientationSetup()
{
    if (orientation == QPrinter::Landscape)
        pageStream << "QLS\n";
}


void QPSPrintEnginePrivate::emitHeader(bool finished)
{
    QString title = docName;
    QString creator = this->creator;
    if (creator.count() == 0)                             // default creator
        creator = QLatin1String("Qt " QT_VERSION_STR);
    outDevice = new QFile();
    static_cast<QFile *>(outDevice)->open(QIODevice::WriteOnly, fd);
    outStream.setDevice(outDevice);
    outStream << "%!PS-Adobe-1.0";
    QPSPrintEngine *q = static_cast<QPSPrintEngine *>(q_ptr);
    scale = 72. / ((qreal) q->metric(QPaintDevice::PdmDpiY));
    QRect pageRect = this->pageRect();
    QRect paperRect = this->paperRect();
    uint mtop = pageRect.top() - paperRect.top();
    uint mleft = pageRect.left() - paperRect.left();
    uint mbottom = paperRect.bottom() - pageRect.bottom();
    uint mright = paperRect.right() - pageRect.right();
    int width = pageRect.width();
    int height = pageRect.height();
    if (finished && pageCount == 1 && copies == 1 &&
         ((fullPage && qt_gen_epsf) || (outputFileName.endsWith(".eps")))
       ) {
        if (!boundingBox.isValid())
            boundingBox.setRect(0, 0, width, height);
        if (orientation == QPrinter::Landscape) {
            if (!fullPage)
                boundingBox.translate(-mleft, -mtop);
            outStream << " EPSF-3.0\n%%BoundingBox: "
                      << (int)(printer->height() - boundingBox.bottom())*scale << " " // llx
                      << (int)(printer->width() - boundingBox.right())*scale - 1 << " " // lly
                      << (int)(printer->height() - boundingBox.top())*scale + 1 << " " // urx
                      << (int)(printer->width() - boundingBox.left())*scale; // ury
        } else {
            if (!fullPage)
                boundingBox.translate(mleft, -mtop);
            outStream << " EPSF-3.0\n%%BoundingBox: "
                      << (int)(boundingBox.left())*scale << " "
                      << (int)(printer->height() - boundingBox.bottom())*scale - 1 << " "
                      << (int)(boundingBox.right())*scale + 1 << " "
                      << (int)(printer->height() - boundingBox.top())*scale;
        }
    } else {
        int w = width + (fullPage ? 0 : mleft + mright);
        int h = height + (fullPage ? 0 : mtop + mbottom);
        w = (int)(w*scale);
        h = (int)(h*scale);
        // set a bounding box according to the DSC
        if (orientation == QPrinter::Landscape)
            outStream << "\n%%BoundingBox: 0 0 " << h << " " << w;
        else
            outStream << "\n%%BoundingBox: 0 0 " << w << " " << h;
    }
    outStream << "\n" << wrapDSC("%%Creator: " + creator);
    if (title.count() == 0)
        outStream << wrapDSC("%%Title: " + title);
    outStream << "%%CreationDate: " << QDateTime::currentDateTime().toString();
    outStream << "\n%%Orientation: ";
    if (orientation == QPrinter::Landscape)
        outStream << "Landscape";
    else
        outStream << "Portrait";
    if (finished)
        outStream << "\n%%Pages: " << pageCount << "\n"
                  << wrapDSC("%%DocumentFonts: " + fontsUsed);
    else
        outStream << "%%Pages: (atend)"
               << "\n%%DocumentFonts: (atend)";
    outStream << "\n%%EndComments\n";

    outStream << "%%BeginProlog\n";
    const char prologLicense[] = "% Prolog copyright 1994-2003 Trolltech. "
                                 "You may copy this prolog in any way\n"
                                 "% that is directly related to this "
                                 "document. For other use of this prolog,\n"
                                 "% see your licensing agreement for Qt.\n";
    outStream << prologLicense << ps_header << "\n";

    // we have to do this here, as scaling can affect this.
    QString lineStyles = "/LArr["                                       // Pen styles:
                         " [] []"                       //   solid line
                         " [w s] [s w]"                 //   dash line
                         " [s s] [s s]"                  //   dot line
                         " [m s s s] [s m s s]"      //   dash dot line
                         " [m s s s s] [s m s s s s]"         //   dash dot dot line
                         "] d\n";
    lineStyles.replace(QRegExp("w"), toString(10./scale));
    lineStyles.replace(QRegExp("m"), toString(5./scale));
    lineStyles.replace(QRegExp("s"), toString(3./scale));

    outStream << lineStyles;

    outStream << "/pageinit {\n";
    if (!fullPage) {
        if (orientation == QPrinter::Portrait)
            outStream << mleft*scale << " "
                   << mbottom*scale << " translate\n";
        else
            outStream << mtop*scale << " "
                   << mleft*scale << " translate\n";
    }
    if (orientation == QPrinter::Portrait) {
        outStream << "% " << printer->widthMM() << "*" << printer->heightMM()
               << "mm (portrait)\n0 " << height*scale
               << " translate " << scale << " -" << scale << " scale/defM matrix CM d } d\n";
    } else {
        outStream << "% " << printer->heightMM() << "*" << printer->widthMM()
               << " mm (landscape)\n 90 rotate " << scale << " -" << scale << " scale/defM matrix CM d } d\n";
    }
    outStream << "%%EndProlog\n";


    outStream << "%%BeginSetup\n";
    if (copies > 1) {
        outStream << "/#copies " << copies << " def\n";
        outStream << "/NumCopies " << copies << " SPD\n";
        outStream << "/Collate " << (collate ? "true" : "false") << " SPD\n";
    }
    fontStream.flush();
    if (fontBuffer->buffer().size()) {
        if (pageCount == 1 || finished)
            outStream << "% Fonts and encodings used\n";
        else
            outStream << "% Fonts and encodings used on pages 1-"
                   << pageCount << "\n";
        for (QHash<QString, QPSPrintEngineFont *>::Iterator it = fonts.begin(); it != fonts.end(); ++it)
            (*it)->download(outStream, true); // true means its global
        fontStream.flush();
        outStream << fontBuffer->buffer();
    }
    outStream << "%%EndSetup\n";
    outStream << buffer->buffer();

    delete buffer;
    buffer = 0;
    fontStream.flush();
    delete fontBuffer;
    fontBuffer = 0;
}

static void putRect(QTextStream &stream, const QRect &r)
{
    stream << r.x() << " "
           << r.y() << " "
           << r.width() << " "
           << r.height() << " ";
}

void QPSPrintEnginePrivate::flushPage(bool last)
{
    if (last && !pageBuffer)
        return;
    bool pageFonts = (buffer == 0);
    if (buffer &&
//         (last || pagesInBuffer++ > -1 ||
//           (pagesInBuffer > 4 && buffer->size() > 262144)))
#ifdef Q_WS_QWS
         (last || buffer->size() > 2000000) // embedded is usually limited in memory
#else
         (last || buffer->size() > 50000000)
#endif
        ) {
//        qDebug("emiting header at page %d", pageCount);
        emitHeader(last);
    }
    outStream << "%%Page: "
              << pageCount << ' ' << pageCount << endl
              << "%%BeginPageSetup\n"
              << "QI\n";
    if (pageFonts) {
        //qDebug("page fonts for page %d", pageCount);
        // we have already downloaded the header. Maybe we have page fonts here
        for (QHash<QString, QPSPrintEngineFont *>::Iterator it = fonts.begin();
             it != fonts.end(); ++it)
            (*it)->download(outStream, false); // false means its for the page only
    }
    outStream  << "%%EndPageSetup\n";
    if (pageBuffer) {
        pageStream.flush();
        outStream << pageBuffer->buffer();
    }
    outStream << "\nQP\n";
    pageCount++;
}

// ================ PSPrinter class ========================

// ### Implementation LinearGradients
QPSPrintEngine::QPSPrintEngine(QPrinter::PrinterMode m)
    : QPaintEngine(*(new QPSPrintEnginePrivate(m)), AllFeatures)
{
}


QPSPrintEngine::~QPSPrintEngine()
{
    if (d_func()->fd >= 0)
        ::close(d_func()->fd);
}



static void ignoreSigPipe(bool b)
{
    static struct sigaction *users_sigpipe_handler = 0;

    if (b) {
        if (users_sigpipe_handler != 0)
            return; // already ignoring sigpipe

        users_sigpipe_handler = new struct sigaction;
        struct sigaction tmp_sigpipe_handler;
        tmp_sigpipe_handler.sa_handler = SIG_IGN;
        sigemptyset(&tmp_sigpipe_handler.sa_mask);
        tmp_sigpipe_handler.sa_flags = 0;

        if (sigaction(SIGPIPE, &tmp_sigpipe_handler, users_sigpipe_handler) == -1) {
            delete users_sigpipe_handler;
            users_sigpipe_handler = 0;
        }
    }
    else {
        if (users_sigpipe_handler == 0)
            return; // not ignoring sigpipe

        if (sigaction(SIGPIPE, users_sigpipe_handler, 0) == -1)
            qWarning("QPSPrintEngine: could not restore SIGPIPE handler");

        delete users_sigpipe_handler;
        users_sigpipe_handler = 0;
    }
}

static const char * const psToStr[QPrinter::NPageSize+1] =
{
    "A4", "B5", "Letter", "Legal", "Executive",
    "A0", "A1", "A2", "A3", "A5", "A6", "A7", "A8", "A9", "B0", "B1",
    "B10", "B2", "B3", "B4", "B6", "B7", "B8", "B9", "C5E", "Comm10E",
    "DLE", "Folio", "Ledger", "Tabloid", 0
};

static void closeAllOpenFds()
{
    // hack time... getting the maximum number of open
    // files, if possible.  if not we assume it's the
    // larger of 256 and the fd we got
    int i;
#if defined(_SC_OPEN_MAX)
    i = (int)sysconf( _SC_OPEN_MAX );
#elif defined(_POSIX_OPEN_MAX)
    i = (int)_POSIX_OPEN_MAX;
#elif defined(OPEN_MAX)
    i = (int)OPEN_MAX;
#else
    i = 256;
#endif
    while( --i > 0 )
	::close( i );
}

bool QPSPrintEngine::begin(QPaintDevice *pdev)
{
    Q_D(QPSPrintEngine);
    d->pdev = pdev;
    d->printer = static_cast<QPrinter*>(pdev);
    if (!d->outputFileName.isEmpty()) {
        d->fd = QT_OPEN( d->outputFileName.toLocal8Bit().constData(), O_CREAT | O_NOCTTY | O_TRUNC | O_WRONLY,
#if defined(Q_OS_WIN)
            _S_IREAD | _S_IWRITE
#else
            0666
#endif
            );
    } else {
        QString pr;
        if (!d->printerName.isEmpty())
            pr = d->printerName;
        int fds[2];
        if ( pipe( fds ) != 0 ) {
            qWarning( "QPSPrinter: could not open pipe to print" );
            return false;
        }

        pid_t pid = fork();
        if ( pid == 0 ) {       // child process
            // if possible, exit quickly, so the actual lp/lpr
            // becomes a child of init, and ::waitpid() is
            // guaranteed not to wait.
            if ( fork() > 0 ) {
                closeAllOpenFds();

                // try to replace this process with "true" - this prevents
                // global destructors from being called (that could possibly
                // do wrong things to the parent process)
                (void)execlp("true", "true", (char *)0);
                (void)execl("/bin/true", "true", (char *)0);
                (void)execl("/usr/bin/true", "true", (char *)0);
                ::exit( 0 );
            }
            dup2( fds[0], 0 );

            closeAllOpenFds();

            if ( !d->printProgram.isEmpty() ) {
                if (!d->selectionOption.isEmpty())
                    pr.prepend(d->selectionOption);
                else
                    pr.prepend( QLatin1String( "-P" ) );
                (void)execlp( d->printProgram.toLocal8Bit().data(), d->printProgram.toLocal8Bit().data(),
                              pr.toLocal8Bit().data(), (char *)0 );
            } else {
                // if no print program has been specified, be smart
                // about the option string too.
                QList<QByteArray> lprhack;
                QList<QByteArray> lphack;
                QByteArray media;
                if ( !pr.isEmpty() || !d->selectionOption.isEmpty() ) {
                    if ( !d->selectionOption.isEmpty() ) {
                        QStringList list = d->selectionOption.split(QChar(' '));
                        for (int i = 0; i < list.size(); ++i)
                            lprhack.append(list.at(i).toLocal8Bit());
                        lphack = lprhack;
                    } else {
                        lprhack.append("-P");
                        lphack.append("-d");
                    }
                    lprhack.append(pr.toLocal8Bit());
                    lphack.append(pr.toLocal8Bit());
                }
                char ** lpargs = new char *[lphack.size()+6];
                lpargs[0] = "lp";
                int i;
                for (i = 0; i < lphack.size(); ++i)
                    lpargs[i+1] = (char *)lphack.at(i).constData();
                if (psToStr[d->pageSize]) {
                    lpargs[++i] = "-o";
                    lpargs[++i] = (char *)psToStr[d->pageSize];
                    lpargs[++i] = "-o";
                    media = "media=";
                    media += psToStr[d->pageSize];
                    lpargs[++i] = (char *)media.constData();
                }
                lpargs[++i] = 0;
                char **lprargs = new char *[lprhack.size()+1];
                lprargs[0] = "lpr";
                for (int i = 0; i < lprhack.size(); ++i)
                    lprargs[i+1] = (char *)lprhack[i].constData();
                lprargs[lprhack.size() + 1] = 0;
                (void)execvp( "lp", lpargs );
                (void)execvp( "lpr", lprargs );
                (void)execv( "/bin/lp", lpargs);
                (void)execv( "/bin/lpr", lprargs);
                (void)execv( "/usr/bin/lp", lpargs);
                (void)execv( "/usr/bin/lpr", lprargs);
            }
            // if we couldn't exec anything, close the fd,
            // wait for a second so the parent process (the
            // child of the GUI process) has exited.  then
            // exit.
            ::close( 0 );
            (void)::sleep( 1 );
            ::exit( 0 );
        }
        // parent process
        ::close( fds[0] );
        d->fd = fds[1];
    }
    if (d->fd < 0)
        return false;

    d->pagesInBuffer = 0;
    d->buffer = new QBuffer();
    d->buffer->open(QIODevice::WriteOnly);
    d->outStream.setCodec(QTextCodec::codecForName("ISO-8895-1"));
    d->outStream.setDevice(d->buffer);
    d->fontBuffer = new QBuffer();
    d->fontBuffer->open(QIODevice::WriteOnly);
    d->fontStream.setCodec(QTextCodec::codecForName("ISO-8895-1"));
    d->fontStream.setDevice(d->fontBuffer);
    d->headerFontNumber = 0;
    d->pageCount           = 1;                // initialize state
    d->clipOn  = false;
    d->boundingBox = QRect(0, 0, -1, -1);
    d->fontsUsed = QLatin1String("");

    d->scale = 72. / ((qreal) d->printer->logicalDpiY());
    setActive(true);

    newPage();

    return true;
}

bool QPSPrintEngine::end()
{
    Q_D(QPSPrintEngine);
    bool pageCountAtEnd = (d->buffer != 0);

    // we're writing to lp/lpr through a pipe, we don't want to crash with SIGPIPE
    // if lp/lpr dies
    ignoreSigPipe(true);
    d->flushPage(true);
    d->outStream << "%%Trailer\n";
    if (pageCountAtEnd)
        d->outStream << "%%Pages: " << d->pageCount - 1 << "\n" <<
            wrapDSC("%%DocumentFonts: " + d->fontsUsed);
    d->outStream << "%%EOF\n";
    ignoreSigPipe(false);

    d->outStream.flush();
    if (d->outDevice)
        d->outDevice->close();
    if (d->fd >= 0)
        ::close(d->fd);
    d->fd = -1;
    delete d->outDevice;
    d->outDevice = 0;
    setActive(false);
    d->printer = 0;

    return true;
}


void QPSPrintEngine::updateState(const QPaintEngineState &state)
{
    QPaintEngine::DirtyFlags flags = state.state();
    if (flags & DirtyTransform) updateMatrix(state.matrix());
    if (flags & DirtyPen) updatePen(state.pen());
    if (flags & DirtyBrush) updateBrush(state.brush(), state.brushOrigin());
    if (flags & DirtyBackground) updateBackground(state.backgroundMode(), state.backgroundBrush());
    if (flags & DirtyFont) updateFont(state.font());
    if (flags & DirtyClipPath) {
        updateClipRegion(QRegion(state.clipPath().toFillPolygon().toPolygon(),
                                 state.clipPath().fillRule()),
                         state.clipOperation());
    }
    if (flags & DirtyClipRegion) updateClipRegion(state.clipRegion(), state.clipOperation());
// ### if (flags & DirtyCompositionMode) updateCompositionMode(state.clipRegion(), state.clipOperation());p
}



void QPSPrintEngine::updatePen(const QPen &pen)
{
    Q_D(QPSPrintEngine);
    d->cpen = pen;
    // we special-case for narrow solid lines with the default
    // cap and join styles
    if (d->cpen.style() == Qt::SolidLine && d->cpen.width() == 0 &&
        d->cpen.capStyle() == Qt::FlatCap &&
        d->cpen.joinStyle() == Qt::MiterJoin)
        d->pageStream << color(d->cpen.color()) << "P1\n";
    else
        d->pageStream << (int)d->cpen.style() << ' ' << d->cpen.width()
                      << ' ' << color(d->cpen.color())
                      << psCap(d->cpen.capStyle())
                      << psJoin(d->cpen.joinStyle()) << "PE\n";
}

void QPSPrintEngine::updateBrush(const QBrush &brush, const QPointF &/*origin*/)
{
    Q_D(QPSPrintEngine);
    // ### use brush origin!
    if (brush.style() == Qt::TexturePattern) {
#if defined(CHECK_RANGE)
        qWarning("QPrinter: Pixmap brush not supported");
#endif
        return;
    }
    d->cbrush = brush;
    // we special-case for nobrush since this is a very common case
    if (d->cbrush.style() == Qt::NoBrush)
        d->pageStream << "NB\n";
    else if (brush.style() == Qt::LinearGradientPattern) {
        const QLinearGradient *lg = static_cast<const QLinearGradient *>(brush.gradient());
        d->pageStream << POINT(lg->start())
                      << POINT(lg->finalStop())
                      << color(lg->stops().first().second)
                      << color(lg->stops().last().second) << " SetLinGrad\n";
    } else {
        d->pageStream << (int)d->cbrush.style() << ' '
                      << color(d->cbrush.color()) << "BR\n";
    }
}

void QPSPrintEngine::updateFont(const QFont &)
{
    // no need to do anything here, as we use the font engines.
}

void QPSPrintEngine::updateBackground(Qt::BGMode bgMode, const QBrush &bgBrush)
{
    Q_D(QPSPrintEngine);
    d->bkColor = bgBrush.color();
    d->bkMode = bgMode;

    d->pageStream << color(d->bkColor) << "BC\n";

    if (d->bkMode == Qt::TransparentMode)
        d->pageStream << "/OMo false d\n";
    else
        d->pageStream << "/OMo true d\n";
}

void QPSPrintEngine::updateMatrix(const QMatrix &matrix)
{
    Q_D(QPSPrintEngine);
    d->pageStream << "["
                  << matrix.m11() << ' ' << matrix.m12() << ' '
                  << matrix.m21() << ' ' << matrix.m22() << ' '
                  << matrix.dx()  << ' ' << matrix.dy()
                  << "]ST\n";
}

void QPSPrintEngine::updateClipRegion(const QRegion &region, Qt::ClipOperation /*op*/)
{
    Q_D(QPSPrintEngine);
    bool clipEnabled = !region.isEmpty();
    if (!d->clipOn && !clipEnabled)
        return;

    if (d->clipOn) {
        d->pageStream << "CLO\n";              // clipping off, includes a restore
        d->clipOn = false;
        setDirty(AllDirty); // so we must force a complete state update
        return;
    }

    if (clipEnabled) {
        QVector<QRect> rects = region.rects();
        int i;
        d->pageStream<< "CLS\n";           // start clipping
        for(i = 0 ; i < rects.size() ; i++) {
            putRect(d->pageStream, rects[i]);
            d->pageStream << "ACR\n";          // add clip rect
            if (d->pageCount == 1)
                d->boundingBox = d->boundingBox.unite(rects[i]);
        }
        d->pageStream << "clip\n";            // end clipping
        d->clipOn = true;
    }
#if 0
    // ###
    else {
        // if we're painting without clipping, the bounding box must
        // be everything.  NOTE: this assumes that this function is
        // only ever called when something is to be painted.
        if (!boundingBox.isValid())
            boundingBox.setRect(0, 0, printer->width(), printer->height());
    }
#endif
}

void QPSPrintEngine::drawLine(const QLineF &line)
{
    Q_D(QPSPrintEngine);
    d->pageStream << POINT(line.p2())
                  << POINT(line.p1()) << "DL\n";
}

void QPSPrintEngine::drawRect(const QRectF &r)
{
    Q_D(QPSPrintEngine);
    d->pageStream << RECT(r) << "R\n";
}

void QPSPrintEngine::drawPoint(const QPointF &p)
{
    Q_D(QPSPrintEngine);
    d->pageStream << POINT(p) << "P\n";
}

void QPSPrintEngine::drawEllipse(const QRectF &r)
{
    Q_D(QPSPrintEngine);
    d->pageStream << RECT(r) << "E\n";
}

void QPSPrintEngine::drawLines(const QLineF *lines, int lineCount)
{
    Q_D(QPSPrintEngine);
    d->pageStream << "NP\n";
    for (int i = 0; i < lineCount; ++i) {
        d->pageStream << POINT(lines[i].p1()) << "MT "
                      << POINT(lines[i].p2()) << "LT\n";
    }
    d->pageStream << "QS\n";
}

void QPSPrintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    Q_D(QPSPrintEngine);

    if (mode == WindingMode)
        d->pageStream << "/WFi true d\n";
    d->pageStream << "NP\n";
    d->pageStream << POINT(points[0]) << "MT\n";
    for(int i = 1; i < pointCount; i++) {
        d->pageStream << POINT(points[i]) << "LT\n";
    }
    if (mode == PolylineMode)
        d->pageStream << "QS\n";
    else
        d->pageStream << "CP BF QS\n";
    if (mode == WindingMode)
        d->pageStream << "/WFi false d\n";
}

void QPSPrintEngine::drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode)
{
    QPolygonF p;
    p.reserve(pointCount);
    for (int i=0; i<pointCount; ++i)
        p << points[i];
    drawPolygon(p.data(), pointCount, mode);
}

void QPSPrintEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr)
{
    Q_D(QPSPrintEngine);
    QImage img = pm.toImage();
    QImage mask;
    if (pm.hasAlphaChannel())
        mask = pm.mask().toImage();
    d->drawImage(r.x(), r.y(), r.width(), r.height(), img.copy(sr.toRect()), mask.copy(sr.toRect()));
}

void QPSPrintEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
    Q_D(QPSPrintEngine);
    const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);
    d->setFont(ti.fontEngine);
    Q_ASSERT(d->currentPSFont);
    if(d->currentPSFont) // better not crash in case somethig goes wrong.
        d->currentPSFont->drawText(d->pageStream, d, p, ti);
}

void QPSPrintEngine::drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &p)
{
    // ### Optimise implementation!
    qreal yPos = r.y();
    qreal yOff = p.y();
    while( yPos < r.y() + r.height() ) {
        qreal drawH = pixmap.height() - yOff;    // Cropping first row
        if ( yPos + drawH > r.y() + r.height() )        // Cropping last row
            drawH = r.y() + r.height() - yPos;
        qreal xPos = r.x();
        qreal xOff = p.x();
        while( xPos < r.x() + r.width() ) {
            qreal drawW = pixmap.width() - xOff; // Cropping first column
            if ( xPos + drawW > r.x() + r.width() )    // Cropping last column
                drawW = r.x() + r.width() - xPos;
            // ########
            painter()->drawPixmap( QPointF(xPos, yPos).toPoint(), pixmap,
                                   QRectF(xOff, yOff, drawW, drawH).toRect());
            xPos += drawW;
            xOff = 0;
        }
        yPos += drawH;
        yOff = 0;
    }

}

#define QT_PATH_ELEMENT(elm) elm.x << ' ' << elm.y << ' '

void QPSPrintEngine::drawPath(const QPainterPath &p)
{
    Q_D(QPSPrintEngine);
    bool winding = (p.fillRule() == Qt::WindingFill);

    if (winding)
        d->pageStream << "/WFi true d\n";
    d->pageStream << "NP\n";

    int start = -1;
    for (int i=0; i<p.elementCount(); ++i) {
        const QPainterPath::Element &elm = p.elementAt(i);
        switch (elm.type) {
        case QPainterPath::MoveToElement:
            if (start >= 0
                && p.elementAt(start).x == p.elementAt(i-1).x
                && p.elementAt(start).y == p.elementAt(i-1).y)
                d->pageStream << "CP\n";
            d->pageStream << QT_PATH_ELEMENT(elm) << "MT\n";
            start = i;
            break;
        case QPainterPath::LineToElement:
            d->pageStream << QT_PATH_ELEMENT(elm) << "LT\n";
            break;
        case QPainterPath::CurveToElement:
            Q_ASSERT(p.elementAt(i+1).type == QPainterPath::CurveToDataElement);
            Q_ASSERT(p.elementAt(i+2).type == QPainterPath::CurveToDataElement);
            d->pageStream << QT_PATH_ELEMENT(elm)
                          << QT_PATH_ELEMENT(p.elementAt(i+1))
                          << QT_PATH_ELEMENT(p.elementAt(i+2))
                          << "curveto\n";
            i += 2;
            break;
        default:
            qFatal("QPSPrintEngine::drawPath(), unhandled type: %d", elm.type);
        }
    }
    if (start >= 0
        && p.elementAt(start).x == p.elementAt(p.elementCount()-1).x
        && p.elementAt(start).y == p.elementAt(p.elementCount()-1).y)
        d->pageStream << "CP\n";

    d->pageStream << "BF QS\n";

    if (winding)
        d->pageStream << "/WFi false d\n";
}


bool QPSPrintEngine::newPage()
{
    Q_D(QPSPrintEngine);
    // we're writing to lp/lpr through a pipe, we don't want to crash with SIGPIPE
    // if lp/lpr dies
    ignoreSigPipe(true);
    if (!d->firstPage)
        d->flushPage();
    d->firstPage = false;
    ignoreSigPipe(false);

    // a restore undefines all the fonts that have been defined
    // inside the scope (normally within pages) and all the glyphs that
    // have been added in the scope.

    for (QHash<QString, QPSPrintEngineFont *>::Iterator it = d->fonts.begin();
         it != d->fonts.end(); ++it)
        (*it)->restore();

    if (!d->buffer) {
        d->pageFontNames.clear();
    }

    d->pageStream.flush();
    if (d->pageBuffer)
        delete d->pageBuffer;
    d->pageBuffer = new QBuffer();
    d->pageBuffer->open(QIODevice::WriteOnly);
    d->pageStream.setCodec(QTextCodec::codecForName("ISO-8859-1"));
    d->pageStream.setDevice(d->pageBuffer);

    d->currentFont = 0; // reset current font
    setDirty(AllDirty);
    d->pageFontNumber = d->headerFontNumber;

    return true;
}

bool QPSPrintEngine::abort()
{
    // ### abort!?!
    return false;
}

QRect QPSPrintEnginePrivate::paperRect() const
{
    PaperSize s = paperSizes[pageSize];
    int w = qRound(s.width*resolution/72.);
    int h = qRound(s.height*resolution/72.);
    if (orientation == QPrinter::Portrait)
        return QRect(0, 0, w, h);
    else
        return QRect(0, 0, h, w);
}

QRect QPSPrintEnginePrivate::pageRect() const
{
    QRect r = paperRect();
    if (fullPage)
        return r;
    // would be nice to get better margins than this.
    return QRect(resolution/3, resolution/3, r.width()-2*resolution/3, r.height()-2*resolution/3);
}

int  QPSPrintEngine::metric(QPaintDevice::PaintDeviceMetric metricType) const
{
    Q_D(const QPSPrintEngine);
    int val;
    QRect r = d->paperRect();
    switch (metricType) {
    case QPaintDevice::PdmWidth:
        val = r.width();
        break;
    case QPaintDevice::PdmHeight:
        val = r.height();
        break;
    case QPaintDevice::PdmDpiX:
        val = d->resolution;
        break;
    case QPaintDevice::PdmDpiY:
        val = d->resolution;
        break;
    case QPaintDevice::PdmPhysicalDpiX:
    case QPaintDevice::PdmPhysicalDpiY:
        val = 1200;
        break;
    case QPaintDevice::PdmWidthMM:
        val = qRound(r.width()*25.4/d->resolution);
        break;
    case QPaintDevice::PdmHeightMM:
        val = qRound(r.height()*25.4/d->resolution);
        break;
    case QPaintDevice::PdmNumColors:
        val = INT_MAX;
        break;
    case QPaintDevice::PdmDepth:
        val = 32;
        break;
    default:
        qWarning("QPrinter::metric: Invalid metric command");
        return 0;
    }
    return val;
}

QPrinter::PrinterState QPSPrintEngine::printerState() const
{
    Q_D(const QPSPrintEngine);
    return d->printerState;
}

void QPSPrintEngine::setProperty(PrintEnginePropertyKey key, const QVariant &value)
{
    Q_D(QPSPrintEngine);
    switch (key) {
    case PPK_CollateCopies:
        d->collate = value.toBool();
        break;
    case PPK_ColorMode:
        d->colorMode = QPrinter::ColorMode(value.toInt());
        break;
    case PPK_Creator:
        d->creator = value.toString();
        break;
    case PPK_DocumentName:
        d->docName = value.toString();
        break;
    case PPK_FullPage:
        d->fullPage = value.toBool();
        break;
    case PPK_NumberOfCopies:
        d->copies = value.toInt();
        break;
    case PPK_Orientation:
        d->orientation = QPrinter::Orientation(value.toInt());
        break;
    case PPK_OutputFileName:
        d->outputFileName = value.toString();
        break;
    case PPK_PageOrder:
        d->pageOrder = QPrinter::PageOrder(value.toInt());
        break;
    case PPK_PageSize:
        d->pageSize = QPrinter::PageSize(value.toInt());
        break;
    case PPK_PaperSource:
        d->paperSource = QPrinter::PaperSource(value.toInt());
        break;
    case PPK_PrinterName:
        d->printerName = value.toString();
        break;
    case PPK_PrinterProgram:
        d->printProgram = value.toString();
        break;
    case PPK_Resolution:
        d->resolution = value.toInt();
        break;
    case PPK_SelectionOption:
        d->selectionOption = value.toString();
        break;
    default:
        break;
    }
}

QVariant QPSPrintEngine::property(PrintEnginePropertyKey key) const
{
    Q_D(const QPSPrintEngine);
    QVariant ret;
    switch (key) {
    case PPK_CollateCopies:
        ret = d->collate;
        break;
    case PPK_ColorMode:
        ret = d->colorMode;
        break;
    case PPK_Creator:
        ret = d->creator;
        break;
    case PPK_DocumentName:
        ret = d->docName;
        break;
    case PPK_FullPage:
        ret = d->fullPage;
        break;
    case PPK_NumberOfCopies:
        ret = d->copies;
        break;
    case PPK_Orientation:
        ret = d->orientation;
        break;
    case PPK_OutputFileName:
        ret = d->outputFileName;
        break;
    case PPK_PageOrder:
        ret = d->pageOrder;
        break;
    case PPK_PageSize:
        ret = d->pageSize;
        break;
    case PPK_PaperSource:
        ret = d->paperSource;
        break;
    case PPK_PrinterName:
        ret = d->printerName;
        break;
    case PPK_PrinterProgram:
        ret = d->printProgram;
        break;
    case PPK_Resolution:
        ret = d->resolution;
        break;
    case PPK_SupportedResolutions:
        ret = QList<QVariant>() << 72;
        break;
    case PPK_PaperRect:
        ret = d->paperRect();
    case PPK_PageRect:
        ret = d->pageRect();
        break;
    case PPK_SelectionOption:
        ret = d->selectionOption;
        break;
    default:
        break;
    }
    return ret;
}

#endif // QT_NO_PRINTER
