/****************************************************************************
**
** Implementation of QPSPrinter class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qplatformdefs.h"

// POSIX Large File Support redefines open -> open64
#if defined(open)
# undef open
#endif

// POSIX Large File Support redefines truncate -> truncate64
#if defined(truncate)
# undef truncate
#endif

#include "qpsprinter_p.h"
#include "qpainter_p.h"

#ifndef QT_NO_PRINTER

#undef Q_PRINTER_USE_TYPE42

#include "qprinter.h"
#include "qpainter.h"
#include "qapplication.h"
#include "qpaintdevicemetrics.h"
#include "qpixmap.h"
#include "qimage.h"
#include "qdatetime.h"
#include "qstring.h"
#include "qbytearray.h"
#include "qhash.h"
#include "qfile.h"
#include "qbuffer.h"
#include "qtextcodec.h"
#include "qsettings.h"
#include "qmap.h"
#include "qfontdatabase.h"
#include "qregexp.h"
#include "qbitmap.h"
#include "qregion.h"
#include <private/qunicodetables_p.h>

#if defined(Q_OS_WIN32)
#include <io.h>
#ifdef Q_PRINTER_USE_TYPE42
#include <stdlib.h>
#endif
#else
#include <unistd.h>
#include <stdlib.h>
#endif

#ifdef Q_WS_X11
#include "qt_x11_p.h"
#ifdef None
#undef None
#endif
#ifdef GrayScale
#undef GrayScale
#endif
#endif

#if defined (Q_WS_X11) || defined (Q_WS_QWS)
#include "qfontdata_p.h"
#include "qfontengine_p.h"
#include "qtextlayout_p.h"
#include "qtextengine_p.h"
#endif

static bool qt_gen_epsf = FALSE;

void qt_generate_epsf( bool b )
{
    qt_gen_epsf = b;
}

static const char *const ps_header =
"/d/def load def/D{bind d}bind d/d2{dup dup}D/B{0 d2}D/W{255 d2}D/ED{exch d}D\n"
"/D0{0 ED}D/LT{lineto}D/MT{moveto}D/S{stroke}D/F{setfont}D/SW{setlinewidth}D\n"
"/CP{closepath}D/RL{rlineto}D/NP{newpath}D/CM{currentmatrix}D/SM{setmatrix}D\n"
"/TR{translate}D/SD{setdash}D/SC{aload pop setrgbcolor}D/CR{currentfile read\n"
"pop}D/i{index}D/bs{bitshift}D/scs{setcolorspace}D/DB{dict dup begin}D/DE{end\n"
"d}D/ie{ifelse}D/sp{astore pop}D/BSt 0 d/LWi 1 d/PSt 1 d/Cx 0 d/Cy 0 d/WFi\n"
"false d/OMo false d/BCol[1 1 1]d/PCol[0 0 0]d/BkCol[1 1 1]d/BDArr[0.94 0.88\n"
"0.63 0.50 0.37 0.12 0.06]d/defM matrix d/nS 0 d/GPS{PSt 1 ge PSt 5 le and{{\n"
"LArr PSt 1 sub 2 mul get}{LArr PSt 2 mul 1 sub get}ie}{[]}ie}D/QS{PSt 0 ne{\n"
"gsave LWi SW true GPS 0 SD S OMo PSt 1 ne and{BkCol SC false GPS dup 0 get\n"
"SD S}if grestore}if}D/r28{{CR dup 32 gt{exit}if pop}loop 3{CR}repeat 0 4{7\n"
"bs exch dup 128 gt{84 sub}if 42 sub 127 and add}repeat}D/rA 0 d/rL 0 d/rB{rL\n"
"0 eq{/rA r28 d/rL 28 d}if dup rL gt{rA exch rL sub rL exch/rA 0 d/rL 0 d rB\n"
"exch bs add}{dup rA 16#fffffff 3 -1 roll bs not and exch dup rL exch sub/rL\n"
"ED neg rA exch bs/rA ED}ie}D/uc{/rL 0 d 0{dup 2 i length ge{exit}if 1 rB 1\n"
"eq{3 rB dup 3 ge{1 add dup rB 1 i 5 ge{1 i 6 ge{1 i 7 ge{1 i 8 ge{128 add}if\n"
"64 add}if 32 add}if 16 add}if 3 add exch pop}if 3 add exch 10 rB 1 add{dup 3\n"
"i lt{dup}{2 i}ie 4 i 3 i 3 i sub 2 i getinterval 5 i 4 i 3 -1 roll\n"
"putinterval dup 4 -1 roll add 3 1 roll 4 -1 roll exch sub dup 0 eq{exit}if 3\n"
"1 roll}loop pop pop}{3 rB 1 add{2 copy 8 rB put 1 add}repeat}ie}loop pop}D\n"
"/sl D0/QCIgray D0/QCIcolor D0/QCIindex D0/QCI{/colorimage where{pop false 3\n"
"colorimage}{exec/QCIcolor ED/QCIgray QCIcolor length 3 idiv string d 0 1\n"
"QCIcolor length 3 idiv 1 sub{/QCIindex ED/x QCIindex 3 mul d QCIgray\n"
"QCIindex QCIcolor x get 0.30 mul QCIcolor x 1 add get 0.59 mul QCIcolor x 2\n"
"add get 0.11 mul add add cvi put}for QCIgray image}ie}D/di{gsave TR 1 i 1 eq\n"
"{false eq{pop true 3 1 roll 4 i 4 i false 4 i 4 i imagemask BkCol SC\n"
"imagemask}{pop false 3 1 roll imagemask}ie}{dup false ne{/languagelevel\n"
"where{pop languagelevel 3 ge}{false}ie}{false}ie{/ma ED 8 eq{/dc[0 1]d\n"
"/DeviceGray}{/dc[0 1 0 1 0 1]d/DeviceRGB}ie scs/im ED/mt ED/h ED/w ED/id 7\n"
"DB/ImageType 1 d/Width w d/Height h d/ImageMatrix mt d/DataSource im d\n"
"/BitsPerComponent 8 d/Decode dc d DE/md 7 DB/ImageType 1 d/Width w d/Height\n"
"h d/ImageMatrix mt d/DataSource ma d/BitsPerComponent 1 d/Decode[0 1]d DE 4\n"
"DB/ImageType 3 d/DataDict id d/MaskDict md d/InterleaveType 3 d end image}{\n"
"pop 8 4 1 roll 8 eq{image}{QCI}ie}ie}ie grestore}d/BF{gsave BSt 1 eq{BCol SC\n"
"WFi{fill}{eofill}ie}if BSt 2 ge BSt 8 le and{BDArr BSt 2 sub get/sc ED BCol{\n"
"1. exch sub sc mul 1. exch sub}forall 3 array astore SC WFi{fill}{eofill}ie}\n"
"if BSt 9 ge BSt 14 le and{WFi{clip}{eoclip}ie defM SM pathbbox 3 i 3 i TR 4\n"
"2 roll 3 2 roll exch sub/h ED sub/w ED OMo{NP 0 0 MT 0 h RL w 0 RL 0 h neg\n"
"RL CP BkCol SC fill}if BCol SC 0.3 SW NP BSt 9 eq BSt 11 eq or{0 4 h{dup 0\n"
"exch MT w exch LT}for}if BSt 10 eq BSt 11 eq or{0 4 w{dup 0 MT h LT}for}if\n"
"BSt 12 eq BSt 14 eq or{w h gt{0 6 w h add{dup 0 MT h sub h LT}for}{0 6 w h\n"
"add{dup 0 exch MT w sub w exch LT}for}ie}if BSt 13 eq BSt 14 eq or{w h gt{0\n"
"6 w h add{dup h MT h sub 0 LT}for}{0 6 w h add{dup w exch MT w sub 0 exch LT\n"
"}for}ie}if S}if BSt 24 eq{}if grestore}D/mat matrix d/ang1 D0/ang2 D0/w D0/h\n"
"D0/x D0/y D0/ARC{/ang2 ED/ang1 ED/h ED/w ED/y ED/x ED mat CM pop x w 2 div\n"
"add y h 2 div add TR 1 h w div neg scale ang2 0 ge{0 0 w 2 div ang1 ang1\n"
"ang2 add arc}{0 0 w 2 div ang1 ang1 ang2 add arcn}ie mat SM}D/C D0/P{NP MT\n"
"0.5 0.5 rmoveto 0 -1 RL -1 0 RL 0 1 RL CP fill}D/M{/Cy ED/Cx ED}D/L{NP Cx Cy\n"
"MT/Cy ED/Cx ED Cx Cy LT QS}D/DL{NP MT LT QS}D/HL{1 i DL}D/VL{2 i exch DL}D/R\n"
"{/h ED/w ED/y ED/x ED NP x y MT 0 h RL w 0 RL 0 h neg RL CP BF QS}D/ACR{/h\n"
"ED/w ED/y ED/x ED x y MT 0 h RL w 0 RL 0 h neg RL CP}D/xr D0/yr D0/rx D0/ry\n"
"D0/rx2 D0/ry2 D0/RR{/yr ED/xr ED/h ED/w ED/y ED/x ED xr 0 le yr 0 le or{x y\n"
"w h R}{xr 100 ge yr 100 ge or{x y w h E}{/rx xr w mul 200 div d/ry yr h mul\n"
"200 div d/rx2 rx 2 mul d/ry2 ry 2 mul d NP x rx add y MT x y rx2 ry2 180 -90\n"
"x y h add ry2 sub rx2 ry2 270 -90 x w add rx2 sub y h add ry2 sub rx2 ry2 0\n"
"-90 x w add rx2 sub y rx2 ry2 90 -90 ARC ARC ARC ARC CP BF QS}ie}ie}D/E{/h\n"
"ED/w ED/y ED/x ED mat CM pop x w 2 div add y h 2 div add TR 1 h w div scale\n"
"NP 0 0 w 2 div 0 360 arc mat SM BF QS}D/A{16 div exch 16 div exch NP ARC QS}\n"
"D/PIE{/ang2 ED/ang1 ED/h ED/w ED/y ED/x ED NP x w 2 div add y h 2 div add MT\n"
"x y w h ang1 16 div ang2 16 div ARC CP BF QS}D/CH{16 div exch 16 div exch NP\n"
"ARC CP BF QS}D/BZ{curveto QS}D/CRGB{255 div 3 1 roll 255 div 3 1 roll 255\n"
"div 3 1 roll}D/BC{CRGB BkCol sp}D/BR{CRGB BCol sp/BSt ED}D/WB{1 W BR}D/NB{0\n"
"B BR}D/PE{setlinejoin setlinecap CRGB PCol sp/LWi ED/PSt ED LWi 0 eq{0.25\n"
"/LWi ED}if PCol SC}D/P1{1 0 5 2 roll 0 0 PE}D/ST{defM SM concat}D/MF{true\n"
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
"exch ashow}D/QI{/C save d pageinit/Cx 0 d/Cy 0 d/OMo false d}D/QP{C restore\n"
"showpage}D/SPD{/setpagedevice where{1 DB 3 1 roll d end setpagedevice}{pop\n"
"pop}ie}D/SV{BSt LWi PSt Cx Cy WFi OMo BCol PCol BkCol/nS nS 1 add d gsave}D\n"
"/RS{nS 0 gt{grestore/BkCol ED/PCol ED/BCol ED/OMo ED/WFi ED/Cy ED/Cx ED/PSt\n"
"ED/LWi ED/BSt ED/nS nS 1 sub d}if}D/CLSTART{/clipTmp matrix CM d defM SM NP}\n"
"D/CLEND{clip NP clipTmp SM}D/CLO{grestore gsave defM SM}D\n";

static const char *agl =
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

static const struct { Q_UINT16 u; Q_UINT16 index; } unicodetoglyph[] = {
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


// ---------------------------------------------------------------------
// postscript font substitution dictionary. We assume every postscript printer has at least
// Helvetica, Times, Courier and Symbol

struct psfont {
    const char *psname;
    float slant;
    float xscale;
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

static const psfont GillSans [] = { // ### some estimated value for xstretch
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

static const psfont Utopia [] = { // ###
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
        { "bitstreamcyberbit", Times, SerifReplacements }, // ###
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
static QString wrapDSC( const QString &str )
{
    QString dsc = str.simplified();
    const int wrapAt = 254;
    QString wrapped;
    if ( dsc.length() < wrapAt )
	wrapped = dsc;
    else {
	wrapped = dsc.left( wrapAt );
	QString tmp = dsc.mid( wrapAt );
	while ( tmp.length() > wrapAt-3 ) {
	    wrapped += "\n%%+" + tmp.left( wrapAt-3 );
	    tmp = tmp.mid( wrapAt-3 );
	}
	wrapped += "\n%%+" + tmp;
    }
    return wrapped + "\n";
}

static QString toString( const float num )
{
    long intNum = (long) num;
    QString ret = QString::number( intNum );
    ret += ".";
    ret += QString::number((long)((num - intNum) * 1000));
    return ret;
}

// ----------------------------- Internal class declarations -----------------------------

class QPSPrinterFontPrivate;

class QPSPrinterPrivate {
public:
    QPSPrinterPrivate( QPrinter *prt, int filedes );
    ~QPSPrinterPrivate();

    void matrixSetup(QPainterState *);
    void clippingSetup(QPainterState *);
    void setClippingOff(QPainterState *);
    void orientationSetup();
    void resetDrawingTools(QPainterState *);
    void emitHeader( bool finished );
    void setFont( const QFont &, int script );
    void drawImage(float x, float y, float w, float h, const QImage &img, const QImage &mask );
    void initPage(QPainterState *paint );
    void flushPage( bool last = FALSE );

    QPrinter   *printer;
    int         pageCount;
    bool        dirtyMatrix;
    bool        dirtyNewPage;
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
    QHash<QString, QPSPrinterFontPrivate *> fonts;
    QPSPrinterFontPrivate *currentFontFile;
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
    QColor bkColor;
    bool dirtyBkColor;
    Qt::BGMode bkMode;
    bool dirtyBkMode;
#ifndef QT_NO_TEXTCODEC
    QTextCodec * currentFontCodec;
#endif
    QString currentFont;
    QFontMetrics fm;
    int textY;
    QFont currentUsed;
    int scriptUsed;
    QFont currentSet;
    float scale;

    bool embedFonts;
    QStringList fontpath;
};


class QPSPrinterFontPrivate {
public:
    QPSPrinterFontPrivate();
    virtual ~QPSPrinterFontPrivate() {}
    virtual QString postScriptFontName() { return psname; }
    virtual QString defineFont( QTextStream &stream, const QString &ps, const QFont &f, const QString &key,
                             QPSPrinterPrivate *d );
    virtual void download(QTextStream& s, bool global);
    virtual void drawText( QTextStream &stream, const QPoint &p, QTextEngine *engine, int item,
                           const QString &text, QPSPrinterPrivate *d, QPainterState *ps);
    virtual unsigned short mapUnicode( unsigned short unicode );
    void downloadMapping( QTextStream &s, bool global );
    QString glyphName( unsigned short glyphindex, bool *glyphSet = 0 );
    virtual void restore();

    virtual unsigned short unicode_for_glyph(int glyphindex) { return glyphindex; }
    virtual unsigned short glyph_for_unicode(unsigned short unicode) { return unicode; }
    unsigned short insertIntoSubset( unsigned short unicode );
    virtual bool embedded() { return FALSE; }

    bool operator == ( const QPSPrinterFontPrivate &other ) {
	return other.psname == psname;
    }
    inline void setSymbol() { symbol = TRUE; }

protected:
    QString psname;
    QStringList replacementList;

    QMap<unsigned short, unsigned short> subset;      // unicode subset in the global font
    QMap<unsigned short, unsigned short> page_subset; // subset added in this page
    int subsetCount;
    int pageSubsetCount;
    bool global_dict;
    bool downloaded;
    bool symbol;
};

// ------------------- end of class declarations ---------------------------

// --------------------------------------------------------------
//   beginning of font related methods
// --------------------------------------------------------------


static int getPsFontType( const QFontEngine *fe )
{
    int weight = fe->fontDef.weight;
    bool italic = fe->fontDef.italic;

    int type = 0; // used to look up in the psname array
    // get the right modification, or build something
    if ( weight > QFont::Normal && italic )
	type = 3;
    else if ( weight > QFont::Normal )
	type = 2;
    else if ( italic )
	type = 1;
    return type;
}

static int addPsFontNameExtension( const QFontEngine *fe, QString &ps, const psfont *psf = 0 )
{
    int type = getPsFontType( fe );

    if ( psf ) {
	ps = QString::fromLatin1( psf[type].psname );
    } else {
	switch ( type ) {
	case 1:
	    ps.append( QString::fromLatin1("-Italic") );
	    break;
	case 2:
	    ps.append( QString::fromLatin1("-Bold") );
	    break;
	case 3:
	    ps.append( QString::fromLatin1("-BoldItalic") );
	    break;
	case 0:
	default:
	    break;
	}
    }
    return type;
}

static QString makePSFontName( const QFontEngine *fe, int *listpos = 0, int *ftype = 0 )
{
  QString ps;
  int i;

  QString family = fe->fontDef.family.toLower();

  // try to make a "good" postscript name
  ps = family.simplified();
  i = 0;
  while( i < ps.length() ) {
    if ( i != 0 && ps[i] == '[') {
      if ( ps[i-1] == ' ' )
	ps.truncate (i-1);
      else
	ps.truncate (i);
      break;
    }
    if ( i == 0 || ps[i-1] == ' ' ) {
      ps[i] = ps[i].toUpper();
      if ( i )
        ps.remove( i-1, 1 );
      else
        i++;
    } else {
      i++;
    }
  }

  if ( ps.isEmpty() )
      ps = "Helvetica";

  // see if the table has a better name
  i = 0;
  QString lowerName = ps.toLower();
  while( postscriptFonts[i].input &&
         postscriptFonts[i].input != lowerName )
    i++;
  const psfont *psf = postscriptFonts[i].ps;

  int type = addPsFontNameExtension( fe, ps, psf );

  if ( listpos )
      *listpos = i;
  if ( ftype )
      *ftype = type;
  return ps;
}

static void appendReplacements( QStringList &list, const psfont * const * replacements, int type, float xscale = 100. )
{
    // iterate through the replacement fonts
    while ( *replacements ) {
        const psfont *psf = *replacements;
        QString ps = "[ /" + QString::fromLatin1( psf[type].psname ) + " " +
		     toString( xscale / psf[type].xscale ) + " " +
		     toString( psf[type].slant ) + " ]";
        list.append( ps );
        ++replacements;
    }
}

static QStringList makePSFontNameList( const QFontEngine *fe, const QString &psname = QString::null, bool useNameForLookup = FALSE )
{
    int i;
    int type;
    QStringList list;
    QString ps = psname;

    if ( !ps.isEmpty() && !useNameForLookup ) {
        QString best = "[ /" + ps + " 1.0 0.0 ]";
        list.append( best );
    }

    ps = makePSFontName( fe, &i, &type );

    const psfont *psf = postscriptFonts[i].ps;
    const psfont * const * replacements = postscriptFonts[i].replacements;
    float xscale = 100;
    if ( psf ) {
        // xscale for the "right" font is always 1. We scale the replacements...
        xscale = psf->xscale;
        ps = "[ /" + QString::fromLatin1( psf[type].psname ) + " 1.0 " +
             toString( psf[type].slant ) + " ]";
    } else {
        ps = "[ /" + ps + " 1.0 0.0 ]";
        // only add default replacement fonts in case this font was unknown.
        if ( fe->fontDef.fixedPitch ) {
            replacements = FixedReplacements;
        } else {
            replacements = SansSerifReplacements;
            // 100 is courier, but most fonts are not as wide as courier. Using 100
            // here would make letters overlap for some fonts. This value is empirical.
            xscale = 83;
        }
    }
    list.append( ps );

    if ( replacements )
        appendReplacements( list, replacements, type, xscale);
    return list;
}

static void emitPSFontNameList( QTextStream &s, const QString &psname, const QStringList &list )
{
    s << "/" << psname << "List [\n";
    s << list.join("\n  ");
    s << "\n] d\n";
}

static inline float pointSize( const QFont &f, float scale )
{
    float psize;
    if ( f.pointSize() != -1 )
	psize = f.pointSize()/scale;
    else
	psize = f.pixelSize();
    return psize;
}


// ========================== FONT CLASSES  ===============


QPSPrinterFontPrivate::QPSPrinterFontPrivate()
{
    global_dict = FALSE;
    downloaded  = FALSE;
    symbol = FALSE;
    // map 0 to .notdef
    subset.insert( 0, 0 );
    subsetCount = 1;
    pageSubsetCount = 0;
}

unsigned short QPSPrinterFontPrivate::insertIntoSubset( unsigned short u )
{
    unsigned short retval = 0;
    if ( subset.find(u) == subset.end() ) {
        if ( !downloaded ) { // we need to add to the page subset
            subset.insert( u, subsetCount ); // mark it as used
            //printf("GLOBAL SUBSET ADDED %04x = %04x\n",u, subsetCount);
            retval = subsetCount;
            subsetCount++;
        } else if ( page_subset.find(u) == page_subset.end() ) {
            page_subset.insert( u, pageSubsetCount ); // mark it as used
            //printf("PAGE SUBSET ADDED %04x = %04x\n",u, pageSubsetCount);
            retval = pageSubsetCount + (subsetCount/256 + 1) * 256;
            pageSubsetCount++;
        }
    } else {
        qWarning("QPSPrinterFont::internal error");
    }
    return retval;
}

void QPSPrinterFontPrivate::restore()
{
    page_subset.clear();
    pageSubsetCount = 0;
    //qDebug("restore for font %s\n",psname.latin1());
}

static inline const char *toHex( uchar u )
{
    static char hexVal[3];
    int i = 1;
    while ( i >= 0 ) {
	ushort hex = (u & 0x000f);
	if ( hex < 0x0a )
	    hexVal[i] = '0'+hex;
	else
	    hexVal[i] = 'A'+(hex-0x0a);
	u = u >> 4;
	i--;
    }
    hexVal[2] = '\0';
    return hexVal;
}

static inline const char *toHex( ushort u )
{
    static char hexVal[5];
    int i = 3;
    while ( i >= 0 ) {
	ushort hex = (u & 0x000f);
	if ( hex < 0x0a )
	    hexVal[i] = '0'+hex;
	else
	    hexVal[i] = 'A'+(hex-0x0a);
	u = u >> 4;
	i--;
    }
    hexVal[4] = '\0';
    return hexVal;
}

static inline const char * toInt( int i )
{
    static char intVal[20];
    intVal[19] = 0;
    int pos = 19;
    if ( i == 0 ) {
	intVal[--pos] = '0';
    } else {
	bool neg = FALSE;
	if ( i < 0 ) {
	    neg = TRUE;
	    i = -i;
	}
	while ( i ) {
	    int dec = i%10;
	    intVal[--pos] = '0'+dec;
	    i /= 10;
	}
	if ( neg )
	    intVal[--pos] = '-';
    }
    return intVal+pos;
}

void QPSPrinterFontPrivate::drawText( QTextStream &stream, const QPoint &p, QTextEngine *engine, int item,
                                 const QString &text, QPSPrinterPrivate *d, QPainterState *ps)
{
    int len = engine->length( item );
    QScriptItem &si = engine->items[item];

    int x = p.x();
    int y = p.y();
    if ( y != d->textY || d->textY == 0 )
        stream << y << " Y";
    d->textY = y;

    stream << "<";
    if ( si.analysis.bidiLevel % 2 ) {
	for ( int i = len-1; i >=0; i-- )
	    stream << toHex( mapUnicode(text.unicode()[i].unicode()) );
    } else {
	for ( int i = 0; i < len; i++ )
	    stream << toHex( mapUnicode(text.unicode()[i].unicode()) );
    }
    stream << ">";

    stream << si.width << " " << x;

    if ( ps->font.underline() )
        stream << ' ' << y + d->fm.underlinePos() + d->fm.lineWidth()
               << " " << d->fm.lineWidth() << " Tl";
    if ( ps->font.strikeOut() )
        stream << ' ' << y + d->fm.strikeOutPos()
               << " " << d->fm.lineWidth() << " Tl";
    stream << " AT\n";

}


QString QPSPrinterFontPrivate::defineFont( QTextStream &stream, const QString &ps, const QFont &f, const QString &key,
                                   QPSPrinterPrivate *d )
{
    QString fontName;
    fontName.sprintf( "/%s-Uni", ps.latin1());

    if ( d->buffer ) {
        ++d->headerFontNumber;
        d->fontStream << "/F" << d->headerFontNumber << " "
                      << pointSize( f, d->scale ) << fontName << " DF\n";
        fontName.sprintf( "F%d", d->headerFontNumber );
        d->headerFontNames.insert(key, fontName);
    } else {
        ++d->pageFontNumber;
        stream << "/F" << d->pageFontNumber << " "
               << pointSize( f, d->scale ) << fontName << " DF\n";
        fontName.sprintf( "F%d", d->pageFontNumber );
        d->pageFontNames.insert(key, fontName);
    }
    return fontName;
}

unsigned short QPSPrinterFontPrivate::mapUnicode( unsigned short unicode )
{
    QMap<unsigned short, unsigned short>::iterator res;
    res = subset.find( unicode );
    unsigned short offset = 0;
    bool found = FALSE;
    if ( res != subset.end() ) {
        found = TRUE;
    } else {
        if ( downloaded ) {
            res = page_subset.find( unicode );
            offset = (subsetCount/256 + 1) * 256;
            if ( res != page_subset.end() )
                found = TRUE;
        }
    }
    if ( !found ) {
        return insertIntoSubset( unicode );
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

QString QPSPrinterFontPrivate::glyphName( unsigned short glyphindex, bool *glyphSet )
{
    QString glyphname;
    int l = 0;
    unsigned short unicode = unicode_for_glyph( glyphindex );
    if (symbol && unicode < 0x100) {
	// map from latin1 to symbol
	unicode = symbol_map[unicode];
    }
    if ( !unicode && glyphindex ) {
	glyphname = "gl";
	glyphname += toHex( glyphindex );
    } else {
	while( unicodetoglyph[l].u < unicode )
	    l++;
	if ( unicodetoglyph[l].u == unicode ) {
	    glyphname = agl[unicodetoglyph[l].index];
	    if ( glyphSet ) {
		int other = 0;
		switch ( unicode ) {
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
		if ( other ) {
		    int oglyph = glyph_for_unicode( other );
		    if( oglyph && oglyph != glyphindex && glyphSet[oglyph] ) {
 			glyphname = "uni";
 			glyphname += toHex( unicode );
		    }
		}
	    }
 	} else {
	    glyphname = "uni";
	    glyphname += toHex( unicode );
	}
    }
    return glyphname;
}

void QPSPrinterFontPrivate::download(QTextStream &s, bool global)
{
    //printf("defining mapping for printer font %s\n",psname.latin1());
    downloadMapping( s, global );
}

void QPSPrinterFontPrivate::downloadMapping( QTextStream &s, bool global )
{
    uchar rangeOffset = 0;
    uchar numRanges = (uchar)(subsetCount/256 + 1);
    uchar range;
    QMap<unsigned short, unsigned short> *subsetDict = &subset;
    if ( !global ) {
        rangeOffset = numRanges;
        numRanges = pageSubsetCount/256 + 1;
        subsetDict = &page_subset;
    }
    // build up inverse table
    unsigned short *inverse = new unsigned short[numRanges * 256];
    memset( inverse, 0, numRanges * 256 * sizeof( unsigned short ) );

    QMap<unsigned short, unsigned short>::iterator it;
    for ( it = subsetDict->begin(); it != subsetDict->end(); ++it) {
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
        for(int k=0; k<256; k++ ) {
            int c = range*256 + k;
            unsigned short glyph = inverse[c];
            glyphname = glyphName( glyph );
            if ( line.length() + glyphname.length() > 76 ) {
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
        s << "/";
        s << psname;
        s << "-Uni-";
        s << toHex((uchar)(range + rangeOffset));
        s << " ";
        s << psname;
        s << "-ENC-";
        s << toHex((uchar)(range + rangeOffset));
        if ( embedded() ) {
            s << " /";
            s << psname;
            s << " MFEmb\n";
        } else {
            s << " " << psname << "List";
            s << " MF\n";
        }
    }

    // === write header ===
    //   int VMMin;
    //   int VMMax;

    s << wrapDSC( "%%BeginFont: " + psname )
      << "%!PS-AdobeFont-1.0 Composite Font\n"
      << wrapDSC( "%%FontName: " + psname + "-Uni" )
      << "%%Creator: Composite font created by Qt\n";

    /* Start the dictionary which will eventually */
    /* become the font. */
    s << "25 dict begin\n"; // need to verify. Sivan

    s << "/FontName /";
    s << psname;
    s << "-Uni";
    s << " def\n";
    s << "/PaintType 0 def\n";

    // This is concatenated with the base fonts, so it should perform
    // no transformation. Sivan
    s << "/FontMatrix[1 0 0 1 0 0]def\n";

    s << "/FontType ";
    s << 0;
    s << " def\n";

    // now come composite font structures
    // FMapTypes:
    // 2: 8/8, 8 bits select the font, 8 the glyph

    s << "/FMapType 2 def\n";

    // The encoding in a composite font is used for indirection.
    // Every char is split into a font-number and a character-selector.
    // PostScript prints glyph number character-selector from the font
    // FDepVector[ Encoding[ font-number ] ].

    s << "/Encoding [";
    for (range=0; range < rangeOffset + numRanges; range++) {
        if (range % 16 == 0)
            s << "\n";
        else
            s << " ";
        s << range;
    }
    s << "]def\n";

  // Descendent fonts

    s << "/FDepVector [\n";
    for (range=0; range < rangeOffset + numRanges; range++) {
        s << "/";
        s << psname;
        s << "-Uni-";
        s << toHex( range );
        s << " findfont\n";
    }
    s << "]def\n";

  // === trailer ===

    s << "FontName currentdict end definefont pop\n";
    s << "%%EndFont\n";
}


