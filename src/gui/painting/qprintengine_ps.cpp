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
#include <private/qprintengine_pdf_p.h>

// <X11/Xlib.h> redefines Status -> int
#if defined(Status)
# undef Status
#endif

#ifndef QT_NO_PRINTER

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
#include "qsettings.h"
#include "qmap.h"
#include "qbitmap.h"
#include "qregion.h"
#include <private/qunicodetables_p.h>
#include <private/qpainterpath_p.h>
#include <qdebug.h>
#include <private/qdrawhelper_p.h>

#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

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
"/BD{bind def}bind def/d2{dup dup}BD/ED{exch def}BD/D0{0 ED}BD/F{setfont}BD\n"
"/RL{rlineto}BD/NP{newpath}BD/CM{currentmatrix}BD/SM{setmatrix}BD/TR\n"
"{translate}BD/SD{setdash}BD/SC{aload pop setrgbcolor}BD/CR{currentfile read\n"
"pop}BD/i{index}BD/scs{setcolorspace}BD/DB{dict dup begin}BD/DE{end def}BD/ie\n"
"{ifelse}BD/gs{gsave}BD/gr{grestore}BD/w{setlinewidth}BD/d{setdash}BD/J\n"
"{setlinecap}BD/j{setlinejoin}BD/scn{3 array astore/BCol exch def}BD/SCN{3\n"
"array astore/PCol exch def}BD/cm{6 array astore concat}BD/m{moveto}BD/l\n"
"{lineto}BD/c{curveto}BD/h{closepath}BD/W{clip}BD/W*{eoclip}BD/n{}BD/q{gsave\n"
"10 dict begin}BD/Q{end grestore}BD/re{4 2 roll m dup 0 exch RL exch 0 RL 0\n"
"exch neg RL h}BD/S{gs PCol SC stroke gr}BD/BSt 0 def/LWi 0 def/WFi false def\n"
"/BCol[1 1 1]def/PCol[0 0 0]def/BkCol[1 1 1]def/BDArr[0.94 0.88 0.63 0.50\n"
"0.37 0.12 0.06]def/defM matrix def/level3{/languagelevel where{pop\n"
"languagelevel 3 ge}{false}ie}BD/QCIgray D0/QCIcolor D0/QCIindex D0/QCI{\n"
"/colorimage where{pop false 3 colorimage}{exec/QCIcolor ED/QCIgray QCIcolor\n"
"length 3 idiv string def 0 1 QCIcolor length 3 idiv 1 sub{/QCIindex ED/_x\n"
"QCIindex 3 mul def QCIgray QCIindex QCIcolor _x get 0.30 mul QCIcolor _x 1\n"
"add get 0.59 mul QCIcolor _x 2 add get 0.11 mul add add cvi put}for QCIgray\n"
"image}ie}BD/di{gs TR 1 i 1 eq{pop pop false 3 1 roll BCol SC imagemask}{dup\n"
"false ne{level3}{false}ie{/_ma ED 8 eq{/_dc[0 1]def/DeviceGray}{/_dc[0 1 0 1\n"
"0 1]def/DeviceRGB}ie scs/_im ED/_mt ED/_h ED/_w ED <</ImageType 3/DataDict\n"
"<</ImageType 1/Width _w/Height _h/ImageMatrix _mt/DataSource _im\n"
"/BitsPerComponent 8/Decode _dc >>/MaskDict <</ImageType 1/Width _w/Height _h\n"
"/ImageMatrix _mt/DataSource _ma/BitsPerComponent 1/Decode[0 1]>>\n"
"/InterleaveType 3 >> image}{pop 8 4 1 roll 8 eq{image}{QCI}ie}ie}ie gr}BD/BF\n"
"{gs BSt 1 eq{BCol SC WFi{fill}{eofill}ie}if BSt 2 ge BSt 8 le and{BDArr BSt\n"
"2 sub get/_sc ED BCol{1. exch sub _sc mul 1. exch sub}forall 3 array astore\n"
"SC WFi{fill}{eofill}ie}if BSt 9 ge BSt 14 le and{WFi{W}{W*}ie pathbbox 3 i 3\n"
"i TR 4 2 roll 3 2 roll exch sub/_h ED sub/_w ED BCol SC 0.3 w NP BSt 9 eq\n"
"BSt 11 eq or{0 4 _h{dup 0 exch m _w exch l}for}if BSt 10 eq BSt 11 eq or{0 4\n"
"_w{dup 0 m _h l}for}if BSt 12 eq BSt 14 eq or{_w _h gt{0 6 _w _h add{dup 0 m\n"
"_h sub _h l}for}{0 6 _w _h add{dup 0 exch m _w sub _w exch l}for}ie}if BSt\n"
"13 eq BSt 14 eq or{_w _h gt{0 6 _w _h add{dup _h m _h sub 0 l}for}{0 6 _w _h\n"
"add{dup _w exch m _w sub 0 exch l}for}ie}if S}if BSt 15 eq{}if BSt 24 eq{}if\n"
"gr}BD/f{/WFi true def BF}BD/f*{/WFi false def BF}BD/B{/WFi true def BF S}BD\n"
"/B*{/WFi false def BF S}BD/BC{/BkCol ED}BD/BR{/BCol ED/BSt ED}BD/NB{0[0 0 0]\n"
"BR}BD/PE{setlinejoin setlinecap/PCol ED/LWi ED/PSt ED PCol SC}BD/P1{1 0 3 2\n"
"roll 0 0 PE}BD/ST{defM SM concat}BD/MF{true exch true exch{exch pop exch pop\n"
"dup 0 get dup findfont dup/FontName get 3 -1 roll eq{exit}if}forall exch dup\n"
"1 get/fxscale ED 2 get/fslant ED exch/fencoding ED[fxscale 0 fslant 1 0 0]\n"
"makefont fencoding false eq{}{dup maxlength dict begin{1 i/FID ne{def}{pop\n"
"pop}ifelse}forall/Encoding fencoding def currentdict end}ie definefont pop}\n"
"BD/MFEmb{findfont dup length dict begin{1 i/FID ne{def}{pop pop}ifelse}\n"
"forall/Encoding ED currentdict end definefont pop}BD/DF{findfont/_fs 3 -1\n"
"roll def[_fs 0 0 _fs -1 mul 0 0]makefont def}BD/XYT{PCol SC m/xyshow where{\n"
"pop pop xyshow}{exch pop 1 i dup length 2 div exch stringwidth pop 3 -1 roll\n"
"exch sub exch div exch 0 exch ashow}ie}BD/AT{PCol SC m 1 i dup length 2 div\n"
"exch stringwidth pop 3 -1 roll exch sub exch div exch 0 exch ashow}BD/QI{/C\n"
"save def pageinit q}BD/QP{Q C restore showpage}BD/SPD{/setpagedevice where{\n"
"<< 3 1 roll >> setpagedevice}{pop pop}ie}BD/CLS{gs NP}BD/ACR{/_h ED/_w ED/_y\n"
"ED/_x ED _x _y m 0 _h RL _w 0 RL 0 _h neg RL h}BD/CLO{gr}BD\n";






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
    { "Arial", 0, 84.04 },
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
static QByteArray wrapDSC(const QByteArray &str)
{
    QByteArray dsc = str.simplified();
    const int wrapAt = 254;
    QByteArray wrapped;
    if (dsc.length() < wrapAt)
        wrapped = dsc;
    else {
        wrapped = dsc.left(wrapAt);
        QByteArray tmp = dsc.mid(wrapAt);
        while (tmp.length() > wrapAt-3) {
            wrapped += "\n%%+" + tmp.left(wrapAt-3);
            tmp = tmp.mid(wrapAt-3);
        }
        wrapped += "\n%%+" + tmp;
    }
    return wrapped + "\n";
}

