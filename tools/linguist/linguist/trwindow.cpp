/**********************************************************************
**   Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
**   trwindow.cpp
**
**   This file is part of Qt Linguist.
**
**   See the file LICENSE included in the distribution for the usage
**   and distribution terms.
**
**   The file is provided AS IS with NO WARRANTY OF ANY KIND,
**   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR
**   A PARTICULAR PURPOSE.
**
**********************************************************************/

/*  TRANSLATOR TrWindow

  This is the application's main window.
*/

#include "trwindow.h"
#include "listviews.h"
#include "finddialog.h"
#include "msgedit.h"
#include "phrasebookbox.h"
#include "printout.h"
#include "about.h"

#include <qaccel.h>
#include <qaction.h>
#include <qapplication.h>
#include <qbitmap.h>
#include <qdict.h>
#include <qdockarea.h>
#include <qdockwindow.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qfontdialog.h>
#include <qheader.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qsettings.h>
#include <qstatusbar.h>
#include <qtextbrowser.h>
#include <qtextstream.h>
#include <qtoolbar.h>
#include <qwhatsthis.h>
#include <qprocess.h>

#include "phraselv.h"

#include <images.h>

#include <stdlib.h> // getenv()

#ifdef Q_WS_MACX
//logo is 40x40 on the mac, if it changes - please notify the Qt/Mac team
static const char *logo_xpm[] = {
"40 40 543 2",
"  	c None",
". 	c #C9D98D",
"+ 	c #A0BC30",
"@ 	c #A4C132",
"# 	c #ABC92F",
"$ 	c #AAC733",
"% 	c #8FA733",
"& 	c #657821",
"* 	c #3F4E17",
"= 	c #2A360C",
"- 	c #182306",
"; 	c #121B06",
"> 	c #141C06",
", 	c #1D2608",
"' 	c #2A350E",
") 	c #405017",
"! 	c #667C1C",
"~ 	c #8FAA24",
"{ 	c #A8C12D",
"] 	c #728326",
"^ 	c #313A14",
"/ 	c #090B08",
"( 	c #000003",
"_ 	c #000001",
": 	c #000002",
"< 	c #000004",
"[ 	c #000000",
"} 	c #080D01",
"| 	c #333E11",
"1 	c #6E8529",
"2 	c #A2C230",
"3 	c #ACC735",
"4 	c #7D8F2C",
"5 	c #2A300E",
"6 	c #000300",
"7 	c #000007",
"8 	c #010009",
"9 	c #020104",
"0 	c #020301",
"a 	c #020300",
"b 	c #010200",
"c 	c #010101",
"d 	c #020010",
"e 	c #010010",
"f 	c #000005",
"g 	c #000503",
"h 	c #28360C",
"i 	c #789128",
"j 	c #AAC835",
"k 	c #A4BE36",
"l 	c #515F1E",
"m 	c #040800",
"n 	c #00000C",
"o 	c #02000D",
"p 	c #040303",
"q 	c #020200",
"r 	c #020203",
"s 	c #020202",
"t 	c #000008",
"u 	c #000105",
"v 	c #000202",
"w 	c #000201",
"x 	c #020207",
"y 	c #02020C",
"z 	c #00000B",
"A 	c #02010E",
"B 	c #000013",
"C 	c #030900",
"D 	c #4C5E1B",
"E 	c #A1BD33",
"F 	c #9BB52F",
"G 	c #333E10",
"H 	c #010002",
"I 	c #03020B",
"J 	c #020109",
"K 	c #030204",
"L 	c #010104",
"M 	c #020009",
"N 	c #02000A",
"O 	c #000009",
"P 	c #000006",
"Q 	c #04050B",
"R 	c #020106",
"S 	c #020307",
"T 	c #040507",
"U 	c #000104",
"V 	c #34410D",
"W 	c #99B236",
"X 	c #9BB332",
"Y 	c #2B360A",
"Z 	c #020006",
"` 	c #040301",
" .	c #030304",
"..	c #01010A",
"+.	c #020113",
"@.	c #010011",
"#.	c #010500",
"$.	c #0A0F05",
"%.	c #131C07",
"&.	c #0B1205",
"*.	c #020706",
"=.	c #010309",
"-.	c #030503",
";.	c #030402",
">.	c #2C350C",
",.	c #9DB52F",
"'.	c #A3BD2F",
").	c #03000A",
"!.	c #030102",
"~.	c #020101",
"{.	c #030209",
"].	c #02010B",
"^.	c #010301",
"/.	c #242C08",
"(.	c #5A6720",
"_.	c #81922B",
":.	c #93A732",
"<.	c #9AB530",
"[.	c #9CB734",
"}.	c #90AA32",
"|.	c #7E942C",
"1.	c #566820",
"2.	c #273310",
"3.	c #000403",
"4.	c #00000E",
"5.	c #020211",
"6.	c #03030B",
"7.	c #020205",
"8.	c #01000C",
"9.	c #33410B",
"0.	c #A7BF37",
"a.	c #ADC935",
"b.	c #4F5F18",
"c.	c #020103",
"d.	c #080401",
"e.	c #050205",
"f.	c #232C06",
"g.	c #768722",
"h.	c #A4BB2E",
"i.	c #D5E38D",
"j.	c #A2BE3A",
"k.	c #738833",
"l.	c #202C0A",
"m.	c #05030A",
"n.	c #020206",
"o.	c #030303",
"p.	c #505B20",
"q.	c #ACC934",
"r.	c #7C8E2B",
"s.	c #050900",
"t.	c #010201",
"u.	c #030200",
"v.	c #020001",
"w.	c #050207",
"x.	c #030004",
"y.	c #000203",
"z.	c #49531D",
"A.	c #A2B836",
"B.	c #9FBD30",
"C.	c #4E5A1C",
"D.	c #000200",
"E.	c #04040A",
"F.	c #040403",
"G.	c #010005",
"H.	c #040600",
"I.	c #7A9222",
"J.	c #ACCB2D",
"K.	c #A9C22F",
"L.	c #282F0B",
"M.	c #020304",
"N.	c #030109",
"O.	c #55671A",
"P.	c #AEC93D",
"Q.	c #A9C931",
"R.	c #546523",
"S.	c #00000A",
"T.	c #020204",
"U.	c #010300",
"V.	c #272F0B",
"W.	c #A7BE34",
"X.	c #728327",
"Y.	c #020108",
"Z.	c #030305",
"`.	c #030208",
" +	c #46511B",
".+	c #ACC739",
"++	c #93AC39",
"@+	c #263605",
"#+	c #566A1D",
"$+	c #ADC93C",
"%+	c #ACC840",
"&+	c #44541E",
"*+	c #010205",
"=+	c #010206",
"-+	c #020107",
";+	c #010007",
">+	c #030400",
",+	c #728226",
"'+	c #AAC732",
")+	c #323A14",
"!+	c #030401",
"~+	c #01000D",
"{+	c #242D06",
"]+	c #A2B835",
"^+	c #95AD3E",
"/+	c #212C0F",
"(+	c #010604",
"_+	c #5F7327",
":+	c #ABC934",
"<+	c #A2BD3A",
"[+	c #202C09",
"}+	c #01010B",
"|+	c #020105",
"1+	c #333B14",
"2+	c #ABC23C",
"3+	c #8EA631",
"4+	c #0A0C08",
"5+	c #010008",
"6+	c #010102",
"7+	c #020011",
"8+	c #030500",
"9+	c #778823",
"0+	c #96AD3A",
"a+	c #232D0D",
"b+	c #020212",
"c+	c #000109",
"d+	c #030800",
"e+	c #5F7126",
"f+	c #ABCB35",
"g+	c #71862A",
"h+	c #020401",
"i+	c #090C03",
"j+	c #8FA238",
"k+	c #647822",
"l+	c #02000B",
"m+	c #232B08",
"n+	c #A5BB30",
"o+	c #96AC35",
"p+	c #242C0A",
"q+	c #020402",
"r+	c #030403",
"s+	c #000100",
"t+	c #576D1D",
"u+	c #A2BC37",
"v+	c #232D0A",
"w+	c #010004",
"x+	c #677822",
"y+	c #596620",
"z+	c #CBDC71",
"A+	c #96B034",
"B+	c #232B0E",
"C+	c #01020B",
"D+	c #010109",
"E+	c #030502",
"F+	c #040603",
"G+	c #010107",
"H+	c #253704",
"I+	c #B2CE3A",
"J+	c #586721",
"K+	c #425014",
"L+	c #C3D771",
"M+	c #29360C",
"N+	c #020302",
"O+	c #81912F",
"P+	c #94AE38",
"Q+	c #23300B",
"R+	c #040408",
"S+	c #010105",
"T+	c #010207",
"U+	c #242C0C",
"V+	c #91AB33",
"W+	c #D0E18D",
"X+	c #7F922B",
"Y+	c #010400",
"Z+	c #010106",
"`+	c #2A3408",
" @	c #182406",
".@	c #00000D",
"+@	c #0B0F06",
"@@	c #93A736",
"#@	c #96AE38",
"$@	c #000101",
"%@	c #030308",
"&@	c #232A0B",
"*@	c #98AC37",
"=@	c #94AB32",
"-@	c #0C1204",
";@	c #1B2307",
">@	c #9EB62F",
",@	c #010100",
"'@	c #9AB431",
")@	c #97AD37",
"!@	c #242B0E",
"~@	c #01000B",
"{@	c #01010C",
"]@	c #272C13",
"^@	c #9CAF3B",
"/@	c #131B07",
"(@	c #151B07",
"_@	c #9CB230",
":@	c #ADC555",
"<@	c #131C06",
"[@	c #94AB38",
"}@	c #252E0A",
"|@	c #262914",
"1@	c #9DAF46",
"2@	c #9DBA30",
"3@	c #141C07",
"4@	c #010302",
"5@	c #141906",
"6@	c #9CB332",
"7@	c #9EBB31",
"8@	c #1D2609",
"9@	c #010003",
"0@	c #0B1105",
"a@	c #91AA31",
"b@	c #222C0D",
"c@	c #05040A",
"d@	c #232B0A",
"e@	c #98AC3A",
"f@	c #91AB2D",
"g@	c #0B1204",
"h@	c #1D2307",
"i@	c #A0B834",
"j@	c #A5C233",
"k@	c #29340F",
"l@	c #020705",
"m@	c #7E942D",
"n@	c #95AE3B",
"o@	c #232E10",
"p@	c #03040C",
"q@	c #02020B",
"r@	c #020400",
"s@	c #222A0B",
"t@	c #92AE2E",
"u@	c #7D9327",
"v@	c #030704",
"w@	c #2A340D",
"x@	c #A7BF35",
"y@	c #BAD355",
"z@	c #3E4E18",
"A@	c #020110",
"B@	c #566821",
"C@	c #243503",
"D@	c #020213",
"E@	c #030501",
"F@	c #030306",
"G@	c #030207",
"H@	c #010203",
"I@	c #030206",
"J@	c #202E0B",
"K@	c #95AF37",
"L@	c #59691D",
"M@	c #43501A",
"N@	c #ADC732",
"O@	c #657A1D",
"P@	c #000102",
"Q@	c #01000E",
"R@	c #010108",
"S@	c #27330F",
"T@	c #A3BE3D",
"U@	c #586C1E",
"V@	c #000404",
"W@	c #00010A",
"X@	c #050704",
"Y@	c #262B13",
"Z@	c #262B15",
"`@	c #02010C",
" #	c #212E08",
".#	c #97AE3B",
"+#	c #ADC440",
"@#	c #232D06",
"##	c #030101",
"$#	c #6B7D21",
"%#	c #D5E48D",
"&#	c #8DA828",
"*#	c #02020A",
"=#	c #000106",
"-#	c #000401",
";#	c #748736",
">#	c #AECB3D",
",#	c #5E7227",
"'#	c #010204",
")#	c #252B0C",
"!#	c #9BAD3F",
"~#	c #A0B148",
"{#	c #232B0B",
"]#	c #020007",
"^#	c #020209",
"/#	c #2C3418",
"(#	c #646E34",
"_#	c #030600",
":#	c #030202",
"<#	c #050102",
"[#	c #0A1002",
"}#	c #90A727",
"|#	c #CFDE8D",
"1#	c #343D13",
"2#	c #02010D",
"3#	c #040509",
"4#	c #020308",
"5#	c #202B09",
"6#	c #9FBE2E",
"7#	c #ACCA34",
"8#	c #5E7026",
"9#	c #252C0D",
"0#	c #9AAE39",
"a#	c #9AAE3D",
"b#	c #22290D",
"c#	c #010306",
"d#	c #010209",
"e#	c #020004",
"f#	c #030201",
"g#	c #030100",
"h#	c #030009",
"i#	c #333C12",
"j#	c #AAC42F",
"k#	c #738429",
"l#	c #03020F",
"m#	c #4B591A",
"n#	c #A9C831",
"o#	c #A9C932",
"p#	c #576B1F",
"q#	c #263706",
"r#	c #91AB34",
"s#	c #92AC37",
"t#	c #202C0C",
"u#	c #6F8422",
"v#	c #A8BF33",
"w#	c #2B340C",
"x#	c #03010D",
"y#	c #516221",
"z#	c #AFC655",
"A#	c #95AE39",
"B#	c #202E09",
"C#	c #000204",
"D#	c #020102",
"E#	c #28360A",
"F#	c #7D8E2C",
"G#	c #040700",
"H#	c #03020C",
"I#	c #475520",
"J#	c #A3BD3C",
"K#	c #98AF3A",
"L#	c #2D3516",
"M#	c #010103",
"N#	c #0B0D05",
"O#	c #7E9627",
"P#	c #B0C833",
"Q#	c #4E5A17",
"R#	c #222B0B",
"S#	c #72852B",
"T#	c #A4BE35",
"U#	c #AFC544",
"V#	c #666F37",
"W#	c #171A0A",
"X#	c #93AB3A",
"Y#	c #A4BA2F",
"Z#	c #36400C",
"`#	c #040404",
" $	c #28310B",
".$	c #5C6C20",
"+$	c #7F922A",
"@$	c #93A833",
"#$	c #9CB92D",
"$$	c #90AA2C",
"%$	c #59691F",
"&$	c #222A07",
"*$	c #040501",
"=$	c #222D0D",
"-$	c #93AA3F",
";$	c #9DB03A",
">$	c #2C340D",
",$	c #000500",
"'$	c #0D1205",
")$	c #131A07",
"!$	c #0B1203",
"~$	c #010502",
"{$	c #020008",
"]$	c #010202",
"^$	c #212D0D",
"/$	c #94AE34",
"($	c #9FB435",
"_$	c #343F0C",
":$	c #040207",
"<$	c #020502",
"[$	c #2D4102",
"}$	c #A6BD33",
"|$	c #4F5B1D",
"1$	c #030701",
"2$	c #020201",
"3$	c #040502",
"4$	c #030505",
"5$	c #020100",
"6$	c #040102",
"7$	c #030007",
"8$	c #090C04",
"9$	c #16190A",
"0$	c #0A1301",
"a$	c #708727",
"b$	c #ACC836",
"c$	c #7B9223",
"d$	c #29300D",
"e$	c #7F9626",
"f$	c #94AB39",
"g$	c #212E0C",
"h$	c #020603",
"i$	c #101A01",
"j$	c #7A912E",
"k$	c #ADCB2F",
"l$	c #B0CD3A",
"m$	c #A6BD34",
"n$	c #728225",
"o$	c #0A0D03",
"p$	c #0C1101",
"q$	c #343D12",
"r$	c #6F8421",
"s$	c #94AB41",
"t$	c #202C0D",
"u$	c #0A1302",
"v$	c #7A902F",
"w$	c #B0CE37",
"x$	c #ABC33C",
"y$	c #8EA236",
"z$	c #677821",
"A$	c #425115",
"B$	c #2A3409",
"C$	c #151A07",
"D$	c #141A06",
"E$	c #1C2307",
"F$	c #2B330D",
"G$	c #43511B",
"H$	c #6C7E22",
"I$	c #ABC530",
"J$	c #92AC32",
"K$	c #314305",
"L$	c #6F8625",
"M$	c #ACCB30",
"N$	c #9CB330",
"O$	c #9CB432",
"P$	c #A1B935",
"Q$	c #A8BF36",
"R$	c #ADC632",
"                                                                                ",
"                                        . + @ #                                 ",
"                          $ % & * = - ; > , ' ) ! ~                             ",
"                      { ] ^ / ( _ _ : < : [ ( ( : } | 1 2                       ",
"                  3 4 5 6 [ 7 8 9 0 a b b c 8 d e 8 f g h i j                   ",
"                k l m n o p q q r s s t u v w x y z A B t C D E                 ",
"              F G [ H I J K L M N t 7 n O P P Q R S T U t < [ V W               ",
"            X Y _ Z `  ...+.@.7 [ #.$.; %.&.*.: : t =.-.;.S z : >.,.            ",
"          '.G < ).!.~.{.].P ^./.(._.:.<.[.}.|.1.2.3.4.5.6.7...8.[ 9.0.          ",
"        a.b.[ M c.d.e.J [ f.g.h.i.              j.k.l.( m.n.L x o.[ p.q.        ",
"        r.s.t.u.v.w.x.y.z.A.                        B.C.D.f E.F.c G.H.I.J.      ",
"      K.L.O M.b 7.N.y.O.P.                            Q.R.v S.T.U.R f V.W.      ",
"      X.a Y.c Z.`.[  +.+                    ++@+#+$+    %+&+[ *+=+-+;+>+,+      ",
"    '+)+_ !+0 ~+f {+]+                    ^+/+O (+_+:+    <+[+: }+n.|+: 1+2+    ",
"    3+4+5+b 6+7+8+9+                    0+a+: b+c+d+e+f+    g+h+7 n.r f i+j+    ",
"    k+( 8 r l+7 m+n+                  o+p+< 6.q+r+< s+t+    u+v+[ w+6+: [ x+    ",
"    * [ 9 R d _ y+z+                A+B+[ C+D+E+F+G+[ H+    I+J+[ < 6+c : K+    ",
"  L+M+[ q |+e N+O+                P+Q+[ L R+S+7.T+[ U+V+W+    X+Y+< [ Z+z `+    ",
"     @_ a T..@+@@@              #@Q+$@D.T.%@( L : &@*@        =@-@f c 5+n ;@>@  ",
"    ; : ,@t O ; '@            )@!@[ h+G+~@;+~@{@]@^@          <+/@f U.w _ (@_@  ",
"  :@> _ c u P <@[.          [@}@[ L  .;+Z+c |+S.|@1@          2@3@P 4@a [ 5@6@  ",
"  7@8@[ 9@w < 0@a@        ^+b@P y c@`.P 6+Y+b ;+_ d@e@        f@g@t *+T.( h@i@  ",
"  j@k@: 8 w f l@m@      n@o@( p@q@x 9 8.5+b r@T.].[ s@t@      u@v@8.( `.< w@x@  ",
"  y@z@: A@x Q _ B@      C@7 D@E+E@F@S+8 7 G@H@I@l+M [ J@K@    L@< 8.~.G@[ M@N@  ",
"    O@P@Q@y R@: S@T@    U@V@W@-.X@x ( Y@Z@: `@N |+r t._  #.#+#@#f N.##I@[ $#%#  ",
"    &#} 5+n *#=#-#;#    >#,#C '#Y._ )#!#~#{#_ ]#q a s ^#[ /#(#_#:#<#x.f [#}#    ",
"    |#1#: 2#3#4#n 5#6#    7#8#D.( 9#0#    a#b#[ t.|+L c#d#n [ e#f#g#h#< i#j#    ",
"      k#U.@.( r+l#: m#n#    o#p#q#r#        s#t#_ =+4@,@9@G._ r 6+c ( 6 u#      ",
"      v#w#S.z 7.x#l+6 y#%+      z#            A#B#[ C#6+c ,@c c s D#[ E#        ",
"        F#G#O H#Z+8 ..y.I#J#                    K#L#8 G.c c c c o.M#N#O#        ",
"        P#Q#[ Y.n.q  .=#[ R#S#T#                U#V#c : c c s c o.T.W#X#        ",
"          Y#Z#[ J `#>+U.*+f -# $.$+$@$  #$$$u@%$&$*$~@T.,@c c o.s s [ =$-$      ",
"            ;$>$_ G@L U.=+Q@O [ [ ,$'$)$3@!$~$( 7 {$9 6+s o.o.s s s ]$[ ^$/$    ",
"              ($_$[ M M n.T.|+G@R ;+P n z 7 7 R@:$<#g#c c H L 6+s s ]$<$[ [$    ",
"                }$|$1$P P q 2$L R c 3$4$'#H@0 a 5$6$7$( [ 8$9$[ ]$]$t.s+0$a$    ",
"                  b$c$d$D.[ f : H S+;+D.b r %@G@9 ( : w M+e$f$g$[ h$s+i$j$k$    ",
"                    l$m$n$1+o$[ : z n [ [ ( f _ s+p$q$r$      s$t$[ u$v$w$      ",
"                          x$y$z$A$B$;@C$D$E$F$G$H$}#I$          J$K$L$M$        ",
"                                    >@N$O$P$Q$R$                                ",
"                                                                                "};
#else
static const char *logo_xpm[] = {
/* width height num_colors chars_per_pixel */
"21 16 213 2",
"   c white",
".  c #A3C511",
"+  c #A2C511",
"@  c #A2C611",
"#  c #A2C510",
"$  c #A2C513",
"%  c #A2C412",
"&  c #A2C413",
"*  c #A2C414",
"=  c #A2C515",
"-  c #A2C50F",
";  c #A3C510",
">  c #A2C410",
",  c #A2C411",
"'  c #A2C314",
")  c #A2C316",
"!  c #A2C416",
"~  c #A0C315",
"{  c #A1C313",
"]  c #A1C412",
"^  c #A2C40F",
"/  c #A1C410",
"(  c #A0C510",
"_  c #A0C511",
":  c #A1C414",
"<  c #9FC30E",
"[  c #98B51B",
"}  c #5F7609",
"|  c #5C6E0E",
"1  c #5B6E10",
"2  c #5C6C14",
"3  c #5A6E0A",
"4  c #839E16",
"5  c #A0C515",
"6  c #A0C513",
"7  c #A2C512",
"8  c #A1C512",
"9  c #A1C511",
"0  c #A1C50F",
"a  c #91AE12",
"b  c #505E11",
"c  c #1F2213",
"d  c #070606",
"e  c #040204",
"f  c #040306",
"g  c #15160F",
"h  c #2F3A0D",
"i  c #859F1B",
"j  c #A1C215",
"k  c #A0C50F",
"l  c #A1C510",
"m  c #A0C110",
"n  c #839C1B",
"o  c #1E240A",
"p  c #050205",
"q  c #030304",
"r  c #323917",
"s  c #556313",
"t  c #56680B",
"u  c #536609",
"v  c #4A561B",
"w  c #0B0D04",
"x  c #030208",
"y  c #090A05",
"z  c #5F6F18",
"A  c #A0C117",
"B  c #91AF10",
"C  c #1E2209",
"D  c #030205",
"E  c #17190D",
"F  c #7D981C",
"G  c #9ABA12",
"H  c #A3C411",
"I  c #A3C713",
"J  c #95B717",
"K  c #7F9A18",
"L  c #8FAE1B",
"M  c #394413",
"N  c #040305",
"O  c #090807",
"P  c #6C7E19",
"Q  c #A6C614",
"R  c #A1C411",
"S  c #64761F",
"T  c #030105",
"U  c #070707",
"V  c #728513",
"W  c #A2C40C",
"X  c #A2C70B",
"Y  c #89A519",
"Z  c #313B11",
"`  c #101409",
" . c #586A19",
".. c #97B620",
"+. c #1B2207",
"@. c #282D11",
"#. c #A6C41B",
"$. c #A1C413",
"%. c #A3C512",
"&. c #2E370B",
"*. c #030108",
"=. c #21260F",
"-. c #A5C21A",
";. c #A0C60D",
">. c #6D841A",
",. c #0F1007",
"'. c #040207",
"). c #0E1009",
"!. c #515F14",
"~. c #A2C41B",
"{. c #5E701B",
"]. c #030203",
"^. c #0B0B04",
"/. c #87A111",
"(. c #A0C411",
"_. c #A0C316",
":. c #212907",
"<. c #222C0B",
"[. c #A3C516",
"}. c #9CBE1A",
"|. c #5E6F1B",
"1. c #0E0F0B",
"2. c #040205",
"3. c #181B0D",
"4. c #93AE25",
"5. c #A0C610",
"6. c #617715",
"7. c #030306",
"8. c #070704",
"9. c #809818",
"0. c #A1C415",
"a. c #475416",
"b. c #030309",
"c. c #12170B",
"d. c #91B01E",
"e. c #5C721F",
"f. c #05050B",
"g. c #33371D",
"h. c #0E0F08",
"i. c #040405",
"j. c #758921",
"k. c #46511B",
"l. c #030207",
"m. c #131409",
"n. c #9FB921",
"o. c #859D21",
"p. c #080809",
"q. c #030305",
"r. c #46521C",
"s. c #8EB017",
"t. c #627713",
"u. c #4D5F17",
"v. c #97B71D",
"w. c #77901D",
"x. c #151708",
"y. c #0D0D0B",
"z. c #0C0B08",
"A. c #455216",
"B. c #A5C616",
"C. c #A0C114",
"D. c #556118",
"E. c #050307",
"F. c #050407",
"G. c #363E17",
"H. c #5D7309",
"I. c #A2BF28",
"J. c #A2C417",
"K. c #A4C620",
"L. c #60701D",
"M. c #030103",
"N. c #030303",
"O. c #809A1B",
"P. c #A0C310",
"Q. c #A0C410",
"R. c #A3C415",
"S. c #9CB913",
"T. c #6F801F",
"U. c #1A210A",
"V. c #1D1E0D",
"W. c #1D220F",
"X. c #1E210F",
"Y. c #0F0F07",
"Z. c #0E1007",
"`. c #090906",
" + c #2B360E",
".+ c #97B813",
"++ c #A2C50E",
"@+ c #A5C517",
"#+ c #90AD20",
"$+ c #5D6C1A",
"%+ c #394115",
"&+ c #050704",
"*+ c #040304",
"=+ c #202807",
"-+ c #5E6B21",
";+ c #728D0C",
">+ c #65791D",
",+ c #29330F",
"'+ c #7A911D",
")+ c #A2C614",
"!+ c #A1C513",
"~+ c #A3C50E",
"{+ c #A3C414",
"]+ c #9CBD11",
"^+ c #95B40C",
"/+ c #94B50F",
"(+ c #95B510",
"_+ c #99B913",
":+ c #A0C414",
"<+ c #9ABC11",
"[+ c #A0C314",
"}+ c #A1C40F",
"|+ c #A3C513",
". + + @ + # # $ % & * = & - + + + + + # # ",
"; > , > # > > $ ' ) ! ~ { ] ^ , - > , > # ",
"+ + / ( _ : < [ } | 1 2 3 4 5 6 : 7 8 # # ",
"+ 9 # ( 0 a b c d e e e f g h i j 9 k l + ",
"+ + > m n o p q r s t u v w x y z A & # # ",
"# % k B C D E F G H I J K L M N O P Q ] , ",
"$ R > S T U V W , X Y Z `  ...+.T @.#.$.] ",
"% %.* &.*.=.-.;.> >.,.'.).!.~.{.].^./.R 7 ",
"7 (._.:.D <.[.}.|.1.2.2.3.4.5.6.7.8.9._ 8 ",
". % 0.a.b.c.d.e.f.N g.h.2.i.j.k.l.m.n.$ # ",
"; + ; o.p.q.r.s.t.u.v.w.x.2.y.z.].A.B.l : ",
"# # R C.D.E.F.G.H.I.J.K.L.2.M.M.N.O.P.; l ",
"# / Q.R.S.T.U.].8.V.W.X.Y.e Z.`.]. +.+++7 ",
"+ + 9 / ; @+#+$+%+&+e *+=+-+;+>+,+'+)+, # ",
"# + > % & !+~+{+]+^+/+(+_+) Q.:+<+[+$ R # ",
"7 + > }+# % k |+8 + > + * $ _ / , 7 8 ] - "};
#endif