// ================== TTF ====================

typedef Q_UINT8  BYTE;
typedef Q_UINT16 USHORT;
typedef Q_UINT16 uFWord;
typedef Q_INT16 SHORT;
typedef Q_INT16 FWord;
typedef Q_UINT32   ULONG;
typedef Q_INT32  FIXED;

typedef struct {
  Q_INT16 whole;
  Q_UINT16 fraction;
} Fixed; // 16.16 bit fixed-point number

static float f2dot14( ushort s )
{
    float f = ((float)( s & 0x3fff ))/ 16384.;
    f += (s & 0x8000) ? ( (s & 0x4000) ? -1 : -2 ) : ( (s & 0x4000) ? 1 : 0 );
    return f;
}

typedef struct {
  int*    epts_ctr;                     /* array of contour endpoints */
  int     num_pts, num_ctr;             /* number of points, number of coutours */
  FWord*  xcoor, *ycoor;                /* arrays of x and y coordinates */
  BYTE*   tt_flags;                     /* array of TrueType flags */
  double* area_ctr;
  char*   check_ctr;
  int*    ctrset;               /* in contour index followed by out contour index */
} charproc_data;


class QPSPrinterFontTTF
    : public QPSPrinterFontPrivate {
public:
    QPSPrinterFontTTF(const QFontEngine *f, QByteArray& data);
    virtual void    download(QTextStream& s, bool global);
    virtual void drawText( QTextStream &stream, const QPoint &p, QTextEngine *engine, int item,
                           const QString &text, QPSPrinterPrivate *d, QPainterState *ps);
    //  virtual ~QPSPrinterFontTTF();

    virtual bool embedded() { return TRUE; }
private:
    QByteArray     data;
    QVector<ushort> uni2glyph; // to speed up lookups
    QVector<ushort> glyph2uni; // to speed up lookups
    bool           defective; // if we can't process this file

    const BYTE*   getTable(const char *);
    void uni2glyphSetup();
    unsigned short unicode_for_glyph(int glyphindex);
    unsigned short glyph_for_unicode(unsigned short unicode);
    int   topost(FWord x) { return (int)( ((int)(x) * 1000 + HUPM) / unitsPerEm ); }

#ifdef Q_PRINTER_USE_TYPE42
    void sfnts_pputBYTE(BYTE n,QTextStream& s,
			int& string_len, int& line_len, bool& in_string);
    void sfnts_pputUSHORT(USHORT n,QTextStream& s,
			  int& string_len, int& line_len, bool& in_string);
    void sfnts_pputULONG(ULONG n,QTextStream& s,
			 int& string_len, int& line_len, bool& in_string);
    void sfnts_end_string(QTextStream& s,
			  int& string_len, int& line_len, bool& in_string);
    void sfnts_new_table(ULONG length,QTextStream& s,
			 int& string_len, int& line_len, bool& in_string);
    void sfnts_glyf_table(ULONG oldoffset,
			  ULONG correct_total_length,
			  QTextStream& s,
			  int& string_len, int& line_len, bool& in_string);
    void download_sfnts(QTextStream& s);
#endif

    void subsetGlyph(int charindex,bool* glyphset);

    void charproc(int charindex, QTextStream& s, bool *glyphSet);
    const BYTE* charprocFindGlyphData(int charindex);
    void charprocComposite(const BYTE *glyph, QTextStream& s, bool *glyphSet);
    void charprocLoad(const BYTE *glyph, charproc_data* cd);

    int target_type;                      /* 42 or 3 */

    int numTables;                        /* number of tables present */
    QString PostName;                     /* Font's PostScript name */
    QString FullName;                     /* Font's full name */
    QString FamilyName;                   /* Font's family name */
    QString Style;                        /* Font's style string */
    QString Copyright;                    /* Font's copyright string */
    QString Version;                      /* Font's version string */
    QString Trademark;                    /* Font's trademark string */
    int llx,lly,urx,ury;          /* bounding box */

    Fixed TTVersion;                      /* Truetype version number from offset table */
    Fixed MfrRevision;                    /* Revision number of this font */

    const BYTE *offset_table;           /* Offset table in memory */
    const BYTE *post_table;                     /* 'post' table in memory */

    const BYTE *loca_table;                     /* 'loca' table in memory */
    const BYTE *glyf_table;                     /* 'glyf' table in memory */
    const BYTE *hmtx_table;                     /* 'hmtx' table in memory */

    USHORT numberOfHMetrics;
    int unitsPerEm;                       /* unitsPerEm converted to int */
    int HUPM;                             /* half of above */

    int numGlyphs;                        /* from 'post' table */

    int indexToLocFormat;         /* short or long offsets */

};


static ULONG getULONG(const BYTE *p)
{
  int x;
  ULONG val=0;

  for(x=0; x<4; x++) {
    val *= 0x100;
    val += p[x];
  }

  return val;
}

static USHORT getUSHORT(const BYTE *p)
{
  int x;
  USHORT val=0;

  for(x=0; x<2; x++) {
    val *= 0x100;
    val += p[x];
  }

  return val;
}

static Fixed getFixed(const BYTE *s)
{
  Fixed val={0,0};

  val.whole = ((s[0] * 256) + s[1]);
  val.fraction = ((s[2] * 256) + s[3]);

  return val;
}

static FWord getFWord(const BYTE* s)  { return (FWord)  getUSHORT(s); }
static uFWord getuFWord(const BYTE* s) { return (uFWord) getUSHORT(s); }
static SHORT getSHORT(const BYTE* s)  { return (SHORT)  getUSHORT(s); }

#if 0
static const char * const Apple_CharStrings[]={
  ".notdef",".null","nonmarkingreturn","space","exclam","quotedbl","numbersign",
  "dollar","percent","ampersand","quotesingle","parenleft","parenright",
  "asterisk","plus", "comma","hyphen","period","slash","zero","one","two",
  "three","four","five","six","seven","eight","nine","colon","semicolon",
  "less","equal","greater","question","at","A","B","C","D","E","F","G","H","I",
  "J","K", "L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z",
  "bracketleft","backslash","bracketright","asciicircum","underscore","grave",
  "a","b","c","d","e","f","g","h","i","j","k", "l","m","n","o","p","q","r","s",
  "t","u","v","w","x","y","z","braceleft","bar","braceright","asciitilde",
  "Adieresis","Aring","Ccedilla","Eacute","Ntilde","Odieresis","Udieresis",
  "aacute","agrave","acircumflex","adieresis","atilde","aring","ccedilla",
  "eacute","egrave","ecircumflex","edieresis","iacute","igrave","icircumflex",
  "idieresis","ntilde","oacute","ograve","ocircumflex","odieresis","otilde",
  "uacute","ugrave","ucircumflex","udieresis","dagger","degree","cent",
  "sterling","section","bullet","paragraph","germandbls","registered",
  "copyright","trademark","acute","dieresis","notequal","AE","Oslash",
  "infinity","plusminus","lessequal","greaterequal","yen","mu","partialdiff",
  "summation","product","pi","integral","ordfeminine","ordmasculine","Omega",
  "ae","oslash","questiondown","exclamdown","logicalnot","radical","florin",
  "approxequal","Delta","guillemotleft","guillemotright","ellipsis",
  "nobreakspace","Agrave","Atilde","Otilde","OE","oe","endash","emdash",
  "quotedblleft","quotedblright","quoteleft","quoteright","divide","lozenge",
  "ydieresis","Ydieresis","fraction","currency","guilsinglleft","guilsinglright",
  "fi","fl","daggerdbl","periodcentered","quotesinglbase","quotedblbase",
  "perthousand","Acircumflex","Ecircumflex","Aacute","Edieresis","Egrave",
  "Iacute","Icircumflex","Idieresis","Igrave","Oacute","Ocircumflex","apple",
  "Ograve","Uacute","Ucircumflex","Ugrave","dotlessi","circumflex","tilde",
  "macron","breve","dotaccent","ring","cedilla","hungarumlaut","ogonek","caron",
  "Lslash","lslash","Scaron","scaron","Zcaron","zcaron","brokenbar","Eth","eth",
  "Yacute","yacute","Thorn","thorn","minus","multiply","onesuperior",
  "twosuperior","threesuperior","onehalf","onequarter","threequarters","franc",
  "Gbreve","gbreve","Idot","Scedilla","scedilla","Cacute","cacute","Ccaron",
  "ccaron","dmacron","markingspace","capslock","shift","propeller","enter",
  "markingtabrtol","markingtabltor","control","markingdeleteltor",
  "markingdeletertol","option","escape","parbreakltor","parbreakrtol",
  "newpage","checkmark","linebreakltor","linebreakrtol","markingnobreakspace",
  "diamond","appleoutline"};
#endif

// #define DEBUG_TRUETYPE