static QByteArray toString(const qreal num)
{
    return QByteArray::number(num, 'f', 3);
}

// ----------------------------- Internal class declarations -----------------------------

class QPSPrintEngineFont;

class QPSPrintEnginePrivate : public QPaintEnginePrivate {
public:
    QPSPrintEnginePrivate(QPrinter::PrinterMode m);
    ~QPSPrintEnginePrivate();

    void emitHeader(bool finished);
    void emitPages();
    void setFont(QFontEngine *fe);
    void drawImage(qreal x, qreal y, qreal w, qreal h, const QImage &img, const QImage &mask);
    void flushPage(bool last = false);
    QRect paperRect() const;
    QRect pageRect() const;
    QRegion getClip();

    int         pageCount;
    bool        epsf;
    QByteArray     fontsUsed;

    // the device the output is in the end streamed to.
    QIODevice *outDevice;
    int fd;

    // stores the descriptions of the n first pages.
    QByteArray buffer;
    // buffer for the current page. Needed because we might have page fonts.
    QByteArray pageBuffer;
    QPdf::ByteStream *pageStream;

    QHash<QByteArray, QByteArray> fontNames;
    QHash<QByteArray, QPSPrintEngineFont *> fonts;
    QPSPrintEngineFont *currentPSFont;
    QFontEngine *currentFont;
    bool firstPage;
    int fontNumber;
    QByteArray fontBuffer;

    QRect boundingBox;

    Qt::BGMode backgroundMode;
    QBrush backgroundBrush;
    QPointF brushOrigin;
    QBrush brush;
    QPen pen;
    QList<QPainterPath> clips;
    bool clipEnabled;
    bool allClipped;
    bool hasPen;
    bool hasBrush;

    QPdf::Stroker stroker;

    QStringList fontpath;

    bool        collate;
    int         copies;
    QString printerName;
    QString outputFileName;
    QString selectionOption;
    QString printProgram;
    QString title;
    QString creator;
    QPrinter::Orientation orientation;
    QPrinter::PageSize pageSize;
    QPrinter::PageOrder pageOrder;
    int resolution;
    QPrinter::ColorMode colorMode;
    bool fullPage;
    QPrinter::PaperSource paperSource;
    QPrinter::PrinterState printerState;
    bool embedFonts;
};


class QPSPrintEngineFont {
public:
    QPSPrintEngineFont(QFontEngine *fe);
    virtual ~QPSPrintEngineFont();
    virtual QByteArray postScriptFontName() { return psname; }
    virtual QByteArray defineFont(const QByteArray &ps, const QByteArray &key,
                                  QPSPrintEnginePrivate *ptr, int pixelSize);
    virtual void download(QPdf::ByteStream& s, bool global);
    virtual void drawText(QPdf::ByteStream &stream, QPSPrintEnginePrivate *d, const QPointF &p, const QTextItemInt &ti);
    virtual unsigned short mapUnicode(unsigned short unicode);
    void downloadMapping(QPdf::ByteStream &s, bool global);
    QByteArray glyphName(unsigned short glyphindex, bool *glyphSet = 0);
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

    bool embedFonts;
protected:
    QByteArray psname;
    QList<QByteArray> replacementList;
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

static int addPsFontNameExtension(const QFontEngine *fe, QByteArray &ps, const psfont *psf = 0)
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

    if (psf) {
        ps = psf[type].psname;
    } else {
        switch (type) {
        case 1:
            ps.append("-Italic");
            break;
        case 2:
            ps.append("-Bold");
            break;
        case 3:
            ps.append("-BoldItalic");
            break;
        case 0:
        default:
            break;
        }
    }
    return type;
}

#ifdef QT_HAVE_FREETYPE
static FT_Face ft_face(const QFontEngine *engine)
{
#ifdef Q_WS_X11
    if (engine->type() == QFontEngine::Freetype) {
        const QFontEngineFT *ft = static_cast<const QFontEngineFT *>(engine);
        return ft->non_locked_face();
    }
#endif
#ifdef Q_WS_QWS
    if (engine->type() == QFontEngine::Freetype) {
        const QFontEngineFT *ft = static_cast<const QFontEngineFT *>(engine);
        return ft->face;
    }
#endif
    return 0;
}
#endif

static QByteArray makePSFontName(const QFontEngine *fe, int *listpos = 0, int *ftype = 0)
{
  QByteArray ps;
  int i;

#ifdef QT_HAVE_FREETYPE
  FT_Face face = ft_face(fe);
  if (face) {
      ps = FT_Get_Postscript_Name(face);
      if (listpos)
          *listpos = 0;
      if (ftype)
          *ftype = 0;
      if (!ps.isEmpty())
          return ps;
  }
#endif

  QByteArray family = fe->fontDef.family.toLower().toUtf8();

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
        ps[i] = ::toupper(ps[i]);
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
  QByteArray lowerName = ps.toLower();
  while(postscriptFonts[i].input &&
         postscriptFonts[i].input != lowerName)
    ++i;
  const psfont *psf = postscriptFonts[i].ps;

  int type = addPsFontNameExtension(fe, ps, psf);

  if (listpos)
      *listpos = i;
  if (ftype)
      *ftype = type;
  return ps;
}

static void appendReplacements(QList<QByteArray> &list, const psfont * const * replacements, int type, qreal xscale = 100.)
{
    // iterate through the replacement fonts
    while (*replacements) {
        const psfont *psf = *replacements;
        QByteArray ps;
        QPdf::ByteStream s(&ps);
        s << "[/"
          << psf[type].psname << " "
          << toString(xscale / psf[type].xscale) << " "
          << toString(psf[type].slant) << "]";
        list.append(ps);
        ++replacements;
    }
}

static QList<QByteArray> makePSFontNameList(const QFontEngine *fe, const QByteArray &psname = QByteArray(), bool useNameForLookup = false)
{
    int i;
    int type;
    QList<QByteArray> list;
    QByteArray ps;

    if (!psname.isEmpty() && !useNameForLookup) {
        QByteArray best = "[/" + psname + " 1.0 0.0]";
        list.append(best);
    }

    makePSFontName(fe, &i, &type);

    const psfont *psf = postscriptFonts[i].ps;
    const psfont * const * replacements = postscriptFonts[i].replacements;
    qreal xscale = 100;
    if (psf) {
        // xscale for the "right" font is always 1. We scale the replacements...
        xscale = psf->xscale;
        ps = QByteArray();
        QPdf::ByteStream s(&ps);
        s << "[/" << psf[type].psname << " 1.0 "
          << toString(psf[type].slant) << "]";
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

static void emitPSFontNameList(QPdf::ByteStream &s, const QByteArray &psname, const QList<QByteArray> &list)
{
    s << "/" << psname << "List [\n";
    for (int i = 0; i < list.size(); ++i)
        s << list.at(i) << "\n";
    s << "] def\n";
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

static inline const char *toHex(uchar u, char *buffer)
{
    int i = 1;
    while (i >= 0) {
        ushort hex = (u & 0x000f);
        if (hex < 0x0a)
            buffer[i] = '0'+hex;
        else
            buffer[i] = 'A'+(hex-0x0a);
        u = u >> 4;
        i--;
    }
    buffer[2] = '\0';
    return buffer;
}

static const char *toHex(ushort u, char *buffer)
{
    int i = 3;
    while (i >= 0) {
        ushort hex = (u & 0x000f);
        if (hex < 0x0a)
            buffer[i] = '0'+hex;
        else
            buffer[i] = 'A'+(hex-0x0a);
        u = u >> 4;
        i--;
    }
    buffer[4] = '\0';
    return buffer;
}

void QPSPrintEngineFont::drawText(QPdf::ByteStream &stream, QPSPrintEnginePrivate *, const QPointF &p, const QTextItemInt &ti)
{
    char buffer[5];
    qreal x = p.x();
    qreal y = p.y();

    int len = ti.num_chars;
    if (len == 0)
        return;

    stream << "<";
    if (ti.flags & QTextItem::RightToLeft) {
        for (int i = len-1; i >=0; i--)
            stream << toHex(mapUnicode(ti.chars[i].unicode()), buffer);
    } else {
        for (int i = 0; i < len; i++)
            stream << toHex(mapUnicode(ti.chars[i].unicode()), buffer);
    }
    stream << ">";

    stream << ti.width.toReal() << x << y << " AT\n";
}


QByteArray QPSPrintEngineFont::defineFont(const QByteArray &ps,
                                          const QByteArray &key, QPSPrintEnginePrivate *d, int pixelSize)
{
    QByteArray fontName = "/";
    fontName += ps;
    fontName += "-Uni";

    ++d->fontNumber;
    QPdf::ByteStream s(&d->fontBuffer);
    s << "/F" << d->fontNumber << " "
      << pixelSize << fontName << " DF\n";
    fontName = "F" + QByteArray::number(d->fontNumber);
    d->fontNames.insert(key, fontName);
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


QByteArray QPSPrintEngineFont::glyphName(unsigned short glyphindex, bool *glyphSet)
{
    QByteArray glyphname;
    int l = 0;
    unsigned short unicode = unicode_for_glyph(glyphindex);
    if (symbol && unicode < 0x100) {
        // map from latin1 to symbol
        unicode = symbol_map[unicode];
    }
    if (!unicode && glyphindex) {
        char buffer[5];
        glyphname = "gl";
        glyphname += toHex(glyphindex, buffer);
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
                        char buffer[5];
                        glyphname = "uni";
                        glyphname += toHex(unicode, buffer);
                    }
                }
            }
        } else {
            char buffer[5];
            glyphname = "uni";
            glyphname += toHex(unicode, buffer);
        }
    }
    return glyphname;
}