#define check_danger_mask_width 17
#define check_danger_mask_height 13
static const uchar check_danger_mask_bits[] = {
   0x00, 0x00, 0x00, 0x80, 0x03, 0x00, 0x80, 0x03, 0x00, 0x80, 0x03, 0x00,
   0x80, 0x03, 0x00, 0x80, 0x03, 0x00, 0x80, 0x03, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x80, 0x03, 0x00, 0x80, 0x03, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00 };

#define check_off_mask_width 17
#define check_off_mask_height 13
static const uchar check_off_mask_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x07, 0x00, 0xe0, 0x0e, 0x00,
   0xe0, 0x0e, 0x00, 0x00, 0x07, 0x00, 0x80, 0x03, 0x00, 0x80, 0x03, 0x00,
   0x00, 0x00, 0x00, 0x80, 0x03, 0x00, 0x80, 0x03, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00 };

#define check_on_mask_width 17
#define check_on_mask_height 13
static const uchar check_on_mask_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x1c, 0x00,
   0x00, 0x1e, 0x00, 0x10, 0x0f, 0x00, 0xb0, 0x07, 0x00, 0xf0, 0x03, 0x00,
   0xe0, 0x01, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00 };

#define pagecurl_mask_width 53
#define pagecurl_mask_height 51
static const uchar pagecurl_mask_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0x0f, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0xfe, 0xff,
   0xff, 0xff, 0xff, 0x0f, 0x00, 0xf0, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00,
   0xc0, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x80, 0xff, 0xff, 0xff, 0xff,
   0x0f, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xfe, 0xff,
   0xff, 0xff, 0x0f, 0x00, 0x00, 0xfc, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00,
   0xfc, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xf8, 0xff, 0xff, 0xff, 0x0f,
   0x00, 0x00, 0xf0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xf0, 0xff, 0xff,
   0xff, 0x0f, 0x00, 0x00, 0xf0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xf0,
   0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00,
   0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff,
   0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff,
   0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00,
   0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f,
   0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff,
   0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0,
   0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00,
   0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xc0, 0xff, 0xff, 0xff,
   0x0f, 0x00, 0x00, 0xc0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0xfc,
   0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0x0f, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xfc, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x0f,
   0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xe0, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x0f, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08 };