QPSPrinterFontTTF::QPSPrinterFontTTF(const QFontEngine *f, QByteArray& d)
{
  data = d;
  defective = FALSE;

  const BYTE *ptr;

  target_type = 3;  // will work on any printer
  //target_type = 42; // works with gs, faster, better quality

#ifdef Q_PRINTER_USE_TYPE42
  char* environment_preference = getenv("QT_TTFTOPS");
  if (environment_preference) {
    if (QString(environment_preference) == "42")
      target_type = 42;
    else if (QString(environment_preference) == "3")
      target_type = 3;
    else
      qWarning("The value of QT_TTFTOPS must be 42 or 3");
  }
#endif
  offset_table = (const BYTE *)data.constData(); /* first 12 bytes */

  /* Determine how many directory entries there are. */
  numTables = getUSHORT( offset_table + 4 );

  /* Extract information from the "Offset" table. */
  TTVersion = getFixed( offset_table );

  /* Load the "head" table and extract information from it. */
  ptr = getTable("head");
  if ( !ptr ) {
      defective = TRUE;
      return;
  }
  MfrRevision = getFixed( ptr + 4 );            /* font revision number */
  unitsPerEm = getUSHORT( ptr + 18 );
  HUPM = unitsPerEm / 2;
#ifdef DEBUG_TRUETYPE
  printf("unitsPerEm=%d",(int)unitsPerEm);
#endif
  llx = topost( getFWord( ptr + 36 ) );         /* bounding box info */
  lly = topost( getFWord( ptr + 38 ) );
  urx = topost( getFWord( ptr + 40 ) );
  ury = topost( getFWord( ptr + 42 ) );
  indexToLocFormat = getSHORT( ptr + 50 );      /* size of 'loca' data */
  if(indexToLocFormat != 0 && indexToLocFormat != 1) {
    qWarning("TrueType font is unusable because indexToLocFormat != 0");
    defective = TRUE;
    return;
  }
  if( getSHORT(ptr+52) != 0 ) {
    qWarning("TrueType font is unusable because glyphDataFormat != 0");
    defective = TRUE;
    return;
  }

  // === Load information from the "name" table ===

  /* Set default values to avoid future references to */
  /* undefined pointers. */

  psname = FullName = FamilyName = Version = Style = "unknown";
  Copyright = "No copyright notice";
  Trademark = "No trademark notice";

  const BYTE* table_ptr = getTable("name");           /* pointer to table */
  if ( !table_ptr ) {
      defective = TRUE;
      qDebug("couldn't find name table" );
      return;
  }
  int numrecords = getUSHORT( table_ptr + 2 );  /* number of names */
  char* strings = (char *)table_ptr + getUSHORT( table_ptr + 4 ); /* start of string storage */

  const BYTE* ptr2 = table_ptr + 6;
  for(int x=0; x < numrecords; x++,ptr2+=12) {
      int platform = getUSHORT(ptr2);
    //int encoding = getUSHORT(ptr2+2);
    //int language = getUSHORT(ptr2+4);
    int nameid   = getUSHORT(ptr2+6);
    int length   = getUSHORT(ptr2+8);
    int offset   = getUSHORT(ptr2+10);

    if( platform == 1 && nameid == 0 )
      Copyright = QString::fromLatin1(strings+offset,length);

    if( platform == 1 && nameid == 1 )
      FamilyName = QString::fromLatin1(strings+offset,length);

    if( platform == 1 && nameid == 2 )
      Style = QString::fromLatin1(strings+offset,length);

    if( platform == 1 && nameid == 4 )
      FullName = QString::fromLatin1(strings+offset,length);

    if( platform == 1 && nameid == 5 )
      Version = QString::fromLatin1(strings+offset,length);

    if( platform == 1 && nameid == 6 )
      psname = QString::fromLatin1(strings+offset,length);

    if( platform == 1 && nameid == 7 )
      Trademark = QString::fromLatin1(strings+offset,length);

  }
  psname.replace(' ', '-');
  psname.replace("/", "");
  if (psname.isEmpty())
      psname = makePSFontName(f);

  //read_cmap(font);

  /* We need to have the PostScript table around. */

  post_table = getTable("post");
#if 0
  if ( post_table ) {
      Fixed post_format = getFixed( post_table );

      if( post_format.whole != 2 || post_format.fraction != 0 ) {
          qWarning("TrueType font does not have a format 2.0 'post' table");
          qWarning("post format is %d.%d",post_format.whole,post_format.fraction);
          // Sivan Feb 2001: no longer defective.
          // defective = TRUE;
      }
  }
#endif
  const BYTE *maxp = getTable("maxp");
  if ( !maxp ) {
      defective = TRUE;
      qDebug("no maxp table in font");
      return;
  }
  numGlyphs = getUSHORT( maxp + 4 );
//  qDebug("number of glyphs is %d", numGlyphs);
  replacementList = makePSFontNameList( f, psname );
  uni2glyphSetup();
}


void QPSPrinterFontTTF::drawText( QTextStream &stream, const QPoint &p, QTextEngine *engine, int item,
				  const QString &text, QPSPrinterPrivate *d, QPainterState *ps)
{
    // we draw glyphs here to get correct shaping of arabic and indic
    QScriptItem &si = engine->items[item];
    engine->shape( item );
    int len = si.num_glyphs;

    int x = p.x();
    int y = p.y();
    if ( y != d->textY || d->textY == 0 )
        stream << y << " Y";
    d->textY = y;

    QByteArray xyarray;
    int xo = 0;
    int yo = 0;

    QGlyphLayout *glyphs = engine->glyphs( &si );
    bool glyphIndices = engine->fontEngine(si)->type() == QFontEngine::Xft;

    stream << "<";
    if ( si.analysis.bidiLevel % 2 ) {
	for ( int i = len-1; i >=0; i-- ) {
	    // map unicode is not really the correct name, as we map glyphs, but we also download glyphs, so this works
	    stream << toHex(mapUnicode(glyphIndices ? glyphs[i].glyph : glyph_for_unicode(text.unicode()[i].unicode())));
	    if ( i != len-1 ) {
		xyarray += toInt( xo + glyphs[i].offset.x + glyphs[i+1].advance );
		xyarray += " ";
		xyarray += toInt( yo + glyphs[i].offset.y );
		xyarray += " ";
		xo = -glyphs[i].offset.x;
		yo = -glyphs[i].offset.y;
	    }
	}
    } else {
	for ( int i = 0; i < len; i++ ) {
	    // map unicode is not really the correct name, as we map glyphs, but we also download glyphs, so this works
	    stream << toHex(mapUnicode(glyphIndices ? glyphs[i].glyph : glyph_for_unicode(text.unicode()[i].unicode())));
	    if ( i ) {
		xyarray += toInt( xo - glyphs[i].offset.x + glyphs[i-1].advance );
		xyarray += " ";
		xyarray += toInt( yo + glyphs[i].offset.y );
		xyarray += " ";
		xo = glyphs[i].offset.x;
		yo = -glyphs[i].offset.y;
	    }
	}
    }
    stream << ">";

    stream << "[" << xyarray << "0 0]";
    stream << si.width << " " << x;

    if ( ps->font.underline() )
        stream << ' ' << y + d->fm.underlinePos() + d->fm.lineWidth()
               << " " << d->fm.lineWidth() << " Tl";
    if ( ps->font.strikeOut() )
        stream << ' ' << y + d->fm.strikeOutPos()
               << " " << d->fm.lineWidth() << " Tl";
    stream << " XYT\n";

}


void QPSPrinterFontTTF::download(QTextStream& s,bool global)
{
    //qDebug("downloading ttf font %s", psname.latin1() );
    //qDebug("target type=%d", target_type);
    global_dict = global;
    QMap<unsigned short, unsigned short> *subsetDict = &subset;
    if ( !global )
        subsetDict = &page_subset;

    downloaded  = TRUE;

    emitPSFontNameList( s, psname, replacementList );
    if (defective) {
        s << "% Font ";
        s << FullName;
        s << " cannot be downloaded\n";
        return;
    }

    // === write header ===
    int VMMin;
    int VMMax;

    s << wrapDSC( "%%BeginFont: " + FullName );
    if( target_type == 42 ) {
        s << "%!PS-TrueTypeFont-"
          << TTVersion.whole
          << "."
          << TTVersion.fraction
          << "-"
          << MfrRevision.whole
          << "."
          << MfrRevision.fraction
          << "\n";
    } else {
        /* If it is not a Type 42 font, we will use a different format. */
        s << "%!PS-Adobe-3.0 Resource-Font\n";
    }     /* See RBIIp 641 */

    if( Copyright != (char*)NULL ) {
        s << wrapDSC( "%%Copyright: " + Copyright );
    }

    if( target_type == 42 )
        s << "%%Creator: Converted from TrueType to type 42 by Qt\n";
    else
        s << "%%Creator: Converted from TrueType by Qt\n";

    /* If VM usage information is available, print it. */
    if( target_type == 42 && post_table)
    {
        VMMin = (int)getULONG( post_table + 16 );
        VMMax = (int)getULONG( post_table + 20 );
        if( VMMin > 0 && VMMax > 0 )
            s << "%%VMUsage: " << VMMin << " " << VMMax << "\n";
    }

    /* Start the dictionary which will eventually */
    /* become the font. */
    if( target_type != 3 ) {
        s << "15 dict begin\n";
    }  else {
        s << "25 dict begin\n";

        /* Type 3 fonts will need some subroutines here. */
        s << "/_d{bind def}bind def\n";
        s << "/_m{moveto}_d\n";
        s << "/_l{lineto}_d\n";
        s << "/_cl{closepath eofill}_d\n";
        s << "/_c{curveto}_d\n";
        s << "/_sc{7 -1 roll{setcachedevice}{pop pop pop pop pop pop}ifelse}_d\n";
        s << "/_e{exec}_d\n";
    }

    s << "/FontName /";
    s << psname;
    s << " def\n";
    s << "/PaintType 0 def\n";

    if(target_type == 42)
        s << "/FontMatrix[1 0 0 1 0 0]def\n";
    else
        s << "/FontMatrix[.001 0 0 .001 0 0]def\n";

    s << "/FontBBox[";
    s<< llx;
    s << " ";
    s<< lly;
    s << " ";
    s<< urx;
    s << " ";
    s<< ury;
    s << "]def\n";

    s << "/FontType ";
    s<< target_type;
    s << " def\n";

    // === write encoding ===

    s << "/Encoding StandardEncoding def\n";

    // === write fontinfo dict ===

    /* We create a sub dictionary named "FontInfo" where we */
    /* store information which though it is not used by the */
    /* interpreter, is useful to some programs which will */
    /* be printing with the font. */
    s << "/FontInfo 10 dict dup begin\n";

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
    if ( post_table ) {
        Fixed ItalicAngle = getFixed( post_table + 4 );
        s << "/ItalicAngle ";
        s << ItalicAngle.whole;
        s << ".";
        s << ItalicAngle.fraction;
        s << " def\n";

        s << "/isFixedPitch ";
        s << (getULONG( post_table + 12 ) ? "true" : "false" );
        s << " def\n";

        s << "/UnderlinePosition ";
        s << (int)getFWord( post_table + 8 );
        s << " def\n";

        s << "/UnderlineThickness ";
        s << (int)getFWord( post_table + 10 );
        s << " def\n";
    }
    s << "end readonly def\n";

#ifdef Q_PRINTER_USE_TYPE42
    /* If we are generating a type 42 font, */
    /* emmit the sfnts array. */
    if( target_type == 42 )
        download_sfnts(s);
#endif
    /* If we are generating a Type 3 font, we will need to */
    /* have the 'loca' and 'glyf' tables around while */
    /* we are generating the CharStrings. */
    if(target_type == 3)
    {
        const BYTE *ptr = getTable("hhea");
        numberOfHMetrics = getUSHORT(ptr + 34);

        loca_table = getTable("loca");
        glyf_table = getTable("glyf");
        hmtx_table = getTable("hmtx");
    }

    // ===  CharStrings array ===

    // subsetting. We turn a char subset into a glyph subset
    // and we mark as used the base glyphs of used composite glyphs.

    bool glyphset[65536];
    for(int c=0; c < 65536; c++)
        glyphset[c] = FALSE;
    glyphset[0] = TRUE; // always output .notdef

    QMap<unsigned short, unsigned short>::iterator it;
    for( it = subsetDict->begin(); it != subsetDict->end(); ++it ) {
	subsetGlyph( it.key(), glyphset );
    }
    int nGlyphs = numGlyphs;
    if ( target_type == 3 ) {
        nGlyphs = 0;;
        for(int c=0; c < 65536; c++)
            if ( glyphset[c] ) nGlyphs++;
    }

    s << "/CharStrings ";
    s << nGlyphs;
    s << " dict dup begin\n";

    // Emmit one key-value pair for each glyph.
    for(int x=0; x < 65536; x++) {
        if(target_type == 42) {
            s << "/";
            s << glyphName( x );
            s << " ";
            s << x;
            s << " def\n";
        } else { /* type 3 */
            if (!glyphset[x]) continue;

	    //qDebug("emitting charproc for glyph %d, name=%s", x, glyphName(x).latin1() );
            s << "/";
            s << glyphName( x, glyphset );
            s << "{";
            charproc(x,s, glyphset);
            s << "}_d\n";     /* "} bind def" */
        }
    }

    s << "end readonly def\n";

    // === trailer ===

    /* If we are generating a type 3 font, we need to provide */
    /* a BuildGlyph and BuildChar procedures. */
    if( target_type == 3 ) {
        s << "\n";

        s << "/BuildGlyph\n";
        s << " {exch begin\n";              /* start font dictionary */
        s << " CharStrings exch\n";
        s << " 2 copy known not{pop /.notdef}if\n";
        s << " true 3 1 roll get exec\n";
        s << " end}_d\n";

        s << "\n";

        /* This procedure is for compatibility with */
        /* level 1 interpreters. */
        s << "/BuildChar {\n";
        s << " 1 index /Encoding get exch get\n";
        s << " 1 index /BuildGlyph get exec\n";
        s << "}_d\n";

        s << "\n";

    }

    /* If we are generating a type 42 font, we need to check to see */
    /* if this PostScript interpreter understands type 42 fonts.  If */
    /* it doesn't, we will hope that the Apple TrueType rasterizer */
    /* has been loaded and we will adjust the font accordingly. */
    /* I found out how to do this by examining a TrueType font */
    /* generated by a Macintosh.  That is where the TrueType interpreter */
    /* setup instructions and part of BuildGlyph came from. */
    else if( target_type == 42 ) {
        s << "\n";

        /* If we have no "resourcestatus" command, or FontType 42 */
        /* is unknown, leave "true" on the stack. */
        s << "systemdict/resourcestatus known\n";
        s << " {42 /FontType resourcestatus\n";
        s << "   {pop pop false}{true}ifelse}\n";
        s << " {true}ifelse\n";

        /* If true, execute code to produce an error message if */
        /* we can't find Apple's TrueDict in VM. */
        s << "{/TrueDict where{pop}{(%%[ Error: no TrueType rasterizer ]%%)= flush}ifelse\n";

        /* Since we are expected to use Apple's TrueDict TrueType */
        /* reasterizer, change the font type to 3. */
        s << "/FontType 3 def\n";

        /* Define a string to hold the state of the Apple */
        /* TrueType interpreter. */
        s << " /TrueState 271 string def\n";

        /* It looks like we get information about the resolution */
        /* of the printer and store it in the TrueState string. */
        s << " TrueDict begin sfnts save\n";
        s << " 72 0 matrix defaultmatrix dtransform dup\n";
        s << " mul exch dup mul add sqrt cvi 0 72 matrix\n";
        s << " defaultmatrix dtransform dup mul exch dup\n";
        s << " mul add sqrt cvi 3 -1 roll restore\n";
        s << " TrueState initer end\n";

        /* This BuildGlyph procedure will look the name up in the */
        /* CharStrings array, and then check to see if what it gets */
        /* is a procedure.  If it is, it executes it, otherwise, it */
        /* lets the TrueType rasterizer loose on it. */

        /* When this procedure is executed the stack contains */
        /* the font dictionary and the character name.  We */
        /* exchange arguments and move the dictionary to the */
        /* dictionary stack. */
        s << " /BuildGlyph{exch begin\n";
        /* stack: charname */

        /* Put two copies of CharStrings on the stack and consume */
        /* one testing to see if the charname is defined in it, */
        /* leave the answer on the stack. */
        s << "  CharStrings dup 2 index known\n";
        /* stack: charname CharStrings bool */

        /* Exchange the CharStrings dictionary and the charname, */
        /* but if the answer was false, replace the character name */
        /* with ".notdef". */
        s << "    {exch}{exch pop /.notdef}ifelse\n";
        /* stack: CharStrings charname */

        /* Get the value from the CharStrings dictionary and see */
        /* if it is executable. */
        s << "  get dup xcheck\n";
        /* stack: CharStrings_entry */

        /* If is a procedure.  Execute according to RBIIp 277-278. */
        s << "    {currentdict systemdict begin begin exec end end}\n";

        /* Is a TrueType character index, let the rasterizer at it. */
        s << "    {TrueDict begin /bander load cvlit exch TrueState render end}\n";

        s << "    ifelse\n";

        /* Pop the font's dictionary off the stack. */
        s << " end}bind def\n";

        /* This is the level 1 compatibility BuildChar procedure. */
        /* See RBIIp 281. */
        s << " /BuildChar{\n";
        s << "  1 index /Encoding get exch get\n";
        s << "  1 index /BuildGlyph get exec\n";
        s << " }bind def\n";

        /* Here we close the condition which is true */
        /* if the printer has no built-in TrueType */
        /* rasterizer. */
        s << "}if\n";
        s << "\n";
    } /* end of if Type 42 not understood. */

    s << "FontName currentdict end definefont pop\n";

    downloadMapping(s, global);
    s << "%%EndFont\n";
}

const BYTE* QPSPrinterFontTTF::getTable(const char* name)
{
    const BYTE *ptr;
    int x;

    /* We must search the table directory. */
    ptr = offset_table + 12;
    x=0;
    while (x != numTables) {
        if( strncmp((const char *)ptr,name,4) == 0 ) {
            ULONG offset;
            //ULONG length;
            const BYTE *table;

            offset = getULONG( ptr + 8 );
            //length = getULONG( ptr + 12 );

            table = offset_table + offset;
            return table;
        }

        x++;
        ptr += 16;
    }

    return 0;
}

void QPSPrinterFontTTF::uni2glyphSetup()
{
    uni2glyph.resize(65536);
    for (int i=0; i<65536; i++)
	uni2glyph[i] = 0x0000;
    glyph2uni.resize(65536);
    for (int i=0; i<65536; i++)
	glyph2uni[i] = 0x0000;

    const BYTE* cmap = getTable("cmap");
    int pos = 0;

    //USHORT version = getUSHORT(cmap + pos);
    pos += 2;
    USHORT nmaps   = getUSHORT(cmap + pos); pos += 2;

    //fprintf(stderr,"cmap version %d (should be 0), %d maps\n",version,nmaps);

    ULONG offset = 0;
    int map = -1;
    bool symbol = TRUE;
    for (int i=0; i<nmaps; i++) {
	USHORT platform = getUSHORT(cmap+pos); pos+=2;
	USHORT encoding = getUSHORT(cmap+pos); pos+=2;
	offset   = getULONG( cmap+pos); pos+=4;
	//fprintf(stderr,"[%d] plat %d enc %d\n",i,platform,encoding);
	if (platform == 3 && encoding == 1) {
	    map = i;
	    symbol = FALSE;
	    break; // unicode
	}
	if (platform == 3 && encoding == 0) {
	    // symbol, continue looking
	    map = i;
	}
    }
    if (map==nmaps) {
	qWarning("Font does not have unicode encoding");
	return; // no unicode encoding!
    }

    pos = 8*map;
    //fprintf(stderr,"Doing Unicode encoding\n");

    pos = offset;
    USHORT format = getUSHORT(cmap+pos); pos+=2;
    //fprintf(stderr,"Unicode cmap format %d\n",format);

    if (format != 4) {
	//qWarning("Unicode cmap format is not 4");
	return;
    }

    pos += 2; // length
    pos += 2; // version
    USHORT segcount = getUSHORT(cmap+pos) / 2; pos+=2;

    //fprintf(stderr,"Unicode cmap seg count %d\n",segcount);

    // skip search data
    pos += 2;
    pos += 2;
    pos += 2;

    const BYTE* endcode    = cmap + offset + 14;
    const BYTE* startcode  = cmap + offset + 16 + 2*segcount;
    const BYTE* iddelta    = cmap + offset + 16 + 4*segcount;
    const BYTE* idrangeoff = cmap + offset + 16 + 6*segcount;
    //unsigned char* glyphid    = cmap + offset + 16 + 8*segcount;
    for (int i=0; i<segcount; i++) {
	USHORT endcode_i    = getUSHORT(endcode   +2*i);
	USHORT startcode_i  = getUSHORT(startcode +2*i);
	SHORT iddelta_i    = getSHORT(iddelta   +2*i);
	USHORT idrangeoff_i = getUSHORT(idrangeoff+2*i);

//     fprintf(stderr,"[%d] %04x-%04x (%x %x)\n",
//         i,startcode_i,endcode_i,iddelta_i,idrangeoff_i);
	if (endcode_i == 0xffff) break; // last dummy segment

	if (idrangeoff_i == 0) {
	    for (USHORT c = startcode_i; c <= endcode_i; c++) {
		USHORT g = c + iddelta_i; // glyph index
		if ( g != 0 ) {
		    uni2glyph[g] = c;
		    glyph2uni[c] = g;
		}
	    }
	} else {
	    for (USHORT c = startcode_i; c <= endcode_i; c++) {
		USHORT g = getUSHORT(idrangeoff+2*i
				     + 2*(c - startcode_i)
				     + idrangeoff_i);
		if ( g != 0 ) {
		    uni2glyph[g] = c;
		    glyph2uni[c] = g;
		}
	    }
	}
    }
    if (symbol && glyph2uni[0x40] == 0 && glyph2uni[0xf040] != 0) {
	// map 0xf000-0xf0ff into latin1 range.
	for (int i = 0; i < 0x100; ++i) {
	    if (!glyph2uni[i])
		glyph2uni[i] = glyph2uni[i+0xf000];

	}
    }
}

USHORT QPSPrinterFontTTF::unicode_for_glyph(int glyphindex)
{
    return uni2glyph[glyphindex];
}

USHORT QPSPrinterFontTTF::glyph_for_unicode(unsigned short unicode)
{
    return glyph2uni[unicode];
}

#ifdef Q_PRINTER_USE_TYPE42
// ****************** SNFTS ROUTINES *******

/*-------------------------------------------------------------------
** sfnts routines
** These routines generate the PostScript "sfnts" array which
** contains one or more strings which contain a reduced version
** of the TrueType font.
**
** A number of functions are required to accomplish this rather
** complicated task.
-------------------------------------------------------------------*/