void QPSPrintEngineFont::download(QPdf::ByteStream &s, bool global)
{
    //printf("defining mapping for printer font %s\n",psname.latin1());
    downloadMapping(s, global);
}

void QPSPrintEngineFont::downloadMapping(QPdf::ByteStream &s, bool global)
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

    QByteArray vector;
    QByteArray glyphname;

    for (range=0; range < numRanges; range++) {
        //printf("outputting range %04x\n",range*256);
        vector = "%% Font Page ";
        char buffer[5];
        vector += toHex((uchar)(range + rangeOffset), buffer);
        vector += "\n/";
        vector += psname;
        vector += "-ENC-";
        vector += toHex((uchar)(range + rangeOffset), buffer);
        vector += " [\n";

        QByteArray line;
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
        char buffer1[5];
        char buffer2[5];
        s << "/"
          << psname
          << "-Uni-"
          << toHex((uchar)(range + rangeOffset), buffer1)
          << " "
          << psname
          << "-ENC-"
          << toHex((uchar)(range + rangeOffset), buffer2);
        if (embedded() && embedFonts) {
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
        char buffer[5];
        s << "/"
          << psname
          << "-Uni-"
          << toHex(range, buffer)
          << " findfont\n";
    }
    s << "]def\n"

        // === trailer ===

        "FontName currentdict end definefont pop\n"
        "%%EndFont\n";
}

#if defined(QT_HAVE_FREETYPE) && !defined(QT_NO_FREETYPE)

class QPSPrintEngineFontFT : public QPSPrintEngineFont {
public:
    QPSPrintEngineFontFT(QFontEngine *f);
    virtual void download(QPdf::ByteStream& s, bool global);
    virtual void drawText(QPdf::ByteStream &stream, QPSPrintEnginePrivate *d, const QPointF &p, const QTextItemInt &ti);
    ~QPSPrintEngineFontFT();

    virtual bool embedded() { return true; }
private:
    FT_Face face;
    QVector<ushort> glyph2uni; // to speed up lookups

    void uni2glyphSetup();
    void charproc(int glyph, QPdf::ByteStream& s);
    unsigned short unicode_for_glyph(int glyphindex);
    unsigned short glyph_for_unicode(unsigned short unicode);
};

QPSPrintEngineFontFT::QPSPrintEngineFontFT(QFontEngine *f)
    : QPSPrintEngineFont(f)
{
    face = ft_face(f);
    Q_ASSERT(face);

    psname = FT_Get_Postscript_Name(face);
    replacementList = makePSFontNameList(f, psname);
}

void QPSPrintEngineFontFT::download(QPdf::ByteStream& s, bool global)
{
    emitPSFontNameList(s, psname, replacementList);

    if (!embedFonts) {
        downloadMapping(s, global);
        return;
    }

    //qDebug("downloading ttf font %s", psname.latin1());
    //qDebug("target type=%d", target_type);
    global_dict = global;
    QMap<unsigned short, unsigned short> *subsetDict = &subset;
    if (!global)
        subsetDict = &page_subset;

    downloaded  = true;

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

        "/m{moveto}bind def\n"
        "/l{lineto}bind def\n"
        "/h{closepath eofill}bind def\n"
        "/c{curveto}bind def\n"
        "/scd{7 -1 roll{setcachedevice}{pop pop pop pop pop pop}ifelse}bind def\n"
        "/e{exec}bind def\n"

        "/FontName /"
      << psname
      << " def\n"
        "/PaintType 0 def\n"
        "/FontMatrix[" << qreal(1.)/face->units_per_EM << "0 0 " << qreal(1.)/face->units_per_EM << "0 0]def\n"

        "/FontBBox["
      << (int)face->bbox.xMin
      << (int)face->bbox.yMin
      << (int)face->bbox.xMax
      << (int)face->bbox.yMax
      << "]def\n"

        "/FontType 3 def\n"

    // === write encoding ===

        "/Encoding StandardEncoding def\n";

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
    s << ">> readonly def\n";
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
        s << "}def\n";
    }

    s << "end readonly def\n"

        // === trailer ===

        "\n"

        "/BuildGlyph\n"
        " {exch begin\n"              /* start font dictionary */
        " CharStrings exch\n"
        " 2 copy known not{pop /.notdef}if\n"
        " true 3 1 roll get exec\n"
        " end}def\n"

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