typedef QValueList<MetaTranslatorMessage> TML;

static const int ErrorMS = 600000; // for error messages
static const int MessageMS = 2500;

static QDict<Embed> *imageDict = 0;

QPixmap * TrWindow::pxOn = 0;
QPixmap * TrWindow::pxOff = 0;
QPixmap * TrWindow::pxObsolete = 0;
QPixmap * TrWindow::pxDanger = 0;

enum Ending { End_None, End_FullStop, End_Interrobang, End_Colon,
	      End_Ellipsis };

static Ending ending( const QString& str )
{
    int ch = 0;
    if ( !str.isEmpty() )
	ch = str.right( 1 )[0].unicode();

    switch ( ch ) {
    case 0x002e: // full stop
	if ( str.endsWith(QString("...")) )
	    return End_Ellipsis;
	else
	    return End_FullStop;
    case 0x0589: // armenian full stop
    case 0x06d4: // arabic full stop
    case 0x3002: // ideographic full stop
	return End_FullStop;
    case 0x0021: // exclamation mark
    case 0x003f: // question mark
    case 0x00a1: // inverted exclamation mark
    case 0x00bf: // inverted question mark
    case 0x01c3: // latin letter retroflex click
    case 0x037e: // greek question mark
    case 0x061f: // arabic question mark
    case 0x203c: // double exclamation mark
    case 0x203d: // interrobang
    case 0x2048: // question exclamation mark
    case 0x2049: // exclamation question mark
    case 0x2762: // heavy exclamation mark ornament
	return End_Interrobang;
    case 0x003a: // colon
	return End_Colon;
    case 0x2026: // horizontal ellipsis
	return End_Ellipsis;
    default:
	return End_None;
    }
}

class Action : public QAction
{
public:
    Action( QPopupMenu *pop, const QString& menuText, QObject *receiver,
	    const char *member, int accel = 0, bool toggle = FALSE );
    Action( QPopupMenu *pop, const QString& menuText, int accel = 0,
	    bool toggle = FALSE );

    virtual void setWhatsThis( const QString& whatsThis );

    bool addToToolbar( QToolBar *tb, const QString& text,
		       const char *imageName );
};

Action::Action( QPopupMenu *pop, const QString& menuText, QObject *receiver,
		const char *member, int accel, bool toggle )
    : QAction( pop->parent(), (const char *) 0, toggle )
{
    QAction::addTo( pop );
    setMenuText( menuText );
    setAccel( accel );
    connect( this, SIGNAL(activated()), receiver, member );
}

Action::Action( QPopupMenu *pop, const QString& menuText, int accel,
		bool toggle )
    : QAction( pop->parent(), (const char *) 0, toggle )
{
    QAction::addTo( pop );
    setMenuText( menuText );
    setAccel( accel );
}

void Action::setWhatsThis( const QString& whatsThis )
{
    QAction::setWhatsThis( whatsThis );
    setStatusTip( whatsThis );
}

bool Action::addToToolbar( QToolBar *tb, const QString& text,
			   const char *imageName )
{
    setText( text );
    Embed *en = imageDict->find( QString("enabled/") + QString(imageName) );
    if ( en != 0 ) {
	QPixmap enabled;
	enabled.loadFromData( en->data, en->size );
 	QIconSet s( enabled );

	Embed *dis = imageDict->find( QString("disabled/") +
				      QString(imageName) );
	if ( dis != 0 ) {
	    QPixmap disabled;
	    disabled.loadFromData( dis->data, dis->size );
	    // work around a bug in QIconSet
	    s.setPixmap( disabled, QIconSet::Small, QIconSet::Disabled,
			 QIconSet::On );
	    s.setPixmap( disabled, QIconSet::Small, QIconSet::Disabled,
			 QIconSet::Off );
	}

 	setIconSet( s );
    }
    return QAction::addTo( tb );
}

const QPixmap TrWindow::splash()
{
    Embed *splash = 0;

    setupImageDict();
    splash = imageDict->find( QString("splash") );
    if ( splash == 0 )
	return 0;
    QPixmap pixmap;
    pixmap.loadFromData( splash->data, splash->size );
    return pixmap;
}

const QPixmap TrWindow::pageCurl()
{
    Embed *ess = 0;

    setupImageDict();
    ess = imageDict->find( QString("pagecurl") );
    if ( ess == 0 )
	return 0;
    QPixmap pixmap;
    pixmap.loadFromData( ess->data, ess->size );
	QBitmap pageCurlMask( pagecurl_mask_width, pagecurl_mask_height,
			pagecurl_mask_bits, TRUE );
	pixmap.setMask( pageCurlMask );
    return pixmap;
}

TrWindow::TrWindow()
    : QMainWindow( 0, "translation window", WType_TopLevel | WDestructiveClose )
{
    setIcon( QPixmap(logo_xpm) );

    setupImageDict();

    // Set up the Scope dock window
    QDockWindow * dwScope = new QDockWindow( QDockWindow::InDock, this,
					     "context");
    dwScope->setResizeEnabled( TRUE );
    dwScope->setCloseMode( QDockWindow::Always );
    addDockWindow( dwScope, tr("Context"), Qt::DockLeft );
    dwScope->setCaption( tr("Context") );
    dwScope->setFixedExtentWidth( 200 );
    lv = new QListView( dwScope, "context list view" );
    lv->setShowSortIndicator( TRUE );
    lv->setAllColumnsShowFocus( TRUE );
    lv->header()->setStretchEnabled( TRUE, 1 );
    lv->addColumn( tr("Done"), 40 );
    lv->addColumn( tr("Context") );
    lv->addColumn( tr("Items"), 55 );
    lv->setColumnAlignment( 0, Qt::AlignCenter );
    lv->setColumnAlignment( 2, Qt::AlignRight );
    lv->setSorting( 0 );
    lv->setHScrollBarMode( QScrollView::AlwaysOff );
    dwScope->setWidget( lv );

    messageIsShown = FALSE;
    me = new MessageEditor( &tor, this, "message editor" );
    setCentralWidget( me );
    slv = me->sourceTextList();
    plv = me->phraseList();

    setupMenuBar();
    setupToolBars();

    progress = new QLabel( statusBar(), "progress" );
    statusBar()->addWidget( progress, 0, TRUE );
    modified = new QLabel( QString(" %1 ").arg(tr("MOD")), statusBar(),
			   "modified?" );
    statusBar()->addWidget( modified, 0, TRUE );

    numFinished = 0;
    numNonobsolete = 0;
    numMessages = 0;
    updateProgress();

    dirty = FALSE;
    updateCaption();

    phraseBooks.setAutoDelete( TRUE );

    f = new FindDialog( FALSE, this, "find", FALSE );
    f->setCaption( tr("Qt Linguist") );
    h = new FindDialog( TRUE, this, "replace", FALSE );
    h->setCaption( tr("Qt Linguist") );
    findMatchCase = FALSE;
    findWhere = 0;
    foundItem = 0;
    foundScope = 0;
    foundWhere = 0;
    foundOffset = 0;

    connect( lv, SIGNAL(currentChanged(QListViewItem *)),
	     this, SLOT(showNewScope(QListViewItem *)) );

    connect( slv, SIGNAL(currentChanged(QListViewItem *)),
	     this, SLOT(showNewCurrent(QListViewItem *)) );

    connect( slv, SIGNAL(clicked(QListViewItem *, const QPoint&, int)),
	     this, SLOT(toggleFinished(QListViewItem *, const QPoint&, int)) );

    connect( me, SIGNAL(translationChanged(const QString&)),
	     this, SLOT(updateTranslation(const QString&)) );
    connect( me, SIGNAL(finished(bool)), this, SLOT(updateFinished(bool)) );
    connect( me, SIGNAL(prevUnfinished()), this, SLOT(prevUnfinished()) );
    connect( me, SIGNAL(nextUnfinished()), this, SLOT(nextUnfinished()) );
    connect( me, SIGNAL(focusSourceList()), this, SLOT(focusSourceList()) );
    connect( me, SIGNAL(focusPhraseList()), this, SLOT(focusPhraseList()) );
    connect( f, SIGNAL(findNext(const QString&, int, bool)),
	     this, SLOT(findNext(const QString&, int, bool)) );

    QWhatsThis::add( lv, tr("This panel lists the source contexts.") );

    QWhatsThis::add( slv, tr("This panel lists the source texts. "
			    "Items that violate validation rules "
			    "are marked with a warning.") );
    showNewCurrent( 0 );

    QSize as( qApp->desktop()->size() );
    as -= QSize( 30, 30 );
    resize( QSize( 1000, 800 ).boundedTo( as ) );
    readConfig();
}