// Write a BYTE as a hexadecimal value as part of the sfnts array.

void QPSPrinterFontTTF::sfnts_pputBYTE(BYTE n,QTextStream& s,
                                  int& string_len, int& line_len, bool& in_string)
{
    static const char hexdigits[]="0123456789ABCDEF";

    if(!in_string) {
	s << "<";
	string_len = 0;
	line_len++;
	in_string = TRUE;
    }

    s << hexdigits[ n / 16 ] ;
    s << hexdigits[ n % 16 ] ;
    string_len++;
    line_len+=2;

    if(line_len > 70) {
	s << "\n";
	line_len=0;
    }
}

// Write a USHORT as a hexadecimal value as part of the sfnts array.

void QPSPrinterFontTTF::sfnts_pputUSHORT(USHORT n,QTextStream& s,
                                  int& string_len, int& line_len, bool& in_string)
{
    sfnts_pputBYTE(n / 256,s, string_len, line_len, in_string);
    sfnts_pputBYTE(n % 256,s, string_len, line_len, in_string);
}


// Write a ULONG as part of the sfnts array.

void QPSPrinterFontTTF::sfnts_pputULONG(ULONG n,QTextStream& s,
                                  int& string_len, int& line_len, bool& in_string)
{
    int x1 = n % 256;   n /= 256;
    int x2 = n % 256;   n /= 256;
    int x3 = n % 256;   n /= 256;

    sfnts_pputBYTE(n,s , string_len, line_len, in_string);
    sfnts_pputBYTE(x3,s, string_len, line_len, in_string);
    sfnts_pputBYTE(x2,s, string_len, line_len, in_string);
    sfnts_pputBYTE(x1,s, string_len, line_len, in_string);
}

/*
** This is called whenever it is
** necessary to end a string in the sfnts array.
**
** (The array must be broken into strings which are
** no longer than 64K characters.)
*/
void QPSPrinterFontTTF::sfnts_end_string(QTextStream& s,
                                    int& string_len, int& line_len, bool& in_string)
{
    if(in_string) {
	string_len=0;               /* fool sfnts_pputBYTE() */

	// s << "\n% dummy byte:\n";

	// extra byte for pre-2013 compatibility
	sfnts_pputBYTE(0, s, string_len, line_len, in_string);

	s << ">";
	line_len++;
    }

    in_string=FALSE;
}

/*
** This is called at the start of each new table.
** The argement is the length in bytes of the table
** which will follow.  If the new table will not fit
** in the current string, a new one is started.
*/
void QPSPrinterFontTTF::sfnts_new_table(ULONG length,QTextStream& s,
                                   int& string_len, int& line_len, bool& in_string)
{
    if( (string_len + length) > 65528 )
	sfnts_end_string(s, string_len, line_len, in_string);
}

/*
** We may have to break up the 'glyf' table.  That is the reason
** why we provide this special routine to copy it into the sfnts
** array.
*/
void QPSPrinterFontTTF::sfnts_glyf_table(ULONG oldoffset,
                                    ULONG correct_total_length,
                                    QTextStream& s,
                                    int& string_len, int& line_len, bool& in_string)

{
    int x;
    ULONG off;
    ULONG length;
    int c;
    ULONG total=0;                /* running total of bytes written to table */

    loca_table = getTable("loca");

    int font_off = oldoffset;

    /* Copy the glyphs one by one */
    for(x=0; x < numGlyphs; x++) {
	/* Read the glyph offset from the index-to-location table. */
	if(indexToLocFormat == 0) {
	    off = getUSHORT( loca_table + (x * 2) );
	    off *= 2;
	    length = getUSHORT( loca_table + ((x+1) * 2) );
	    length *= 2;
	    length -= off;
	} else {
	    off = getULONG( loca_table + (x * 4) );
	    length = getULONG( loca_table + ((x+1) * 4) );
	    length -= off;
	}

	//  fprintf(stderr,"glyph length=%d",(int)length);

	/* Start new string if necessary. */
	sfnts_new_table( (int)length, s, string_len, line_len, in_string );

	/*
	** Make sure the glyph is padded out to a
	** two byte boundary.
	*/
	if( length % 2 ) {
	    qWarning("TrueType font contains a 'glyf' table without 2 byte padding");
	    defective = TRUE;
	    return;
	}

	/* Copy the bytes of the glyph. */
	while( length-- ) {
	    c = offset_table[ font_off ];
	    font_off++;

	    sfnts_pputBYTE(c, s, string_len, line_len, in_string);
	    total++;          /* add to running total */
	}
    }

    /* Pad out to full length from table directory */
    while( total < correct_total_length ) {
	sfnts_pputBYTE(0, s, string_len, line_len, in_string);
	total++;
    }

    /* Look for unexplainable descrepancies between sizes */
    if( total != correct_total_length ) {
	qWarning("QPSPrinterFontTTF::sfnts_glyf_table: total != correct_total_length");
	defective = TRUE;
	return;
    }
}

/*
** Here is the routine which ties it all together.
**
** Create the array called "sfnts" which
** holds the actual TrueType data.
*/

void QPSPrinterFontTTF::download_sfnts(QTextStream& s)
{
    // tables worth including in a type 42 font
    const char *table_names[]= {
	"cvt ",
	"fpgm",
	"glyf",
	"head",
	"hhea",
	"hmtx",
	"loca",
	"maxp",
	"prep"
    };

    struct {                      /* The location of each of */
	ULONG oldoffset;    /* the above tables. */
	ULONG newoffset;
	ULONG length;
	ULONG checksum;
    } tables[9];

    int c;                        /* Input character. */
    int diff;
    int count;                    /* How many `important' tables did we find? */

    BYTE* ptr = offset_table + 12; // original table directory
    ULONG nextoffset=0;
    count=0;

    /*
    ** Find the tables we want and store there vital
    ** statistics in tables[].
    */
    for(int x=0; x < 9; x++ ) {
	do {
	    diff = strncmp( (char*)ptr, table_names[x], 4 );

	    if( diff > 0 ) {          /* If we are past it. */
		tables[x].length = 0;
		diff = 0;
	    }
	    else if( diff < 0 ) {             /* If we haven't hit it yet. */
		ptr += 16;
	    }
	    else if( diff == 0 ) {    /* Here it is! */
		tables[x].newoffset = nextoffset;
		tables[x].checksum = getULONG( ptr + 4 );
		tables[x].oldoffset = getULONG( ptr + 8 );
		tables[x].length = getULONG( ptr + 12 );
		nextoffset += ( ((tables[x].length + 3) / 4) * 4 );
		count++;
		ptr += 16;
	    }
	} while(diff != 0);
    } /* end of for loop which passes over the table directory */

    /* Begin the sfnts array. */

    s << "/sfnts[<";

    bool in_string=TRUE;
    int  string_len=0;
    int  line_len=8;

    /* Generate the offset table header */
    /* Start by copying the TrueType version number. */
    ptr = offset_table;
    for(int x=0; x < 4; x++)
	sfnts_pputBYTE( *(ptr++) , s, string_len, line_len, in_string );

    /* Now, generate those silly numTables numbers. */
    sfnts_pputUSHORT(count,s, string_len, line_len, in_string);           /* number of tables */
    if( count == 9 ) {
	sfnts_pputUSHORT(7,s, string_len, line_len, in_string);             /* searchRange */
	sfnts_pputUSHORT(3,s, string_len, line_len, in_string);             /* entrySelector */
	sfnts_pputUSHORT(81,s, string_len, line_len, in_string);            /* rangeShift */
    }
    else {
	qWarning("Fewer than 9 tables selected");
    }

    /* Now, emmit the table directory. */
    for(int x=0; x < 9; x++) {
	if( tables[x].length == 0 ) /* Skip missing tables */
	    continue;

	/* Name */
	sfnts_pputBYTE( table_names[x][0], s, string_len, line_len, in_string);
	sfnts_pputBYTE( table_names[x][1], s, string_len, line_len, in_string);
	sfnts_pputBYTE( table_names[x][2], s, string_len, line_len, in_string);
	sfnts_pputBYTE( table_names[x][3], s, string_len, line_len, in_string);

	/* Checksum */
	sfnts_pputULONG( tables[x].checksum, s, string_len, line_len, in_string );

	/* Offset */
	sfnts_pputULONG( tables[x].newoffset + 12 + (count * 16), s,
			 string_len, line_len, in_string );

	/* Length */
	sfnts_pputULONG( tables[x].length, s,
			 string_len, line_len, in_string );
    }

    /* Now, send the tables */
    for(int x=0; x < 9; x++) {
	if( tables[x].length == 0 ) /* skip tables that aren't there */
	    continue;

	/* 'glyf' table gets special treatment */
	if( strcmp(table_names[x],"glyf")==0 ) {
	    sfnts_glyf_table(tables[x].oldoffset,tables[x].length, s,
			     string_len, line_len, in_string);
	} else { // other tables should not exceed 64K (not always true; Sivan)
	    if( tables[x].length > 65535 ) {
		qWarning("TrueType font has a table which is too long");
		defective = TRUE;
		return;
	    }

	    /* Start new string if necessary. */
	    sfnts_new_table(tables[x].length, s,
			    string_len, line_len, in_string);

	    int font_off = tables[x].oldoffset;
	    /* Copy the bytes of the table. */
	    for( int y=0; y < (int)tables[x].length; y++ ) {
		c = offset_table[ font_off ];
		font_off++;

		sfnts_pputBYTE(c, s, string_len, line_len, in_string);
	    }
	}

	/* Padd it out to a four byte boundary. */
	int y=tables[x].length;
	while( (y % 4) != 0 ) {
	    sfnts_pputBYTE(0, s, string_len, line_len, in_string);
	    y++;
	}

    } /* End of loop for all tables */

    /* Close the array. */
    sfnts_end_string(s, string_len, line_len, in_string);
    s << "]def\n";
}
#endif

// ****************** Type 3 CharProcs *******

/*
** This routine is used to break the character
** procedure up into a number of smaller
** procedures.  This is necessary so as not to
** overflow the stack on certain level 1 interpreters.
**
** Prepare to push another item onto the stack,
** starting a new procedure if necessary.
**
** Not all the stack depth calculations in this routine
** are perfectly accurate, but they do the job.
*/
static int stack_depth = 0;
static void stack(int num_pts, int newnew, QTextStream& s)
{
    if( num_pts > 25 ) {          /* Only do something of we will */
	/* have a log of points. */
	if(stack_depth == 0) {
	    s << "{";
	    stack_depth=1;
	}

	stack_depth += newnew;              /* Account for what we propose to add */

	if(stack_depth > 100) {
	    s << "}_e{";
	    stack_depth = 3 + newnew; /* A rough estimate */
	}
    }
}

static void stack_end(QTextStream& s)                   /* called at end */
{
    if(stack_depth) {
	s << "}_e";
	stack_depth=0;
    }
}

// postscript drawing commands

static void PSMoveto(FWord x, FWord y, QTextStream& ts)
{
    ts << x;
    ts << " ";
    ts << y;
    ts << " _m\n";
}

static void PSLineto(FWord x, FWord y, QTextStream& ts)
{
    ts << x;
    ts << " ";
    ts << y;
    ts << " _l\n";
}

/* Emmit a PostScript "curveto" command. */
static void PSCurveto(FWord* xcoor, FWord* ycoor,
                      FWord x, FWord y, int s, int t, QTextStream& ts)
{
    int N, i;
    double sx[3], sy[3], cx[4], cy[4];

    N = t-s+2;
    for(i=0; i<N-1; i++) {
	sx[0] = i==0?xcoor[s-1]:(xcoor[i+s]+xcoor[i+s-1])/2;
	sy[0] = i==0?ycoor[s-1]:(ycoor[i+s]+ycoor[i+s-1])/2;
	sx[1] = xcoor[s+i];
	sy[1] = ycoor[s+i];
	sx[2] = i==N-2?x:(xcoor[s+i]+xcoor[s+i+1])/2;
	sy[2] = i==N-2?y:(ycoor[s+i]+ycoor[s+i+1])/2;
	cx[3] = sx[2];
	cy[3] = sy[2];
	cx[1] = (2*sx[1]+sx[0])/3;
	cy[1] = (2*sy[1]+sy[0])/3;
	cx[2] = (sx[2]+2*sx[1])/3;
	cy[2] = (sy[2]+2*sy[1])/3;

	ts << (int)cx[1];
	ts << " ";
	ts << (int)cy[1];
	ts << " ";
	ts << (int)cx[2];
	ts << " ";
	ts << (int)cy[2];
	ts << " ";
	ts << (int)cx[3];
	ts << " ";
	ts << (int)cy[3];
	ts << " _c\n";
    }
}

/* The PostScript bounding box. */
/* Variables to hold the character data. */

//void load_char(struct TTFONT *font, BYTE *glyph);
//void clear_data();

//void PSMoveto(FWord x, FWord y, QTextStream& ts);
//void PSLineto(FWord x, FWord y, QTextStream& ts);
//void PSCurveto(FWord x, FWord y, int s, int t, QTextStream& ts);

//double area(FWord *x, FWord *y, int n);
//int nextinctr(int co, int ci);
//int nextoutctr(int co);
//int nearout(int ci);
//double intest(int co, int ci);
#define sqr(x) ((x)*(x))

#define NOMOREINCTR -1
#define NOMOREOUTCTR -1

/*
** Find the area of a contour?
*/
static double area(FWord *x, FWord *y, int n)
{
    int i;
    double sum;

    sum=x[n-1]*y[0]-y[n-1]*x[0];
    for (i=0; i<=n-2; i++) sum += x[i]*y[i+1] - y[i]*x[i+1];
    return sum;
}

static int nextoutctr(int /*co*/, charproc_data* cd)
{
    int j;

    for(j=0; j<cd->num_ctr; j++)
	if (cd->check_ctr[j]==0 && cd->area_ctr[j] < 0) {
	    cd->check_ctr[j]=1;
	    return j;
	}

    return NOMOREOUTCTR;
} /* end of nextoutctr() */

static int nextinctr(int co, int /*ci*/, charproc_data* cd)
{
    int j;

    for(j=0; j<cd->num_ctr; j++)
	if (cd->ctrset[2*j+1]==co)
	    if (cd->check_ctr[ cd->ctrset[2*j] ]==0) {
		cd->check_ctr[ cd->ctrset[2*j] ]=1;
		return cd->ctrset[2*j];
	    }

    return NOMOREINCTR;
}

static double intest( int co, int ci, charproc_data *cd )
{
    int i, j, start, end;
    double r1, r2;
    FWord xi[3], yi[3];

    j = start = ( co == 0 ) ? 0 : ( cd->epts_ctr[co - 1] + 1 );
    end = cd->epts_ctr[co];
    i = ( ci == 0 ) ? 0 : ( cd->epts_ctr[ci - 1] + 1 );
    xi[0] = cd->xcoor[i];
    yi[0] = cd->ycoor[i];
    r1 = sqr( cd->xcoor[start] - xi[0] ) + sqr( cd->ycoor[start] - yi[0] );

    for ( i = start; i <= end; i++ ) {
	r2 = sqr( cd->xcoor[i] - xi[0] ) + sqr( cd->ycoor[i] - yi[0] );
	if ( r2 < r1 ) {
	    r1 = r2;
	    j = i;
	}
    }
    if ( j == start ) {
	xi[1] = cd->xcoor[end];
	yi[1] = cd->ycoor[end];
    } else {
	xi[1] = cd->xcoor[j - 1];
	yi[1] = cd->ycoor[j - 1];
    }
    if ( j == end ) {
	xi[2] = cd->xcoor[start];
	yi[2] = cd->ycoor[start];
    } else {
	xi[2] = cd->xcoor[j + 1];
	yi[2] = cd->ycoor[j + 1];
    }
    return area( xi, yi, 3 );
}

/*
** find the nearest out contour to a specified in contour.
*/
static int nearout(int ci, charproc_data* cd)
{
    int k = 0;                    /* !!! is this right? */
    int co;
    double a, a1=0;

    for (co=0; co < cd->num_ctr; co++) {
	if(cd->area_ctr[co] < 0) {
	    a=intest(co,ci, cd);
	    if (a<0 && a1==0) {
		k=co;
		a1=a;
	    }
	    if(a<0 && a1!=0 && a>a1) {
		k=co;
		a1=a;
	    }
	}
    }

    return k;
} /* end of nearout() */


/*
** We call this routine to emmit the PostScript code
** for the character we have loaded with load_char().
*/
static void PSConvert(QTextStream& s, charproc_data* cd)
{
    int i,j,k,fst,start_offpt;
    int end_offpt=0;

    cd->area_ctr = new double[cd->num_ctr];
    memset(cd->area_ctr, 0, (cd->num_ctr*sizeof(double)));

    cd->check_ctr = new char[cd->num_ctr];
    memset(cd->check_ctr, 0, (cd->num_ctr*sizeof(char)));

    cd->ctrset = new int[2*(cd->num_ctr)];
    memset(cd->ctrset, 0, (cd->num_ctr*2*sizeof(int)));

    cd->check_ctr[0]=1;
    cd->area_ctr[0]=area(cd->xcoor, cd->ycoor, cd->epts_ctr[0]+1);

    for (i=1; i<cd->num_ctr; i++)
	cd->area_ctr[i]=area(cd->xcoor+cd->epts_ctr[i-1]+1,
			     cd->ycoor+cd->epts_ctr[i-1]+1,
			     cd->epts_ctr[i]-cd->epts_ctr[i-1]);

    for (i=0; i<cd->num_ctr; i++) {
	if (cd->area_ctr[i]>0) {
	    cd->ctrset[2*i]=i;
	    cd->ctrset[2*i+1]=nearout(i,cd);
	} else {
	    cd->ctrset[2*i]=-1;
	    cd->ctrset[2*i+1]=-1;
	}
    }

    /* Step thru the coutours. */
    /* I believe that a contour is a detatched */
    /* set of curves and lines. */
    i=j=k=0;
    while (i < cd->num_ctr ) {
	fst = j = (k==0) ? 0 : (cd->epts_ctr[k-1]+1);

	/* Move to the first point on the contour. */
	stack(cd->num_pts,3,s);
	PSMoveto(cd->xcoor[j],cd->ycoor[j],s);
	start_offpt = 0;            /* No off curve points yet. */

	/* Step thru the remaining points of this contour. */
	for(j++; j <= cd->epts_ctr[k]; j++) {
	    if (!(cd->tt_flags[j]&1)) { /* Off curve */
		if (!start_offpt)
		{ start_offpt = end_offpt = j; }
		else
		    end_offpt++;
	    } else {                  /* On Curve */
		if (start_offpt) {
		    stack(cd->num_pts,7,s);
		    PSCurveto(cd->xcoor,cd->ycoor,
			      cd->xcoor[j],cd->ycoor[j],
			      start_offpt,end_offpt,s);
		    start_offpt = 0;
		} else {
		    stack(cd->num_pts,3,s);
		    PSLineto(cd->xcoor[j], cd->ycoor[j],s);
		}
	    }
	}

	/* Do the final curve or line */
	/* of this coutour. */
	if (start_offpt) {
	    stack(cd->num_pts,7,s);
	    PSCurveto(cd->xcoor,cd->ycoor,
		      cd->xcoor[fst],cd->ycoor[fst],
		      start_offpt,end_offpt,s);
	} else {
	    stack(cd->num_pts,3,s);
	    PSLineto(cd->xcoor[fst],cd->ycoor[fst],s);
	}

	k=nextinctr(i,k,cd);

	if (k==NOMOREINCTR)
	    i=k=nextoutctr(i,cd);

	if (i==NOMOREOUTCTR)
	    break;
    }

    /* Now, we can fill the whole thing. */
    stack(cd->num_pts,1,s);
    s << "_cl";           /* "closepath eofill" */

    /* Free our work arrays. */
    delete [] cd->area_ctr;
    delete [] cd->check_ctr;
    delete [] cd->ctrset;
}