void QPSPrintEngineFontFT::drawText(QPdf::ByteStream &stream, QPSPrintEnginePrivate *, const QPointF &p, const QTextItemInt &ti)
{
    char buffer[5];
    QFixed x = QFixed::fromReal(p.x());
    QFixed y = QFixed::fromReal(p.y());

    QByteArray xyarray;
    QFixed xo;
    QFixed yo;

    QGlyphLayout *glyphs = ti.glyphs;

    int len;
    len = ti.num_glyphs;
    if (len == 0)
        return;

    stream << "<";
    if (ti.flags & QTextItem::RightToLeft) {
        stream << toHex(mapUnicode(glyphs[len-1].glyph), buffer);
        QFixed last_advance = glyphs[len-1].advance.x;
        for (int i = len-2; i >=0; i--) {
            // map unicode is not really the correct name, as we map glyphs, but we also download glyphs, so this works
            if (glyphs[i].nKashidas) {
                QChar ch(0x640); // Kashida character
                QGlyphLayout g[8];
                int nglyphs = 7;
                ti.fontEngine->stringToCMap(&ch, 1, g, &nglyphs, 0);
                for (uint k = 0; k < glyphs[i].nKashidas; ++k) {
                    char buffer[5];
                    xyarray += " ";
                    xyarray += QByteArray::number((yo + g[0].offset.y).toReal());
                    xyarray += " ";
                    stream << toHex(mapUnicode(g[0].glyph), buffer);
                    last_advance = g[0].advance.x;
                    xo = -g[0].offset.x;
                    yo = -g[0].offset.y;
                }
            }
            xyarray += QByteArray::number((xo + glyphs[i].offset.x + last_advance).toReal());
            xyarray += " ";
            xyarray += QByteArray::number((yo + glyphs[i].offset.y).toReal());
            xyarray += " ";
            stream << toHex(mapUnicode(glyphs[i].glyph), buffer);
            xo = -glyphs[i].offset.x;
            yo = -glyphs[i].offset.y;
            last_advance = glyphs[i].advance.x + QFixed::fromFixed(glyphs[i].nKashidas ? 0 : glyphs[i].space_18d6);
        }
    } else {
        stream << toHex(mapUnicode(glyphs[0].glyph), buffer);
        for (int i = 1; i < len; i++) {
            // map unicode is not really the correct name, as we map glyphs, but we also download glyphs, so this works
            stream << toHex(mapUnicode(glyphs[i].glyph), buffer);
            xyarray += QByteArray::number((xo + glyphs[i].offset.x + glyphs[i-1].advance.x + QFixed::fromFixed(glyphs[i-1].space_18d6)).toReal());
            xyarray += " ";
            xyarray += QByteArray::number((yo + glyphs[i].offset.y).toReal());
            xyarray += " ";
            xo = -glyphs[i].offset.x;
            yo = -glyphs[i].offset.y;
        }
    }
    stream << ">\n"
        "[" << xyarray << "0 0]"
           << ti.width.toReal() << x.toReal() << y.toReal() << "XYT\n";

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
    QPdf::ByteStream *s;
};

static int move_to(FT_Vector *to, void *data)
{
    decompose_data *d = (decompose_data *)data;
    *d->s << (int)to->x
          << (int)to->y
          << "m\n";

    d->x = to->x;
    d->y = to->y;
    return 0;
}

static int line_to(FT_Vector *to, void *data)
{
    decompose_data *d = (decompose_data *)data;
    *d->s << (int)to->x
          << (int)to->y
          << "l\n";

    d->x = to->x;
    d->y = to->y;
    return 0;
}

static int conic_to(FT_Vector *control, FT_Vector *to, void *data)
{
    decompose_data *d = (decompose_data *)data;
    qreal cx[2], cy[2];

    cx[0] = (2*control->x + d->x)/3.;
    cy[0] = (2*control->y + d->y)/3.;
    cx[1] = (to->x + 2*control->x)/3.;
    cy[1] = (to->y + 2*control->y)/3.;

    *d->s << cx[0]
          << cy[0]
          << cx[1]
          << cy[1]
          << (int)to->x
          << (int)to->y
          << "c\n";

    d->x = to->x;
    d->y = to->y;
    return 0;
}

static int cubic_to(FT_Vector *control1, FT_Vector *control2, FT_Vector *to, void *data)
{
    decompose_data *d = (decompose_data *)data;

    *d->s << (int)control1->x
          << (int)control1->y
          << (int)control2->x
          << (int)control2->y
          << (int)to->x
          << (int)to->y
          << "c\n";

    d->x = to->x;
    d->y = to->y;
    return 0;
}



void QPSPrintEngineFontFT::charproc(int glyph, QPdf::ByteStream& s)
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

    s << (int)face->glyph->metrics.horiAdvance
      << (int)0
      << (int)bounds.xMin
      << (int)bounds.yMin
      << (int)bounds.xMax
      << (int)bounds.yMax
      << "scd\n";

    FT_Outline_Decompose(&face->glyph->outline, &outline_funcs, &d);
    s << "h";
}
#endif

// ================== AFontFileNotFound ============



class QPSPrintEngineFontNotFound
    : public QPSPrintEngineFont {
public:
    QPSPrintEngineFontNotFound(QFontEngine* f);
    virtual void download(QPdf::ByteStream& s, bool global);
};

QPSPrintEngineFontNotFound::QPSPrintEngineFontNotFound(QFontEngine* f)
    : QPSPrintEngineFont(f)
{
    psname = makePSFontName(f);
    replacementList = makePSFontNameList(f);
}

void QPSPrintEngineFontNotFound::download(QPdf::ByteStream& s, bool)
{
    //qDebug("downloading not found font %s", psname.latin1());
    emitPSFontNameList(s, psname, replacementList);
    s << "% No embeddable font for "
      << psname
      << " found\n";
    QPSPrintEngineFont::download(s, true);
}

// =================== Multi font engine ================

class QPSPrintEngineFontMulti
    : public QPSPrintEngineFont {
public:
    QPSPrintEngineFontMulti(QFontEngine* f);
    virtual QByteArray defineFont(const QByteArray &ps, const QByteArray &key,
                                  QPSPrintEnginePrivate *ptr, int pixelSize);
    virtual void download(QPdf::ByteStream& s, bool global);
    virtual void drawText(QPdf::ByteStream &stream, QPSPrintEnginePrivate *d, const QPointF &p, const QTextItemInt &ti);
};

QPSPrintEngineFontMulti::QPSPrintEngineFontMulti(QFontEngine* f)
    : QPSPrintEngineFont(f)
{
    Q_ASSERT(f->type() == QFontEngine::Multi);
    psname = "Multi:" + f->fontDef.family.toUtf8();
    psname += char(f->fontDef.style);
    psname += char(f->fontDef.weight);
}

QByteArray QPSPrintEngineFontMulti::defineFont(const QByteArray &, const QByteArray &,
                                            QPSPrintEnginePrivate *, int)
{
    return QByteArray();
}

void QPSPrintEngineFontMulti::download(QPdf::ByteStream&, bool)
{
}

void QPSPrintEngineFontMulti::drawText(QPdf::ByteStream &stream, QPSPrintEnginePrivate *d, const QPointF &p, const QTextItemInt &ti)
{
    QFontEngineMulti *multi = static_cast<QFontEngineMulti *>(ti.fontEngine);
    QGlyphLayout *glyphs = ti.glyphs;
    int which = glyphs[0].glyph >> 24;

    QFixed x = QFixed::fromReal(p.x());
    QFixed y = QFixed::fromReal(p.y());

    int start = 0;
    int end, i;
    for (end = 0; end < ti.num_glyphs; ++end) {
        const int e = glyphs[end].glyph >> 24;
        if (e == which)
            continue;

        // set the high byte to zero
        for (i = start; i < end; ++i)
            glyphs[i].glyph = glyphs[i].glyph & 0xffffff;

        // draw the text
        QTextItemInt ti2 = ti;
        ti2.glyphs = ti.glyphs + start;
        ti2.num_glyphs = end - start;
        ti2.fontEngine = multi->engine(which);
        ti2.f = ti.f;
        d->setFont(ti2.fontEngine);
        if(d->currentPSFont) // better not crash in case somethig goes wrong.
            d->currentPSFont->drawText(stream, d, QPointF(x.toReal(), y.toReal()), ti2);

        // reset the high byte for all glyphs and advance to the next sub-string
        const int hi = which << 24;
        for (i = start; i < end; ++i) {
            glyphs[i].glyph = hi | glyphs[i].glyph;
            x += glyphs[i].advance.x;
        }

        // change engine
        start = end;
        which = e;
    }

    // set the high byte to zero
    for (i = start; i < end; ++i)
        glyphs[i].glyph = glyphs[i].glyph & 0xffffff;

    // draw the text
    QTextItemInt ti2 = ti;
    ti2.glyphs = ti.glyphs + start;
    ti2.num_glyphs = end - start;
    ti2.fontEngine = multi->engine(which);
    ti2.f = ti.f;
    d->setFont(ti2.fontEngine);
    if(d->currentPSFont) // better not crash in case somethig goes wrong.
        d->currentPSFont->drawText(stream, d, QPointF(x.toReal(), y.toReal()), ti2);

    // reset the high byte for all glyphs
    const int hi = which << 24;
    for (i = start; i < end; ++i)
        glyphs[i].glyph = hi | glyphs[i].glyph;
}


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
    : outDevice(0), fd(-1), pageStream(0),
      collate(false), copies(1), orientation(QPrinter::Portrait),
      pageSize(QPrinter::A4), pageOrder(QPrinter::FirstPageFirst), colorMode(QPrinter::Color),
      fullPage(false), printerState(QPrinter::Idle)
{
    backgroundMode = Qt::TransparentMode;

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

#ifndef QT_NO_SETTINGS
    QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
    settings.beginGroup(QLatin1String("Qt"));
    embedFonts = settings.value(QLatin1String("embedFonts"), true).toBool();
#else
    embedFonts = true;
#endif
#if defined(Q_WS_X11) && defined(QT_NO_FONTCONFIG)
    if (!X11->use_xrender)
        fontpath = ::fontPath();
#endif
}