TrWindow::~TrWindow()
{
    writeConfig();
}

void TrWindow::openFile( const QString& name )
{
    if ( !name.isEmpty() ) {
	statusBar()->message( tr("Loading...") );
	qApp->processEvents();
	if ( tor.load(name) ) {
	    slv->clear();
	    slv->repaint();
	    slv->viewport()->repaint();
	    slv->setUpdatesEnabled( FALSE );
	    slv->viewport()->setUpdatesEnabled( FALSE );
	    lv->clear();
	    lv->repaint();
	    lv->viewport()->repaint();
	    lv->setUpdatesEnabled( FALSE );
	    lv->viewport()->setUpdatesEnabled( FALSE );
	    setEnabled( FALSE );
	    numFinished = 0;
	    numNonobsolete = 0;
	    numMessages = 0;

	    TML all = tor.messages();
	    TML::Iterator it;
	    QDict<ContextLVI> contexts( 1009 );

	    for ( it = all.begin(); it != all.end(); ++it ) {
		qApp->processEvents();
		ContextLVI *c = contexts.find( QString((*it).context()) );
		if ( c == 0 ) {
		    c = new ContextLVI( lv, tor.toUnicode((*it).context(),
							  (*it).utf8()) );
		    contexts.insert( QString((*it).context()), c );
		}
		if ( (*it).sourceText()[0] == '\0' ) {
		    c->appendToComment( tor.toUnicode((*it).comment(),
						      (*it).utf8()) );
		} else {
		    MessageLVI * tmp = new MessageLVI( slv, *it,
					   tor.toUnicode((*it).sourceText(),
							 (*it).utf8()),
					   tor.toUnicode((*it).comment(),
							 (*it).utf8()), c );
		    tmp->setDanger( danger(tmp->sourceText(),
					   tmp->translation()) &&
				    tmp->message().type() ==
				    MetaTranslatorMessage::Finished );
		    c->instantiateMessageItem( slv, tmp );

		    if ( (*it).type() != MetaTranslatorMessage::Obsolete ) {
			numNonobsolete++;
			if ( (*it).type() == MetaTranslatorMessage::Finished )
			    numFinished++;
		    } else {
			c->incrementObsoleteCount();
		    }
		    numMessages++;
		}
		c->updateStatus();
	    }
	    slv->viewport()->setUpdatesEnabled( TRUE );
	    slv->setUpdatesEnabled( TRUE );
	    lv->viewport()->setUpdatesEnabled( TRUE );
	    lv->setUpdatesEnabled( TRUE );
	    setEnabled( TRUE );
	    slv->repaint();
	    slv->viewport()->repaint();
	    lv->repaint();
	    lv->viewport()->repaint();
	    updateProgress();
	    filename = name;
	    dirty = FALSE;
	    updateCaption();
	    me->showNothing();
	    doneAndNextAct->setEnabled( FALSE );
	    doneAndNextAlt->setEnabled( FALSE );
	    messageIsShown = FALSE;
	    statusBar()->message(
		    tr("%1 source phrase(s) loaded.").arg(numMessages),
		    MessageMS );

	    foundItem = 0;
	    foundWhere = 0;
	    foundOffset = 0;
	    if ( lv->childCount() > 0 ) {
		findAct->setEnabled( TRUE );
		findAgainAct->setEnabled( FALSE );
#ifdef notyet
		replaceAct->setEnabled( TRUE );
#endif
		lv->setCurrentItem( lv->firstChild() );
	    }
	    addRecentlyOpenedFile( name, recentFiles );
	} else {
	    statusBar()->clear();
	    QMessageBox::warning( this, tr("Qt Linguist"),
				  tr("Cannot open '%1'.").arg(name) );
	}
    }
}

void TrWindow::exitApp()
{
    if ( maybeSave() )
	close();
}

void TrWindow::open()
{
    if ( maybeSave() ) {
	QString newFilename = QFileDialog::getOpenFileName( filename,
				      tr("Qt translation source (*.ts)\n"
					 "All files (*)"), this );
	openFile( newFilename );
    }
}

void TrWindow::save()
{
    if ( filename.isEmpty() )
	return;

    if ( tor.save(filename) ) {
	dirty = FALSE;
	updateCaption();
	statusBar()->message( tr("File saved."), MessageMS );
    } else {
	QMessageBox::warning( this, tr("Qt Linguist"), tr("Cannot save '%1'.")
			      .arg(filename) );
    }
}

void TrWindow::saveAs()
{
    QString newFilename = QFileDialog::getSaveFileName( filename,
	    tr("Qt translation source (*.ts)\n"
	       "All files (*)") );
    if ( !newFilename.isEmpty() ) {
	filename = newFilename;
	save();
	updateCaption();
    }
}

void TrWindow::release()
{
    QString newFilename = filename;
    newFilename.replace( QRegExp(".ts$"), "" );
    newFilename += QString( ".qm" );

    newFilename = QFileDialog::getSaveFileName( newFilename,
	    tr("Qt message files for released applications (*.qm)\n"
	       "All files (*)") );
    if ( !newFilename.isEmpty() ) {
	if ( tor.release(newFilename) )
	    statusBar()->message( tr("File created."), MessageMS );
	else
	    QMessageBox::warning( this, tr("Qt Linguist"),
				  tr("Cannot save '%1'.").arg(newFilename) );
    }
}

void TrWindow::print()
{
    int pageNum = 0;

    if ( printer.setup(this) ) {
	QApplication::setOverrideCursor( WaitCursor );
	printer.setDocName( filename );
	statusBar()->message( tr("Printing...") );
	PrintOut pout( &printer );
	ContextLVI *c = (ContextLVI *) lv->firstChild();
	while ( c != 0 ) {
	    setCurrentContextItem( c );
	    pout.vskip();
	    pout.setRule( PrintOut::ThickRule );
	    pout.setGuide( c->context() );
	    pout.addBox( 100, tr("Context: %1").arg(c->context()),
			 PrintOut::Strong );
	    pout.flushLine();
	    pout.addBox( 4 );
	    pout.addBox( 92, c->comment(), PrintOut::Emphasis );
	    pout.flushLine();
	    pout.setRule( PrintOut::ThickRule );

	    MessageLVI *m = (MessageLVI *) slv->firstChild();
	    while ( m != 0 ) {
		pout.setRule( PrintOut::ThinRule );

		QString type;
		switch ( m->message().type() ) {
		case MetaTranslatorMessage::Finished:
		    type = tr( "finished" );
		    break;
		case MetaTranslatorMessage::Unfinished:
		    type = m->danger() ? tr( "(!)" ) : QString( "" );
		    break;
		case MetaTranslatorMessage::Obsolete:
		    type = tr( "obsolete" );
		    break;
		default:
		    type = QString( "" );
		}
		pout.addBox( 40, m->sourceText() );
		pout.addBox( 4 );
		pout.addBox( 40, m->translation() );
		pout.addBox( 4 );
		pout.addBox( 12, type, PrintOut::Normal, Qt::AlignRight );
		if ( !m->comment().isEmpty() ) {
		    pout.flushLine();
		    pout.addBox( 4 );
		    pout.addBox( 92, m->comment(), PrintOut::Emphasis );
		}
		pout.flushLine( TRUE );

		if ( pout.pageNum() != pageNum ) {
		    pageNum = pout.pageNum();
		    statusBar()->message( tr("Printing... (page %1)")
					  .arg(pageNum) );
		}
		m = (MessageLVI *) m->nextSibling();
	    }
	    c = (ContextLVI *) c->nextSibling();
	}
	pout.flushLine( TRUE );
	QApplication::restoreOverrideCursor();
	statusBar()->message( tr("Printing completed"), MessageMS );
    } else {
	statusBar()->message( tr("Printing aborted"), MessageMS );
    }
}

void TrWindow::find()
{
    h->hide();
    f->show();
    f->setActiveWindow();
    f->raise();
}

void TrWindow::findAgain()
{
    int pass = 0;
    int oldItemNo = itemToIndex( slv, slv->currentItem() );
    QListViewItem * j = foundScope;
    QListViewItem * k = indexToItem( slv, foundItem );
    QListViewItem * oldScope = lv->currentItem();

    if ( lv->childCount() == 0 )
	return;
#if 1
    /*
      As long as we don't implement highlighting of the text in the QTextView,
      we may have only one match per message.
    */
    foundOffset = (int) 0x7fffffff;
#else
    foundOffset++;
#endif

    slv->setUpdatesEnabled( FALSE );
    do {
	// Iterate through every item in all contexts
	if ( j == 0 ) {
	    j = lv->firstChild();
	    setCurrentContextItem( j );
	    if ( foundScope != 0 )
		statusBar()->message( tr("Search wrapped."), MessageMS );
	}
	if ( k == 0 )
	    k = slv->firstChild();

	while ( k ) {
	    MessageLVI * m = (MessageLVI *) k;
	    switch ( foundWhere ) {
		case 0:
		    foundWhere  = FindDialog::SourceText;
		    foundOffset = 0;
		    // fall-through
		case FindDialog::SourceText:
		    if ( searchItem( m->sourceText(), j, k ) )
			return;
		    foundWhere  = FindDialog::Translations;
		    foundOffset = 0;
		    // fall-through
		case FindDialog::Translations:
		    if ( searchItem( m->translation(), j, k ) )
			return;
		    foundWhere  = FindDialog::Comments;
		    foundOffset = 0;
		    // fall-through
		case FindDialog::Comments:
		    if ( searchItem( ((ContextLVI *) j)->fullContext(), j, k) )
			return;
		    foundWhere  = 0;
		    foundOffset = 0;
	    }
	    k = k->nextSibling();
	}

	j = j->nextSibling();
	if ( j ) {
	    setCurrentContextItem( j );
	    k = slv->firstChild();
	}
    } while ( pass++ != lv->childCount() );

    // This is just to keep the current scope and source text item
    // selected after a search failed.
    if ( oldScope ) {
	setCurrentContextItem( oldScope );
	QListViewItem * tmp = indexToItem( slv, oldItemNo );
	if( tmp )
	    setCurrentMessageItem( tmp );
    } else {
	if( lv->firstChild() )
	    setCurrentContextItem( lv->firstChild() );
	if( slv->firstChild() )
	    setCurrentMessageItem( slv->firstChild() );
    }

    slv->setUpdatesEnabled( TRUE );
    slv->triggerUpdate();
    statusBar()->message( tr("No match."), MessageMS );
    qApp->beep();
    foundItem   = 0;
    foundWhere  = 0;
    foundOffset = 0;
}

void TrWindow::replace()
{
    f->hide();
    h->show();
    h->setActiveWindow();
    h->raise();
}

