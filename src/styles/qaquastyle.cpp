/****************************************************************************
**
** Implementation of Aqua-like style class
**
** Created : 001129
**
** Copyright (C) 1998-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
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
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
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

#include "qaquastyle.h"

#ifndef QT_NO_STYLE_AQUA

/* Mask for Aqua buttons - left */
#define aqua_btn_mask_left_xbm_width 13
#define aqua_btn_mask_left_xbm_height 23
static const unsigned char aqua_btn_mask_left_xbm_bits[] = {
   0xc0, 0x1f, 0xf0, 0x1f, 0xf8, 0x1f, 0xfc, 0x1f, 0xfe, 0x1f, 0xff, 0x1f,
   0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f,
   0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f,
   0xff, 0x1f, 0xfe, 0x1f, 0xfc, 0x1f, 0xf8, 0x1f, 0xe0, 0x1f };

/* Mask for Aqua buttons - right */
#define aqua_btn_mask_right_xbm_width 13
#define aqua_btn_mask_right_xbm_height 23
static const unsigned char aqua_btn_mask_right_xbm_bits[] = {
   0x7f, 0x00, 0xff, 0x01, 0xff, 0x03, 0xff, 0x07, 0xff, 0x0f, 0xff, 0x1f,
   0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f,
   0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f,
   0xff, 0x1f, 0xff, 0x0f, 0xff, 0x07, 0xff, 0x03, 0xff, 0x00 };

/* Mask for Aqua scrollbars */
#define aqua_hsbr_tip_left_mask_width 10
#define aqua_hsbr_tip_left_mask_height 14
static const unsigned char aqua_hsbr_tip_left_mask_bits[] = {
   0xf0, 0x03, 0xfc, 0x03, 0xfe, 0x03, 0xfe, 0x03, 0xff, 0x03, 0xff, 0x03,
   0xff, 0x03, 0xff, 0x03, 0xff, 0x03, 0xff, 0x03, 0xfe, 0x03, 0xfe, 0x03,
   0xfc, 0x03, 0xf0, 0x03 };

/* Mask for Aqua scrollbars */
#define aqua_hsbr_tip_right_mask_width 10
#define aqua_hsbr_tip_right_mask_height 14
static const unsigned char aqua_hsbr_tip_right_mask_bits[] = {
   0x3f, 0x00, 0xff, 0x00, 0xff, 0x01, 0xff, 0x01, 0xff, 0x03, 0xff, 0x03,
   0xff, 0x03, 0xff, 0x03, 0xff, 0x03, 0xff, 0x03, 0xff, 0x01, 0xff, 0x01,
   0xff, 0x00, 0x3f, 0x00 };

/* Mask for Aqua radio buttons */
#define aqua_radio_mask_xbm_width 14
#define aqua_radio_mask_xbm_height 17
static const unsigned char aqua_radio_mask_xbm_bits[] = {
   0xe0, 0x01, 0xf8, 0x07, 0xfc, 0x0f, 0xfe, 0x1f, 0xff, 0x3f, 0xff, 0x3f,
   0xff, 0x3f, 0xff, 0x3f, 0xff, 0x3f, 0xff, 0x3f, 0xff, 0x3f, 0xff, 0x3f,
   0xff, 0x3f, 0xff, 0x3f, 0xfe, 0x1f, 0xfc, 0x0f, 0xf8, 0x07 };

/* Mask for pointy Aqua sliders */
#define aqua_sldr_pty_mask_width 17
#define aqua_sldr_pty_mask_height 20
static const unsigned char aqua_sldr_pty_mask_bits[] = {
   0xfe, 0xff, 0x00, 0xff, 0xff, 0x01, 0xff, 0xff, 0x01, 0xff, 0xff, 0x01,
   0xff, 0xff, 0x01, 0xff, 0xff, 0x01, 0xff, 0xff, 0x01, 0xff, 0xff, 0x01,
   0xff, 0xff, 0x01, 0xff, 0xff, 0x01, 0xff, 0xff, 0x01, 0xff, 0xff, 0x01,
   0xff, 0xff, 0x01, 0xfe, 0xff, 0x00, 0xfe, 0x7f, 0x00, 0xfc, 0x3f, 0x00,
   0xf8, 0x1f, 0x00, 0xf0, 0x0f, 0x00, 0xe0, 0x07, 0x00, 0xc0, 0x03, 0x00 };

/* Mask for Aqua scrollbars */
#define aqua_vsbr_tip_down_mask_width 14
#define aqua_vsbr_tip_down_mask_height 10
static const unsigned char aqua_vsbr_tip_down_mask_bits[] = {
   0xff, 0x3f, 0xff, 0x3f, 0xff, 0x3f, 0xff, 0x3f, 0xff, 0x3f, 0xff, 0x3f,
   0xfe, 0x1f, 0xfc, 0x1f, 0xf8, 0x0f, 0xf0, 0x03 };

/* Mask for Aqua scrollbars */
#define aqua_vsbr_tip_up_mask_width 14
#define aqua_vsbr_tip_up_mask_height 11
static const unsigned char aqua_vsbr_tip_up_mask_bits[] = {
   0xf0, 0x03, 0xf8, 0x07, 0xfc, 0x0f, 0xfe, 0x1f, 0xff, 0x3f, 0xff, 0x3f,
   0xff, 0x3f, 0xff, 0x3f, 0xff, 0x3f, 0xff, 0x3f, 0xff, 0x3f };

/* XPM */
static const char * const aqua_btn_def_left_xpm[] = {
"13 23 147 2",
"       c None",
".      c #FFFFFF",
"+      c #F7F7F7",
"@      c #D6D6DE",
"#      c #8C8CA5",
"$      c #525284",
"%      c #293973",
"&      c #102163",
"*      c #10296B",
"=      c #183173",
"-      c #EFEFEF",
";      c #8C8CAD",
">      c #313973",
",      c #10216B",
"'      c #294284",
")      c #425A94",
"!      c #526BA5",
"~      c #5A7BAD",
"{      c #637BAD",
"]      c #636B94",
"^      c #001063",
"/      c #18397B",
"(      c #4A6BA5",
"_      c #7394BD",
":      c #849CC6",
"<      c #8CA5CE",
"[      c #94ADCE",
"}      c #E7E7E7",
"|      c #001863",
"1      c #082973",
"2      c #426BA5",
"3      c #9CADD6",
"4      c #9CB5D6",
"5      c #A5BDD6",
"6      c #A5BDDE",
"7      c #848CAD",
"8      c #00186B",
"9      c #002973",
"0      c #18428C",
"a      c #6384BD",
"b      c #ADC6DE",
"c      c #C6CED6",
"d      c #29427B",
"e      c #00297B",
"f      c #08398C",
"g      c #295AA5",
"h      c #7394C6",
"i      c #9CB5DE",
"j      c #A5C6DE",
"k      c #ADC6E7",
"l      c #7B8CAD",
"m      c #00317B",
"n      c #10398C",
"o      c #184A9C",
"p      c #3163AD",
"q      c #6394C6",
"r      c #8CADD6",
"s      c #94BDDE",
"t      c #9CBDE7",
"u      c #A5BDE7",
"v      c #A5C6E7",
"w      c #4A6B9C",
"x      c #10428C",
"y      c #396BB5",
"z      c #4A84C6",
"A      c #6B94CE",
"B      c #7BA5D6",
"C      c #84ADDE",
"D      c #8CB5DE",
"E      c #CED6D6",
"F      c #294A8C",
"G      c #184A94",
"H      c #316BB5",
"I      c #427BBD",
"J      c #5A94CE",
"K      c #6394CE",
"L      c #6B9CD6",
"M      c #73A5D6",
"N      c #BDC6CE",
"O      c #215AA5",
"P      c #316BAD",
"Q      c #528CC6",
"R      c #639CD6",
"S      c #7BA5DE",
"T      c #7BADDE",
"U      c #215294",
"V      c #2963AD",
"W      c #3973BD",
"X      c #6BA5D6",
"Y      c #73A5DE",
"Z      c #84B5DE",
"`      c #8CB5E7",
" .     c #31639C",
"..     c #528CCE",
"+.     c #6394D6",
"@.     c #73ADDE",
"#.     c #84B5E7",
"$.     c #8CBDE7",
"%.     c #94BDE7",
"&.     c #527BA5",
"*.     c #3973B5",
"=.     c #94BDEF",
"-.     c #94C6F7",
";.     c #9CC6F7",
">.     c #7394AD",
",.     c #397BBD",
"'.     c #7BB5E7",
").     c #8CBDEF",
"!.     c #9CCEFF",
"~.     c #A5CEFF",
"{.     c #A5D6FF",
"].     c #9CADBD",
"^.     c #4A7BAD",
"/.     c #6B9CDE",
"(.     c #7BADE7",
"_.     c #ADD6FF",
":.     c #ADDEFF",
"<.     c #ADB5BD",
"[.     c #7394B5",
"}.     c #6BA5DE",
"|.     c #7BADEF",
"1.     c #8CBDF7",
"2.     c #94CEFF",
"3.     c #A5DEFF",
"4.     c #B5E7FF",
"5.     c #A5ADB5",
"6.     c #6B8CB5",
"7.     c #BDEFFF",
"8.     c #94A5B5",
"9.     c #73ADE7",
"0.     c #84BDF7",
"a.     c #9CD6FF",
"b.     c #ADE7FF",
"c.     c #DEDEE7",
"d.     c #ADBDC6",
"e.     c #949CAD",
"f.     c #7B94B5",
"g.     c #8CC6F7",
"h.     c #9CA5AD",
"i.     c #8494AD",
"j.     c #84ADCE",
"k.     c #8C94A5",
"l.     c #84949C",
"m.     c #7B8C94",
"n.     c #7B8C9C",
"o.     c #A5B5BD",
"p.     c #B5BDC6",
". . . . + + @ # $ % & * = ",
". . . + - ; > , ' ) ! ~ { ",
". + + @ ] ^ / ( _ : < [ [ ",
". + } ] | 1 2 : 3 4 5 6 6 ",
"+ } 7 8 9 0 a 4 6 6 b b b ",
"- c d e f g h i 6 j k k k ",
"} l m n o p q r s t u v v ",
"@ w x o g y z A B C C D D ",
"E F G g H I z J K L M M B ",
"N 0 O P I Q J R L M S S T ",
"N U V W z J R X Y T C Z ` ",
"N  .P I ..+.Y @.C #.$.$.%.",
"N &.*.z J X T #.` =.-.;.;.",
"N >.,.Q R Y '.).-.!.~.~.{.",
"c ].^.../.(.).-.!.{._.:.:.",
"E <.[...}.|.1.2.{.3.:.4.4.",
"@ N 5.6./.(.1.2.{.:.4.4.7.",
"} E <.8._ 9.0.2.a.:.b.4.4.",
"- c.c d.e.f.B g.2.{.{.:.:.",
"+ } @ N d.h.i.f.j.` ).!.!.",
". + } @ c <.5.e.k.l.m.n.n.",
". + + } c.E N <.o.5.5.h.h.",
". . . + - } @ E c N N p.p."};
/* XPM */
static const char * const aqua_btn_def_left1_xpm[] = {
"13 23 147 2",
"       c None",
".      c #FFFFFF",
"+      c #F7F7F7",
"@      c #D8D8E0",
"#      c #9292AA",
"$      c #5B5B8A",
"%      c #34437A",
"&      c #1C2C6B",
"*      c #1C3473",
"=      c #243C7A",
"-      c #F0F0F0",
";      c #9292B1",
">      c #3C437A",
",      c #1C2C73",
"'      c #344C8A",
")      c #4C6299",
"!      c #5B73AA",
"~      c #6282B1",
"{      c #6B82B1",
"]      c #6B7399",
"^      c #0D1C6B",
"/      c #244382",
"(      c #5373AA",
"_      c #7A99C0",
":      c #8AA1C9",
"<      c #92AAD0",
"[      c #99B1D0",
"}      c #E8E8E8",
"|      c #0D246B",
"1      c #15347A",
"2      c #4C73AA",
"3      c #A1B1D8",
"4      c #A1B9D8",
"5      c #AAC0D8",
"6      c #AAC0E0",
"7      c #8A92B1",
"8      c #0D2473",
"9      c #0D347A",
"0      c #244C92",
"a      c #6B8AC0",
"b      c #B1C9E0",
"c      c #C9D0D8",
"d      c #344C82",
"e      c #0D3482",
"f      c #154392",
"g      c #3462AA",
"h      c #7A99C9",
"i      c #A1B9E0",
"j      c #AAC9E0",
"k      c #B1C9E8",
"l      c #8292B1",
"m      c #0D3C82",
"n      c #1C4392",
"o      c #2453A1",
"p      c #3C6BB1",
"q      c #6B99C9",
"r      c #92B1D8",
"s      c #99C0E0",
"t      c #A1C0E8",
"u      c #AAC0E8",
"v      c #AAC9E8",
"w      c #5373A1",
"x      c #1C4C92",
"y      c #4373B9",
"z      c #538AC9",
"A      c #7399D0",
"B      c #82AAD8",
"C      c #8AB1E0",
"D      c #92B9E0",
"E      c #D0D8D8",
"F      c #345392",
"G      c #245399",
"H      c #3C73B9",
"I      c #4C82C0",
"J      c #6299D0",
"K      c #6B99D0",
"L      c #73A1D8",
"M      c #7AAAD8",
"N      c #C0C9D0",
"O      c #2C62AA",
"P      c #3C73B1",
"Q      c #5B92C9",
"R      c #6BA1D8",
"S      c #82AAE0",
"T      c #82B1E0",
"U      c #2C5B99",
"V      c #346BB1",
"W      c #437AC0",
"X      c #73AAD8",
"Y      c #7AAAE0",
"Z      c #8AB9E0",
"`      c #92B9E8",
" .     c #3C6BA1",
"..     c #5B92D0",
"+.     c #6B99D8",
"@.     c #7AB1E0",
"#.     c #8AB9E8",
"$.     c #92C0E8",
"%.     c #99C0E8",
"&.     c #5B82AA",
"*.     c #437AB9",
"=.     c #99C0F0",
"-.     c #99C9F7",
";.     c #A1C9F7",
">.     c #7A99B1",
",.     c #4382C0",
"'.     c #82B9E8",
").     c #92C0F0",
"!.     c #A1D0FF",
"~.     c #AAD0FF",
"{.     c #AAD8FF",
"].     c #A1B1C0",
"^.     c #5382B1",
"/.     c #73A1E0",
"(.     c #82B1E8",
"_.     c #B1D8FF",
":.     c #B1E0FF",
"<.     c #B1B9C0",
"[.     c #7A99B9",
"}.     c #73AAE0",
"|.     c #82B1F0",
"1.     c #92C0F7",
"2.     c #99D0FF",
"3.     c #AAE0FF",
"4.     c #B9E8FF",
"5.     c #AAB1B9",
"6.     c #7392B9",
"7.     c #C0F0FF",
"8.     c #99AAB9",
"9.     c #7AB1E8",
"0.     c #8AC0F7",
"a.     c #A1D8FF",
"b.     c #B1E8FF",
"c.     c #E0E0E8",
"d.     c #B1C0C9",
"e.     c #99A1B1",
"f.     c #8299B9",
"g.     c #92C9F7",
"h.     c #A1AAB1",
"i.     c #8A99B1",
"j.     c #8AB1D0",
"k.     c #9299AA",
"l.     c #8A99A1",
"m.     c #829299",
"n.     c #8292A1",
"o.     c #AAB9C0",
"p.     c #B9C0C9",
". . . . + + @ # $ % & * = ",
". . . + - ; > , ' ) ! ~ { ",
". + + @ ] ^ / ( _ : < [ [ ",
". + } ] | 1 2 : 3 4 5 6 6 ",
"+ } 7 8 9 0 a 4 6 6 b b b ",
"- c d e f g h i 6 j k k k ",
"} l m n o p q r s t u v v ",
"@ w x o g y z A B C C D D ",
"E F G g H I z J K L M M B ",
"N 0 O P I Q J R L M S S T ",
"N U V W z J R X Y T C Z ` ",
"N  .P I ..+.Y @.C #.$.$.%.",
"N &.*.z J X T #.` =.-.;.;.",
"N >.,.Q R Y '.).-.!.~.~.{.",
"c ].^.../.(.).-.!.{._.:.:.",
"E <.[...}.|.1.2.{.3.:.4.4.",
"@ N 5.6./.(.1.2.{.:.4.4.7.",
"} E <.8._ 9.0.2.a.:.b.4.4.",
"- c.c d.e.f.B g.2.{.{.:.:.",
"+ } @ N d.h.i.f.j.` ).!.!.",
". + } @ c <.5.e.k.l.m.n.n.",
". + + } c.E N <.o.5.5.h.h.",
". . . + - } @ E c N N p.p."};
/* XPM */
static const char * const aqua_btn_def_left2_xpm[] = {
"13 23 147 2",
"       c None",
".      c #FFFFFF",
"+      c #F8F8F8",
"@      c #DADAE1",
"#      c #9898AE",
"$      c #646491",
"%      c #3F4D81",
"&      c #283873",
"*      c #283F7A",
"=      c #304681",
"-      c #F1F1F1",
";      c #9898B5",
">      c #464D81",
",      c #28387A",
"'      c #3F5591",
")      c #556B9F",
"!      c #647AAE",
"~      c #6B88B5",
"{      c #7388B5",
"]      c #737A9F",
"^      c #1A2873",
"/      c #304D88",
"(      c #5C7AAE",
"_      c #819FC4",
":      c #91A6CC",
"<      c #98AED3",
"[      c #9FB5D3",
"}      c #E9E9E9",
"|      c #1A3073",
"1      c #213F81",
"2      c #557AAE",
"3      c #A6B5DA",
"4      c #A6BDDA",
"5      c #AEC4DA",
"6      c #AEC4E1",
"7      c #9198B5",
"8      c #1A307A",
"9      c #1A3F81",
"0      c #305598",
"a      c #7391C4",
"b      c #B5CCE1",
"c      c #CCD3DA",
"d      c #3F5588",
"e      c #1A3F88",
"f      c #214D98",
"g      c #3F6BAE",
"h      c #819FCC",
"i      c #A6BDE1",
"j      c #AECCE1",
"k      c #B5CCE9",
"l      c #8898B5",
"m      c #1A4688",
"n      c #284D98",
"o      c #305CA6",
"p      c #4673B5",
"q      c #739FCC",
"r      c #98B5DA",
"s      c #9FC4E1",
"t      c #A6C4E9",
"u      c #AEC4E9",
"v      c #AECCE9",
"w      c #5C7AA6",
"x      c #285598",
"y      c #4D7ABD",
"z      c #5C91CC",
"A      c #7A9FD3",
"B      c #88AEDA",
"C      c #91B5E1",
"D      c #98BDE1",
"E      c #D3DADA",
"F      c #3F5C98",
"G      c #305C9F",
"H      c #467ABD",
"I      c #5588C4",
"J      c #6B9FD3",
"K      c #739FD3",
"L      c #7AA6DA",
"M      c #81AEDA",
"N      c #C4CCD3",
"O      c #386BAE",
"P      c #467AB5",
"Q      c #6498CC",
"R      c #73A6DA",
"S      c #88AEE1",
"T      c #88B5E1",
"U      c #38649F",
"V      c #3F73B5",
"W      c #4D81C4",
"X      c #7AAEDA",
"Y      c #81AEE1",
"Z      c #91BDE1",
"`      c #98BDE9",
" .     c #4673A6",
"..     c #6498D3",
"+.     c #739FDA",
"@.     c #81B5E1",
"#.     c #91BDE9",
"$.     c #98C4E9",
"%.     c #9FC4E9",
"&.     c #6488AE",
"*.     c #4D81BD",
"=.     c #9FC4F1",
"-.     c #9FCCF8",
";.     c #A6CCF8",
">.     c #819FB5",
",.     c #4D88C4",
"'.     c #88BDE9",
").     c #98C4F1",
"!.     c #A6D3FF",
"~.     c #AED3FF",
"{.     c #AEDAFF",
"].     c #A6B5C4",
"^.     c #5C88B5",
"/.     c #7AA6E1",
"(.     c #88B5E9",
"_.     c #B5DAFF",
":.     c #B5E1FF",
"<.     c #B5BDC4",
"[.     c #819FBD",
"}.     c #7AAEE1",
"|.     c #88B5F1",
"1.     c #98C4F8",
"2.     c #9FD3FF",
"3.     c #AEE1FF",
"4.     c #BDE9FF",
"5.     c #AEB5BD",
"6.     c #7A98BD",
"7.     c #C4F1FF",
"8.     c #9FAEBD",
"9.     c #81B5E9",
"0.     c #91C4F8",
"a.     c #A6DAFF",
"b.     c #B5E9FF",
"c.     c #E1E1E9",
"d.     c #B5C4CC",
"e.     c #9FA6B5",
"f.     c #889FBD",
"g.     c #98CCF8",
"h.     c #A6AEB5",
"i.     c #919FB5",
"j.     c #91B5D3",
"k.     c #989FAE",
"l.     c #919FA6",
"m.     c #88989F",
"n.     c #8898A6",
"o.     c #AEBDC4",
"p.     c #BDC4CC",
". . . . + + @ # $ % & * = ",
". . . + - ; > , ' ) ! ~ { ",
". + + @ ] ^ / ( _ : < [ [ ",
". + } ] | 1 2 : 3 4 5 6 6 ",
"+ } 7 8 9 0 a 4 6 6 b b b ",
"- c d e f g h i 6 j k k k ",
"} l m n o p q r s t u v v ",
"@ w x o g y z A B C C D D ",
"E F G g H I z J K L M M B ",
"N 0 O P I Q J R L M S S T ",
"N U V W z J R X Y T C Z ` ",
"N  .P I ..+.Y @.C #.$.$.%.",
"N &.*.z J X T #.` =.-.;.;.",
"N >.,.Q R Y '.).-.!.~.~.{.",
"c ].^.../.(.).-.!.{._.:.:.",
"E <.[...}.|.1.2.{.3.:.4.4.",
"@ N 5.6./.(.1.2.{.:.4.4.7.",
"} E <.8._ 9.0.2.a.:.b.4.4.",
"- c.c d.e.f.B g.2.{.{.:.:.",
"+ } @ N d.h.i.f.j.` ).!.!.",
". + } @ c <.5.e.k.l.m.n.n.",
". + + } c.E N <.o.5.5.h.h.",
". . . + - } @ E c N N p.p."};
/* XPM */
static const char * const aqua_btn_def_left3_xpm[] = {
"13 23 147 2",
"       c None",
".      c #FFFFFF",
"+      c #F8F8F8",
"@      c #DCDCE3",
"#      c #9E9EB3",
"$      c #6C6C97",
"%      c #4A5788",
"&      c #35437B",
"*      c #354A82",
"=      c #3B5188",
"-      c #F1F1F1",
";      c #9E9EBA",
">      c #515788",
",      c #354382",
"'      c #4A5F97",
")      c #5F73A4",
"!      c #6C82B3",
"~      c #738FBA",
"{      c #7B8FBA",
"]      c #7B82A4",
"^      c #27357B",
"/      c #3B578F",
"(      c #6682B3",
"_      c #88A4C7",
":      c #97ABCF",
"<      c #9EB3D5",
"[      c #A4BAD5",
"}      c #EBEBEB",
"|      c #273B7B",
"1      c #2E4A88",
"2      c #5F82B3",
"3      c #ABBADC",
"4      c #ABC0DC",
"5      c #B3C7DC",
"6      c #B3C7E3",
"7      c #979EBA",
"8      c #273B82",
"9      c #274A88",
"0      c #3B5F9E",
"a      c #7B97C7",
"b      c #BACFE3",
"c      c #CFD5DC",
"d      c #4A5F8F",
"e      c #274A8F",
"f      c #2E579E",
"g      c #4A73B3",
"h      c #88A4CF",
"i      c #ABC0E3",
"j      c #B3CFE3",
"k      c #BACFEB",
"l      c #8F9EBA",
"m      c #27518F",
"n      c #35579E",
"o      c #3B66AB",
"p      c #517BBA",
"q      c #7BA4CF",
"r      c #9EBADC",
"s      c #A4C7E3",
"t      c #ABC7EB",
"u      c #B3C7EB",
"v      c #B3CFEB",
"w      c #6682AB",
"x      c #355F9E",
"y      c #5782C0",
"z      c #6697CF",
"A      c #82A4D5",
"B      c #8FB3DC",
"C      c #97BAE3",
"D      c #9EC0E3",
"E      c #D5DCDC",
"F      c #4A669E",
"G      c #3B66A4",
"H      c #5182C0",
"I      c #5F8FC7",
"J      c #73A4D5",
"K      c #7BA4D5",
"L      c #82ABDC",
"M      c #88B3DC",
"N      c #C7CFD5",
"O      c #4373B3",
"P      c #5182BA",
"Q      c #6C9ECF",
"R      c #7BABDC",
"S      c #8FB3E3",
"T      c #8FBAE3",
"U      c #436CA4",
"V      c #4A7BBA",
"W      c #5788C7",
"X      c #82B3DC",
"Y      c #88B3E3",
"Z      c #97C0E3",
"`      c #9EC0EB",
" .     c #517BAB",
"..     c #6C9ED5",
"+.     c #7BA4DC",
"@.     c #88BAE3",
"#.     c #97C0EB",
"$.     c #9EC7EB",
"%.     c #A4C7EB",
"&.     c #6C8FB3",
"*.     c #5788C0",
"=.     c #A4C7F1",
"-.     c #A4CFF8",
";.     c #ABCFF8",
">.     c #88A4BA",
",.     c #578FC7",
"'.     c #8FC0EB",
").     c #9EC7F1",
"!.     c #ABD5FF",
"~.     c #B3D5FF",
"{.     c #B3DCFF",
"].     c #ABBAC7",
"^.     c #668FBA",
"/.     c #82ABE3",
"(.     c #8FBAEB",
"_.     c #BADCFF",
":.     c #BAE3FF",
"<.     c #BAC0C7",
"[.     c #88A4C0",
"}.     c #82B3E3",
"|.     c #8FBAF1",
"1.     c #9EC7F8",
"2.     c #A4D5FF",
"3.     c #B3E3FF",
"4.     c #C0EBFF",
"5.     c #B3BAC0",
"6.     c #829EC0",
"7.     c #C7F1FF",
"8.     c #A4B3C0",
"9.     c #88BAEB",
"0.     c #97C7F8",
"a.     c #ABDCFF",
"b.     c #BAEBFF",
"c.     c #E3E3EB",
"d.     c #BAC7CF",
"e.     c #A4ABBA",
"f.     c #8FA4C0",
"g.     c #9ECFF8",
"h.     c #ABB3BA",
"i.     c #97A4BA",
"j.     c #97BAD5",
"k.     c #9EA4B3",
"l.     c #97A4AB",
"m.     c #8F9EA4",
"n.     c #8F9EAB",
"o.     c #B3C0C7",
"p.     c #C0C7CF",
". . . . + + @ # $ % & * = ",
". . . + - ; > , ' ) ! ~ { ",
". + + @ ] ^ / ( _ : < [ [ ",
". + } ] | 1 2 : 3 4 5 6 6 ",
"+ } 7 8 9 0 a 4 6 6 b b b ",
"- c d e f g h i 6 j k k k ",
"} l m n o p q r s t u v v ",
"@ w x o g y z A B C C D D ",
"E F G g H I z J K L M M B ",
"N 0 O P I Q J R L M S S T ",
"N U V W z J R X Y T C Z ` ",
"N  .P I ..+.Y @.C #.$.$.%.",
"N &.*.z J X T #.` =.-.;.;.",
"N >.,.Q R Y '.).-.!.~.~.{.",
"c ].^.../.(.).-.!.{._.:.:.",
"E <.[...}.|.1.2.{.3.:.4.4.",
"@ N 5.6./.(.1.2.{.:.4.4.7.",
"} E <.8._ 9.0.2.a.:.b.4.4.",
"- c.c d.e.f.B g.2.{.{.:.:.",
"+ } @ N d.h.i.f.j.` ).!.!.",
". + } @ c <.5.e.k.l.m.n.n.",
". + + } c.E N <.o.5.5.h.h.",
". . . + - } @ E c N N p.p."};
/* XPM */
static const char * const aqua_btn_def_left4_xpm[] = {
"13 23 147 2",
"       c None",
".      c #FFFFFF",
"+      c #F9F9F9",
"@      c #DEDEE5",
"#      c #A3A3B7",
"$      c #75759D",
"%      c #556190",
"&      c #414E83",
"*      c #415589",
"=      c #475B90",
"-      c #F2F2F2",
";      c #A3A3BE",
">      c #5B6190",
",      c #414E89",
"'      c #55699D",
")      c #697CAA",
"!      c #7589B7",
"~      c #7C96BE",
"{      c #8396BE",
"]      c #8389AA",
"^      c #344183",
"/      c #476196",
"(      c #6F89B7",
"_      c #90AACA",
":      c #9DB0D2",
"<      c #A3B7D8",
"[      c #AABED8",
"}      c #ECECEC",
"|      c #344783",
"1      c #3A5590",
"2      c #6989B7",
"3      c #B0BEDE",
"4      c #B0C4DE",
"5      c #B7CADE",
"6      c #B7CAE5",
"7      c #9DA3BE",
"8      c #344789",
"9      c #345590",
"0      c #4769A3",
"a      c #839DCA",
"b      c #BED2E5",
"c      c #D2D8DE",
"d      c #556996",
"e      c #345596",
"f      c #3A61A3",
"g      c #557CB7",
"h      c #90AAD2",
"i      c #B0C4E5",
"j      c #B7D2E5",
"k      c #BED2EC",
"l      c #96A3BE",
"m      c #345B96",
"n      c #4161A3",
"o      c #476FB0",
"p      c #5B83BE",
"q      c #83AAD2",
"r      c #A3BEDE",
"s      c #AACAE5",
"t      c #B0CAEC",
"u      c #B7CAEC",
"v      c #B7D2EC",
"w      c #6F89B0",
"x      c #4169A3",
"y      c #6189C4",
"z      c #6F9DD2",
"A      c #89AAD8",
"B      c #96B7DE",
"C      c #9DBEE5",
"D      c #A3C4E5",
"E      c #D8DEDE",
"F      c #556FA3",
"G      c #476FAA",
"H      c #5B89C4",
"I      c #6996CA",
"J      c #7CAAD8",
"K      c #83AAD8",
"L      c #89B0DE",
"M      c #90B7DE",
"N      c #CAD2D8",
"O      c #4E7CB7",
"P      c #5B89BE",
"Q      c #75A3D2",
"R      c #83B0DE",
"S      c #96B7E5",
"T      c #96BEE5",
"U      c #4E75AA",
"V      c #5583BE",
"W      c #6190CA",
"X      c #89B7DE",
"Y      c #90B7E5",
"Z      c #9DC4E5",
"`      c #A3C4EC",
" .     c #5B83B0",
"..     c #75A3D8",
"+.     c #83AADE",
"@.     c #90BEE5",
"#.     c #9DC4EC",
"$.     c #A3CAEC",
"%.     c #AACAEC",
"&.     c #7596B7",
"*.     c #6190C4",
"=.     c #AACAF2",
"-.     c #AAD2F9",
";.     c #B0D2F9",
">.     c #90AABE",
",.     c #6196CA",
"'.     c #96C4EC",
").     c #A3CAF2",
"!.     c #B0D8FF",
"~.     c #B7D8FF",
"{.     c #B7DEFF",
"].     c #B0BECA",
"^.     c #6F96BE",
"/.     c #89B0E5",
"(.     c #96BEEC",
"_.     c #BEDEFF",
":.     c #BEE5FF",
"<.     c #BEC4CA",
"[.     c #90AAC4",
"}.     c #89B7E5",
"|.     c #96BEF2",
"1.     c #A3CAF9",
"2.     c #AAD8FF",
"3.     c #B7E5FF",
"4.     c #C4ECFF",
"5.     c #B7BEC4",
"6.     c #89A3C4",
"7.     c #CAF2FF",
"8.     c #AAB7C4",
"9.     c #90BEEC",
"0.     c #9DCAF9",
"a.     c #B0DEFF",
"b.     c #BEECFF",
"c.     c #E5E5EC",
"d.     c #BECAD2",
"e.     c #AAB0BE",
"f.     c #96AAC4",
"g.     c #A3D2F9",
"h.     c #B0B7BE",
"i.     c #9DAABE",
"j.     c #9DBED8",
"k.     c #A3AAB7",
"l.     c #9DAAB0",
"m.     c #96A3AA",
"n.     c #96A3B0",
"o.     c #B7C4CA",
"p.     c #C4CAD2",
". . . . + + @ # $ % & * = ",
". . . + - ; > , ' ) ! ~ { ",
". + + @ ] ^ / ( _ : < [ [ ",
". + } ] | 1 2 : 3 4 5 6 6 ",
"+ } 7 8 9 0 a 4 6 6 b b b ",
"- c d e f g h i 6 j k k k ",
"} l m n o p q r s t u v v ",
"@ w x o g y z A B C C D D ",
"E F G g H I z J K L M M B ",
"N 0 O P I Q J R L M S S T ",
"N U V W z J R X Y T C Z ` ",
"N  .P I ..+.Y @.C #.$.$.%.",
"N &.*.z J X T #.` =.-.;.;.",
"N >.,.Q R Y '.).-.!.~.~.{.",
"c ].^.../.(.).-.!.{._.:.:.",
"E <.[...}.|.1.2.{.3.:.4.4.",
"@ N 5.6./.(.1.2.{.:.4.4.7.",
"} E <.8._ 9.0.2.a.:.b.4.4.",
"- c.c d.e.f.B g.2.{.{.:.:.",
"+ } @ N d.h.i.f.j.` ).!.!.",
". + } @ c <.5.e.k.l.m.n.n.",
". + + } c.E N <.o.5.5.h.h.",
". . . + - } @ E c N N p.p."};
/* XPM */
static const char * const aqua_btn_def_left5_xpm[] = {
"13 23 147 2",
"       c None",
".      c #FFFFFF",
"+      c #F9F9F9",
"@      c #E0E0E6",
"#      c #A9A9BC",
"$      c #7D7DA3",
"%      c #5F6B96",
"&      c #4C598A",
"*      c #4C5F90",
"=      c #526596",
"-      c #F3F3F3",
";      c #A9A9C2",
">      c #656B96",
",      c #4C5990",
"'      c #5F71A3",
")      c #7183AF",
"!      c #7D90BC",
"~      c #839CC2",
"{      c #8A9CC2",
"]      c #8A90AF",
"^      c #404C8A",
"/      c #526B9C",
"(      c #7790BC",
"_      c #96AFCE",
":      c #A3B5D4",
"<      c #A9BCDA",
"[      c #AFC2DA",
"}      c #EDEDED",
"|      c #40528A",
"1      c #465F96",
"2      c #7190BC",
"3      c #B5C2E0",
"4      c #B5C8E0",
"5      c #BCCEE0",
"6      c #BCCEE6",
"7      c #A3A9C2",
"8      c #405290",
"9      c #405F96",
"0      c #5271A9",
"a      c #8AA3CE",
"b      c #C2D4E6",
"c      c #D4DAE0",
"d      c #5F719C",
"e      c #405F9C",
"f      c #466BA9",
"g      c #5F83BC",
"h      c #96AFD4",
"i      c #B5C8E6",
"j      c #BCD4E6",
"k      c #C2D4ED",
"l      c #9CA9C2",
"m      c #40659C",
"n      c #4C6BA9",
"o      c #5277B5",
"p      c #658AC2",
"q      c #8AAFD4",
"r      c #A9C2E0",
"s      c #AFCEE6",
"t      c #B5CEED",
"u      c #BCCEED",
"v      c #BCD4ED",
"w      c #7790B5",
"x      c #4C71A9",
"y      c #6B90C8",
"z      c #77A3D4",
"A      c #90AFDA",
"B      c #9CBCE0",
"C      c #A3C2E6",
"D      c #A9C8E6",
"E      c #DAE0E0",
"F      c #5F77A9",
"G      c #5277AF",
"H      c #6590C8",
"I      c #719CCE",
"J      c #83AFDA",
"K      c #8AAFDA",
"L      c #90B5E0",
"M      c #96BCE0",
"N      c #CED4DA",
"O      c #5983BC",
"P      c #6590C2",
"Q      c #7DA9D4",
"R      c #8AB5E0",
"S      c #9CBCE6",
"T      c #9CC2E6",
"U      c #597DAF",
"V      c #5F8AC2",
"W      c #6B96CE",
"X      c #90BCE0",
"Y      c #96BCE6",
"Z      c #A3C8E6",
"`      c #A9C8ED",
" .     c #658AB5",
"..     c #7DA9DA",
"+.     c #8AAFE0",
"@.     c #96C2E6",
"#.     c #A3C8ED",
"$.     c #A9CEED",
"%.     c #AFCEED",
"&.     c #7D9CBC",
"*.     c #6B96C8",
"=.     c #AFCEF3",
"-.     c #AFD4F9",
";.     c #B5D4F9",
">.     c #96AFC2",
",.     c #6B9CCE",
"'.     c #9CC8ED",
").     c #A9CEF3",
"!.     c #B5DAFF",
"~.     c #BCDAFF",
"{.     c #BCE0FF",
"].     c #B5C2CE",
"^.     c #779CC2",
"/.     c #90B5E6",
"(.     c #9CC2ED",
"_.     c #C2E0FF",
":.     c #C2E6FF",
"<.     c #C2C8CE",
"[.     c #96AFC8",
"}.     c #90BCE6",
"|.     c #9CC2F3",
"1.     c #A9CEF9",
"2.     c #AFDAFF",
"3.     c #BCE6FF",
"4.     c #C8EDFF",
"5.     c #BCC2C8",
"6.     c #90A9C8",
"7.     c #CEF3FF",
"8.     c #AFBCC8",
"9.     c #96C2ED",
"0.     c #A3CEF9",
"a.     c #B5E0FF",
"b.     c #C2EDFF",
"c.     c #E6E6ED",
"d.     c #C2CED4",
"e.     c #AFB5C2",
"f.     c #9CAFC8",
"g.     c #A9D4F9",
"h.     c #B5BCC2",
"i.     c #A3AFC2",
"j.     c #A3C2DA",
"k.     c #A9AFBC",
"l.     c #A3AFB5",
"m.     c #9CA9AF",
"n.     c #9CA9B5",
"o.     c #BCC8CE",
"p.     c #C8CED4",
". . . . + + @ # $ % & * = ",
". . . + - ; > , ' ) ! ~ { ",
". + + @ ] ^ / ( _ : < [ [ ",
". + } ] | 1 2 : 3 4 5 6 6 ",
"+ } 7 8 9 0 a 4 6 6 b b b ",
"- c d e f g h i 6 j k k k ",
"} l m n o p q r s t u v v ",
"@ w x o g y z A B C C D D ",
"E F G g H I z J K L M M B ",
"N 0 O P I Q J R L M S S T ",
"N U V W z J R X Y T C Z ` ",
"N  .P I ..+.Y @.C #.$.$.%.",
"N &.*.z J X T #.` =.-.;.;.",
"N >.,.Q R Y '.).-.!.~.~.{.",
"c ].^.../.(.).-.!.{._.:.:.",
"E <.[...}.|.1.2.{.3.:.4.4.",
"@ N 5.6./.(.1.2.{.:.4.4.7.",
"} E <.8._ 9.0.2.a.:.b.4.4.",
"- c.c d.e.f.B g.2.{.{.:.:.",
"+ } @ N d.h.i.f.j.` ).!.!.",
". + } @ c <.5.e.k.l.m.n.n.",
". + + } c.E N <.o.5.5.h.h.",
". . . + - } @ E c N N p.p."};
/* XPM */
static const char * const aqua_btn_def_left6_xpm[] = {
"13 23 147 2",
"       c None",
".      c #FFFFFF",
"+      c #F9F9F9",
"@      c #E3E3E8",
"#      c #AFAFC1",
"$      c #8787AA",
"%      c #6A769E",
"&      c #596593",
"*      c #596A98",
"=      c #5F709E",
"-      c #F4F4F4",
";      c #AFAFC6",
">      c #70769E",
",      c #596598",
"'      c #6A7CAA",
")      c #7C8CB5",
"!      c #8798C1",
"~      c #8CA3C6",
"{      c #93A3C6",
"]      c #9398B5",
"^      c #4E5993",
"/      c #5F76A3",
"(      c #8198C1",
"_      c #9EB5D1",
":      c #AABAD7",
"<      c #AFC1DD",
"[      c #B5C6DD",
"}      c #EEEEEE",
"|      c #4E5F93",
"1      c #546A9E",
"2      c #7C98C1",
"3      c #BAC6E3",
"4      c #BACCE3",
"5      c #C1D1E3",
"6      c #C1D1E8",
"7      c #AAAFC6",
"8      c #4E5F98",
"9      c #4E6A9E",
"0      c #5F7CAF",
"a      c #93AAD1",
"b      c #C6D7E8",
"c      c #D7DDE3",
"d      c #6A7CA3",
"e      c #4E6AA3",
"f      c #5476AF",
"g      c #6A8CC1",
"h      c #9EB5D7",
"i      c #BACCE8",
"j      c #C1D7E8",
"k      c #C6D7EE",
"l      c #A3AFC6",
"m      c #4E70A3",
"n      c #5976AF",
"o      c #5F81BA",
"p      c #7093C6",
"q      c #93B5D7",
"r      c #AFC6E3",
"s      c #B5D1E8",
"t      c #BAD1EE",
"u      c #C1D1EE",
"v      c #C1D7EE",
"w      c #8198BA",
"x      c #597CAF",
"y      c #7698CC",
"z      c #81AAD7",
"A      c #98B5DD",
"B      c #A3C1E3",
"C      c #AAC6E8",
"D      c #AFCCE8",
"E      c #DDE3E3",
"F      c #6A81AF",
"G      c #5F81B5",
"H      c #7098CC",
"I      c #7CA3D1",
"J      c #8CB5DD",
"K      c #93B5DD",
"L      c #98BAE3",
"M      c #9EC1E3",
"N      c #D1D7DD",
"O      c #658CC1",
"P      c #7098C6",
"Q      c #87AFD7",
"R      c #93BAE3",
"S      c #A3C1E8",
"T      c #A3C6E8",
"U      c #6587B5",
"V      c #6A93C6",
"W      c #769ED1",
"X      c #98C1E3",
"Y      c #9EC1E8",
"Z      c #AACCE8",
"`      c #AFCCEE",
" .     c #7093BA",
"..     c #87AFDD",
"+.     c #93B5E3",
"@.     c #9EC6E8",
"#.     c #AACCEE",
"$.     c #AFD1EE",
"%.     c #B5D1EE",
"&.     c #87A3C1",
"*.     c #769ECC",
"=.     c #B5D1F4",
"-.     c #B5D7F9",
";.     c #BAD7F9",
">.     c #9EB5C6",
",.     c #76A3D1",
"'.     c #A3CCEE",
").     c #AFD1F4",
"!.     c #BADDFF",
"~.     c #C1DDFF",
"{.     c #C1E3FF",
"].     c #BAC6D1",
"^.     c #81A3C6",
"/.     c #98BAE8",
"(.     c #A3C6EE",
"_.     c #C6E3FF",
":.     c #C6E8FF",
"<.     c #C6CCD1",
"[.     c #9EB5CC",
"}.     c #98C1E8",
"|.     c #A3C6F4",
"1.     c #AFD1F9",
"2.     c #B5DDFF",
"3.     c #C1E8FF",
"4.     c #CCEEFF",
"5.     c #C1C6CC",
"6.     c #98AFCC",
"7.     c #D1F4FF",
"8.     c #B5C1CC",
"9.     c #9EC6EE",
"0.     c #AAD1F9",
"a.     c #BAE3FF",
"b.     c #C6EEFF",
"c.     c #E8E8EE",
"d.     c #C6D1D7",
"e.     c #B5BAC6",
"f.     c #A3B5CC",
"g.     c #AFD7F9",
"h.     c #BAC1C6",
"i.     c #AAB5C6",
"j.     c #AAC6DD",
"k.     c #AFB5C1",
"l.     c #AAB5BA",
"m.     c #A3AFB5",
"n.     c #A3AFBA",
"o.     c #C1CCD1",
"p.     c #CCD1D7",
". . . . + + @ # $ % & * = ",
". . . + - ; > , ' ) ! ~ { ",
". + + @ ] ^ / ( _ : < [ [ ",
". + } ] | 1 2 : 3 4 5 6 6 ",
"+ } 7 8 9 0 a 4 6 6 b b b ",
"- c d e f g h i 6 j k k k ",
"} l m n o p q r s t u v v ",
"@ w x o g y z A B C C D D ",
"E F G g H I z J K L M M B ",
"N 0 O P I Q J R L M S S T ",
"N U V W z J R X Y T C Z ` ",
"N  .P I ..+.Y @.C #.$.$.%.",
"N &.*.z J X T #.` =.-.;.;.",
"N >.,.Q R Y '.).-.!.~.~.{.",
"c ].^.../.(.).-.!.{._.:.:.",
"E <.[...}.|.1.2.{.3.:.4.4.",
"@ N 5.6./.(.1.2.{.:.4.4.7.",
"} E <.8._ 9.0.2.a.:.b.4.4.",
"- c.c d.e.f.B g.2.{.{.:.:.",
"+ } @ N d.h.i.f.j.` ).!.!.",
". + } @ c <.5.e.k.l.m.n.n.",
". + + } c.E N <.o.5.5.h.h.",
". . . + - } @ E c N N p.p."};
/* XPM */
static const char * const aqua_btn_def_left7_xpm[] = {
"13 23 147 2",
"       c None",
".      c #FFFFFF",
"+      c #FAFAFA",
"@      c #E4E4EA",
"#      c #B5B5C5",
"$      c #8F8FAF",
"%      c #757FA4",
"&      c #646F9A",
"*      c #64759F",
"=      c #6A7AA4",
"-      c #F5F5F5",
";      c #B5B5CA",
">      c #7A7FA4",
",      c #646F9F",
"'      c #7585AF",
")      c #8594BA",
"!      c #8F9FC5",
"~      c #94AACA",
"{      c #9AAACA",
"]      c #9A9FBA",
"^      c #5A649A",
"/      c #6A7FAA",
"(      c #8A9FC5",
"_      c #A4BAD4",
":      c #AFBFDA",
"<      c #B5C5DF",
"[      c #BACADF",
"}      c #EFEFEF",
"|      c #5A6A9A",
"1      c #5F75A4",
"2      c #859FC5",
"3      c #BFCAE4",
"4      c #BFCFE4",
"5      c #C5D4E4",
"6      c #C5D4EA",
"7      c #AFB5CA",
"8      c #5A6A9F",
"9      c #5A75A4",
"0      c #6A85B5",
"a      c #9AAFD4",
"b      c #CADAEA",
"c      c #DADFE4",
"d      c #7585AA",
"e      c #5A75AA",
"f      c #5F7FB5",
"g      c #7594C5",
"h      c #A4BADA",
"i      c #BFCFEA",
"j      c #C5DAEA",
"k      c #CADAEF",
"l      c #AAB5CA",
"m      c #5A7AAA",
"n      c #647FB5",
"o      c #6A8ABF",
"p      c #7A9ACA",
"q      c #9ABADA",
"r      c #B5CAE4",
"s      c #BAD4EA",
"t      c #BFD4EF",
"u      c #C5D4EF",
"v      c #C5DAEF",
"w      c #8A9FBF",
"x      c #6485B5",
"y      c #7F9FCF",
"z      c #8AAFDA",
"A      c #9FBADF",
"B      c #AAC5E4",
"C      c #AFCAEA",
"D      c #B5CFEA",
"E      c #DFE4E4",
"F      c #758AB5",
"G      c #6A8ABA",
"H      c #7A9FCF",
"I      c #85AAD4",
"J      c #94BADF",
"K      c #9ABADF",
"L      c #9FBFE4",
"M      c #A4C5E4",
"N      c #D4DADF",
"O      c #6F94C5",
"P      c #7A9FCA",
"Q      c #8FB5DA",
"R      c #9ABFE4",
"S      c #AAC5EA",
"T      c #AACAEA",
"U      c #6F8FBA",
"V      c #759ACA",
"W      c #7FA4D4",
"X      c #9FC5E4",
"Y      c #A4C5EA",
"Z      c #AFCFEA",
"`      c #B5CFEF",
" .     c #7A9ABF",
"..     c #8FB5DF",
"+.     c #9ABAE4",
"@.     c #A4CAEA",
"#.     c #AFCFEF",
"$.     c #B5D4EF",
"%.     c #BAD4EF",
"&.     c #8FAAC5",
"*.     c #7FA4CF",
"=.     c #BAD4F5",
"-.     c #BADAFA",
";.     c #BFDAFA",
">.     c #A4BACA",
",.     c #7FAAD4",
"'.     c #AACFEF",
").     c #B5D4F5",
"!.     c #BFDFFF",
"~.     c #C5DFFF",
"{.     c #C5E4FF",
"].     c #BFCAD4",
"^.     c #8AAACA",
"/.     c #9FBFEA",
"(.     c #AACAEF",
"_.     c #CAE4FF",
":.     c #CAEAFF",
"<.     c #CACFD4",
"[.     c #A4BACF",
"}.     c #9FC5EA",
"|.     c #AACAF5",
"1.     c #B5D4FA",
"2.     c #BADFFF",
"3.     c #C5EAFF",
"4.     c #CFEFFF",
"5.     c #C5CACF",
"6.     c #9FB5CF",
"7.     c #D4F5FF",
"8.     c #BAC5CF",
"9.     c #A4CAEF",
"0.     c #AFD4FA",
"a.     c #BFE4FF",
"b.     c #CAEFFF",
"c.     c #EAEAEF",
"d.     c #CAD4DA",
"e.     c #BABFCA",
"f.     c #AABACF",
"g.     c #B5DAFA",
"h.     c #BFC5CA",
"i.     c #AFBACA",
"j.     c #AFCADF",
"k.     c #B5BAC5",
"l.     c #AFBABF",
"m.     c #AAB5BA",
"n.     c #AAB5BF",
"o.     c #C5CFD4",
"p.     c #CFD4DA",
". . . . + + @ # $ % & * = ",
". . . + - ; > , ' ) ! ~ { ",
". + + @ ] ^ / ( _ : < [ [ ",
". + } ] | 1 2 : 3 4 5 6 6 ",
"+ } 7 8 9 0 a 4 6 6 b b b ",
"- c d e f g h i 6 j k k k ",
"} l m n o p q r s t u v v ",
"@ w x o g y z A B C C D D ",
"E F G g H I z J K L M M B ",
"N 0 O P I Q J R L M S S T ",
"N U V W z J R X Y T C Z ` ",
"N  .P I ..+.Y @.C #.$.$.%.",
"N &.*.z J X T #.` =.-.;.;.",
"N >.,.Q R Y '.).-.!.~.~.{.",
"c ].^.../.(.).-.!.{._.:.:.",
"E <.[...}.|.1.2.{.3.:.4.4.",
"@ N 5.6./.(.1.2.{.:.4.4.7.",
"} E <.8._ 9.0.2.a.:.b.4.4.",
"- c.c d.e.f.B g.2.{.{.:.:.",
"+ } @ N d.h.i.f.j.` ).!.!.",
". + } @ c <.5.e.k.l.m.n.n.",
". + + } c.E N <.o.5.5.h.h.",
". . . + - } @ E c N N p.p."};
/* XPM */
static const char * const aqua_btn_def_left8_xpm[] = {
"13 23 147 2",
"       c None",
".      c #FFFFFF",
"+      c #FAFAFA",
"@      c #E7E7EB",
"#      c #BABAC9",
"$      c #9898B6",
"%      c #7F89AC",
"&      c #717BA2",
"*      c #717FA7",
"=      c #7584AC",
"-      c #F5F5F5",
";      c #BABACE",
">      c #8489AC",
",      c #717BA7",
"'      c #7F8EB6",
")      c #8E9DBF",
"!      c #98A7C9",
"~      c #9DB0CE",
"{      c #A2B0CE",
"]      c #A2A7BF",
"^      c #6771A2",
"/      c #7589B0",
"(      c #93A7C9",
"_      c #ACBFD8",
":      c #B6C4DD",
"<      c #BAC9E2",
"[      c #BFCEE2",
"}      c #F1F1F1",
"|      c #6775A2",
"1      c #6C7FAC",
"2      c #8EA7C9",
"3      c #C4CEE7",
"4      c #C4D3E7",
"5      c #C9D8E7",
"6      c #C9D8EB",
"7      c #B6BACE",
"8      c #6775A7",
"9      c #677FAC",
"0      c #758EBA",
"a      c #A2B6D8",
"b      c #CEDDEB",
"c      c #DDE2E7",
"d      c #7F8EB0",
"e      c #677FB0",
"f      c #6C89BA",
"g      c #7F9DC9",
"h      c #ACBFDD",
"i      c #C4D3EB",
"j      c #C9DDEB",
"k      c #CEDDF1",
"l      c #B0BACE",
"m      c #6784B0",
"n      c #7189BA",
"o      c #7593C4",
"p      c #84A2CE",
"q      c #A2BFDD",
"r      c #BACEE7",
"s      c #BFD8EB",
"t      c #C4D8F1",
"u      c #C9D8F1",
"v      c #C9DDF1",
"w      c #93A7C4",
"x      c #718EBA",
"y      c #89A7D3",
"z      c #93B6DD",
"A      c #A7BFE2",
"B      c #B0C9E7",
"C      c #B6CEEB",
"D      c #BAD3EB",
"E      c #E2E7E7",
"F      c #7F93BA",
"G      c #7593BF",
"H      c #84A7D3",
"I      c #8EB0D8",
"J      c #9DBFE2",
"K      c #A2BFE2",
"L      c #A7C4E7",
"M      c #ACC9E7",
"N      c #D8DDE2",
"O      c #7B9DC9",
"P      c #84A7CE",
"Q      c #98BADD",
"R      c #A2C4E7",
"S      c #B0C9EB",
"T      c #B0CEEB",
"U      c #7B98BF",
"V      c #7FA2CE",
"W      c #89ACD8",
"X      c #A7C9E7",
"Y      c #ACC9EB",
"Z      c #B6D3EB",
"`      c #BAD3F1",
" .     c #84A2C4",
"..     c #98BAE2",
"+.     c #A2BFE7",
"@.     c #ACCEEB",
"#.     c #B6D3F1",
"$.     c #BAD8F1",
"%.     c #BFD8F1",
"&.     c #98B0C9",
"*.     c #89ACD3",
"=.     c #BFD8F5",
"-.     c #BFDDFA",
";.     c #C4DDFA",
">.     c #ACBFCE",
",.     c #89B0D8",
"'.     c #B0D3F1",
").     c #BAD8F5",
"!.     c #C4E2FF",
"~.     c #C9E2FF",
"{.     c #C9E7FF",
"].     c #C4CED8",
"^.     c #93B0CE",
"/.     c #A7C4EB",
"(.     c #B0CEF1",
"_.     c #CEE7FF",
":.     c #CEEBFF",
"<.     c #CED3D8",
"[.     c #ACBFD3",
"}.     c #A7C9EB",
"|.     c #B0CEF5",
"1.     c #BAD8FA",
"2.     c #BFE2FF",
"3.     c #C9EBFF",
"4.     c #D3F1FF",
"5.     c #C9CED3",
"6.     c #A7BAD3",
"7.     c #D8F5FF",
"8.     c #BFC9D3",
"9.     c #ACCEF1",
"0.     c #B6D8FA",
"a.     c #C4E7FF",
"b.     c #CEF1FF",
"c.     c #EBEBF1",
"d.     c #CED8DD",
"e.     c #BFC4CE",
"f.     c #B0BFD3",
"g.     c #BADDFA",
"h.     c #C4C9CE",
"i.     c #B6BFCE",
"j.     c #B6CEE2",
"k.     c #BABFC9",
"l.     c #B6BFC4",
"m.     c #B0BABF",
"n.     c #B0BAC4",
"o.     c #C9D3D8",
"p.     c #D3D8DD",
". . . . + + @ # $ % & * = ",
". . . + - ; > , ' ) ! ~ { ",
". + + @ ] ^ / ( _ : < [ [ ",
". + } ] | 1 2 : 3 4 5 6 6 ",
"+ } 7 8 9 0 a 4 6 6 b b b ",
"- c d e f g h i 6 j k k k ",
"} l m n o p q r s t u v v ",
"@ w x o g y z A B C C D D ",
"E F G g H I z J K L M M B ",
"N 0 O P I Q J R L M S S T ",
"N U V W z J R X Y T C Z ` ",
"N  .P I ..+.Y @.C #.$.$.%.",
"N &.*.z J X T #.` =.-.;.;.",
"N >.,.Q R Y '.).-.!.~.~.{.",
"c ].^.../.(.).-.!.{._.:.:.",
"E <.[...}.|.1.2.{.3.:.4.4.",
"@ N 5.6./.(.1.2.{.:.4.4.7.",
"} E <.8._ 9.0.2.a.:.b.4.4.",
"- c.c d.e.f.B g.2.{.{.:.:.",
"+ } @ N d.h.i.f.j.` ).!.!.",
". + } @ c <.5.e.k.l.m.n.n.",
". + + } c.E N <.o.5.5.h.h.",
". . . + - } @ E c N N p.p."};
/* XPM */
static const char * const aqua_btn_def_left9_xpm[] = {
"13 23 147 2",
"       c None",
".      c #FFFFFF",
"+      c #FBFBFB",
"@      c #E8E8ED",
"#      c #C0C0CE",
"$      c #A0A0BB",
"%      c #8A92B2",
"&      c #7C85A9",
"*      c #7C8AAE",
"=      c #808EB2",
"-      c #F6F6F6",
";      c #C0C0D2",
">      c #8E92B2",
",      c #7C85AE",
"'      c #8A97BB",
")      c #97A4C4",
"!      c #A0AECE",
"~      c #A4B7D2",
"{      c #A9B7D2",
"]      c #A9AEC4",
"^      c #737CA9",
"/      c #8092B7",
"(      c #9CAECE",
"_      c #B2C4DB",
":      c #BBC9E0",
"<      c #C0CEE4",
"[      c #C4D2E4",
"}      c #F2F2F2",
"|      c #7380A9",
"1      c #778AB2",
"2      c #97AECE",
"3      c #C9D2E8",
"4      c #C9D6E8",
"5      c #CEDBE8",
"6      c #CEDBED",
"7      c #BBC0D2",
"8      c #7380AE",
"9      c #738AB2",
"0      c #8097C0",
"a      c #A9BBDB",
"b      c #D2E0ED",
"c      c #E0E4E8",
"d      c #8A97B7",
"e      c #738AB7",
"f      c #7792C0",
"g      c #8AA4CE",
"h      c #B2C4E0",
"i      c #C9D6ED",
"j      c #CEE0ED",
"k      c #D2E0F2",
"l      c #B7C0D2",
"m      c #738EB7",
"n      c #7C92C0",
"o      c #809CC9",
"p      c #8EA9D2",
"q      c #A9C4E0",
"r      c #C0D2E8",
"s      c #C4DBED",
"t      c #C9DBF2",
"u      c #CEDBF2",
"v      c #CEE0F2",
"w      c #9CAEC9",
"x      c #7C97C0",
"y      c #92AED6",
"z      c #9CBBE0",
"A      c #AEC4E4",
"B      c #B7CEE8",
"C      c #BBD2ED",
"D      c #C0D6ED",
"E      c #E4E8E8",
"F      c #8A9CC0",
"G      c #809CC4",
"H      c #8EAED6",
"I      c #97B7DB",
"J      c #A4C4E4",
"K      c #A9C4E4",
"L      c #AEC9E8",
"M      c #B2CEE8",
"N      c #DBE0E4",
"O      c #85A4CE",
"P      c #8EAED2",
"Q      c #A0C0E0",
"R      c #A9C9E8",
"S      c #B7CEED",
"T      c #B7D2ED",
"U      c #85A0C4",
"V      c #8AA9D2",
"W      c #92B2DB",
"X      c #AECEE8",
"Y      c #B2CEED",
"Z      c #BBD6ED",
"`      c #C0D6F2",
" .     c #8EA9C9",
"..     c #A0C0E4",
"+.     c #A9C4E8",
"@.     c #B2D2ED",
"#.     c #BBD6F2",
"$.     c #C0DBF2",
"%.     c #C4DBF2",
"&.     c #A0B7CE",
"*.     c #92B2D6",
"=.     c #C4DBF6",
"-.     c #C4E0FB",
";.     c #C9E0FB",
">.     c #B2C4D2",
",.     c #92B7DB",
"'.     c #B7D6F2",
").     c #C0DBF6",
"!.     c #C9E4FF",
"~.     c #CEE4FF",
"{.     c #CEE8FF",
"].     c #C9D2DB",
"^.     c #9CB7D2",
"/.     c #AEC9ED",
"(.     c #B7D2F2",
"_.     c #D2E8FF",
":.     c #D2EDFF",
"<.     c #D2D6DB",
"[.     c #B2C4D6",
"}.     c #AECEED",
"|.     c #B7D2F6",
"1.     c #C0DBFB",
"2.     c #C4E4FF",
"3.     c #CEEDFF",
"4.     c #D6F2FF",
"5.     c #CED2D6",
"6.     c #AEC0D6",
"7.     c #DBF6FF",
"8.     c #C4CED6",
"9.     c #B2D2F2",
"0.     c #BBDBFB",
"a.     c #C9E8FF",
"b.     c #D2F2FF",
"c.     c #EDEDF2",
"d.     c #D2DBE0",
"e.     c #C4C9D2",
"f.     c #B7C4D6",
"g.     c #C0E0FB",
"h.     c #C9CED2",
"i.     c #BBC4D2",
"j.     c #BBD2E4",
"k.     c #C0C4CE",
"l.     c #BBC4C9",
"m.     c #B7C0C4",
"n.     c #B7C0C9",
"o.     c #CED6DB",
"p.     c #D6DBE0",
". . . . + + @ # $ % & * = ",
". . . + - ; > , ' ) ! ~ { ",
". + + @ ] ^ / ( _ : < [ [ ",
". + } ] | 1 2 : 3 4 5 6 6 ",
"+ } 7 8 9 0 a 4 6 6 b b b ",
"- c d e f g h i 6 j k k k ",
"} l m n o p q r s t u v v ",
"@ w x o g y z A B C C D D ",
"E F G g H I z J K L M M B ",
"N 0 O P I Q J R L M S S T ",
"N U V W z J R X Y T C Z ` ",
"N  .P I ..+.Y @.C #.$.$.%.",
"N &.*.z J X T #.` =.-.;.;.",
"N >.,.Q R Y '.).-.!.~.~.{.",
"c ].^.../.(.).-.!.{._.:.:.",
"E <.[...}.|.1.2.{.3.:.4.4.",
"@ N 5.6./.(.1.2.{.:.4.4.7.",
"} E <.8._ 9.0.2.a.:.b.4.4.",
"- c.c d.e.f.B g.2.{.{.:.:.",
"+ } @ N d.h.i.f.j.` ).!.!.",
". + } @ c <.5.e.k.l.m.n.n.",
". + + } c.E N <.o.5.5.h.h.",
". . . + - } @ E c N N p.p."};
/* XPM */
static const char * const aqua_btn_def_mid_xpm[] = {
"7 23 38 1",
"       c None",
".      c #183173",
"+      c #21397B",
"@      c #6384AD",
"#      c #6384B5",
"$      c #6B84B5",
"%      c #94ADCE",
"&      c #9CADCE",
"*      c #9CADD6",
"=      c #A5BDDE",
"-      c #ADBDDE",
";      c #ADC6E7",
">      c #ADCEE7",
",      c #B5CEE7",
"'      c #A5C6E7",
")      c #8CB5DE",
"!      c #94B5DE",
"~      c #7BA5D6",
"{      c #7BADD6",
"]      c #7BADDE",
"^      c #84ADDE",
"/      c #8CB5E7",
"(      c #94BDE7",
"_      c #94BDEF",
":      c #9CBDEF",
"<      c #9CC6F7",
"[      c #9CCEF7",
"}      c #A5CEF7",
"|      c #ADD6FF",
"1      c #A5D6FF",
"2      c #ADDEFF",
"3      c #B5DEFF",
"4      c #B5E7FF",
"5      c #BDE7FF",
"6      c #BDEFFF",
"7      c #7B8C9C",
"8      c #9CA5AD",
"9      c #B5BDC6",
".++++++",
"@#$$$$$",
"%&&**&&",
"===----",
";;;;;;;",
">>,,,,,",
"'';;;;;",
"))!!!!!",
"~~{{{{{",
"]^^^^^^",
"///////",
"(_:::::",
"<<[[[}}",
"|1|||||",
"2333333",
"4455555",
"6666666",
"4456555",
"3444444",
"1111111",
"7777777",
"8888888",
"9999999"};
/* XPM */
static const char * const aqua_btn_def_mid1_xpm[] = {
"7 23 41 1",
"       c None",
".      c #243C7A",
"+      c #2C4382",
"@      c #6B82B1",
"#      c #6B8AB1",
"$      c #6B8AB9",
"%      c #738AB9",
"&      c #99B1D0",
"*      c #A1B1D0",
"=      c #A1B1D8",
"-      c #AAC0E0",
";      c #B1C0E0",
">      c #B1C9E0",
",      c #B1C9E8",
"'      c #B1D0E8",
")      c #B9D0E8",
"!      c #AAC9E8",
"~      c #92B9E0",
"{      c #99B9E0",
"]      c #82AAD8",
"^      c #82B1D8",
"/      c #82B1E0",
"(      c #8AB1E0",
"_      c #92B9E8",
":      c #99C0E8",
"<      c #99C0F0",
"[      c #A1C0F0",
"}      c #A1C9F7",
"|      c #A1D0F7",
"1      c #AAD0F7",
"2      c #AAD8FF",
"3      c #B1D8FF",
"4      c #B1E0FF",
"5      c #B9E0FF",
"6      c #B9E8FF",
"7      c #C0E8FF",
"8      c #C0F0FF",
"9      c #A1D0FF",
"0      c #8292A1",
"a      c #A1AAB1",
"b      c #B9C0C9",
"..+++++",
"@#$%%%%",
"&&**==*",
"----;;;",
">,,,,,,",
",''))))",
"!!!,,,,",
"~~~{{{{",
"]]]^^^^",
"//(((((",
"_______",
"::<[[[[",
"}}}|||1",
"2323333",
"4455555",
"6667777",
"8888888",
"6667877",
"4566666",
"9222222",
"0000000",
"aaaaaaa",
"bbbbbbb"};
/* XPM */
static const char * const aqua_btn_def_mid2_xpm[] = {
"7 23 39 1",
"       c None",
".      c #304681",
"+      c #384D88",
"@      c #384D81",
"#      c #7391B5",
"$      c #7391BD",
"%      c #7A91BD",
"&      c #9FB5D3",
"*      c #A6B5D3",
"=      c #A6B5DA",
"-      c #AEC4E1",
";      c #B5C4E1",
">      c #B5CCE9",
",      c #B5D3E9",
"'      c #BDD3E9",
")      c #AECCE9",
"!      c #98BDE1",
"~      c #9FBDE1",
"{      c #88AEDA",
"]      c #88B5DA",
"^      c #88B5E1",
"/      c #91B5E1",
"(      c #98BDE9",
"_      c #9FC4E9",
":      c #9FC4F1",
"<      c #A6C4F1",
"[      c #A6CCF8",
"}      c #A6D3F8",
"|      c #AED3F8",
"1      c #B5DAFF",
"2      c #AEDAFF",
"3      c #B5E1FF",
"4      c #BDE1FF",
"5      c #BDE9FF",
"6      c #C4E9FF",
"7      c #C4F1FF",
"8      c #8898A6",
"9      c #A6AEB5",
"0      c #BDC4CC",
".+++++@",
"#$%%%%%",
"&**==*&",
"---;;;-",
">>>>>>>",
",,'''',",
"))>>>>)",
"!!~~~~!",
"{{]]]]{",
"^/////^",
"(((((((",
"_:<<<<_",
"[[}}}|[",
"1211112",
"3444444",
"5566665",
"7777777",
"5567665",
"4555553",
"2222222",
"8888888",
"9999999",
"0000000"};
/* XPM */
static const char * const aqua_btn_def_mid3_xpm[] = {
"7 23 39 1",
"       c None",
".      c #3B5188",
"+      c #43578F",
"@      c #435788",
"#      c #7B97BA",
"$      c #7B97C0",
"%      c #8297C0",
"&      c #A4BAD5",
"*      c #ABBAD5",
"=      c #ABBADC",
"-      c #B3C7E3",
";      c #BAC7E3",
">      c #BACFEB",
",      c #BAD5EB",
"'      c #C0D5EB",
")      c #B3CFEB",
"!      c #9EC0E3",
"~      c #A4C0E3",
"{      c #8FB3DC",
"]      c #8FBADC",
"^      c #8FBAE3",
"/      c #97BAE3",
"(      c #9EC0EB",
"_      c #A4C7EB",
":      c #A4C7F1",
"<      c #ABC7F1",
"[      c #ABCFF8",
"}      c #ABD5F8",
"|      c #B3D5F8",
"1      c #BADCFF",
"2      c #B3DCFF",
"3      c #BAE3FF",
"4      c #C0E3FF",
"5      c #C0EBFF",
"6      c #C7EBFF",
"7      c #C7F1FF",
"8      c #8F9EAB",
"9      c #ABB3BA",
"0      c #C0C7CF",
".+++++@",
"#$%%%%%",
"&**==*&",
"---;;;-",
">>>>>>>",
",,'''',",
"))>>>>)",
"!!~~~~!",
"{{]]]]{",
"^/////^",
"(((((((",
"_:<<<<_",
"[[}}}|[",
"1211112",
"3444444",
"5566665",
"7777777",
"5567665",
"4555553",
"2222222",
"8888888",
"9999999",
"0000000"};
/* XPM */
static const char * const aqua_btn_def_mid4_xpm[] = {
"7 23 39 1",
"       c None",
".      c #475B90",
"+      c #4E6196",
"@      c #4E6190",
"#      c #839DBE",
"$      c #839DC4",
"%      c #899DC4",
"&      c #AABED8",
"*      c #B0BED8",
"=      c #B0BEDE",
"-      c #B7CAE5",
";      c #BECAE5",
">      c #BED2EC",
",      c #BED8EC",
"'      c #C4D8EC",
")      c #B7D2EC",
"!      c #A3C4E5",
"~      c #AAC4E5",
"{      c #96B7DE",
"]      c #96BEDE",
"^      c #96BEE5",
"/      c #9DBEE5",
"(      c #A3C4EC",
"_      c #AACAEC",
":      c #AACAF2",
"<      c #B0CAF2",
"[      c #B0D2F9",
"}      c #B0D8F9",
"|      c #B7D8F9",
"1      c #BEDEFF",
"2      c #B7DEFF",
"3      c #BEE5FF",
"4      c #C4E5FF",
"5      c #C4ECFF",
"6      c #CAECFF",
"7      c #CAF2FF",
"8      c #96A3B0",
"9      c #B0B7BE",
"0      c #C4CAD2",
".+++++@",
"#$%%%%%",
"&**==*&",
"---;;;-",
">>>>>>>",
",,'''',",
"))>>>>)",
"!!~~~~!",
"{{]]]]{",
"^/////^",
"(((((((",
"_:<<<<_",
"[[}}}|[",
"1211112",
"3444444",
"5566665",
"7777777",
"5567665",
"4555553",
"2222222",
"8888888",
"9999999",
"0000000"};
/* XPM */
static const char * const aqua_btn_def_mid5_xpm[] = {
"7 23 39 1",
"       c None",
".      c #526596",
"+      c #596B9C",
"@      c #596B96",
"#      c #8AA3C2",
"$      c #8AA3C8",
"%      c #90A3C8",
"&      c #AFC2DA",
"*      c #B5C2DA",
"=      c #B5C2E0",
"-      c #BCCEE6",
";      c #C2CEE6",
">      c #C2D4ED",
",      c #C2DAED",
"'      c #C8DAED",
")      c #BCD4ED",
"!      c #A9C8E6",
"~      c #AFC8E6",
"{      c #9CBCE0",
"]      c #9CC2E0",
"^      c #9CC2E6",
"/      c #A3C2E6",
"(      c #A9C8ED",
"_      c #AFCEED",
":      c #AFCEF3",
"<      c #B5CEF3",
"[      c #B5D4F9",
"}      c #B5DAF9",
"|      c #BCDAF9",
"1      c #C2E0FF",
"2      c #BCE0FF",
"3      c #C2E6FF",
"4      c #C8E6FF",
"5      c #C8EDFF",
"6      c #CEEDFF",
"7      c #CEF3FF",
"8      c #9CA9B5",
"9      c #B5BCC2",
"0      c #C8CED4",
".+++++@",
"#$%%%%%",
"&**==*&",
"---;;;-",
">>>>>>>",
",,'''',",
"))>>>>)",
"!!~~~~!",
"{{]]]]{",
"^/////^",
"(((((((",
"_:<<<<_",
"[[}}}|[",
"1211112",
"3444444",
"5566665",
"7777777",
"5567665",
"4555553",
"2222222",
"8888888",
"9999999",
"0000000"};
/* XPM */
static const char * const aqua_btn_def_mid6_xpm[] = {
"7 23 39 1",
"       c None",
".      c #5F709E",
"+      c #6576A3",
"@      c #65769E",
"#      c #93AAC6",
"$      c #93AACC",
"%      c #98AACC",
"&      c #B5C6DD",
"*      c #BAC6DD",
"=      c #BAC6E3",
"-      c #C1D1E8",
";      c #C6D1E8",
">      c #C6D7EE",
",      c #C6DDEE",
"'      c #CCDDEE",
")      c #C1D7EE",
"!      c #AFCCE8",
"~      c #B5CCE8",
"{      c #A3C1E3",
"]      c #A3C6E3",
"^      c #A3C6E8",
"/      c #AAC6E8",
"(      c #AFCCEE",
"_      c #B5D1EE",
":      c #B5D1F4",
"<      c #BAD1F4",
"[      c #BAD7F9",
"}      c #BADDF9",
"|      c #C1DDF9",
"1      c #C6E3FF",
"2      c #C1E3FF",
"3      c #C6E8FF",
"4      c #CCE8FF",
"5      c #CCEEFF",
"6      c #D1EEFF",
"7      c #D1F4FF",
"8      c #A3AFBA",
"9      c #BAC1C6",
"0      c #CCD1D7",
".+++++@",
"#$%%%%%",
"&**==*&",
"---;;;-",
">>>>>>>",
",,'''',",
"))>>>>)",
"!!~~~~!",
"{{]]]]{",
"^/////^",
"(((((((",
"_:<<<<_",
"[[}}}|[",
"1211112",
"3444444",
"5566665",
"7777777",
"5567665",
"4555553",
"2222222",
"8888888",
"9999999",
"0000000"};
/* XPM */
static const char * const aqua_btn_def_mid7_xpm[] = {
"7 23 39 1",
"       c None",
".      c #6A7AA4",
"+      c #6F7FAA",
"@      c #6F7FA4",
"#      c #9AAFCA",
"$      c #9AAFCF",
"%      c #9FAFCF",
"&      c #BACADF",
"*      c #BFCADF",
"=      c #BFCAE4",
"-      c #C5D4EA",
";      c #CAD4EA",
">      c #CADAEF",
",      c #CADFEF",
"'      c #CFDFEF",
")      c #C5DAEF",
"!      c #B5CFEA",
"~      c #BACFEA",
"{      c #AAC5E4",
"]      c #AACAE4",
"^      c #AACAEA",
"/      c #AFCAEA",
"(      c #B5CFEF",
"_      c #BAD4EF",
":      c #BAD4F5",
"<      c #BFD4F5",
"[      c #BFDAFA",
"}      c #BFDFFA",
"|      c #C5DFFA",
"1      c #CAE4FF",
"2      c #C5E4FF",
"3      c #CAEAFF",
"4      c #CFEAFF",
"5      c #CFEFFF",
"6      c #D4EFFF",
"7      c #D4F5FF",
"8      c #AAB5BF",
"9      c #BFC5CA",
"0      c #CFD4DA",
".+++++@",
"#$%%%%%",
"&**==*&",
"---;;;-",
">>>>>>>",
",,'''',",
"))>>>>)",
"!!~~~~!",
"{{]]]]{",
"^/////^",
"(((((((",
"_:<<<<_",
"[[}}}|[",
"1211112",
"3444444",
"5566665",
"7777777",
"5567665",
"4555553",
"2222222",
"8888888",
"9999999",
"0000000"};
/* XPM */
static const char * const aqua_btn_def_mid8_xpm[] = {
"7 23 39 1",
"       c None",
".      c #7584AC",
"+      c #7B89B0",
"@      c #7B89AC",
"#      c #A2B6CE",
"$      c #A2B6D3",
"%      c #A7B6D3",
"&      c #BFCEE2",
"*      c #C4CEE2",
"=      c #C4CEE7",
"-      c #C9D8EB",
";      c #CED8EB",
">      c #CEDDF1",
",      c #CEE2F1",
"'      c #D3E2F1",
")      c #C9DDF1",
"!      c #BAD3EB",
"~      c #BFD3EB",
"{      c #B0C9E7",
"]      c #B0CEE7",
"^      c #B0CEEB",
"/      c #B6CEEB",
"(      c #BAD3F1",
"_      c #BFD8F1",
":      c #BFD8F5",
"<      c #C4D8F5",
"[      c #C4DDFA",
"}      c #C4E2FA",
"|      c #C9E2FA",
"1      c #CEE7FF",
"2      c #C9E7FF",
"3      c #CEEBFF",
"4      c #D3EBFF",
"5      c #D3F1FF",
"6      c #D8F1FF",
"7      c #D8F5FF",
"8      c #B0BAC4",
"9      c #C4C9CE",
"0      c #D3D8DD",
".+++++@",
"#$%%%%%",
"&**==*&",
"---;;;-",
">>>>>>>",
",,'''',",
"))>>>>)",
"!!~~~~!",
"{{]]]]{",
"^/////^",
"(((((((",
"_:<<<<_",
"[[}}}|[",
"1211112",
"3444444",
"5566665",
"7777777",
"5567665",
"4555553",
"2222222",
"8888888",
"9999999",
"0000000"};
/* XPM */
static const char * const aqua_btn_def_mid9_xpm[] = {
"7 23 39 1",
"       c None",
".      c #808EB2",
"+      c #8592B7",
"@      c #8592B2",
"#      c #A9BBD2",
"$      c #A9BBD6",
"%      c #AEBBD6",
"&      c #C4D2E4",
"*      c #C9D2E4",
"=      c #C9D2E8",
"-      c #CEDBED",
";      c #D2DBED",
">      c #D2E0F2",
",      c #D2E4F2",
"'      c #D6E4F2",
")      c #CEE0F2",
"!      c #C0D6ED",
"~      c #C4D6ED",
"{      c #B7CEE8",
"]      c #B7D2E8",
"^      c #B7D2ED",
"/      c #BBD2ED",
"(      c #C0D6F2",
"_      c #C4DBF2",
":      c #C4DBF6",
"<      c #C9DBF6",
"[      c #C9E0FB",
"}      c #C9E4FB",
"|      c #CEE4FB",
"1      c #D2E8FF",
"2      c #CEE8FF",
"3      c #D2EDFF",
"4      c #D6EDFF",
"5      c #D6F2FF",
"6      c #DBF2FF",
"7      c #DBF6FF",
"8      c #B7C0C9",
"9      c #C9CED2",
"0      c #D6DBE0",
".+++++@",
"#$%%%%%",
"&**==*&",
"---;;;-",
">>>>>>>",
",,'''',",
"))>>>>)",
"!!~~~~!",
"{{]]]]{",
"^/////^",
"(((((((",
"_:<<<<_",
"[[}}}|[",
"1211112",
"3444444",
"5566665",
"7777777",
"5567665",
"4555553",
"2222222",
"8888888",
"9999999",
"0000000"};
/* XPM */
static const char * const aqua_btn_def_right_xpm[] = {
"13 23 137 2",
"       c None",
".      c #213973",
"+      c #183173",
"@      c #18296B",
"#      c #313973",
"$      c #525A84",
"%      c #8C8CA5",
"&      c #D6D6DE",
"*      c #FFFFFF",
"=      c #6B84B5",
"-      c #637BAD",
";      c #5A73AD",
">      c #4A639C",
",      c #214284",
"'      c #081863",
")      c #8C8CAD",
"!      c #94ADCE",
"~      c #94A5CE",
"{      c #8CA5C6",
"]      c #738CBD",
"^      c #39639C",
"/      c #082973",
"(      c #001063",
"_      c #636B94",
":      c #A5BDDE",
"<      c #A5BDD6",
"[      c #A5B5D6",
"}      c #9CADD6",
"|      c #7B9CC6",
"1      c #315A9C",
"2      c #002973",
"3      c #00216B",
"4      c #636B9C",
"5      c #E7E7E7",
"6      c #ADC6E7",
"7      c #ADC6DE",
"8      c #9CB5D6",
"9      c #5A84BD",
"0      c #18428C",
"a      c #00317B",
"b      c #002173",
"c      c #848CAD",
"d      c #ADCEE7",
"e      c #A5C6E7",
"f      c #A5C6DE",
"g      c #9CBDDE",
"h      c #7394C6",
"i      c #295AA5",
"j      c #10428C",
"k      c #083184",
"l      c #294A84",
"m      c #C6CED6",
"n      c #EFEFEF",
"o      c #9CBDE7",
"p      c #94BDDE",
"q      c #8CB5D6",
"r      c #6394C6",
"s      c #316BAD",
"t      c #2152A5",
"u      c #104294",
"v      c #7B8CAD",
"w      c #8CB5DE",
"x      c #84ADDE",
"y      c #7BA5D6",
"z      c #6B9CCE",
"A      c #528CC6",
"B      c #3973BD",
"C      c #215AA5",
"D      c #184A94",
"E      c #526B9C",
"F      c #73A5D6",
"G      c #6B9CD6",
"H      c #6394CE",
"I      c #4A84C6",
"J      c #21529C",
"K      c #315A94",
"L      c #CED6D6",
"M      c #7BADDE",
"N      c #528CCE",
"O      c #3163AD",
"P      c #BDC6CE",
"Q      c #8CB5E7",
"R      c #84B5E7",
"S      c #316BB5",
"T      c #94BDE7",
"U      c #8CBDE7",
"V      c #73A5DE",
"W      c #396BA5",
"X      c #9CC6F7",
"Y      c #94BDEF",
"Z      c #8CBDEF",
"`      c #639CD6",
" .     c #427BBD",
"..     c #5A7BAD",
"+.     c #A5D6FF",
"@.     c #A5CEFF",
"#.     c #9CCEFF",
"$.     c #94C6F7",
"%.     c #6BA5DE",
"&.     c #5A94CE",
"*.     c #7B94B5",
"=.     c #B5DEFF",
"-.     c #ADDEFF",
";.     c #ADD6FF",
">.     c #7BB5E7",
",.     c #5284B5",
"'.     c #9CADBD",
").     c #B5E7FF",
"!.     c #8CBDF7",
"~.     c #84B5EF",
"{.     c #6394D6",
"].     c #7B9CB5",
"^.     c #ADB5BD",
"/.     c #BDEFFF",
"(.     c #8CC6F7",
"_.     c #6B94B5",
":.     c #A5ADB5",
"<.     c #ADE7FF",
"[.     c #94CEFF",
"}.     c #7BB5EF",
"|.     c #739CBD",
"1.     c #94A5B5",
"2.     c #A5DEFF",
"3.     c #7B9CBD",
"4.     c #949CAD",
"5.     c #ADBDC6",
"6.     c #DEDEE7",
"7.     c #94C6EF",
"8.     c #84ADCE",
"9.     c #8494AD",
"0.     c #9CA5AD",
"a.     c #7B8C9C",
"b.     c #7B8C94",
"c.     c #84949C",
"d.     c #8C94A5",
"e.     c #A5B5BD",
"f.     c #B5BDC6",
". + @ # $ % & * * * * * * ",
"= - ; > , ' # ) * * * * * ",
"! ! ~ { ] ^ / ( _ & * * * ",
": : < [ } | 1 2 3 4 5 * * ",
"6 7 7 7 : 8 9 0 a b c 5 * ",
"d 6 6 e f g h i j k l m n ",
"e e e o p q r s t u k v 5 ",
"w w w x y z A B s C D E & ",
"y F F G G H A I B s J K L ",
"M M y F F G H N I B O J P ",
"Q R x M M F G H N I S i P ",
"T T U Q R M V G H A B W P ",
"X X X Y Z R M V ` N  ...P ",
"+.@.@.#.$.Z R M %.&.I *.P ",
"=.-.;.+.#.$.Z >.V H ,.'.m ",
").).).-.+.#.!.~.V {.].^.L ",
"/.).).-.+.#.(.~.V _.:.P & ",
").).<.-.+.[.!.}.|.1.^.L 5 ",
"-.-.2.+.#.$.M 3.4.5.m 6.n ",
"+.#.7.Q 8.*.9.0.5.P & 5 * ",
"a.a.b.c.d.4.:.^.m & 5 * * ",
"0.0.:.:.e.^.P L 6.5 * * * ",
"f.f.P P m L & 5 * * * * * "};
/* XPM */
static const char * const aqua_btn_def_right1_xpm[] = {
"13 23 137 2",
"       c None",
".      c #2C437A",
"+      c #243C7A",
"@      c #243473",
"#      c #3C437A",
"$      c #5B628A",
"%      c #9292AA",
"&      c #D8D8E0",
"*      c #FFFFFF",
"=      c #738AB9",
"-      c #6B82B1",
";      c #627AB1",
">      c #536BA1",
",      c #2C4C8A",
"'      c #15246B",
")      c #9292B1",
"!      c #99B1D0",
"~      c #99AAD0",
"{      c #92AAC9",
"]      c #7A92C0",
"^      c #436BA1",
"/      c #15347A",
"(      c #0D1C6B",
"_      c #6B7399",
":      c #AAC0E0",
"<      c #AAC0D8",
"[      c #AAB9D8",
"}      c #A1B1D8",
"|      c #82A1C9",
"1      c #3C62A1",
"2      c #0D347A",
"3      c #0D2C73",
"4      c #6B73A1",
"5      c #E8E8E8",
"6      c #B1C9E8",
"7      c #B1C9E0",
"8      c #A1B9D8",
"9      c #628AC0",
"0      c #244C92",
"a      c #0D3C82",
"b      c #0D2C7A",
"c      c #8A92B1",
"d      c #B1D0E8",
"e      c #AAC9E8",
"f      c #AAC9E0",
"g      c #A1C0E0",
"h      c #7A99C9",
"i      c #3462AA",
"j      c #1C4C92",
"k      c #153C8A",
"l      c #34538A",
"m      c #C9D0D8",
"n      c #F0F0F0",
"o      c #A1C0E8",
"p      c #99C0E0",
"q      c #92B9D8",
"r      c #6B99C9",
"s      c #3C73B1",
"t      c #2C5BAA",
"u      c #1C4C99",
"v      c #8292B1",
"w      c #92B9E0",
"x      c #8AB1E0",
"y      c #82AAD8",
"z      c #73A1D0",
"A      c #5B92C9",
"B      c #437AC0",
"C      c #2C62AA",
"D      c #245399",
"E      c #5B73A1",
"F      c #7AAAD8",
"G      c #73A1D8",
"H      c #6B99D0",
"I      c #538AC9",
"J      c #2C5BA1",
"K      c #3C6299",
"L      c #D0D8D8",
"M      c #82B1E0",
"N      c #5B92D0",
"O      c #3C6BB1",
"P      c #C0C9D0",
"Q      c #92B9E8",
"R      c #8AB9E8",
"S      c #3C73B9",
"T      c #99C0E8",
"U      c #92C0E8",
"V      c #7AAAE0",
"W      c #4373AA",
"X      c #A1C9F7",
"Y      c #99C0F0",
"Z      c #92C0F0",
"`      c #6BA1D8",
" .     c #4C82C0",
"..     c #6282B1",
"+.     c #AAD8FF",
"@.     c #AAD0FF",
"#.     c #A1D0FF",
"$.     c #99C9F7",
"%.     c #73AAE0",
"&.     c #6299D0",
"*.     c #8299B9",
"=.     c #B9E0FF",
"-.     c #B1E0FF",
";.     c #B1D8FF",
">.     c #82B9E8",
",.     c #5B8AB9",
"'.     c #A1B1C0",
").     c #B9E8FF",
"!.     c #92C0F7",
"~.     c #8AB9F0",
"{.     c #6B99D8",
"].     c #82A1B9",
"^.     c #B1B9C0",
"/.     c #C0F0FF",
"(.     c #92C9F7",
"_.     c #7399B9",
":.     c #AAB1B9",
"<.     c #B1E8FF",
"[.     c #99D0FF",
"}.     c #82B9F0",
"|.     c #7AA1C0",
"1.     c #99AAB9",
"2.     c #AAE0FF",
"3.     c #82A1C0",
"4.     c #99A1B1",
"5.     c #B1C0C9",
"6.     c #E0E0E8",
"7.     c #99C9F0",
"8.     c #8AB1D0",
"9.     c #8A99B1",
"0.     c #A1AAB1",
"a.     c #8292A1",
"b.     c #829299",
"c.     c #8A99A1",
"d.     c #9299AA",
"e.     c #AAB9C0",
"f.     c #B9C0C9",
". + @ # $ % & * * * * * * ",
"= - ; > , ' # ) * * * * * ",
"! ! ~ { ] ^ / ( _ & * * * ",
": : < [ } | 1 2 3 4 5 * * ",
"6 7 7 7 : 8 9 0 a b c 5 * ",
"d 6 6 e f g h i j k l m n ",
"e e e o p q r s t u k v 5 ",
"w w w x y z A B s C D E & ",
"y F F G G H A I B s J K L ",
"M M y F F G H N I B O J P ",
"Q R x M M F G H N I S i P ",
"T T U Q R M V G H A B W P ",
"X X X Y Z R M V ` N  ...P ",
"+.@.@.#.$.Z R M %.&.I *.P ",
"=.-.;.+.#.$.Z >.V H ,.'.m ",
").).).-.+.#.!.~.V {.].^.L ",
"/.).).-.+.#.(.~.V _.:.P & ",
").).<.-.+.[.!.}.|.1.^.L 5 ",
"-.-.2.+.#.$.M 3.4.5.m 6.n ",
"+.#.7.Q 8.*.9.0.5.P & 5 * ",
"a.a.b.c.d.4.:.^.m & 5 * * ",
"0.0.:.:.e.^.P L 6.5 * * * ",
"f.f.P P m L & 5 * * * * * "};
/* XPM */
static const char * const aqua_btn_def_right2_xpm[] = {
"13 23 137 2",
"       c None",
".      c #384D81",
"+      c #304681",
"@      c #303F7A",
"#      c #464D81",
"$      c #646B91",
"%      c #9898AE",
"&      c #DADAE1",
"*      c #FFFFFF",
"=      c #7A91BD",
"-      c #7388B5",
";      c #6B81B5",
">      c #5C73A6",
",      c #385591",
"'      c #213073",
")      c #9898B5",
"!      c #9FB5D3",
"~      c #9FAED3",
"{      c #98AECC",
"]      c #8198C4",
"^      c #4D73A6",
"/      c #213F81",
"(      c #1A2873",
"_      c #737A9F",
":      c #AEC4E1",
"<      c #AEC4DA",
"[      c #AEBDDA",
"}      c #A6B5DA",
"|      c #88A6CC",
"1      c #466BA6",
"2      c #1A3F81",
"3      c #1A387A",
"4      c #737AA6",
"5      c #E9E9E9",
"6      c #B5CCE9",
"7      c #B5CCE1",
"8      c #A6BDDA",
"9      c #6B91C4",
"0      c #305598",
"a      c #1A4688",
"b      c #1A3881",
"c      c #9198B5",
"d      c #B5D3E9",
"e      c #AECCE9",
"f      c #AECCE1",
"g      c #A6C4E1",
"h      c #819FCC",
"i      c #3F6BAE",
"j      c #285598",
"k      c #214691",
"l      c #3F5C91",
"m      c #CCD3DA",
"n      c #F1F1F1",
"o      c #A6C4E9",
"p      c #9FC4E1",
"q      c #98BDDA",
"r      c #739FCC",
"s      c #467AB5",
"t      c #3864AE",
"u      c #28559F",
"v      c #8898B5",
"w      c #98BDE1",
"x      c #91B5E1",
"y      c #88AEDA",
"z      c #7AA6D3",
"A      c #6498CC",
"B      c #4D81C4",
"C      c #386BAE",
"D      c #305C9F",
"E      c #647AA6",
"F      c #81AEDA",
"G      c #7AA6DA",
"H      c #739FD3",
"I      c #5C91CC",
"J      c #3864A6",
"K      c #466B9F",
"L      c #D3DADA",
"M      c #88B5E1",
"N      c #6498D3",
"O      c #4673B5",
"P      c #C4CCD3",
"Q      c #98BDE9",
"R      c #91BDE9",
"S      c #467ABD",
"T      c #9FC4E9",
"U      c #98C4E9",
"V      c #81AEE1",
"W      c #4D7AAE",
"X      c #A6CCF8",
"Y      c #9FC4F1",
"Z      c #98C4F1",
"`      c #73A6DA",
" .     c #5588C4",
"..     c #6B88B5",
"+.     c #AEDAFF",
"@.     c #AED3FF",
"#.     c #A6D3FF",
"$.     c #9FCCF8",
"%.     c #7AAEE1",
"&.     c #6B9FD3",
"*.     c #889FBD",
"=.     c #BDE1FF",
"-.     c #B5E1FF",
";.     c #B5DAFF",
">.     c #88BDE9",
",.     c #6491BD",
"'.     c #A6B5C4",
").     c #BDE9FF",
"!.     c #98C4F8",
"~.     c #91BDF1",
"{.     c #739FDA",
"].     c #88A6BD",
"^.     c #B5BDC4",
"/.     c #C4F1FF",
"(.     c #98CCF8",
"_.     c #7A9FBD",
":.     c #AEB5BD",
"<.     c #B5E9FF",
"[.     c #9FD3FF",
"}.     c #88BDF1",
"|.     c #81A6C4",
"1.     c #9FAEBD",
"2.     c #AEE1FF",
"3.     c #88A6C4",
"4.     c #9FA6B5",
"5.     c #B5C4CC",
"6.     c #E1E1E9",
"7.     c #9FCCF1",
"8.     c #91B5D3",
"9.     c #919FB5",
"0.     c #A6AEB5",
"a.     c #8898A6",
"b.     c #88989F",
"c.     c #919FA6",
"d.     c #989FAE",
"e.     c #AEBDC4",
"f.     c #BDC4CC",
". + @ # $ % & * * * * * * ",
"= - ; > , ' # ) * * * * * ",
"! ! ~ { ] ^ / ( _ & * * * ",
": : < [ } | 1 2 3 4 5 * * ",
"6 7 7 7 : 8 9 0 a b c 5 * ",
"d 6 6 e f g h i j k l m n ",
"e e e o p q r s t u k v 5 ",
"w w w x y z A B s C D E & ",
"y F F G G H A I B s J K L ",
"M M y F F G H N I B O J P ",
"Q R x M M F G H N I S i P ",
"T T U Q R M V G H A B W P ",
"X X X Y Z R M V ` N  ...P ",
"+.@.@.#.$.Z R M %.&.I *.P ",
"=.-.;.+.#.$.Z >.V H ,.'.m ",
").).).-.+.#.!.~.V {.].^.L ",
"/.).).-.+.#.(.~.V _.:.P & ",
").).<.-.+.[.!.}.|.1.^.L 5 ",
"-.-.2.+.#.$.M 3.4.5.m 6.n ",
"+.#.7.Q 8.*.9.0.5.P & 5 * ",
"a.a.b.c.d.4.:.^.m & 5 * * ",
"0.0.:.:.e.^.P L 6.5 * * * ",
"f.f.P P m L & 5 * * * * * "};
/* XPM */
static const char * const aqua_btn_def_right3_xpm[] = {
"13 23 137 2",
"       c None",
".      c #435788",
"+      c #3B5188",
"@      c #3B4A82",
"#      c #515788",
"$      c #6C7397",
"%      c #9E9EB3",
"&      c #DCDCE3",
"*      c #FFFFFF",
"=      c #8297C0",
"-      c #7B8FBA",
";      c #7388BA",
">      c #667BAB",
",      c #435F97",
"'      c #2E3B7B",
")      c #9E9EBA",
"!      c #A4BAD5",
"~      c #A4B3D5",
"{      c #9EB3CF",
"]      c #889EC7",
"^      c #577BAB",
"/      c #2E4A88",
"(      c #27357B",
"_      c #7B82A4",
":      c #B3C7E3",
"<      c #B3C7DC",
"[      c #B3C0DC",
"}      c #ABBADC",
"|      c #8FABCF",
"1      c #5173AB",
"2      c #274A88",
"3      c #274382",
"4      c #7B82AB",
"5      c #EBEBEB",
"6      c #BACFEB",
"7      c #BACFE3",
"8      c #ABC0DC",
"9      c #7397C7",
"0      c #3B5F9E",
"a      c #27518F",
"b      c #274388",
"c      c #979EBA",
"d      c #BAD5EB",
"e      c #B3CFEB",
"f      c #B3CFE3",
"g      c #ABC7E3",
"h      c #88A4CF",
"i      c #4A73B3",
"j      c #355F9E",
"k      c #2E5197",
"l      c #4A6697",
"m      c #CFD5DC",
"n      c #F1F1F1",
"o      c #ABC7EB",
"p      c #A4C7E3",
"q      c #9EC0DC",
"r      c #7BA4CF",
"s      c #5182BA",
"t      c #436CB3",
"u      c #355FA4",
"v      c #8F9EBA",
"w      c #9EC0E3",
"x      c #97BAE3",
"y      c #8FB3DC",
"z      c #82ABD5",
"A      c #6C9ECF",
"B      c #5788C7",
"C      c #4373B3",
"D      c #3B66A4",
"E      c #6C82AB",
"F      c #88B3DC",
"G      c #82ABDC",
"H      c #7BA4D5",
"I      c #6697CF",
"J      c #436CAB",
"K      c #5173A4",
"L      c #D5DCDC",
"M      c #8FBAE3",
"N      c #6C9ED5",
"O      c #517BBA",
"P      c #C7CFD5",
"Q      c #9EC0EB",
"R      c #97C0EB",
"S      c #5182C0",
"T      c #A4C7EB",
"U      c #9EC7EB",
"V      c #88B3E3",
"W      c #5782B3",
"X      c #ABCFF8",
"Y      c #A4C7F1",
"Z      c #9EC7F1",
"`      c #7BABDC",
" .     c #5F8FC7",
"..     c #738FBA",
"+.     c #B3DCFF",
"@.     c #B3D5FF",
"#.     c #ABD5FF",
"$.     c #A4CFF8",
"%.     c #82B3E3",
"&.     c #73A4D5",
"*.     c #8FA4C0",
"=.     c #C0E3FF",
"-.     c #BAE3FF",
";.     c #BADCFF",
">.     c #8FC0EB",
",.     c #6C97C0",
"'.     c #ABBAC7",
").     c #C0EBFF",
"!.     c #9EC7F8",
"~.     c #97C0F1",
"{.     c #7BA4DC",
"].     c #8FABC0",
"^.     c #BAC0C7",
"/.     c #C7F1FF",
"(.     c #9ECFF8",
"_.     c #82A4C0",
":.     c #B3BAC0",
"<.     c #BAEBFF",
"[.     c #A4D5FF",
"}.     c #8FC0F1",
"|.     c #88ABC7",
"1.     c #A4B3C0",
"2.     c #B3E3FF",
"3.     c #8FABC7",
"4.     c #A4ABBA",
"5.     c #BAC7CF",
"6.     c #E3E3EB",
"7.     c #A4CFF1",
"8.     c #97BAD5",
"9.     c #97A4BA",
"0.     c #ABB3BA",
"a.     c #8F9EAB",
"b.     c #8F9EA4",
"c.     c #97A4AB",
"d.     c #9EA4B3",
"e.     c #B3C0C7",
"f.     c #C0C7CF",
". + @ # $ % & * * * * * * ",
"= - ; > , ' # ) * * * * * ",
"! ! ~ { ] ^ / ( _ & * * * ",
": : < [ } | 1 2 3 4 5 * * ",
"6 7 7 7 : 8 9 0 a b c 5 * ",
"d 6 6 e f g h i j k l m n ",
"e e e o p q r s t u k v 5 ",
"w w w x y z A B s C D E & ",
"y F F G G H A I B s J K L ",
"M M y F F G H N I B O J P ",
"Q R x M M F G H N I S i P ",
"T T U Q R M V G H A B W P ",
"X X X Y Z R M V ` N  ...P ",
"+.@.@.#.$.Z R M %.&.I *.P ",
"=.-.;.+.#.$.Z >.V H ,.'.m ",
").).).-.+.#.!.~.V {.].^.L ",
"/.).).-.+.#.(.~.V _.:.P & ",
").).<.-.+.[.!.}.|.1.^.L 5 ",
"-.-.2.+.#.$.M 3.4.5.m 6.n ",
"+.#.7.Q 8.*.9.0.5.P & 5 * ",
"a.a.b.c.d.4.:.^.m & 5 * * ",
"0.0.:.:.e.^.P L 6.5 * * * ",
"f.f.P P m L & 5 * * * * * "};
/* XPM */
static const char * const aqua_btn_def_right4_xpm[] = {
"13 23 137 2",
"       c None",
".      c #4E6190",
"+      c #475B90",
"@      c #475589",
"#      c #5B6190",
"$      c #757C9D",
"%      c #A3A3B7",
"&      c #DEDEE5",
"*      c #FFFFFF",
"=      c #899DC4",
"-      c #8396BE",
";      c #7C90BE",
">      c #6F83B0",
",      c #4E699D",
"'      c #3A4783",
")      c #A3A3BE",
"!      c #AABED8",
"~      c #AAB7D8",
"{      c #A3B7D2",
"]      c #90A3CA",
"^      c #6183B0",
"/      c #3A5590",
"(      c #344183",
"_      c #8389AA",
":      c #B7CAE5",
"<      c #B7CADE",
"[      c #B7C4DE",
"}      c #B0BEDE",
"|      c #96B0D2",
"1      c #5B7CB0",
"2      c #345590",
"3      c #344E89",
"4      c #8389B0",
"5      c #ECECEC",
"6      c #BED2EC",
"7      c #BED2E5",
"8      c #B0C4DE",
"9      c #7C9DCA",
"0      c #4769A3",
"a      c #345B96",
"b      c #344E90",
"c      c #9DA3BE",
"d      c #BED8EC",
"e      c #B7D2EC",
"f      c #B7D2E5",
"g      c #B0CAE5",
"h      c #90AAD2",
"i      c #557CB7",
"j      c #4169A3",
"k      c #3A5B9D",
"l      c #556F9D",
"m      c #D2D8DE",
"n      c #F2F2F2",
"o      c #B0CAEC",
"p      c #AACAE5",
"q      c #A3C4DE",
"r      c #83AAD2",
"s      c #5B89BE",
"t      c #4E75B7",
"u      c #4169AA",
"v      c #96A3BE",
"w      c #A3C4E5",
"x      c #9DBEE5",
"y      c #96B7DE",
"z      c #89B0D8",
"A      c #75A3D2",
"B      c #6190CA",
"C      c #4E7CB7",
"D      c #476FAA",
"E      c #7589B0",
"F      c #90B7DE",
"G      c #89B0DE",
"H      c #83AAD8",
"I      c #6F9DD2",
"J      c #4E75B0",
"K      c #5B7CAA",
"L      c #D8DEDE",
"M      c #96BEE5",
"N      c #75A3D8",
"O      c #5B83BE",
"P      c #CAD2D8",
"Q      c #A3C4EC",
"R      c #9DC4EC",
"S      c #5B89C4",
"T      c #AACAEC",
"U      c #A3CAEC",
"V      c #90B7E5",
"W      c #6189B7",
"X      c #B0D2F9",
"Y      c #AACAF2",
"Z      c #A3CAF2",
"`      c #83B0DE",
" .     c #6996CA",
"..     c #7C96BE",
"+.     c #B7DEFF",
"@.     c #B7D8FF",
"#.     c #B0D8FF",
"$.     c #AAD2F9",
"%.     c #89B7E5",
"&.     c #7CAAD8",
"*.     c #96AAC4",
"=.     c #C4E5FF",
"-.     c #BEE5FF",
";.     c #BEDEFF",
">.     c #96C4EC",
",.     c #759DC4",
"'.     c #B0BECA",
").     c #C4ECFF",
"!.     c #A3CAF9",
"~.     c #9DC4F2",
"{.     c #83AADE",
"].     c #96B0C4",
"^.     c #BEC4CA",
"/.     c #CAF2FF",
"(.     c #A3D2F9",
"_.     c #89AAC4",
":.     c #B7BEC4",
"<.     c #BEECFF",
"[.     c #AAD8FF",
"}.     c #96C4F2",
"|.     c #90B0CA",
"1.     c #AAB7C4",
"2.     c #B7E5FF",
"3.     c #96B0CA",
"4.     c #AAB0BE",
"5.     c #BECAD2",
"6.     c #E5E5EC",
"7.     c #AAD2F2",
"8.     c #9DBED8",
"9.     c #9DAABE",
"0.     c #B0B7BE",
"a.     c #96A3B0",
"b.     c #96A3AA",
"c.     c #9DAAB0",
"d.     c #A3AAB7",
"e.     c #B7C4CA",
"f.     c #C4CAD2",
". + @ # $ % & * * * * * * ",
"= - ; > , ' # ) * * * * * ",
"! ! ~ { ] ^ / ( _ & * * * ",
": : < [ } | 1 2 3 4 5 * * ",
"6 7 7 7 : 8 9 0 a b c 5 * ",
"d 6 6 e f g h i j k l m n ",
"e e e o p q r s t u k v 5 ",
"w w w x y z A B s C D E & ",
"y F F G G H A I B s J K L ",
"M M y F F G H N I B O J P ",
"Q R x M M F G H N I S i P ",
"T T U Q R M V G H A B W P ",
"X X X Y Z R M V ` N  ...P ",
"+.@.@.#.$.Z R M %.&.I *.P ",
"=.-.;.+.#.$.Z >.V H ,.'.m ",
").).).-.+.#.!.~.V {.].^.L ",
"/.).).-.+.#.(.~.V _.:.P & ",
").).<.-.+.[.!.}.|.1.^.L 5 ",
"-.-.2.+.#.$.M 3.4.5.m 6.n ",
"+.#.7.Q 8.*.9.0.5.P & 5 * ",
"a.a.b.c.d.4.:.^.m & 5 * * ",
"0.0.:.:.e.^.P L 6.5 * * * ",
"f.f.P P m L & 5 * * * * * "};
/* XPM */
static const char * const aqua_btn_def_right5_xpm[] = {
"13 23 137 2",
"       c None",
".      c #596B96",
"+      c #526596",
"@      c #525F90",
"#      c #656B96",
"$      c #7D83A3",
"%      c #A9A9BC",
"&      c #E0E0E6",
"*      c #FFFFFF",
"=      c #90A3C8",
"-      c #8A9CC2",
";      c #8396C2",
">      c #778AB5",
",      c #5971A3",
"'      c #46528A",
")      c #A9A9C2",
"!      c #AFC2DA",
"~      c #AFBCDA",
"{      c #A9BCD4",
"]      c #96A9CE",
"^      c #6B8AB5",
"/      c #465F96",
"(      c #404C8A",
"_      c #8A90AF",
":      c #BCCEE6",
"<      c #BCCEE0",
"[      c #BCC8E0",
"}      c #B5C2E0",
"|      c #9CB5D4",
"1      c #6583B5",
"2      c #405F96",
"3      c #405990",
"4      c #8A90B5",
"5      c #EDEDED",
"6      c #C2D4ED",
"7      c #C2D4E6",
"8      c #B5C8E0",
"9      c #83A3CE",
"0      c #5271A9",
"a      c #40659C",
"b      c #405996",
"c      c #A3A9C2",
"d      c #C2DAED",
"e      c #BCD4ED",
"f      c #BCD4E6",
"g      c #B5CEE6",
"h      c #96AFD4",
"i      c #5F83BC",
"j      c #4C71A9",
"k      c #4665A3",
"l      c #5F77A3",
"m      c #D4DAE0",
"n      c #F3F3F3",
"o      c #B5CEED",
"p      c #AFCEE6",
"q      c #A9C8E0",
"r      c #8AAFD4",
"s      c #6590C2",
"t      c #597DBC",
"u      c #4C71AF",
"v      c #9CA9C2",
"w      c #A9C8E6",
"x      c #A3C2E6",
"y      c #9CBCE0",
"z      c #90B5DA",
"A      c #7DA9D4",
"B      c #6B96CE",
"C      c #5983BC",
"D      c #5277AF",
"E      c #7D90B5",
"F      c #96BCE0",
"G      c #90B5E0",
"H      c #8AAFDA",
"I      c #77A3D4",
"J      c #597DB5",
"K      c #6583AF",
"L      c #DAE0E0",
"M      c #9CC2E6",
"N      c #7DA9DA",
"O      c #658AC2",
"P      c #CED4DA",
"Q      c #A9C8ED",
"R      c #A3C8ED",
"S      c #6590C8",
"T      c #AFCEED",
"U      c #A9CEED",
"V      c #96BCE6",
"W      c #6B90BC",
"X      c #B5D4F9",
"Y      c #AFCEF3",
"Z      c #A9CEF3",
"`      c #8AB5E0",
" .     c #719CCE",
"..     c #839CC2",
"+.     c #BCE0FF",
"@.     c #BCDAFF",
"#.     c #B5DAFF",
"$.     c #AFD4F9",
"%.     c #90BCE6",
"&.     c #83AFDA",
"*.     c #9CAFC8",
"=.     c #C8E6FF",
"-.     c #C2E6FF",
";.     c #C2E0FF",
">.     c #9CC8ED",
",.     c #7DA3C8",
"'.     c #B5C2CE",
").     c #C8EDFF",
"!.     c #A9CEF9",
"~.     c #A3C8F3",
"{.     c #8AAFE0",
"].     c #9CB5C8",
"^.     c #C2C8CE",
"/.     c #CEF3FF",
"(.     c #A9D4F9",
"_.     c #90AFC8",
":.     c #BCC2C8",
"<.     c #C2EDFF",
"[.     c #AFDAFF",
"}.     c #9CC8F3",
"|.     c #96B5CE",
"1.     c #AFBCC8",
"2.     c #BCE6FF",
"3.     c #9CB5CE",
"4.     c #AFB5C2",
"5.     c #C2CED4",
"6.     c #E6E6ED",
"7.     c #AFD4F3",
"8.     c #A3C2DA",
"9.     c #A3AFC2",
"0.     c #B5BCC2",
"a.     c #9CA9B5",
"b.     c #9CA9AF",
"c.     c #A3AFB5",
"d.     c #A9AFBC",
"e.     c #BCC8CE",
"f.     c #C8CED4",
". + @ # $ % & * * * * * * ",
"= - ; > , ' # ) * * * * * ",
"! ! ~ { ] ^ / ( _ & * * * ",
": : < [ } | 1 2 3 4 5 * * ",
"6 7 7 7 : 8 9 0 a b c 5 * ",
"d 6 6 e f g h i j k l m n ",
"e e e o p q r s t u k v 5 ",
"w w w x y z A B s C D E & ",
"y F F G G H A I B s J K L ",
"M M y F F G H N I B O J P ",
"Q R x M M F G H N I S i P ",
"T T U Q R M V G H A B W P ",
"X X X Y Z R M V ` N  ...P ",
"+.@.@.#.$.Z R M %.&.I *.P ",
"=.-.;.+.#.$.Z >.V H ,.'.m ",
").).).-.+.#.!.~.V {.].^.L ",
"/.).).-.+.#.(.~.V _.:.P & ",
").).<.-.+.[.!.}.|.1.^.L 5 ",
"-.-.2.+.#.$.M 3.4.5.m 6.n ",
"+.#.7.Q 8.*.9.0.5.P & 5 * ",
"a.a.b.c.d.4.:.^.m & 5 * * ",
"0.0.:.:.e.^.P L 6.5 * * * ",
"f.f.P P m L & 5 * * * * * "};
/* XPM */
static const char * const aqua_btn_def_right6_xpm[] = {
"13 23 137 2",
"       c None",
".      c #65769E",
"+      c #5F709E",
"@      c #5F6A98",
"#      c #70769E",
"$      c #878CAA",
"%      c #AFAFC1",
"&      c #E3E3E8",
"*      c #FFFFFF",
"=      c #98AACC",
"-      c #93A3C6",
";      c #8C9EC6",
">      c #8193BA",
",      c #657CAA",
"'      c #545F93",
")      c #AFAFC6",
"!      c #B5C6DD",
"~      c #B5C1DD",
"{      c #AFC1D7",
"]      c #9EAFD1",
"^      c #7693BA",
"/      c #546A9E",
"(      c #4E5993",
"_      c #9398B5",
":      c #C1D1E8",
"<      c #C1D1E3",
"[      c #C1CCE3",
"}      c #BAC6E3",
"|      c #A3BAD7",
"1      c #708CBA",
"2      c #4E6A9E",
"3      c #4E6598",
"4      c #9398BA",
"5      c #EEEEEE",
"6      c #C6D7EE",
"7      c #C6D7E8",
"8      c #BACCE3",
"9      c #8CAAD1",
"0      c #5F7CAF",
"a      c #4E70A3",
"b      c #4E659E",
"c      c #AAAFC6",
"d      c #C6DDEE",
"e      c #C1D7EE",
"f      c #C1D7E8",
"g      c #BAD1E8",
"h      c #9EB5D7",
"i      c #6A8CC1",
"j      c #597CAF",
"k      c #5470AA",
"l      c #6A81AA",
"m      c #D7DDE3",
"n      c #F4F4F4",
"o      c #BAD1EE",
"p      c #B5D1E8",
"q      c #AFCCE3",
"r      c #93B5D7",
"s      c #7098C6",
"t      c #6587C1",
"u      c #597CB5",
"v      c #A3AFC6",
"w      c #AFCCE8",
"x      c #AAC6E8",
"y      c #A3C1E3",
"z      c #98BADD",
"A      c #87AFD7",
"B      c #769ED1",
"C      c #658CC1",
"D      c #5F81B5",
"E      c #8798BA",
"F      c #9EC1E3",
"G      c #98BAE3",
"H      c #93B5DD",
"I      c #81AAD7",
"J      c #6587BA",
"K      c #708CB5",
"L      c #DDE3E3",
"M      c #A3C6E8",
"N      c #87AFDD",
"O      c #7093C6",
"P      c #D1D7DD",
"Q      c #AFCCEE",
"R      c #AACCEE",
"S      c #7098CC",
"T      c #B5D1EE",
"U      c #AFD1EE",
"V      c #9EC1E8",
"W      c #7698C1",
"X      c #BAD7F9",
"Y      c #B5D1F4",
"Z      c #AFD1F4",
"`      c #93BAE3",
" .     c #7CA3D1",
"..     c #8CA3C6",
"+.     c #C1E3FF",
"@.     c #C1DDFF",
"#.     c #BADDFF",
"$.     c #B5D7F9",
"%.     c #98C1E8",
"&.     c #8CB5DD",
"*.     c #A3B5CC",
"=.     c #CCE8FF",
"-.     c #C6E8FF",
";.     c #C6E3FF",
">.     c #A3CCEE",
",.     c #87AACC",
"'.     c #BAC6D1",
").     c #CCEEFF",
"!.     c #AFD1F9",
"~.     c #AACCF4",
"{.     c #93B5E3",
"].     c #A3BACC",
"^.     c #C6CCD1",
"/.     c #D1F4FF",
"(.     c #AFD7F9",
"_.     c #98B5CC",
":.     c #C1C6CC",
"<.     c #C6EEFF",
"[.     c #B5DDFF",
"}.     c #A3CCF4",
"|.     c #9EBAD1",
"1.     c #B5C1CC",
"2.     c #C1E8FF",
"3.     c #A3BAD1",
"4.     c #B5BAC6",
"5.     c #C6D1D7",
"6.     c #E8E8EE",
"7.     c #B5D7F4",
"8.     c #AAC6DD",
"9.     c #AAB5C6",
"0.     c #BAC1C6",
"a.     c #A3AFBA",
"b.     c #A3AFB5",
"c.     c #AAB5BA",
"d.     c #AFB5C1",
"e.     c #C1CCD1",
"f.     c #CCD1D7",
". + @ # $ % & * * * * * * ",
"= - ; > , ' # ) * * * * * ",
"! ! ~ { ] ^ / ( _ & * * * ",
": : < [ } | 1 2 3 4 5 * * ",
"6 7 7 7 : 8 9 0 a b c 5 * ",
"d 6 6 e f g h i j k l m n ",
"e e e o p q r s t u k v 5 ",
"w w w x y z A B s C D E & ",
"y F F G G H A I B s J K L ",
"M M y F F G H N I B O J P ",
"Q R x M M F G H N I S i P ",
"T T U Q R M V G H A B W P ",
"X X X Y Z R M V ` N  ...P ",
"+.@.@.#.$.Z R M %.&.I *.P ",
"=.-.;.+.#.$.Z >.V H ,.'.m ",
").).).-.+.#.!.~.V {.].^.L ",
"/.).).-.+.#.(.~.V _.:.P & ",
").).<.-.+.[.!.}.|.1.^.L 5 ",
"-.-.2.+.#.$.M 3.4.5.m 6.n ",
"+.#.7.Q 8.*.9.0.5.P & 5 * ",
"a.a.b.c.d.4.:.^.m & 5 * * ",
"0.0.:.:.e.^.P L 6.5 * * * ",
"f.f.P P m L & 5 * * * * * "};
/* XPM */
static const char * const aqua_btn_def_right7_xpm[] = {
"13 23 137 2",
"       c None",
".      c #6F7FA4",
"+      c #6A7AA4",
"@      c #6A759F",
"#      c #7A7FA4",
"$      c #8F94AF",
"%      c #B5B5C5",
"&      c #E4E4EA",
"*      c #FFFFFF",
"=      c #9FAFCF",
"-      c #9AAACA",
";      c #94A4CA",
">      c #8A9ABF",
",      c #6F85AF",
"'      c #5F6A9A",
")      c #B5B5CA",
"!      c #BACADF",
"~      c #BAC5DF",
"{      c #B5C5DA",
"]      c #A4B5D4",
"^      c #7F9ABF",
"/      c #5F75A4",
"(      c #5A649A",
"_      c #9A9FBA",
":      c #C5D4EA",
"<      c #C5D4E4",
"[      c #C5CFE4",
"}      c #BFCAE4",
"|      c #AABFDA",
"1      c #7A94BF",
"2      c #5A75A4",
"3      c #5A6F9F",
"4      c #9A9FBF",
"5      c #EFEFEF",
"6      c #CADAEF",
"7      c #CADAEA",
"8      c #BFCFE4",
"9      c #94AFD4",
"0      c #6A85B5",
"a      c #5A7AAA",
"b      c #5A6FA4",
"c      c #AFB5CA",
"d      c #CADFEF",
"e      c #C5DAEF",
"f      c #C5DAEA",
"g      c #BFD4EA",
"h      c #A4BADA",
"i      c #7594C5",
"j      c #6485B5",
"k      c #5F7AAF",
"l      c #758AAF",
"m      c #DADFE4",
"n      c #F5F5F5",
"o      c #BFD4EF",
"p      c #BAD4EA",
"q      c #B5CFE4",
"r      c #9ABADA",
"s      c #7A9FCA",
"t      c #6F8FC5",
"u      c #6485BA",
"v      c #AAB5CA",
"w      c #B5CFEA",
"x      c #AFCAEA",
"y      c #AAC5E4",
"z      c #9FBFDF",
"A      c #8FB5DA",
"B      c #7FA4D4",
"C      c #6F94C5",
"D      c #6A8ABA",
"E      c #8F9FBF",
"F      c #A4C5E4",
"G      c #9FBFE4",
"H      c #9ABADF",
"I      c #8AAFDA",
"J      c #6F8FBF",
"K      c #7A94BA",
"L      c #DFE4E4",
"M      c #AACAEA",
"N      c #8FB5DF",
"O      c #7A9ACA",
"P      c #D4DADF",
"Q      c #B5CFEF",
"R      c #AFCFEF",
"S      c #7A9FCF",
"T      c #BAD4EF",
"U      c #B5D4EF",
"V      c #A4C5EA",
"W      c #7F9FC5",
"X      c #BFDAFA",
"Y      c #BAD4F5",
"Z      c #B5D4F5",
"`      c #9ABFE4",
" .     c #85AAD4",
"..     c #94AACA",
"+.     c #C5E4FF",
"@.     c #C5DFFF",
"#.     c #BFDFFF",
"$.     c #BADAFA",
"%.     c #9FC5EA",
"&.     c #94BADF",
"*.     c #AABACF",
"=.     c #CFEAFF",
"-.     c #CAEAFF",
";.     c #CAE4FF",
">.     c #AACFEF",
",.     c #8FAFCF",
"'.     c #BFCAD4",
").     c #CFEFFF",
"!.     c #B5D4FA",
"~.     c #AFCFF5",
"{.     c #9ABAE4",
"].     c #AABFCF",
"^.     c #CACFD4",
"/.     c #D4F5FF",
"(.     c #B5DAFA",
"_.     c #9FBACF",
":.     c #C5CACF",
"<.     c #CAEFFF",
"[.     c #BADFFF",
"}.     c #AACFF5",
"|.     c #A4BFD4",
"1.     c #BAC5CF",
"2.     c #C5EAFF",
"3.     c #AABFD4",
"4.     c #BABFCA",
"5.     c #CAD4DA",
"6.     c #EAEAEF",
"7.     c #BADAF5",
"8.     c #AFCADF",
"9.     c #AFBACA",
"0.     c #BFC5CA",
"a.     c #AAB5BF",
"b.     c #AAB5BA",
"c.     c #AFBABF",
"d.     c #B5BAC5",
"e.     c #C5CFD4",
"f.     c #CFD4DA",
". + @ # $ % & * * * * * * ",
"= - ; > , ' # ) * * * * * ",
"! ! ~ { ] ^ / ( _ & * * * ",
": : < [ } | 1 2 3 4 5 * * ",
"6 7 7 7 : 8 9 0 a b c 5 * ",
"d 6 6 e f g h i j k l m n ",
"e e e o p q r s t u k v 5 ",
"w w w x y z A B s C D E & ",
"y F F G G H A I B s J K L ",
"M M y F F G H N I B O J P ",
"Q R x M M F G H N I S i P ",
"T T U Q R M V G H A B W P ",
"X X X Y Z R M V ` N  ...P ",
"+.@.@.#.$.Z R M %.&.I *.P ",
"=.-.;.+.#.$.Z >.V H ,.'.m ",
").).).-.+.#.!.~.V {.].^.L ",
"/.).).-.+.#.(.~.V _.:.P & ",
").).<.-.+.[.!.}.|.1.^.L 5 ",
"-.-.2.+.#.$.M 3.4.5.m 6.n ",
"+.#.7.Q 8.*.9.0.5.P & 5 * ",
"a.a.b.c.d.4.:.^.m & 5 * * ",
"0.0.:.:.e.^.P L 6.5 * * * ",
"f.f.P P m L & 5 * * * * * "};
/* XPM */
static const char * const aqua_btn_def_right8_xpm[] = {
"13 23 137 2",
"       c None",
".      c #7B89AC",
"+      c #7584AC",
"@      c #757FA7",
"#      c #8489AC",
"$      c #989DB6",
"%      c #BABAC9",
"&      c #E7E7EB",
"*      c #FFFFFF",
"=      c #A7B6D3",
"-      c #A2B0CE",
";      c #9DACCE",
">      c #93A2C4",
",      c #7B8EB6",
"'      c #6C75A2",
")      c #BABACE",
"!      c #BFCEE2",
"~      c #BFC9E2",
"{      c #BAC9DD",
"]      c #ACBAD8",
"^      c #89A2C4",
"/      c #6C7FAC",
"(      c #6771A2",
"_      c #A2A7BF",
":      c #C9D8EB",
"<      c #C9D8E7",
"[      c #C9D3E7",
"}      c #C4CEE7",
"|      c #B0C4DD",
"1      c #849DC4",
"2      c #677FAC",
"3      c #677BA7",
"4      c #A2A7C4",
"5      c #F1F1F1",
"6      c #CEDDF1",
"7      c #CEDDEB",
"8      c #C4D3E7",
"9      c #9DB6D8",
"0      c #758EBA",
"a      c #6784B0",
"b      c #677BAC",
"c      c #B6BACE",
"d      c #CEE2F1",
"e      c #C9DDF1",
"f      c #C9DDEB",
"g      c #C4D8EB",
"h      c #ACBFDD",
"i      c #7F9DC9",
"j      c #718EBA",
"k      c #6C84B6",
"l      c #7F93B6",
"m      c #DDE2E7",
"n      c #F5F5F5",
"o      c #C4D8F1",
"p      c #BFD8EB",
"q      c #BAD3E7",
"r      c #A2BFDD",
"s      c #84A7CE",
"t      c #7B98C9",
"u      c #718EBF",
"v      c #B0BACE",
"w      c #BAD3EB",
"x      c #B6CEEB",
"y      c #B0C9E7",
"z      c #A7C4E2",
"A      c #98BADD",
"B      c #89ACD8",
"C      c #7B9DC9",
"D      c #7593BF",
"E      c #98A7C4",
"F      c #ACC9E7",
"G      c #A7C4E7",
"H      c #A2BFE2",
"I      c #93B6DD",
"J      c #7B98C4",
"K      c #849DBF",
"L      c #E2E7E7",
"M      c #B0CEEB",
"N      c #98BAE2",
"O      c #84A2CE",
"P      c #D8DDE2",
"Q      c #BAD3F1",
"R      c #B6D3F1",
"S      c #84A7D3",
"T      c #BFD8F1",
"U      c #BAD8F1",
"V      c #ACC9EB",
"W      c #89A7C9",
"X      c #C4DDFA",
"Y      c #BFD8F5",
"Z      c #BAD8F5",
"`      c #A2C4E7",
" .     c #8EB0D8",
"..     c #9DB0CE",
"+.     c #C9E7FF",
"@.     c #C9E2FF",
"#.     c #C4E2FF",
"$.     c #BFDDFA",
"%.     c #A7C9EB",
"&.     c #9DBFE2",
"*.     c #B0BFD3",
"=.     c #D3EBFF",
"-.     c #CEEBFF",
";.     c #CEE7FF",
">.     c #B0D3F1",
",.     c #98B6D3",
"'.     c #C4CED8",
").     c #D3F1FF",
"!.     c #BAD8FA",
"~.     c #B6D3F5",
"{.     c #A2BFE7",
"].     c #B0C4D3",
"^.     c #CED3D8",
"/.     c #D8F5FF",
"(.     c #BADDFA",
"_.     c #A7BFD3",
":.     c #C9CED3",
"<.     c #CEF1FF",
"[.     c #BFE2FF",
"}.     c #B0D3F5",
"|.     c #ACC4D8",
"1.     c #BFC9D3",
"2.     c #C9EBFF",
"3.     c #B0C4D8",
"4.     c #BFC4CE",
"5.     c #CED8DD",
"6.     c #EBEBF1",
"7.     c #BFDDF5",
"8.     c #B6CEE2",
"9.     c #B6BFCE",
"0.     c #C4C9CE",
"a.     c #B0BAC4",
"b.     c #B0BABF",
"c.     c #B6BFC4",
"d.     c #BABFC9",
"e.     c #C9D3D8",
"f.     c #D3D8DD",
". + @ # $ % & * * * * * * ",
"= - ; > , ' # ) * * * * * ",
"! ! ~ { ] ^ / ( _ & * * * ",
": : < [ } | 1 2 3 4 5 * * ",
"6 7 7 7 : 8 9 0 a b c 5 * ",
"d 6 6 e f g h i j k l m n ",
"e e e o p q r s t u k v 5 ",
"w w w x y z A B s C D E & ",
"y F F G G H A I B s J K L ",
"M M y F F G H N I B O J P ",
"Q R x M M F G H N I S i P ",
"T T U Q R M V G H A B W P ",
"X X X Y Z R M V ` N  ...P ",
"+.@.@.#.$.Z R M %.&.I *.P ",
"=.-.;.+.#.$.Z >.V H ,.'.m ",
").).).-.+.#.!.~.V {.].^.L ",
"/.).).-.+.#.(.~.V _.:.P & ",
").).<.-.+.[.!.}.|.1.^.L 5 ",
"-.-.2.+.#.$.M 3.4.5.m 6.n ",
"+.#.7.Q 8.*.9.0.5.P & 5 * ",
"a.a.b.c.d.4.:.^.m & 5 * * ",
"0.0.:.:.e.^.P L 6.5 * * * ",
"f.f.P P m L & 5 * * * * * "};
/* XPM */
static const char * const aqua_btn_def_right9_xpm[] = {
"13 23 137 2",
"       c None",
".      c #8592B2",
"+      c #808EB2",
"@      c #808AAE",
"#      c #8E92B2",
"$      c #A0A4BB",
"%      c #C0C0CE",
"&      c #E8E8ED",
"*      c #FFFFFF",
"=      c #AEBBD6",
"-      c #A9B7D2",
";      c #A4B2D2",
">      c #9CA9C9",
",      c #8597BB",
"'      c #7780A9",
")      c #C0C0D2",
"!      c #C4D2E4",
"~      c #C4CEE4",
"{      c #C0CEE0",
"]      c #B2C0DB",
"^      c #92A9C9",
"/      c #778AB2",
"(      c #737CA9",
"_      c #A9AEC4",
":      c #CEDBED",
"<      c #CEDBE8",
"[      c #CED6E8",
"}      c #C9D2E8",
"|      c #B7C9E0",
"1      c #8EA4C9",
"2      c #738AB2",
"3      c #7385AE",
"4      c #A9AEC9",
"5      c #F2F2F2",
"6      c #D2E0F2",
"7      c #D2E0ED",
"8      c #C9D6E8",
"9      c #A4BBDB",
"0      c #8097C0",
"a      c #738EB7",
"b      c #7385B2",
"c      c #BBC0D2",
"d      c #D2E4F2",
"e      c #CEE0F2",
"f      c #CEE0ED",
"g      c #C9DBED",
"h      c #B2C4E0",
"i      c #8AA4CE",
"j      c #7C97C0",
"k      c #778EBB",
"l      c #8A9CBB",
"m      c #E0E4E8",
"n      c #F6F6F6",
"o      c #C9DBF2",
"p      c #C4DBED",
"q      c #C0D6E8",
"r      c #A9C4E0",
"s      c #8EAED2",
"t      c #85A0CE",
"u      c #7C97C4",
"v      c #B7C0D2",
"w      c #C0D6ED",
"x      c #BBD2ED",
"y      c #B7CEE8",
"z      c #AEC9E4",
"A      c #A0C0E0",
"B      c #92B2DB",
"C      c #85A4CE",
"D      c #809CC4",
"E      c #A0AEC9",
"F      c #B2CEE8",
"G      c #AEC9E8",
"H      c #A9C4E4",
"I      c #9CBBE0",
"J      c #85A0C9",
"K      c #8EA4C4",
"L      c #E4E8E8",
"M      c #B7D2ED",
"N      c #A0C0E4",
"O      c #8EA9D2",
"P      c #DBE0E4",
"Q      c #C0D6F2",
"R      c #BBD6F2",
"S      c #8EAED6",
"T      c #C4DBF2",
"U      c #C0DBF2",
"V      c #B2CEED",
"W      c #92AECE",
"X      c #C9E0FB",
"Y      c #C4DBF6",
"Z      c #C0DBF6",
"`      c #A9C9E8",
" .     c #97B7DB",
"..     c #A4B7D2",
"+.     c #CEE8FF",
"@.     c #CEE4FF",
"#.     c #C9E4FF",
"$.     c #C4E0FB",
"%.     c #AECEED",
"&.     c #A4C4E4",
"*.     c #B7C4D6",
"=.     c #D6EDFF",
"-.     c #D2EDFF",
";.     c #D2E8FF",
">.     c #B7D6F2",
",.     c #A0BBD6",
"'.     c #C9D2DB",
").     c #D6F2FF",
"!.     c #C0DBFB",
"~.     c #BBD6F6",
"{.     c #A9C4E8",
"].     c #B7C9D6",
"^.     c #D2D6DB",
"/.     c #DBF6FF",
"(.     c #C0E0FB",
"_.     c #AEC4D6",
":.     c #CED2D6",
"<.     c #D2F2FF",
"[.     c #C4E4FF",
"}.     c #B7D6F6",
"|.     c #B2C9DB",
"1.     c #C4CED6",
"2.     c #CEEDFF",
"3.     c #B7C9DB",
"4.     c #C4C9D2",
"5.     c #D2DBE0",
"6.     c #EDEDF2",
"7.     c #C4E0F6",
"8.     c #BBD2E4",
"9.     c #BBC4D2",
"0.     c #C9CED2",
"a.     c #B7C0C9",
"b.     c #B7C0C4",
"c.     c #BBC4C9",
"d.     c #C0C4CE",
"e.     c #CED6DB",
"f.     c #D6DBE0",
". + @ # $ % & * * * * * * ",
"= - ; > , ' # ) * * * * * ",
"! ! ~ { ] ^ / ( _ & * * * ",
": : < [ } | 1 2 3 4 5 * * ",
"6 7 7 7 : 8 9 0 a b c 5 * ",
"d 6 6 e f g h i j k l m n ",
"e e e o p q r s t u k v 5 ",
"w w w x y z A B s C D E & ",
"y F F G G H A I B s J K L ",
"M M y F F G H N I B O J P ",
"Q R x M M F G H N I S i P ",
"T T U Q R M V G H A B W P ",
"X X X Y Z R M V ` N  ...P ",
"+.@.@.#.$.Z R M %.&.I *.P ",
"=.-.;.+.#.$.Z >.V H ,.'.m ",
").).).-.+.#.!.~.V {.].^.L ",
"/.).).-.+.#.(.~.V _.:.P & ",
").).<.-.+.[.!.}.|.1.^.L 5 ",
"-.-.2.+.#.$.M 3.4.5.m 6.n ",
"+.#.7.Q 8.*.9.0.5.P & 5 * ",
"a.a.b.c.d.4.:.^.m & 5 * * ",
"0.0.:.:.e.^.P L 6.5 * * * ",
"f.f.P P m L & 5 * * * * * "};
/* XPM */
static const char * const aqua_btn_dis_left_xpm[] = {
"13 23 14 1",
"       c None",
".      c #E7E7E7",
"+      c #DEDEDE",
"@      c #D6D6D6",
"#      c #B5B5B5",
"$      c #A5A5A5",
"%      c #9C9C9C",
"&      c #EFEFEF",
"*      c #BDBDBD",
"=      c #C6C6C6",
"-      c #FFFFFF",
";      c #F7F7F7",
">      c #ADADAD",
",      c #CECECE",
".....+@#$$%$$",
"&&&&.*$$#**==",
"--;&=>*,+....",
"&..#$>=@@++++",
".+*$>#,@+++++",
".@#>#*@++++++",
";,*=,@.&&&&&&",
"+##*==,@@++++",
",#**=,,@@@@@@",
"@#*=,,@@@@+++",
"+=@++....&&&&",
",*=,@@@++++++",
"=*=,@@+++....",
",=,,@++.....&",
".@@..&;;-----",
"@==@@+..&&&&&",
"@=#*@+.&&&&&&",
".@=*=+..&&&&&",
";&.@,,.;-----",
"&.+,==*=,+.&&",
".++@,*###>>>>",
"&&..+@,==***=",
"----;;&..++++"};
/* XPM */
static const char * const aqua_btn_dis_mid_xpm[] = {
"7 23 9 1",
"       c None",
".      c #ADADAD",
"+      c #CECECE",
"@      c #E7E7E7",
"#      c #DEDEDE",
"$      c #EFEFEF",
"%      c #D6D6D6",
"&      c #FFFFFF",
"*      c #C6C6C6",
".......",
"+++++++",
"@@@@@@@",
"#######",
"#######",
"#######",
"$$$$$$$",
"#######",
"%%%%%%%",
"#######",
"$$$$$$$",
"@@@@@@@",
"@@@@@@@",
"$$$$$$$",
"&&&&&&&",
"$$$$$$$",
"$$$$$$$",
"$$$$$$$",
"&&&&&&&",
"$$$$$$$",
".......",
"*******",
"#######"};
/* XPM */
static const char * const aqua_btn_dis_right_xpm[] = {
"13 23 13 1",
"       c None",
".      c #ADADAD",
"+      c #A5A5A5",
"@      c #B5B5B5",
"#      c #D6D6D6",
"$      c #DEDEDE",
"%      c #E7E7E7",
"&      c #C6C6C6",
"*      c #BDBDBD",
"=      c #EFEFEF",
"-      c #CECECE",
";      c #F7F7F7",
">      c #FFFFFF",
"..+++@#$%%%%%",
"&&&*.++*%====",
"%%%%$-@.&=;>>",
"$$$$##*.+*%%=",
"$$$$$$-@.+*$%",
"$$$$$$#*@.@#%",
"======%#-&*#;",
"$$$$##--&*@*$",
"######---&**-",
"$$$#####--&@#",
"=====%%%%$#-$",
"$$$$$$$##--&-",
"%%%%$$$$##-*&",
"=%%%%%$$##-&-",
">>>>;;;==%$#%",
"=====%%$$#&&#",
"======%$$&@&#",
"=====%%$&*&#%",
">>>>>;%#-#%=;",
"==%$-&*&&-$%=",
"....@@@*-#$$%",
"&***&&-#$%%==",
"$$$$%%=;;>>>>"};
/* XPM */
static const char * const aqua_btn_nrm_left_xpm[] = {
"13 23 22 1",
"       c None",
".      c #FFFFFF",
"+      c #D6D6D6",
"@      c #9C9C9C",
"#      c #737373",
"$      c #636363",
"%      c #5A5A5A",
"&      c #6B6B6B",
"*      c #DEDEDE",
"=      c #949494",
"-      c #848484",
";      c #A5A5A5",
">      c #ADADAD",
",      c #7B7B7B",
"'      c #BDBDBD",
")      c #C6C6C6",
"!      c #CECECE",
"~      c #E7E7E7",
"{      c #8C8C8C",
"]      c #B5B5B5",
"^      c #EFEFEF",
"/      c #F7F7F7",
"......+@#$%&#",
"....*=$&-=;>>",
"...+-%,;')!!!",
"..~{$#;)++***",
".*@$#{'+****~",
"*'#,-@)***~~~",
"*@,{=>)+**~~~",
"+={=;]'!+++**",
")-=;>'))!!+++",
"'-@>'))!+++++",
"'{;]))!+++***",
")@>')!++**~~~",
"'@])!+**~~^^^",
"]@])!+*~^^///",
");;)+*~^/....",
"!]>)+~^/.....",
"!'@;+~^/.....",
"+';=;*^/.....",
"~+)>=;)^/....",
".~+)];@;)+^//",
"..*!)>@={-,,,",
"...+!'];@=={=",
".....*!))'']]"};
/* XPM */
static const char * const aqua_btn_nrm_mid_xpm[] = {
"7 23 11 1",
"       c None",
".      c #737373",
"+      c #7B7B7B",
"@      c #B5B5B5",
"#      c #D6D6D6",
"$      c #DEDEDE",
"%      c #E7E7E7",
"&      c #EFEFEF",
"*      c #F7F7F7",
"=      c #FFFFFF",
"-      c #949494",
".++++++",
"@@@@@@@",
"#######",
"$$$$$$$",
"%%%%%%%",
"%%%%%%%",
"%%%%%%%",
"$$$$$$$",
"#######",
"#$$$$$$",
"$$$$$$$",
"%%%%%%%",
"&&&&&&&",
"*******",
"=======",
"=======",
"=======",
"=======",
"=======",
"=======",
"+++++++",
"-------",
"@@@@@@@"};
/* XPM */
static const char * const aqua_btn_nrm_right_xpm[] = {
"13 23 22 1",
"       c None",
".      c #737373",
"+      c #636363",
"@      c #6B6B6B",
"#      c #9C9C9C",
"$      c #D6D6D6",
"%      c #FFFFFF",
"&      c #B5B5B5",
"*      c #ADADAD",
"=      c #848484",
"-      c #949494",
";      c #DEDEDE",
">      c #CECECE",
",      c #BDBDBD",
"'      c #5A5A5A",
")      c #C6C6C6",
"!      c #E7E7E7",
"~      c #8C8C8C",
"{      c #7B7B7B",
"]      c #A5A5A5",
"^      c #EFEFEF",
"/      c #F7F7F7",
"..+@.#$%%%%%%",
"&**#=++-;%%%%",
"$>>>,#.'=$%%%",
";;;;$)#.+-!%%",
"!!;;;$,~{@];%",
"!!!!;;)]~={,;",
"!!!;;$)*#-=];",
";;;$$>)&*#-#$",
"$$$$>>),&*#~)",
"$$$$$>>),&*-,",
";;;;$$$>),&#,",
"!!!!;;$$>)&])",
"^^^!!!;$>),#,",
"////^!!;$>,]&",
"%%%%/^!;$>]])",
"%%%%%/^!$>*&>",
"%%%%%/^!;]#,>",
"%%%%%/^!*-],$",
"%%%%/^)]-*)$!",
"%/^$)]#]&)$!%",
"{{{=~-#*)>;%%",
"-~--#]&,>$%%%",
"&&,,))>;%%%%%"};
/* XPM */
static const char * const aqua_bvlbtn_small_dis_xpm[] = {
"24 24 12 1",
"       c None",
".      c #EFEFEF",
"+      c #DEDEDE",
"@      c #C6C6C6",
"#      c #CECECE",
"$      c #BDBDBD",
"%      c #D6D6D6",
"&      c #E7E7E7",
"*      c #F7F7F7",
"=      c #FFFFFF",
"-      c #B5B5B5",
";      c #ADADAD",
".+@@@##############@@@+.",
"+$%&&&&&&&&&&&&&&&&&&%$+",
"+@&&................&&@+",
"&&*==================*&&",
"#%&&&.............&&&&%#",
"@%+&&&&&&&&&&&&&&&&&&+%@",
"#+++&&&&&&&&&&&&&&&&+++#",
"+..******************..+",
"#++&&&&&&&&&&&&&&&&&&++#",
"@++&&&&&&&&&&&&&&&&&&++@",
"#++&&&&&&&&&&&&&&&&&&++#",
"+..******************..+",
"#++&&&&&&&&&&&&&&&&&&++#",
"@++&&&&&&&&&&&&&&&&&&++@",
"#++&&&&&&&&&&&&&&&&&&++#",
"+..******************..+",
"#++&&&&&&&&&&&&&&&&&&++#",
"@+&&&&&&&&&&&&&&&&&&&&+@",
"#+&&&...............&&+#",
"+.*==================*.+",
"#%&..................&%#",
"#$&..................&$#",
"+$$%................%$$+",
"*&#-;;;;;;;;;;;;;;;;-#&*"};
/* XPM */
static const char * const aqua_bvlbtn_small_off_xpm[] = {
"24 24 20 1",
"       c None",
".      c #E7E7E7",
"+      c #C6C6C6",
"@      c #ADADAD",
"#      c #9C9C9C",
"$      c #B5B5B5",
"%      c #BDBDBD",
"&      c #CECECE",
"*      c #D6D6D6",
"=      c #EFEFEF",
"-      c #F7F7F7",
";      c #A5A5A5",
">      c #FFFFFF",
",      c #DEDEDE",
"'      c #949494",
")      c #848484",
"!      c #8C8C8C",
"~      c #6B6B6B",
"{      c #5A5A5A",
"]      c #525252",
".+@#$$%%%%%%%%%%%%$$#@+.",
"&#*=----------------=*#&",
"+;=------------------=;+",
"%+------>->->>-->-----+%",
";&--------------------&;",
"#*..================..*#",
"#*,.===============...*#",
"@,..===============...,@",
"#,..================..,#",
"',..================..,'",
"#,..================..,#",
"@,..================..,@",
"#,..================..,#",
"',..================..,'",
"#,..================..,#",
"@,..================..,@",
"#,.==================.,#",
"',.===---============.,'",
"#,==---->>>===========.#",
"$.=->>>>>>>>>->>->>>-=.$",
";%->>>>>>>>>>>>>>>>>>-%;",
";#->>>>>>>>>>>>>>>>>>-#;",
"+)!*>>>>>>>>>>>>>>>>*!)+",
".%!~{]]]]]]]]]]]]]]{~!%."};
/* XPM */
static const char * const aqua_bvlbtn_small_on_xpm[] = {
"24 24 18 1",
"       c None",
".      c #E7E7E7",
"+      c #BDBDBD",
"@      c #848484",
"#      c #5A5A5A",
"$      c #636363",
"%      c #6B6B6B",
"&      c #737373",
"*      c #CECECE",
"=      c #8C8C8C",
"-      c #ADADAD",
";      c #B5B5B5",
">      c #C6C6C6",
",      c #A5A5A5",
"'      c #9C9C9C",
")      c #949494",
"!      c #7B7B7B",
"~      c #525252",
".+@#$%&&&&&&&&&&&&%$#@+.",
"*&=-;;+++++++++++++;-=&*",
">$-++++++++++++++++++-$>",
"+@++++++++++++++++++++@+",
",=;;++++++++++++++++;;=,",
"'),-------;;;;;;;----,)'",
"'),,----------------,,)'",
"-',,----------------,,'-",
"'',,----------------,,''",
")',,-----------------,')",
"'',,-----------------,''",
"-',,-----------------,'-",
"'',,-----------------,''",
")',,-----------------,')",
"'',,-----------------,''",
"-',,-------------;---,'-",
"'',,-;------------;;-,''",
")'-;;;;+-+--+--+------')",
"',-;++++++>++++++++-;-,'",
";,;+>*****>+*+*+*++>+;,;",
",=+*****************>;@,",
",!+******************+!,",
">!&,****************,%!>",
".+=%#~~~~~~~~~~~~~~#%=+."};
/* XPM */
static const char * const aqua_chk_act_f_xpm[] = {
"15 18 24 1",
"       c None",
".      c #E7E7E7",
"+      c #EFEFEF",
"@      c #D6D6D6",
"#      c #BDBDBD",
"$      c #CECECE",
"%      c #4A4A4A",
"&      c #525252",
"*      c #6B6B6B",
"=      c #424242",
"-      c #FFFFFF",
";      c #8C8C8C",
">      c #ADADAD",
",      c #737373",
"'      c #7B7B7B",
")      c #848484",
"!      c #9C9C9C",
"~      c #DEDEDE",
"{      c #A5A5A5",
"]      c #B5B5B5",
"^      c #C6C6C6",
"/      c #F7F7F7",
"(      c #949494",
"_      c #5A5A5A",
"...............",
"+@##########$++",
"@%&********&=#-",
";*>$$$$$$$$>,'+",
")!$~@~@@~~@${,.",
"){]#^^^#^^#]>'.",
";{]########]>'/",
")>#^^$^^$^^^]'.",
"'#^@@@@@@@@$#,~",
";^@~~~~~~~~@^'.",
"($~........~$)/",
";@.////////.@'.",
")@+--------/~'~",
")$----------~)~",
"{)>#]]]]]]#];!+",
"],_&%%%%%%&_,>~",
"${(;;;;;;;;({^~",
"+.~@@@@@@@@~~.+"};
/* XPM */
static const char * const aqua_chk_act_t_xpm[] = {
"15 18 142 2",
"       c None",
".      c #E7E7E7",
"+      c #B5B5B5",
"@      c #000000",
"#      c #424242",
"$      c #EFEFEF",
"%      c #D6D6D6",
"&      c #BDBDBD",
"*      c #C6C6C6",
"=      c #5A5A5A",
"-      c #A5A5A5",
";      c #212152",
">      c #18185A",
",      c #31397B",
"'      c #39427B",
")      c #39397B",
"!      c #18214A",
"~      c #FFFFFF",
"{      c #8C8C94",
"]      c #213184",
"^      c #848CC6",
"/      c #B5BDE7",
"(      c #BDBDE7",
"_      c #BDC6EF",
":      c #ADB5DE",
"<      c #181829",
"[      c #000008",
"}      c #73737B",
"|      c #7B7B8C",
"1      c #396BB5",
"2      c #94B5E7",
"3      c #ADC6EF",
"4      c #A5C6F7",
"5      c #A5CEF7",
"6      c #ADCEF7",
"7      c #ADD6FF",
"8      c #6B7B94",
"9      c #08214A",
"0      c #5A637B",
"a      c #737B84",
"b      c #296BBD",
"c      c #4A8CD6",
"d      c #5294E7",
"e      c #529CE7",
"f      c #5294DE",
"g      c #528CCE",
"h      c #000810",
"i      c #296BB5",
"j      c #5A6B84",
"k      c #7B848C",
"l      c #397BC6",
"m      c #296394",
"n      c #001018",
"o      c #183152",
"p      c #428CDE",
"q      c #3984D6",
"r      c #3984CE",
"s      c #4294E7",
"t      c #18395A",
"u      c #214A73",
"v      c #637384",
"w      c #F7F7F7",
"x      c #5284C6",
"y      c #5A9CDE",
"z      c #102129",
"A      c #5284BD",
"B      c #639CE7",
"C      c #63A5EF",
"D      c #4A7BB5",
"E      c #000808",
"F      c #101829",
"G      c #5A94D6",
"H      c #5294CE",
"I      c #63737B",
"J      c #6B9CCE",
"K      c #7BADE7",
"L      c #5A84AD",
"M      c #293952",
"N      c #84BDF7",
"O      c #7BB5EF",
"P      c #5A8CB5",
"Q      c #7BADEF",
"R      c #73A5DE",
"S      c #636B7B",
"T      c #DEDEDE",
"U      c #7BADDE",
"V      c #8CBDEF",
"W      c #94C6FF",
"X      c #212931",
"Y      c #7BADD6",
"Z      c #638CAD",
"`      c #31425A",
" .     c #84B5DE",
"..     c #6B7B84",
"+.     c #8CB5DE",
"@.     c #9CC6F7",
"#.     c #A5D6FF",
"$.     c #7B9CBD",
"%.     c #31424A",
"&.     c #8CB5D6",
"*.     c #94BDE7",
"=.     c #84848C",
"-.     c #9CC6E7",
";.     c #B5E7FF",
">.     c #B5DEFF",
",.     c #293942",
"'.     c #5A7384",
").     c #BDE7FF",
"!.     c #7B7B84",
"~.     c #A5C6EF",
"{.     c #B5E7F7",
"].     c #BDEFFF",
"^.     c #C6F7FF",
"/.     c #8CB5BD",
"(.     c #212929",
"_.     c #ADCEDE",
":.     c #BDEFF7",
"<.     c #B5DEF7",
"[.     c #A5CEFF",
"}.     c #6B737B",
"|.     c #7B8484",
"1.     c #A5C6E7",
"2.     c #D6FFFF",
"3.     c #DEFFFF",
"4.     c #B5E7E7",
"5.     c #CEF7F7",
"6.     c #CEFFFF",
"7.     c #ADD6F7",
"8.     c #738494",
"9.     c #9CBDBD",
"0.     c #A5C6C6",
"a.     c #9CBDC6",
"b.     c #A5C6CE",
"c.     c #7B8C94",
"d.     c #9C9C9C",
"e.     c #737373",
"f.     c #525252",
"g.     c #4A4A4A",
"h.     c #ADADAD",
"i.     c #CECECE",
"j.     c #949494",
"k.     c #8C8C8C",
". . . . . . . . . . . . + @ # ",
"$ % & & & & & & & & & * = @ - ",
"% ; > , ' ' ) ) ' ' ) ! @ = ~ ",
"{ ] ^ / / / / / ( _ : < [ } $ ",
"| 1 2 3 4 5 4 4 6 7 8 @ 9 0 . ",
"a b c d e f f f f g h h i j . ",
"k l m n o p q r s t @ u r v w ",
"k x y z @ A B C D E F G H I . ",
"a J K L @ M N O M @ P Q R S T ",
"k U V W X E Y Z @ ` W V  .... ",
"{ +.@.#.$.@ %.%.@ &.7 @.*.k w ",
"=.-.7 ;.>.,.@ @ '.).>.7 @.a . ",
"!.~.{.].^./.@ (._.^.:.<.[.}.T ",
"|.1.2.3.3.2.4.5.3.3.3.6.7.k T ",
"- 8.9.0.a.a.0.a.a.a.b.a.c.d.$ ",
"+ e.= f.g.g.g.g.g.g.f.= e.h.T ",
"i.- j.k.k.k.k.k.k.k.k.j.- * T ",
"$ . T % % % % % % % % T T . $ "};
/* XPM */
static const char * const aqua_chk_dis_t_xpm[] = {
"15 18 32 1",
"       c None",
".      c #E7E7E7",
"+      c #B5B5B5",
"@      c #000000",
"#      c #424242",
"$      c #EFEFEF",
"%      c #D6D6D6",
"&      c #BDBDBD",
"*      c #C6C6C6",
"=      c #5A5A5A",
"-      c #A5A5A5",
";      c #4A4A4A",
">      c #525252",
",      c #6B6B6B",
"'      c #FFFFFF",
")      c #8C8C8C",
"!      c #ADADAD",
"~      c #CECECE",
"{      c #212121",
"]      c #080808",
"^      c #7B7B7B",
"/      c #848484",
"(      c #9C9C9C",
"_      c #DEDEDE",
":      c #101010",
"<      c #181818",
"[      c #636363",
"}      c #F7F7F7",
"|      c #292929",
"1      c #949494",
"2      c #737373",
"3      c #313131",
"............+@#",
"$%&&&&&&&&&*=@-",
"%;>,,,,,,,,#@='",
"),!~~~~~~~*{]^$",
"/(~%%_%%_./@#^.",
"/-&****&*+]:(^.",
")!/<;*&+*;@[+^}",
"/!*|@-*~-]{&+^.",
"^&~1@#_%;@(~&2_",
")*~.3]&(@>.%*^.",
"1~_$!@##@*$_~/}",
")%.}$#@@^'}.%^.",
"/%$''&@3_''}_^_",
"/~''''.}''''_/_",
"-/!&++++++&+)($",
"+2=>;;;;;;>=2!_",
"~-1))))))))1-*_",
"$._%%%%%%%%__.$"};
/* XPM */
static const char * const aqua_cmb_act_left_xpm[] = {
"7 22 18 1",
"       c None",
".      c #F7F7F7",
"+      c #EFEFEF",
"@      c #C6C6C6",
"#      c #A5A5A5",
"$      c #949494",
"%      c #CECECE",
"&      c #B5B5B5",
"*      c #D6D6D6",
"=      c #DEDEDE",
"-      c #9C9C9C",
";      c #ADADAD",
">      c #E7E7E7",
",      c #BDBDBD",
"'      c #FFFFFF",
")      c #7B7B7B",
"!      c #737373",
"~      c #6B6B6B",
".+@#$##",
".%#&%*=",
"=-;%>>>",
"@-,=>++",
"@&@>>++",
"@,*=>>>",
",@*>>>>",
"&%=>>++",
",*>++++",
"@*>++..",
",*>....",
"&*>....",
",*+....",
"@=+..''",
",=+.'''",
"&=.''''",
",@.''''",
"%&.''''",
"%-@''''",
"*;-&=''",
"+*&$)!~",
"'.>*%@@"};
/* XPM */
static const char * const aqua_cmb_act_mid_xpm[] = {
"20 22 9 1",
"       c None",
".      c #A5A5A5",
"+      c #DEDEDE",
"@      c #EFEFEF",
"#      c #E7E7E7",
"$      c #F7F7F7",
"%      c #FFFFFF",
"&      c #636363",
"*      c #B5B5B5",
"....................",
"++++++++++++++++++++",
"@@@@@@@@@@@@@@@@@@@@",
"@@@@@@@@@@@@@@@@@@@@",
"@@@@@@@@@@@@@@@@@@@@",
"####################",
"####################",
"@@@@@@@@@@@@@@@@@@@@",
"@@@@@@@@@@@@@@@@@@@@",
"$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$",
"%%%%%%%%%%%%%%%%%%%%",
"%%%%%%%%%%%%%%%%%%%%",
"%%%%%%%%%%%%%%%%%%%%",
"%%%%%%%%%%%%%%%%%%%%",
"%%%%%%%%%%%%%%%%%%%%",
"%%%%%%%%%%%%%%%%%%%%",
"%%%%%%%%%%%%%%%%%%%%",
"&&&&&&&&&&&&&&&&&&&&",
"********************"};
/* XPM */
static const char * const aqua_cmb_act_right_xpm[] = {
"22 22 139 2",
"       c None",
".      c #1052AD",
"+      c #1052A5",
"@      c #0852A5",
"#      c #084AA5",
"$      c #08429C",
"%      c #103994",
"&      c #4A5A94",
"*      c #9494AD",
"=      c #DEDEDE",
"-      c #E7E7E7",
";      c #8CADD6",
">      c #9CC6E7",
",      c #9CBDE7",
"'      c #94BDDE",
")      c #84ADD6",
"!      c #528CC6",
"~      c #215AAD",
"{      c #21398C",
"]      c #9C9CB5",
"^      c #A5C6DE",
"/      c #BDD6EF",
"(      c #BDCEEF",
"_      c #B5CEE7",
":      c #ADCEE7",
"<      c #5A8CC6",
"[      c #00429C",
"}      c #4A5294",
"|      c #9CBDDE",
"1      c #B5CEEF",
"2      c #A5C6E7",
"3      c #7BA5DE",
"4      c #0852AD",
"5      c #082984",
"6      c #C6C6C6",
"7      c #73A5DE",
"8      c #2163BD",
"9      c #00399C",
"0      c #B5B5B5",
"a      c #5A94C6",
"b      c #84ADE7",
"c      c #84ADDE",
"d      c #84B5E7",
"e      c #527394",
"f      c #7BADDE",
"g      c #6BA5DE",
"h      c #5A94D6",
"i      c #216BBD",
"j      c #0042A5",
"k      c #4A84C6",
"l      c #73ADE7",
"m      c #5A8CBD",
"n      c #182131",
"o      c #5A9CDE",
"p      c #4A8CD6",
"q      c #3173C6",
"r      c #7BB5EF",
"s      c #7BADE7",
"t      c #182939",
"u      c #000000",
"v      c #7BADEF",
"w      c #73ADEF",
"x      c #6BA5E7",
"y      c #639CDE",
"z      c #5294DE",
"A      c #397BCE",
"B      c #105AB5",
"C      c #5A94CE",
"D      c #84BDF7",
"E      c #527BA5",
"F      c #182931",
"G      c #639CE7",
"H      c #428CD6",
"I      c #ADADAD",
"J      c #639CD6",
"K      c #8CC6FF",
"L      c #101821",
"M      c #8CBDF7",
"N      c #6BA5EF",
"O      c #296BC6",
"P      c #94CEFF",
"Q      c #94C6FF",
"R      c #84BDFF",
"S      c #73B5F7",
"T      c #5A9CE7",
"U      c #317BCE",
"V      c #73ADDE",
"W      c #9CD6FF",
"X      c #9CCEFF",
"Y      c #7BBDFF",
"Z      c #63A5EF",
"`      c #7BB5DE",
" .     c #A5D6FF",
"..     c #9CCEF7",
"+.     c #84C6FF",
"@.     c #6BADF7",
"#.     c #3984D6",
"$.     c #A5DEFF",
"%.     c #6B94AD",
"&.     c #213139",
"*.     c #84BDDE",
"=.     c #ADE7FF",
"-.     c #ADDEFF",
";.     c #293942",
">.     c #6BADFF",
",.     c #428CDE",
"'.     c #8CC6DE",
").     c #B5E7FF",
"!.     c #8CBDD6",
"~.     c #A5E7FF",
"{.     c #9CDEFF",
"].     c #8CCEFF",
"^.     c #73BDFF",
"/.     c #94CEDE",
"(.     c #B5EFFF",
"_.     c #739CAD",
":.     c #94D6FF",
"<.     c #4284CE",
"[.     c #BDF7FF",
"}.     c #BDEFFF",
"|.     c #ADEFFF",
"1.     c #527BAD",
"2.     c #BDBDBD",
"3.     c #9CDEDE",
"4.     c #BDFFFF",
"5.     c #B5F7FF",
"6.     c #94DEFF",
"7.     c #8494A5",
"8.     c #D6D6D6",
"9.     c #94D6DE",
"0.     c #94C6DE",
"a.     c #739CB5",
"b.     c #7B8C9C",
"c.     c #636363",
"d.     c #6B6B6B",
"e.     c #737373",
"f.     c #8C8C8C",
"g.     c #A5A5A5",
"h.     c #EFEFEF",
". . . . . . . . + . . . . + @ # $ % & * = - ",
"; > > > , > > > > > , , , , , ' ) ! ~ { ] - ",
"^ / / / / / / / / / ( / / / ( _ : , < [ } - ",
"| 1 1 1 1 1 1 1 1 1 1 1 1 1 _ : : 2 3 4 5 6 ",
"; 2 2 2 2 2 2 2 2 2 2 2 2 2 > > , ' 7 8 9 0 ",
"a b c d c c c c c e c c c c f f 7 g h i j 0 ",
"k l l l l l l l m n m l l l l g g o p q 4 6 ",
"! r s r r r r r t u t r r v w l x y z A B 0 ",
"C D D D D D D E F u F E D D D r r w G H i I ",
"J K K K K K D L u u u L D K M M D r N z O 0 ",
"g P P P P P P P P P P P P P Q K K R S T U 6 ",
"V W W W W W W W W W W W W W W X P K Y Z U 0 ",
"`  . . . . ...L u u u L .. . . .W P +.@.#.I ",
"` $.$.$.$.$.$.%.&.u &.%.$.$.$. .W P +.@.#.0 ",
"*.=.-.-.=.-.=.-.;.u ;.=.-.-.-.$.W W K >.,.6 ",
"'.).).).=.).=.).!.&.!.=.).=.=.=.~.{.].^.,.0 ",
"/.(.(.(.(.(.(.(.(._.(.(.(.(.(.(.=.~.:.^.<.I ",
"/.[.[.[.[.}.[.[.[.[.}.(.(.}.}.(.|.=.:.Y 1.2.",
"3.[.[.[.4.[.[.[.[.[.[.[.[.4.4.[.5.=.6.g 7.8.",
"9.[.[.[.[.[.[.[.[.[.[.[.[.[.[.[.|.0.a.b.I 8.",
"c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.c.d.e.f.g.6 = ",
"0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2.6 8.- h."};
/* XPM */
static const char * const aqua_cmb_dis_right_xpm[] = {
"22 22 29 1",
"       c None",
".      c #737373",
"+      c #6B6B6B",
"@      c #636363",
"#      c #5A5A5A",
"$      c #9C9C9C",
"%      c #DEDEDE",
"&      c #E7E7E7",
"*      c #BDBDBD",
"=      c #CECECE",
"-      c #C6C6C6",
";      c #525252",
">      c #A5A5A5",
",      c #D6D6D6",
"'      c #4A4A4A",
")      c #848484",
"!      c #B5B5B5",
"~      c #ADADAD",
"{      c #8C8C8C",
"]      c #292929",
"^      c #949494",
"/      c #313131",
"(      c #000000",
"_      c #7B7B7B",
":      c #181818",
"<      c #EFEFEF",
"[      c #F7F7F7",
"}      c #393939",
"|      c #FFFFFF",
"..............++@#+$%&",
"*==============-*$.;>&",
"=%%%%%%%%%%%%%%%,=>@+&",
"-%%%%%%%%%%%%%%,,=*.'-",
"*=,===,,,====,===-*)#!",
">--------)----***!~{+!",
"$-******$]$**-**!!>^.-",
">-------/(/-----*!~$_!",
"~======)](])====--*>)~",
"!,%,,%=:(((:=,,,=--~{!",
"*&%%%%%%%%%&&%%,%,=*^-",
"-&&&&&&&&&&&&&&&&,,-$!",
"-&<<<<&:(((:&<<&&,,-$~",
"=[[[[[[>/(/>[[<<&%%->!",
"=[[[[[[[}(}[[[[[<<%=~-",
"%|||||||,},||||||<&,~!",
"%||||||||~|||||||[<,$~",
"%|||||||||||||||||[%{*",
"%|||||||||||||||||[!$,",
"%||||||||||||||||%>{~,",
"@@@@@@@@@@@@@@@@+.{>-%",
"!!!!!!!!!!!!!!!!!*-,&<"};
/* XPM */
static const char * const aqua_gen_back_xpm[] = {
"90 72 4 1",
"       c None",
".      c #FFFFFF",
"+      c #F7F7F7",
"@      c #EFEFEF",
"..........................................................................................",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"..........................................................................................",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"..........................................................................................",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"..........................................................................................",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"..........................................................................................",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"..........................................................................................",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"..........................................................................................",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"..........................................................................................",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"..........................................................................................",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"..........................................................................................",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"..........................................................................................",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"..........................................................................................",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"..........................................................................................",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"..........................................................................................",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"..........................................................................................",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"..........................................................................................",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"..........................................................................................",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"..........................................................................................",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@",
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"};
/* XPM */
static const char * const aqua_hsbr_arw_left_xpm[] = {
"26 14 22 1",
"       c None",
".      c #B5B5B5",
"+      c #BDBDBD",
"@      c #CECECE",
"#      c #D6D6D6",
"$      c #A5A5A5",
"%      c #ADADAD",
"&      c #EFEFEF",
"*      c #F7F7F7",
"=      c #C6C6C6",
"-      c #FFFFFF",
";      c #9C9C9C",
">      c #DEDEDE",
",      c #949494",
"'      c #5A5A5A",
")      c #4A4A4A",
"!      c #737373",
"~      c #E7E7E7",
"{      c #424242",
"]      c #8C8C8C",
"^      c #636363",
"/      c #525252",
"...++++++++++++++++++...++",
"@#################+$%..+++",
"&***************&=$%.+====",
"***-*--------***#;$.+==@@@",
"&-------&$---*>>%,%==@@###",
">*-----.')---&@#;$+=@##>>>",
"#&*--#!)))**&>#>,%+@##>>~~",
"#>>>%{{{{{~~~~>~]%=@#>~~&&",
">~~~~=^{{{&~&&>~,%=#>>~&&*",
"~~&&&&&%/{&&&&~&$$+#>~&&**",
"&&******&,****&*=,.@>~&***",
"**--------------*%%@>~&***",
"**---------------~.+#>~&**",
"***---------------~==>~&&*"};
/* XPM */
static const char * const aqua_hsbr_arw_right_xpm[] = {
"26 14 23 1",
"       c None",
".      c #BDBDBD",
"+      c #B5B5B5",
"@      c #ADADAD",
"#      c #A5A5A5",
"$      c #D6D6D6",
"%      c #CECECE",
"&      c #ADADBD",
"*      c #C6C6C6",
"=      c #EFEFEF",
"-      c #F7F7F7",
";      c #9C9C9C",
">      c #FFFFFF",
",      c #949494",
"'      c #DEDEDE",
")      c #4A4A4A",
"!      c #5A5A5A",
"~      c #E7E7E7",
"{      c #737373",
"]      c #8C8C8C",
"^      c #424242",
"/      c #636363",
"(      c #525252",
"..+++.................+++@",
"...++@#.$$$$$$$$$$$$$$$$%&",
"****.+@#*=--------------=.",
"%%%**.+#;$-->>>>>>>>->---+",
"$$$%%**@,@'->>>#=>>>>>>>=@",
"'''$$%*.#;$=>>>)!+>>>>>-'@",
"~~''$$%.@,''=--))){$>>-=$@",
"==~~'$%*@]~~~~~^^^^^@'''$.",
"-==~''$*@,~==~=^^^/*~~~~'$",
"--==~'$.##=====^(@=====~~.",
"---=~'%+,*-----,=------==$",
"---=~'%@@->>>>>>>>>>>>>--$",
"--=~'$.+~>>>>>>>>>>>>>>--$",
"-==~'**~>>>>>>>>>>>>>>---$"};
/* XPM */
static const char * const aqua_hsbr_back_fill_xpm[] = {
"32 14 10 1",
"       c None",
".      c #BDBDBD",
"+      c #C6C6C6",
"@      c #CECECE",
"#      c #D6D6D6",
"$      c #DEDEDE",
"%      c #E7E7E7",
"&      c #EFEFEF",
"*      c #F7F7F7",
"=      c #FFFFFF",
"................................",
"++++++++++++++++++++++++++++++++",
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@",
"################################",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%",
"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&",
"********************************",
"********************************",
"================================",
"================================",
"================================",
"================================",
"********************************"};
/* XPM */
static const char * const aqua_hsbr_fill_xpm[] = {
"16 14 56 1",
"       c None",
".      c #ADADAD",
"+      c #0039B5",
"@      c #739CDE",
"#      c #73A5DE",
"$      c #73A5E7",
"%      c #7BADE7",
"&      c #7BA5DE",
"*      c #7B9CDE",
"=      c #8CBDE7",
"-      c #94BDE7",
";      c #94BDEF",
">      c #9CC6EF",
",      c #8CB5E7",
"'      c #84B5E7",
")      c #7BB5E7",
"!      c #84B5EF",
"~      c #8CB5EF",
"{      c #8CBDEF",
"]      c #428CE7",
"^      c #428CDE",
"/      c #4294E7",
"(      c #4A94E7",
"_      c #5294E7",
":      c #529CE7",
"<      c #398CE7",
"[      c #3984E7",
"}      c #529CEF",
"|      c #5A9CEF",
"1      c #5AA5EF",
"2      c #63A5F7",
"3      c #63A5EF",
"4      c #63ADF7",
"5      c #6BADF7",
"6      c #4A94EF",
"7      c #63ADFF",
"8      c #6BADFF",
"9      c #6BB5FF",
"0      c #73B5FF",
"a      c #7BBDFF",
"b      c #73BDFF",
"c      c #7BC6FF",
"d      c #84C6FF",
"e      c #8CCEFF",
"f      c #84CEFF",
"g      c #94D6FF",
"h      c #94DEFF",
"i      c #9CDEFF",
"j      c #8CD6FF",
"k      c #7BCEFF",
"l      c #9CE7FF",
"m      c #A5E7FF",
"n      c #A5EFFF",
"o      c #8CDEFF",
"p      c #84D6FF",
"q      c #5A5A5A",
"................",
"++++++++++++++++",
"@@####$$%&###@*@",
"=----;>;;-;---,,",
",,==;;;;;-;==,,'",
")''!~{{{{{{,!'%%",
"]^/((__:__((]^<[",
"}|11234552311}}6",
"789900aaa0009877",
"bacddeeeedddcabb",
"feegghiiiggjjffk",
"hhhllmmnmmlihhoj",
"pjgghiiliigggeff",
"qqqqqqqqqqqqqqqq"};
/* XPM */
static const char * const aqua_hsbr_tip_left_xpm[] = {
"10 14 93 2",
"       c None",
".      c #BDBDBD",
"+      c #B5B5B5",
"@      c #ADADAD",
"#      c #A5A5A5",
"$      c #D6D6D6",
"%      c #848494",
"&      c #525A94",
"*      c #213194",
"=      c #0821A5",
"-      c #0029AD",
";      c #0031B5",
">      c #EFEFEF",
",      c #6B6B8C",
"'      c #0021AD",
")      c #3973CE",
"!      c #6B94DE",
"~      c #73A5DE",
"{      c #CECECE",
"]      c #737384",
"^      c #18298C",
"/      c #0039B5",
"(      c #427BD6",
"_      c #84ADE7",
":      c #8CBDE7",
"<      c #94BDE7",
"[      c #94BDEF",
"}      c #9C9C9C",
"|      c #394A84",
"1      c #0039BD",
"2      c #004ABD",
"3      c #4A8CD6",
"4      c #8CB5E7",
"5      c #848484",
"6      c #18399C",
"7      c #004AC6",
"8      c #085AC6",
"9      c #317BD6",
"0      c #6BA5E7",
"a      c #7BADE7",
"b      c #84B5EF",
"c      c #8CBDEF",
"d      c #737373",
"e      c #0039AD",
"f      c #186BCE",
"g      c #2973D6",
"h      c #3184DE",
"i      c #428CE7",
"j      c #4A8CE7",
"k      c #4A94E7",
"l      c #6B6B6B",
"m      c #0042AD",
"n      c #297BD6",
"o      c #398CE7",
"p      c #529CEF",
"q      c #5A9CEF",
"r      c #63A5F7",
"s      c #104A9C",
"t      c #1063C6",
"u      c #3984DE",
"v      c #4A94EF",
"w      c #5AA5F7",
"x      c #63ADFF",
"y      c #6BB5FF",
"z      c #73B5FF",
"A      c #29527B",
"B      c #4294E7",
"C      c #73BDFF",
"D      c #7BC6FF",
"E      c #84C6FF",
"F      c #4A525A",
"G      c #295A9C",
"H      c #3984D6",
"I      c #84CEFF",
"J      c #8CD6FF",
"K      c #94D6FF",
"L      c #E7E7E7",
"M      c #7B7B7B",
"N      c #4A5A6B",
"O      c #396BA5",
"P      c #5294E7",
"Q      c #8CCEFF",
"R      c #94DEFF",
"S      c #9CE7FF",
"T      c #F7F7F7",
"U      c #C6C6C6",
"V      c #525A63",
"W      c #4A6384",
"X      c #4A7BB5",
"Y      c #63A5EF",
"Z      c #FFFFFF",
"`      c #8C8C8C",
" .     c #5A5A5A",
". . . . + @ @ # @ @ ",
"$ $ + % & * = - - ; ",
"> . , * ' ; ) ! ~ ~ ",
"{ ] ^ - / ( _ : < [ ",
"} | - 1 2 3 _ 4 : [ ",
"5 6 / 7 8 9 0 a b c ",
"d e 2 8 f g h i j k ",
"l m 8 f n o k p q r ",
"l s t g u v w x y z ",
"5 A f 9 B w y C D E ",
"# F G H k x C I J K ",
"L M N O P x D Q R S ",
"T U M V W X Y E Q K ",
"Z T U ` d l  . . . ."};
/* XPM */
static const char * const aqua_hsbr_tip_right_xpm[] = {
"10 14 88 1",
"       c None",
".      c #ADADAD",
"+      c #B5B5B5",
"@      c #BDBDBD",
"#      c #0031B5",
"$      c #0831AD",
"%      c #2942A5",
"&      c #5A6BA5",
"*      c #9C9CAD",
"=      c #C6C6C6",
"-      c #73A5DE",
";      c #739CDE",
">      c #6B94DE",
",      c #426BCE",
"'      c #0831B5",
")      c #0018A5",
"!      c #29399C",
"~      c #8484A5",
"{      c #CECECE",
"]      c #94BDE7",
"^      c #8CB5E7",
"/      c #8CADE7",
"(      c #4A7BD6",
"_      c #0029AD",
":      c #29429C",
"<      c #A5A5B5",
"[      c #8CBDE7",
"}      c #84B5E7",
"|      c #7BADE7",
"1      c #4A8CDE",
"2      c #0042BD",
"3      c #0039BD",
"4      c #73A5E7",
"5      c #3984D6",
"6      c #0052C6",
"7      c #004ABD",
"8      c #214AA5",
"9      c #4A8CE7",
"0      c #428CDE",
"a      c #3184DE",
"b      c #297BD6",
"c      c #216BCE",
"d      c #1063C6",
"e      c #085AC6",
"f      c #0052BD",
"g      c #0042AD",
"h      c #5AA5F7",
"i      c #529CEF",
"j      c #4A94E7",
"k      c #317BD6",
"l      c #2173CE",
"m      c #186BCE",
"n      c #105AC6",
"o      c #004AAD",
"p      c #6BB5FF",
"q      c #63ADFF",
"r      c #5294EF",
"s      c #186BC6",
"t      c #215AA5",
"u      c #7BC6FF",
"v      c #73BDFF",
"w      c #4A94EF",
"x      c #3984DE",
"y      c #52739C",
"z      c #8CD6FF",
"A      c #84CEFF",
"B      c #52A5F7",
"C      c #428CE7",
"D      c #316BA5",
"E      c #8C949C",
"F      c #DEDEDE",
"G      c #9CDEFF",
"H      c #4294E7",
"I      c #4273AD",
"J      c #6B7B8C",
"K      c #EFEFEF",
"L      c #94D6FF",
"M      c #8CCEFF",
"N      c #63A5EF",
"O      c #5284BD",
"P      c #4A6B8C",
"Q      c #737B84",
"R      c #F7F7F7",
"S      c #5A5A5A",
"T      c #636363",
"U      c #737373",
"V      c #949494",
"W      c #D6D6D6",
"...+++@@@@",
"###$%&*@==",
"-;>,')!~={",
"]]^/(#_:<{",
"[^}|123#&{",
"}}|456728=",
"90abcdefg@",
"hij0klmno@",
"pqhr0klst@",
"uvphwxkly{",
"zAupBC5DEF",
"GzAvhHIJ@K",
"LMuNOPQ+FR",
"SSSTUV+WKR"};
/* XPM */
static const char * const aqua_radio_f_xpm[] = {
"14 17 26 1",
"       c None",
".      c #FFFFFF",
"+      c #DEDEDE",
"@      c #CECECE",
"#      c #C6C6C6",
"$      c #737373",
"%      c #4A4A4A",
"&      c #393939",
"*      c #313131",
"=      c #525252",
"-      c #6B6B6B",
";      c #B5B5B5",
">      c #ADADAD",
",      c #5A5A5A",
"'      c #848484",
")      c #9C9C9C",
"!      c #636363",
"~      c #7B7B7B",
"{      c #A5A5A5",
"]      c #D6D6D6",
"^      c #8C8C8C",
"/      c #BDBDBD",
"(      c #949494",
"_      c #E7E7E7",
":      c #EFEFEF",
"<      c #F7F7F7",
".....+@@+.....",
"...#$%&*=-;...",
"..>,,'))'!%)..",
".@$~{#]]#;'!;.",
"+^~{/@]]]#{~-]",
">~)>/###//;)$)",
"()>/######/>)^",
");/@]]]]]@@/;^",
"(;@+_____++@/^",
"){+_::<<::_+>^",
">^+:<...<<:+^)",
"]^{:......<>^@",
"]{$):....<{$)@",
"+/(!$(>;($!^/+",
"._#)~-!!-~)#+.",
"..<_@;>>;@+<..",
"...:_++++_:..."};
/* XPM */
static const char * const aqua_radio_t_xpm[] = {
"14 17 135 2",
"       c None",
".      c #FFFFFF",
"+      c #DEDEDE",
"@      c #CECECE",
"#      c #C6C6C6",
"$      c #6B6B7B",
"%      c #4A4A4A",
"&      c #313131",
"*      c #292929",
"=      c #6B6B73",
"-      c #B5B5B5",
";      c #A5A5AD",
">      c #39316B",
",      c #424273",
"'      c #7B7B94",
")      c #9494A5",
"!      c #949CA5",
"~      c #848494",
"{      c #525273",
"]      c #31315A",
"^      c #C6C6CE",
"/      c #314284",
"(      c #314294",
"_      c #7B7BC6",
":      c #B5B5E7",
"<      c #D6CEEF",
"[      c #DED6F7",
"}      c #C6C6E7",
"|      c #949CCE",
"1      c #4A529C",
"2      c #292973",
"3      c #ADADBD",
"4      c #526B94",
"5      c #104A94",
"6      c #3973BD",
"7      c #73A5DE",
"8      c #94BDEF",
"9      c #ADC6FF",
"0      c #ADCEFF",
"a      c #A5C6F7",
"b      c #84ADE7",
"c      c #4273C6",
"d      c #103994",
"e      c #31427B",
"f      c #D6D6D6",
"g      c #ADADAD",
"h      c #21528C",
"i      c #296BB5",
"j      c #4284CE",
"k      c #5A94DE",
"l      c #5A84BD",
"m      c #425A6B",
"n      c #425263",
"o      c #527BAD",
"p      c #5A94D6",
"q      c #4284D6",
"r      c #2163B5",
"s      c #083984",
"t      c #94949C",
"u      c #8C949C",
"v      c #316BAD",
"w      c #4A84CE",
"x      c #5A9CE7",
"y      c #639CDE",
"z      c #182931",
"A      c #000000",
"B      c #101821",
"C      c #528CC6",
"D      c #4284C6",
"E      c #2163AD",
"F      c #737B8C",
"G      c #528CCE",
"H      c #6BA5DE",
"I      c #7BB5EF",
"J      c #73A5D6",
"K      c #638CB5",
"L      c #7BB5F7",
"M      c #738494",
"N      c #8C8C94",
"O      c #6394CE",
"P      c #84BDF7",
"Q      c #94C6FF",
"R      c #8CB5E7",
"S      c #293142",
"T      c #182131",
"U      c #7BA5D6",
"V      c #8CC6FF",
"W      c #6394D6",
"X      c #73848C",
"Y      c #949C9C",
"Z      c #638CBD",
"`      c #94CEFF",
" .     c #A5D6FF",
"..     c #9CCEF7",
"+.     c #7394B5",
"@.     c #6B84A5",
"#.     c #9CCEFF",
"$.     c #6394BD",
"%.     c #848C8C",
"&.     c #6B7B94",
"*.     c #B5DEFF",
"=.     c #A5CEEF",
"-.     c #94BDD6",
";.     c #9CC6EF",
">.     c #ADD6FF",
",.     c #ADDEFF",
"'.     c #63849C",
").     c #9C9C9C",
"!.     c #84848C",
"~.     c #7B94BD",
"{.     c #B5E7FF",
"].     c #C6EFFF",
"^.     c #BDEFFF",
"/.     c #B5DEF7",
"(.     c #84A5C6",
"_.     c #A5A5A5",
":.     c #7B94AD",
"<.     c #D6FFFF",
"[.     c #DEFFFF",
"}.     c #849CB5",
"|.     c #6B7373",
"1.     c #BDBDBD",
"2.     c #949494",
"3.     c #636363",
"4.     c #63737B",
"5.     c #7B949C",
"6.     c #94B5BD",
"7.     c #9CBDBD",
"8.     c #849CA5",
"9.     c #5A6363",
"0.     c #8C8C8C",
"a.     c #E7E7E7",
"b.     c #7B7B7B",
"c.     c #6B6B6B",
"d.     c #F7F7F7",
". . . . . + @ @ + . . . . . ",
". . . # $ % & * % = - . . . ",
". . ; > , ' ) ! ~ { ] ) . . ",
". ^ / ( _ : < [ } | 1 2 3 . ",
"+ 4 5 6 7 8 9 0 a b c d e f ",
"g h i j k l m n o p q r s t ",
"u v w x y z A A B C x D E F ",
"t G H I J A A A A K L y w M ",
"N O P Q R S A A T U V I W X ",
"Y Z `  ...+.n n @.8 #.V $.%.",
"g &...*.*.=.-.-.;.>.,.Q '.).",
"f !.~.{.].^././.{.^.{.(.!.@ ",
"f _.= :.^.<.[.[.<.^.}.|.).@ ",
"+ 1.2.3.4.5.6.7.8.4.9.0.1.+ ",
". a.# ).b.c.3.3.c.b.).# + . ",
". . d.a.@ - g g - @ + d.. . ",
". . . . a.+ + + + a.. . . . "};
/* XPM */
static const char * const aqua_radio_dis_t_xpm[] = {
"14 17 30 1",
"       c None",
".      c #FFFFFF",
"+      c #EFEFEF",
"@      c #DEDEDE",
"#      c #CECECE",
"$      c #E7E7E7",
"%      c #C6C6C6",
"&      c #6B6B6B",
"*      c #424242",
"=      c #313131",
"-      c #292929",
";      c #4A4A4A",
">      c #B5B5B5",
",      c #ADADAD",
"'      c #525252",
")      c #7B7B7B",
"!      c #949494",
"~      c #393939",
"{      c #9C9C9C",
"]      c #A5A5A5",
"^      c #D6D6D6",
"/      c #848484",
"(      c #737373",
"_      c #636363",
":      c #BDBDBD",
"<      c #000000",
"[      c #212121",
"}      c #8C8C8C",
"|      c #F7F7F7",
"1      c #5A5A5A",
"...++@##@++...",
"..$%&*=-;&>$..",
".+,;')!!)'~{+.",
".#&)]#@@^>/'>.",
"@/)]%@++$^](_^",
",({>%]__{:>{&{",
"!!>%%=<<[>#>!}",
"{>%@%<<<<]$%>}",
"!:@+^~<<-%+^:}",
"{,+|$]_1{@+$,}",
",}+..$^^@|.+!{",
"^},...||...:}#",
"^](]......,({#",
"@:!_){::])_}:@",
"+$%{)&__&){%@+",
"..|$#>,,>#@|..",
"...+$@@@@$+..."};
/* XPM */
static const char * const aqua_sel_back_xpm[] = {
"11 20 4 1",
"       c None",
".      c #295AB5",
"+      c #316BBD",
"@      c #3973C6",
"...........",
"+++++++++++",
"@@@@@@@@@@@",
"+++++++++++",
"...........",
"+++++++++++",
"@@@@@@@@@@@",
"+++++++++++",
"...........",
"+++++++++++",
"@@@@@@@@@@@",
"+++++++++++",
"...........",
"+++++++++++",
"@@@@@@@@@@@",
"+++++++++++",
"...........",
"+++++++++++",
"@@@@@@@@@@@",
"+++++++++++"};
/* XPM */
static const char * const aqua_sldr_act_pty_xpm[] = {
"17 20 125 2",
"       c None",
".      c #EFEFEF",
"+      c #BDBDC6",
"@      c #636384",
"#      c #31316B",
"$      c #292963",
"%      c #29296B",
"&      c #39396B",
"*      c #73738C",
"=      c #C6C6C6",
"-      c #D6D6D6",
";      c #424284",
">      c #4A4AA5",
",      c #8C8CC6",
"'      c #9494CE",
")      c #949CCE",
"!      c #8484C6",
"~      c #39399C",
"{      c #5A5A8C",
"]      c #9C9C9C",
"^      c #10107B",
"/      c #7B8CBD",
"(      c #ADB5DE",
"_      c #ADADD6",
":      c #ADB5D6",
"<      c #A5B5D6",
"[      c #A5ADD6",
"}      c #7B84C6",
"|      c #101073",
"1      c #BDBDBD",
"2      c #292931",
"3      c #18399C",
"4      c #849CC6",
"5      c #9CB5DE",
"6      c #8CA5D6",
"7      c #083194",
"8      c #393939",
"9      c #393942",
"0      c #1052B5",
"a      c #6394C6",
"b      c #84ADD6",
"c      c #84A5D6",
"d      c #7BA5D6",
"e      c #7BADD6",
"f      c #6B9CCE",
"g      c #1052A5",
"h      c #4A4A4A",
"i      c #4A525A",
"j      c #296BC6",
"k      c #4A84BD",
"l      c #5A8CC6",
"m      c #6394CE",
"n      c #316BBD",
"o      c #636363",
"p      c #525A63",
"q      c #4A84CE",
"r      c #5A8CCE",
"s      c #5A94CE",
"t      c #528CCE",
"u      c #737373",
"v      c #63636B",
"w      c #639CDE",
"x      c #739CD6",
"y      c #73A5D6",
"z      c #6BA5D6",
"A      c #73737B",
"B      c #6BA5DE",
"C      c #7BA5DE",
"D      c #84B5E7",
"E      c #8CB5E7",
"F      c #7BA5CE",
"G      c #848484",
"H      c #7B7B84",
"I      c #7BADE7",
"J      c #94C6EF",
"K      c #9CC6EF",
"L      c #94BDEF",
"M      c #8C8C8C",
"N      c #949C9C",
"O      c #739CC6",
"P      c #9CC6F7",
"Q      c #9CCEF7",
"R      c #A5CEFF",
"S      c #ADCEFF",
"T      c #ADD6FF",
"U      c #A5CEF7",
"V      c #7B94B5",
"W      c #ADADAD",
"X      c #73849C",
"Y      c #ADDEFF",
"Z      c #B5DEFF",
"`      c #737B84",
" .     c #CECECE",
"..     c #63737B",
"+.     c #BDE7FF",
"@.     c #C6EFFF",
"#.     c #BDEFFF",
"$.     c #ADD6EF",
"%.     c #636B73",
"&.     c #F7F7F7",
"*.     c #949494",
"=.     c #73848C",
"-.     c #CEF7FF",
";.     c #CEFFFF",
">.     c #C6F7FF",
",.     c #BDDEEF",
"'.     c #6B7373",
").     c #A5A5A5",
"!.     c #DEDEDE",
"~.     c #E7E7E7",
"{.     c #738484",
"].     c #D6FFFF",
"^.     c #DEFFFF",
"/.     c #C6E7E7",
"(.     c #636B6B",
"_.     c #B5B5B5",
":.     c #7B8484",
"<.     c #E7FFFF",
"[.     c #CEE7E7",
"}.     c #EFFFFF",
"|.     c #D6EFEF",
"1.     c #FFFFFF",
"2.     c #8C9494",
"3.     c #EFF7F7",
"4.     c #7B7B7B",
". + @ # $ % % $ $ % $ $ $ & * = . ",
"- ; > , ' ' ) ' ' ' ' ' ' ! ~ { . ",
"] ^ / ( _ : < < ( ( ( : [ ( } | 1 ",
"2 3 4 5 5 5 5 5 5 5 5 5 5 5 6 7 8 ",
"9 0 a b c c b b b b b b d e f g h ",
"i j k l m m a m m m m m m m l n o ",
"p q k r m s m m m m m m m s r t u ",
"v w a x y y y d y y y y y y y z u ",
"A B C D E E E E E E E E E D D F G ",
"H I E J K K K K K K K K K L L F M ",
"N O P Q R R S T R S R R U U R V W ",
"W X Y T Z Z Z Z Z Z Z Z Z Y T ` 1 ",
" .M ..+.@.#.@.#.+.#.@.+.@.$.%.] - ",
"&.- *.=.-.-.-.-.;.-.>.-.,.'.).!.&.",
". ~.1 G {.].^.].^.].^./.(.]  .~.. ",
"~.~.!._.G :.^.<.^.<.[.'.*.= !.~.~.",
". . . ~.1 G :.<.}.|.'.*. .~.. . . ",
"1.1.1.1.&. .*.2.3.4.).!.&.1.1.1.1.",
". . . . . ~.1 M M ]  .~.. . . . . ",
"~.~.~.~.~.~.!.= 1  .!.~.~.~.~.~.~."};
/* XPM */
static const char * const aqua_sldr_dis_pty_xpm[] = {
"17 20 26 1",
"       c None",
".      c #EFEFEF",
"+      c #BDBDBD",
"@      c #737373",
"#      c #4A4A4A",
"$      c #424242",
"%      c #7B7B7B",
"&      c #C6C6C6",
"*      c #D6D6D6",
"=      c #6B6B6B",
"-      c #8C8C8C",
";      c #B5B5B5",
">      c #A5A5A5",
",      c #9C9C9C",
"'      c #525252",
")      c #CECECE",
"!      c #292929",
"~      c #393939",
"{      c #949494",
"]      c #848484",
"^      c #ADADAD",
"/      c #636363",
"(      c #DEDEDE",
"_      c #E7E7E7",
":      c #F7F7F7",
"<      c #FFFFFF",
".+@#$#$$$$$$$#%&.",
"*=-;+++++++++>@@.",
",'+))))))))))),#+",
"!@;)))))))))))>=~",
"$-,++++++++++;{]#",
"'>{>>>>>^>>>>>>,/",
"/;>;;;+;+;;;;;;;@",
"=)+&&&&&&&&&&&&&@",
"%))************+]",
"])*((((((((((((+-",
",;_____.______.^^",
"^{:..::::::....]+",
")-%<<<<<<<<<<_=,*",
":*{-<<<<<<<<.@>(:",
"._+]]<<<<<<_=,)_.",
"__(;]]<<<<_@{&(__",
"..._+]]<<.@{)_...",
"<<<<:){{:%>(:<<<<",
"....._+--,)_.....",
"______(&+)(______"};
/* XPM */
static const char * const aqua_sldr_grv_mid_xpm[] = {
"20 7 8 1",
"       c None",
".      c #525252",
"+      c #737373",
"@      c #949494",
"#      c #ADADAD",
"$      c #B5B5B5",
"%      c #C6C6C6",
"&      c #CECECE",
"....................",
"++++++++++++++++++++",
"@@@@@@@@@@@@@@@@@@@@",
"####################",
"$$$$$$$$$$$$$$$$$$$$",
"%%%%%%%%%%%%%%%%%%%%",
"&&&&&&&&&&&&&&&&&&&&"};
/* XPM */
static const char * const aqua_sldr_grv_tip_left_xpm[] = {
"4 7 18 1",
"       c None",
".      c #E7E7E7",
"+      c #A5A5A5",
"@      c #737373",
"#      c #424242",
"$      c #ADADAD",
"%      c #525252",
"&      c #636363",
"*      c #6B6B6B",
"=      c #8C8C8C",
"-      c #848484",
";      c #9C9C9C",
">      c #B5B5B5",
",      c #C6C6C6",
"'      c #BDBDBD",
")      c #FFFFFF",
"!      c #DEDEDE",
"~      c #CECECE",
".+@#",
"$%&*",
"=@--",
"*-;;",
";;+>",
",;$'",
")!~'"};
/* XPM */
static const char * const aqua_sldr_grv_tip_right_xpm[] = {
"4 7 22 1",
"       c None",
".      c #424242",
"+      c #737373",
"@      c #A5A5A5",
"#      c #E7E7E7",
"$      c #6B6B6B",
"%      c #636363",
"&      c #525252",
"*      c #ADADAD",
"=      c #848484",
"-      c #7B7B7B",
";      c #8C8C8C",
">      c #949494",
",      c #73736B",
"'      c #B5B5B5",
")      c #9C9C9C",
"!      c #A5A59C",
"~      c #BDBDBD",
"{      c #CECEC6",
"]      c #C6C6C6",
"^      c #CECECE",
"/      c #FFFFFF",
".+@#",
"$%&*",
"=-+;",
"@>;,",
"'*)!",
"~'@{",
"]^#/"};
/* XPM */
static const char * const aqua_vsbr_arw_down_xpm[] = {
"14 26 22 1",
"       c None",
".      c #BDBDBD",
"+      c #C6C6C6",
"@      c #CECECE",
"#      c #D6D6D6",
"$      c #DEDEDE",
"%      c #E7E7E7",
"&      c #EFEFEF",
"*      c #F7F7F7",
"=      c #B5B5B5",
"-      c #ADADAD",
";      c #A5A5A5",
">      c #9C9C9C",
",      c #949494",
"'      c #FFFFFF",
")      c #8C8C8C",
"!      c #4A4A4A",
"~      c #424242",
"{      c #5A5A5A",
"]      c #525252",
"^      c #737373",
"/      c #636363",
"..+@#$%&******",
"..+@#$%&&****&",
"=.+@#$$%&&**&&",
"==++@#$%%&&&%%",
"==.+@##$$%%%$$",
".-=.+@##$$$$#+",
".;-=++@@##@@.+",
"..;;-..++.=-=%",
".#+>,;---;,-%'",
".#&#->,),;+*''",
".#**$#$%%&*'''",
".#***&$%&&*'''",
".#*'''*%&&*'''",
".#*'''*%&&*'''",
".#*'''*%&&*'''",
".#*';!!~~~,'''",
".#*'&{!~~]&'''",
".#*''=!~~-*'''",
".#*'''^~/&*'''",
".#*'''#~+&*'''",
".#**'''-%&*'''",
".#*''''$%&*'''",
"=#**''*$%&*''*",
"=#**'*&$%%&***",
"=@&*&$##$%&***",
"--.=--=.+@@###"};
/* XPM */
static const char * const aqua_vsbr_arw_up_xpm[] = {
"14 26 22 1",
"       c None",
".      c #B5B5B5",
"+      c #CECECE",
"@      c #EFEFEF",
"#      c #F7F7F7",
"$      c #DEDEDE",
"%      c #D6D6D6",
"&      c #E7E7E7",
"*      c #FFFFFF",
"=      c #BDBDBD",
"-      c #ADADAD",
";      c #424242",
">      c #C6C6C6",
",      c #737373",
"'      c #636363",
")      c #4A4A4A",
"!      c #5A5A5A",
"~      c #525252",
"{      c #A5A5A5",
"]      c #949494",
"^      c #9C9C9C",
"/      c #8C8C8C",
".+@#@$%%$&@###",
".%##*#@$&&@###",
".%##**#$&@#**#",
"=%#****$&@#***",
"=%##***-&@#***",
"=%#***%;>@#***",
"=%#***,;'@#***",
"=%#**.);;-#***",
"=%#*@!);;~@***",
"=%#*{));;;]***",
"=%#***#&@@#***",
"=%#***#&&@#***",
"=%#***@&@@#***",
"=%###@$&@@#***",
"=%##$%$&&@#***",
"=%@%-^]/]{>#**",
"=%>^]{---{]-&*",
"=={{-==>>=.-.&",
"={-.>>++%%++=>",
"=-.=>+%%$$$$%>",
"..=>+%%$$&&&$$",
"..>>+%$&&@@@&&",
".=>+%$$&@@##@@",
"==>+%$&@@####@",
"==>+%%$&@@@###",
"==>+%$&@######"};
/* XPM */
static const char * const aqua_vsbr_back_fill_xpm[] = {
"14 16 10 1",
"       c None",
".      c #BDBDBD",
"+      c #C6C6C6",
"@      c #CECECE",
"#      c #D6D6D6",
"$      c #DEDEDE",
"%      c #E7E7E7",
"&      c #EFEFEF",
"*      c #F7F7F7",
"=      c #FFFFFF",
".+@#$%&**====*",
".+@#$%&**====*",
".+@#$%&**====*",
".+@#$%&**====*",
".+@#$%&**====*",
".+@#$%&**====*",
".+@#$%&**====*",
".+@#$%&**====*",
".+@#$%&**====*",
".+@#$%&**====*",
".+@#$%&**====*",
".+@#$%&**====*",
".+@#$%&**====*",
".+@#$%&**====*",
".+@#$%&**====*",
".+@#$%&**====*"};
/* XPM */
static const char * const aqua_vsbr_fill_xpm[] = {
"14 16 56 1",
"       c None",
".      c #ADADAD",
"+      c #0039B5",
"@      c #739CDE",
"#      c #8CBDE7",
"$      c #8CB5E7",
"%      c #7BB5E7",
"&      c #428CE7",
"*      c #529CEF",
"=      c #63ADFF",
"-      c #73BDFF",
";      c #84CEFF",
">      c #94DEFF",
",      c #84D6FF",
"'      c #5A5A5A",
")      c #94BDE7",
"!      c #84B5E7",
"~      c #428CDE",
"{      c #5A9CEF",
"]      c #6BADFF",
"^      c #7BBDFF",
"/      c #8CCEFF",
"(      c #8CD6FF",
"_      c #73A5DE",
":      c #4294E7",
"<      c #5AA5EF",
"[      c #6BB5FF",
"}      c #7BC6FF",
"|      c #94D6FF",
"1      c #84B5EF",
"2      c #4A94E7",
"3      c #84C6FF",
"4      c #9CE7FF",
"5      c #94BDEF",
"6      c #8CB5EF",
"7      c #63A5F7",
"8      c #73B5FF",
"9      c #8CBDEF",
"0      c #5294E7",
"a      c #63A5EF",
"b      c #A5E7FF",
"c      c #9CDEFF",
"d      c #73A5E7",
"e      c #9CC6EF",
"f      c #63ADF7",
"g      c #529CE7",
"h      c #6BADF7",
"i      c #A5EFFF",
"j      c #7BADE7",
"k      c #7BA5DE",
"l      c #7B9CDE",
"m      c #398CE7",
"n      c #8CDEFF",
"o      c #3984E7",
"p      c #4A94EF",
"q      c #7BCEFF",
".+@#$%&*=-;>,'",
".+@)$!~{]^/>('",
".+_)#!:<[}/>|'",
".+_)#12<[3|4|'",
".+_)562783|4>'",
".+_5590a8/>bc'",
".+de590f^/cbc'",
".+d559gh^/ci4'",
".+j5590h^/cbc'",
".+k))90783|bc'",
".+_5592a83|4|'",
".+_)#$2<83(c|'",
".+_)#1&<[}(>|'",
".+@)$!~*]^;>/'",
".+l$$jm*=-;n;'",
".+@$!jop=-q(;'"};
/* XPM */
static const char * const aqua_vsbr_tip_down_xpm[] = {
"14 10 83 1",
"       c None",
".      c #ADADAD",
"+      c #0031B5",
"@      c #73A5DE",
"#      c #94BDEF",
"$      c #8CB5EF",
"%      c #4A94E7",
"&      c #63A5F7",
"*      c #73B5FF",
"=      c #84C6FF",
"-      c #94DEFF",
";      c #9CE7FF",
">      c #5A5A5A",
",      c #0029AD",
"'      c #94BDE7",
")      c #8CBDE7",
"!      c #84B5E7",
"~      c #4A8CE7",
"{      c #5A9CEF",
"]      c #6BADFF",
"^      c #7BC6FF",
"/      c #8CCEFF",
"(      c #8CD6FF",
"_      c #6B94DE",
":      c #8CB5E7",
"<      c #7BADE7",
"[      c #3984DE",
"}      c #529CEF",
"|      c #B5B5B5",
"1      c #0821A5",
"2      c #4273CE",
"3      c #73ADE7",
"4      c #317BD6",
"5      c #428CE7",
"6      c #63ADF7",
"7      c #73BDFF",
"8      c #6BA5EF",
"9      c #636363",
"0      c #29399C",
"a      c #0839B5",
"b      c #4A84D6",
"c      c #5294DE",
"d      c #4284D6",
"e      c #2973D6",
"f      c #3984D6",
"g      c #5AA5F7",
"h      c #5284BD",
"i      c #737373",
"j      c #5A639C",
"k      c #0021AD",
"l      c #0039B5",
"m      c #004ABD",
"n      c #085AC6",
"o      c #186BCE",
"p      c #2973CE",
"q      c #428CDE",
"r      c #5294E7",
"s      c #526B8C",
"t      c #949494",
"u      c #BDBDBD",
"v      c #9C9CAD",
"w      c #0039BD",
"x      c #2173CE",
"y      c #4273AD",
"z      c #737B84",
"A      c #8484A5",
"B      c #1063C6",
"C      c #3163A5",
"D      c #6B7B8C",
"E      c #D6D6D6",
"F      c #C6C6C6",
"G      c #A5A5B5",
"H      c #5A6BA5",
"I      c #2142A5",
"J      c #0039AD",
"K      c #0042AD",
"L      c #1852A5",
"M      c #4A739C",
"N      c #8C949C",
"O      c #DEDEDE",
"P      c #EFEFEF",
"Q      c #CECECE",
"R      c #F7F7F7",
".+@##$%&*=-;->",
".,@')!~{]^/-(>",
".,_):<[}&*=(=>",
"|12:<345}67^89",
"|0abcdef%}g6hi",
"|jklmnopfq%rst",
"uv0,wmnox4fyz|",
"uuA0,lmnBoCD|E",
"uFFGHIJKLMNuOP",
"uFQQQFuuuQOPRR"};
/* XPM */
static const char * const aqua_vsbr_tip_up_xpm[] = {
"14 11 89 1",
"       c None",
".      c #BDBDBD",
"+      c #C6C6C6",
"@      c #CECECE",
"#      c #B5B5B5",
"$      c #DEDEDE",
"%      c #EFEFEF",
"&      c #F7F7F7",
"*      c #A5A5B5",
"=      c #5A73A5",
"-      c #2152A5",
";      c #004AB5",
">      c #0852B5",
",      c #2963A5",
"'      c #5A7BA5",
")      c #8C949C",
"!      c #8484A5",
"~      c #294AA5",
"{      c #0042BD",
"]      c #004ABD",
"^      c #085AC6",
"/      c #186BCE",
"(      c #2973CE",
"_      c #317BCE",
":      c #396BA5",
"<      c #6B7B8C",
"[      c #D6D6D6",
"}      c #9C9CAD",
"|      c #29429C",
"1      c #0031B5",
"2      c #317BD6",
"3      c #4284DE",
"4      c #428CDE",
"5      c #4A73AD",
"6      c #7B7B84",
"7      c #5A639C",
"8      c #0029AD",
"9      c #0042B5",
"0      c #0052C6",
"a      c #1063C6",
"b      c #2173CE",
"c      c #317BDE",
"d      c #4A94E7",
"e      c #529CEF",
"f      c #529CE7",
"g      c #52738C",
"h      c #949494",
"i      c #29399C",
"j      c #427BD6",
"k      c #4A8CD6",
"l      c #2973D6",
"m      c #398CE7",
"n      c #4A94EF",
"o      c #5AA5F7",
"p      c #63ADFF",
"q      c #5284BD",
"r      c #737373",
"s      c #08219C",
"t      c #396BCE",
"u      c #84ADE7",
"v      c #7BADE7",
"w      c #6BA5E7",
"x      c #428CE7",
"y      c #529CF7",
"z      c #6BBDFF",
"A      c #73BDFF",
"B      c #639CEF",
"C      c #636363",
"D      c #ADADAD",
"E      c #0018A5",
"F      c #6B8CD6",
"G      c #8CB5E7",
"H      c #73ADE7",
"I      c #4294EF",
"J      c #63B5FF",
"K      c #73C6FF",
"L      c #7BC6FF",
"M      c #5A5A5A",
"N      c #7394DE",
"O      c #3184DE",
"P      c #7BCEFF",
"Q      c #0021AD",
"R      c #739CDE",
"S      c #8CBDE7",
"T      c #84B5E7",
"U      c #3984DE",
"V      c #8CD6FF",
"W      c #84C6FF",
"X      c #84CEFF",
"..+@@+.#.+$%%&",
"...*=-;>,').$%",
"..!~{]^/(_:<#[",
".}|1]^/(23456#",
"#7890abc4defgh",
"#i1jk2lmnoppqr",
"#stuvwcxypzABC",
"DEFGvHcIoJKLAM",
"DENGvHOIoJKPAM",
"DQRSTvUnpAPVWM",
"DQRSGTxepAXVXM"};
/* XPM */
static const char * const aqua_hsbr_dis_fill_xpm[] = {
"16 14 11 1",
"       c None",
".      c #ADADAD",
"+      c #949494",
"@      c #8C8C8C",
"#      c #E7E7E7",
"$      c #FFFFFF",
"%      c #F7F7F7",
"&      c #DEDEDE",
"*      c #D6D6D6",
"=      c #EFEFEF",
"-      c #5A5A5A",
"................",
"+@@@@@@@@@@@@@@@",
"################",
"$%%%%%%%%%%%%%%%",
"$%%%%%%%%%%%%%%%",
"%%%%%%%%%%%%%%%%",
"&*&&&&&&&*&&&&&&",
"################",
"================",
"%%%%%$%%%%%%%$%%",
"$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$",
"----------------"};
/* XPM */
static const char * const aqua_hsbr_tip_dis_left_xpm[] = {
"10 14 21 1",
"       c None",
".      c #BDBDBD",
"+      c #B5B5B5",
"@      c #ADADAD",
"#      c #A5A5A5",
"$      c #D6D6D6",
"%      c #8C8C8C",
"&      c #848484",
"*      c #949494",
"=      c #EFEFEF",
"-      c #9C9C9C",
";      c #C6C6C6",
">      c #DEDEDE",
",      c #E7E7E7",
"'      c #CECECE",
")      c #7B7B7B",
"!      c #F7F7F7",
"~      c #FFFFFF",
"{      c #737373",
"]      c #6B6B6B",
"^      c #5A5A5A",
"....+@@#@@",
"$$+%%&&%**",
"=.&&*-;>,,",
"'))-#'=!!~",
"-)-@+>!!!!",
"&%@+.$==!!",
"{-+;;'$$$>",
"]#.'$$>>,,",
"]*;$>>,===",
"&)'$>,==!!",
"#^->,==!!~",
",)]@=!!~~~",
"!;)]*.=~~~",
"~!;%{]^^^^"};
/* XPM */
static const char * const aqua_hsbr_tip_dis_right_xpm[] = {
"10 14 20 1",
"       c None",
".      c #ADADAD",
"+      c #B5B5B5",
"@      c #BDBDBD",
"#      c #949494",
"$      c #8C8C8C",
"%      c #848484",
"&      c #A5A5A5",
"*      c #C6C6C6",
"=      c #E7E7E7",
"-      c #DEDEDE",
";      c #9C9C9C",
">      c #CECECE",
",      c #F7F7F7",
"'      c #D6D6D6",
")      c #EFEFEF",
"!      c #FFFFFF",
"~      c #5A5A5A",
"{      c #636363",
"]      c #737373",
"...+++@@@@",
"##$%$#&@**",
"==-*&#$;*>",
",,,,'&;$.>",
",,,,-+.;;>",
",,,)'@+.#*",
"-'''>**+;@",
"==--''>@&@",
"))==-''*;@",
",,))=-'>;>",
"!!,)==-&;-",
"!!!,)=.$@)",
"!!!)@#$+-,",
"~~~{]#+'),"};
/* XPM */
static const char * const aqua_vsbr_dis_fill_xpm[] = {
"14 16 9 1",
"       c None",
".      c #ADADAD",
"+      c #8C8C8C",
"@      c #E7E7E7",
"#      c #F7F7F7",
"$      c #DEDEDE",
"%      c #EFEFEF",
"&      c #FFFFFF",
"*      c #5A5A5A",
".+@###$@%#&&&*",
".+@###$@%#&&&*",
".+@###$@%#&&&*",
".+@###$@%#&&&*",
".+@###$@%#&&&*",
".+@###$@%#&&&*",
".+@###$@%#&&&*",
".+@###$@%#&&&*",
".+@###$@%#&&&*",
".+@###$@%#&&&*",
".+@###$@%#&&&*",
".+@###$@%#&&&*",
".+@###$@%#&&&*",
".+@###$@%#&&&*",
".+@###$@%#&&&*",
".+@###$@%#&&&*"};
/* XPM */
static const char * const aqua_vsbr_tip_dis_down_xpm[] = {
"14 10 20 1",
"       c None",
".      c #ADADAD",
"+      c #949494",
"@      c #E7E7E7",
"#      c #FFFFFF",
"$      c #F7F7F7",
"%      c #DEDEDE",
"&      c #EFEFEF",
"*      c #5A5A5A",
"=      c #D6D6D6",
"-      c #8C8C8C",
";      c #B5B5B5",
">      c #848484",
",      c #C6C6C6",
"'      c #636363",
")      c #9C9C9C",
"!      c #CECECE",
"~      c #737373",
"{      c #A5A5A5",
"]      c #BDBDBD",
".+@#$$%@&$###*",
".+@$$$=@&$$##*",
".-%$$&=%&&$##*",
";>,&$&=%@&&$&'",
";-)!%=!=%@&$,~",
";++{;],=%%@&)+",
"]{-).;,!==%;-;",
"]])-).;],!{-;=",
"],,.)+){)))]%&",
"],!!!,]]]!%&$$"};
/* XPM */
static const char * const aqua_vsbr_tip_dis_up_xpm[] = {
"14 11 20 1",
"       c None",
".      c #BDBDBD",
"+      c #C6C6C6",
"@      c #CECECE",
"#      c #B5B5B5",
"$      c #DEDEDE",
"%      c #EFEFEF",
"&      c #F7F7F7",
"*      c #ADADAD",
"=      c #9C9C9C",
"-      c #949494",
";      c #A5A5A5",
">      c #8C8C8C",
",      c #D6D6D6",
"'      c #E7E7E7",
")      c #737373",
"!      c #848484",
"~      c #636363",
"{      c #FFFFFF",
"]      c #5A5A5A",
"..+@@+.#.+$%%&",
"...*=-=;===.$%",
"..=>=*#.+@;>#,",
".;>=*#+@,,$*>#",
"#--;#.+,,$''--",
"#>;,$,@,$''%.)",
"#!+&&%,$'%%&%~",
"*>$&&&,$'%&{{]",
"*-'&&&,'%&{{{]",
"*-'&&&$'%&{{{]",
"*-%{&&$'%&{{{]"};
/* XPM */
static const char * const aqua_toolbtn_nrm_left_xpm[] = {
"23 21 18 1",
"       c None",
".      c #F7F7F7",
"+      c #CECECE",
"@      c #ADADAD",
"#      c #9C9C9C",
"$      c #A5A5A5",
"%      c #737373",
"&      c #BDBDBD",
"*      c #B5B5B5",
"=      c #D6D6D6",
"-      c #DEDEDE",
";      c #E7E7E7",
">      c #EFEFEF",
",      c #C6C6C6",
"'      c #FFFFFF",
")      c #949494",
"!      c #848484",
"~      c #7B7B7B",
".+@#$$$$$$$$$$$$$$$$$$%",
"&$*+=-----------------&",
"#@+;;;>>>;>>>>>>>>>>>>+",
"#&-;>>>>>>>>>>>>>>>>>>,",
"*,;;>>>;;;;;>;;;;;;>>>&",
"&=-;;;;;;;;;;;;;;;;;;;$",
",=;;;;;;;;;;;;;;;;;;;;#",
",=;;;;;>;;>;>;>;;>;;;;#",
"+-;;>>>>>>>>>>>>>>>>>>$",
"=;>>>>>>>>>>>>>>>>>>.>@",
"=;>>..................*",
"=;....................&",
"=>....................,",
"->..................''+",
"->.''''...'.''''.'''''+",
"-.''''''''''''''''''''-",
"+.''''''''''''''''''''-",
"@.''''''''''''''''''''-",
"#+''''''''''''''''''''-",
"@)*;''''''''''''''''''-",
"-&#!~%%%%%%%%%%%%%%%%%%"};
/* XPM */
static const char * const aqua_toolbtn_nrm_mid_xpm[] = {
"23 21 14 1",
"       c None",
".      c #A5A5A5",
"+      c #737373",
"@      c #DEDEDE",
"#      c #BDBDBD",
"$      c #E7E7E7",
"%      c #EFEFEF",
"&      c #CECECE",
"*      c #C6C6C6",
"=      c #9C9C9C",
"-      c #F7F7F7",
";      c #ADADAD",
">      c #B5B5B5",
",      c #FFFFFF",
"......................+",
"@@@@@@@@@@@@@@@@@@@@@@#",
"$$%%%%%$%$%%%%$%%%%%%%&",
"%%%%%%%%%%%%%%%%%%%%%%*",
"%%%%%$$$$$$$$$$$$%%%%%#",
"$$$$$$$$$$$$$$$$$$$$$$.",
"$$$$$$$$$$$$$$$$$$$$$$=",
"$$$$$$$$$$$$$$$$$$$$$$=",
"%%%%%%%%%%%%%%%%%%%%%%.",
"%%%%%%%%%%%%%%%%%%%%--;",
"----%%%%%%%%%%%%%----->",
"----------------------#",
"----------------------*",
",,,,-------------,,,,,&",
",,,,,,,,,,,,,,,,,,,,,,&",
",,,,,-,,,,,,,,,,,,,,,,@",
",,,,,,,,,,,,,,,,,,,,,,@",
",,,,,,,,,,,,,,,,,,,,,,@",
",,,,,,,,,,,,,,,,,,,,,,@",
",,,,,,,,,,,,,,,,,,,,,,@",
"+++++++++++++++++++++++"};
/* XPM */
static const char * const aqua_toolbtn_nrm_right_xpm[] = {
"23 21 18 1",
"       c None",
".      c #A5A5A5",
"+      c #9C9C9C",
"@      c #ADADAD",
"#      c #CECECE",
"$      c #F7F7F7",
"%      c #DEDEDE",
"&      c #D6D6D6",
"*      c #B5B5B5",
"=      c #BDBDBD",
"-      c #EFEFEF",
";      c #E7E7E7",
">      c #C6C6C6",
",      c #FFFFFF",
"'      c #949494",
")      c #737373",
"!      c #7B7B7B",
"~      c #848484",
"...................+@#$",
"%%%%%%%%%%%%%%%%%%&#*.=",
"-------------;---;;;#@+",
"-------------------;%=+",
"----;;;;;;;;;;;;;--;;>*",
";;;;;;;;;;;;;;;;;;;;%&=",
";;;;;;;;;;;;;;;;;;;;;&>",
";;;;;;;;;;;;;;;;;;;;;&>",
"------;---;;--;;-;;;;%#",
"---------------------;&",
"$$-------------------;&",
"$$$----------------$$;&",
"$$$$$$$----------$$$$-&",
",,$$$$$$$$$$$$$$$$$$$-%",
",,,$$$$$$$$$$$$$$$,,$-%",
",,,,,$$$$$$$$$$$$$,,,$%",
",,,,,,,,,,,,,,,,,,,,,$#",
",,,,,,,,,,,,,,,,,,,,,$@",
",,,,,,,,,,,,,,,,,,,,,#+",
",,,,,,,,,,,,,,,,,,,;*'@",
"))))))))))))))))))!~+=%"};
/* XPM */
static const char * const aqua_toolbtn_act_mid_xpm[] = {
"23 21 33 1",
"       c None",
".      c #1052AD",
"+      c #737373",
"@      c #9CBDE7",
"#      c #9CC6E7",
"$      c #BDBDBD",
"%      c #BDD6EF",
"&      c #CECECE",
"*      c #B5CEEF",
"=      c #C6C6C6",
"-      c #A5C6E7",
";      c #84B5E7",
">      c #84B5DE",
",      c #A5A5A5",
"'      c #73ADE7",
")      c #9C9C9C",
"!      c #7BB5EF",
"~      c #7BB5E7",
"{      c #84BDF7",
"]      c #ADADAD",
"^      c #8CC6FF",
"/      c #8CC6F7",
"(      c #B5B5B5",
"_      c #94CEFF",
":      c #9CD6FF",
"<      c #A5DEFF",
"[      c #ADDEFF",
"}      c #B5E7FF",
"|      c #ADE7FF",
"1      c #DEDEDE",
"2      c #B5EFFF",
"3      c #BDF7FF",
"4      c #BDFFFF",
"......................+",
"@#####@#####@#####@###$",
"%%%%%%%%%%%%%%%%%%%%%%&",
"**********************=",
"----------------------$",
";;;>>>>>>>>>>>>>>>;;;;,",
"'''''''''''''''''''''')",
"'''''''''''''''''''''')",
"!!!~~~~~~~~~~~~~~~!!!!,",
"{{{{{{{{{{{{{{{{{{{{{{]",
"^^^^^^^^^^^^^^^^^/^^^^(",
"______________________$",
"::::::::::::::::::::::=",
"<<<<<<<<<<<<<<<<<<<<<<&",
"[[[[[[[[[[[[[[[[[[[[[[&",
"}}}|||||||||||||||}}}}1",
"22222222222222222222221",
"33333333333333333333331",
"43333343333343333343331",
"33333333333333333333331",
"+++++++++++++++++++++++"};
/* XPM */
static const char * const aqua_chk_psh_f_xpm[] = {
"15 18 27 1",
"       c None",
".      c #EFEFEF",
"+      c #F7F7F7",
"@      c #DEDEDE",
"#      c #C6C6C6",
"$      c #D6D6D6",
"%      c #292929",
"&      c #424242",
"*      c #181818",
"=      c #BDBDBD",
"-      c #FFFFFF",
";      c #949494",
">      c #848484",
",      c #A5A5A5",
"'      c #8C8C8C",
")      c #4A4A4A",
"!      c #737373",
"~      c #7B7B7B",
"{      c #B5B5B5",
"]      c #ADADAD",
"^      c #636363",
"/      c #9C9C9C",
"(      c #E7E7E7",
"_      c #6B6B6B",
":      c #CECECE",
"<      c #5A5A5A",
"[      c #525252",
"...............",
"+@##########$++",
"$%%&&&&&&&&%*=-",
";&>,,,,,,,,')!+",
"~!,{]{]]{{],~^.",
"~~;;///;//;;>^.",
"~~';;;;;;;;'>^+",
"!>;//,//,///'^.",
"!;/]]]]]]]],;^(",
"!/]{{{{{{{{]/^.",
"~,{========{,_+",
"!]#::::::::#]^.",
"!]:$$$$$$$$:{^(",
"~,$$$$$$$$$${_(",
",^';'''''';'_;.",
"=~<[))))))[<~{(",
"$];'''''''';]:(",
"+.(@@@@@@@@((.+"};
/* XPM */
static const char * const aqua_chk_psh_t_xpm[] = {
"15 18 123 2",
"       c None",
".      c #EFEFEF",
"+      c #BDBDBD",
"@      c #000000",
"#      c #424242",
"$      c #F7F7F7",
"%      c #DEDEDE",
"&      c #C6C6C6",
"*      c #CECECE",
"=      c #5A5A5A",
"-      c #ADADAD",
";      c #D6D6D6",
">      c #212129",
",      c #000031",
"'      c #081052",
")      c #101852",
"!      c #101052",
"~      c #000021",
"{      c #FFFFFF",
"]      c #949494",
"^      c #00085A",
"/      c #5A6B9C",
"(      c #8C94BD",
"_      c #9494BD",
":      c #949CC6",
"<      c #848CB5",
"[      c #737373",
"}      c #7B7B7B",
"|      c #10428C",
"1      c #6B8CBD",
"2      c #849CC6",
"3      c #7B9CCE",
"4      c #7BA5CE",
"5      c #84A5CE",
"6      c #84ADD6",
"7      c #42526B",
"8      c #63636B",
"9      c #004294",
"0      c #2963AD",
"a      c #296BBD",
"b      c #2973BD",
"c      c #296BB5",
"d      c #2963A5",
"e      c #00428C",
"f      c #5A5A6B",
"g      c #10529C",
"h      c #00396B",
"i      c #000829",
"j      c #1863B5",
"k      c #105AAD",
"l      c #105AA5",
"m      c #186BBD",
"n      c #001031",
"o      c #00214A",
"p      c #295A9C",
"q      c #3173B5",
"r      c #295A94",
"s      c #3973BD",
"t      c #397BC6",
"u      c #21528C",
"v      c #316BB5",
"w      c #296BA5",
"x      c #525A63",
"y      c #4273A5",
"z      c #5284C6",
"A      c #315A84",
"B      c #001029",
"C      c #5A94CE",
"D      c #528CC6",
"E      c #31638C",
"F      c #4A7BB5",
"G      c #E7E7E7",
"H      c #5284B5",
"I      c #6394C6",
"J      c #6B9CD6",
"K      c #000008",
"L      c #5284AD",
"M      c #396384",
"N      c #081831",
"O      c #5A8CB5",
"P      c #525A6B",
"Q      c #638CB5",
"R      c #739CCE",
"S      c #7BADD6",
"T      c #527394",
"U      c #081821",
"V      c #638CAD",
"W      c #6B94BD",
"X      c #5A6B73",
"Y      c #73737B",
"Z      c #739CBD",
"`      c #8CBDD6",
" .     c #8CB5D6",
"..     c #001018",
"+.     c #314A5A",
"@.     c #94BDD6",
"#.     c #5A636B",
"$.     c #7B9CC6",
"%.     c #94C6D6",
"&.     c #9CCED6",
"*.     c #638C94",
"=.     c #84A5B5",
"-.     c #94C6CE",
";.     c #8CBDCE",
">.     c #7BA5D6",
",.     c #7B9CBD",
"'.     c #ADD6D6",
").     c #B5D6D6",
"!.     c #8CBDBD",
"~.     c #A5CECE",
"{.     c #A5D6D6",
"].     c #84ADCE",
"^.     c #636B73",
"/.     c #A5A5A5",
"(.     c #52636B",
"_.     c #73949C",
":.     c #7B9CA5",
"<.     c #7B949C",
"[.     c #526B73",
"}.     c #525252",
"|.     c #4A4A4A",
"1.     c #B5B5B5",
"2.     c #8C8C8C",
". . . . . . . . . . . . + @ # ",
"$ % & & & & & & & & & * = @ - ",
"; > , ' ) ) ! ! ) ) ! ~ @ = { ",
"] ^ / ( ( ( ( ( _ : < @ @ [ $ ",
"} | 1 2 3 4 3 3 5 6 7 @ ~ 8 . ",
"} 9 0 a b c c c c d @ @ e f . ",
"} g h @ i j k l m n @ o l f $ ",
"[ p q @ @ r s t u @ @ v w x . ",
"[ y z A @ B C D B @ E z F x G ",
"[ H I J K @ L M @ N J I O P . ",
"} Q R S T @ U U @ V 6 R W X $ ",
"Y Z 6 `  ...@ @ +.@. .6 R #.. ",
"Y $.` %.&.*.@ @ =.&.-.;.>.#.G ",
"} ,.'.).).'.!.~.).).).{.].^.G ",
"/.(._.:._._.:.<._._.:._.[.] . ",
"+ } = }.|.|.|.|.|.|.}.= } 1.G ",
"; - ] 2.2.2.2.2.2.2.2.] - * G ",
"$ . G % % % % % % % % G G . $ "};
/* XPM */
static const char * const aqua_radio_psh_f_xpm[] = {
"14 17 29 1",
"       c None",
".      c #F7F7F7",
"+      c #E7E7E7",
"@      c #D6D6D6",
"#      c #EFEFEF",
"$      c #CECECE",
"%      c #737373",
"&      c #313131",
"*      c #181818",
"=      c #101010",
"-      c #6B6B6B",
";      c #BDBDBD",
">      c #ADADAD",
",      c #393939",
"'      c #5A5A5A",
")      c #292929",
"!      c #9C9C9C",
"~      c #FFFFFF",
"{      c #C6C6C6",
"]      c #525252",
"^      c #7B7B7B",
"/      c #8C8C8C",
"(      c #949494",
"_      c #A5A5A5",
":      c #4A4A4A",
"<      c #DEDEDE",
"[      c #B5B5B5",
"}      c #848484",
"|      c #636363",
".....+@@+.....",
"###$%&*=&-;###",
"..>,&'%%',)!..",
"~{]]^!>>!/',>~",
"+%]^(_>>>!^]:<",
"['%}(!!!((/%:!",
"(%}(!!!!!!(}%%",
"//(_>>>>>__(/^",
"//_[;;;;;[[_(%",
"(^[;{{$${{;[}^",
"[-[{$@@@$${[-_",
"@^}{@@@@@@$/%$",
"<>'^{@@@@$^'_@",
"+{(-]-//%]|/{+",
".#$_}-||-}_$+.",
"~~.+$[>>[$<.~~",
"....#++++#...."};
/* XPM */
static const char * const aqua_radio_psh_t_xpm[] = {
"14 17 120 2",
"       c None",
".      c #F7F7F7",
"+      c #E7E7E7",
"@      c #D6D6D6",
"#      c #EFEFEF",
"$      c #CECECE",
"%      c #737373",
"&      c #313131",
"*      c #080808",
"=      c #000000",
"-      c #6B6B6B",
";      c #BDBDBD",
">      c #ADADAD",
",      c #29294A",
"'      c #18184A",
")      c #52526B",
"!      c #6B6B7B",
"~      c #6B737B",
"{      c #5A5A6B",
"]      c #181839",
"^      c #9C9C9C",
"/      c #FFFFFF",
"(      c #C6C6C6",
"_      c #29295A",
":      c #08186B",
"<      c #525A9C",
"[      c #8C8CBD",
"}      c #ADA5C6",
"|      c #B5ADCE",
"1      c #9C9CBD",
"2      c #6B73A5",
"3      c #212973",
"4      c #52527B",
"5      c #00216B",
"6      c #104A94",
"7      c #4A7BB5",
"8      c #6B94C6",
"9      c #849CD6",
"0      c #84A5D6",
"a      c #7B9CCE",
"b      c #5A84BD",
"c      c #184A9C",
"d      c #00106B",
"e      c #31315A",
"f      c #DEDEDE",
"g      c #B5B5B5",
"h      c #10296B",
"i      c #00428C",
"j      c #185AA5",
"k      c #316BB5",
"l      c #315A94",
"m      c #183142",
"n      c #182939",
"o      c #295284",
"p      c #316BAD",
"q      c #185AAD",
"r      c #00398C",
"s      c #00105A",
"t      c #949494",
"u      c #084284",
"v      c #2163A5",
"w      c #3173BD",
"x      c #3973B5",
"y      c #000008",
"z      c #29639C",
"A      c #003984",
"B      c #73737B",
"C      c #8C8C8C",
"D      c #2963A5",
"E      c #427BB5",
"F      c #528CC6",
"G      c #4A7BAD",
"H      c #39638C",
"I      c #528CCE",
"J      c #215AA5",
"K      c #396BA5",
"L      c #5A94CE",
"M      c #6B9CD6",
"N      c #638CBD",
"O      c #000818",
"P      c #527BAD",
"Q      c #639CD6",
"R      c #396BAD",
"S      c #426B94",
"T      c #6BA5D6",
"U      c #7BADD6",
"V      c #73A5CE",
"W      c #4A6B8C",
"X      c #425A7B",
"Y      c #73A5D6",
"Z      c #396B9C",
"`      c #7B7B7B",
" .     c #4A6373",
"..     c #8CB5D6",
"+.     c #7BA5C6",
"@.     c #6B94AD",
"#.     c #739CC6",
"$.     c #84ADD6",
"%.     c #84B5D6",
"&.     c #42637B",
"*.     c #A5A5A5",
"=.     c #527394",
"-.     c #8CBDD6",
";.     c #9CC6D6",
">.     c #94C6D6",
",.     c #8CB5CE",
"'.     c #5A7BA5",
").     c #5A5A5A",
"!.     c #527384",
"~.     c #ADD6D6",
"{.     c #B5D6D6",
"].     c #5A7394",
"^.     c #52525A",
"/.     c #4A525A",
"(.     c #5A6B7B",
"_.     c #738C94",
":.     c #73949C",
"<.     c #5A7384",
"[.     c #636363",
"}.     c #848484",
". . . . . + @ @ + . . . . . ",
"# # # $ % & * = & - ; # # # ",
". . > , ' ) ! ~ { , ] ^ . . ",
"/ ( _ : < [ } | 1 2 3 ' > / ",
"+ 4 5 6 7 8 9 0 a b c d e f ",
"g h i j k l m n o p q r s ^ ",
"t u v w x y = = = z w j A B ",
"C D E F G = = = = H I x J ! ",
"C K L M N O = = y P Q F R ! ",
"t S T U V W n n X 8 Y Q Z ` ",
"g  .V ....+.@.@.#.$.%.M &.*.",
"@ ` =.-.;.>.,.,.-.>.-.'.% $ ",
"f > ).!.>.~.{.{.~.>.].^.*.@ ",
"+ ( t - /.(._.:.<./.[.C ( + ",
". # $ *.}.- [.[.- }.*.$ + . ",
"/ / . + $ g > > g $ f . / / ",
". . . . # + + + + # . . . . "};
/* XPM */
static const char * const aqua_toolbtn_psh_mid_xpm[] = {
"23 21 14 1",
"       c None",
".      c #636363",
"+      c #737373",
"@      c #BDBDBD",
"#      c #B5B5B5",
"$      c #C6C6C6",
"%      c #CECECE",
"&      c #A5A5A5",
"*      c #9C9C9C",
"=      c #D6D6D6",
"-      c #ADADAD",
";      c #DEDEDE",
">      c #E7E7E7",
",      c #6B6B6B",
"......................+",
"@@@@@@#@@@@@@@@@#@@@@@@",
"$$%%%%%%%%%%%$%%%%%%%%%",
"%%%%%%%%%%%%%%%%%%%%%%$",
"%%%$$$$$$$$$$$$$$$$$%%@",
"$$$$$$$$$$$$$$$$$$$$$$&",
"$$$$$$$$$$$$$$$$$$$$$$*",
"$$$$$$$$$$$$$$$$$$$$$$*",
"%%$$$$$$$$$$$$%%%%%%%%&",
"%%%%%%%%%%%%%%========-",
"=%%%%%%%%%%%%%========#",
"======================@",
";;============;;;;;;;;$",
";;;;;;;;;;;;;;;;;;;;;;%",
">>;;;;;;;;;;;;>>>>>>>>%",
">>>>>>>>>>>>>>>>>>>>>>;",
">>>>>>>>>>>>>>>>>>>>>>;",
">>>>>>>>>>>>>>>>>>>>>>;",
">>>>>>>>>>>>>>>>>>>>>>;",
">>>>>>>>>>>>>>>>>>>>>>;",
",,,,,,,,,,,,,,+++++++,,"};
/* XPM */
static const char * const aqua_toolbtn_psh_right_xpm[] = {
"23 21 18 1",
"       c None",
".      c #636363",
"+      c #737373",
"@      c #9C9C9C",
"#      c #DEDEDE",
"$      c #BDBDBD",
"%      c #B5B5B5",
"&      c #ADADAD",
"*      c #7B7B7B",
"=      c #A5A5A5",
"-      c #C6C6C6",
";      c #CECECE",
">      c #6B6B6B",
",      c #848484",
"'      c #949494",
")      c #8C8C8C",
"!      c #D6D6D6",
"~      c #E7E7E7",
"....................+@#",
"$$$$$$%$$$$$$$$$$%&@*.=",
"--;;;;;;;;;;;-;;;--$=+>",
";;;;;;;;;;;;;;;;;;;-%,.",
";;;-----------------$'*",
"--------------------$=)",
"--------------------$&'",
"--------------------$&'",
";;-------------------%=",
";;;;;;;;;;;;;;;;;;;;;$=",
"!;;;;;;;;;;;;;;;;;;;;-&",
"!!!!!!!!!!!!!!!!!!;!!-&",
"##!!!!!!!!!!!!!!!!!!!;&",
"#####################;&",
"~~#################~#;%",
"~~~~~~~~~~~~~~~~~~~~#!%",
"~~~~~~~~~~~~~~~~~~~~~!@",
"~~~~~~~~~~~~~~~~~~~~~!*",
"~~~~~~~~~~~~~~~~~~~~~&+",
"~~~~~~~~~~~~~~~~~~~-,.&",
">>>>>>>>>>>>>>>>>>++)&-"};
/* XPM */
static const char * const aqua_toolbtn_psh_left_xpm[] = {
"23 21 18 1",
"       c None",
".      c #DEDEDE",
"+      c #9C9C9C",
"@      c #737373",
"#      c #636363",
"$      c #A5A5A5",
"%      c #7B7B7B",
"&      c #ADADAD",
"*      c #B5B5B5",
"=      c #BDBDBD",
"-      c #6B6B6B",
";      c #C6C6C6",
">      c #CECECE",
",      c #848484",
"'      c #949494",
")      c #8C8C8C",
"!      c #D6D6D6",
"~      c #E7E7E7",
".+@###################@",
"$#%+&*==========*======",
"-@$=;;>>>;>>>>>>>>>>>>>",
"#,*;>>>>>>>>>>>>>>>>>>;",
"%'=;>;;;;;;;;;;;;;;;>>=",
")$=;;;;;;;;;;;;;;;;;;;$",
"'&=;;;;;;;;;;;;;;;;;;;+",
"'&=;;;;;;;;;;;;;;;;;;;+",
"$*;;;>>>>>>>>>>>>>>>>>$",
"$=>>>>!!!!!!!!!!!!!!!!&",
"&;>!!!!!!!!!!!!!!!!!!!*",
"&;!!!!!!!!!!!!!!!!!!!!=",
"&>!!.!................;",
"&>....................>",
"*>.~~~~~~~~~~~~~~~~~~~>",
"*!.~~~~~~~~~~~~~~~~~~~.",
"+!~~~~~~~~~~~~~~~~~~~~.",
"%!~~~~~~~~~~~~~~~~~~~~.",
"@&~~~~~~~~~~~~~~~~~~~~.",
"&#,;~~~~~~~~~~~~~~~~~~.",
";&)@@@@@@@@@@@@@@@@@@--"};
/* XPM */
static const char * const aqua_toolbtn_act_left_xpm[] = {
"23 21 109 2",
"       c None",
".      c #DEDEDE",
"+      c #9C9CB5",
"@      c #52639C",
"#      c #184294",
"$      c #08429C",
"%      c #084AA5",
"&      c #0852A5",
"*      c #1052A5",
"=      c #1052AD",
"-      c #737373",
";      c #9C9CBD",
">      c #294294",
",      c #215AAD",
"'      c #528CC6",
")      c #84ADD6",
"!      c #94BDDE",
"~      c #9CBDE7",
"{      c #9CC6E7",
"]      c #BDBDBD",
"^      c #4A5A94",
"/      c #00429C",
"(      c #5A8CC6",
"_      c #ADCEE7",
":      c #B5CEE7",
"<      c #BDCEEF",
"[      c #BDD6EF",
"}      c #CECECE",
"|      c #103994",
"1      c #0852AD",
"2      c #7BA5DE",
"3      c #A5C6E7",
"4      c #B5CEEF",
"5      c #C6C6C6",
"6      c #00399C",
"7      c #2163BD",
"8      c #73A5DE",
"9      c #0042A5",
"0      c #216BBD",
"a      c #5A94D6",
"b      c #6BA5DE",
"c      c #7BADDE",
"d      c #84B5E7",
"e      c #84B5DE",
"f      c #A5A5A5",
"g      c #3173C6",
"h      c #4A8CD6",
"i      c #5A9CDE",
"j      c #73ADDE",
"k      c #73ADE7",
"l      c #9C9C9C",
"m      c #105AB5",
"n      c #397BCE",
"o      c #5294DE",
"p      c #639CDE",
"q      c #6BA5E7",
"r      c #7BB5EF",
"s      c #428CD6",
"t      c #639CE7",
"u      c #73ADEF",
"v      c #7BB5E7",
"w      c #84BDF7",
"x      c #ADADAD",
"y      c #296BC6",
"z      c #6BA5EF",
"A      c #84BDEF",
"B      c #8CBDF7",
"C      c #8CC6FF",
"D      c #B5B5B5",
"E      c #317BCE",
"F      c #5A9CE7",
"G      c #73B5F7",
"H      c #84BDFF",
"I      c #94CEFF",
"J      c #63A5EF",
"K      c #7BBDFF",
"L      c #9CCEFF",
"M      c #9CD6FF",
"N      c #3984D6",
"O      c #6BADF7",
"P      c #84C6FF",
"Q      c #A5DEFF",
"R      c #428CDE",
"S      c #6BADFF",
"T      c #A5D6FF",
"U      c #ADDEFF",
"V      c #73BDFF",
"W      c #8CCEFF",
"X      c #9CDEFF",
"Y      c #A5E7FF",
"Z      c #A5DEF7",
"`      c #B5E7FF",
" .     c #4A94D6",
"..     c #94D6FF",
"+.     c #ADE7FF",
"@.     c #B5EFFF",
"#.     c #5A84B5",
"$.     c #ADEFFF",
"%.     c #BDEFFF",
"&.     c #BDF7FF",
"*.     c #8494A5",
"=.     c #94DEFF",
"-.     c #B5F7FF",
";.     c #BDFFFF",
">.     c #7B8C9C",
",.     c #7BA5BD",
"'.     c #94CEEF",
").     c #8C8C8C",
"!.     c #6B6B6B",
". + @ # $ % & * = = = = * = = = = = = = = = - ",
"; > , ' ) ! ~ ~ ~ ~ ~ { { { { { { { { ~ { { ] ",
"^ / ( ~ _ : < [ [ [ [ [ [ [ [ [ [ [ [ [ [ [ } ",
"| 1 2 3 _ _ : 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 5 ",
"6 7 8 ! ~ ~ { ~ ~ ~ ~ 3 3 3 ~ ~ ~ ~ 3 3 3 3 ] ",
"9 0 a b 8 c c d d d d d d d d d d d d e d d f ",
"1 g h i b b b b b b b b b b b b b b b j k k l ",
"1 g h i b b b b b b b b b b b b b b b b k k l ",
"m n o p q k r r r r r r r r r r r r r r r r f ",
"0 s t u r r v v v v v v v v v v v v v w w w x ",
"y o z r w w A B B B B B B B B B B B B B C C D ",
"E F G H C C I I I I I I I I I I I I I I I I ] ",
"E J K C I L M M M M M M M M M M M M M M M M 5 ",
"N O P I M M Q Q Q Q Q Q Q Q Q Q Q Q Q Q Q Q } ",
"R S C M M T Q Q Q Q Q Q Q Q Q Q Q Q Q Q U U } ",
"R V W X Y Q Z ` ` ` ` ` ` ` ` ` ` ` ` ` ` ` . ",
" .V ..Y +.` ` @.@.@.@.@.@.@.@.@.@.@.@.@.@.@.. ",
"#.K ..+.$.@.@.@.@.@.@.%.&.%.@.@.@.@.%.&.&.&.. ",
"*.b =.+.-.-.&.;.&.&.&.&.&.&.&.&.&.&.&.;.&.&.. ",
"x >.,.'.$.&.&.&.&.&.&.&.&.&.&.&.&.&.&.&.&.&.. ",
"5 x ).- - !.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!.!."};
/* XPM */
static const char * const aqua_toolbtn_act_right_xpm[] = {
"23 21 110 2",
"       c None",
".      c #1052AD",
"+      c #1052A5",
"@      c #0852A5",
"#      c #084AA5",
"$      c #08429C",
"%      c #184294",
"&      c #52639C",
"*      c #9C9CB5",
"=      c #DEDEDE",
"-      c #9CC6E7",
";      c #9CBDE7",
">      c #94BDDE",
",      c #84ADD6",
"'      c #528CC6",
")      c #215AAD",
"!      c #294294",
"~      c #9C9CBD",
"{      c #BDD6EF",
"]      c #BDCEEF",
"^      c #B5CEE7",
"/      c #ADCEE7",
"(      c #5A8CC6",
"_      c #00429C",
":      c #4A5A94",
"<      c #B5CEEF",
"[      c #A5C6E7",
"}      c #7BA5DE",
"|      c #0852AD",
"1      c #103994",
"2      c #9CBDDE",
"3      c #94BDE7",
"4      c #73A5DE",
"5      c #2163BD",
"6      c #00399C",
"7      c #84ADE7",
"8      c #84B5E7",
"9      c #84B5DE",
"0      c #7BA5D6",
"a      c #6BA5DE",
"b      c #5A94D6",
"c      c #216BBD",
"d      c #0042A5",
"e      c #73ADE7",
"f      c #73ADDE",
"g      c #6BA5D6",
"h      c #639CD6",
"i      c #5A9CDE",
"j      c #4A8CD6",
"k      c #3173C6",
"l      c #6394C6",
"m      c #639CCE",
"n      c #7BB5EF",
"o      c #6B9CD6",
"p      c #639CDE",
"q      c #5294DE",
"r      c #397BCE",
"s      c #105AB5",
"t      c #84BDF7",
"u      c #84B5EF",
"v      c #73ADEF",
"w      c #639CE7",
"x      c #428CD6",
"y      c #8CC6FF",
"z      c #8CBDF7",
"A      c #7BADE7",
"B      c #6BA5EF",
"C      c #296BC6",
"D      c #94CEFF",
"E      c #84BDFF",
"F      c #73B5F7",
"G      c #5A9CE7",
"H      c #317BCE",
"I      c #9CD6FF",
"J      c #94C6FF",
"K      c #7BBDFF",
"L      c #63A5EF",
"M      c #A5DEFF",
"N      c #8CCEFF",
"O      c #84C6FF",
"P      c #6BADF7",
"Q      c #3984D6",
"R      c #ADDEFF",
"S      c #94D6FF",
"T      c #6BADFF",
"U      c #428CDE",
"V      c #B5E7FF",
"W      c #ADE7FF",
"X      c #9CDEFF",
"Y      c #73BDFF",
"Z      c #BDEFFF",
"`      c #B5EFFF",
" .     c #ADE7F7",
"..     c #A5E7FF",
"+.     c #4A94D6",
"@.     c #BDF7FF",
"#.     c #ADEFFF",
"$.     c #5A84B5",
"%.     c #BDFFFF",
"&.     c #B5F7FF",
"*.     c #94DEFF",
"=.     c #8494A5",
"-.     c #94CEEF",
";.     c #7BA5BD",
">.     c #7B8C9C",
",.     c #ADADAD",
"'.     c #6B6B6B",
").     c #737373",
"!.     c #8C8C8C",
"~.     c #C6C6C6",
". . . . . . . . . . + . . . . + @ # $ % & * = ",
"- - - ; - - - - - - - - ; ; ; ; ; > , ' ) ! ~ ",
"{ { { { { { { { { { { { { { { { ] ^ / ; ( _ : ",
"< < < < < < < < < < < < < < < < ^ / / [ } | 1 ",
"[ [ [ [ ; ; ; ; 2 ; ; ; 2 ; ; ; ; ; 3 > 4 5 6 ",
"7 8 9 4 4 4 4 4 4 4 4 4 4 4 4 4 4 0 4 a b c d ",
"e e f g h h h h h h h h h h h h h h h i j k | ",
"e e 4 h h h h h h h h h h h h l h m h i j k | ",
"n n 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 o a p q r s ",
"t t u u u u u u u u u u u u u u u u e v w x c ",
"y y z 8 A 8 u 8 A 8 u 8 A 8 u 8 A A n n B q C ",
"D D D D D D D D D D D D D D D D E E E E F G H ",
"I I I I I I I I I I I I I I I J J J J J K L H ",
"M M M M M M M M M M M M M M M M N N D N O P Q ",
"R R R R R R R R R R R R R R R R R S I S y T U ",
"V V W W W W W W W W W W W W W W W W M X N Y U ",
"Z `  . . . . . . . . . . . . . . . .W ..S Y +.",
"@.@.@.Z ` ` ` ` ` ` ` ` ` ` ` ` ` ` #.W S K $.",
"@.@.@.%.@.@.@.@.@.@.@.@.@.@.@.%.@.&.&.W *.a =.",
"@.@.@.@.@.@.@.@.@.@.@.@.@.@.@.@.@.@.#.-.;.>.,.",
"'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.'.).).!.,.~."};
/* XPM */
static const char * const aqua_tbar_hsep_xpm[] = {
"8 22 32 1",
"       c None",
".      c #A5A5A5",
"+      c #737373",
"@      c #DBDBDB",
"#      c #808080",
"$      c #FFFFFF",
"%      c #B9B9B9",
"&      c #EDEDED",
"*      c #CCCCCC",
"=      c #EFEFEF",
"-      c #C7C7C7",
";      c #BEBEBE",
">      c #E8E8E8",
",      c #AAAAAA",
"'      c #E7E7E7",
")      c #9E9E9E",
"!      c #9C9C9C",
"~      c #EBEBEB",
"{      c #A1A1A1",
"]      c #A9A9A9",
"^      c #F3F3F3",
"/      c #B1B1B1",
"(      c #F7F7F7",
"_      c #B8B8B8",
":      c #C0C0C0",
"<      c #FAFAFA",
"[      c #C9C9C9",
"}      c #CECECE",
"|      c #D2D2D2",
"1      c #DEDEDE",
"2      c #F8F8F8",
"3      c #D8D8D8",
".......+",
"@@@#$@@%",
"&&&#$&&*",
"===#$==-",
"===#$==;",
">>>#$>>,",
"'''#$'')",
"'''#$''!",
"~~~#$~~{",
"===#$==]",
"^^^#$^^/",
"(((#$((_",
"(((#$((:",
"<<<#$<<[",
"$$$#$$$}",
"$$$#$$$|",
"$$$#$$$1",
"$$$#$$$1",
"$$$#$$$1",
"$$$#$$$1",
"222#$223",
"++++++++"};
/* XPM */
static const char * const aqua_tbar_vsep_xpm[] = {
"22 8 32 1",
"       c None",
".      c #737373",
"+      c #F8F8F8",
"@      c #FFFFFF",
"#      c #FAFAFA",
"$      c #F7F7F7",
"%      c #F3F3F3",
"&      c #EFEFEF",
"*      c #EBEBEB",
"=      c #E7E7E7",
"-      c #E8E8E8",
";      c #EDEDED",
">      c #DBDBDB",
",      c #A5A5A5",
"'      c #808080",
")      c #D8D8D8",
"!      c #DEDEDE",
"~      c #D2D2D2",
"{      c #CECECE",
"]      c #C9C9C9",
"^      c #C0C0C0",
"/      c #B8B8B8",
"(      c #B1B1B1",
"_      c #A9A9A9",
":      c #A1A1A1",
"<      c #9C9C9C",
"[      c #9E9E9E",
"}      c #AAAAAA",
"|      c #BEBEBE",
"1      c #C7C7C7",
"2      c #CCCCCC",
"3      c #B9B9B9",
".+@@@@@@#$$%&*==-&&;>,",
".+@@@@@@#$$%&*==-&&;>,",
".+@@@@@@#$$%&*==-&&;>,",
".'''''''''''''''''''',",
".@@@@@@@@@@@@@@@@@@@@,",
".+@@@@@@#$$%&*==-&&;>,",
".+@@@@@@#$$%&*==-&&;>,",
".)!!!!~{]^/(_:<[}|123."};
/* XPM */
static const char * const aqua_tab_act_left_xpm[] = {
"8 25 117 2",
"       c None",
".      c #E7E7E7",
"+      c #DEDEDE",
"@      c #B5B5BD",
"#      c #6B6B94",
"$      c #31318C",
"%      c #08187B",
"&      c #00218C",
"*      c #9C9CAD",
"=      c #39428C",
"-      c #4A63B5",
";      c #6B94CE",
">      c #84ADD6",
",      c #94BDDE",
"'      c #9CBDDE",
")      c #C6C6CE",
"!      c #313984",
"~      c #4A73B5",
"{      c #8CB5DE",
"]      c #A5C6E7",
"^      c #ADC6E7",
"/      c #63638C",
"(      c #083194",
"_      c #528CC6",
":      c #21297B",
"<      c #0042A5",
"[      c #3173BD",
"}      c #6394CE",
"|      c #7BA5D6",
"1      c #8CADD6",
"2      c #94B5DE",
"3      c #002184",
"4      c #1052AD",
"5      c #4A84C6",
"6      c #6394C6",
"7      c #6B9CCE",
"8      c #739CCE",
"9      c #00399C",
"0      c #2163B5",
"a      c #4284C6",
"b      c #5A94C6",
"c      c #73A5D6",
"d      c #004AA5",
"e      c #739CD6",
"f      c #0852AD",
"g      c #397BBD",
"h      c #7BADDE",
"i      c #7BADD6",
"j      c #84ADDE",
"k      c #185AAD",
"l      c #639CCE",
"m      c #8CADDE",
"n      c #1863B5",
"o      c #4A84CE",
"p      c #6B9CD6",
"q      c #7BA5DE",
"r      c #216BBD",
"s      c #528CCE",
"t      c #94BDE7",
"u      c #2973C6",
"v      c #5A94D6",
"w      c #73A5DE",
"x      c #84B5DE",
"y      c #8CBDE7",
"z      c #3173C6",
"A      c #8CB5E7",
"B      c #9CC6E7",
"C      c #9CC6EF",
"D      c #397BCE",
"E      c #639CD6",
"F      c #7BADE7",
"G      c #94C6EF",
"H      c #A5C6EF",
"I      c #3984CE",
"J      c #6BA5DE",
"K      c #84B5E7",
"L      c #A5CEEF",
"M      c #428CD6",
"N      c #73ADE7",
"O      c #8CBDEF",
"P      c #9CC6F7",
"Q      c #A5CEF7",
"R      c #ADCEF7",
"S      c #428CDE",
"T      c #73ADEF",
"U      c #8CBDF7",
"V      c #9CCEF7",
"W      c #A5D6FF",
"X      c #ADD6FF",
"Y      c #4A94DE",
"Z      c #7BB5EF",
"`      c #94C6FF",
" .     c #A5CEFF",
"..     c #B5DEFF",
"+.     c #5294E7",
"@.     c #7BB5F7",
"#.     c #9CCEFF",
"$.     c #BDE7FF",
"%.     c #BDDEFF",
"&.     c #529CEF",
"*.     c #84BDFF",
"=.     c #C6E7FF",
"-.     c #52A5F7",
";.     c #84C6FF",
">.     c #B5E7FF",
",.     c #BDEFFF",
"'.     c #C6EFFF",
").     c #5AA5FF",
"!.     c #8CCEFF",
"~.     c #ADDEFF",
"{.     c #C6F7FF",
"].     c #CEF7FF",
"^.     c #ADE7FF",
"/.     c #CEFFFF",
"(.     c #D6FFFF",
"_.     c #9CBDE7",
":.     c #ADCEEF",
". + @ # $ % & & ",
". * = - ; > , ' ",
") ! ~ { ' ] ] ^ ",
"/ ( _ { ' ] ^ ^ ",
": < [ } | > 1 2 ",
"3 4 [ 5 6 ; 7 8 ",
"9 0 a b 7 8 c | ",
"d [ _ } e c | | ",
"f g b 7 h i > j ",
"k a l c | j j m ",
"n o p q j { { { ",
"r s c j { { t t ",
"u v w x y t t t ",
"z v h A t t B C ",
"D E F t G C C H ",
"I J K G C H L L ",
"M N O P Q Q R R ",
"S T U V W X X X ",
"Y Z `  .X ......",
"+.@.#.X ..$.%.$.",
"&.*.#...$.$.$.=.",
"-.;.W >.,.'.'.'.",
").!.~.,.{.].].].",
").!.^.{././.(./.",
"] _.G H L :.L :."};
/* XPM */
static const char * const aqua_tab_act_mid_xpm[] = {
"4 25 39 1",
"       c None",
".      c #00218C",
"+      c #00298C",
"@      c #002994",
"#      c #94BDDE",
"$      c #9CBDDE",
"%      c #A5C6E7",
"&      c #ADC6E7",
"*      c #ADCEE7",
"=      c #8CADD6",
"-      c #94B5DE",
";      c #6B9CCE",
">      c #739CCE",
",      c #73A5D6",
"'      c #7BA5D6",
")      c #7BADD6",
"!      c #84ADD6",
"~      c #84ADDE",
"{      c #8CADDE",
"]      c #8CB5DE",
"^      c #94BDE7",
"/      c #9CC6E7",
"(      c #9CC6EF",
"_      c #A5C6EF",
":      c #A5CEEF",
"<      c #A5CEF7",
"[      c #ADCEF7",
"}      c #ADD6FF",
"|      c #B5D6FF",
"1      c #B5DEFF",
"2      c #BDDEFF",
"3      c #BDE7FF",
"4      c #C6E7FF",
"5      c #C6EFFF",
"6      c #C6F7FF",
"7      c #CEF7FF",
"8      c #CEFFFF",
"9      c #D6FFFF",
"0      c #ADCEEF",
"..+@",
"#$$$",
"%&&&",
"&&**",
"=---",
";>>>",
",'''",
"''))",
"!~~!",
"~{{~",
"]]]]",
"^^-#",
"^^^^",
"/(^/",
"(_(_",
":::<",
"[[[[",
"}}|}",
"1111",
"2332",
"3453",
"5556",
"7788",
"9889",
":000"};
/* XPM */
static const char * const aqua_tab_act_right_xpm[] = {
"8 25 118 2",
"       c None",
".      c #00218C",
"+      c #08187B",
"@      c #31318C",
"#      c #6B6B94",
"$      c #B5B5BD",
"%      c #DEDEDE",
"&      c #E7E7E7",
"*      c #9CBDDE",
"=      c #94BDDE",
"-      c #8CADD6",
";      c #638CC6",
">      c #294AA5",
",      c #313984",
"'      c #9C9CAD",
")      c #ADC6E7",
"!      c #A5C6E7",
"~      c #7BA5D6",
"{      c #315AAD",
"]      c #313184",
"^      c #C6C6CE",
"/      c #ADCEE7",
"(      c #94B5DE",
"_      c #5A8CC6",
":      c #002994",
"<      c #63638C",
"[      c #8CB5DE",
"}      c #84ADD6",
"|      c #6394CE",
"1      c #3973BD",
"2      c #00429C",
"3      c #21297B",
"4      c #739CCE",
"5      c #6B9CCE",
"6      c #6B94CE",
"7      c #6394C6",
"8      c #4A84C6",
"9      c #3173BD",
"0      c #1052AD",
"a      c #002184",
"b      c #739CD6",
"c      c #5A94C6",
"d      c #4284C6",
"e      c #2163B5",
"f      c #00399C",
"g      c #7BADD6",
"h      c #528CC6",
"i      c #004AA5",
"j      c #84ADDE",
"k      c #397BC6",
"l      c #0852AD",
"m      c #73A5D6",
"n      c #639CCE",
"o      c #105AB5",
"p      c #6B9CD6",
"q      c #216BBD",
"r      c #94BDE7",
"s      c #7BADDE",
"t      c #6BA5D6",
"u      c #528CCE",
"v      c #84B5DE",
"w      c #73A5DE",
"x      c #5A94CE",
"y      c #2973BD",
"z      c #9CC6E7",
"A      c #9CBDE7",
"B      c #8CBDE7",
"C      c #5A94D6",
"D      c #317BC6",
"E      c #9CC6EF",
"F      c #A5C6EF",
"G      c #94C6EF",
"H      c #84B5E7",
"I      c #639CDE",
"J      c #317BCE",
"K      c #A5CEF7",
"L      c #A5CEEF",
"M      c #6BA5DE",
"N      c #3984CE",
"O      c #ADCEF7",
"P      c #9CC6F7",
"Q      c #8CBDEF",
"R      c #73A5E7",
"S      c #428CD6",
"T      c #ADD6FF",
"U      c #A5D6FF",
"V      c #A5CEFF",
"W      c #8CBDF7",
"X      c #73ADE7",
"Y      c #4A8CDE",
"Z      c #B5DEFF",
"`      c #ADDEFF",
" .     c #94C6FF",
"..     c #73ADEF",
"+.     c #4A94DE",
"@.     c #BDDEFF",
"#.     c #94CEFF",
"$.     c #7BB5F7",
"%.     c #529CE7",
"&.     c #C6EFFF",
"*.     c #BDE7FF",
"=.     c #B5E7FF",
"-.     c #9CCEFF",
";.     c #84BDFF",
">.     c #529CEF",
",.     c #A5DEFF",
"'.     c #8CC6FF",
").     c #52A5F7",
"!.     c #CEFFFF",
"~.     c #CEF7FF",
"{.     c #C6F7FF",
"].     c #BDEFFF",
"^.     c #ADE7FF",
"/.     c #8CCEFF",
"(.     c #5AA5FF",
"_.     c #D6FFFF",
":.     c #BDF7FF",
"<.     c #ADCEEF",
". . + @ # $ % & ",
"* = - ; > , ' & ",
") ! ! * ~ { ] ^ ",
"/ ) ! * ( _ : < ",
"( [ - } | 1 2 3 ",
"4 5 6 7 8 9 0 a ",
"~ 4 b 5 c d e f ",
"g ~ ~ b 5 h 9 i ",
"j } ~ ~ b c k l ",
"j j j ~ m n d o ",
"[ [ [ j ~ p 8 q ",
"r r [ [ s t u q ",
"r r r [ v w x y ",
"z r A r B s C D ",
"E F E G B H I J ",
"K K L E G H M N ",
"O O K K P Q R S ",
"T T T U V W X Y ",
"Z Z ` ` U  ...+.",
"@.@.Z Z T #.$.%.",
"&.*.*.=.` -.;.>.",
"&.&.&.&.*.,.'.).",
"!.!.~.{.].^./.(.",
"!._.!.!.:.^./.(.",
"<.<.<.F F G r ! "};
/* XPM */
static const char * const aqua_tab_dis_left_xpm[] = {
"8 25 24 1",
"       c None",
".      c #E7E7E7",
"+      c #DEDEDE",
"@      c #B5B5B5",
"#      c #8C8C8C",
"$      c #737373",
"%      c #6B6B6B",
"&      c #EFEFEF",
"*      c #A5A5A5",
"=      c #7B7B7B",
"-      c #BDBDBD",
";      c #CECECE",
">      c #D6D6D6",
",      c #C6C6C6",
"'      c #ADADAD",
")      c #848484",
"!      c #9C9C9C",
"~      c #636363",
"{      c #949494",
"]      c #F7F7F7",
"^      c #FFFFFF",
"/      c #0852AD",
"(      c #0852B5",
"_      c #A5C6E7",
"..+@#$%$",
"&.*=*-;>",
"&,$'>+++",
"-))->++.",
"!~#',;>>",
"#~!'-,,,",
"#=*@,,;;",
"$)'-,;;;",
"%#@-;;;>",
"${@,;;>>",
"={-;>>>>",
"$!,;>>>+",
"%*,>>+++",
"$*;>++++",
"=';>+...",
"$';+....",
"%@>..&&&",
"$@+.&&&]",
"=-+&&]]]",
"%,.&]^^^",
"%,&]^^^^",
"%;&^^^^^",
"%>]^^^^^",
"/((/////",
"________"};
/* XPM */
static const char * const aqua_tab_dis_mid_xpm[] = {
"4 25 13 1",
"       c None",
".      c #7B7B7B",
"+      c #737373",
"@      c #DEDEDE",
"#      c #E7E7E7",
"$      c #D6D6D6",
"%      c #CECECE",
"&      c #C6C6C6",
"*      c #EFEFEF",
"=      c #F7F7F7",
"-      c #FFFFFF",
";      c #0852AD",
">      c #A5C6E7",
"..++",
"@@@@",
"#@##",
"####",
"$$$$",
"%&&&",
"%%%%",
"%%%%",
"$$$$",
"$$$$",
"$$$$",
"@@@@",
"@@@@",
"@@##",
"####",
"#*##",
"****",
"*===",
"====",
"----",
"----",
"----",
"----",
";;;;",
">>>>"};
/* XPM */
static const char * const aqua_tab_dis_right_xpm[] = {
"8 25 23 1",
"       c None",
".      c #737373",
"+      c #6B6B6B",
"@      c #8C8C8C",
"#      c #B5B5B5",
"$      c #DEDEDE",
"%      c #E7E7E7",
"&      c #D6D6D6",
"*      c #BDBDBD",
"=      c #949494",
"-      c #A5A5A5",
";      c #EFEFEF",
">      c #CECECE",
",      c #C6C6C6",
"'      c #7B7B7B",
")      c #848484",
"!      c #9C9C9C",
"~      c #ADADAD",
"{      c #F7F7F7",
"]      c #FFFFFF",
"^      c #0852AD",
"/      c #0852B5",
"(      c #A5C6E7",
".+.@#$%%",
"&&*=.-%;",
"$$$>-.,;",
"$$$&*')*",
"&&>,#@+!",
",,,*~!.@",
",>,,#-@@",
">>>,*~=.",
"&>>>*#!+",
"&&>>,*-.",
"&&&&>*-'",
"$&&&>*~.",
"$$$&&,~+",
"$$$$&,#.",
"%%%$$>*'",
"%%%%$&*.",
";;;%%&,+",
";{;;%$,.",
"{{{;;$>'",
"]{]{;%&+",
"]]]]{%&+",
"]]]]]{$+",
"]]]]]]%+",
"^^//^^^^",
"(((((((("};
/* XPM */
static const char * const aqua_tab_sel_dis_left_xpm[] = {
"8 25 26 1",
"       c None",
".      c #EFEFEF",
"+      c #E7E7E7",
"@      c #BDBDBD",
"#      c #7B7B7B",
"$      c #4A4A4A",
"%      c #393939",
"&      c #424242",
"*      c #DEDEDE",
"=      c #949494",
"-      c #5A5A5A",
";      c #A5A5A5",
">      c #C6C6C6",
",      c #B5B5B5",
"'      c #8C8C8C",
")      c #CECECE",
"!      c #737373",
"~      c #525252",
"{      c #9C9C9C",
"]      c #D6D6D6",
"^      c #636363",
"/      c #848484",
"(      c #ADADAD",
"_      c #6B6B6B",
":      c #F7F7F7",
"<      c #FFFFFF",
"..+@#$%&",
"+*=-#;@>",
"*,$'@>))",
")!~{>))]",
";&^';,@@",
"/&!'{;((",
"#-/=;((,",
"#_'{(,,,",
"!!=;(,@@",
"_#{(,@@@",
"!#{(@>>>",
"#/;,@>>>",
"!'(@>>))",
"_=(@>)))",
"!{,@>)]]",
"#{,>)]]]",
"!;@)]***",
"_;>]**++",
"!(>]*+++",
"!,)*+...",
"_,]+.:::",
"^@*.:<<<",
"^>+:<<<<",
"')+<<<<<",
"**]**+++"};
/* XPM */
static const char * const aqua_tab_sel_dis_mid_xpm[] = {
"4 25 14 1",
"       c None",
".      c #424242",
"+      c #4A4A4A",
"@      c #C6C6C6",
"#      c #CECECE",
"$      c #D6D6D6",
"%      c #ADADAD",
"&      c #B5B5B5",
"*      c #BDBDBD",
"=      c #DEDEDE",
"-      c #E7E7E7",
";      c #EFEFEF",
">      c #F7F7F7",
",      c #FFFFFF",
".+++",
"@###",
"#$$$",
"$$$$",
"@@@@",
"%%%%",
"&&&&",
"****",
"****",
"**@*",
"@@@@",
"@@@@",
"####",
"####",
"$$$$",
"$$$$",
"====",
"----",
"--;-",
">>;;",
">,>>",
",,,,",
",,,,",
",,,,",
"----"};
/* XPM */
static const char * const aqua_tab_sel_dis_right_xpm[] = {
"8 25 26 1",
"       c None",
".      c #393939",
"+      c #4A4A4A",
"@      c #7B7B7B",
"#      c #BDBDBD",
"$      c #E7E7E7",
"%      c #EFEFEF",
"&      c #C6C6C6",
"*      c #9C9C9C",
"=      c #636363",
"-      c #949494",
";      c #DEDEDE",
">      c #CECECE",
",      c #B5B5B5",
"'      c #737373",
")      c #424242",
"!      c #D6D6D6",
"~      c #ADADAD",
"{      c #8C8C8C",
"]      c #A5A5A5",
"^      c #848484",
"/      c #525252",
"(      c #5A5A5A",
"_      c #6B6B6B",
":      c #F7F7F7",
"<      c #FFFFFF",
"..+@#$%%",
"&#*=+-;$",
">>&,'),;",
"!>>&*+'>",
"###~{=.]",
"~~]*{'.^",
",~~]-^/@",
",,,~*{(@",
"##,~]-_'",
"###,~*'_",
"&&##~*''",
">&&#,]'@",
">>&&,~@'",
">>>&#~^_",
"!!>&&,^'",
";!!>&#{@",
";;;!>#-'",
"$$;;!&*_",
"%$$;!&*'",
"%%%$;>]'",
"::%%$!]_",
"<<<:%;~=",
"<<<<:$,=",
"<<<<<$>{",
"$$;;;!;;"};
/* XPM */
static const char * const aqua_tab_top_act_xpm[] = {
"32 8 11 1",
"       c None",
".      c #0852AD",
"+      c #A5C6E7",
"@      c #B5D6EF",
"#      c #BDD6EF",
"$      c #7BADEF",
"%      c #9CCEFF",
"&      c #BDEFFF",
"*      c #BDF7FF",
"=      c #6B6B6B",
"-      c #B5B5B5",
"................................",
"++++++++++++++++++++++++++++++++",
"@#@#@#@#@#@#@#@#@#@#@#@#@#@#@#@#",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%",
"&&*&*&&&*&&&*&&&*&&&*&&&*&&&&&*&",
"================================",
"--------------------------------"};
/* XPM */
static const char * const aqua_tab_top_dis_xpm[] = {
"32 8 9 1",
"       c None",
".      c #8C8C8C",
"+      c #DEDEDE",
"@      c #E7E7E7",
"#      c #D6D6D6",
"$      c #EFEFEF",
"%      c #FFFFFF",
"&      c #6B6B6B",
"*      c #B5B5B5",
"................................",
"++++++++++++++++++++++++++++++++",
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@",
"################################",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%",
"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&",
"********************************"};
/* XPM */
static const char * const aqua_tab_usel_dis_left_xpm[] = {
"8 25 21 1",
"       c None",
".      c #EFEFEF",
"+      c #E7E7E7",
"@      c #BDBDBD",
"#      c #949494",
"$      c #737373",
"%      c #6B6B6B",
"&      c #DEDEDE",
"*      c #9C9C9C",
"=      c #7B7B7B",
"-      c #A5A5A5",
";      c #CECECE",
">      c #D6D6D6",
",      c #B5B5B5",
"'      c #ADADAD",
")      c #8C8C8C",
"!      c #848484",
"~      c #C6C6C6",
"{      c #F7F7F7",
"]      c #FFFFFF",
"^      c #636363",
"..+@#$%$",
"+&*=-@;>",
"&,%'>&&&",
";)!@>&&+",
"-%)'~;>>",
"!%*'@~~~",
"=)-,~~;;",
"=#'@~;;;",
"$*,@;;;>",
"%-,~;;>>",
"$-@;>>>>",
"='~;>>>&",
"$,~>>&&&",
"%,;>&&&&",
"$@;>&+++",
"=@;&++++",
"$~>++...",
"%~&+...{",
"$;&..{{{",
"$>+.{]]]",
"%>.{]]]]",
"^&.]]]]]",
"^+{]]]]]",
"))))))))",
"&&&&&&&&"};
/* XPM */
static const char * const aqua_tab_usel_dis_mid_xpm[] = {
"4 25 12 1",
"       c None",
".      c #737373",
"+      c #7B7B7B",
"@      c #DEDEDE",
"#      c #E7E7E7",
"$      c #D6D6D6",
"%      c #C6C6C6",
"&      c #CECECE",
"*      c #EFEFEF",
"=      c #F7F7F7",
"-      c #FFFFFF",
";      c #8C8C8C",
"..++",
"@@@@",
"@#@#",
"@###",
"$$$$",
"%%%&",
"&&&&",
"&&&&",
"$$$$",
"$$$$",
"$$$$",
"@@@@",
"@@@@",
"#@@@",
"####",
"##*#",
"****",
"*==*",
"====",
"----",
"----",
"----",
"----",
";;;;",
"@@@@"};
/* XPM */
static char * aqua_tab_usel_dis_right_xpm[] = {
"8 25 21 1",
"       c None",
".      c #737373",
"+      c #6B6B6B",
"@      c #949494",
"#      c #BDBDBD",
"$      c #E7E7E7",
"%      c #EFEFEF",
"&      c #D6D6D6",
"*      c #9C9C9C",
"=      c #DEDEDE",
"-      c #CECECE",
";      c #A5A5A5",
">      c #B5B5B5",
",      c #7B7B7B",
"'      c #8C8C8C",
")      c #C6C6C6",
"!      c #636363",
"~      c #ADADAD",
"{      c #848484",
"]      c #F7F7F7",
"^      c #FFFFFF",
".+.@#$%%",
"&&#@.*=$",
"===-;+>=",
"===&#,'-",
"&&-)>'!;",
")))#~*+{",
")-))>;,,",
"---)#~{,",
"&---#>'.",
"&&--)#@+",
"&&&&-#@.",
"=&&&-#*,",
"===&&)*.",
"====&);+",
"$$$==-~.",
"$$$$=&~,",
"%%%$$&>.",
"%]%%$=>+",
"]]]%%=#.",
"^]^]%$).",
"^^^^]$)+",
"^^^^^]-!",
"^^^^^^&!",
"''''''''",
"========"};

#include "qapplication.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qpixmap.h"
#include "qpalette.h"
#include "qpushbutton.h"
#include "qtoolbutton.h"
#include "qdockwindow.h"
#include "qtabbar.h"
#include "qscrollbar.h"
#include "qimage.h"
#include "qbitmap.h"
#include "qpixmapcache.h"
#include "qtoolbutton.h"
#include "qtoolbar.h"
#include "qobjcoll.h"
#include "qlayout.h"
#include "qbuttongroup.h"
#include "qtabwidget.h"
#include <limits.h>
#define INCLUDE_MENUITEM_DEF
#include "qpopupmenu.h"

class QAquaStylePrivate
{
public:
    struct buttonState {
    public:
        buttonState() : frame(0), dir(1) {}
        int frame;
        int dir;
    };

    QPushButton * defaultButton;
    buttonState   buttonState;
    int timerId;
};

// NOT REVISED
/*!
  \class QAquaStyle qaquastyle.h
  \brief Aqua Look and Feel
  \ingroup appearance

  This class implements the Aqua look and feel. It's an
  experimental class that tries to resemble a Macinosh-like GUI style
  with the QStyle system. The emulation is, however, far from being
  perfect yet.
*/

static int qAquaGetNum( const QString & s )
{
    int num = 0;
    int i = s.findRev( '_' );

    if( i != -1 ){
        num = s.right( (s.length() - 1) - i ).toInt();
    }

    return num;
}


/*
  Hackish method of finding out whether the window is active or not
 */
static bool qAquaActive( const QColorGroup & g )
{
    if( g.buttonText() == QColor( 148,148,148 ) )
        return FALSE;
    else
        return TRUE;
}

/*
  Handle scaling and caching of pixmaps used in the Aqua style.
  Pixmaps are removed from the cache if the cache is idle for a
  certain amount of time

  The pixmaps are inserted into the pixmap cache with the following
  key prefix:   $qt_aqua_

 */

static void qAquaPixmap( const QString & s, QPixmap & p )
{
    int i, f, size = 0;

    QString str = "$qt_aqua_" + s;

    if( QPixmapCache::find( str, p ) )
        return;

    // The pixmap was not found in the cache - create/scale/insert it into
    // the cache

    if( s.contains("gen_back") ) {
	// Double the size of the background pixmap to decrease the
	// number of blits necessary to fill large areas.
	QPixmap tmp( (const char **) aqua_gen_back_xpm );
	QPixmap gen_back( tmp.width()*2, tmp.height()*2 );
	QPainter p( &gen_back );
	p.drawTiledPixmap( gen_back.rect(), tmp );
        QPixmapCache::insert( "$qt_aqua_gen_back", gen_back );
    }
    
    if( s.contains("sel_back" ) )
        QPixmapCache::insert( "$qt_aqua_sel_back",
                              QPixmap( (const char **) aqua_sel_back_xpm ) );
    QPixmap px;
    QImage  im;
    QBitmap left_mask( aqua_btn_mask_left_xbm_width, aqua_btn_mask_left_xbm_height,
                       (const uchar *) aqua_btn_mask_left_xbm_bits, TRUE );
    QBitmap right_mask( aqua_btn_mask_right_xbm_width, aqua_btn_mask_right_xbm_height,
                        (const uchar *) aqua_btn_mask_right_xbm_bits, TRUE );

    // Scale the button pixmaps to cover height of area to paint
    // Pixmaps for default buttons
    if( s.contains("btn_def_" ) ){
        const char ** bytes = 0;
        size = qAquaGetNum( s );
        for(f = 0; f < 10; f++ ){
            switch( f ){
                case 0: bytes = (const char **) aqua_btn_def_left_xpm; break;
                case 1: bytes = (const char **) aqua_btn_def_left1_xpm; break;
                case 2: bytes = (const char **) aqua_btn_def_left2_xpm; break;
                case 3: bytes = (const char **) aqua_btn_def_left3_xpm; break;
                case 4: bytes = (const char **) aqua_btn_def_left4_xpm; break;
                case 5: bytes = (const char **) aqua_btn_def_left5_xpm; break;
                case 6: bytes = (const char **) aqua_btn_def_left6_xpm; break;
                case 7: bytes = (const char **) aqua_btn_def_left7_xpm; break;
                case 8: bytes = (const char **) aqua_btn_def_left8_xpm; break;
                case 9: bytes = (const char **) aqua_btn_def_left9_xpm; break;
            }
            QPixmap p( bytes );
            p.setMask( left_mask );
            im = p;
            px = im.smoothScale( im.width(), size );
            QPixmapCache::insert( "$qt_aqua_btn_def_left" +
                                  ((f > 0) ? QString::number( f ) :
                                   QString("")) + QString("_") +
                                   QString::number( size ), px );
        }

        for(f = 0; f < 10; f++ ){
            switch( f ){
                case 0: bytes = (const char **) aqua_btn_def_mid_xpm; break;
                case 1: bytes = (const char **) aqua_btn_def_mid1_xpm; break;
                case 2: bytes = (const char **) aqua_btn_def_mid2_xpm; break;
                case 3: bytes = (const char **) aqua_btn_def_mid3_xpm; break;
                case 4: bytes = (const char **) aqua_btn_def_mid4_xpm; break;
                case 5: bytes = (const char **) aqua_btn_def_mid5_xpm; break;
                case 6: bytes = (const char **) aqua_btn_def_mid6_xpm; break;
                case 7: bytes = (const char **) aqua_btn_def_mid7_xpm; break;
                case 8: bytes = (const char **) aqua_btn_def_mid8_xpm; break;
                case 9: bytes = (const char **) aqua_btn_def_mid9_xpm; break;
            }
            im = QImage( bytes );
            px = im.smoothScale( im.width(), size );
            QPixmapCache::insert( "$qt_aqua_btn_def_mid" +
                                  ((f > 0) ? QString::number( f ) :
                                   QString("")) + QString("_") +
                                   QString::number( size ), px );
        }

        for(f = 0; f < 10; f++ ){
            switch( f ){
                case 0: bytes = (const char **) aqua_btn_def_right_xpm; break;
                case 1: bytes = (const char **) aqua_btn_def_right1_xpm; break;
                case 2: bytes = (const char **) aqua_btn_def_right2_xpm; break;
                case 3: bytes = (const char **) aqua_btn_def_right3_xpm; break;
                case 4: bytes = (const char **) aqua_btn_def_right4_xpm; break;
                case 5: bytes = (const char **) aqua_btn_def_right5_xpm; break;
                case 6: bytes = (const char **) aqua_btn_def_right6_xpm; break;
                case 7: bytes = (const char **) aqua_btn_def_right7_xpm; break;
                case 8: bytes = (const char **) aqua_btn_def_right8_xpm; break;
                case 9: bytes = (const char **) aqua_btn_def_right9_xpm; break;
            }
            QPixmap p( bytes );
            p.setMask( right_mask );
            im = p;
            px = im.smoothScale( im.width(), size );
            QPixmapCache::insert( "$qt_aqua_btn_def_right" +
                                  ((f > 0) ? QString::number( f ) :
                                   QString("")) + QString("_") +
                                   QString::number( size ), px );
        }
    }
    // Pixmaps for normal buttons
    if( s.contains("btn_nrm_" ) ){
        size = qAquaGetNum( s );
        QString sizestr = QString::number( size );

        QPixmap left( (const char **) aqua_btn_nrm_left_xpm );
        left.setMask( left_mask );
        im = left;
        px = im.smoothScale( im.width(), size );
        QPixmapCache::insert( "$qt_aqua_btn_nrm_left_" + sizestr, px );

        im = QImage( (const char **) aqua_btn_nrm_mid_xpm );
        px = im.smoothScale( im.width(), size );
        QPixmapCache::insert( "$qt_aqua_btn_nrm_mid_" + sizestr, px );

        QPixmap right( (const char **) aqua_btn_nrm_right_xpm );
        right.setMask( right_mask );
        im = right;
        px = im.smoothScale( im.width(), size );
        QPixmapCache::insert( "$qt_aqua_btn_nrm_right_"+ sizestr, px );
    }

    // Pixmaps for disabled buttons
    if( s.contains("btn_dis_" ) ){
        size = qAquaGetNum( s );
        QString sizestr = QString::number( size );

        QPixmap left( (const char **) aqua_btn_dis_left_xpm );
        left.setMask( left_mask );
        im = left;
        px = im.smoothScale( im.width(), size );
        QPixmapCache::insert( "$qt_aqua_btn_dis_left_" + sizestr, px );

        im = QImage( (const char **) aqua_btn_dis_mid_xpm );
        px = im.smoothScale( im.width(), size );
        QPixmapCache::insert( "$qt_aqua_btn_dis_mid_" + sizestr, px );

        QPixmap right( (const char **) aqua_btn_dis_right_xpm );
        right.setMask( right_mask );
        im = right;
        px = im.smoothScale( im.width(), size );
        QPixmapCache::insert( "$qt_aqua_btn_dis_right_" + sizestr, px );
    }

    // Radio buttons
    if( s.contains("radio_") ){
        QPixmapCache::insert( "$qt_aqua_radio_f", (const char **) aqua_radio_f_xpm );
        QPixmapCache::insert( "$qt_aqua_radio_t", (const char **) aqua_radio_t_xpm );
        QPixmapCache::insert( "$qt_aqua_radio_dis_t",
                              (const char **) aqua_radio_dis_t_xpm );
        QPixmapCache::insert( "$qt_aqua_radio_psh_t",
                              (const char **) aqua_radio_psh_t_xpm );
        QPixmapCache::insert( "$qt_aqua_radio_psh_f",
                              (const char **) aqua_radio_psh_f_xpm );
    }

    // Small bevel buttons
    if( s.contains("bvlbtn_") ){
        QPixmap off( (const char **) aqua_bvlbtn_small_off_xpm );
        QPixmap on( (const char **) aqua_bvlbtn_small_on_xpm );
        QPixmap dis( (const char **) aqua_bvlbtn_small_dis_xpm );

        int width = 0, height = 0, j;
        i = s.findRev( '_' );
        if( i != -1 ){
            height = s.right( (s.length() - 1) - i ).toInt();
        }
        j = s.findRev( '_', i - 1 );
        if( j != -1 ){
            width = s.mid( j + 1, i - j - 1 ).toInt();
        }
        QString wstr = QString::number( width );
        QString hstr = QString::number( height );

        im = off;
        px = im.smoothScale( width, height );
        QPixmapCache::insert( "$qt_aqua_bvlbtn_small_off_" + wstr + "_" + hstr, px );
        im = on;
        px = im.smoothScale( width, height );
        QPixmapCache::insert( "$qt_aqua_bvlbtn_small_on_" + wstr + "_" + hstr, px );
        im = dis;
        px = im.smoothScale( width, height );
        QPixmapCache::insert( "$qt_aqua_bvlbtn_small_dis_" + wstr + "_" + hstr, px );
    }

    // Checkboxes
    if( s.contains("chk_") ){
        QPixmapCache::insert( "$qt_aqua_chk_act_f",
                              (const char **) aqua_chk_act_f_xpm );
        QPixmapCache::insert( "$qt_aqua_chk_act_t",
                              (const char **) aqua_chk_act_t_xpm );
        QPixmapCache::insert( "$qt_aqua_chk_dis_t",
                              (const char **) aqua_chk_dis_t_xpm );
        QPixmapCache::insert( "$qt_aqua_chk_psh_t",
                              (const char **) aqua_chk_psh_t_xpm );
        QPixmapCache::insert( "$qt_aqua_chk_psh_f",
                              (const char **) aqua_chk_psh_f_xpm );
    }

    // Horizontal scrollbar
    if( s.contains("hsbr_") ){
        QBitmap left_mask( aqua_hsbr_tip_left_mask_width,
                           aqua_hsbr_tip_left_mask_height,
                         (const uchar *) aqua_hsbr_tip_left_mask_bits, TRUE );

        QBitmap right_mask( aqua_hsbr_tip_right_mask_width,
                            aqua_hsbr_tip_right_mask_height,
                         (const uchar *) aqua_hsbr_tip_right_mask_bits, TRUE );

        QPixmap a_left( (const char **) aqua_hsbr_arw_left_xpm );
        QPixmap a_right( (const char **) aqua_hsbr_arw_right_xpm );
        QPixmap t_right( (const char **) aqua_hsbr_tip_right_xpm );
        t_right.setMask( right_mask );
        QPixmap t_left( (const char **) aqua_hsbr_tip_left_xpm );
        t_left.setMask( left_mask );
        QPixmap fill( (const char **) aqua_hsbr_fill_xpm );
        QPixmap t_right_dis( (const char **) aqua_hsbr_tip_dis_right_xpm );
        t_right_dis.setMask( right_mask );
        QPixmap t_left_dis( (const char **) aqua_hsbr_tip_dis_left_xpm );
        t_left_dis.setMask( left_mask );
        QPixmap fill_dis( (const char **) aqua_hsbr_dis_fill_xpm );
        QPixmap back_fill( (const char **) aqua_hsbr_back_fill_xpm );

        size = qAquaGetNum( s );
        QString sizestr = QString::number( size );

        im = a_left;
        px = im.smoothScale( im.width(), size );
        QPixmapCache::insert( "$qt_aqua_hsbr_arw_left_" + sizestr, px );
        im = a_right;
        px = im.smoothScale( im.width(), size );
        QPixmapCache::insert( "$qt_aqua_hsbr_arw_right_" + sizestr, px );

        im = t_left;
        px = im.smoothScale( im.width(), size );
        QPixmapCache::insert( "$qt_aqua_hsbr_tip_left_" + sizestr, px );
        im = t_right;
        px = im.smoothScale( im.width(), size );
        QPixmapCache::insert( "$qt_aqua_hsbr_tip_right_" + sizestr, px );

        im = fill;
        px = im.smoothScale( im.width(), size );
        QPixmapCache::insert( "$qt_aqua_hsbr_fill_" + sizestr, px );
        im = t_left_dis;
        px = im.smoothScale( im.width(), size );
        QPixmapCache::insert( "$qt_aqua_hsbr_tip_dis_left_" + sizestr, px );
        im = t_right_dis;
        px = im.smoothScale( im.width(), size );
        QPixmapCache::insert( "$qt_aqua_hsbr_tip_dis_right_" + sizestr, px );

        im = fill_dis;
        px = im.smoothScale( im.width(), size );
        QPixmapCache::insert( "$qt_aqua_hsbr_dis_fill_" + sizestr, px );
        im = back_fill;
        px = im.smoothScale( im.width(), size );
        QPixmapCache::insert( "$qt_aqua_hsbr_back_fill_" + sizestr, px );
    }

    // Vertical scrollbar
    if( s.contains("vsbr_") ){
        QBitmap up_mask( aqua_vsbr_tip_up_mask_width, aqua_vsbr_tip_up_mask_height,
                         (const uchar *) aqua_vsbr_tip_up_mask_bits, TRUE );

        QBitmap down_mask( aqua_vsbr_tip_down_mask_width,
                           aqua_vsbr_tip_down_mask_height,
                          (const uchar *) aqua_vsbr_tip_down_mask_bits, TRUE );


        QPixmap a_up( (const char **) aqua_vsbr_arw_up_xpm );
        QPixmap a_down( (const char **) aqua_vsbr_arw_down_xpm );
        QPixmap t_down( (const char **) aqua_vsbr_tip_down_xpm );
        t_down.setMask( down_mask );
        QPixmap t_up( (const char **) aqua_vsbr_tip_up_xpm );
        t_up.setMask( up_mask );
        QPixmap fill( (const char **) aqua_vsbr_fill_xpm );
        QPixmap t_down_dis( (const char **) aqua_vsbr_tip_dis_down_xpm );
        t_down_dis.setMask( down_mask );
        QPixmap t_up_dis( (const char **) aqua_vsbr_tip_dis_up_xpm );
        t_up_dis.setMask( up_mask );
        QPixmap fill_dis( (const char **) aqua_vsbr_dis_fill_xpm );
        QPixmap back_fill( (const char **) aqua_vsbr_back_fill_xpm );

        size = qAquaGetNum( s );
        QString sizestr = QString::number( size );

        im = a_up;
        px = im.smoothScale( size, im.height() );
        QPixmapCache::insert( "$qt_aqua_vsbr_arw_up_" + sizestr, px );
        im = a_down;
        px = im.smoothScale( size, im.height() );
        QPixmapCache::insert( "$qt_aqua_vsbr_arw_down_" + sizestr, px );

        im = t_up;
        px = im.smoothScale( size, im.height() );
        QPixmapCache::insert( "$qt_aqua_vsbr_tip_up_" + sizestr, px );
        im = t_down;
        px = im.smoothScale( size, im.height() );
        QPixmapCache::insert( "$qt_aqua_vsbr_tip_down_" + sizestr, px );
        im = t_up_dis;
        px = im.smoothScale( size, im.height() );
        QPixmapCache::insert( "$qt_aqua_vsbr_tip_dis_up_" + sizestr, px );
        im = t_down_dis;
        px = im.smoothScale( size, im.height() );
        QPixmapCache::insert( "$qt_aqua_vsbr_tip_dis_down_" + sizestr, px );

        im = fill;
        px = im.smoothScale( size, im.height() );
        QPixmapCache::insert( "$qt_aqua_vsbr_fill_" + sizestr, px );
        im = fill_dis;
        px = im.smoothScale( size, im.height() );
        QPixmapCache::insert( "$qt_aqua_vsbr_dis_fill_" + sizestr, px );
        im = back_fill;
        px = im.smoothScale( size, im.height() );
        QPixmapCache::insert( "$qt_aqua_vsbr_back_fill_" + sizestr, px );
    }

    // Sliders
    if( s.contains("_pty_") ) {
        size = qAquaGetNum( s );
        QString sizestr = QString::number( size );

        im = QImage( (const char **) aqua_sldr_act_pty_xpm );
        px = im.smoothScale( im.width(), size );
        QPixmapCache::insert( "$qt_aqua_sldr_act_pty_" + sizestr, px );

        im = QImage( (const char **) aqua_sldr_dis_pty_xpm );
        px = im.smoothScale( im.width(), size );
        QPixmapCache::insert( "$qt_aqua_sldr_dis_pty_" + sizestr, px );
    }
    if( s.contains("sldr_grv") ){
        QPixmapCache::insert( "$qt_aqua_sldr_grv_tip_left",
                              (const char **) aqua_sldr_grv_tip_left_xpm);
        QPixmapCache::insert( "$qt_aqua_sldr_grv_tip_right",
                              (const char **) aqua_sldr_grv_tip_right_xpm );
        QPixmapCache::insert( "$qt_aqua_sldr_grv_mid",
                              (const char **) aqua_sldr_grv_mid_xpm );
    }

    // Combobox buttons
    if( s.contains("cmb_") ){
        QPixmap left( (const char **) aqua_cmb_act_left_xpm );
        QPixmap right( (const char **) aqua_cmb_act_right_xpm );
        QPixmap mid( (const char **) aqua_cmb_act_mid_xpm );
        QPixmap right_dis( (const char **) aqua_cmb_dis_right_xpm );

        size = qAquaGetNum( s );
        QString sizestr = QString::number( size );

        im = left;
        px = im.smoothScale( im.width(), size );
        QPixmapCache::insert( "$qt_aqua_cmb_act_left_" + sizestr, px );
        im = right;
        px = im.smoothScale( im.width(), size );
        QPixmapCache::insert( "$qt_aqua_cmb_act_right_" +sizestr, px );
        im = right_dis;
        px = im.smoothScale( im.width(), size );
        QPixmapCache::insert( "$qt_aqua_cmb_dis_right_" +sizestr, px );
        im = mid;
        px = im.smoothScale( im.width(), size );
        QPixmapCache::insert( "$qt_aqua_cmb_act_mid_" + sizestr, px );
    }

    // Tabs - top position
    if( s.contains("tab_t_") ){
        QPixmapCache::insert( "$qt_aqua_tab_t_act_left",
                              (const char **) aqua_tab_act_left_xpm );
        QPixmapCache::insert( "$qt_aqua_tab_t_act_mid",
                              (const char **) aqua_tab_act_mid_xpm );
        QPixmapCache::insert( "$qt_aqua_tab_t_act_right",
                              (const char **) aqua_tab_act_right_xpm );

        QPixmapCache::insert( "$qt_aqua_tab_t_dis_left",
                              (const char **) aqua_tab_dis_left_xpm );
        QPixmapCache::insert( "$qt_aqua_tab_t_dis_mid",
                              (const char **) aqua_tab_dis_mid_xpm );
        QPixmapCache::insert( "$qt_aqua_tab_t_dis_right",
                              (const char **) aqua_tab_dis_right_xpm );

        // Pixmaps for the inactive state
        QPixmapCache::insert( "$qt_aqua_tab_t_usel_dis_left",
                              (const char **) aqua_tab_usel_dis_left_xpm );
        QPixmapCache::insert( "$qt_aqua_tab_t_usel_dis_mid",
                              (const char **) aqua_tab_usel_dis_mid_xpm );
        QPixmapCache::insert( "$qt_aqua_tab_t_usel_dis_right",
                              (const char **) aqua_tab_usel_dis_right_xpm );

        QPixmapCache::insert( "$qt_aqua_tab_t_sel_dis_left",
                              (const char **) aqua_tab_sel_dis_left_xpm );
        QPixmapCache::insert( "$qt_aqua_tab_t_sel_dis_mid",
                              (const char **) aqua_tab_sel_dis_mid_xpm );
        QPixmapCache::insert( "$qt_aqua_tab_t_sel_dis_right",
                              (const char **) aqua_tab_sel_dis_right_xpm );

        QPixmapCache::insert( "$qt_aqua_tab_t_top_act",
                              (const char **) aqua_tab_top_act_xpm );
        QPixmapCache::insert( "$qt_aqua_tab_t_top_dis",
                              (const char **) aqua_tab_top_dis_xpm );

    }
    // Tabs - bottom position (top pixmaps mirrored)
    if( s.contains("tab_b_") ){
        QPixmap tab_act_left( (const char **) aqua_tab_act_left_xpm );
        QPixmap tab_act_mid( (const char **) aqua_tab_act_mid_xpm );
        QPixmap tab_act_right( (const char **) aqua_tab_act_right_xpm );
        QPixmap tab_dis_left( (const char **) aqua_tab_dis_left_xpm );
        QPixmap tab_dis_mid( (const char **) aqua_tab_dis_mid_xpm );
        QPixmap tab_dis_right( (const char **) aqua_tab_dis_right_xpm );

        QPixmap tab_usel_dis_left((const char **) aqua_tab_usel_dis_left_xpm );
        QPixmap tab_usel_dis_mid((const char **) aqua_tab_usel_dis_mid_xpm );
        QPixmap tab_usel_dis_right((const char **) aqua_tab_usel_dis_right_xpm );

        QPixmap tab_sel_dis_left( (const char **) aqua_tab_sel_dis_left_xpm );
        QPixmap tab_sel_dis_mid((const char **) aqua_tab_sel_dis_mid_xpm );
        QPixmap tab_sel_dis_right((const char **) aqua_tab_sel_dis_right_xpm );

        QPixmap tab_top_act( (const char **) aqua_tab_top_act_xpm );
        QPixmap tab_top_dis( (const char **) aqua_tab_top_dis_xpm );

        im = tab_act_left;
        px = im.mirror( FALSE, TRUE );
        QPixmapCache::insert( "$qt_aqua_tab_b_act_left", px );
        im = tab_act_mid;
        px = im.mirror( FALSE, TRUE );
        QPixmapCache::insert( "$qt_aqua_tab_b_act_mid", px );
        im = tab_act_right;
        px = im.mirror( FALSE, TRUE );
        QPixmapCache::insert( "$qt_aqua_tab_b_act_right", px );

        im = tab_dis_left;
        px = im.mirror( FALSE, TRUE );
        QPixmapCache::insert( "$qt_aqua_tab_b_dis_left", px );
        im = tab_dis_mid;
        px = im.mirror( FALSE, TRUE );
        QPixmapCache::insert( "$qt_aqua_tab_b_dis_mid", px );
        im = tab_dis_right;
        px = im.mirror( FALSE, TRUE );
        QPixmapCache::insert( "$qt_aqua_tab_b_dis_right", px );

        // Pixmaps for the inactive state
        im = tab_usel_dis_left;
        px = im.mirror( FALSE, TRUE );
        QPixmapCache::insert( "$qt_aqua_tab_b_usel_dis_left", px );
        im = tab_usel_dis_mid;
        px = im.mirror( FALSE, TRUE );
        QPixmapCache::insert( "$qt_aqua_tab_b_usel_dis_mid", px );
        im = tab_usel_dis_right;
        px = im.mirror( FALSE, TRUE );
        QPixmapCache::insert( "$qt_aqua_tab_b_usel_dis_right", px );

        im = tab_sel_dis_left;
        px = im.mirror( FALSE, TRUE );
        QPixmapCache::insert( "$qt_aqua_tab_b_sel_dis_left", px );
        im = tab_sel_dis_mid;
        px = im.mirror( FALSE, TRUE );
        QPixmapCache::insert( "$qt_aqua_tab_b_sel_dis_mid", px );
        im = tab_sel_dis_right;
        px = im.mirror( FALSE, TRUE );
        QPixmapCache::insert( "$qt_aqua_tab_b_sel_dis_right", px );

        im = tab_top_act;
        px = im.mirror( FALSE, TRUE );
        QPixmapCache::insert( "$qt_aqua_tab_b_top_act", px );
        im = tab_top_dis;
        px = im.mirror( FALSE, TRUE );
        QPixmapCache::insert( "$qt_aqua_tab_b_top_dis", px );

    }

    // Tool buttons
    if( s.contains("toolbtn_") ){
        // Normal state buttons
        QPixmap right( (const char **) aqua_toolbtn_nrm_right_xpm );
        QPixmap left( (const char **) aqua_toolbtn_nrm_left_xpm );
        QPixmap mid( (const char **) aqua_toolbtn_nrm_mid_xpm );
        // Toggle buttons that are turned on
        QPixmap act_left( (const char **) aqua_toolbtn_act_left_xpm );
        QPixmap act_right( (const char **) aqua_toolbtn_act_right_xpm );
        QPixmap act_mid( (const char **) aqua_toolbtn_act_mid_xpm );
        // Any button that is pushed down
        QPixmap psh_left( (const char **) aqua_toolbtn_psh_left_xpm );
        QPixmap psh_right( (const char **) aqua_toolbtn_psh_right_xpm );
        QPixmap psh_mid( (const char **) aqua_toolbtn_psh_mid_xpm );

        int width = 0, height = 0, j;
        i = s.findRev( '_' );
        if( i != -1 ){
            height = s.right( (s.length() - 1) - i ).toInt();
        }
        j = s.findRev( '_', i - 1 );
        if( j != -1 ){
            width = s.mid( j + 1, i - j - 1 ).toInt();
        }
        QString wstr = QString::number( width );
        QString hstr = QString::number( height );

        im = left;
        px = im.smoothScale( width, height );
        QPixmapCache::insert( "$qt_aqua_toolbtn_nrm_left_" + wstr + "_" + hstr, px );
        im = right;
        px = im.smoothScale( width, height );
        QPixmapCache::insert( "$qt_aqua_toolbtn_nrm_right_" + wstr + "_" + hstr, px );
        im = mid;
        px = im.smoothScale( width, height );
        QPixmapCache::insert( "$qt_aqua_toolbtn_nrm_mid_" + wstr + "_" + hstr, px );
        im = act_mid;
        px = im.smoothScale( width, height );
        QPixmapCache::insert( "$qt_aqua_toolbtn_act_mid_" + wstr + "_" + hstr, px );
        im = act_left;
        px = im.smoothScale( width, height );
        QPixmapCache::insert( "$qt_aqua_toolbtn_act_left_" + wstr + "_" + hstr, px );
        im = act_right;
        px = im.smoothScale( width, height );
        QPixmapCache::insert( "$qt_aqua_toolbtn_act_right_" + wstr + "_" +hstr, px );
        im = psh_mid;
        px = im.smoothScale( width, height );
        QPixmapCache::insert( "$qt_aqua_toolbtn_psh_mid_" + wstr + "_" + hstr, px );
        im = psh_left;
        px = im.smoothScale( width, height );
        QPixmapCache::insert( "$qt_aqua_toolbtn_psh_left_" + wstr + "_" + hstr, px );
        im = psh_right;
        px = im.smoothScale( width, height );
        QPixmapCache::insert( "$qt_aqua_toolbtn_psh_right_" + wstr + "_" +hstr, px );
    }

    // Tool bar separators
    if( s.contains("tbar_") ){
        QPixmap hsep( (const char **) aqua_tbar_hsep_xpm );
        QPixmap vsep( (const char **) aqua_tbar_vsep_xpm );

        size = qAquaGetNum( s );
        QString sizestr = QString::number( size );

        im = hsep;
        px = im.smoothScale( im.width(), size );
        QPixmapCache::insert( "$qt_aqua_tbar_hsep_" + sizestr, px );

        im = vsep;
        px = im.smoothScale( size, im.width() );
        QPixmapCache::insert( "$qt_aqua_tbar_vsep_" + sizestr, px );
    }

    QPixmapCache::find( str, p );
}

/*!
    Constructs a QAquaStyle
*/
QAquaStyle::QAquaStyle()
{
    d = new QAquaStylePrivate;
    d->timerId = -1;
}

/*!\reimp
*/
QAquaStyle::~QAquaStyle()
{
    delete d;
}

/*! \reimp
*/
void QAquaStyle::polish( QPalette & pal )
{
    oldPalette = pal;

    QPixmap px;
    qAquaPixmap( "gen_back", px );
    QBrush background( Qt::black, px );
    pal.setBrush( QPalette::Active, QColorGroup::Background, background );
    pal.setBrush( QPalette::Inactive, QColorGroup::Background, background );
    pal.setBrush( QPalette::Disabled, QColorGroup::Background, background );
    
    pal.setBrush( QPalette::Active, QColorGroup::Button, background );
    pal.setBrush( QPalette::Inactive, QColorGroup::Button, background );
    pal.setBrush( QPalette::Disabled, QColorGroup::Button, background );

    pal.setColor( QPalette::Inactive, QColorGroup::ButtonText,
                  QColor( 148,148,148 ));
    pal.setColor( QPalette::Disabled, QColorGroup::ButtonText,
                  QColor( 148,148,148 ));

    pal.setColor( QPalette::Active, QColorGroup::Highlight,
                  QColor( 49,255,255 ) );
    pal.setColor( QPalette::Inactive, QColorGroup::Highlight,
                  QColor( 148,148,148 ));
    pal.setColor( QPalette::Disabled, QColorGroup::Highlight,
                  QColor( 148,148,148 ));

    pal.setColor( QPalette::Active, QColorGroup::HighlightedText, Qt::black);
    pal.setColor( QPalette::Inactive, QColorGroup::HighlightedText, Qt::black);
    pal.setColor( QPalette::Disabled, QColorGroup::HighlightedText, Qt::black);
}

/*! \reimp
*/
void QAquaStyle::unPolish( QPalette & pal )
{
    pal = oldPalette;
    qApp->setPalette( pal, TRUE );
}

/*! \reimp
*/
void QAquaStyle::polish( QWidget * w )
{
    if( w->inherits("QPushButton") ){
        QPushButton * btn = (QPushButton *) w;
        if( btn->isDefault() || btn->autoDefault() ){
            btn->installEventFilter( this );
            if( d->timerId == -1 ){
                d->timerId = startTimer( 50 );
            }
        }
    }

    if( w->inherits("QToolButton") ){
        QToolButton * btn = (QToolButton *) w;
        btn->setAutoRaise( FALSE );
         if( btn->group() ){
             btn->group()->setMargin( 0 );
             btn->group()->setInsideSpacing( 0 );
         }
    }

     if( w->inherits("QToolBar") ){
         QToolBar * bar = (QToolBar *) w;
         QBoxLayout * layout = bar->boxLayout();
         layout->setSpacing( 0 );
         layout->setMargin( 0 );
     }

    if ( !w->isTopLevel() ) {
        if( !w->inherits("QSplitter") && w->backgroundPixmap() &&
            (w->backgroundMode() == QWidget::PaletteBackground) )
            w->setBackgroundOrigin( QWidget::WindowOrigin );
    }
}

/*! \reimp
*/
void QAquaStyle::unPolish( QWidget * w )
{
    if( w->inherits("QPushButton") ){
        QPushButton * btn = (QPushButton *) w;
        if( btn == d->defaultButton )
	    d->defaultButton = 0;

        if( d->timerId != -1 ){
            killTimer( d->timerId );
            d->timerId = -1;
        }
    }

    if( w->inherits("QToolButton") ){
        QToolButton * btn = (QToolButton *) w;
        btn->setAutoRaise( TRUE );
    }

    if ( !w->isTopLevel() ) {
        if( !w->inherits("QSplitter") && w->backgroundPixmap() &&
            (w->backgroundMode() == QWidget::PaletteBackground) )
            w->setBackgroundOrigin( QWidget::WidgetOrigin );
    }
}

/*! \reimp
  Called to animate the default buttons (emulates the pulsing effect found
  on the Mac).
*/
void QAquaStyle::timerEvent( QTimerEvent * te )
{
    if( te->timerId() == d->timerId ){
	if( d->defaultButton != 0 && (d->defaultButton->isDefault() || 
				      d->defaultButton->autoDefault()) )
	    d->defaultButton->repaint( FALSE );
    }
}

/*! \reimp
*/
bool QAquaStyle::eventFilter( QObject * o, QEvent * e )
{
    if( o->inherits("QPushButton") ){
        QPushButton * btn = (QPushButton *) o;

        if( e->type() == QEvent::FocusIn ) {
	    // Kb Focus received - make this the default button
	    d->defaultButton = btn;
	} else if( e->type() == QEvent::FocusOut || e->type() == QEvent::Show ) {

	    // Find the correct button to use as default button
	    QObjectList *list = btn->topLevelWidget()->queryList( "QPushButton" );
	    QObjectListIt it( * list );
	    QPushButton * pb;
	    while ( (pb = (QPushButton*)it.current()) ) {
		++it;
		if( ((e->type() == QEvent::FocusOut) && (pb->isDefault() || 
							 pb->autoDefault() && 
							 (pb != btn)) ) ||
		    ((e->type() == QEvent::Show) && pb->isDefault()) ) 
		{
		    QPushButton * tmp = d->defaultButton;
		    d->defaultButton = 0;
		    if( tmp != 0 )
			tmp->repaint( FALSE );
		    d->defaultButton = pb;
		    break;
		}
	    }
	    delete list;
        } else if( e->type() == QEvent::Hide ) {
	    if( d->defaultButton == btn ) 
		d->defaultButton = 0;
	}
    }
    return FALSE;
}

/*! \reimp
 */
int QAquaStyle::buttonDefaultIndicatorWidth() const
{
    return 0;
}

/*! \reimp */
void QAquaStyle::drawPopupPanel( QPainter *p, int x, int y, int w, int h,
                               const QColorGroup &g,  int lineWidth,
                               const QBrush *fill )
{
    QWindowsStyle::drawPopupPanel( p, x, y, w, h, g, lineWidth, fill );
}

/*!\reimp
*/
void QAquaStyle::drawButton( QPainter *p, int x, int y, int w, int h,
                             const QColorGroup &g, bool sunken, const QBrush* fill)
{
    // ### Reimplement this?
    QPen oldPen = p->pen();

     if (!sunken) {
         p->fillRect(x+3, y+3, w-6, h-6,fill ? *fill :
                                         g.brush( QColorGroup::Button ));
         // the bright side
         p->setPen(g.shadow());
         p->drawLine(x, y, x+w-1, y);
         p->drawLine(x, y, x, y+h-1);

         p->setPen(g.button());
         p->drawLine(x+1, y+1, x+w-2, y+1);
         p->drawLine(x+1, y+1, x+1, y+h-2);

         p->setPen(g.light());
         p->drawLine(x+2, y+2, x+2, y+h-2);
         p->drawLine(x+2, y+2, x+w-2, y+2);


         // the dark side!

         p->setPen(g.mid());
         p->drawLine(x+3, y+h-3 ,x+w-3, y+h-3);
         p->drawLine(x+w-3, y+3, x+w-3, y+h-3);

         p->setPen(g.dark());
         p->drawLine(x+2, y+h-2 ,x+w-2, y+h-2);
         p->drawLine(x+w-2, y+2, x+w-2, y+h-2);

         p->setPen(g.shadow());
         p->drawLine(x+1, y+h-1,x+w-1, y+h-1);
         p->drawLine(x+w-1, y, x+w-1, y+h-1);


         // top left corner:
         p->setPen(g.background());
         p->drawPoint(x, y);
         p->drawPoint(x+1, y);
         p->drawPoint(x, y+1);
         p->setPen(g.shadow());
         p->drawPoint(x+1, y+1);
         p->setPen(g.button());
         p->drawPoint(x+2, y+2);
         p->setPen(white);
         p->drawPoint(x+3, y+3);
         // bottom left corner:
         p->setPen(g.background());
         p->drawPoint(x, y+h-1);
         p->drawPoint(x+1, y+h-1);
         p->drawPoint(x, y+h-2);
         p->setPen(g.shadow());
         p->drawPoint(x+1, y+h-2);
         p->setPen(g.dark());
         p->drawPoint(x+2, y+h-3);
         // top right corner:
         p->setPen(g.background());
         p->drawPoint(x+w-1, y);
         p->drawPoint(x+w-2, y);
         p->drawPoint(x+w-1, y+1);
         p->setPen(g.shadow());
         p->drawPoint(x+w-2, y+1);
         p->setPen(g.dark());
         p->drawPoint(x+w-3, y+2);
         // bottom right corner:
         p->setPen(g.background());
         p->drawPoint(x+w-1, y+h-1);
         p->drawPoint(x+w-2, y+h-1);
         p->drawPoint(x+w-1, y+h-2);
         p->setPen(g.shadow());
         p->drawPoint(x+w-2, y+h-2);
         p->setPen(g.dark());
         p->drawPoint(x+w-3, y+h-3);
         p->setPen(g.mid());
         p->drawPoint(x+w-4, y+h-4);

     }
     else {
         p->fillRect(x+2, y+2, w-4, h-4,fill ? *fill :
                                           g.brush( QColorGroup::Dark ));

         // the dark side
         p->setPen(g.shadow());
         p->drawLine(x, y, x+w-1, y);
         p->drawLine(x, y, x, y+h-1);

         p->setPen(g.dark().dark());
         p->drawLine(x+1, y+1, x+w-2, y+1);
         p->drawLine(x+1, y+1, x+1, y+h-2);


         // the bright side!

         p->setPen(g.button());
         p->drawLine(x+1, y+h-2 ,x+w-2, y+h-2);
         p->drawLine(x+w-2, y+1, x+w-2, y+h-2);

         p->setPen(g.dark());
         p->drawLine(x, y+h-1,x+w-1, y+h-1);
         p->drawLine(x+w-1, y, x+w-1, y+h-1);

         // top left corner:
         p->setPen(g.background());
         p->drawPoint(x, y);
         p->drawPoint(x+1, y);
         p->drawPoint(x, y+1);
         p->setPen(g.shadow());
         p->drawPoint(x+1, y+1);
         p->setPen(g.dark().dark());
         p->drawPoint(x+3, y+3);
         // bottom left corner:
         p->setPen(g.background());
         p->drawPoint(x, y+h-1);
         p->drawPoint(x+1, y+h-1);
         p->drawPoint(x, y+h-2);
         p->setPen(g.shadow());
         p->drawPoint(x+1, y+h-2);
         // top right corner:
         p->setPen(g.background());
         p->drawPoint(x+w-1, y);
         p->drawPoint(x+w-2, y);
         p->drawPoint(x+w-1, y+1);
         p->setPen(g.shadow());
         p->drawPoint(x+w-2, y+1);
         // bottom right corner:
         p->setPen(g.background());
         p->drawPoint(x+w-1, y+h-1);
         p->drawPoint(x+w-2, y+h-1);
         p->drawPoint(x+w-1, y+h-2);
         p->setPen(g.shadow());
         p->drawPoint(x+w-2, y+h-2);
         p->setPen(g.dark());
         p->drawPoint(x+w-3, y+h-3);
         p->setPen(g.mid());
         p->drawPoint(x+w-4, y+h-4);


     }

     //     // top left corner:
//     p->setPen(g.background());
//     p->drawPoint(x, y);
//     p->drawPoint(x, y);


    p->setPen(oldPen);
}

/*! \reimp */

QRect QAquaStyle::buttonRect( int x, int y, int w, int h) const
{
    QRect r = QCommonStyle::buttonRect(x,y,w,h);
    r.setTop( r.top()+1);
    r.setLeft( r.left()+1);
    r.setBottom( r.bottom()-1);
    r.setRight( r.right()-1);
    return r;
}

void QAquaStyle::drawToolButton ( QToolButton * btn, QPainter * p )
{
    QPixmap px;
    QObject * obj = btn->parent();
    int x = 0;
    int y = 0;
    QString w = QString::number( btn->width() );
    QString h = QString::number( btn->height() );

    if( obj && obj->inherits("QToolBar") ){
        QToolBar * bar  = (QToolBar *) obj;
        QObjectList * l = bar->queryList( "QToolButton", 0, FALSE, FALSE );
        QObjectListIt it( *l );
        if( it.toFirst() == btn ){
            if( btn->isOn() )
                qAquaPixmap( "toolbtn_nrm_left_"+ w + "_" + h, px );
            else if( btn->isDown() )
                qAquaPixmap( "toolbtn_psh_left_"+ w + "_" + h, px );
            else
                qAquaPixmap( "toolbtn_nrm_left_"+ w + "_" + h, px );
        } else if( it.toLast() == btn ){
            if( btn->isOn() )
                qAquaPixmap( "toolbtn_psh_right_" + w + "_" + h, px );
            else if( btn->isDown() )
                qAquaPixmap( "toolbtn_psh_right_" + w + "_" + h, px );
            else
                qAquaPixmap( "toolbtn_nrm_right_" + w + "_" + h, px );
        } else {
            if( btn->isOn() )
                qAquaPixmap( "toolbtn_act_mid_" + w + "_" + h, px );
            else if( btn->isDown() )
                qAquaPixmap( "toolbtn_psh_mid_" + w + "_" + h, px );
            else
                qAquaPixmap( "toolbtn_nrm_mid_"+ w + "_" + h, px );
        }
        delete l;
    } else {
        if( btn->isDown() || btn->isOn() )
            qAquaPixmap( "bvlbtn_small_on_"+ w + "_" + h, px );
        else
            qAquaPixmap( "bvlbtn_small_off_" + w + "_" + h, px );
    }
    p->drawPixmap( x, y, px );
}

/*! \reimp */

void QAquaStyle::drawBevelButton( QPainter * p, int x, int y, int w, int h,
                                  const QColorGroup & g, bool sunken,
                                  const QBrush * fill)
{
    // ### Reimplement this?

    QPen oldPen = p->pen();

    // small or non-square bevel buttons are drawn in a small style, others in a big style.
    if ( w * h < 1600 || QABS(w-h) > 10) {
        // small buttons

        if (!sunken) {
            p->fillRect(x+2, y+2, w-4, h-4,fill ? *fill :
                                             g.brush( QColorGroup::Button ));
            // the bright side
            p->setPen(g.dark());
            p->drawLine(x, y, x+w-1, y);
            p->drawLine(x, y, x, y+h-1);

            p->setPen(g.light());
            p->drawLine(x+1, y+1, x+w-2, y+1);
            p->drawLine(x+1, y+1, x+1, y+h-2);

            // the dark side!

            p->setPen(g.mid());
            p->drawLine(x+2, y+h-2 ,x+w-2, y+h-2);
            p->drawLine(x+w-2, y+2, x+w-2, y+h-3);

            p->setPen(g.dark().dark());
            p->drawLine(x+1, y+h-1,x+w-1, y+h-1);
            p->drawLine(x+w-1, y+1, x+w-1, y+h-2);


        }
        else {
            p->fillRect(x+2, y+2, w-4, h-4,fill ? *fill :
                                               g.brush( QColorGroup::Mid ));

            // the dark side
            p->setPen(g.dark().dark());
            p->drawLine(x, y, x+w-1, y);
            p->drawLine(x, y, x, y+h-1);

            p->setPen(g.mid().dark());
            p->drawLine(x+1, y+1, x+w-2, y+1);
            p->drawLine(x+1, y+1, x+1, y+h-2);


            // the bright side!

            p->setPen(g.button());
            p->drawLine(x+1, y+h-2 ,x+w-2, y+h-2);
            p->drawLine(x+w-2, y+1, x+w-2, y+h-2);

            p->setPen(g.dark());
            p->drawLine(x, y+h-1,x+w-1, y+h-1);
            p->drawLine(x+w-1, y, x+w-1, y+h-1);
        }
    }
    else {
        // big ones

        if (!sunken) {
            p->fillRect(x+3, y+3, w-6, h-6,fill ? * fill :
                                            g.brush( QColorGroup::Button ));
            // the bright side
            p->setPen(g.button().dark());
            p->drawLine(x, y, x+w-1, y);
            p->drawLine(x, y, x, y+h-1);

            p->setPen(g.button());
            p->drawLine(x+1, y+1, x+w-2, y+1);
            p->drawLine(x+1, y+1, x+1, y+h-2);

            p->setPen(g.light());
            p->drawLine(x+2, y+2, x+2, y+h-2);
            p->drawLine(x+2, y+2, x+w-2, y+2);


         // the dark side!

            p->setPen(g.mid());
            p->drawLine(x+3, y+h-3 ,x+w-3, y+h-3);
            p->drawLine(x+w-3, y+3, x+w-3, y+h-3);

            p->setPen(g.dark());
            p->drawLine(x+2, y+h-2 ,x+w-2, y+h-2);
            p->drawLine(x+w-2, y+2, x+w-2, y+h-2);

            p->setPen(g.dark().dark());
            p->drawLine(x+1, y+h-1,x+w-1, y+h-1);
            p->drawLine(x+w-1, y+1, x+w-1, y+h-1);


        }
        else {
            p->fillRect(x+3, y+3, w-6, h-6,fill ? *fill :
                                                  g.brush( QColorGroup::Mid ));

            // the dark side
            p->setPen(g.dark().dark().dark());
            p->drawLine(x, y, x+w-1, y);
            p->drawLine(x, y, x, y+h-1);

            p->setPen(g.dark().dark());
            p->drawLine(x+1, y+1, x+w-2, y+1);
            p->drawLine(x+1, y+1, x+1, y+h-2);

            p->setPen(g.mid().dark());
            p->drawLine(x+2, y+2, x+2, y+w-2);
            p->drawLine(x+2, y+2, x+w-2, y+2);


            // the bright side!

            p->setPen(g.button());
            p->drawLine(x+2, y+h-3 ,x+w-3, y+h-3);
            p->drawLine(x+w-3, y+3, x+w-3, y+h-3);

            p->setPen(g.midlight());
            p->drawLine(x+1, y+h-2 ,x+w-2, y+h-2);
            p->drawLine(x+w-2, y+1, x+w-2, y+h-2);

            p->setPen(g.dark());
            p->drawLine(x, y+h-1,x+w-1, y+h-1);
            p->drawLine(x+w-1, y, x+w-1, y+h-1);


            // corners
//            p->setPen( mixedColor(g.dark().dark().dark(), g.dark()) );
            p->drawPoint( x, y+h-1 );
            p->drawPoint( x+w-1, y);

//            p->setPen( mixedColor(g.dark().dark(), g.midlight() ) );
            p->drawPoint( x+1, y+h-2 );
            p->drawPoint( x+w-2, y+1);

            //          p->setPen( mixedColor(g.mid().dark(), g.button() ) );
            p->drawPoint( x+2, y+h-3 );
            p->drawPoint( x+w-3, y+2);
        }
    }
    p->setPen(oldPen);

}

QRect QAquaStyle::pushButtonContentsRect( QPushButton* btn ) const
{
#ifndef QT_NO_PUSHBUTTON
    return buttonRect( 0, 0, btn->width(), btn->height() );
#endif
}

/*! \reimp */

void QAquaStyle::drawPushButton( QPushButton* btn, QPainter *p)
{
#ifndef QT_NO_PUSHBUTTON
    QPixmap left, mid, right;
    QColorGroup g = btn->colorGroup();
    int x, y, w, h;

    btn->rect().coords( &x, &y, &w, &h );
    p->fillRect( x, y, w+1, h+1, g.brush( QColorGroup::Background ) );


    // ### What about buttons that are so small that the pixmaps don't fit?
    if( w < 33 ){
        QWindowsStyle::drawPanel( p, x, y, w, h, btn->colorGroup(), btn->isDown(), 1 );
        return;
    }

    QString hstr = QString::number( h - y );
    if( (btn->isDefault() || btn->autoDefault()) && (d->defaultButton == btn) ) {
        int & alt = d->buttonState.frame;
        int & dir = d->buttonState.dir;
        if( alt > 0 ){
            QString num = QString::number( alt );
            qAquaPixmap( "btn_def_left" + num + "_" + hstr, left );
            qAquaPixmap( "btn_def_mid" + num + "_" + hstr, mid );
            qAquaPixmap( "btn_def_right" + num + "_" + hstr, right );
        } else {
            qAquaPixmap( "btn_def_left_" + hstr, left );
            qAquaPixmap( "btn_def_mid_" + hstr, mid );
            qAquaPixmap( "btn_def_right_" + hstr, right );
        }
	// Pause animation if button is down
	if( !btn->isDown() ) {
	    if( (dir == 1) && (alt == 9) ) dir = -1;
	    if( (dir == -1) && (alt == 0) ) dir = 1;
	    alt += dir;
	}
    } else if( btn->isDown() ){
        qAquaPixmap( "btn_def_left_" + hstr, left );
        qAquaPixmap( "btn_def_mid_" + hstr, mid );
        qAquaPixmap( "btn_def_right_" + hstr, right );
    } else if( !btn->isEnabled() ){
        qAquaPixmap( "btn_dis_left_" + hstr, left );
        qAquaPixmap( "btn_dis_mid_" + hstr, mid );
        qAquaPixmap( "btn_dis_right_" + hstr, right );
    } else {
        qAquaPixmap( "btn_nrm_left_" + hstr, left );
        qAquaPixmap( "btn_nrm_mid_" + hstr, mid );
        qAquaPixmap( "btn_nrm_right_" + hstr, right );
    }

    QBrush mid_f( Qt::black, mid );
    p->drawPixmap( x, y, left );
    p->drawTiledPixmap( x+left.width(), y, w-x-left.width()*2, h-y, mid );
    p->drawPixmap( w-right.width(), y, right );
#endif
}

/*! \reimp */

void QAquaStyle::drawPushButtonLabel( QPushButton* btn, QPainter *p)
{
#ifndef QT_NO_PUSHBUTTON
    bool on = btn->isDown() || btn->isOn();
    QRect r = pushButtonContentsRect( btn );
    int x, y, w, h;
    r.rect( &x, &y, &w, &h );
    if ( btn->isMenuButton() ) {
        int dx = menuButtonIndicatorWidth( btn->height() );

        QColorGroup g( btn->colorGroup() );
        int xx = x+w-dx-4;
        int yy = y-3;
        int hh = h+6;

        if ( !on ) {
            p->setPen( g.mid() ); // mid
            p->drawLine(xx, yy+2, xx, yy+hh-3);
            p->setPen( g.button() );
            p->drawLine(xx+1, yy+1, xx+1, yy+hh-2);
            p->setPen( g.light() );
            p->drawLine(xx+2, yy+2, xx+2, yy+hh-2);
        }
        drawArrow( p, DownArrow, FALSE,
                   x+w-dx-1, y+2, dx, h-4,
                   btn->colorGroup(),
                   btn->isEnabled() );
        w -= dx;
    }

    if ( btn->iconSet() && !btn->iconSet()->isNull() ) {
        QIconSet::Mode mode = btn->isEnabled()
                              ? QIconSet::Normal : QIconSet::Disabled;
        if ( mode == QIconSet::Normal && btn->hasFocus() )
            mode = QIconSet::Active;
        QPixmap pixmap = btn->iconSet()->pixmap( QIconSet::Small, mode );
        int pixw = pixmap.width();
        int pixh = pixmap.height();
        p->drawPixmap( x+2, y+h/2-pixh/2, pixmap );
        x += pixw + 4;
        w -= pixw + 4;
    }

    drawItem( p, x, y, w, h,
              AlignCenter|ShowPrefix, btn->colorGroup(), btn->isEnabled(),
              btn->pixmap(), btn->text(), -1, &btn->colorGroup().buttonText());
#endif
}


#define HORIZONTAL      (sb->orientation() == QScrollBar::Horizontal)
#define VERTICAL        !HORIZONTAL
#define MOTIF_BORDER    2
#define SLIDER_MIN      9 // ### motif says 6 but that's too small

/*! \reimp */

void QAquaStyle::scrollBarMetrics( const QScrollBar* sb,
                                       int &sliderMin, int &sliderMax,
                                       int &sliderLength, int& buttonDim )const
{
#ifndef QT_NO_SCROLLBAR
    int maxLength;
    int b = 0;
    int length = HORIZONTAL ? sb->width()  : sb->height();
    int extent = HORIZONTAL ? sb->height() : sb->width();

    if ( length > ( extent - b*2 - 1 )*2 + b*2  )
        buttonDim = extent - b*2;
    else
        buttonDim = ( length - b*2 )/2 - 1;

    if( HORIZONTAL ){
        sliderMin = b + 26;
        maxLength  = length - b*2 - 26*2;
    } else {
        sliderMin = b + 26; //buttonDim;
        maxLength  = length - b*2 - 26*2; //buttonDim*2;
    }

    if ( sb->maxValue() == sb->minValue() ) {
        sliderLength = maxLength;
    } else {
        sliderLength = (sb->pageStep()*maxLength)/
                        (sb->maxValue()-sb->minValue()+sb->pageStep());
        uint range = sb->maxValue()-sb->minValue();
        if ( sliderLength < SLIDER_MIN || range > INT_MAX/2 )
            sliderLength = SLIDER_MIN;
        if ( sliderLength > maxLength )
            sliderLength = maxLength;
    }
    sliderMax = sliderMin + maxLength - sliderLength;
#endif
}

/*!\reimp
 */
void QAquaStyle::drawScrollBarControls( QPainter* p, const QScrollBar* sb,
                                        int sliderStart, uint /*controls*/,
                                        uint /*activeControl*/ )
{
#ifndef QT_NO_SCROLLBAR
#define ADD_LINE_ACTIVE ( activeControl == AddLine )
#define SUB_LINE_ACTIVE ( activeControl == SubLine )
    QColorGroup g  = sb->colorGroup();
    bool active = qAquaActive( g );

    int sliderMin, sliderMax, sliderLength, buttonDim;
    scrollBarMetrics( sb, sliderMin, sliderMax, sliderLength, buttonDim );

    if (sliderStart > sliderMax) { // sanity check
        sliderStart = sliderMax;
    }

    int b = 0;
    int dimB = buttonDim;
    QRect addB;
    QRect subB;
    QRect addPageR;
    QRect subPageR;
    QRect sliderR;
    int addX, addY, subX, subY;
    int length = HORIZONTAL ? sb->width()  : sb->height();
    int extent = HORIZONTAL ? sb->height() : sb->width();

    if ( HORIZONTAL ) {
        subY = addY = ( extent - dimB ) / 2;
        subX = b;
        addX = length - 26;//dimB - b;
    } else {
        subX = addX = ( extent - dimB ) / 2;
        subY = b;
        addY = length - 26;//dimB - b;
    }

    if( HORIZONTAL ){
        subB.setRect( subX,subY,26,dimB );
        addB.setRect( addX,addY,26,dimB );
    } else {
        subB.setRect( subX,subY,dimB,26 );
        addB.setRect( addX,addY,dimB,26 );
    }

    int sliderEnd = sliderStart + sliderLength;
    int sliderW = extent - b*2;
    if ( HORIZONTAL ) {
        subPageR.setRect( subB.right() + 1, b,
                          sliderStart - subB.right() - 1 , sliderW );
        addPageR.setRect( sliderEnd, b, addX - sliderEnd, sliderW );
        sliderR .setRect( sliderStart, b, sliderLength, sliderW );
    } else {
        subPageR.setRect( b, subB.bottom() + 1, sliderW,
                          sliderStart - subB.bottom() - 1 );
        addPageR.setRect( b, sliderEnd, sliderW, addY - sliderEnd );
        sliderR .setRect( b, sliderStart, sliderW, sliderLength );
    }

    QString nstr = QString::number( extent );
    p->setBackgroundMode( OpaqueMode );

    QPixmap arw_l, arw_r, arw_u, arw_d;
    QPixmap tip_l, tip_r, tip_u, tip_d, fill, back_fill;

    if( active ){
        if( HORIZONTAL ){
            qAquaPixmap( "hsbr_tip_right_" + nstr, tip_r );
            qAquaPixmap( "hsbr_tip_left_" + nstr, tip_l );
            qAquaPixmap( "hsbr_fill_" + nstr, fill );
            qAquaPixmap( "hsbr_back_fill_" + nstr, back_fill );
            qAquaPixmap( "hsbr_arw_left_" + nstr, arw_l );
            qAquaPixmap( "hsbr_arw_right_" + nstr, arw_r );
        } else {
            qAquaPixmap( "vsbr_tip_up_" + nstr, tip_u );
            qAquaPixmap( "vsbr_tip_down_" + nstr, tip_d );
            qAquaPixmap( "vsbr_fill_" + nstr, fill );
            qAquaPixmap( "vsbr_back_fill_" + nstr, back_fill );
            qAquaPixmap( "vsbr_arw_up_" + nstr, arw_u );
            qAquaPixmap( "vsbr_arw_down_" + nstr, arw_d );
        }
    } else {
        if( HORIZONTAL ){
            qAquaPixmap( "hsbr_tip_dis_right_" + nstr, tip_r );
            qAquaPixmap( "hsbr_tip_dis_left_" + nstr, tip_l );
            qAquaPixmap( "hsbr_dis_fill_" + nstr, fill );
            qAquaPixmap( "hsbr_back_fill_" + nstr, back_fill );
            qAquaPixmap( "hsbr_arw_left_" + nstr, arw_l );
            qAquaPixmap( "hsbr_arw_right_" + nstr, arw_r );
        } else {
            qAquaPixmap( "vsbr_tip_dis_up_" + nstr, tip_u );
            qAquaPixmap( "vsbr_tip_dis_down_" + nstr, tip_d );
            qAquaPixmap( "vsbr_dis_fill_" + nstr, fill );
            qAquaPixmap( "vsbr_back_fill_" + nstr, back_fill );
            qAquaPixmap( "vsbr_arw_up_" + nstr, arw_u );
            qAquaPixmap( "vsbr_arw_down_" + nstr, arw_d );
        }
    }
    p->drawTiledPixmap( subPageR, back_fill );
    p->drawTiledPixmap( addPageR, back_fill );
    if( HORIZONTAL ){
        p->drawPixmap( addB.x(), addB.y(), arw_r );
        p->drawPixmap( subB.x(), subB.y(), arw_l );
    } else {
        p->drawPixmap( addB.x(), addB.y(), arw_d );
        p->drawPixmap( subB.x(), subB.y(), arw_u );
    }

    if( HORIZONTAL ){
        p->drawPixmap( sliderR.x()-tip_l.width(), sliderR.y(), tip_l );
        p->drawPixmap( sliderR.x()+sliderR.width(), sliderR.y(), tip_r );
    } else {
        p->drawPixmap( sliderR.x(), sliderR.y()-tip_u.height(), tip_u );
        p->drawPixmap( sliderR.x(), sliderR.y()+sliderR.height(), tip_d );
    }
    QBrush br( Qt::black, fill );
    p->fillRect( sliderR, br );
#endif
}

/*! \reimp */

void QAquaStyle::drawIndicator( QPainter* p, int x, int y, int /*w*/, int/*h*/,
                                const QColorGroup  & g, int s, bool down,
                                bool enabled )
{
    QPixmap px;
    bool on = (s == QButton::On);
    if( enabled && qAquaActive( g ) ){
        if( down && on )
            qAquaPixmap("chk_psh_t", px);
        else if( on )
            qAquaPixmap("chk_act_t", px);
        else if( !on && down )
            qAquaPixmap("chk_psh_f", px);
        else
            qAquaPixmap("chk_act_f", px);
    } else {
        on ? qAquaPixmap("chk_dis_t", px) : qAquaPixmap("chk_act_f", px);
    }
    p->drawPixmap( x, y, px );
}

/*! \reimp */

void QAquaStyle::drawIndicatorMask( QPainter *p, int x, int y, int w, int h,
                                    int /*s*/ )
{
    p->fillRect(x, y, w, h, color1);
}

/*! \reimp */

QSize
QAquaStyle::indicatorSize() const
{
    return QSize(15,18);
}

#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)
/*! \reimp */

void QAquaStyle::drawExclusiveIndicator( QPainter* p, int x, int y, int /*w*/,
                                         int /*h*/, const QColorGroup & g,
                                         bool on, bool down,
                                         bool enabled )
{
    QPixmap px;
    if( enabled  && qAquaActive( g ) ){
        if( down && on )
            qAquaPixmap("radio_psh_t", px);
        else if( on )
            qAquaPixmap("radio_t", px);
        else if( !on && down )
            qAquaPixmap("radio_psh_f", px);
        else
            qAquaPixmap("radio_f", px);
    } else {
        on ? qAquaPixmap("radio_dis_t", px) : qAquaPixmap("radio_f", px);
    }
    p->drawPixmap( x, y, px );
}

/*!\reimp
*/
void QAquaStyle::drawExclusiveIndicatorMask( QPainter *p, int x, int y,
                                             int /*w*/, int /*h*/,
                                             bool /* on */)
{
    QBitmap radio_mask( aqua_radio_mask_xbm_width,
                        aqua_radio_mask_xbm_height,
                        (const uchar *) aqua_radio_mask_xbm_bits, TRUE );
    p->drawPixmap( x, y, radio_mask );
}

/*! \reimp */

QSize QAquaStyle::exclusiveIndicatorSize() const
{
    return QSize(14,17);
}

/*! \reimp */

void QAquaStyle::drawComboButton( QPainter *p, int x, int y, int w, int h,
                                  const QColorGroup & g, bool /* sunken */,
                                  bool /*editable*/, bool /* enabled */,
                                  const QBrush * /*fill*/ )
{

    QPixmap left, mid, right;
    QString hstr = QString::number( h );
    bool active = qAquaActive( g );

    qAquaPixmap( "cmb_act_left_" + hstr, left );
    qAquaPixmap( "cmb_act_mid_" + hstr, mid );
    if( active )
        qAquaPixmap( "cmb_act_right_" + hstr, right );
    else
        qAquaPixmap( "cmb_dis_right_" + hstr, right );

    p->drawPixmap( x, y, left );
    p->drawTiledPixmap( x + left.width(), y, w - left.width()*2, h, mid );
    p->drawPixmap( x + w - right.width(), y, right );
}


/*! \reimp */

QRect QAquaStyle::comboButtonRect( int x, int y, int w, int h) const
{
    QRect r(x+3, y+3, w-6-20, h-6);
    if( QApplication::reverseLayout() )
        r.moveBy( 16, 0 );
    return r;
}

/*! \reimp */

QRect QAquaStyle::comboButtonFocusRect( int x, int y, int w, int h) const
{
    return QRect(x+4, y+4, w-8-20, h-8);
}


/*! \reimp */
int QAquaStyle::sliderLength() const
{
    return 17;
}

/*! \reimp */
void QAquaStyle::drawSlider( QPainter *p, int x, int y, int /*w*/, int h,
                             const QColorGroup & g,
                             Orientation /*orient*/ , bool /* tickAbove */,
                             bool /*tickBelow*/ )
{
    // ### Orientation
    QPixmap px;
    QString hstr = QString::number( h );

    if( qAquaActive( g ) )
        qAquaPixmap( "sldr_act_pty_" + hstr, px );
    else
        qAquaPixmap( "sldr_dis_pty_" + hstr, px );

    p->drawPixmap( x, y, px );
}

/*! \reimp */
void QAquaStyle::drawSliderMask( QPainter *p, int x, int y, int /*w*/,
                                 int /*h*/,
                                 Orientation /*orient*/ , bool /* tickAbove */,
                                 bool /*tickBelow*/ )
{
    // ### Orientation
    QBitmap slider_mask( aqua_sldr_pty_mask_width,
                         aqua_sldr_pty_mask_height,
                         (const uchar *) aqua_sldr_pty_mask_bits, TRUE );
    p->drawPixmap( x, y, slider_mask );

}


/*! \reimp */
void QAquaStyle::drawSliderGroove( QPainter *p, int x, int y, int w, int /*h*/,
                                   const QColorGroup& /*g*/, QCOORD /*c*/,
                                   Orientation /*orient*/ )
{
    // ### Orientation

    int offset = 3;
    QPixmap sldr_l, sldr_mid, sldr_r;

    qAquaPixmap( "sldr_grv_tip_left", sldr_l );
    qAquaPixmap( "sldr_grv_mid", sldr_mid );
    qAquaPixmap( "sldr_grv_tip_right", sldr_r );

    p->drawPixmap( x, y + offset, sldr_l );
    p->drawTiledPixmap( x + sldr_l.width(), y + offset, w - sldr_l.width()*2,
                        sldr_l.height(), sldr_mid );
    p->drawPixmap( x + w - sldr_r.width(), y + offset, sldr_r );
}

/*! \reimp
  */
int QAquaStyle::maximumSliderDragDistance() const
{
    return -1;
}


/*! \reimp
*/
void QAquaStyle::polishPopupMenu( QPopupMenu* p)
{
    QWindowsStyle::polishPopupMenu( p );
}



/*! \reimp
*/
void QAquaStyle::drawCheckMark( QPainter *p, int x, int y, int w, int h,
                                const QColorGroup &g,
                                bool act, bool dis )
{
    QWindowsStyle::drawCheckMark( p, x, y, w, h, g, act, dis );
}

static const int aquaSepHeight         = 10;    // separator height
static const int aquaItemFrame         = 2;    // menu item frame width
static const int aquaItemHMargin       = 3;    // menu item hor text margin
static const int aquaItemVMargin       = 2;    // menu item ver text margin
static const int aquaArrowHMargin      = 6;    // arrow horizontal margin
static const int aquaTabSpacing        = 12;   // space between text and tab
static const int aquaCheckMarkHMargin  = 2;    // horiz. margins of check mark
static const int aquaRightBorder       = 12;   // right border on aqua
static const int aquaCheckMarkWidth    = 12;   // checkmarks width on aqua

/*! \reimp
*/
int QAquaStyle::extraPopupMenuItemWidth( bool checkable, int maxpmw,
                                         QMenuItem* mi,
                                         const QFontMetrics& /*fm*/ ) const
{
#ifndef QT_NO_POPUPMENU
    int w = 2*aquaItemHMargin + 2*aquaItemFrame; // a little bit of border can never harm

    if ( mi->pixmap() )
        w += mi->pixmap()->width();     // pixmap only

    if ( !mi->text().isNull() ) {
        if ( mi->text().find('\t') >= 0 )       // string contains tab
            w += aquaTabSpacing;
    }

    if ( maxpmw ) { // we have iconsets
        w += maxpmw;
        w += 6; // add a little extra border around the iconset
    }

    if ( checkable && maxpmw < aquaCheckMarkWidth ) {
        w += aquaCheckMarkWidth - maxpmw; // space for the checkmarks
    }

    if ( maxpmw > 0 || checkable ) // we have a check-column ( iconsets or checkmarks)
        w += aquaCheckMarkHMargin; // add space to separate the columns

    w += aquaRightBorder;

    return w;
#endif
}


/*! \reimp
*/
int QAquaStyle::popupMenuItemHeight( bool /*checkable*/, QMenuItem* mi,
                                     const QFontMetrics& fm ) const
{
#ifndef QT_NO_POPUPMENU
    int h = 0;

    if( mi->isSeparator() )
        h = aquaSepHeight;
    else if ( mi->pixmap() )         // pixmap height
        h = mi->pixmap()->height() + 2*aquaItemFrame;
    else                                        // text height
        h = fm.height() + 2*aquaItemVMargin + 2*aquaItemFrame;

    if ( !mi->isSeparator() && mi->iconSet() != 0 ) {
        h = QMAX( h, mi->iconSet()->pixmap( QIconSet::Small,
                              QIconSet::Normal ).height() + 2*aquaItemFrame );
    }
    if ( mi->custom() )
        h = QMAX( h, mi->custom()->sizeHint().height() + 2*aquaItemVMargin +
                  2*aquaItemFrame );
    return h;
#endif
}

/*! \reimp
*/
void QAquaStyle::drawPopupMenuItem( QPainter* p, bool checkable,
                                    int maxpmw, int tab, QMenuItem* mi,
                                    const QPalette& pal,
                                    bool act, bool enabled,
                                    int x, int y, int w, int h )
{
#ifndef QT_NO_POPUPMENU
    const QColorGroup & g = pal.active();
    bool dis = !enabled;
    QColorGroup itemg = dis ? pal.disabled() : pal.active();

    if ( checkable ) {
#if defined(Q_WS_WIN)
        if ( qWinVersion() == Qt::WV_2000 ||
             qWinVersion() == Qt::WV_98 || 
	     qWinVersion() == Qt::WV_XP )
            maxpmw = QMAX( maxpmw, 16 );
#endif
        maxpmw = QMAX( maxpmw, 12 ); // space for the checkmarks
    }

    int checkcol = maxpmw;
    QPixmap selectedBackground;
    qAquaPixmap( "sel_back", selectedBackground );

    if ( mi && mi->isSeparator() ) // Aqua separators are just empty menuitems
        return;

    QBrush fill = act? QBrush( Qt::black, selectedBackground ) :
                       g.brush( QColorGroup::Button );
    p->fillRect( x, y, w, h, fill);

    if ( !mi )
        return;

    bool reverse = QApplication::reverseLayout();

    int xpos = x;
    if ( reverse )
        xpos += w - checkcol;
    if ( mi->isChecked() ) {
        // Do nothing
    } else if ( !act ) {
        p->fillRect(xpos, y, checkcol , h,
                    g.brush( QColorGroup::Button ));
    }

    if ( mi->iconSet() ) {              // draw iconset
        QIconSet::Mode mode = dis ? QIconSet::Disabled : QIconSet::Normal;
        if (act && !dis )
            mode = QIconSet::Active;
	QPixmap pixmap;
	if ( checkable && mi->isChecked() )
	    pixmap = mi->iconSet()->pixmap( QIconSet::Small, mode, QIconSet::On );
	else
	    pixmap = mi->iconSet()->pixmap( QIconSet::Small, mode );
        int pixw = pixmap.width();
        int pixh = pixmap.height();
        if ( act && !dis ) {
            if ( !mi->isChecked() )
                qDrawShadePanel( p, xpos, y, checkcol, h, g, FALSE, 1,
                                 &g.brush( QColorGroup::Button ) );
        }
        QRect cr( xpos, y, checkcol, h );
        QRect pmr( 0, 0, pixw, pixh );
        pmr.moveCenter( cr.center() );
        p->setPen( itemg.text() );
        p->drawPixmap( pmr.topLeft(), pixmap );

        QBrush fill = act? QBrush( Qt::black, selectedBackground ) :
                           g.brush( QColorGroup::Button );
        int xp;
        if ( reverse )
            xp = x;
        else
            xp = xpos + checkcol + 1;
        p->fillRect( xp, y, w - checkcol - 1, h, fill);
    } else  if ( checkable ) {  // just "checking"...
        int mw = checkcol + aquaItemFrame;
        int mh = h - 2*aquaItemFrame;
        if ( mi->isChecked() ) {
            int xp = xpos;
            if( reverse )
                xp -= aquaItemFrame;
            else
                xp += aquaItemFrame;
            drawCheckMark( p, xp, y+aquaItemFrame, mw, mh, itemg, act, dis );
        }
    }

    p->setPen( act ? Qt::white/*g.highlightedText()*/ : g.buttonText() );

    QColor discol;
    if ( dis ) {
        discol = itemg.text();
        p->setPen( discol );
    }

    int xm = aquaItemFrame + checkcol + aquaItemHMargin;
    if ( reverse )
        xpos = aquaItemFrame + tab;
    else
        xpos += xm;

    if ( mi->custom() ) {
        int m = aquaItemVMargin;
        p->save();
        if ( dis && !act ) {
            p->setPen( g.light() );
            mi->custom()->paint( p, itemg, act, enabled,
                                 xpos+1, y+m+1, w-xm-tab+1, h-2*m );
            p->setPen( discol );
        }
        mi->custom()->paint( p, itemg, act, enabled,
                             x+xm, y+m, w-xm-tab+1, h-2*m );
        p->restore();
    }
    QString s = mi->text();
    if ( !s.isNull() ) {                        // draw text
        int t = s.find( '\t' );
        int m = aquaItemVMargin;
        const int text_flags = AlignVCenter|ShowPrefix | DontClip | SingleLine;
        if ( t >= 0 ) {                         // draw tab text
            int xp;
            if( reverse )
                xp = x + aquaRightBorder+aquaItemHMargin+aquaItemFrame - 1;
            else
                xp = x + w - tab - aquaRightBorder-aquaItemHMargin-aquaItemFrame+1;
            if ( dis && !act ) {
                p->setPen( g.light() );
                p->drawText( xp, y+m+1, tab, h-2*m, text_flags, s.mid( t+1 ));
                p->setPen( discol );
            }
            p->drawText( xp, y+m, tab, h-2*m, text_flags, s.mid( t+1 ) );
            s = s.left( t );
        }
        if ( dis && !act ) {
            p->setPen( g.light() );
            p->drawText( xpos+1, y+m+1, w-xm-tab+1, h-2*m, text_flags, s, t );
            p->setPen( discol );
        }
        p->drawText( xpos, y+m, w-xm-tab+1, h-2*m, text_flags, s, t );
    } else if ( mi->pixmap() ) {                        // draw pixmap
        QPixmap *pixmap = mi->pixmap();
        if ( pixmap->depth() == 1 )
            p->setBackgroundMode( OpaqueMode );
        p->drawPixmap( xpos, y+aquaItemFrame, *pixmap );
        if ( pixmap->depth() == 1 )
            p->setBackgroundMode( TransparentMode );
    }
    if ( mi->popup() ) {                        // draw sub menu arrow
        int dim = (h-2*aquaItemFrame) / 2;
        ArrowType arrow;
        if ( reverse ) {
            arrow = LeftArrow;
            xpos = x + aquaArrowHMargin + aquaItemFrame;
        } else {
            arrow = RightArrow;
            xpos = x+w - aquaArrowHMargin - aquaItemFrame - dim;
        }
        if ( act ) {
            if ( !dis )
                discol = white;
            QColorGroup g2( discol, g.highlight(),
                            white, white,
                            dis ? discol : white,
                            discol, white );
            drawArrow( p, arrow, FALSE,
                               xpos,  y+h/2-dim/2,
                               dim, dim, g2, TRUE );
        } else {
            drawArrow( p, arrow, FALSE,
                               xpos,  y+h/2-dim/2,
                               dim, dim, g, mi->isEnabled() );
        }
    }
#endif
}

/*!\reimp
 */
void QAquaStyle::getButtonShift( int &x, int &y) const
{
    x = 0;
    y = 0;
}

/*!\reimp
 */
void QAquaStyle::drawTab( QPainter * p,  const QTabBar * tb, QTab * t,
                          bool selected )
{
#ifndef QT_NO_TABBAR
    QPixmap left, mid, right;
    QRect r( t->rect() );
    QString pos;

    if( tb->shape() == QTabBar::RoundedAbove )
        pos = "t";
    else
        pos = "b";


    if( qAquaActive( tb->colorGroup() ) ){
        if( selected ){
            qAquaPixmap( "tab_"+ pos +"_act_left", left );
            qAquaPixmap( "tab_"+ pos +"_act_mid", mid );
            qAquaPixmap( "tab_"+ pos +"_act_right", right );
        } else {
            qAquaPixmap( "tab_"+ pos +"_dis_left", left );
            qAquaPixmap( "tab_"+ pos +"_dis_mid", mid );
            qAquaPixmap( "tab_"+ pos +"_dis_right", right );
        }
    } else {
        if( selected ){
            qAquaPixmap( "tab_"+ pos +"_sel_dis_left", left );
            qAquaPixmap( "tab_"+ pos +"_sel_dis_mid", mid );
            qAquaPixmap( "tab_"+ pos +"_sel_dis_right", right );
        } else {
            qAquaPixmap( "tab_"+ pos +"_usel_dis_left", left );
            qAquaPixmap( "tab_"+ pos +"_usel_dis_mid", mid );
            qAquaPixmap( "tab_"+ pos +"_usel_dis_right", right );
        }
    }

    p->drawPixmap( r.x(), r.y(), left );
    p->drawTiledPixmap( r.x() + left.width(), r.y(), r.width()-left.width()*2,
                        r.height(), mid );
    p->drawPixmap( r.x() + r.width() - right.width(), r.y(), right );
#endif
}

/*!\reimp
*/
void QAquaStyle::tabBarExtensionMetrics( const QTabWidget * tw, int & w,
                                         int & h, int & overlap ) const
{
    w = tw->width();
    h = 8;
    overlap = 2;
}


/*!\reimp
*/
void QAquaStyle::drawTabBarExtension( QPainter * p, int, int, int w, int h,
                                      const QColorGroup & cg,
                                      const QTabWidget * tw )
{
    QPixmap px;

    if( qAquaActive( cg ) ){
        if( tw->tabPosition() == QTabWidget::Top )
           qAquaPixmap( "tab_t_top_act", px );
        else
           qAquaPixmap( "tab_b_top_act", px );
    } else {
        if( tw->tabPosition() == QTabWidget::Top )
           qAquaPixmap( "tab_t_top_dis", px );
        else
           qAquaPixmap( "tab_b_top_dis", px );
    }

    p->drawTiledPixmap( 0, 0, w, h, px );
}

/*!\reimp
*/
void QAquaStyle::drawPanel( QPainter *p, int x, int y, int w, int h,
                            const QColorGroup & g, bool sunken,
                            int lineWidth, const QBrush * fill )
{
    QWindowsStyle::drawPanel( p, x, y, w, h, g, sunken, lineWidth, fill );
}

/*
  !\reimp
 */
void QAquaStyle::drawToolBarPanel( QPainter * p, int x, int y,
                                   int w, int h, const QColorGroup & g,
                                   const QBrush * /*fill*/ )
{
    p->fillRect( x, y, w, h, g.brush( QColorGroup::Button ) );
}

/*
  !\reimp
*/
void QAquaStyle::drawToolBarSeparator( QPainter * p, int, int, int w, int h,
                                       const QColorGroup &,
                                       Qt::Orientation orientation )
{
    QPixmap px;
    if( orientation == Vertical ){
        qAquaPixmap( "tbar_vsep_" + QString::number(w), px );
    } else {
        qAquaPixmap( "tbar_hsep_" + QString::number(h), px );
    }
    p->drawPixmap( 0, 0, px );
}

/*
  !\reimp
 */
QSize QAquaStyle::toolBarSeparatorSize( Qt::Orientation orientation ) const
{
    if( orientation == Vertical )
        return QSize( 0, 8 );
    else
        return QSize( 8, 0 );
}


/*
  !\reimp
 */
void QAquaStyle::drawMenuBarPanel( QPainter * p, int x, int y,
                                   int w, int h, const QColorGroup & g,
                                   const QBrush * /*fill*/ )
{
    p->fillRect( x, y, w, h, g.brush( QColorGroup::Button ) );
}

/*!\reimp
 */
void QAquaStyle::drawToolBarHandle( QPainter *p, const QRect &r,
                                    Qt::Orientation orientation,
                                    bool highlight, const QColorGroup &cg,
                                    bool drawBorder )
{
    p->save();
    p->translate( r.x(), r.y() );

    QColor dark( cg.dark() );
    QColor light( cg.light() );
    unsigned int i;
    if ( orientation == Qt::Vertical ) {
        int w = r.width();
        if ( w > 6 ) {
            if ( highlight )
                p->fillRect( 1, 1, w - 2, 9, cg.highlight() );
            QPointArray a( 2 * ((w-6)/3) );

            int x = 3 + (w%3)/2;
            p->setPen( dark );
            p->drawLine( 1, 8, w-2, 8 );
            for( i=0; 2*i < a.size(); i ++ ) {
                a.setPoint( 2*i, x+1+3*i, 6 );
                a.setPoint( 2*i+1, x+2+3*i, 3 );
            }
            p->drawPoints( a );
            p->setPen( light );
            p->drawLine( 1, 9, w-2, 9 );
            for( i=0; 2*i < a.size(); i++ ) {
                a.setPoint( 2*i, x+3*i, 5 );
                a.setPoint( 2*i+1, x+1+3*i, 2 );
            }
            p->drawPoints( a );
            if ( drawBorder ) {
                p->setPen( QPen( Qt::darkGray ) );
                p->drawLine( r.width() - 1, 0,
                             r.width() - 1, toolBarHandleExtent() );
            }
        }
    } else {
        int h = r.height();
        if ( h > 6 ) {
            if ( highlight )
                p->fillRect( 1, 1, 8, h - 2, cg.highlight() );
            QPointArray a( 2 * ((h-6)/3) );
            int y = 3 + (h%3)/2;
            p->setPen( dark );
            p->drawLine( 8, 1, 8, h-2 );
            for( i=0; 2*i < a.size(); i ++ ) {
                a.setPoint( 2*i, 5, y+1+3*i );
                a.setPoint( 2*i+1, 2, y+2+3*i );
            }
            p->drawPoints( a );
            p->setPen( light );
            p->drawLine( 9, 1, 9, h-2 );
            for( i=0; 2*i < a.size(); i++ ) {
                a.setPoint( 2*i, 4, y+3*i );
                a.setPoint( 2*i+1, 1, y+1+3*i );
            }
            p->drawPoints( a );
            if ( drawBorder ) {
                p->setPen( QPen( Qt::darkGray ) );
                p->drawLine( 0, r.height() - 1,
                             toolBarHandleExtent(), r.height() - 1 );
            }
        }
    }
    p->restore();
}

void QAquaStyle::drawMenuBarItem( QPainter* p, int x, int y, int w, int h,
                                  QMenuItem* mi, QColorGroup& g,
                                  bool active, bool down, bool )
{
    if( down && active ){
        QPixmap px;
        qAquaPixmap( "sel_back", px );
        p->fillRect( x,y,w,h, QBrush( black, px ) );
    } else {
        p->fillRect( x,y,w,h, g.brush( QColorGroup::Button ) );
    }
    drawItem( p, x, y, w, h, AlignCenter|ShowPrefix|DontClip|SingleLine,
              g, mi->isEnabled(), mi->pixmap(), mi->text(), -1,
              (down && active) ? &white : &g.buttonText() );
}

void  QAquaStyle::drawFocusRect( QPainter * p, const QRect & r,
                                 const QColorGroup & g, const QColor* bg,
                                 bool atBorder)
{
    QWindowsStyle::drawFocusRect( p, r, g, bg, atBorder );
}

#endif