QPSPrintEnginePrivate::~QPSPrintEnginePrivate()
{
    qDeleteAll(fonts);
    delete pageStream;
}

void QPSPrintEnginePrivate::setFont(QFontEngine *fe)
{
    Q_ASSERT(fe);
    if (currentFont == fe && currentPSFont)
        return;
    currentFont = fe;

    QByteArray fontKey;

    QFontEngine::Type fontType = fe->type();
    bool embed = false;
    bool multi = false;

    if (fontType == QFontEngine::Multi) {
        fontKey = "Multi:" + fe->fontDef.family.toUtf8();
        fontKey += char(fe->fontDef.style);
        fontKey += char(fe->fontDef.weight);
        multi = true;
    }
#if defined(QT_HAVE_FREETYPE) && !defined(QT_NO_FREETYPE)
    else {
#ifdef Q_WS_X11
#ifndef QT_NO_FONTCONFIG
        if (X11->use_xrender && fontType == QFontEngine::Freetype && FT_IS_SCALABLE(ft_face(fe))) {
            FcPattern *pattern = static_cast<QFontEngineFT *>(fe)->pattern();
            FcChar8 *filename = 0;
            FcPatternGetString (pattern, FC_FILE, 0, &filename);
            //qDebug("filename for font is '%s'", filename);
            if (filename)
                fontKey = (const char *)filename;
            embed = true;
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
            if (!fontKey.isEmpty()) {
                fontKey = ::fontFile(fontpath, fontKey);
                embed = true;
            }
        }
#endif
#endif
        // ######## add embedded here
    }
#endif

    if (fontKey.isEmpty())
        fontKey = "NonEmbed:" + makePSFontName(fe);

    Q_ASSERT(!fontKey.isEmpty());
    currentPSFont = fonts.value(fontKey);

    if (!currentPSFont) {
#if defined(QT_HAVE_FREETYPE) && !defined(QT_NO_FREETYPE)
        if (embed) {
            currentPSFont = new QPSPrintEngineFontFT(fe);
        } else
#endif
        if (multi) {
            currentPSFont = new QPSPrintEngineFontMulti(fe);
        } else {
            // ### add Han handling
            currentPSFont = new QPSPrintEngineFontNotFound(fe);
        }
        if (currentPSFont->postScriptFontName() == "Symbol")
            currentPSFont->setSymbol();

        // this is needed to make sure we don't get the same postscriptname twice
        for (QHash<QByteArray, QPSPrintEngineFont *>::ConstIterator it = fonts.constBegin(); it != fonts.constEnd(); ++it) {
            if (*(*it) == *currentPSFont) {
                qWarning("Post script driver: font already in dict");
                delete currentPSFont;
                currentPSFont = *it;
            }
        }
        fonts[fontKey] = currentPSFont;
    }

    currentPSFont->embedFonts = embedFonts;

    QByteArray ps = currentPSFont->postScriptFontName();

    QByteArray key = fontKey + '/' + toString(fe->fontDef.pixelSize);
    QByteArray tmp = fontNames.value(key);

    QByteArray fontName;
    if (!tmp.isNull())
        fontName = tmp;

    if (fontName.isEmpty())
        fontName = currentPSFont->defineFont(ps, key, this, fe->fontDef.pixelSize);

    if (!fontName.isEmpty()) {
        *pageStream << fontName << " F\n";
    }

    ps.append(' ');
    ps.prepend(' ');
    if (!fontsUsed.contains(ps) && !ps.startsWith(" Multi:") && !ps.startsWith(" NonEmbed:"))
        fontsUsed += ps;
}