int TrWindow::itemToIndex( QListView * view, QListViewItem * item )
{
    int no = 0;
    QListViewItem * tmp;

    if( view && item ){
	if( (tmp = view->firstChild()) != 0 )
	    do {
		no++;
		tmp = tmp->nextSibling();
	    } while( tmp && (tmp != item) );
    }
    return no;
}

QListViewItem * TrWindow::indexToItem( QListView * view, int index )
{
    QListViewItem * item = 0;

    if ( view && index > 0 ) {
	item = view->firstChild();
	while( item && index-- > 0 )
	    item = item->nextSibling();
    }
    return item;
}

bool TrWindow::searchItem( const QString & searchWhat, QListViewItem * j,
			   QListViewItem * k )
{
    if ( (findWhere & foundWhere) != 0 ) {
	foundOffset = searchWhat.find( findText, foundOffset, findMatchCase );
	if ( foundOffset >= 0 ) {
	    foundItem = itemToIndex( slv, k );
	    foundScope = j;
	    setCurrentMessageItem( k );
	    slv->setUpdatesEnabled( TRUE );
	    slv->triggerUpdate();
	    return TRUE;
	}
    }
    foundOffset = 0;
    return FALSE;
}

void TrWindow::newPhraseBook()
{
    QString name;
    while ( TRUE ) {
	name = QFileDialog::getSaveFileName( QString::null,
		       tr("Qt phrase books (*.qph)\n"
			  "All files (*)"), 0, "new_phrasebook",
					     tr("Create new phrase book") );
	if ( !QFile::exists(name) )
	    break;
	QMessageBox::warning( this, tr("Qt Linguist"),
			      tr("A file called '%1' already exists."
				 "  Please choose another name.").arg(name) );
    }
    if ( !name.isEmpty() ) {
	PhraseBook pb;
	if ( savePhraseBook(name, pb) ) {
	    if ( openPhraseBook(name) )
		statusBar()->message( tr("Phrase book created."), MessageMS );
	}
    }
}

void TrWindow::openPhraseBook()
{
    QString qtdirenv = getenv("QTDIR");
    QString name = QFileDialog::getOpenFileName( QString( qtdirenv + "/tools/linguist/phrasebooks" ),
	    tr("Qt phrase books (*.qph)\n"
	       "All files (*)"), 0, "open_phrasebook",
	       tr("Open phrase book") );

    if ( !name.isEmpty() && !phraseBookNames.contains(name) ) {
	if ( openPhraseBook(name) ) {
	    int n = phraseBooks.at( phraseBooks.count() - 1 )->count();
	    statusBar()->message( tr("%1 phrase(s) loaded.").arg(n),
				  MessageMS );
	}
    }
}

void TrWindow::closePhraseBook( int id )
{
    int index = closePhraseBookp->indexOf( id );
    phraseBooks.remove( index );
    phraseBookNames.remove( phraseBookNames.at(index) );
    updatePhraseDict();

    closePhraseBookp->removeItem( id );
    editPhraseBookp->removeItem( editPhraseBookp->idAt(index) );
    printPhraseBookp->removeItem( printPhraseBookp->idAt(index) );
}

void TrWindow::editPhraseBook( int id )
{
    int index = editPhraseBookp->indexOf( id );
    PhraseBookBox box( phraseBookNames[index], *phraseBooks.at(index), this,
		       "phrase book box", TRUE );
    box.setCaption( tr("%1 - %2").arg(tr("Qt Linguist"))
				 .arg(friendlyPhraseBookName(index)) );
    box.resize( 500, 300 );
    box.exec();
    *phraseBooks.at( index ) = box.phraseBook();
    updatePhraseDict();
}

void TrWindow::printPhraseBook( int id )
{
    int index = printPhraseBookp->indexOf( id );
    int pageNum = 0;

    if ( printer.setup(this) ) {
	printer.setDocName( phraseBookNames[index] );
	statusBar()->message( tr("Printing...") );
	PrintOut pout( &printer );
	PhraseBook *phraseBook = phraseBooks.at( index );
	PhraseBook::Iterator p;
	pout.setRule( PrintOut::ThinRule );
	for ( p = phraseBook->begin(); p != phraseBook->end(); ++p ) {
	    pout.setGuide( (*p).source() );
	    pout.addBox( 29, (*p).source() );
	    pout.addBox( 4 );
	    pout.addBox( 29, (*p).target() );
	    pout.addBox( 4 );
	    pout.addBox( 34, (*p).definition(), PrintOut::Emphasis );

	    if ( pout.pageNum() != pageNum ) {
		pageNum = pout.pageNum();
		statusBar()->message( tr("Printing... (page %1)")
				      .arg(pageNum) );
	    }
	    pout.setRule( PrintOut::NoRule );
	    pout.flushLine( TRUE );
	}
	pout.flushLine( TRUE );
	statusBar()->message( tr("Printing completed"), MessageMS );
    } else {
	statusBar()->message( tr("Printing aborted"), MessageMS );
    }
}

void TrWindow::revertSorting()
{
    lv->setSorting( 0 );
    slv->setSorting( 0 );
}

void TrWindow::manual()
{
    QStringList lst;
#ifdef Q_OS_MACX
    lst << QDir::cleanDirPath(QString(getenv("QTDIR")) + QDir::separator() +
			      "bin" + QDir::separator() +
			      "assistant.app/Contents/MacOS/assistant");
#else
    lst << "assistant";
#endif
    lst << "linguist-manual.html";
    QProcess proc( lst );
    proc.start();
}

void TrWindow::about()
{
    AboutDialog about( this, 0, TRUE );
    about.exec();
}

void TrWindow::aboutQt()
{
    QMessageBox::aboutQt( this, tr("Qt Linguist") );
}

void TrWindow::setupPhrase()
{
    bool enabled = !phraseBooks.isEmpty();
    phrasep->setItemEnabled( closePhraseBookId, enabled );
    phrasep->setItemEnabled( editPhraseBookId, enabled );
    phrasep->setItemEnabled( printPhraseBookId, enabled );
}

bool TrWindow::maybeSave()
{
    if ( dirty ) {
	switch ( QMessageBox::information(this, tr("Qt Linguist"),
				  tr("Do you want to save '%1'?")
				  .arg(filename),
				  QMessageBox::Yes | QMessageBox::Default,
				  QMessageBox::No,
				  QMessageBox::Cancel ) )
	{
	    case QMessageBox::Cancel:
		return FALSE;
	    case QMessageBox::Yes:
		save();
		break;
	    case QMessageBox::No:
		break;
	}
    }
    return TRUE;
}

void TrWindow::updateCaption()
{
    QString cap;
    bool enable = !filename.isEmpty();
    saveAct->setEnabled( enable );
    saveAsAct->setEnabled( enable );
    releaseAct->setEnabled( enable );
    printAct->setEnabled( enable );
    acceleratorsAct->setEnabled( enable );
    endingPunctuationAct->setEnabled( enable );
    phraseMatchesAct->setEnabled( enable );
    revertSortingAct->setEnabled( enable );

    if ( filename.isEmpty() )
	cap = tr( "Qt Linguist by Trolltech" );
    else
	cap = tr( "%1 - %2" ).arg( tr("Qt Linguist by Trolltech") )
			     .arg( filename );
    setCaption( cap );
    modified->setEnabled( dirty );
}

//
// New scope selected - build a new list of source text items
// for that scope.
//
void TrWindow::showNewScope( QListViewItem *item )
{
    static ContextLVI * oldContext = 0;

    if( item != 0 ) {
	ContextLVI * c = (ContextLVI *) item;
	bool upe = slv->isUpdatesEnabled();
	slv->setUpdatesEnabled( FALSE );
	slv->viewport()->setUpdatesEnabled( FALSE );
	if ( oldContext != 0 ) {
	    MessageLVI * tmp;
	    slv->blockSignals( TRUE );
	    while ( (tmp = (MessageLVI *) slv->firstChild()) != 0 )
		oldContext->appendMessageItem( slv, tmp );
	    slv->blockSignals( FALSE );
	}
	MessageLVI * tmp;
	while ( c->messageItemsInList() ) {
	    tmp = c->takeMessageItem( c->messageItemsInList() - 1);
	    slv->insertItem( tmp );
	    tmp->updateTranslationText();
	}
	
	slv->setUpdatesEnabled( upe );
	if( upe )
	    slv->triggerUpdate();
	oldContext = (ContextLVI *) item;
	statusBar()->clear();
    }
}

void TrWindow::showNewCurrent( QListViewItem *item )
{
    messageIsShown = (item != 0);
    MessageLVI *m = (MessageLVI *) item;
    ContextLVI *c = (ContextLVI *) m ? m->contextLVI() : 0;

    if ( messageIsShown ) {
	me->showMessage( m->sourceText(), m->comment(), c->fullContext(),
			 m->translation(), m->message().type(),
			 getPhrases(m->sourceText()) );
	if ( (m->message().type() != MetaTranslatorMessage::Finished) &&
	     m->danger() )
	    danger( m->sourceText(), m->translation(), TRUE );
	else
	    statusBar()->clear();

	doneAndNextAct->setEnabled( m->message().type() !=
				    MetaTranslatorMessage::Obsolete );
    } else {
	if ( item == 0 )
	    me->showNothing();
	else
	    me->showContext( c->fullContext(), c->finished() );
	doneAndNextAct->setEnabled( FALSE );
    }
    doneAndNextAlt->setEnabled( doneAndNextAct->isEnabled() );

    selectAllAct->setEnabled( messageIsShown );
}

void TrWindow::updateTranslation( const QString& translation )
{
    QListViewItem *item = slv->currentItem();
    if ( item != 0 ) {
	MessageLVI *m = (MessageLVI *) item;

	/*
          Remove trailing '\n's, as they were probably inserted by
          mistake. Don't dare to strip all white-space though, as some
          idioms require them. Whether these idioms are recommendable
          is beyond the scope of this comment.
	*/
	QString stripped = translation;
	while ( stripped.endsWith(QChar('\n')) )
            stripped.truncate( stripped.length() - 1 );

	if ( stripped != m->translation() ) {
	    bool dngr;
	    m->setTranslation( stripped );
	    if ( m->finished() &&
		 (dngr = danger( m->sourceText(), m->translation(), TRUE )) ) {
		numFinished -= 1;
		m->setDanger( dngr );
		m->setFinished( FALSE );
		m->contextLVI()->updateStatus();
		updateProgress();
	    }
	    tor.insert( m->message() );
	    if ( !dirty ) {
		dirty = TRUE;
		updateCaption();
	    }
	    m->updateTranslationText();
	}
    }
}

void TrWindow::updateFinished( bool finished )
{
    QListViewItem *item = slv->currentItem();
    if ( item != 0 ) {
	MessageLVI *m = (MessageLVI *) item;
	if ( finished != m->finished() ) {
	    numFinished += finished ? +1 : -1;
	    updateProgress();
	    m->setFinished( finished );
	    bool oldDanger = m->danger();
	    m->setDanger( /*m->finished() &&*/
			  danger(m->sourceText(), m->translation(),
			  !oldDanger) );
	    if ( !oldDanger && m->danger() )
		qApp->beep();
	    tor.insert( m->message() );
	    if ( !dirty ) {
		dirty = TRUE;
		updateCaption();
	    }
	}
    }
}

void TrWindow::doneAndNext()
{
    MessageLVI * m = (MessageLVI *) slv->currentItem();
    bool dngr = FALSE;

    if ( !m ) return;
    dngr = danger( m->sourceText(), m->translation(), TRUE );
    if ( !dngr ) {
	me->finishAndNext();
	m->contextLVI()->updateStatus();
    } else {
	if ( m->danger() != dngr )
	    m->setDanger( dngr );
	tor.insert( m->message() );
	if ( !dirty ) {
	    dirty = TRUE;
	    updateCaption();
	}
	qApp->beep();
    }
}

void TrWindow::toggleFinished( QListViewItem *item, const QPoint& /* p */,
			       int column )
{
    if ( item != 0 && column == 0 ) {
	MessageLVI *m = (MessageLVI *) item;
	bool dngr = FALSE;

	if ( m->message().type() == MetaTranslatorMessage::Unfinished ) {
	    dngr = danger( m->sourceText(), m->translation(), TRUE );
	}
	if ( !dngr && m->message().type() != MetaTranslatorMessage::Obsolete) {
	    setCurrentMessageItem( m );
	    me->setFinished( !m->finished() );
	    m->contextLVI()->updateStatus();
	} else {
	    bool oldDanger = m->danger();
	    m->setDanger( danger(m->sourceText(), m->translation(),
				 !oldDanger) );
	    if ( !oldDanger && m->danger() )
		qApp->beep();
	    tor.insert( m->message() );
	    if ( !dirty ) {
		dirty = TRUE;
		updateCaption();
	    }
	}
    }
}