/*
** Load the simple glyph data pointed to by glyph.
** The pointer "glyph" should point 10 bytes into
** the glyph data.
*/
void QPSPrinterFontTTF::charprocLoad(const BYTE *glyph, charproc_data* cd)
{
    int x;
    BYTE c, ct;

    /* Read the contour endpoints list. */
    cd->epts_ctr = new int[cd->num_ctr];
    //cd->epts_ctr = (int *)myalloc(cd->num_ctr,sizeof(int));
    for (x = 0; x < cd->num_ctr; x++) {
	cd->epts_ctr[x] = getUSHORT(glyph);
	glyph += 2;
    }

    /* From the endpoint of the last contour, we can */
    /* determine the number of points. */
    cd->num_pts = cd->epts_ctr[cd->num_ctr-1]+1;
#ifdef DEBUG_TRUETYPE
    fprintf(stderr,"num_pts=%d\n",cd->num_pts);
#endif

    /* Skip the instructions. */
    x = getUSHORT(glyph);
    glyph += 2;
    glyph += x;

    /* Allocate space to hold the data. */
    //cd->tt_flags = (BYTE *)myalloc(num_pts,sizeof(BYTE));
    //cd->xcoor    = (FWord *)myalloc(num_pts,sizeof(FWord));
    //cd->ycoor    = (FWord *)myalloc(num_pts,sizeof(FWord));
    cd->tt_flags = new BYTE[cd->num_pts];
    cd->xcoor    = new FWord[cd->num_pts];
    cd->ycoor    = new FWord[cd->num_pts];

    /* Read the flags array, uncompressing it as we go. */
    /* There is danger of overflow here. */
    for (x = 0; x < cd->num_pts; ) {
	cd->tt_flags[x++] = c = *(glyph++);

	if (c&8) {          /* If next byte is repeat count, */
	    ct = *(glyph++);

	    if( (x + ct) > cd->num_pts ) {
		qWarning("Fatal Error in TT flags");
		return;
	    }

	    while (ct--)
		cd->tt_flags[x++] = c;
	}
    }

    /* Read the x coordinates */
    for (x = 0; x < cd->num_pts; x++) {
	if (cd->tt_flags[x] & 2) {          /* one byte value with */
	    /* external sign */
	    c = *(glyph++);
	    cd->xcoor[x] = (cd->tt_flags[x] & 0x10) ? c : (-1 * (int)c);
	} else if(cd->tt_flags[x] & 0x10) { /* repeat last */
	    cd->xcoor[x] = 0;
	} else {                            /* two byte signed value */
	    cd->xcoor[x] = getFWord(glyph);
	    glyph+=2;
	}
    }

    /* Read the y coordinates */
    for(x = 0; x < cd->num_pts; x++) {
	if (cd->tt_flags[x] & 4) {          /* one byte value with */
	    /* external sign */
	    c = *(glyph++);
	    cd->ycoor[x] = (cd->tt_flags[x] & 0x20) ? c : (-1 * (int)c);
	} else if (cd->tt_flags[x] & 0x20) {        /* repeat last value */
	    cd->ycoor[x] = 0;
	} else {                    /* two byte signed value */
	    cd->ycoor[x] = getUSHORT(glyph);
	    glyph+=2;
	}
    }

    /* Convert delta values to absolute values. */
    for(x = 1; x < cd->num_pts; x++) {
	cd->xcoor[x] += cd->xcoor[x-1];
	cd->ycoor[x] += cd->ycoor[x-1];
    }

    for(x=0; x < cd->num_pts; x++) {
	cd->xcoor[x] = topost(cd->xcoor[x]);
	cd->ycoor[x] = topost(cd->ycoor[x]);
    }
}

#define ARG_1_AND_2_ARE_WORDS 1
#define ARGS_ARE_XY_VALUES 2
#define ROUND_XY_TO_GRID 4
#define WE_HAVE_A_SCALE 8
/* RESERVED 16 */
#define MORE_COMPONENTS 32
#define WE_HAVE_AN_X_AND_Y_SCALE 64
#define WE_HAVE_A_TWO_BY_TWO 128
#define WE_HAVE_INSTRUCTIONS 256
#define USE_MY_METRICS 512

void QPSPrinterFontTTF::subsetGlyph(int charindex,bool* glyphset)
{
    USHORT flags;
    USHORT glyphIndex;
    charproc_data cd;

    glyphset[charindex] = TRUE;
    //printf("subsetting %s ==> ",glyphName(charindex).latin1());

    /* Get a pointer to the data. */
    const BYTE* glyph = charprocFindGlyphData( charindex );

    /* If the character is blank, it has no bounding box, */
    /* otherwise read the bounding box. */
    if( glyph == (BYTE*)NULL ) {
	cd.num_ctr=0;
    } else {
	cd.num_ctr = getSHORT(glyph);
	/* Advance the pointer past bounding box. */
	glyph += 10;
    }

    if( cd.num_ctr < 0 ) { // composite
	/* Once around this loop for each component. */
	do {
	    flags = getUSHORT(glyph); /* read the flags word */
	    glyph += 2;
	    glyphIndex = getUSHORT(glyph);    /* read the glyphindex word */
	    glyph += 2;

	    glyphset[ glyphIndex ] = TRUE;
	    subsetGlyph( glyphIndex, glyphset );
	    //printf("subset contains: %d %s ",glyphIndex, glyphName(glyphIndex).latin1());

	    if(flags & ARG_1_AND_2_ARE_WORDS) {
		glyph += 2;
		glyph += 2;
	    } else {
		glyph += 1;
		glyph += 1;
	    }

	    if(flags & WE_HAVE_A_SCALE) {
		glyph += 2;
	    } else if(flags & WE_HAVE_AN_X_AND_Y_SCALE) {
		glyph += 2;
		glyph += 2;
	    } else if(flags & WE_HAVE_A_TWO_BY_TWO) {
		glyph += 2;
		glyph += 2;
		glyph += 2;
		glyph += 2;
	    } else {
	    }
	} while(flags & MORE_COMPONENTS);
    }
    //printf("\n");
}


/*
** Emmit PostScript code for a composite character.
*/
void QPSPrinterFontTTF::charprocComposite(const BYTE *glyph, QTextStream& s, bool *glyphSet)
{
    USHORT flags;
    USHORT glyphIndex;
    int arg1;
    int arg2;
    float xscale = 1;
    float yscale = 1;
#ifdef DEBUG_TRUETYPE
    float scale01 = 0;
    float scale10 = 0;
#endif

    /* Once around this loop for each component. */
    do {
	flags = getUSHORT(glyph);   /* read the flags word */
	glyph += 2;

	glyphIndex = getUSHORT(glyph);      /* read the glyphindex word */
	glyph += 2;

	if(flags & ARG_1_AND_2_ARE_WORDS) {
	    /* The tt spec. seems to say these are signed. */
	    arg1 = getSHORT(glyph);
	    glyph += 2;
	    arg2 = getSHORT(glyph);
	    glyph += 2;
	} else {                    /* The tt spec. does not clearly indicate */
	    /* whether these values are signed or not. */
	    arg1 = (char)*(glyph++);
	    arg2 = (char)*(glyph++);
	}

	if(flags & WE_HAVE_A_SCALE) {
	    xscale = yscale = f2dot14( getUSHORT(glyph) );
	    glyph += 2;
	} else if(flags & WE_HAVE_AN_X_AND_Y_SCALE) {
	    xscale = f2dot14( getUSHORT(glyph) );
	    glyph += 2;
	    yscale = f2dot14( getUSHORT(glyph) );
	    glyph += 2;
	} else if(flags & WE_HAVE_A_TWO_BY_TWO) {
	    xscale = f2dot14( getUSHORT(glyph) );
	    glyph += 2;
#ifdef DEBUG_TRUETYPE
	    scale01 = f2dot14( getUSHORT(glyph) );
#endif
	    glyph += 2;
#ifdef DEBUG_TRUETYPE
	    scale10 = f2dot14( getUSHORT(glyph) );
#endif
	    glyph += 2;
	    yscale = f2dot14( getUSHORT(glyph) );
	    glyph += 2;
	}

	/* Debugging */
#ifdef DEBUG_TRUETYPE
	s << "% flags=" << flags << ", arg1=" << arg1 << ", arg2=" << arg2 << ", xscale=" << xscale << ", yscale=" << yscale <<
	    ", scale01=" << scale01 << ", scale10=" << scale10 << endl;
#endif


	if ( (flags & ARGS_ARE_XY_VALUES) != ARGS_ARE_XY_VALUES ) {
	    s << "% unimplemented shift, arg1=" << arg1;
	    s << ", arg2=" << arg2 << "\n";
	    arg1 = arg2 = 0;
	}

	/* If we have an (X,Y) shif and it is non-zero, */
	/* translate the coordinate system. */
	if ( flags & (WE_HAVE_A_TWO_BY_TWO|WE_HAVE_AN_X_AND_Y_SCALE) ) {
#if 0
	    // code similar to this would be needed for two_by_two
	    s << "gsave [ " << xscale << " " << scale01 << " " << scale10 << " "
	      << yscale << " " << topost(arg1) << " " << topost(arg2) << "] SM\n";
#endif
	    if ( flags & WE_HAVE_A_TWO_BY_TWO )
		s << "% Two by two transformation, unimplemented\n";
	    s << "gsave " << topost(arg1);
	    s << " " << topost(arg2);
	    s << " translate\n";
	    s << xscale << " " << yscale << " scale\n";
	} else if ( flags & ARGS_ARE_XY_VALUES && ( arg1 != 0 || arg2 != 0 ) ) {
	    s << "gsave " << topost(arg1);
	    s << " " << topost(arg2);
	    s << " translate\n";
	}

	/* Invoke the CharStrings procedure to print the component. */
	s << "false CharStrings /";
	s << glyphName( glyphIndex, glyphSet );
	s << " get exec\n";

	//  printf("false CharStrings /%s get exec\n",
	//ttfont_CharStrings_getname(font,glyphIndex));

	/* If we translated the coordinate system, */
	/* put it back the way it was. */
	if( (flags & ARGS_ARE_XY_VALUES && (arg1 != 0 || arg2 != 0) ) ||
	    ( flags & (WE_HAVE_A_TWO_BY_TWO|WE_HAVE_AN_X_AND_Y_SCALE) ) ) {
	    s << "grestore ";
	}
    } while (flags & MORE_COMPONENTS);
}

/*
** Return a pointer to a specific glyph's data.
*/
const BYTE* QPSPrinterFontTTF::charprocFindGlyphData(int charindex)
{
    ULONG off;
    ULONG length;

    /* Read the glyph offset from the index to location table. */
    if(indexToLocFormat == 0) {
	off = getUSHORT( loca_table + (charindex * 2) );
	off *= 2;
	length = getUSHORT( loca_table + ((charindex+1) * 2) );
	length *= 2;
	length -= off;
    } else {
	off = getULONG( loca_table + (charindex * 4) );
	length = getULONG( loca_table + ((charindex+1) * 4) );
	length -= off;
    }

    if(length > 0)
	return glyf_table + off;
    else
	return (BYTE*)NULL;
}

void QPSPrinterFontTTF::charproc(int charindex, QTextStream& s, bool *glyphSet )
{
    int llx,lly,urx,ury;
    int advance_width;
    charproc_data cd;

#ifdef DEBUG_TRUETYPE
    s << "% tt_type3_charproc for ";
    s << charindex;
    s << "\n";
#endif

    /* Get a pointer to the data. */
    const BYTE* glyph = charprocFindGlyphData( charindex );

    /* If the character is blank, it has no bounding box, */
    /* otherwise read the bounding box. */
    if( glyph == (BYTE*)NULL ) {
	llx=lly=urx=ury=0;  /* A blank char has an all zero BoundingBox */
	cd.num_ctr=0;               /* Set this for later if()s */
    } else {
	/* Read the number of contours. */
	cd.num_ctr = getSHORT(glyph);

	/* Read PostScript bounding box. */
	llx = getFWord(glyph + 2);
	lly = getFWord(glyph + 4);
	urx = getFWord(glyph + 6);
	ury = getFWord(glyph + 8);

	/* Advance the pointer. */
	glyph += 10;
    }

    /* If it is a simple character, load its data. */
    if (cd.num_ctr > 0)
	charprocLoad(glyph, &cd);
    else
	cd.num_pts=0;

    /* Consult the horizontal metrics table to determine */
    /* the character width. */
    if( charindex < numberOfHMetrics )
	advance_width = getuFWord( hmtx_table + (charindex * 4) );
    else
	advance_width = getuFWord( hmtx_table + ((numberOfHMetrics-1) * 4) );

    /* Execute setcachedevice in order to inform the font machinery */
    /* of the character bounding box and advance width. */
    stack(cd.num_pts,7,s);
    s << topost(advance_width);
    s << " 0 ";
    s << topost(llx);
    s << " ";
    s << topost(lly);
    s << " ";
    s << topost(urx);
    s << " ";
    s << topost(ury);
    s << " _sc\n";

    /* If it is a simple glyph, convert it, */
    /* otherwise, close the stack business. */
    if( cd.num_ctr > 0 ) {        // simple
	PSConvert(s,&cd);
	delete [] cd.tt_flags;
	delete [] cd.xcoor;
	delete [] cd.ycoor;
	delete [] cd.epts_ctr;
    } else if( cd.num_ctr < 0 ) { // composite
	charprocComposite(glyph,s, glyphSet);
    }

    stack_end(s);
} /* end of tt_type3_charproc() */


// ================== PFA ====================

class QPSPrinterFontPFA
    : public QPSPrinterFontPrivate {
public:
    QPSPrinterFontPFA(const QFontEngine *f, QByteArray& data);
    virtual void    download(QTextStream& s, bool global);
    virtual bool embedded() { return TRUE; }
private:
    QByteArray     data;
};

QPSPrinterFontPFA::QPSPrinterFontPFA(const QFontEngine *f, QByteArray& d)
{
    data = d;

    int pos = 0;
    const char* p = data.constData();
    QString fontname;

    if (p[ pos ] != '%' || p[ pos+1 ] != '!') { // PFA marker
	qWarning("invalid pfa file");
	return;
    }

    const char* fontnameptr = strstr(p+pos,"/FontName");
    if (fontnameptr == NULL)
	return;

    fontnameptr += strlen("/FontName") + 1;
    while (*fontnameptr == ' ' || *fontnameptr == '/') fontnameptr++;
    int l=0;
    while (fontnameptr[l] != ' ') l++;

    psname = QString::fromLatin1(fontnameptr,l);
    replacementList = makePSFontNameList( f, psname );
}

void QPSPrinterFontPFA::download(QTextStream& s, bool global)
{
    //qDebug("downloading pfa font %s", psname.latin1() );
    char* p = data.data();

    emitPSFontNameList( s, psname, replacementList );
    s << "% Font resource\n";
    for (int i=0; i < (int)data.size(); i++) s << p[i];
    s << "% End of font resource\n";
    downloadMapping( s, global );
}

// ================== PFB ====================

class QPSPrinterFontPFB
    : public QPSPrinterFontPrivate {
public:
    QPSPrinterFontPFB(const QFontEngine *f, QByteArray& data);
    virtual void    download(QTextStream& s, bool global);
    virtual bool embedded() { return TRUE; }
private:
    QByteArray     data;
};

QPSPrinterFontPFB::QPSPrinterFontPFB(const QFontEngine *f, QByteArray& d)
{
    data = d;

    int pos = 0;
    int len;
    //  int typ;
    const BYTE* p = (const BYTE *)data.constData();
    QString fontname;

    if (p[ pos ] != 0x80) { // PFB marker
	qWarning("pfb file does not start with 0x80");
	return;
    }
    pos++;
    //  typ = p[ pos ]; // 1=ascii 2=binary 3=done
    pos++;
    len = p[ pos ];          pos++;
    len |= (p[ pos ] << 8) ; pos++;
    len |= (p[ pos ] << 16); pos++;
    len |= (p[ pos ] << 24); pos++;

    //printf("font block type %d len %d\n",typ,len);

    char* fontnameptr = strstr((char*)p+pos,"/FontName");
    if (fontnameptr == NULL)
	return;

    fontnameptr += strlen("/FontName") + 1;
    while (*fontnameptr == ' ' || *fontnameptr == '/') fontnameptr++;
    int l=0;
    while (fontnameptr[l] != ' ') l++;

    psname = QString::fromLatin1(fontnameptr,l);
    replacementList = makePSFontNameList( f, psname );
}

void QPSPrinterFontPFB::download(QTextStream& s, bool global)
{
    //qDebug("downloading pfb font %s", psname.latin1() );
    const unsigned char* p = (const BYTE *)data.constData();
    int pos;
    int len;
    int typ;

    int hexcol = 0;
    int line_length = 64;

    emitPSFontNameList( s, psname, replacementList );
    s << "% Font resource\n";

    pos = 0;
    typ = -1;
    while (typ != 3) { // not end of file
	if (p[ pos ] != 0x80) // PFB marker
	    return; // pfb file does not start with 0x80
	pos++;
	typ = p[ pos ]; // 1=ascii 2=binary 3=done
	pos++;

	if (typ == 3) break;

	len = p[ pos ];         pos++;
	len |= (p[ pos ] << 8) ; pos++;
	len |= (p[ pos ] << 16); pos++;
	len |= (p[ pos ] << 24); pos++;

	//qDebug("font block type %d len %d",typ,len);

	int end = pos + len;
	if (typ==1) {
	    while (pos < end) {
		if (hexcol > 0) {
		    s << "\n";
		    hexcol = 0;
		}
		//qWarning(QString::fromLatin1((char*)(p+pos),1));
		if (p[pos] == '\r' || p[pos] == '\n') {
		    s << "\n";
		    while (pos < end && p[pos] == '\r' || p[pos] == '\n')
			pos++;
		} else {
		    s << QString::fromLatin1((char*)(p+pos),1);
		    pos++;
		}
	    }
	}
	if (typ==2) {
	    static const char *hexchar = "0123456789abcdef";
	    while (pos < end) {
		/* trim hexadecimal lines to line_length columns */
		if (hexcol >= line_length) {
		    s << "\n";
		    hexcol = 0;
		}
		s << QString::fromLatin1(hexchar+((p[pos] >> 4) & 0xf),1)
		  << QString::fromLatin1(hexchar+((p[pos]     ) & 0xf),1);
		pos++;
		hexcol += 2;
	    }
	}
    }
    s << "% End of font resource\n";
    downloadMapping( s, global );
}

// ================== AFontFileNotFound ============



class QPSPrinterFontNotFound
    : public QPSPrinterFontPrivate {
public:
    QPSPrinterFontNotFound(const QFontEngine* f);
    virtual void    download(QTextStream& s, bool global);
private:
    QByteArray     data;
};

QPSPrinterFontNotFound::QPSPrinterFontNotFound(const QFontEngine* f)
{
    psname = makePSFontName( f );
    replacementList = makePSFontNameList( f );
}

void QPSPrinterFontNotFound::download(QTextStream& s, bool)
{
    //qDebug("downloading not found font %s", psname.latin1() );
    emitPSFontNameList( s, psname, replacementList );
    s << "% No embeddable font for ";
    s << psname;
    s << " found\n";
    QPSPrinterFontPrivate::download(s, TRUE);
}

#ifndef QT_NO_TEXTCODEC
// =================== A font file for asian ============

class QPSPrinterFontAsian
    : public QPSPrinterFontPrivate {
public:
    QPSPrinterFontAsian()
	: QPSPrinterFontPrivate(), codec( 0 ) {}
    void download(QTextStream& s, bool global);
    QString defineFont( QTextStream &stream, const QString &ps, const QFont &f, const QString &key,
			QPSPrinterPrivate *d );
    void drawText( QTextStream &stream, const QPoint &p, QTextEngine *engine, int item,
		   const QString &text, QPSPrinterPrivate *d, QPainterState *ps );

    QString makePSFontName( const QFontEngine *f, int type ) const;
    virtual QString extension() const = 0;

    QTextCodec *codec;
};

QString QPSPrinterFontAsian::makePSFontName( const QFontEngine *f, int type ) const
{
    QString ps;
    int i;

    QString family = f->fontDef.family.toLower();

    // try to make a "good" postscript name
    ps = family.simplified();
    i = 0;
    while( i < ps.length() ) {
        if ( i != 0 && ps[i] == '[') {
	    if ( ps[i-1] == ' ' )
		ps.truncate (i-1);
	    else
		ps.truncate (i);
	    break;
        }
	if ( i == 0 || ps[i-1] == ' ' ) {
	    ps[i] = ps[i].toUpper();
	    if ( i )
		ps.remove( i-1, 1 );
	    else
		i++;
	} else {
	    i++;
	}
    }

    switch ( type ) {
    case 1:
	ps.append( QString::fromLatin1("-Italic") );
	break;
    case 2:
	ps.append( QString::fromLatin1("-Bold") );
	break;
    case 3:
	ps.append( QString::fromLatin1("-BoldItalic") );
	break;
    case 0:
    default:
	break;
    }

    ps += extension();

    return ps;
}