static void ps_r7(QPdf::ByteStream& stream, const char * s, int l)
{
    int i = 0;
    uchar line[80];
    int col = 0;

    while(i < l) {
        line[col++] = s[i++];
        if (i < l - 1 && col >= 76) {
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
    int written = 0;
    while (1) {
        bool flush = (i == input.size());
        if (!flush) {
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
        }
        if (flush || i - start == 128) {
            int size = i - start;
            if (state == Equal) {
                out.append((char)(uchar)(257-size));
                out.append(last);
                written += size;
            } else {
                out.append((char)(uchar)size-1);
                while (start < i)
                    out.append(data[start++]);
                written += size;
            }
            state = Undef;
            start = i;
            if (i == input.size())
                break;
        }
        last = data[i];
        ++i;
    };
    out.append((char)(uchar)128);
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
    if (remaining) {
        uint val = 0;
        for (int i = isize; i < input.size(); ++i)
            val = (val << 8) + in[i];
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
    }
    *(out++) = '~';
    *(out++) = '>';
    output.resize(out-output.data());
    return output;
}

static QByteArray compress(const QImage &img, bool gray) {
    // we can't use premultiplied here
    QImage image = img;
    if (image.format() == QImage::Format_ARGB32_Premultiplied)
        image = image.convertToFormat(QImage::Format_ARGB32);
    int width = image.width();
    int height = image.height();
    int depth = image.depth();
    int size = width*height;

    if (depth == 1)
        size = (width+7)/8*height;
    else if (!gray)
        size = size*3;

    QByteArray pixelData;
    pixelData.resize(size);
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
        for(int y=0; y < height; y++) {
            QRgb * s = (QRgb*)(image.scanLine(y));
            for(int x=0; x < width; x++) {
                QRgb rgb = (*s++);
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

    QByteArray runlength = runlengthEncode(pixelData);
    QByteArray outarr = ascii85Encode(runlength);
    return outarr;
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
            *pageStream << "/mask currentfile/ASCII85Decode filter/RunLengthDecode filter "
                        << size << " string readstring\n";
            ps_r7(*pageStream, out, out.size());
            *pageStream << " pop def\n";
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
        *pageStream << "/sl currentfile/ASCII85Decode filter/RunLengthDecode filter "
                    << size << " string readstring\n";
        ps_r7(*pageStream, out, out.size());
        *pageStream << " pop def\n";
        *pageStream << width << ' ' << height << "[" << scaleX << " 0 0 " << scaleY << " 0 0]sl "
                    << bits << (!mask.isNull() ? "mask " : "false ")
                    << x << ' ' << y << " di\n";
    }
}

void QPSPrintEnginePrivate::emitHeader(bool finished)
{
    QPSPrintEngine *q = static_cast<QPSPrintEngine *>(q_ptr);
    QPrinter *printer = static_cast<QPrinter*>(pdev);

    if (creator.isEmpty())
        creator = QLatin1String("Qt " QT_VERSION_STR);
    outDevice = new QFile();
    static_cast<QFile *>(outDevice)->open(fd, QIODevice::WriteOnly);

    QByteArray header;
    QPdf::ByteStream s(&header);
    s << "%!PS-Adobe-1.0";

    qreal scale = 72. / ((qreal) q->metric(QPaintDevice::PdmDpiY));
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
            s << " EPSF-3.0\n%%BoundingBox: "
              << (int)(printer->height() - boundingBox.bottom())*scale << " " // llx
              << (int)(printer->width() - boundingBox.right())*scale - 1 << " " // lly
              << (int)(printer->height() - boundingBox.top())*scale + 1 << " " // urx
              << (int)(printer->width() - boundingBox.left())*scale; // ury
        } else {
            if (!fullPage)
                boundingBox.translate(mleft, -mtop);
            s << " EPSF-3.0\n%%BoundingBox: "
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
            s << "\n%%BoundingBox: 0 0 " << h << " " << w;
        else
            s << "\n%%BoundingBox: 0 0 " << w << " " << h;
    }
    s << "\n" << wrapDSC("%%Creator: " + creator.toUtf8());
    if (!title.isEmpty())
        s << wrapDSC("%%Title: " + title.toUtf8());
#ifndef QT_NO_DATESTRING
    s << "%%CreationDate: " << QDateTime::currentDateTime().toString().toUtf8();
#endif
    s << "\n%%Orientation: ";
    if (orientation == QPrinter::Landscape)
        s << "Landscape";
    else
        s << "Portrait";

    s << "\n%%Pages: (atend)"
        "\n%%DocumentFonts: (atend)"
        "\n%%EndComments\n"

        "%%BeginProlog\n"
        "% Prolog copyright 1994-2003 Trolltech. You may copy this prolog in any way\n"
        "% that is directly related to this document. For other use of this prolog,\n"
        "% see your licensing agreement for Qt.\n"
      << ps_header << "\n";

    // we have to do this here, as scaling can affect this.
    QByteArray lineStyles = "/LArr["                                       // Pen styles:
                            " [] []"                       //   solid line
                            " [w s] [s w]"                 //   dash line
                            " [s s] [s s]"                  //   dot line
                            " [m s s s] [s m s s]"      //   dash dot line
                            " [m s s s s] [s m s s s s]"         //   dash dot dot line
                            "] def\n";
    lineStyles.replace("w", toString(10./scale));
    lineStyles.replace("m", toString(5./scale));
    lineStyles.replace("s", toString(3./scale));

    s << lineStyles;

    s << "/pageinit {\n";
    if (!fullPage) {
        if (orientation == QPrinter::Portrait)
            s << mleft*scale << " "
              << mbottom*scale << " translate\n";
        else
            s << mtop*scale << " "
              << mleft*scale << " translate\n";
    }
    if (orientation == QPrinter::Portrait) {
        s << "% " << printer->widthMM() << "*" << printer->heightMM()
          << "mm (portrait)\n0 " << height*scale
          << " translate " << scale << " -" << scale << " scale/defM matrix CM def } def\n";
    } else {
        s << "% " << printer->heightMM() << "*" << printer->widthMM()
          << " mm (landscape)\n 90 rotate " << scale << " -" << scale << " scale/defM matrix CM def } def\n";
    }
    s << "%%EndProlog\n";


    s << "%%BeginSetup\n";
    if (copies > 1) {
        s << "/#copies " << copies << " def\n";
        s << "/NumCopies " << copies << " SPD\n";
        s << "/Collate " << (collate ? "true" : "false") << " SPD\n";
    }
    s << "%%EndSetup\n";

    outDevice->write(header);

}


void QPSPrintEnginePrivate::emitPages()
{
    // ############# fix fonts for huge documents
    if (!fontBuffer.isEmpty()) {
        QByteArray data;
        QPdf::ByteStream ds(&data);
        for (QHash<QByteArray, QPSPrintEngineFont *>::Iterator it = fonts.begin(); it != fonts.end(); ++it)
            (*it)->download(ds, true); // true means its global
        outDevice->write(data);
        outDevice->write(fontBuffer);
        fontBuffer = QByteArray();
    }
    outDevice->write(buffer);

    buffer = QByteArray();
    fontBuffer = QByteArray();
    qDeleteAll(fonts);
    fonts.clear();
    currentPSFont = 0;
    currentFont = 0;
}


#ifdef Q_WS_QWS
const int max_in_memory_size = 50000000;
#else
const int max_in_memory_size = 2000000;
#endif

void QPSPrintEnginePrivate::flushPage(bool last)
{
    if (!last && pageBuffer.isEmpty())
        return;
    QPdf::ByteStream s(&buffer);
    s << "%%Page: "
      << pageCount << pageCount << "\n"
      << "QI\n"
      << pageBuffer
      << "\nQP\n";
    pageBuffer = QByteArray();
    if (last) { // ############## || buffer.size() > max_in_memory_size) {
//        qDebug("emiting header at page %d", pageCount);
        if (!outDevice)
            emitHeader(last);
        emitPages();
    }
    pageCount++;
}

// ================ PSPrinter class ========================

QPSPrintEngine::QPSPrintEngine(QPrinter::PrinterMode m)
    : QPaintEngine(*(new QPSPrintEnginePrivate(m)),
                   PrimitiveTransform
                   | PatternTransform
                   | PixmapTransform
                   | PainterPaths
                   | PatternBrush
       )
{
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

QPSPrintEngine::~QPSPrintEngine()
{
    Q_D(QPSPrintEngine);
    if (d->fd >= 0)
        ::close(d->fd);
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
    i = (int)sysconf(_SC_OPEN_MAX);
#elif defined(_POSIX_OPEN_MAX)
    i = (int)_POSIX_OPEN_MAX;
#elif defined(OPEN_MAX)
    i = (int)OPEN_MAX;
#else
    i = 256;
#endif
    while(--i > 0)
	::close(i);
}

bool QPSPrintEngine::begin(QPaintDevice *pdev)
{
    Q_D(QPSPrintEngine);

    if (d->fd >= 0)
        return true;

    d->pdev = pdev;
    if (!d->outputFileName.isEmpty()) {
        d->fd = QT_OPEN(d->outputFileName.toLocal8Bit().constData(), O_CREAT | O_NOCTTY | O_TRUNC | O_WRONLY,
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
        if (pipe(fds) != 0) {
            qWarning("QPSPrinter: could not open pipe to print");
            return false;
        }

        pid_t pid = fork();
        if (pid == 0) {       // child process
            // if possible, exit quickly, so the actual lp/lpr
            // becomes a child of init, and ::waitpid() is
            // guaranteed not to wait.
            if (fork() > 0) {
                closeAllOpenFds();

                // try to replace this process with "true" - this prevents
                // global destructors from being called (that could possibly
                // do wrong things to the parent process)
                (void)execlp("true", "true", (char *)0);
                (void)execl("/bin/true", "true", (char *)0);
                (void)execl("/usr/bin/true", "true", (char *)0);
                ::exit(0);
            }
            dup2(fds[0], 0);

            closeAllOpenFds();

            if (!d->printProgram.isEmpty()) {
                if (!d->selectionOption.isEmpty())
                    pr.prepend(d->selectionOption);
                else
                    pr.prepend(QLatin1String("-P"));
                (void)execlp(d->printProgram.toLocal8Bit().data(), d->printProgram.toLocal8Bit().data(),
                              pr.toLocal8Bit().data(), (char *)0);
            } else {
                // if no print program has been specified, be smart
                // about the option string too.
                QList<QByteArray> lprhack;
                QList<QByteArray> lphack;
                QByteArray media;
                if (!pr.isEmpty() || !d->selectionOption.isEmpty()) {
                    if (!d->selectionOption.isEmpty()) {
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
#ifndef Q_OS_OSF
                if (psToStr[d->pageSize]) {
                    lpargs[++i] = "-o";
                    lpargs[++i] = (char *)psToStr[d->pageSize];
                    lpargs[++i] = "-o";
                    media = "media=";
                    media += psToStr[d->pageSize];
                    lpargs[++i] = (char *)media.constData();
                }
#endif
                lpargs[++i] = 0;
                char **lprargs = new char *[lprhack.size()+2];
                lprargs[0] = "lpr";
                for (int i = 0; i < lprhack.size(); ++i)
                    lprargs[i+1] = (char *)lprhack[i].constData();
                lprargs[lprhack.size() + 1] = 0;
                (void)execvp("lp", lpargs);
                (void)execvp("lpr", lprargs);
                (void)execv("/bin/lp", lpargs);
                (void)execv("/bin/lpr", lprargs);
                (void)execv("/usr/bin/lp", lpargs);
                (void)execv("/usr/bin/lpr", lprargs);
            }
            // if we couldn't exec anything, close the fd,
            // wait for a second so the parent process (the
            // child of the GUI process) has exited.  then
            // exit.
            ::close(0);
            (void)::sleep(1);
            ::exit(0);
        }
        // parent process
        ::close(fds[0]);
        d->fd = fds[1];
    }
    if (d->fd < 0)
        return false;

    d->fontNumber = 0;
    d->pageCount = 1;                // initialize state

    d->pen = QPen(Qt::black);
    d->brush = Qt::NoBrush;
    d->hasPen = true;
    d->hasBrush = false;
    d->clipEnabled = false;
    d->allClipped = false;
    d->boundingBox = QRect();
    d->fontsUsed = "";

    setActive(true);

    newPage();

    return true;
}

bool QPSPrintEngine::end()
{
    Q_D(QPSPrintEngine);

    // we're writing to lp/lpr through a pipe, we don't want to crash with SIGPIPE
    // if lp/lpr dies
    ignoreSigPipe(true);
    d->flushPage(true);
    QByteArray trailer;
    QPdf::ByteStream s(&trailer);
    s << "%%Trailer\n";
    s << "%%Pages: " << d->pageCount - 1 << "\n" <<
        wrapDSC("%%DocumentFonts: " + d->fontsUsed);
    s << "%%EOF\n";
    d->outDevice->write(trailer);
    ignoreSigPipe(false);

    if (d->outDevice)
        d->outDevice->close();
    if (d->fd >= 0)
        ::close(d->fd);
    d->fd = -1;
    delete d->outDevice;
    d->outDevice = 0;

    qDeleteAll(d->fonts);
    d->fonts.clear();
    d->fontNames.clear();
    d->currentPSFont = 0;
    d->currentFont = 0;
    d->firstPage = true;

    setActive(false);
    d->pdev = 0;

    return true;
}


void QPSPrintEngine::updateState(const QPaintEngineState &state)
{
    Q_D(QPSPrintEngine);
    QPaintEngine::DirtyFlags flags = state.state();

    if (flags & DirtyTransform)
        d->stroker.matrix = state.matrix();

    if (flags & DirtyPen) {
        d->pen = state.pen();
        d->hasPen = (d->pen != Qt::NoPen);
        d->stroker.setPen(d->pen);
    }
    if (flags & DirtyBrush) {
        d->brush = state.brush();
        d->hasBrush = (d->brush != Qt::NoBrush);
    }
    if (flags & DirtyBrushOrigin) {
        d->brushOrigin = state.brushOrigin();
        flags |= DirtyBrush;
    }

    if (flags & DirtyBackground)
        d->backgroundBrush = state.backgroundBrush();
    if (flags & DirtyBackgroundMode)
        d->backgroundMode = state.backgroundMode();

    bool ce = d->clipEnabled;
    if (flags & DirtyClipEnabled)
        d->clipEnabled = state.isClipEnabled();
    if (flags & DirtyClipPath)
        updateClipPath(state.clipPath(), state.clipOperation());
    if (flags & DirtyClipRegion) {
        QPainterPath path;
        QVector<QRect> rects = state.clipRegion().rects();
        for (int i = 0; i < rects.size(); ++i)
            path.addRect(rects.at(i));
        updateClipPath(path, state.clipOperation());
        flags |= DirtyClipPath;
    }

    if (ce != d->clipEnabled)
        flags |= DirtyClipPath;
    else if (!d->clipEnabled)
        flags &= ~DirtyClipPath;

    if (flags & DirtyClipPath) {
        *d->pageStream << "Q q\n";
        flags |= DirtyPen|DirtyBrush;
    }

    if (flags & DirtyClipPath) {
        d->allClipped = false;
        if (d->clipEnabled && !d->clips.isEmpty()) {
            for (int i = 0; i < d->clips.size(); ++i) {
                if (d->clips.at(i).isEmpty()) {
                    d->allClipped = true;
                    break;
                }
            }
            if (!d->allClipped) {
                for (int i = 0; i < d->clips.size(); ++i) {
                    *d->pageStream << QPdf::generatePath(d->clips.at(i), QMatrix(), QPdf::ClipPath);
                }
            }
        }
    }

    if (flags & DirtyBrush)
        setBrush();
}

void QPSPrintEngine::updateClipPath(const QPainterPath &p, Qt::ClipOperation op)
{
    Q_D(QPSPrintEngine);
    QPainterPath path = d->stroker.matrix.map(p);
    //qDebug() << "updateClipPath: " << matrix << p.boundingRect() << path.boundingRect();

    if (op == Qt::NoClip) {
        d->clipEnabled = false;
    } else if (op == Qt::ReplaceClip) {
        d->clips.clear();
        d->clips.append(path);
    } else if (op == Qt::IntersectClip) {
        d->clips.append(path);
    } else { // UniteClip
        // ask the painter for the current clipping path. that's the easiest solution
        path = painter()->clipPath();
        path = d->stroker.matrix.map(path);
        d->clips.clear();
        d->clips.append(path);
    }
}

void QPSPrintEngine::setPen()
{
    Q_D(QPSPrintEngine);
    QBrush b = d->pen.brush();
    Q_ASSERT(b.style() == Qt::SolidPattern && b.isOpaque());

    QColor rgba = b.color();
    *d->pageStream << rgba.redF()
                   << rgba.greenF()
                   << rgba.blueF()
                   << "SCN\n";

    *d->pageStream << d->pen.widthF() << "w ";

    int pdfCapStyle = 0;
    switch(d->pen.capStyle()) {
    case Qt::FlatCap:
        pdfCapStyle = 0;
        break;
    case Qt::SquareCap:
        pdfCapStyle = 2;
        break;
    case Qt::RoundCap:
        pdfCapStyle = 1;
        break;
    default:
        break;
    }
    *d->pageStream << pdfCapStyle << "J ";

    int pdfJoinStyle = 0;
    switch(d->pen.joinStyle()) {
    case Qt::MiterJoin:
        pdfJoinStyle = 0;
        break;
    case Qt::BevelJoin:
        pdfJoinStyle = 2;
        break;
    case Qt::RoundJoin:
        pdfJoinStyle = 1;
        break;
    default:
        break;
    }
    *d->pageStream << pdfJoinStyle << "j ";

    *d->pageStream << QPdf::generateDashes(d->pen) << " 0 d\n";
}

void QPSPrintEngine::setBrush()
{
    Q_D(QPSPrintEngine);
#if 0
    bool specifyColor;
    int gStateObject = 0;
    int patternObject = d->addBrushPattern(brush, d->stroker.matrix, brushOrigin, &specifyColor, &gStateObject);

    *d->pageStream << (patternObject ? "/PCSp cs " : "/CSp cs ");
    if (specifyColor) {
        QColor rgba = brush.color();
        *d->pageStream << rgba.redF()
                        << rgba.greenF()
                        << rgba.blueF();
    }
    if (patternObject)
        *d->pageStream << "/Pat" << patternObject;
    *d->pageStream << "scn\n";
#endif
    QColor rgba = d->brush.color();
    *d->pageStream << rgba.redF()
                    << rgba.greenF()
                    << rgba.blueF()
                    << "scn\n";
    *d->pageStream << "/BSt " << d->brush.style() << "def\n";
}



void QPSPrintEngine::drawLines(const QLineF *lines, int lineCount)
{
    if (!lines || !lineCount)
        return;

    QPainterPath p;
    for (int i=0; i!=lineCount;++i) {
        p.moveTo(lines[i].p1());
        p.lineTo(lines[i].p2());
    }
    drawPath(p);
}

void QPSPrintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    if (!points || !pointCount)
        return;
    Q_D(QPSPrintEngine);

    QPainterPath p;

    bool hb = d->hasBrush;

    switch(mode) {
    case OddEvenMode:
        p.setFillRule(Qt::OddEvenFill);
        break;
    case ConvexMode:
    case WindingMode:
        p.setFillRule(Qt::WindingFill);
        break;
    case PolylineMode:
        d->hasBrush = false;
        break;
    default:
        break;
    }

    p.moveTo(points[0]);
    for (int i = 1; i < pointCount; ++i)
        p.lineTo(points[i]);

    if (mode != PolylineMode)
        p.closeSubpath();
    drawPath(p);

    d->hasBrush = hb;
}

void QPSPrintEngine::drawImageInternal(const QRectF &r, QImage image, bool bitmap)
{
    Q_D(QPSPrintEngine);
    if (d->clipEnabled && d->allClipped)
        return;
    if (bitmap && image.depth() != 1)
        bitmap = false;
    QImage mask;
    if (!bitmap) {
        if (image.format() == QImage::Format_Mono || image.format() == QImage::Format_MonoLSB)
            image = image.convertToFormat(QImage::Format_Indexed8);
        if (image.hasAlphaChannel()) {
            // get better alpha dithering
            int xscale = image.width();
            xscale *= xscale <= 800 ? 4 : (xscale <= 1600 ? 2 : 1);
            int yscale = image.height();
            yscale *= yscale <= 800 ? 4 : (yscale <= 1600 ? 2 : 1);
            image = image.scaled(xscale, yscale);
            mask = image.createAlphaMask(Qt::OrderedAlphaDither);
        }
    }
    *d->pageStream << "q\n" << QPdf::generateMatrix(d->stroker.matrix);
    QBrush b = d->brush;
    if (image.depth() == 1) {
        if (d->backgroundMode == Qt::OpaqueMode) {
            // draw background
            d->brush = d->backgroundBrush;
            setBrush();
            *d->pageStream << r.x() << r.y() << r.width() << r.height() << "re f\n";
        }
        // set current pen as brush
        d->brush = d->pen.brush();
        setBrush();
    }
    d->drawImage(r.x(), r.y(), r.width(), r.height(), image, mask);
    *d->pageStream << "Q\n";
    d->brush = b;
}


void QPSPrintEngine::drawImage(const QRectF &r, const QImage &img, const QRectF &sr,
                               Qt::ImageConversionFlags)
{
    QImage image = img.copy(sr.toRect());
    drawImageInternal(r, image, false);
}

void QPSPrintEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr)
{
    QImage img = pm.copy(sr.toRect()).toImage();
    drawImageInternal(r, img, true);
}

void QPSPrintEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
    Q_D(QPSPrintEngine);
    if (d->clipEnabled && d->allClipped)
        return;
    const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);
    d->setFont(ti.fontEngine);
    Q_ASSERT(d->currentPSFont);
    QBrush b = d->pen.brush();
    if(d->currentPSFont && b.style() == Qt::SolidPattern && b.isOpaque()) {
        *d->pageStream << "q\n" << QPdf::generateMatrix(d->stroker.matrix);
        setPen();
        d->currentPSFont->drawText(*d->pageStream, d, p, ti);
        if (ti.flags & QTextItem::Underline)
            *d->pageStream << p.x() << p.y() + d->currentFont->underlinePosition().toReal() + d->currentFont->lineThickness().toReal()
                           << ti.width.toReal() << d->currentFont->lineThickness().toReal() << "re f\n";
        if (ti.flags & QTextItem::StrikeOut)
            *d->pageStream << p.x() << p.y() + d->currentFont->ascent().toReal()/qreal(3.) << ti.width.toReal() << d->currentFont->lineThickness().toReal() << " re f\n";
        *d->pageStream << "Q\n";
    } else {
        *d->pageStream << "q\n";
        QBrush br = d->brush;
        bool hp = d->hasPen;
        d->hasPen = false;
        d->brush = b;
        setBrush();
        QPaintEngine::drawTextItem(p, textItem);
        *d->pageStream << "Q\n";
        d->brush = br;
        d->hasPen = hp;
    }
}

void QPSPrintEngine::drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &p)
{
    Q_D(QPSPrintEngine);
    if (d->clipEnabled && d->allClipped)
        return;
    // ### Optimise implementation!
    qreal yPos = r.y();
    qreal yOff = p.y();
    while(yPos < r.y() + r.height()) {
        qreal drawH = pixmap.height() - yOff;    // Cropping first row
        if (yPos + drawH > r.y() + r.height())        // Cropping last row
            drawH = r.y() + r.height() - yPos;
        qreal xPos = r.x();
        qreal xOff = p.x();
        while(xPos < r.x() + r.width()) {
            qreal drawW = pixmap.width() - xOff; // Cropping first column
            if (xPos + drawW > r.x() + r.width())    // Cropping last column
                drawW = r.x() + r.width() - xPos;
            // ########
            painter()->drawPixmap(QPointF(xPos, yPos).toPoint(), pixmap,
                                   QRectF(xOff, yOff, drawW, drawH).toRect());
            xPos += drawW;
            xOff = 0;
        }
        yPos += drawH;
        yOff = 0;
    }

}

void QPSPrintEngine::drawPath(const QPainterPath &p)
{
    Q_D(QPSPrintEngine);
    if (d->clipEnabled && d->allClipped)
        return;
    QBrush penBrush = d->pen.brush();
    if (d->hasPen && penBrush == Qt::SolidPattern && penBrush.isOpaque()) {
        // draw strokes natively in this case for better output
        *d->pageStream << "q\n";
        setPen();
        *d->pageStream << QPdf::generateMatrix(d->stroker.matrix);
        *d->pageStream << "NP " << QPdf::generatePath(p, QMatrix(), d->hasBrush ? QPdf::FillAndStrokePath : QPdf::StrokePath);
        *d->pageStream << "Q\n";
    } else {
        if (d->hasBrush) {
            *d->pageStream << "NP " << QPdf::generatePath(p, d->stroker.matrix, QPdf::FillPath);
        }
        if (d->hasPen) {
            *d->pageStream << "q\n";
            QBrush b = d->brush;
            d->brush = d->pen.brush();
            setBrush();
            *d->pageStream << "NP ";
            d->stroker.strokePath(p);
            *d->pageStream << "Q\n";
            d->brush = b;
        }
    }
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

    d->pageBuffer = QByteArray();
    delete d->pageStream;
    d->pageStream = new QPdf::ByteStream(&d->pageBuffer);
    d->stroker.stream = d->pageStream;

    d->currentFont = 0; // reset current font

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
    QRect r = d->fullPage ? d->paperRect() : d->pageRect();
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
        d->title = value.toString();
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
    case PPK_FontEmbedding:
        d->embedFonts = value.toBool();
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
        ret = d->title;
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
        break;
    case PPK_PageRect:
        ret = d->pageRect();
        break;
    case PPK_SelectionOption:
        ret = d->selectionOption;
        break;
    case PPK_FontEmbedding:
        ret = d->embedFonts;
        break;
    default:
        break;
    }
    return ret;
}

#endif // QT_NO_PRINTER