void TrWindow::nextUnfinished()
{
    if ( nextUnfinishedAct->isEnabled() ) {
	// Select a message to translate, grab the first available if
	// there are no current selection.
	QListViewItem * cItem = lv->currentItem(); // context item
	QListViewItem * mItem = slv->currentItem(); // message item

	// Make sure an item is selected from both the context and the
	// message list.
	if( (mItem == 0) && !(mItem = slv->firstChild()) ) {
	    if( (cItem == 0) && !(cItem = lv->firstChild()) ) {
		statusBar()->message( tr("No phrase to translate."),
				      MessageMS );
		qApp->beep();
		return;
	    } else {
		showNewScope( cItem );
		while( cItem && !(mItem = slv->firstChild()) ) {
		    // no children in this node - try next one
		    cItem = cItem->nextSibling();
		    showNewScope( cItem );
		}
		setCurrentContextItem( cItem );
		if( mItem ) {
		    setCurrentMessageItem( cItem );
		} else {
		    statusBar()->message( tr("No phrase to translate."),
					  MessageMS );
		    qApp->beep();
		    return;
		}
	    }
	} else {
	    setCurrentMessageItem( mItem );
	}

	MessageLVI * m = (MessageLVI *) mItem;
	MessageLVI * n;
	ContextLVI * p = (ContextLVI *) cItem;
	ContextLVI * q;

	// Find the next Unfinished sibling within the same context.
	m = (MessageLVI *) mItem->nextSibling();
	n = m;
	do {
	    if ( n == 0 )
		break;
	    if ( n && !n->finished() && n != mItem ) {
		setCurrentMessageItem( n );
		return;
	    }
	    n = (MessageLVI *) n->nextSibling();
	} while ( n != m );

	// If all siblings are Finished or Obsolete, look in the first
	// Unfinished context.
	p = (ContextLVI *) p->nextSibling();
	q = p;
	do {
	    if ( q == 0 )
		q = (ContextLVI *) lv->firstChild();
	    if ( q && !q->finished() ) {
		showNewScope( q );
		setCurrentContextItem( q );
		n = (MessageLVI *) slv->firstChild();
		while ( n && n->finished() )
		    n = (MessageLVI *) n->nextSibling();
		if ( n && q ) {
		    setCurrentMessageItem( n );
		    return;
		}
	    }
	    q = (ContextLVI *) q->nextSibling();
	} while ( q != p );
    }

    // If no Unfinished message is left, the user has finished the job.  We
    // congratulate on a job well done with this ringing bell.
    statusBar()->message( tr("No untranslated phrases left."), MessageMS );
    qApp->beep();
}

static QListViewItem * lastChild( QListView * view )
{
    if ( view ) {
	QListViewItem * ret, * tmp;
	ret = view->firstChild();
	while ( ret ) {
	    tmp = ret->nextSibling();
	    if ( tmp == 0 )
		return ret;
	    ret = tmp;
	}
    }
    return 0;
}

void TrWindow::prevUnfinished()
{
    if ( prevUnfinishedAct->isEnabled() ) {
	// Select a message to translate, grab the first available if
	// there are no current selection.
	QListViewItem * cItem = lv->currentItem();  // context item
	QListViewItem * mItem = slv->currentItem(); // message item

	// Make sure an item is selected from both the context and the
	// message list.
	if( (mItem == 0) && !(mItem = slv->firstChild()) ) {
	    if( (cItem == 0) && !(cItem = lv->firstChild()) ) {
		statusBar()->message( tr("No phrase to translate."),
				      MessageMS );
		qApp->beep();
		return;
	    } else {
		showNewScope( cItem );
		while( cItem && !(mItem = slv->firstChild()) ) {
		    // no children in this node - try next one
		    cItem = cItem->nextSibling();
		    showNewScope( cItem );
		}
		setCurrentContextItem( cItem );
		if( mItem ) {
		    setCurrentMessageItem( cItem );
		} else {
		    statusBar()->message( tr("No phrase to translate."),
					  MessageMS );
		    qApp->beep();
		    return;
		}
	    }
	} else {
	    setCurrentMessageItem( mItem );
	}

	MessageLVI * m = (MessageLVI *) mItem;
	MessageLVI * n;
	ContextLVI * p = (ContextLVI *) cItem;
	ContextLVI * q;

	// Find the next Unfinished sibling within the same context.
	n = m;
	do {
	    n = (MessageLVI * ) n->itemAbove();
	    if ( n == 0 )
		break;
	    if ( n && !n->finished() ) {
		setCurrentMessageItem( n );
		return;
	    }
	} while ( !((ContextLVI *) cItem)->finished() && n != 0 );

	// If all siblings are Finished or Obsolete, look in the prev
	// Unfinished context.
	q = p;
	do {
	    q = (ContextLVI *) q->itemAbove();
	    if ( q == 0 )
		q = (ContextLVI *) lastChild( lv );
	    if ( q && !q->finished() ) {
		showNewScope( q );
		setCurrentContextItem( q );
		n = (MessageLVI *) lastChild( slv );
		while ( n && n->finished() )
		    n = (MessageLVI *) n->itemAbove();
		if ( n && q ) {
		    setCurrentMessageItem( n );
		    return;
		}
	    }
	} while ( q != 0 );
    }
    statusBar()->message( tr("No untranslated phrases left."), MessageMS );
    qApp->beep();
}

void TrWindow::prev()
{
    QListViewItem * cItem = lv->currentItem();  // context item
    QListViewItem * mItem = slv->currentItem(); // message item
    QListViewItem * tmp;

    if ( !cItem ) {
	cItem = lv->firstChild();
	if ( !cItem ) return;
	setCurrentContextItem( cItem );
    }

    if ( !mItem ) {
	mItem = lastChild( slv );
	if ( !mItem ) return;
	setCurrentMessageItem( mItem );
    } else {
	if ( (tmp = mItem->itemAbove()) != 0 ) {
	    setCurrentMessageItem( tmp );
	    return;
	} else {
	    if ( (tmp = cItem->itemAbove()) == 0 ) {
		tmp = lastChild( lv );
	    }
	    if ( !tmp ) return;
	    setCurrentContextItem( tmp );
	    setCurrentMessageItem( lastChild( slv ) );
	}
    }
}

void TrWindow::next()
{
    QListViewItem * cItem = lv->currentItem();  // context item
    QListViewItem * mItem = slv->currentItem(); // message item
    QListViewItem * tmp;

    if ( !cItem ) {
	cItem = lv->firstChild();
	if ( !cItem ) return;
	setCurrentContextItem( cItem );
    }

    if ( !mItem ) {
	mItem = slv->firstChild();
	if ( !mItem ) return;
	setCurrentMessageItem( mItem );
    } else {
	if ( (tmp = mItem->nextSibling()) != 0 ) {
	    setCurrentMessageItem( tmp );
	    return;
	} else {
	    if ( (tmp = cItem->nextSibling()) == 0 ) {
		tmp = lv->firstChild();
	    }
	    if ( !tmp ) return;
	    setCurrentContextItem( tmp );
	    setCurrentMessageItem( slv->firstChild() );
	}
    }
}


void TrWindow::findNext( const QString& text, int where, bool matchCase )
{
    findText = text;
    if ( findText.isEmpty() )
	findText = QString( "magicwordthatyoushouldavoid" );
    findWhere = where;
    findMatchCase = matchCase;
    findAgainAct->setEnabled( TRUE );
    findAgain();
}

void TrWindow::revalidate()
{
    ContextLVI *c = (ContextLVI *) lv->firstChild();
    QListViewItem * oldScope = lv->currentItem();
    int oldItemNo = itemToIndex( slv, slv->currentItem() );
    slv->setUpdatesEnabled( FALSE );

    while ( c != 0 ) {
	showNewScope( c );
	MessageLVI *m = (MessageLVI *) slv->firstChild();
	while ( m != 0 ) {
	    m->setDanger( danger(m->sourceText(), m->translation()) &&
		    m->message().type() == MetaTranslatorMessage::Finished );
	    m = (MessageLVI *) m->nextSibling();
	}
	c = (ContextLVI *) c->nextSibling();
    }

    if ( oldScope ){
	showNewScope( oldScope );
	QListViewItem * tmp = indexToItem( slv, oldItemNo );
	if( tmp )
	    setCurrentMessageItem( tmp );
    }
    slv->setUpdatesEnabled( TRUE );
    slv->triggerUpdate();
}

void TrWindow::setupImageDict()
{
    (void) qembed_findData;

    if ( imageDict == 0 ) {
	imageDict = new QDict<Embed>( 101 );
	Embed *em;
	for ( em = embed_vec; em->size > 0; em++ ) {
	    QString name = em->name;
	    int k = name.findRev( QChar('.') );
	    if ( k != -1 )
		name.truncate( k );
	    imageDict->insert( name, em );
	}

	// Create the application global listview symbols
	pxOn  = new QPixmap;
	pxOff = new QPixmap;
	pxObsolete = new QPixmap;
	pxDanger = new QPixmap;

	em = imageDict->find( QString("symbols/check_on") );
	pxOn->loadFromData( em->data, em->size );
	em = imageDict->find( QString("symbols/check_off") );
	pxOff->loadFromData( em->data, em->size );
	em = imageDict->find( QString("symbols/check_obs") );
	pxObsolete->loadFromData( em->data, em->size );
	em = imageDict->find( QString("symbols/check_danger") );
	pxDanger->loadFromData( em->data, em->size );

	QBitmap onMask( check_on_mask_width, check_on_mask_height,
			check_on_mask_bits, TRUE );
	QBitmap offMask( check_off_mask_width, check_off_mask_height,
			 check_off_mask_bits, TRUE );
	QBitmap dangerMask( check_danger_mask_width, check_danger_mask_height,
			    check_danger_mask_bits, TRUE );
	pxOn->setMask( onMask );
	pxOff->setMask( offMask );
	pxObsolete->setMask( onMask );
	pxDanger->setMask( dangerMask );
    }
}

QString TrWindow::friendlyString( const QString& str )
{
    QString f = str.lower();
    f.replace( QRegExp(QString("[.,:;!?()-]")), QString(" ") );
    f.replace( QRegExp(QString("&")), QString("") );
    f = f.simplifyWhiteSpace();
    f = f.lower();
    return f;
}