QString QPSPrinterFontAsian::defineFont( QTextStream &stream, const QString &ps, const QFont &f,
					 const QString &key, QPSPrinterPrivate *d)
{
    QString fontName;
    QString fontName2;

    QString tmp = d->headerFontNames.value(ps, QString::null);

    if ( d->buffer ) {
        if ( !tmp.isNull() ) {
            fontName = tmp;
        } else {
	    fontName.sprintf( "F%d", ++d->headerFontNumber );
	    d->fontStream << "/" << fontName << " false " << ps << "List MF\n";
	    d->headerFontNames.insert(ps, fontName);
	}
        fontName2.sprintf( "F%d", ++d->headerFontNumber );
        d->fontStream << "/" << fontName2 << " "
                      << pointSize( f, d->scale ) << "/" << fontName << " DF\n";
        d->headerFontNames.insert(key, fontName2);
    } else {
        if ( !tmp.isNull() ) {
            fontName = tmp;
        } else {
	    fontName.sprintf( "F%d", ++d->pageFontNumber );
	    stream << "/" << fontName << " false " << ps << "List MF\n";
	    d->pageFontNames.insert(ps, fontName);
	}
        fontName2.sprintf( "F%d", ++d->pageFontNumber );
        stream << "/" << fontName2 << " "
               << pointSize( f, d->scale ) << "/" << fontName << " DF\n";
        d->pageFontNames.insert(key, fontName2);
    }
    return fontName2;
}


void QPSPrinterFontAsian::download(QTextStream& s, bool)
{
    //qDebug("downloading asian font %s", psname.latin1() );
    s << "% Asian postscript font requested. Using "
      << psname << endl;
    emitPSFontNameList( s, psname, replacementList );
}