void TrWindow::setupMenuBar()
{
    QMenuBar * m = menuBar();
    QPopupMenu * filep = new QPopupMenu( this );
    QPopupMenu * editp  = new QPopupMenu( this );
    QPopupMenu * translationp = new QPopupMenu( this );
    QPopupMenu * validationp = new QPopupMenu( this );
    validationp->setCheckable( TRUE );
    phrasep = new QPopupMenu( this );
    closePhraseBookp = new QPopupMenu( this );
    editPhraseBookp = new QPopupMenu( this );
    printPhraseBookp = new QPopupMenu( this );
    QPopupMenu * viewp = new QPopupMenu( this );
    viewp->setCheckable( TRUE );
    QPopupMenu * helpp = new QPopupMenu( this );

    m->insertItem( tr("&File"), filep );
    m->insertItem( tr("&Edit"), editp );
    m->insertItem( tr("&Translation"), translationp );
    m->insertItem( tr("V&alidation"), validationp );
    m->insertItem( tr("&Phrases"), phrasep );
    m->insertItem( tr("&View"), viewp );
    m->insertSeparator();
    m->insertItem( tr("&Help"), helpp );

    connect( closePhraseBookp, SIGNAL(activated(int)),
	     this, SLOT(closePhraseBook(int)) );
    connect( editPhraseBookp, SIGNAL(activated(int)),
	     this, SLOT(editPhraseBook(int)) );
    connect( printPhraseBookp, SIGNAL(activated(int)),
	     this, SLOT(printPhraseBook(int)) );
    // File menu
    openAct = new Action( filep, tr("&Open..."), this, SLOT(open()),
			  QAccel::stringToKey(tr("Ctrl+O")) );

    filep->insertSeparator();

    saveAct = new Action( filep, tr("&Save"), this, SLOT(save()),
			  QAccel::stringToKey(tr("Ctrl+S")) );
    saveAsAct = new Action( filep, tr("Save &As..."), this, SLOT(saveAs()) );
    releaseAct = new Action( filep, tr("&Release..."), this, SLOT(release()) );
    filep->insertSeparator();
    printAct = new Action( filep, tr("&Print..."), this, SLOT(print()),
			   QAccel::stringToKey(tr("Ctrl+P")) );

    filep->insertSeparator();

    recentFilesMenu = new QPopupMenu( this );
    filep->insertItem( tr("Re&cently opened files"), recentFilesMenu );
    connect( recentFilesMenu, SIGNAL(aboutToShow()), this,
	     SLOT(setupRecentFilesMenu()) );
    connect( recentFilesMenu, SIGNAL(activated( int )), this,
	     SLOT(recentFileActivated( int )) );

    filep->insertSeparator();

    exitAct = new Action( filep, tr("E&xit"), this, SLOT(exitApp()),
			  QAccel::stringToKey(tr("Ctrl+Q")) );
    // Edit menu
    undoAct = new Action( editp, tr("&Undo"), me, SLOT(undo()),
			  QAccel::stringToKey(tr("Ctrl+Z")) );
    undoAct->setEnabled( FALSE );
    connect( me, SIGNAL(undoAvailable(bool)), undoAct, SLOT(setEnabled(bool)) );
    redoAct = new Action( editp, tr("&Redo"), me, SLOT(redo()),
			  QAccel::stringToKey(tr("Ctrl+Y")) );
    redoAct->setEnabled( FALSE );
    connect( me, SIGNAL(redoAvailable(bool)), redoAct, SLOT(setEnabled(bool)) );
    editp->insertSeparator();
    cutAct = new Action( editp, tr("Cu&t"), me, SLOT(cut()),
			 QAccel::stringToKey(tr("Ctrl+X")) );
    cutAct->setEnabled( FALSE );
    connect( me, SIGNAL(cutAvailable(bool)), cutAct, SLOT(setEnabled(bool)) );
    copyAct = new Action( editp, tr("&Copy"), me, SLOT(copy()),
			  QAccel::stringToKey(tr("Ctrl+C")) );
    copyAct->setEnabled( FALSE );
    connect( me, SIGNAL(copyAvailable(bool)), copyAct, SLOT(setEnabled(bool)) );
    pasteAct = new Action( editp, tr("&Paste"), me, SLOT(paste()),
			   QAccel::stringToKey(tr("Ctrl+V")) );
    pasteAct->setEnabled( FALSE );
    connect( me, SIGNAL(pasteAvailable(bool)),
	     pasteAct, SLOT(setEnabled(bool)) );
    selectAllAct = new Action( editp, tr("Select &All"), me, SLOT(selectAll()),
			       QAccel::stringToKey(tr("Ctrl+A")) );
    selectAllAct->setEnabled( FALSE );
    editp->insertSeparator();
    findAct = new Action( editp, tr("&Find..."), this, SLOT(find()),
			  QAccel::stringToKey(tr("Ctrl+F")) );
    findAct->setEnabled( FALSE );
    findAgainAct = new Action( editp, tr("Find &Next"),
			       this, SLOT(findAgain()), Key_F3 );
    findAgainAct->setEnabled( FALSE );
#ifdef notyet
    replaceAct = new Action( editp, tr("&Replace..."), this, SLOT(replace()),
			     QAccel::stringToKey(tr("Ctrl+H")) );
    replaceAct->setEnabled( FALSE );
#endif

    // Translation menu
    // when updating the accelerators, remember the status bar
    prevUnfinishedAct = new Action( translationp, tr("&Prev Unfinished"),
				    this, SLOT(prevUnfinished()),
				    QAccel::stringToKey(tr("Ctrl+K")) );
    nextUnfinishedAct = new Action( translationp, tr("&Next Unfinished"),
				    this, SLOT(nextUnfinished()),
				    QAccel::stringToKey(tr("Ctrl+L")) );

    prevAct = new Action( translationp, tr("P&rev"),
			  this, SLOT(prev()),
			  QAccel::stringToKey(tr("Ctrl+Shift+K")) );
    nextAct = new Action( translationp, tr("Ne&xt"),
			  this, SLOT(next()),
			  QAccel::stringToKey(tr("Ctrl+Shift+L")) );
    doneAndNextAct = new Action( translationp, tr("Done and &Next"),
				 this, SLOT(doneAndNext()),
				 QAccel::stringToKey(tr("Ctrl+Enter")) );
    doneAndNextAlt = new QAction( this );
    doneAndNextAlt->setAccel( QAccel::stringToKey(tr("Ctrl+Return")) );
    connect( doneAndNextAlt, SIGNAL(activated()), this, SLOT(doneAndNext()) );
    beginFromSourceAct = new Action( translationp, tr("&Begin from Source"),
				     me, SLOT(beginFromSource()),
				     QAccel::stringToKey(tr("Ctrl+B")) );
    connect( me, SIGNAL(updateActions(bool)), beginFromSourceAct,
	     SLOT(setEnabled(bool)) );

    // Phrasebook menu
    newPhraseBookAct = new Action( phrasep, tr("&New Phrase Book..."),
				   this, SLOT(newPhraseBook()),
				   QAccel::stringToKey(tr("Ctrl+N")));
    openPhraseBookAct = new Action( phrasep, tr("&Open Phrase Book..."),
				    this, SLOT(openPhraseBook()),
				    QAccel::stringToKey(tr("Ctrl+H")) );
    closePhraseBookId = phrasep->insertItem( tr("&Close Phrase Book"),
					     closePhraseBookp );
    phrasep->insertSeparator();
    editPhraseBookId = phrasep->insertItem( tr("&Edit Phrase Book..."),
					    editPhraseBookp );
    printPhraseBookId = phrasep->insertItem( tr("&Print Phrase Book..."),
					     printPhraseBookp );
    connect( phrasep, SIGNAL(aboutToShow()), this, SLOT(setupPhrase()) );

    // Validation menu
    acceleratorsAct = new Action( validationp, tr("&Accelerators"),
				  this, SLOT(revalidate()), 0, TRUE );
    acceleratorsAct->setOn( TRUE );
    endingPunctuationAct = new Action( validationp, tr("&Ending Punctuation"),
				       this, SLOT(revalidate()), 0, TRUE );
    endingPunctuationAct->setOn( TRUE );
    phraseMatchesAct = new Action( validationp, tr("&Phrase Matches"),
				   this, SLOT(revalidate()), 0, TRUE );
    phraseMatchesAct->setOn( TRUE );

    // View menu
    revertSortingAct = new Action( viewp, tr("&Revert Sorting"),
				   this, SLOT(revertSorting()) );
    doGuessesAct = new Action( viewp, tr("&Display guesses"),
			       this, SLOT(toggleGuessing()) );
    doGuessesAct->setToggleAction( TRUE );
    doGuessesAct->setOn( TRUE );
    viewp->insertSeparator();
    viewp->insertItem( tr("Vie&ws"), createDockWindowMenu( NoToolBars ) );
    viewp->insertItem( tr("&Toolbars"), createDockWindowMenu( OnlyToolBars ) );

    // Help
    manualAct = new Action( helpp, tr("&Manual..."), this, SLOT(manual()),
			    Key_F1 );
    helpp->insertSeparator();
    aboutAct = new Action( helpp, tr("&About..."), this, SLOT(about()) );
    aboutQtAct = new Action( helpp, tr("About &Qt..."), this, SLOT(aboutQt()) );
    helpp->insertSeparator();
    whatsThisAct = new Action( helpp, tr("&What's This?"),
			       this, SLOT(whatsThis()), SHIFT + Key_F1 );

    openAct->setWhatsThis( tr("Open a Qt translation source file (TS file) for"
			      " editing.") );
    saveAct->setWhatsThis( tr("Save changes made to this Qt translation "
				"source file.") );
    saveAsAct->setWhatsThis( tr("Save changes made to this Qt translation"
				"source file into a new file.") );
    releaseAct->setWhatsThis( tr("Create a Qt message file suitable for"
				 " released applications"
				 " from the current message file.") );
    printAct->setWhatsThis( tr("Print a list of all the phrases in the current"
			       " Qt translation source file.") );
    exitAct->setWhatsThis( tr("Close this window and exit.") );

    undoAct->setWhatsThis( tr("Undo the last editing operation performed on the"
			      " translation.") );
    redoAct->setWhatsThis( tr("Redo an undone editing operation performed on"
			      " the translation.") );
    cutAct->setWhatsThis( tr("Copy the selected translation text to the"
			     " clipboard and deletes it.") );
    copyAct->setWhatsThis( tr("Copy the selected translation text to the"
			      " clipboard.") );
    pasteAct->setWhatsThis( tr("Paste the clipboard text into the"
			       " translation.") );
    selectAllAct->setWhatsThis( tr("Select the whole translation text.") );
    findAct->setWhatsThis( tr("Search for some text in the translation "
				"source file.") );
    findAgainAct->setWhatsThis( tr("Continue the search where it was left.") );
#ifdef notyet
    replaceAct->setWhatsThis( tr("Search for some text in the translation"
				 " source file and replace it by another"
				 " text.") );
#endif

    newPhraseBookAct->setWhatsThis( tr("Create a new phrase book.") );
    openPhraseBookAct->setWhatsThis( tr("Open a phrase book to assist"
					" translation.") );
    acceleratorsAct->setWhatsThis( tr("Toggle validity checks of"
				      " accelerators.") );
    endingPunctuationAct->setWhatsThis( tr("Toggle validity checks"
					   " of ending punctuation.") );
    phraseMatchesAct->setWhatsThis( tr("Toggle checking that phrase"
				       " suggestions are used.") );

    revertSortingAct->setWhatsThis( tr("Sort the items back in the same order"
				       " as in the message file.") );

    doGuessesAct->setWhatsThis( tr("Set whether or not to display translation guesses.") );
    manualAct->setWhatsThis( tr("Display the manual for %1.")
			       .arg(tr("Qt Linguist")) );
    aboutAct->setWhatsThis( tr("Display information about %1.")
			    .arg(tr("Qt Linguist")) );
    aboutQtAct->setWhatsThis( tr("Display information about the Qt toolkit by"
				 " Trolltech.") );
    whatsThisAct->setWhatsThis( tr("Enter What's This? mode.") );

    beginFromSourceAct->setWhatsThis( tr("Copies the source text into"
					 " the translation field.") );
    nextAct->setWhatsThis( tr("Moves to the next item.") );
    prevAct->setWhatsThis( tr("Moves to the previous item.") );
    nextUnfinishedAct->setWhatsThis( tr("Moves to the next unfinished item.") );
    prevUnfinishedAct->setWhatsThis( tr("Moves to the previous unfinished item.") );
    doneAndNextAct->setWhatsThis( tr("Marks this item as done and moves to the"
				     " next unfinished item.") );
    doneAndNextAlt->setWhatsThis( doneAndNextAct->whatsThis() );
}

void TrWindow::setupToolBars()
{
    QToolBar *filet = new QToolBar( tr("File"), this );
    QToolBar *editt = new QToolBar( tr("Edit"), this );
    QToolBar *translationst = new QToolBar( tr("Translation"), this );
    QToolBar *validationt   = new QToolBar( tr("Validation"), this );
    QToolBar *helpt = new QToolBar( tr("Help"), this );

    openAct->addToToolbar( filet, tr("Open"), "fileopen" );
    saveAct->addToToolbar( filet, tr("Save"), "filesave" );
    printAct->addToToolbar( filet, tr("Print"), "print" );
    filet->addSeparator();
    openPhraseBookAct->addToToolbar( filet, tr("Open Phrase Book"), "book" );

    undoAct->addToToolbar( editt, tr("Undo"), "undo" );
    redoAct->addToToolbar( editt, tr("Redo"), "redo" );
    editt->addSeparator();
    cutAct->addToToolbar( editt, tr("Cut"), "editcut" );
    copyAct->addToToolbar( editt, tr("Copy"), "editcopy" );
    pasteAct->addToToolbar( editt, tr("Paste"), "editpaste" );
    editt->addSeparator();
    findAct->addToToolbar( editt, tr("Find"), "searchfind" );
#ifdef notyet
    replaceAct->addToToolbar( editt, tr("Replace"), "replace" );
#endif

    // beginFromSourceAct->addToToolbar( translationst,
    //                                tr("Begin from Source"), "searchfind" );
    prevAct->addToToolbar( translationst, tr("Prev"), "prev" );
    nextAct->addToToolbar( translationst, tr("Next"), "next" );
    prevUnfinishedAct->addToToolbar( translationst, tr("Prev Unfinished"),
				     "prevunfinished" );
    nextUnfinishedAct->addToToolbar( translationst, tr("Next Unfinished"),
				     "nextunfinished" );
    doneAndNextAct->addToToolbar( translationst, tr("Done and Next"),
				  "doneandnext" );

    acceleratorsAct->addToToolbar( validationt, tr("Accelerators"), "accel" );
    endingPunctuationAct->addToToolbar( validationt, tr("Punctuation"),
					"endpunct" );
    phraseMatchesAct->addToToolbar( validationt, tr("Phrases"), "phrase" );

    whatsThisAct->addToToolbar( helpt, tr("What's This?"), "whatsthis" );
}

void TrWindow::setCurrentContextItem( QListViewItem *item )
{
    lv->ensureItemVisible( item );
    lv->setSelected( item, TRUE );
}

void TrWindow::setCurrentMessageItem( QListViewItem *item )
{
    slv->ensureItemVisible( item );
    slv->setSelected( item, TRUE );
}

QString TrWindow::friendlyPhraseBookName( int k )
{
    return QFileInfo( phraseBookNames[k] ).fileName();
}

bool TrWindow::openPhraseBook( const QString& name )
{
    PhraseBook *pb = new PhraseBook;
    if ( !pb->load(name) ) {
	QMessageBox::warning( this, tr("Qt Linguist"),
			      tr("Cannot read from phrase book '%1'.")
			      .arg(name) );
	return FALSE;
    }

    int index = (int) phraseBooks.count();
    phraseBooks.append( pb );
    phraseBookNames.append( name );
    int id = closePhraseBookp->insertItem( friendlyPhraseBookName(index) );
    closePhraseBookp->setWhatsThis( id, tr("Close this phrase book.") );
    id = editPhraseBookp->insertItem( friendlyPhraseBookName(index) );
    editPhraseBookp->setWhatsThis( id, tr("Allow you to add, modify, or delete"
					  " phrases of this phrase book.") );
    id = printPhraseBookp->insertItem( friendlyPhraseBookName(index) );
    printPhraseBookp->setWhatsThis( id, tr("Print the entries of the phrase"
					   " book.") );
    updatePhraseDict();
    return TRUE;
}

bool TrWindow::savePhraseBook( QString& name, const PhraseBook& pb )
{
    if ( !name.contains( ".qph" ) && !name.contains(".") )
	name += ".qph";

    if ( !pb.save(name) ) {
	QMessageBox::warning( this, tr("Qt Linguist"),
			      tr("Cannot create phrase book '%1'.")
			      .arg(name) );
	return FALSE;
    }
    return TRUE;
}

void TrWindow::updateProgress()
{
    if ( numNonobsolete == 0 )
	progress->setText( QString("    " "    ") );
    else
	progress->setText( QString(" %1/%2 ").arg(numFinished)
			   .arg(numNonobsolete) );
    prevUnfinishedAct->setEnabled( numFinished != numNonobsolete );
    nextUnfinishedAct->setEnabled( numFinished != numNonobsolete );
    prevAct->setEnabled( lv->firstChild() != 0 );
    nextAct->setEnabled( lv->firstChild() != 0 );
}

void TrWindow::updatePhraseDict()
{
    QPtrListIterator<PhraseBook> pb = phraseBooks;
    PhraseBook::Iterator p;
    PhraseBook *ent;
    phraseDict.clear();
    while ( pb.current() != 0 ) {
	for ( p = (*pb)->begin(); p != (*pb)->end(); ++p ) {
	    QString f = friendlyString( (*p).source() );
	    if ( f.length() > 0 ) {
		f = QStringList::split( QChar(' '), f ).first();
		ent = phraseDict.find( f );
		if ( ent == 0 ) {
		    ent = new PhraseBook;
		    phraseDict.insert( f, ent );
		}
		ent->append( *p );
	    }
	}
	++pb;
    }
    revalidate();
}

PhraseBook TrWindow::getPhrases( const QString& source )
{
    PhraseBook phrases;
    QString f = friendlyString( source );
    QStringList lookupWords = QStringList::split( QChar(' '), f );
    QStringList::Iterator w;
    PhraseBook::Iterator p;

    for ( w = lookupWords.begin(); w != lookupWords.end(); ++w ) {
	PhraseBook *ent = phraseDict.find( *w );
	if ( ent != 0 ) {
	    for ( p = ent->begin(); p != ent->end(); ++p ) {
		if ( f.find(friendlyString((*p).source())) >= 0 )
		    phrases.append( *p );
	    }
	}
    }
    return phrases;
}

bool TrWindow::danger( const QString& source, const QString& translation,
		       bool verbose )
{
    if ( acceleratorsAct->isOn() ) {
	int sk = QAccel::shortcutKey( source );
	int tk = QAccel::shortcutKey( translation );
	if ( sk == 0 && tk != 0 ) {
	    if ( verbose )
		statusBar()->message( tr("Accelerator possibly superfluous in"
					 " translation."), ErrorMS );
	    return TRUE;
	} else if ( sk != 0 && tk == 0 ) {
	    if ( verbose )
		statusBar()->message( tr("Accelerator possibly missing in"
					 " translation."), ErrorMS );
	    return TRUE;
	}
    }
    if ( endingPunctuationAct->isOn() ) {
	if ( ending(source) != ending(translation) ) {
	    if ( verbose )
		statusBar()->message( tr("Translation does not end with the"
					 " same punctuation as the source"
					 " text."), ErrorMS );
	    return TRUE;
	}
    }
    if ( phraseMatchesAct->isOn() ) {
	QString fsource = friendlyString( source );
	QString ftranslation = friendlyString( translation );
	QStringList lookupWords = QStringList::split( QChar(' '), fsource );
	QStringList::Iterator w;
	PhraseBook::Iterator p;

	for ( w = lookupWords.begin(); w != lookupWords.end(); ++w ) {
	    PhraseBook *ent = phraseDict.find( *w );
	    if ( ent != 0 ) {
		for ( p = ent->begin(); p != ent->end(); ++p ) {
		    if ( fsource.find(friendlyString((*p).source())) < 0 ||
			 ftranslation.find(friendlyString((*p).target())) >= 0 )
			break;
		}
		if ( p == ent->end() ) {
		    if ( verbose )
			statusBar()->message( tr("A phrase book suggestion for"
						 " '%1' was ignored.")
						 .arg(*w), ErrorMS );
		    return TRUE;
		}
	    }
	}
    }
    if ( verbose )
	statusBar()->clear();

    return FALSE;
}

void TrWindow::readConfig()
{
    QString   keybase("/Qt Linguist/3.0/");
    QSettings config;

    config.insertSearchPath( QSettings::Windows, "/Trolltech" );

    QRect r( pos(), size() );
    recentFiles = config.readListEntry( keybase + "RecentlyOpenedFiles", ',' );
    if ( !config.readBoolEntry( keybase + "Geometry/MainwindowMaximized", FALSE ) ) {
	r.setX( config.readNumEntry( keybase + "Geometry/MainwindowX", r.x() ) );
	r.setY( config.readNumEntry( keybase + "Geometry/MainwindowY", r.y() ) );
	r.setWidth( config.readNumEntry( keybase + "Geometry/MainwindowWidth", r.width() ) );
	r.setHeight( config.readNumEntry( keybase + "Geometry/MainwindowHeight", r.height() ) );

	QRect desk = QApplication::desktop()->geometry();
	QRect inter = desk.intersect( r );
	resize( r.size() );
	if ( inter.width() * inter.height() > ( r.width() * r.height() / 20 ) ) {
	    move( r.topLeft() );
	}
    }

    QDockWindow * dw;
    dw = (QDockWindow *) lv->parent();
    int place;
    place = config.readNumEntry( keybase + "Geometry/ContextwindowInDock" );
    r.setX( config.readNumEntry( keybase + "Geometry/ContextwindowX" ) );
    r.setY( config.readNumEntry( keybase + "Geometry/ContextwindowY" ) );
    r.setWidth( config.readNumEntry( keybase +
				     "Geometry/ContextwindowWidth" ) );
    r.setHeight( config.readNumEntry( keybase +
				      "Geometry/ContextwindowHeight" ) );
    if ( place == QDockWindow::OutsideDock )
	dw->undock();
    dw->setGeometry( r );

    dw = (QDockWindow *) slv->parent();
    place = config.readNumEntry( keybase + "Geometry/ContextwindowInDock" );
    r.setX( config.readNumEntry( keybase + "Geometry/SourcewindowX" ) );
    r.setY( config.readNumEntry( keybase + "Geometry/SourcewindowY" ) );
    r.setWidth( config.readNumEntry( keybase +
				     "Geometry/SourcewindowWidth" ) );
    r.setHeight( config.readNumEntry( keybase +
				      "Geometry/SourcewindowHeight" ) );
    if ( place == QDockWindow::OutsideDock )
	dw->undock();
    dw->setGeometry( r );

    dw = (QDockWindow *) plv->parent()->parent();
    place = config.readNumEntry( keybase + "Geometry/PhrasewindowInDock" );
    r.setX( config.readNumEntry( keybase + "Geometry/PhrasewindowX" ) );
    r.setY( config.readNumEntry( keybase + "Geometry/PhrasewindowY" ) );
    r.setWidth( config.readNumEntry( keybase +
				     "Geometry/PhrasewindowWidth" ) );
    r.setHeight( config.readNumEntry( keybase +
				      "Geometry/PhrasewindowHeight" ) );
    if ( place == QDockWindow::OutsideDock )
	dw->undock();
    dw->setGeometry( r );
    QApplication::sendPostedEvents();
}

void TrWindow::writeConfig()
{
    QString   keybase("/Qt Linguist/3.0/");
    QSettings config;

    config.insertSearchPath( QSettings::Windows, "/Trolltech" );
    config.writeEntry( keybase + "RecentlyOpenedFiles", recentFiles, ',' );
    config.writeEntry( keybase + "Geometries/MainwindowMaximized", isMaximized() );
    config.writeEntry( keybase + "Geometry/MainwindowX", x() );
    config.writeEntry( keybase + "Geometry/MainwindowY", y() );
    config.writeEntry( keybase + "Geometry/MainwindowWidth", width() );
    config.writeEntry( keybase + "Geometry/MainwindowHeight", height() );

    QDockWindow * dw =(QDockWindow *) lv->parent();
    config.writeEntry( keybase + "Geometry/ContextwindowInDock", dw->place() );
    config.writeEntry( keybase + "Geometry/ContextwindowX", dw->x() );
    config.writeEntry( keybase + "Geometry/ContextwindowY", dw->y() );
    config.writeEntry( keybase + "Geometry/ContextwindowWidth", dw->width() );
    config.writeEntry( keybase + "Geometry/ContextwindowHeight", dw->height() );

    dw =(QDockWindow *) slv->parent();
    config.writeEntry( keybase + "Geometry/SourcewindowInDock",
		       dw->place() );
    config.writeEntry( keybase + "Geometry/SourcewindowX", dw->geometry().x() );
    config.writeEntry( keybase + "Geometry/SourcewindowY", dw->geometry().y() );
    config.writeEntry( keybase + "Geometry/SourcewindowWidth", dw->width() );
    config.writeEntry( keybase + "Geometry/SourcewindowHeight", dw->height() );

    dw =(QDockWindow *) plv->parent()->parent();
    config.writeEntry( keybase + "Geometry/PhrasewindowInDock",
		       dw->place() );
    config.writeEntry( keybase + "Geometry/PhrasewindowX", dw->geometry().x() );
    config.writeEntry( keybase + "Geometry/PhrasewindowY", dw->geometry().y() );
    config.writeEntry( keybase + "Geometry/PhrasewindowWidth", dw->width() );
    config.writeEntry( keybase + "Geometry/PhrasewindowHeight", dw->height() );
}

void TrWindow::setupRecentFilesMenu()
{
    recentFilesMenu->clear();
    int id = 0;
    QStringList::Iterator it = recentFiles.begin();
    for ( ; it != recentFiles.end(); ++it )
    {
	recentFilesMenu->insertItem( *it, id );
	id++;
    }
}

void TrWindow::recentFileActivated( int id )
{
    if ( id != -1 ) {
	if ( maybeSave() )
	    openFile( *recentFiles.at( id ) );
    }
}

void TrWindow::addRecentlyOpenedFile( const QString &fn, QStringList &lst )
{
    if ( lst.find( fn ) != lst.end() )
	return;
    if ( lst.count() >= 10 )
	lst.remove( lst.begin() );
    lst << fn;
}

void TrWindow::toggleGuessing()
{
    me->toggleGuessing();
}

void TrWindow::focusSourceList()
{
    slv->setFocus();
}

void TrWindow::focusPhraseList()
{
    plv->setFocus();
}