void QPSPrinterFontAsian::drawText( QTextStream &stream, const QPoint &p, QTextEngine *engine, int item,
				    const QString &text, QPSPrinterPrivate *d, QPainterState *ps)
{
    int len = engine->length( item );
    QScriptItem &si = engine->items[item];

    int x = p.x();
    int y = p.y();
    if ( y != d->textY || d->textY == 0 )
        stream << y << " Y";
    d->textY = y;

    QString mdf;
    if ( ps->font.underline() )
        mdf += " " + QString().setNum( y + d->fm.underlinePos() + d->fm.lineWidth() ) +
               " " + toString( d->fm.lineWidth() ) + " Tl";
    if ( ps->font.strikeOut() )
        mdf += " " + QString().setNum( y + d->fm.strikeOutPos() ) +
               " " + toString( d->fm.lineWidth() ) + " Tl";
    QByteArray mb;
    QByteArray out;
    QString dummy( QChar(0x20) );

    if ( si.analysis.bidiLevel % 2 ) {
	for ( int i = len-1; i >= 0; i-- ) {
	    QChar ch = text.unicode()[i];
	    if ( !ch.row() ) {
		; // ignore, we should never get here anyway
	    } else {
		if ( codec ) {
		    dummy[0] = ch;
		    mb = codec->fromUnicode( dummy );
		} else
		    mb = "  ";

		for ( int j = 0; j < mb.size (); j++ ) {
		    if ( mb.at(j) == '(' || mb.at(j) == ')' || mb.at(j) == '\\' )
			out += "\\";
		    out += mb.at(j);
		}
	    }
	}
    } else {
	for ( int i = 0; i < len; i++ ) {
	    QChar ch = text.unicode()[i];
	    if ( !ch.row() ) {
		; // ignore, we should never get here anyway
	    } else {
		if ( codec ) {
		    dummy[0] = ch;
		    mb = codec->fromUnicode( dummy );
		} else
		    mb = "  ";

		for ( int j = 0; j < mb.size(); j++ ) {
		    if ( mb.at(j) == '(' || mb.at(j) == ')' || mb.at(j) == '\\' )
			out += "\\";
		    out += mb.at(j);
		}
	    }
	}
    }
    stream << "(" << out << ")" << si.width << " " << x << mdf << " AT\n";
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

class QPSPrinterFontJapanese
  : public QPSPrinterFontAsian {
public:
      QPSPrinterFontJapanese(const QFontEngine* f);
      virtual QString extension() const;
};

QPSPrinterFontJapanese::QPSPrinterFontJapanese(const QFontEngine* f)
{
    codec = QTextCodec::codecForMib( 63 ); // jisx0208.1983-0

    int type = getPsFontType( f );
    psname = makePSFontName( f, type );
    QString best = "[ /" + psname + " 1.0 0.0 ]";
    replacementList.append( best );

    const psfont *const *replacements = ( psname.contains( "Helvetica" ) ? Japanese2Replacements : Japanese1Replacements );
    appendReplacements( replacementList, replacements, type );
}

QString QPSPrinterFontJapanese::extension() const
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
#if 0 // ### this is never used?
static const psfont SMMyungjo [] = {
    { "SMMyungjo-Light-KSC-EUC-H", 0, 100. },
    { "SMMyungjo-Light-KSC-EUC-H", 0.2, 100. },
    { "SMMyungjo-Bold-KSC-EUC-H", 0, 100. },
    { "SMMyungjo-Bold-KSC-EUC-H", 0.2, 100. }
};
#endif

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



static const psfont * const KoreanReplacements[] = {
    SMGothic, Munhwa, MunhwaGothic, MKai, MunhwaGungSeo, MunhwaGungSeoHeulim,
    MunhwaHoonMin, Helvetica, 0
};

class QPSPrinterFontKorean
    : public QPSPrinterFontAsian {
public:
    QPSPrinterFontKorean(const QFontEngine* f);
    QString extension() const;
};

QPSPrinterFontKorean::QPSPrinterFontKorean(const QFontEngine* f)
{
    codec = QTextCodec::codecForMib( 38 ); // eucKR
    int type = getPsFontType( f );
    psname = makePSFontName( f, type );
    QString best = "[ /" + psname + " 1.0 0.0 ]";
    replacementList.append( best );
    appendReplacements( replacementList, KoreanReplacements, type );
}

QString QPSPrinterFontKorean::extension() const
{
    return "-KSC-EUC-H";
}
// ----------- traditional chinese ------------

// Arphic Public License Big5 TrueType fonts (on Debian and CLE and others)
static const psfont ShanHeiSun [] = {
    { "ShanHeiSun-Light-ETen-B5-H", 0, 100. },
    { "ShanHeiSun-Light-Italic-ETen-B5-H", 0.2, 100. },
    { "ShanHeiSun-Light-Bold-ETen-B5-H", 0, 100. },
    { "ShanHeiSun-Light-BoldItalic-ETen-B5-H", 0.2, 100. },
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

// Dr. Wang Hann-Tzong's GPL'ed Big5 TrueType fonts
#if 0 // ### this is never used?
static const psfont HtWFangSong [] = {
    { "HtW-FSong-Light-ETen-B5-H", 0, 100. },
    { "HtW-FSong-Light-Italic-ETen-B5-H", 0.2, 100. },
    { "HtW-FSong-Light-Bold-ETen-B5-H", 0, 100. },
    { "HtW-FSong-Light-BoldItalic-ETen-B5-H", 0.2, 100. },
};
#endif

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

#if 0 // ### these are never used?
static const psfont * const SongB5Replacements[] = {
    SongB5, ShanHeiSun, MingB5, MSung, MOESung, Helvetica, 0
	};

static const psfont * const FangSongB5Replacements[] = {
    FangSongB5, HtWFangSong, Courier, 0
	};
static const psfont * const KaiB5Replacements[] = {
    KaiB5, ZenKai, Times, 0
	};
static const psfont * const HeiB5Replacements[] = {
    HeiB5, LinGothic, YenRound, LucidaSans, 0
	};
static const psfont * const YuanB5Replacements[] = {
    YenRound, LinGothic, HeiB5, LucidaSans, 0
	};
#endif


class QPSPrinterFontTraditionalChinese
  : public QPSPrinterFontAsian {
public:
      QPSPrinterFontTraditionalChinese(const QFontEngine* f);
      QString extension() const;
};

QPSPrinterFontTraditionalChinese::QPSPrinterFontTraditionalChinese(const QFontEngine* f)
{
    codec = QTextCodec::codecForMib( -2026 ); // Big5-0
    int type = getPsFontType( f );
    psname = makePSFontName( f, type );
    QString best = "[ /" + psname + " 1.0 0.0 ]";
    replacementList.append( best );
    appendReplacements( replacementList, TraditionalReplacements, type );
}

QString QPSPrinterFontTraditionalChinese::extension() const
{
    return "-ETen-B5-H";
}

// ----------- simplified chinese ------------

#if 0
// GB18030 fonts on XteamLinux (?)
static const psfont SimplifiedGBK2K [] = {
    { "MSung-Light-GBK2K-H", 0, 100. },
    { "MSung-Light-GBK2K-H", 0.2, 100. },
    { "MKai-Medium-GBK2K-H", 0, 100. },
    { "MKai-Medium-GBK2K-H", 0.2, 100. },
};
#endif

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

class QPSPrinterFontSimplifiedChinese
  : public QPSPrinterFontAsian {
public:
      QPSPrinterFontSimplifiedChinese(const QFontEngine* f);
      QString extension() const;
};

QPSPrinterFontSimplifiedChinese::QPSPrinterFontSimplifiedChinese(const QFontEngine* f)
{
    codec = QTextCodec::codecForMib( 114 ); // GB18030
    int type = getPsFontType( f );
    QString family = f->fontDef.family.toLower();
    if( family.contains("kai",QString::CaseInsensitive) ) {
	psname = KaiGBK2K[type].psname;
	appendReplacements( replacementList, KaiGBK2KReplacements, type );
    } else if( family.contains("fangsong",QString::CaseInsensitive) ) {
	psname = FangSongGBK2K[type].psname;
	appendReplacements( replacementList, FangSongGBK2KReplacements, type );
    } else if( family.contains("hei",QString::CaseInsensitive) ) {
	psname = HeiGBK2K[type].psname;
	appendReplacements( replacementList, HeiGBK2KReplacements, type );
    } else {
	psname = SongGBK2K[type].psname;
	appendReplacements( replacementList, SimplifiedReplacements, type );
    }
    //qDebug("simplified chinese: fontname is %s, psname=%s", f.family().latin1(), psname.latin1() );
}

QString QPSPrinterFontSimplifiedChinese::extension() const
{
    return "-GBK2K-H";
}

#endif


// ================== QPSPrinterFont ====================

class QPSPrinterFont {
public:
    QPSPrinterFont(const QFont& f, int script, QPSPrinterPrivate *priv);
    ~QPSPrinterFont();
    QString postScriptFontName()     { return p->postScriptFontName(); }
    QString defineFont( QTextStream &stream, const QString &ps, const QFont &f, const QString &key,
			QPSPrinterPrivate *d )
	{ return p->defineFont( stream, ps, f, key, d ); }
    void    download(QTextStream& s, bool global) { p->download(s, global); }
    QPSPrinterFontPrivate *handle() { return p; }
    QString xfontname;
private:
    QByteArray       data;
    QPSPrinterFontPrivate* p;
};

QPSPrinterFont::~QPSPrinterFont()
{
    // the dict in QFontPrivate does deletion for us.
    //  delete p;
}


QPSPrinterFont::QPSPrinterFont(const QFont &f, int script, QPSPrinterPrivate *priv)
    : p(0)
{
    QString fontfilename;
    QString fontname;

    enum { NONE, PFB, PFA, TTF } type = NONE;

    QFontEngine *engine = f.d->engineForScript( (QFont::Script) script );
    // ### implement similar code for QWS and WIN
    xfontname = makePSFontName( engine );

#if defined( Q_WS_X11 )
    bool xlfd = FALSE;
    if ( priv->embedFonts ) {
	//qDebug("engine = %p name=%s, script=%d", engine, engine ? engine->name() : "(null)", script);

#ifndef QT_NO_XFTFREETYPE
	if ( X11->has_xft && engine && engine->type() == QFontEngine::Xft ) {
	    XftPattern *pattern = static_cast<QFontEngineXft *>( engine )->pattern();
	    char *filename = 0;
	    XftPatternGetString (pattern, XFT_FILE, 0, &filename);
	    //qDebug("filename for font is '%s'", filename);
	    if ( filename ) {
		fontfilename = QString::fromLocal8Bit( filename );
		xfontname = fontfilename;
	    }
	} else
#endif
	{
	    QString rawName;
	    if ( engine && engine != (QFontEngine *)-1 )
		rawName = engine->name();
	    int index = rawName.indexOf('-');
	    if (index == 0) {
		// this is an XLFD font name
		for (int i=0; i < 6; i++) {
		    index = rawName.indexOf('-',index+1);
		}
		xfontname = rawName.mid(0,index);
		if ( xfontname.endsWith( "*" ) )
		    xfontname.truncate( xfontname.length() - 1 );
		xlfd = TRUE;
	    }
	}
    }
#endif // Q_WS_X11
#ifndef QT_NO_TEXTCODEC
    // map some scripts to something more useful
    if ( script == QFont::Han ) {
	QTextCodec *lc = QTextCodec::codecForLocale();
	switch( lc->mibEnum() ) {
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
    } else if ( script == QFont::Katakana )
	script = QFont::Hiragana;
    else if ( script == QFont::Bopomofo )
	script = QFont::Han;
#endif

    QString searchname = xfontname;
#if defined(Q_WS_X11)
    // we need an extension here due to the fact that we use different
    // fonts for different scripts
    if ( xlfd && script >= QFont::Han && script <= QFont::Bopomofo )
	xfontname += "/" + toString( script );
#endif

    //qDebug("looking for font %s in dict", xfontname.latin1() );
    p = priv->fonts.value(xfontname);
    if ( p )
	return;

#if defined(Q_WS_X11)
    if ( priv->embedFonts && xlfd ) {

	for (QStringList::Iterator it=priv->fontpath.begin(); it!=priv->fontpath.end() && fontfilename.isEmpty(); ++it) {
	    if ((*it).left(1) != "/") continue; // not a path name, a font server
	    QString fontmapname;
	    int num = 0;
	    // search font.dir and font.scale for the right file
	    while ( num < 2 ) {
		if ( num == 0 )
		    fontmapname = (*it) + "/fonts.scale";
		else
		    fontmapname = (*it) + "/fonts.dir";
		//qWarning(fontmapname);
		QFile fontmap(fontmapname);
		if (fontmap.open(IO_ReadOnly)) {
		    while (!fontmap.atEnd()) {
			QString mapping;
			fontmap.readLine(mapping,512);
			// fold to lower (since X folds to lowercase)
			//qWarning(xfontname);
			//qWarning(mapping);
			if (mapping.toLower().contains(searchname.toLower())) {
			    int index = mapping.indexOf(' ');
			    QString ffn = mapping.mid(0,index);
			    // remove the most common bitmap formats
			    if( !ffn.contains( ".pcf" ) && !ffn.contains( ".bdf" ) &&
				!ffn.contains( ".spd" ) && !ffn.contains( ".phont" ) ) {
				fontfilename = (*it) + QString("/") + ffn;
				if ( QFile::exists(fontfilename) ) {
				    //qDebug("found font file %s", fontfilename.latin1());
				    break;
				} else // unset fontfilename
				    fontfilename = QString();
			    }
			}
		    }
		    fontmap.close();
		}
		num++;
	    }
	}
    }
#endif

    //qDebug("font=%s, fontname=%s, file=%s, p=%p", f.family().latin1(), xfontname.latin1(), fontfilename.latin1(), p);

    // memory mapping would be better here
    if (fontfilename.length() > 0) { // maybe there is no file name
	QFile fontfile(fontfilename);
	if ( fontfile.exists() ) {
	    //printf("font name %s size = %d\n",fontfilename.latin1(),fontfile.size());
	    data = QByteArray();
	    data.resize(fontfile.size());

	    fontfile.open(IO_Raw | IO_ReadOnly);
	    fontfile.readBlock(data.data(), fontfile.size());
	    fontfile.close();
	}
    }

    if (!data.isNull() && data.size() > 0) {
        const BYTE* d = (const BYTE *)data.constData();
        if (d[0] == 0x80 && d[6] == '%' && d[7] == '!' && d[8] == 'P' && d[9] == 'S' )
            type = PFB;
        else if (d[0] == '%' && d[1] == '!' && d[2] == 'P' && d[3] == 'S')
            type = PFA;
        else if (d[0]==0x00 && d[1]==0x01 && d[2]==0x00 && d[3]==0x00)
            type = TTF;
        else
            type = NONE;
    } else
        type = NONE;

    //qDebug("font is of type %d", type );
    switch (type) {
    case TTF :
	p = new QPSPrinterFontTTF(engine, data);
	break;
    case PFB:
	p = new QPSPrinterFontPFB(engine, data);
	break;
    case PFA:
	p = new QPSPrinterFontPFA(engine, data);
	break;
    case NONE:
    default:

#ifndef QT_NO_TEXTCODEC

	if ( script == QFont::Hiragana )
	    p = new QPSPrinterFontJapanese( engine );
	else if ( script == QFont::Hangul )
	    p = new QPSPrinterFontKorean( engine );
	else if ( script == QFont::Han ) {
	    QTextCodec *lc = QTextCodec::codecForLocale();
	    switch( lc->mibEnum() ) {
	    case 2025: // GB2312
	    case 57: // gb2312.1980-0
	    case 113: // GBK
	    case -113: // gbk-0
	    case 114: // GB18030
	    case -114: // gb18030-0
		p = new QPSPrinterFontSimplifiedChinese( engine );
		break;
	    case 2026: // Big5
	    case -2026: // big5-0, big5.eten-0
	    case 2101: // Big5-HKSCS
	    case -2101: // big5hkscs-0, hkscs-1
		p = new QPSPrinterFontTraditionalChinese( engine );
		break;
	    default:
		p = new QPSPrinterFontJapanese( engine );
	    }
	} else
#endif
	    //qDebug("didnt find font for %s", xfontname.latin1());
	    p = new QPSPrinterFontNotFound( engine );
	break;
    }

    if (p->postScriptFontName() == "Symbol")
	p->setSymbol();

    // this is needed to make sure we don't get the same postscriptname twice
    for (QHash<QString, QPSPrinterFontPrivate *>::ConstIterator it = priv->fonts.constBegin();
	 it != priv->fonts.constEnd(); ++it) {
	if ( *(*it) == *p ) {
	    qWarning("Post script driver: font already in dict");
	    delete p;
	    p = *it;
	    return;
	}
    }

    //qDebug("inserting font %s in dict psname=%s", xfontname.latin1(), p->postScriptFontName().latin1() );
    priv->fonts.insert( xfontname, p );
}

// ================= END OF PS FONT METHODS ============


QPSPrinterPrivate::QPSPrinterPrivate( QPrinter *prt, int filedes )
    : buffer( 0 ), outDevice( 0 ), fd( filedes ), pageBuffer( 0 ), fontBuffer(0), savedImage( 0 ),
      dirtypen( FALSE ), dirtybrush( FALSE ), dirtyBkColor( FALSE ), bkMode( Qt::TransparentMode ), dirtyBkMode( FALSE ),
#ifndef QT_NO_TEXTCODEC
      currentFontCodec( 0 ),
#endif
	fm( QFont() ), textY( 0 )
{
    printer = prt;
    currentFontFile = 0;
    scale = 1.;
    scriptUsed = -1;

#ifdef Q_WS_X11
    // append qsettings fontpath
    QSettings settings;
    embedFonts = settings.readBoolEntry( "/qt/embedFonts", TRUE );

    if ( embedFonts ) {
	int npaths;
	char** font_path;
	font_path = XGetFontPath( qt_xdisplay(), &npaths);
	bool xfsconfig_read = FALSE;
	for (int i=0; i<npaths; i++) {
	    // If we're using xfs, append font paths from /etc/X11/fs/config
	    // can't hurt, and chances are we'll get all fonts that way.
	    if (((font_path[i])[0] != '/') && !xfsconfig_read) {
		// We're using xfs -> read its config
		bool finished = FALSE;
		QFile f("/etc/X11/fs/config");
		if ( !f.exists() )
		    f.setName("/usr/X11R6/lib/X11/fs/config");
		if ( !f.exists() )
		    f.setName("/usr/X11/lib/X11/fs/config");
		if ( f.exists() ) {
		    f.open(IO_ReadOnly);
		    while(f.status()==IO_Ok && !finished) {
			QString fs;
			f.readLine(fs, 1024);
			fs=fs.trimmed();
			if (fs.left(9)=="catalogue" && fs.contains('=')) {
			    fs = fs.mid(fs.indexOf('=') + 1).trimmed();
			    bool end = FALSE;
			    while( f.status()==IO_Ok && !end ) {
				if ( fs[int(fs.length())-1] == ',' )
				    fs = fs.left(fs.length()-1);
				else
				    end = TRUE;
				if (fs[0] != '#' && !fs.contains(":unscaled"))
				    fontpath += fs;
				f.readLine(fs, 1024);
				fs=fs.trimmed();
			    }
			    finished = TRUE;
			}
		    }
		    f.close();
		}
		xfsconfig_read = TRUE;
	    } else if(!strstr(font_path[i], ":unscaled")) {
		// Fonts paths marked :unscaled are always bitmapped fonts
		// -> we can as well ignore them now and save time
		fontpath += font_path[i];
	    }
	}
	XFreeFontPath(font_path);

	// append qsettings fontpath
	QStringList fp = settings.readListEntry( "/qt/fontPath", ':' );
	if ( !fp.isEmpty() )
	    fontpath += fp;
    }
#else
    embedFonts = FALSE;
#endif
}

QPSPrinterPrivate::~QPSPrinterPrivate()
{
    delete pageBuffer;
    QHash<QString, QPSPrinterFontPrivate *>::ConstIterator it = fonts.constBegin();
    while (it != fonts.constEnd()) {
	delete it.value();
	++it;
    }
}

void QPSPrinterPrivate::setFont( const QFont & fnt, int script )
{
    QFont f = fnt;
    if ( f.rawMode() ) {
        QFont fnt( QString::fromLatin1("Helvetica"), 12 );
        setFont( fnt, QFont::Unicode );
        return;
    }
    if ( f.pointSize() == 0 ) {
#if defined(CHECK_RANGE)
        qWarning( "QPrinter: Cannot set a font with zero point size" );
#endif
        f.setPointSize(QApplication::font().pointSize());
        if ( f.pointSize() == 0 )
            f.setPointSize( 11 );
    }

    QPSPrinterFont ff( f, script, this );
    QString ps = ff.postScriptFontName();

    QString s = ps;
    s.append( ' ' );
    s.prepend( ' ' );

    QString key = ff.xfontname;

    if ( f.pointSize() != -1 )
	key += " " + toString( f.pointSize() );
    else
	key += " px" + toString( f.pixelSize() );
    QString tmp;
    if ( !buffer )
        tmp = pageFontNames.value(key, QString::null);
    else
        tmp = headerFontNames.value(key, QString::null);

    QString fontName;
    if ( !tmp.isNull() )
        fontName = tmp;

    if ( fontName.isEmpty() ) {
        fontName = ff.defineFont( pageStream, ps, f, key, this );
    }
    pageStream << fontName << " F\n";

    ps.append( ' ' );
    ps.prepend( ' ' );
    if ( !fontsUsed.contains( ps ) )
        fontsUsed += ps;

#ifndef QT_NO_TEXTCODEC
    QTextCodec * codec = 0;
// ###
// #ifndef QT_NO_TEXTCODEC
//     i = 0;
//     do {
//      if ( unicodevalues[i].cs == f.charSet() )
//          codec = QTextCodec::codecForMib( unicodevalues[i++].mib );
//     } while( codec == 0 && unicodevalues[i++].cs != unicodevalues_LAST );
// #endif
    currentFontCodec = codec;
#endif
    currentFont = fontName;
    currentFontFile = ff.handle();
    scriptUsed = script;
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
static const int maxQuoteLength = 4+16+32+64+128+256; // magic extended quote
static const int quoteReach = 10; // ... 1-1024 pixels back
static const int tableSize = 1024; // 2 ** quoteReach;
static const int numAttempts = 128;

static const int hashSize = 71;

static const int None = INT_MAX;

/* puts the lowest numBits of data into the out array starting at postion (byte/bit).
   Adjusts byte and bit to point ot the next position.

   Need to make sure the out array is long enough before calling the method.
*/
static void emitBits( char *out, int & byte, int & bit,
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
        }
    }
}

//#define DEBUG_COMPRESS
#ifdef DEBUG_COMPRESS
#include <qdatetime.h>
#endif

QByteArray compress( const QImage & image, bool gray ) {
#ifdef DEBUG_COMPRESS
    QTime t;
    t.start();
    int sizeUncompressed[11];
    for( int i = 0; i < 11; i++ )
	sizeUncompressed[i] = 0;
    int sizeCompressed[11];
    for( int i = 0; i < 11; i++ )
	sizeCompressed[i] = 0;
#endif

    int width = image.width();
    int height = image.height();
    int depth = image.depth();
    int size = width*height;

    int pastPixel[tableSize];
    int mostRecentPixel[hashSize];
    if ( depth == 1 )
	size = (width+7)/8*height;
    else if ( !gray )
	size = size*3;

    unsigned char *pixel = new unsigned char[size+1];
    int i = 0;
    if ( depth == 1 ) {
	QImage::Endian bitOrder = image.bitOrder();
	memset( pixel, 0xff, size );
        for( int y=0; y < height; y++ ) {
            uchar * s = image.scanLine( y );
	    for( int x=0; x < width; x++ ) {
		// need to copy bit for bit...
		bool b = ( bitOrder == QImage::LittleEndian ) ?
			 (*(s + (x >> 3)) >> (x & 7)) & 1 :
			  (*(s + (x >> 3)) << (x & 7)) & 0x80 ;
		if ( b )
		    pixel[i >> 3] ^= (0x80 >> ( i & 7 ));
		i++;
	    }
	    // we need to align to 8 bit here
	    i = (i+7) & 0xffffff8;
        }
    } else if ( depth == 8 ) {
        for( int y=0; y < height; y++ ) {
            uchar * s = image.scanLine( y );
            for( int x=0; x < width; x++ ) {
                QRgb rgb = image.color( s[x] );
		if ( gray ) {
		    pixel[i] = (unsigned char) qGray( rgb );
		    i++;
		} else {
		    pixel[i] = (unsigned char) qRed( rgb );
		    pixel[i+1] = (unsigned char) qGreen( rgb );
		    pixel[i+2] = (unsigned char) qBlue( rgb );
		    i += 3;
		}
            }
        }
    } else {
        for( int y=0; y < height; y++ ) {
            QRgb * s = (QRgb*)(image.scanLine( y ));
            for( int x=0; x < width; x++ ) {
		QRgb rgb = (*s++);
		if ( qAlpha( rgb ) < 0x40 ) // 25% alpha, convert to white -
		    rgb = qRgb( 0xff, 0xff, 0xff );
		if ( gray ) {
		    pixel[i] = (unsigned char) qGray( rgb );
		    i++;
		} else {
		    pixel[i] = (unsigned char) qRed( rgb );
		    pixel[i+1] = (unsigned char) qGreen( rgb );
		    pixel[i+2] = (unsigned char) qBlue( rgb );
		    i += 3;
		}
	    }
	}
    }

    pixel[size] = 0;

    /* this compression function emits blocks of data, where each
       block is an unquoted series of pixels, or a quote from earlier
       pixels. if the six-letter string "banana" were a six-pixel
       image, it might be unquoted "ban" followed by a 3-pixel quote
       from -2.  note that the final "a" is then copied from the
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
       fast than get the ultimate in compression.

       The format of the compressed stream is as follows:
       // 2 bits step size for search and backreference ( 1 or 3 )
       1 bit compressed or uncompressed block follows

       uncompressed block:
       3 bits size of block in bytes
       size*8 bits data

       compressed block:
       3 bits compression header
           0-2 size of block is 1-3 bytes
           3-7 size of block is bigger, 4-8 additional bits specifying size follow
       0/4-8 additional size fields
       10 location of backreference
    */

    for( i=0; i < hashSize; i++ )
        mostRecentPixel[i] = None;
    int index = 0;
    int emittedUntil = 0;
    char *out = (char *)malloc( 256 * sizeof( char ) );
    int outLen = 256;
    int outOffset = 0;
    int outBit = 0;

    /* we process pixels serially, emitting as necessary/possible. */
    while( index <= size ) {
        int bestCandidate = None;
        int bestLength = 0;
        i = index % tableSize;
        int h = pixel[index] % hashSize;
        int start, end;
        start = end = pastPixel[i] = mostRecentPixel[h];
        mostRecentPixel[h] = index;
        /* if our first candidate quote is unusable, or we don't need
           to quote because we've already emitted something for this
           pixel, just skip. */
        if ( start < index - tableSize || index >= size ||
             emittedUntil > index)
            start = end = None;
        int attempts = 0;
        /* scan for suitable quote candidates: not too far back, and
           if we've found one that's as big as it can get, don't look
           for more */
        while( start != None && end != None &&
               bestLength < maxQuoteLength &&
               start >= index - tableSize &&
               end >= index - tableSize + bestLength ) {
            /* scan backwards, looking for something good enough to
               try a (slow) string comparison. we maintain indexes to
               the start and the end of the quote candidate here */
            while( start != None && end != None &&
                   ( pixel[start] != pixel[index] ||
                     pixel[end] != pixel[index+bestLength] ) ) {
                if ( attempts++ > numAttempts ) {
                    start = None;
                } else if ( pixel[end] % hashSize ==
                            pixel[index+bestLength] % hashSize ) {
                    /* we move the area along the end index' chain */
                    end = pastPixel[end%tableSize];
                    start = end - bestLength;
                } else if ( pixel[start] % hashSize ==
                            pixel[index] % hashSize ) {
                    /* ... or along the start index' chain */
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
                    /* and if it ins't, we'll try some more. but we'll
                       count each string compare extra, since they're
                       so expensive. */
                    attempts += 2;
                    if ( attempts > numAttempts ) {
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
                   candidate, we don't try any more */
                if ( start < index - tableSize || start < 0 || start > size+1 )
                    start = None;
                if ( end < index - tableSize + bestLength || end < 0 || end > size+1 )
                    end = None;
            }
        }
	/* backreferences to 1 byte of data are actually more costly than
         emitting the data directly, 2 bytes don't save much. */
	if ( bestCandidate != None && bestLength < 3 )
	    bestCandidate = None;
        /* at this point, bestCandidate is a candidate of bestLength
         length, or else it's None. if we have such a candidate, or
         we're at the end, we have to emit all unquoted data. */
        if ( index == size || bestCandidate != None ) {
            /* we need a double loop, because there's a maximum length
               on the "unquoted data" section. */
            while( emittedUntil < index ) {
#ifdef DEBUG_COMPRESS
		int x = 0;
		int bl = emittedUntil - index;
		while ( (bl /= 2) )
		    x++;
		if ( x > 10 ) x = 10;
		sizeUncompressed[x]++;
#endif
                int l = qMin( 8, index - emittedUntil );
		if ( outOffset + l + 2 >= outLen ) {
		    outLen *= 2;
		    out = (char *) realloc( out, outLen );
		}
                emitBits( out, outOffset, outBit,
                          1, 0 );
                emitBits( out, outOffset, outBit,
                          quoteSize, l-1 );
                while( l-- ) {
		    emitBits( out, outOffset, outBit,
			      8, pixel[emittedUntil] );
                    emittedUntil++;
                }
            }
        }
        /* if we have some quoted data to output, do it. */
        if ( bestCandidate != None ) {
#ifdef DEBUG_COMPRESS
	    int x = 0;
	    int bl = bestLength;
	    while ( (bl /= 2) )
		x++;
	    if ( x > 10 ) x = 10;
	    sizeCompressed[x]++;
#endif
	    if ( outOffset + 4 >= outLen ) {
		outLen *= 2;
		out = (char *) realloc( out, outLen );
	    }
            emitBits( out, outOffset, outBit,
                      1, 1 );
	    int l = bestLength - 3;
	    const struct off_len {
		int off;
		int bits;
	    } ol_table [] = {
		/* Warning: if you change the table here, change /uc in the PS code! */
		{ 3, 0/*dummy*/ },
		{ 16, 4 },
		{ 32, 5 },
		{ 64, 6 },
		{ 128, 7 },
		{ /*256*/ 0xfffffff, 8 },
	    };

            if ( l < ol_table[0].off ) {
                emitBits( out, outOffset, outBit,
                          quoteSize, l );
	    } else {
		const off_len *ol = ol_table;
		l -= ol->off;
		ol++;
		while ( l >= ol->off ) {
		    l -= ol->off;
		    ol++;
		}
                emitBits( out, outOffset, outBit,
                          quoteSize, ol->bits-1 );
                emitBits( out, outOffset, outBit,
                          ol->bits, l );
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
    i = 0;
    /* we have to make sure the data is encoded in a stylish way :) */
    while( i < outOffset ) {
        uchar c = out[i];
        c += 42;
        if ( c > 'Z' && ( c != 't' || i == 0 || out[i-1] != 'Q' ) )
            c += 84;
        out[i] = c;
        i++;
    }
    QByteArray outarr(out, outOffset);
    free( out );
    delete [] pixel;

#ifdef DEBUG_COMPRESS
    qDebug( "------------- image compression statistics ----------------" );
    qDebug(" compression time %d", t.elapsed() );
    qDebug( "Size dist of uncompressed blocks:" );
    qDebug( "\t%d\t%d\t%d\t%d\t%d\t%d\n", sizeUncompressed[0], sizeUncompressed[1],
	    sizeUncompressed[2], sizeUncompressed[3], sizeUncompressed[4], sizeUncompressed[5]);
    qDebug( "\t%d\t%d\t%d\t%d\t%d\n", sizeUncompressed[6], sizeUncompressed[7],
	    sizeUncompressed[8], sizeUncompressed[9], sizeUncompressed[10] );
    qDebug( "Size dist of compressed blocks:" );
    qDebug( "\t%d\t%d\t%d\t%d\t%d\t%d\n", sizeCompressed[0], sizeCompressed[1],
	    sizeCompressed[2], sizeCompressed[3], sizeCompressed[4], sizeCompressed[5]);
    qDebug( "\t%d\t%d\t%d\t%d\t%d\n", sizeCompressed[6], sizeCompressed[7],
	    sizeCompressed[8], sizeCompressed[9], sizeCompressed[10] );
    qDebug( "===> total compression ratio %d/%d = %f", outOffset, size, (float)outOffset/(float)size );
    qDebug( "-----------------------------------------------------------" );
#endif

    return outarr;
}

#undef XCOORD
#undef YCOORD
#undef WIDTH
#undef HEIGHT
#undef POINT
#undef RECT
#undef INT_ARG

#define XCOORD(x)       (float)(x)
#define YCOORD(y)       (float)(y)
#define WIDTH(w)        (float)(w)
#define HEIGHT(h)       (float)(h)

#define POINT(p) XCOORD(p.x()+0.5) << ' ' << YCOORD(p.y()+0.5) << ' '
#define RECT(r) XCOORD(r.x())  << ' ' << YCOORD(r.y())  << ' ' <<     \
                        WIDTH (r.width()) << ' ' << HEIGHT(r.height()) << ' '
#define INT_ARG(x)  x << ' '

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



void QPSPrinterPrivate::drawImage(float x, float y, float w, float h,
				  const QImage &img, const QImage &mask )
{
    if ( !w || !h || img.isNull() ) return;

    int width  = img.width();
    int height = img.height();
    float scaleX = (float)width/w;
    float scaleY = (float)height/h;

    bool gray = (printer->colorMode() == QPrinter::GrayScale) ||
		img.allGray();
    int splitSize = 21830 * (gray ? 3 : 1 );
    if ( width * height > splitSize ) { // 65535/3, tolerance for broken printers
        int images, subheight;
        images = ( width * height + splitSize - 1 ) / splitSize;
        subheight = ( height + images-1 ) / images;
        while ( subheight * width > splitSize ) {
            images++;
            subheight = ( height + images-1 ) / images;
        }
        int suby = 0;
        while( suby < height ) {
            drawImage(x, y + suby/scaleY, w, qMin( subheight, height-suby )/scaleY,
		      img.copy( 0, suby, width, qMin( subheight, height-suby ) ),
		      mask.isNull() ? mask : mask.copy( 0, suby, width, qMin( subheight, height-suby ) ));
            suby += subheight;
        }
    } else {
	QByteArray out;
	int size = 0;
	const char *bits;

	if ( !mask.isNull() ) {
	    out = ::compress( mask, TRUE );
	    size = (width+7)/8*height;
	    pageStream << "/mask " << size << " string uc\n";
	    ps_r7( pageStream, out, out.size() );
	    pageStream << "d\n";
	}
	if ( img.depth() == 1 ) {
	    size = (width+7)/8*height;
	    bits = "1 ";
	} else if ( gray ) {
	    size = width*height;
	    bits = "8 ";
        } else {
	    size = width*height*3;
	    bits = "24 ";
        }

	out = ::compress( img, gray );
	pageStream << "/sl " << size << " string uc\n";
	ps_r7( pageStream, out, out.size() );
	pageStream << "d\n"
		   << width << ' ' << height << "[" << scaleX << " 0 0 " << scaleY << " 0 0]sl "
		   << bits << (!mask.isNull() ? "mask " : "false ")
		   << x << ' ' << y << " di\n";
    }
}


void QPSPrinterPrivate::matrixSetup(QPainterState *ps)
{
#ifndef QT_NO_TRANSFORMATIONS
    QWMatrix tmp = ps->matrix;
    pageStream << "["
           << tmp.m11() << ' ' << tmp.m12() << ' '
           << tmp.m21() << ' ' << tmp.m22() << ' '
           << tmp.dx()  << ' ' << tmp.dy()
           << "]ST\n";
#else
    QPoint p(0,0);
    p = paint->xForm(p);
    pageStream << "["
           << 0 << ' ' << 0 << ' '
           << 0 << ' ' << 0 << ' '
           << p.x()    << ' ' << p.y()
           << "]ST\n";
#endif
    dirtyMatrix = FALSE;
}

void QPSPrinterPrivate::orientationSetup()
{
    if ( printer->orientation() == QPrinter::Landscape )
        pageStream << "QLS\n";
}


void QPSPrinterPrivate::emitHeader( bool finished )
{
    QString title = printer->docName();
    QString creator = printer->creator();
    if ( !creator )                             // default creator
        creator = QString::fromLatin1("Qt " QT_VERSION_STR);
    outDevice = new QFile();
    (void)((QFile *)outDevice)->open( IO_WriteOnly, fd );
    outStream.setDevice( outDevice );
    outStream << "%!PS-Adobe-1.0";
    QPaintDeviceMetrics m( printer );
    scale = 72. / ((float) m.logicalDpiY());
    uint mtop, mleft, mbottom, mright;
    printer->margins( &mtop, &mleft, &mbottom, &mright );
    int width = m.width();
    int height = m.height();
    bool fullPage = printer->fullPage();
    if ( finished && pageCount == 1 && printer->numCopies() == 1 &&
         ( ( printer->fullPage() && qt_gen_epsf ) ||
	   ( printer->outputToFile() && printer->outputFileName().endsWith( ".eps" ) ) )
	) {
        if ( !boundingBox.isValid() )
            boundingBox.setRect( 0, 0, width, height );
	if ( printer->orientation() == QPrinter::Landscape ) {
	    if ( !fullPage )
		boundingBox.moveBy( -mleft, -mtop );
	    outStream << " EPSF-3.0\n%%BoundingBox: "
		      << (int)(m.height() - boundingBox.bottom())*scale << " " // llx
		      << (int)(m.width() - boundingBox.right())*scale - 1 << " " // lly
		      << (int)(m.height() - boundingBox.top())*scale + 1 << " " // urx
		      << (int)(m.width() - boundingBox.left())*scale; // ury
	} else {
	    if ( !fullPage )
		boundingBox.moveBy( mleft, -mtop );
	    outStream << " EPSF-3.0\n%%BoundingBox: "
		      << (int)(boundingBox.left())*scale << " "
		      << (int)(m.height() - boundingBox.bottom())*scale - 1 << " "
		      << (int)(boundingBox.right())*scale + 1 << " "
		      << (int)(m.height() - boundingBox.top())*scale;
	}
    } else {
	int w = width + (fullPage ? 0 : mleft + mright);
        int h = height + (fullPage ? 0 : mtop + mbottom);
	w = (int)(w*scale);
	h = (int)(h*scale);
        // set a bounding box according to the DSC
        if ( printer->orientation() == QPrinter::Landscape )
            outStream << "\n%%BoundingBox: 0 0 " << h << " " << w;
        else
            outStream << "\n%%BoundingBox: 0 0 " << w << " " << h;
    }
    outStream << "\n" << wrapDSC( "%%Creator: " + creator );
    if ( !!title )
        outStream << wrapDSC( "%%Title: " + title );
    outStream << "%%CreationDate: " << QDateTime::currentDateTime().toString();
    outStream << "\n%%Orientation: ";
    if ( printer->orientation() == QPrinter::Landscape )
        outStream << "Landscape";
    else
        outStream << "Portrait";
    if ( finished )
        outStream << "\n%%Pages: " << pageCount << "\n"
		  << wrapDSC( "%%DocumentFonts: " + fontsUsed );
    else
        outStream << "%%Pages: (atend)"
               << "\n%%DocumentFonts: (atend)";
    outStream << "\n%%EndComments\n";

    outStream << "%%BeginProlog\n";
    const char * const prologLicense = "% Prolog copyright 1994-2003 Trolltech. "
                                 "You may copy this prolog in any way\n"
                                 "% that is directly related to this "
                                 "document. For other use of this prolog,\n"
                                 "% see your licensing agreement for Qt.\n";
    outStream << prologLicense << ps_header << "\n";

    // we have to do this here, as scaling can affect this.
    QString lineStyles = "/LArr["                                       // Pen styles:
			 " [] []"                       //   solid line
			 " [ w s ] [ s w ]"                 //   dash line
			 " [ s s ] [ s s ]"                  //   dot line
			 " [ m s s s ] [ s m s s ]"      //   dash dot line
			 " [ m s s s s ] [ s m s s s s ]"         //   dash dot dot line
			 " ] d\n";
    lineStyles.replace( QRegExp( "w" ), toString( 10./scale ) );
    lineStyles.replace( QRegExp( "m" ), toString( 5./scale ) );
    lineStyles.replace( QRegExp( "s" ), toString( 3./scale ) );

    outStream << lineStyles;

    outStream << "/pageinit {\n";
    if ( !printer->fullPage() ) {
        if ( printer->orientation() == QPrinter::Portrait )
            outStream << mleft*scale << " "
                   << mbottom*scale << " translate\n";
        else
            outStream << mtop*scale << " "
                   << mleft*scale << " translate\n";
    }
    if ( printer->orientation() == QPrinter::Portrait ) {
        outStream << "% " << m.widthMM() << "*" << m.heightMM()
               << "mm (portrait)\n0 " << height*scale
               << " translate " << scale << " -" << scale << " scale/defM matrix CM d } d\n";
    } else {
        outStream << "% " << m.heightMM() << "*" << m.widthMM()
               << " mm (landscape)\n 90 rotate " << scale << " -" << scale << " scale/defM matrix CM d } d\n";
    }
    outStream << "%%EndProlog\n";


    outStream << "%%BeginSetup\n";
    if ( printer->numCopies() > 1 ) {
	outStream << "/#copies " << printer->numCopies() << " def\n";
	outStream << "/NumCopies " << printer->numCopies() << " SPD\n";
	outStream << "/Collate " << (printer->collateCopies() ? "true" : "false") << " SPD\n";
    }
    if ( fontBuffer->buffer().size() ) {
        if ( pageCount == 1 || finished )
            outStream << "% Fonts and encodings used\n";
        else
            outStream << "% Fonts and encodings used on pages 1-"
                   << pageCount << "\n";
	for (QHash<QString, QPSPrinterFontPrivate *>::Iterator it = fonts.begin();
	     it != fonts.end(); ++it)
	    (*it)->download(outStream,TRUE); // true means its global
        outStream.writeRawBytes( fontBuffer->buffer(),
                              fontBuffer->buffer().size() );
    }
    outStream << "%%EndSetup\n";

    outStream.writeRawBytes( buffer->buffer(),
                          buffer->buffer().size() );

    delete buffer;
    buffer = 0;
    fontStream.unsetDevice();
    delete fontBuffer;
    fontBuffer = 0;
}


/* Called whenever a restore has been done. Currently done at the top of a
  new page and whenever clipping is turned off. */
void QPSPrinterPrivate::resetDrawingTools(QPainterState *ps )
{
    QPen   defaultPen;                  // default drawing tools
    QBrush defaultBrush;

    QColor c = ps->bgBrush;
    if ( c != Qt::white )
        pageStream << color( c, printer ) << "BC\n";

    if ( ps->bgMode != Qt::TransparentMode )
	pageStream << "/OMo true d\n";

    //currentUsed = currentSet;
    //setFont( currentSet );
    currentFontFile = 0;

    if ( ps->brush != defaultBrush ) {
        if ( ps->brush == Qt::CustomPattern ) {
#if defined(CHECK_RANGE)
            qWarning( "QPrinter: Pixmap brush not supported" );
#endif
        } else {
            cbrush = ps->brush;
        }
    }

    dirtypen = TRUE;
    dirtybrush = TRUE;

    if (ps->VxF || ps->WxF)
        matrixSetup(ps);
}


static void putRect( QTextStream &stream, const QRect &r )
{
    stream << r.x() << " "
           << r.y() << " "
           << r.width() << " "
           << r.height() << " ";
}


void QPSPrinterPrivate::setClippingOff( QPainterState *ps )
{
    pageStream << "CLO\n";              // clipping off, includes a restore
    resetDrawingTools(ps);     // so drawing tools must be reset
}


void QPSPrinterPrivate::clippingSetup(QPainterState *ps)
{
    if ( ps->clipEnabled ) {
        if ( !firstClipOnPage )
            setClippingOff(ps);
        const QRegion rgn = ps->clipRegion;
        QVector<QRect> rects = rgn.rects();
        int i;
        pageStream<< "CLSTART\n";           // start clipping
        for( i = 0 ; i < rects.size() ; i++ ) {
            putRect( pageStream, rects[i] );
            pageStream << "ACR\n";          // add clip rect
            if ( pageCount == 1 )
                boundingBox = boundingBox.unite( rects[i] );
        }
        pageStream << "CLEND\n";            // end clipping
        firstClipOnPage = FALSE;
    } else {
        if ( !firstClipOnPage )      // no need to turn off if first on page
            setClippingOff( ps );
        // if we're painting without clipping, the bounding box must
        // be everything.  NOTE: this assumes that this function is
        // only ever called when something is to be painted.
        QPaintDeviceMetrics m( printer );
        if ( !boundingBox.isValid() )
            boundingBox.setRect( 0, 0, m.width(), m.height() );
    }
    dirtyClipping = FALSE;
}

void QPSPrinterPrivate::initPage(QPainterState *ps)
{

    // a restore undefines all the fonts that have been defined
    // inside the scope (normally within pages) and all the glyphs that
    // have been added in the scope.

    for (QHash<QString, QPSPrinterFontPrivate *>::Iterator it = fonts.begin();
	 it != fonts.end(); ++it)
	(*it)->restore();

    if ( !buffer ) {
        pageFontNames.clear();
    }

    pageStream.unsetDevice();
    if ( pageBuffer )
        delete pageBuffer;
    pageBuffer = new QBuffer();
    pageBuffer->open( IO_WriteOnly );
    pageStream.setEncoding( QTextStream::Latin1 );
    pageStream.setDevice( pageBuffer );
    delete savedImage;
    savedImage = 0;
    textY = 0;
    dirtyClipping   = TRUE;
    firstClipOnPage = TRUE;


    resetDrawingTools(ps);
    dirtyNewPage      = FALSE;
    pageFontNumber = headerFontNumber;
}

void QPSPrinterPrivate::flushPage( bool last )
{
    if ( last && !pageBuffer )
	return;
    bool pageFonts = ( buffer == 0 );
    if ( buffer &&
//         ( last || pagesInBuffer++ > -1 ||
//           ( pagesInBuffer > 4 && buffer->size() > 262144 ) ) )
         (last || buffer->size() > 2000000) )
    {
//        qDebug("emiting header at page %d", pageCount );
        emitHeader( last );
    }
    outStream << "%%Page: "
              << pageCount << ' ' << pageCount << endl
              << "%%BeginPageSetup\n"
              << "QI\n";
    if ( pageFonts ) {
        //qDebug("page fonts for page %d", pageCount);
        // we have already downloaded the header. Maybe we have page fonts here
	for (QHash<QString, QPSPrinterFontPrivate *>::Iterator it = fonts.begin();
	     it != fonts.end(); ++it)
            (*it)->download( outStream, FALSE ); // FALSE means its for the page only
    }
    outStream  << "%%EndPageSetup\n";
    if ( pageBuffer )
	outStream.writeRawBytes( pageBuffer->buffer(),
				 pageBuffer->buffer().size() );
    outStream << "\nQP\n";
    pageCount++;
}

// ================ PSPrinter class ========================

QPSPrinter::QPSPrinter( QPrinter *prt, int fd )
    : QPaintEngine(CoordTransform|PenWidthTransform|PatternTransform|PixmapTransform)
{
    d = new QPSPrinterPrivate( prt, fd );
}


QPSPrinter::~QPSPrinter()
{
    if ( d->fd >= 0 )
#if defined(_OS_WIN32_)
        ::_close( d->fd );
#else
        ::close( d->fd );
#endif
    delete d;
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
	    qWarning("QPSPrinter: could not restore SIGPIPE handler");

	delete users_sigpipe_handler;
	users_sigpipe_handler = 0;
    }
}

bool QPSPrinter::begin(const QPaintDevice *pdev, QPainterState *state, bool unclipped)
{
    d->pagesInBuffer = 0;
    d->buffer = new QBuffer();
    d->buffer->open( IO_WriteOnly );
    d->outStream.setEncoding( QTextStream::Latin1 );
    d->outStream.setDevice( d->buffer );
    d->fontBuffer = new QBuffer();
    d->fontBuffer->open( IO_WriteOnly );
    d->fontStream.setEncoding( QTextStream::Latin1 );
    d->fontStream.setDevice( d->fontBuffer );
    d->headerFontNumber = 0;
    d->pageCount           = 1;                // initialize state
    d->dirtyMatrix         = TRUE;
    d->dirtyClipping    = TRUE;
    d->dirtyNewPage        = TRUE;
    d->firstClipOnPage  = TRUE;
    d->boundingBox = QRect( 0, 0, -1, -1 );
    d->fontsUsed = QString::fromLatin1("");

    QPaintDeviceMetrics m( d->printer );
    d->scale = 72. / ((float) m.logicalDpiY());

    return TRUE;
}

bool QPSPrinter::end()
{
    bool pageCountAtEnd = (d->buffer != 0);

    // we're writing to lp/lpr through a pipe, we don't want to crash with SIGPIPE
    // if lp/lpr dies
    ignoreSigPipe(TRUE);
    d->flushPage( TRUE );
    d->outStream << "%%Trailer\n";
    if ( pageCountAtEnd )
	d->outStream << "%%Pages: " << d->pageCount - 1 << "\n" <<
	    wrapDSC( "%%DocumentFonts: " + d->fontsUsed );
    d->outStream << "%%EOF\n";
    ignoreSigPipe(FALSE);

    d->outStream.unsetDevice();
    if ( d->outDevice )
	d->outDevice->close();
    if ( d->fd >= 0 )
	::close( d->fd );
    d->fd = -1;
    delete d->outDevice;
    d->outDevice = 0;

    return true;
}

bool QPSPrinter::updateState()
{
    if ( d->dirtyNewPage )
	d->initPage(state);
    if ( d->dirtyMatrix )
	d->matrixSetup(state);
    if ( d->dirtyClipping ) // Must be after matrixSetup and initPage
	d->clippingSetup(state);
    if ( d->dirtypen ) {
	// we special-case for narrow solid lines with the default
	// cap and join styles
	if ( d->cpen.style() == Qt::SolidLine && d->cpen.width() == 0 &&
	     d->cpen.capStyle() == Qt::FlatCap &&
	     d->cpen.joinStyle() == Qt::MiterJoin )
	    d->pageStream << color( d->cpen.color(), d->printer ) << "P1\n";
	else
	    d->pageStream << (int)d->cpen.style() << ' ' << d->cpen.width()
			  << ' ' << color( d->cpen.color(), d->printer )
			  << psCap( d->cpen.capStyle() )
			  << psJoin( d->cpen.joinStyle() ) << "PE\n";
	d->dirtypen = FALSE;
    }
    if ( d->dirtybrush ) {
	// we special-case for nobrush and solid white, since
	// those are the two most common brushes
	if ( d->cbrush.style() == Qt::NoBrush )
	    d->pageStream << "NB\n";
	else if ( d->cbrush.style() == Qt::SolidPattern &&
		  d->cbrush.color() == Qt::white )
	    d->pageStream << "WB\n";
	else
	    d->pageStream << (int)d->cbrush.style() << ' '
			  << color( d->cbrush.color(), d->printer ) << "BR\n";
	d->dirtybrush = FALSE;
    }
    if ( d->dirtyBkColor ) {
	d->pageStream << color( d->bkColor, d->printer ) << "BC\n";
	d->dirtyBkColor = FALSE;
    }
    if ( d->dirtyBkMode ) {
	if ( d->bkMode == Qt::TransparentMode )
	    d->pageStream << "/OMo false d\n";
	else
	    d->pageStream << "/OMo true d\n";
	d->dirtyBkMode = FALSE;
    }
    return true;
}

void QPSPrinter::updatePen(QPainterState *ps)
{
    d->dirtypen = TRUE;
    d->cpen = ps->pen;
}

void QPSPrinter::updateBrush(QPainterState *ps)
{
    if ( ps->brush.style() == Qt::CustomPattern ) {
#if defined(CHECK_RANGE)
	qWarning( "QPrinter: Pixmap brush not supported" );
#endif
	return;
    }
    d->dirtybrush = TRUE;
    d->cbrush = ps->brush;
}

void QPSPrinter::updateFont(QPainterState *ps)
{
    d->currentSet = ps->font;
    d->fm = QFontMetrics(*ps->pfont);
    // turn these off - they confuse the 'avoid font change' logic
    d->currentSet.setUnderline( FALSE );
    d->currentSet.setStrikeOut( FALSE );
}

void QPSPrinter::updateRasterOp(QPainterState *)
{
    qWarning("raster ops not supported on Postscript driver");
}

void QPSPrinter::updateBackground(QPainterState *ps)
{
    d->bkColor = ps->bgBrush;
    d->dirtyBkColor = TRUE;
    d->bkMode = ps->bgMode;
    d->dirtyBkMode = TRUE;
}

void QPSPrinter::updateXForm(QPainterState *)
{
    d->dirtyMatrix = TRUE;
}

void QPSPrinter::updateClipRegion(QPainterState *)
{
    d->dirtyClipping = TRUE;
}

void QPSPrinter::drawLine(const QPoint &p1, const QPoint &p2)
{
    d->pageStream << POINT(p2);
    if (p1.y() == p2.y())
	d->pageStream << p1.x() << " HL\n";
    else if (p1.x() == p2.x())
	d->pageStream << p1.y() << " VL\n";
    else
	d->pageStream << POINT(p1) << "DL\n";
}

void QPSPrinter::drawRect(const QRect &r)
{
    d->pageStream << RECT(r) << "R\n";
}

void QPSPrinter::drawPoint(const QPoint &p)
{
    d->pageStream << POINT(p) << "P\n";
}

void QPSPrinter::drawPoints(const QPointArray &pa, int index, int npoints)
{
    for (int i = 0; i < npoints; ++i)
	drawPoint(pa[i+index]);
}

void QPSPrinter::drawWinFocusRect(const QRect &r, bool xorPaint, const QColor &bgColor)
{
    drawRect(r);
}

void QPSPrinter::drawRoundRect(const QRect &r, int xRnd, int yRnd)
{
    d->pageStream << RECT(r) << INT_ARG(xRnd) << INT_ARG(yRnd) << "RR\n";
}

void QPSPrinter::drawEllipse(const QRect &r)
{
    d->pageStream << RECT(r) << "E\n";
}

void QPSPrinter::drawArc(const QRect &r, int a, int alen)
{
    d->pageStream << RECT(r) << INT_ARG(a) << INT_ARG(alen) << "A\n";
}

void QPSPrinter::drawPie(const QRect &r, int a, int alen)
{
    d->pageStream << RECT(r) << INT_ARG(a) << INT_ARG(alen) << "PIE\n";
}

void QPSPrinter::drawChord(const QRect &r, int a, int alen)
{
    d->pageStream << RECT(r) << INT_ARG(a) << INT_ARG(alen) << "CH\n";
}

void QPSPrinter::drawLineSegments(const QPointArray &a, int index, int nlines)
{
    d->pageStream << "NP\n";
    for (int i = index; i < index+nlines; i ++) {
	QPoint pt = a.point( 2*i );
	d->pageStream << XCOORD(pt.x()) << ' '
		      << YCOORD(pt.y()) << " MT\n";
	pt = a.point( 2*i+1 );
	d->pageStream << XCOORD(pt.x()) << ' '
		      << YCOORD(pt.y()) << " LT\n";
    }
    d->pageStream << "QS\n";
}

void QPSPrinter::drawPolyline(const QPointArray &a, int index, int npoints)
{
    QPoint pt = a.point(index);
    d->pageStream << "NP\n"
		  << XCOORD(pt.x()) << ' ' << YCOORD(pt.y()) << " MT\n";
    for (int i = 1; i < npoints; i++) {
	pt = a.point(i + index);
	d->pageStream << XCOORD(pt.x()) << ' '
		      << YCOORD(pt.y()) << " LT\n";
    }
    d->pageStream << "QS\n";
}

void QPSPrinter::drawPolygon(const QPointArray &a, bool winding, int index, int npoints)
{
    if (winding)
	d->pageStream << "/WFi true d\n";
    QPoint pt = a.point(index);
    d->pageStream << "NP\n";
    d->pageStream << XCOORD(pt.x()) << ' '
		  << YCOORD(pt.y()) << " MT\n";
    for(int i = 1; i < npoints; i++) {
	pt = a.point(i + index);
	d->pageStream << XCOORD(pt.x()) << ' '
		      << YCOORD(pt.y()) << " LT\n";
    }
    d->pageStream << "CP BF QS\n";
    if ( winding )
	d->pageStream << "/WFi false d\n";
}

void QPSPrinter::drawConvexPolygon(const QPointArray &, int index, int npoints)
{
    // ####################
}

#ifndef QT_NO_BEZIER
void QPSPrinter::drawCubicBezier(const QPointArray &a, int index)
{
    d->pageStream << "NP\n";
    d->pageStream << XCOORD(a[index].x()) << ' '
		  << YCOORD(a[index].y()) << " MT ";
    for (int i = 1; i < 4; i++) {
	d->pageStream << XCOORD(a[index + i].x()) << ' '
		      << YCOORD(a[index + i].y()) << ' ';
    }
    d->pageStream << "BZ\n";
}
#endif

void QPSPrinter::drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr)
{
    // ###### fix sr
    QImage img = pm;
    QImage mask;
    if ( pm.mask() )
	mask = *pm.mask();
    d->drawImage(r.x(), r.y(), r.width(), r.height(), img, mask);
}

void QPSPrinter::drawTextItem(const QPoint &p, const QTextItem &ti, int textflags)
{
    int x = p.x();
    int y = p.y();
    // #########################
//     QScriptItem &si = ti.engine()->items[ti.item()];
//     int len = ti.engine()->length( ti.item() );
//     if ( si.isSpace || si.isObject )
// 	return;

//     if ( d->currentSet != d->currentUsed || d->scriptUsed != si.analysis.script || !d->currentFontFile ) {
// 	d->currentUsed = d->currentSet;
// 	d->setFont( d->currentSet, si.analysis.script );
//     }
//     if( d->currentFontFile ) // better not crash in case somethig goes wrong.
// 	d->currentFontFile->drawText( d->pageStream, QPoint(x, y), ti.engine(), ti.item(),
// 				      ti.engine()->string.mid( si.position, len ), d, state);
}

void QPSPrinter::drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &p, bool optim)
{
    // ####################
}

#if 0
 case PdcDrawImage: {
        if ( p[1].image->isNull() )
            break;
        QRect r = *(p[0].rect);
        QImage img = *(p[1].image);
	QImage mask;
#ifndef QT_NO_IMAGE_DITHER_TO_1
	if ( img.hasAlphaBuffer() )
	    mask = img.createAlphaMask();
#endif
        d->drawImage(r.x(), r.y(), r.width(), r.height(), img, mask);
        break;
    }
#endif

void QPSPrinter::newPage()
{
 // we're writing to lp/lpr through a pipe, we don't want to crash with SIGPIPE
// if lp/lpr dies
ignoreSigPipe(TRUE);
d->flushPage();
ignoreSigPipe(FALSE);

d->dirtyNewPage = TRUE;
}

void QPSPrinter::abort()
{
}

#endif // QT_NO_PRINTER
