/****************************************************************************
**
** Definition of Qt namespace (as class for compiler compatibility).
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

#ifndef QNAMESPACE_H
#define QNAMESPACE_H

#ifndef QT_H
#include "qglobal.h"
#endif // QT_H


class Q_EXPORT Qt {
#ifdef Q_MOC_RUN
    Q_OBJECT
    Q_ENUMS( Orientation TextFormat BackgroundMode DateFormat )
    Q_SETS( Alignment )
#endif
public:
    // documented in qcolor.cpp
    enum GlobalColor {
	color0,
	color1,
	black,
	white,
	darkGray,
	gray,
	lightGray,
	red,
	green,
	blue,
	cyan,
	magenta,
	yellow,
	darkRed,
	darkGreen,
	darkBlue,
	darkCyan,
	darkMagenta,
	darkYellow
    };

    // documented in qevent.cpp
    enum ButtonState {				// mouse/keyboard state values
	NoButton	= 0x0000,
	LeftButton	= 0x0001,
	RightButton	= 0x0002,
	MidButton	= 0x0004,
	MouseButtonMask = 0x0007,
	ShiftButton	= 0x0100,
	ControlButton   = 0x0200,
	AltButton	= 0x0400,
	MetaButton	= 0x0800,
	KeyButtonMask	= 0x0f00,
	Keypad		= 0x4000
    };

    // documented in qobject.cpp
    // ideally would start at 1, as in QSizePolicy, but that breaks other things
    enum Orientation {
        Horizontal = 0,
	Vertical
    };

    // documented in qlistview.cpp
    enum SortOrder {
	Ascending,
	Descending
    };

    // Text formatting flags for QPainter::drawText and QLabel
    // the following four enums can be combined to one integer which
    // is passed as textflag to drawText and qt_format_text.

    // documented in qpainter.cpp
    enum AlignmentFlags {
	AlignAuto		= 0x0000, 	// text alignment
	AlignLeft		= 0x0001,
	AlignRight		= 0x0002,
	AlignHCenter		= 0x0004,
	AlignJustify		= 0x0008,
	AlignHorizontal_Mask	= AlignLeft | AlignRight | AlignHCenter | AlignJustify,
	AlignTop		= 0x0010,
	AlignBottom		= 0x0020,
	AlignVCenter		= 0x0040,
	AlignVertical_Mask 	= AlignTop | AlignBottom | AlignVCenter,
	AlignCenter		= AlignVCenter | AlignHCenter
    };

#ifdef Q_MOC_RUN
    enum Alignment { // public text alignment without masks
	AlignAuto, AlignLeft, AlignRight, AlignHCenter, AlignJustify,
	AlignTop, AlignBottom, AlignVCenter, AlignCenter
    };
#else
    typedef AlignmentFlags Alignment;
#endif

    // documented in qpainter.cpp
    enum TextFlags {
	SingleLine	= 0x0080,		// misc. flags
	DontClip	= 0x0100,
	ExpandTabs	= 0x0200,
	ShowPrefix	= 0x0400,
	WordBreak	= 0x0800,
	BreakAnywhere = 0x1000,
#ifndef Q_QDOC
	DontPrint	= 0x2000,
	Underline = 0x01000000,
	Overline  = 0x02000000,
	StrikeOut = 0x04000000,
	IncludeTrailingSpaces = 0x08000000,
#endif
	NoAccel = 0x4000
    };

    // Widget flags; documented in qwidget.cpp

    // QWidget state flags (internal, barely documented in qwidget.cpp)
    enum WidgetState {
	WState_Created		= 0x00000001,
	WState_Reserved3	= 0x00000002, // was Disabled
	WState_Visible		= 0x00000004,
	WState_Hidden		= 0x00000008,
	WState_ForceHide	= WState_Hidden,
	WState_Reserve6		= 0x00000010,
	WState_Reserve5		= 0x00000020, // was MouseTracking
	WState_CompressKeys	= 0x00000040,
	WState_BlockUpdates	= 0x00000080,
	WState_InPaintEvent	= 0x00000100,
	WState_Reparented	= 0x00000200,
	WState_ConfigPending	= 0x00000400,
	WState_Resized		= 0x00000800,
	WState_AutoMask		= 0x00001000,
	WState_Polished		= 0x00002000,
	WState_DND		= 0x00004000,
	WState_Reserved0	= 0x00008000,
	WState_Reserved1	= 0x00010000,
	WState_OwnSizePolicy	= 0x00020000,
	WState_ExplicitShowHide	= 0x00040000,
	WState_Maximized	= 0x00080000,
	WState_Minimized	= 0x00100000,
	WState_Reserved4	= 0x00200000, // was ForceDisabled
	WState_Exposed		= 0x00400000,
	WState_Reserved2	= 0x00800000 // was HasMouse
    };
    typedef QFlags<WidgetState> WState;

    // Widget flags2; documented in qwidget.cpp

    // documented in qwidget.cpp
    enum WindowFlags {
	WType_TopLevel		= 0x00000001,	// widget type flags
	WType_Dialog		= 0x00000002,
	WType_Popup		= 0x00000004,
	WType_Desktop		= 0x00000008,
	WType_Mask		= 0x0000000f,

	WStyle_Customize	= 0x00000010,	// window style flags
	WStyle_NormalBorder	= 0x00000020,
	WStyle_DialogBorder	= 0x00000040, // MS-Windows only
	WStyle_NoBorder		= 0x00002000,
	WStyle_Title		= 0x00000080,
	WStyle_SysMenu		= 0x00000100,
	WStyle_Minimize		= 0x00000200,
	WStyle_Maximize		= 0x00000400,
	WStyle_MinMax		= WStyle_Minimize | WStyle_Maximize,
	WStyle_Tool		= 0x00000800,
	WStyle_StaysOnTop	= 0x00001000,
	WStyle_ContextHelp	= 0x00004000,
	WStyle_Reserved		= 0x00008000,
	WStyle_Mask		= 0x0000fff0,

	WDestructiveClose	= 0x00010000,	// misc flags
	WPaintDesktop		= 0x00020000,
	WPaintUnclipped		= 0x00040000,
	//reserved WPaintClever		= 0x00080000,
	//reserved		= 0x00100000, // was ResizeNoErase
	WMouseNoMask		= 0x00200000,

#if defined(Q_WS_X11)
	WX11BypassWM		= 0x01000000,
	WWinOwnDC		= 0x00000000,
	WMacNoSheet             = 0x00000000,
        WMacDrawer              = 0x00000000,
#elif defined(Q_WS_MAC)
	WX11BypassWM		= 0x00000000,
	WWinOwnDC		= 0x00000000,
	WMacNoSheet             = 0x01000000,
        WMacDrawer              = 0x20000000,
#else
	WX11BypassWM		= 0x00000000,
	WWinOwnDC		= 0x01000000,
	WMacNoSheet             = 0x00000000,
        WMacDrawer              = 0x00000000,
#endif
	WGroupLeader		= 0x02000000,
	WShowModal		= 0x04000000,
	WNoMousePropagation	= 0x08000000,
	WSubWindow              = 0x10000000,
#if defined(Q_WS_X11)
        WStyle_Splash           = 0x20000000
#else
	WStyle_Splash           = WStyle_NoBorder | WStyle_StaysOnTop | WMacNoSheet |
	WStyle_Tool | WWinOwnDC
#endif
#ifndef QT_NO_COMPAT
	,
	WStaticContents		= 0x00400000,
	WNoAutoErase		= 0x00800000,
	WRepaintNoErase	= WNoAutoErase,
	WResizeNoErase = 0,
	WNorthWestGravity	= WStaticContents,
	WType_Modal		= WType_Dialog | WShowModal,
	WStyle_Dialog		= WType_Dialog,
	WStyle_NoBorderEx	= WStyle_NoBorder,
	WPaintClever = 0
#endif
    };

    typedef QFlags<WindowFlags> WFlags;


    // Image conversion flags.  The unusual ordering is caused by
    // compatibility and default requirements.
    // Documented in qimage.cpp

    enum ImageConversionFlags {
	ColorMode_Mask		= 0x00000003,
	AutoColor		= 0x00000000,
	ColorOnly		= 0x00000003,
	MonoOnly		= 0x00000002,
	//	  Reserved	= 0x00000001,

	AlphaDither_Mask	= 0x0000000c,
	ThresholdAlphaDither	= 0x00000000,
	OrderedAlphaDither	= 0x00000004,
	DiffuseAlphaDither	= 0x00000008,
	NoAlpha			= 0x0000000c, // Not supported

	Dither_Mask		= 0x00000030,
	DiffuseDither		= 0x00000000,
	OrderedDither		= 0x00000010,
	ThresholdDither		= 0x00000020,
	//	  ReservedDither= 0x00000030,

	DitherMode_Mask		= 0x000000c0,
	AutoDither		= 0x00000000,
	PreferDither		= 0x00000040,
	AvoidDither		= 0x00000080
    };

    // documented in qpainter.cpp
    enum BGMode	{				// background mode
	TransparentMode,
	OpaqueMode
    };

#ifndef QT_NO_COMPAT
    // documented in qpainter.cpp
    enum PaintUnit {				// paint unit
	PixelUnit,
	LoMetricUnit, // OBSOLETE
	HiMetricUnit, // OBSOLETE
	LoEnglishUnit, // OBSOLETE
	HiEnglishUnit, // OBSOLETE
	TwipsUnit // OBSOLETE
    };
#endif

    // documented in qstyle.cpp
#ifdef QT_NO_COMPAT
    enum GUIStyle {
	WindowsStyle = 1,     // ### Qt 4.0: either remove the obsolete enums or clean up compat vs.
	MotifStyle = 4        // ### QT_NO_COMPAT by reordering or combination into one enum.
    };
#else
    enum GUIStyle {
	MacStyle, // OBSOLETE
	WindowsStyle,
	Win3Style, // OBSOLETE
	PMStyle, // OBSOLETE
	MotifStyle
    };
#endif

    // documented in qkeysequence.cpp
    enum SequenceMatch {
	NoMatch,
	PartialMatch,
	Identical
    };

    // documented in qevent.cpp
    enum Modifier {		// accelerator modifiers
	META          = 0x02000000,
	SHIFT         = 0x04000000,
	CTRL          = 0x08000000,
	ALT           = 0x10000000,
	MODIFIER_MASK = 0x1e000000,

	UNICODE_ACCEL = 0x00000000, // Qt 3.x compat
	ASCII_ACCEL = UNICODE_ACCEL // 1.x compat
    };

    // documented in qevent.cpp
    enum Key {
	Key_Escape = 0x01000000,		// misc keys
	Key_Tab = 0x01000001,
	Key_Backtab = 0x01000002, Key_BackTab = Key_Backtab,
	Key_Backspace = 0x01000003, Key_BackSpace = Key_Backspace,
	Key_Return = 0x01000004,
	Key_Enter = 0x01000005,
	Key_Insert = 0x01000006,
	Key_Delete = 0x01000007,
	Key_Pause = 0x01000008,
	Key_Print = 0x01000009,
	Key_SysReq = 0x0100000a,
	Key_Clear = 0x0100000b,
	Key_Home = 0x01000010,		// cursor movement
	Key_End = 0x01000011,
	Key_Left = 0x01000012,
	Key_Up = 0x01000013,
	Key_Right = 0x01000014,
	Key_Down = 0x01000015,
	Key_Prior = 0x01000016, Key_PageUp = Key_Prior,
	Key_Next = 0x01000017, Key_PageDown = Key_Next,
	Key_Shift = 0x01000020,		// modifiers
	Key_Control = 0x01000021,
	Key_Meta = 0x01000022,
	Key_Alt = 0x01000023,
	Key_CapsLock = 0x01000024,
	Key_NumLock = 0x01000025,
	Key_ScrollLock = 0x01000026,
	Key_F1 = 0x01000030,		// function keys
	Key_F2 = 0x01000031,
	Key_F3 = 0x01000032,
	Key_F4 = 0x01000033,
	Key_F5 = 0x01000034,
	Key_F6 = 0x01000035,
	Key_F7 = 0x01000036,
	Key_F8 = 0x01000037,
	Key_F9 = 0x01000038,
	Key_F10 = 0x01000039,
	Key_F11 = 0x0100003a,
	Key_F12 = 0x0100003b,
	Key_F13 = 0x0100003c,
	Key_F14 = 0x0100003d,
	Key_F15 = 0x0100003e,
	Key_F16 = 0x0100003f,
	Key_F17 = 0x01000040,
	Key_F18 = 0x01000041,
	Key_F19 = 0x01000042,
	Key_F20 = 0x01000043,
	Key_F21 = 0x01000044,
	Key_F22 = 0x01000045,
	Key_F23 = 0x01000046,
	Key_F24 = 0x01000047,
	Key_F25 = 0x01000048,		// F25 .. F35 only on X11
	Key_F26 = 0x01000049,
	Key_F27 = 0x0100004a,
	Key_F28 = 0x0100004b,
	Key_F29 = 0x0100004c,
	Key_F30 = 0x0100004d,
	Key_F31 = 0x0100004e,
	Key_F32 = 0x0100004f,
	Key_F33 = 0x01000050,
	Key_F34 = 0x01000051,
	Key_F35 = 0x01000052,
	Key_Super_L = 0x01000053, 		// extra keys
	Key_Super_R = 0x01000054,
	Key_Menu = 0x01000055,
	Key_Hyper_L = 0x01000056,
	Key_Hyper_R = 0x01000057,
	Key_Help = 0x01000058,
	Key_Direction_L = 0x01000059,
	Key_Direction_R = 0x01000060,
	Key_Space = 0x20,		// 7 bit printable ASCII
	Key_Any = Key_Space,
	Key_Exclam = 0x21,
	Key_QuoteDbl = 0x22,
	Key_NumberSign = 0x23,
	Key_Dollar = 0x24,
	Key_Percent = 0x25,
	Key_Ampersand = 0x26,
	Key_Apostrophe = 0x27,
	Key_ParenLeft = 0x28,
	Key_ParenRight = 0x29,
	Key_Asterisk = 0x2a,
	Key_Plus = 0x2b,
	Key_Comma = 0x2c,
	Key_Minus = 0x2d,
	Key_Period = 0x2e,
	Key_Slash = 0x2f,
	Key_0 = 0x30,
	Key_1 = 0x31,
	Key_2 = 0x32,
	Key_3 = 0x33,
	Key_4 = 0x34,
	Key_5 = 0x35,
	Key_6 = 0x36,
	Key_7 = 0x37,
	Key_8 = 0x38,
	Key_9 = 0x39,
	Key_Colon = 0x3a,
	Key_Semicolon = 0x3b,
	Key_Less = 0x3c,
	Key_Equal = 0x3d,
	Key_Greater = 0x3e,
	Key_Question = 0x3f,
	Key_At = 0x40,
	Key_A = 0x41,
	Key_B = 0x42,
	Key_C = 0x43,
	Key_D = 0x44,
	Key_E = 0x45,
	Key_F = 0x46,
	Key_G = 0x47,
	Key_H = 0x48,
	Key_I = 0x49,
	Key_J = 0x4a,
	Key_K = 0x4b,
	Key_L = 0x4c,
	Key_M = 0x4d,
	Key_N = 0x4e,
	Key_O = 0x4f,
	Key_P = 0x50,
	Key_Q = 0x51,
	Key_R = 0x52,
	Key_S = 0x53,
	Key_T = 0x54,
	Key_U = 0x55,
	Key_V = 0x56,
	Key_W = 0x57,
	Key_X = 0x58,
	Key_Y = 0x59,
	Key_Z = 0x5a,
	Key_BracketLeft = 0x5b,
	Key_Backslash = 0x5c,
	Key_BracketRight = 0x5d,
	Key_AsciiCircum = 0x5e,
	Key_Underscore = 0x5f,
	Key_QuoteLeft = 0x60,
	Key_BraceLeft = 0x7b,
	Key_Bar = 0x7c,
	Key_BraceRight = 0x7d,
	Key_AsciiTilde = 0x7e,

	// Latin 1 codes adapted from X: keysymdef.h,v 1.21 94/08/28 16:17:06

	Key_nobreakspace = 0x0a0,
	Key_exclamdown = 0x0a1,
	Key_cent = 0x0a2,
	Key_sterling = 0x0a3,
	Key_currency = 0x0a4,
	Key_yen = 0x0a5,
	Key_brokenbar = 0x0a6,
	Key_section = 0x0a7,
	Key_diaeresis = 0x0a8,
	Key_copyright = 0x0a9,
	Key_ordfeminine = 0x0aa,
	Key_guillemotleft = 0x0ab,	// left angle quotation mark
	Key_notsign = 0x0ac,
	Key_hyphen = 0x0ad,
	Key_registered = 0x0ae,
	Key_macron = 0x0af,
	Key_degree = 0x0b0,
	Key_plusminus = 0x0b1,
	Key_twosuperior = 0x0b2,
	Key_threesuperior = 0x0b3,
	Key_acute = 0x0b4,
	Key_mu = 0x0b5,
	Key_paragraph = 0x0b6,
	Key_periodcentered = 0x0b7,
	Key_cedilla = 0x0b8,
	Key_onesuperior = 0x0b9,
	Key_masculine = 0x0ba,
	Key_guillemotright = 0x0bb,	// right angle quotation mark
	Key_onequarter = 0x0bc,
	Key_onehalf = 0x0bd,
	Key_threequarters = 0x0be,
	Key_questiondown = 0x0bf,
	Key_Agrave = 0x0c0,
	Key_Aacute = 0x0c1,
	Key_Acircumflex = 0x0c2,
	Key_Atilde = 0x0c3,
	Key_Adiaeresis = 0x0c4,
	Key_Aring = 0x0c5,
	Key_AE = 0x0c6,
	Key_Ccedilla = 0x0c7,
	Key_Egrave = 0x0c8,
	Key_Eacute = 0x0c9,
	Key_Ecircumflex = 0x0ca,
	Key_Ediaeresis = 0x0cb,
	Key_Igrave = 0x0cc,
	Key_Iacute = 0x0cd,
	Key_Icircumflex = 0x0ce,
	Key_Idiaeresis = 0x0cf,
	Key_ETH = 0x0d0,
	Key_Ntilde = 0x0d1,
	Key_Ograve = 0x0d2,
	Key_Oacute = 0x0d3,
	Key_Ocircumflex = 0x0d4,
	Key_Otilde = 0x0d5,
	Key_Odiaeresis = 0x0d6,
	Key_multiply = 0x0d7,
	Key_Ooblique = 0x0d8,
	Key_Ugrave = 0x0d9,
	Key_Uacute = 0x0da,
	Key_Ucircumflex = 0x0db,
	Key_Udiaeresis = 0x0dc,
	Key_Yacute = 0x0dd,
	Key_THORN = 0x0de,
	Key_ssharp = 0x0df,
#ifndef QT_NO_COMPAT
	Key_agrave = Key_Agrave,
	Key_aacute = Key_Aacute,
	Key_acircumflex = Key_Acircumflex,
	Key_atilde = Key_Atilde,
	Key_adiaeresis = Key_Adiaeresis,
	Key_aring = Key_Aring,
	Key_ae = Key_AE,
	Key_ccedilla = Key_Ccedilla,
	Key_egrave = Key_Egrave,
	Key_eacute = Key_Eacute,
	Key_ecircumflex = Key_Ecircumflex,
	Key_ediaeresis = Key_Ediaeresis,
	Key_igrave = Key_Igrave,
	Key_iacute = Key_Iacute,
	Key_icircumflex = Key_Icircumflex,
	Key_idiaeresis = Key_Idiaeresis,
	Key_eth = Key_ETH,
	Key_ntilde = Key_Ntilde,
	Key_ograve = Key_Ograve,
	Key_oacute = Key_Oacute,
	Key_ocircumflex = Key_Ocircumflex,
	Key_otilde = Key_Otilde,
	Key_odiaeresis = Key_Odiaeresis,
#endif
	Key_division = 0x0f7,
#ifndef QT_NO_COMPAT
	Key_oslash = Key_Ooblique,
	Key_ugrave = Key_Ugrave,
	Key_uacute = Key_Uacute,
	Key_ucircumflex = Key_Ucircumflex,
	Key_udiaeresis = Key_Udiaeresis,
	Key_yacute = Key_Yacute,
	Key_thorn = Key_THORN,
#endif
	Key_ydiaeresis = 0x0ff,

	// multimedia/internet keys - ignored by default - see QKeyEvent c'tor

	Key_Back  = 0x01000061,
	Key_Forward  = 0x01000062,
	Key_Stop  = 0x01000063,
	Key_Refresh  = 0x01000064,

	Key_VolumeDown = 0x01000070,
	Key_VolumeMute  = 0x01000071,
	Key_VolumeUp = 0x01000072,
	Key_BassBoost = 0x01000073,
	Key_BassUp = 0x01000074,
	Key_BassDown = 0x01000075,
	Key_TrebleUp = 0x01000076,
	Key_TrebleDown = 0x01000077,

	Key_MediaPlay  = 0x01000080,
	Key_MediaStop  = 0x01000081,
	Key_MediaPrev  = 0x01000082,
	Key_MediaNext  = 0x01000083,
	Key_MediaRecord = 0x01000084,

	Key_HomePage  = 0x01000090,
	Key_Favorites  = 0x01000091,
	Key_Search  = 0x01000092,
	Key_Standby = 0x01000093,
	Key_OpenUrl = 0x01000094,

	Key_LaunchMail  = 0x010000a0,
	Key_LaunchMedia = 0x010000a1,
	Key_Launch0  = 0x010000a2,
	Key_Launch1  = 0x010000a3,
	Key_Launch2  = 0x010000a4,
	Key_Launch3  = 0x010000a5,
	Key_Launch4  = 0x010000a6,
	Key_Launch5  = 0x010000a7,
	Key_Launch6  = 0x010000a8,
	Key_Launch7  = 0x010000a9,
	Key_Launch8  = 0x010000aa,
	Key_Launch9  = 0x010000ab,
	Key_LaunchA  = 0x010000ac,
	Key_LaunchB  = 0x010000ad,
	Key_LaunchC  = 0x010000ae,
	Key_LaunchD  = 0x010000af,
	Key_LaunchE  = 0x010000b0,
	Key_LaunchF  = 0x010000b1,

	Key_MediaLast = 0x0100ffff,

	Key_unknown = 0x01ffffff
    };

    // documented in qcommonstyle.cpp
    enum ArrowType {
	UpArrow,
	DownArrow,
	LeftArrow,
	RightArrow
    };

    // documented in qpainter.cpp
    enum RasterOp { // raster op mode
	CopyROP,
	OrROP,
	XorROP,
	NotAndROP, EraseROP=NotAndROP,
	NotCopyROP,
	NotOrROP,
	NotXorROP,
	AndROP,	NotEraseROP=AndROP,
	NotROP,
	ClearROP,
	SetROP,
	NopROP,
	AndNotROP,
	OrNotROP,
	NandROP,
	NorROP,	LastROP=NorROP
    };

    // documented in qpainter.cpp
    enum PenStyle { // pen style
	NoPen,
	SolidLine,
	DashLine,
	DotLine,
	DashDotLine,
	DashDotDotLine,
	MPenStyle = 0x0f
    };

    // documented in qpainter.cpp
    enum PenCapStyle { // line endcap style
	FlatCap = 0x00,
	SquareCap = 0x10,
	RoundCap = 0x20,
	MPenCapStyle = 0x30
    };

    // documented in qpainter.cpp
    enum PenJoinStyle { // line join style
	MiterJoin = 0x00,
	BevelJoin = 0x40,
	RoundJoin = 0x80,
	MPenJoinStyle = 0xc0
    };

    // documented in qpainter.cpp
    enum BrushStyle { // brush style
	NoBrush,
	SolidPattern,
	Dense1Pattern,
	Dense2Pattern,
	Dense3Pattern,
	Dense4Pattern,
	Dense5Pattern,
	Dense6Pattern,
	Dense7Pattern,
	HorPattern,
	VerPattern,
	CrossPattern,
	BDiagPattern,
	FDiagPattern,
	DiagCrossPattern,
	CustomPattern=24
    };

    // documented in qapplication_mac.cpp
    enum MacintoshVersion {
	//Unknown
	MV_Unknown      = 0x0000,

	//Version numbers
	MV_9            = 0x0001,
	MV_10_DOT_0     = 0x0002,
	MV_10_DOT_1     = 0x0003,
	MV_10_DOT_2     = 0x0004,
	MV_10_DOT_3     = 0x0005,

	//Code names
	MV_CHEETAH      = MV_10_DOT_0,
	MV_PUMA         = MV_10_DOT_1,
	MV_JAGUAR       = MV_10_DOT_2,
	MV_PANTHER      = MV_10_DOT_3
    };

    // documented in qapplication_win.cpp
    enum WindowsVersion {
	WV_32s 		= 0x0001,
	WV_95 		= 0x0002,
	WV_98		= 0x0003,
	WV_Me		= 0x0004,
	WV_DOS_based	= 0x000f,

	WV_NT 		= 0x0010,
	WV_2000 	= 0x0020,
	WV_XP		= 0x0030,
	WV_NT_based	= 0x00f0,

	WV_CE           = 0x0100,
	WV_CENET	= 0x0200,
	WV_CE_based	= 0x0f00
    };

    // documented in qstyle.cpp
    enum UIEffect {
	UI_General,
	UI_AnimateMenu,
	UI_FadeMenu,
	UI_AnimateCombo,
	UI_AnimateTooltip,
	UI_FadeTooltip,
	UI_AnimateToolBox
    };

    // documented in qcursor.cpp
    enum CursorShape {
	ArrowCursor,
	UpArrowCursor,
	CrossCursor,
	WaitCursor,
	IbeamCursor,
	SizeVerCursor,
	SizeHorCursor,
	SizeBDiagCursor,
	SizeFDiagCursor,
	SizeAllCursor,
	BlankCursor,
	SplitVCursor,
	SplitHCursor,
	PointingHandCursor,
	ForbiddenCursor,
	WhatsThisCursor,
	LastCursor	= WhatsThisCursor,
	BitmapCursor	= 24

#ifndef QT_NO_COMPAT
	,
	arrowCursor = ArrowCursor,
	upArrowCursor = UpArrowCursor,
	crossCursor = CrossCursor,
	waitCursor = WaitCursor,
	ibeamCursor = IbeamCursor,
	sizeVerCursor = SizeVerCursor,
	sizeHorCursor = SizeHorCursor,
	sizeBDiagCursor = SizeBDiagCursor,
	sizeFDiagCursor = SizeFDiagCursor,
	sizeAllCursor = SizeAllCursor,
	blankCursor = BlankCursor,
	splitVCursor = SplitVCursor,
	splitHCursor = SplitHCursor,
	pointingHandCursor = PointingHandCursor,
	forbiddenCursor = ForbiddenCursor,
	whatsThisCursor = WhatsThisCursor
#endif
    };

    enum TextFormat {
	PlainText,
	RichText,
	AutoText,
	LogText
    };

    // Documented in qtextedit.cpp
    enum AnchorAttribute {
	AnchorName,
	AnchorHref
    };

    // Documented in qmainwindow.cpp
    enum Dock {
	DockUnmanaged,
	DockTornOff,
	DockTop,
	DockBottom,
	DockRight,
	DockLeft,
	DockMinimized
#ifndef QT_NO_COMPAT
        ,
	Unmanaged = DockUnmanaged,
	TornOff = DockTornOff,
	Top = DockTop,
	Bottom = DockBottom,
	Right = DockRight,
	Left = DockLeft,
	Minimized = DockMinimized
#endif
    };
    // compatibility
    typedef Dock ToolBarDock;

    // documented in qdatetime.cpp
    enum DateFormat {
	TextDate,      // default Qt
	ISODate,       // ISO 8601
	LocalDate      // locale dependant
    };

    // documented in qdatetime.cpp
    enum TimeSpec {
	LocalTime,
	UTC
    };

    // documented in qdatetime.cpp
    enum Day {
	Monday = 1,
	Tuesday = 2,
	Wednesday = 3,
	Thursday = 4,
	Friday = 5,
	Saturday = 6,
	Sunday = 7
    };

#ifndef QT_NO_COMPAT
    enum BackgroundMode {
	FixedColor,
	FixedPixmap,
	NoBackground,
	PaletteForeground,
	PaletteButton,
	PaletteLight,
	PaletteMidlight,
	PaletteDark,
	PaletteMid,
	PaletteText,
	PaletteBrightText,
	PaletteBase,
	PaletteBackground,
	PaletteShadow,
	PaletteHighlight,
	PaletteHighlightedText,
	PaletteButtonText,
	PaletteLink,
	PaletteLinkVisited,
	X11ParentRelative
    };
#endif

    // Documented in qstring.cpp
    enum StringComparisonMode {
        CaseSensitive   = 0x00001, // 0 0001
        BeginsWith      = 0x00002, // 0 0010
        EndsWith        = 0x00004, // 0 0100
        Contains        = 0x00008, // 0 1000
        ExactMatch      = 0x00010  // 1 0000
    };
    typedef QFlags<StringComparisonMode> ComparisonFlags;

    // Documented in qtabwidget.cpp
    enum Corner {
	TopLeft     = 0x00000,
	TopRight    = 0x00001,
	BottomLeft  = 0x00002,
	BottomRight = 0x00003
    };

    // "handle" type for system objects. Documented as \internal in
    // qapplication.cpp
#if defined(Q_WS_MAC)
    typedef void * HANDLE;
#elif defined(Q_WS_WIN)
    typedef void *HANDLE;
#elif defined(Q_WS_X11)
    typedef unsigned long HANDLE;
#elif defined(Q_WS_QWS)
    typedef void * HANDLE;
#endif
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Qt::WidgetState);
Q_DECLARE_OPERATORS_FOR_FLAGS(Qt::WindowFlags);
Q_DECLARE_OPERATORS_FOR_FLAGS(Qt::StringComparisonMode);

class Q_EXPORT QInternal {
public:
    enum PaintDeviceFlags {
	UndefinedDevice = 0x00,
	Widget = 0x01,
	Pixmap = 0x02,
	Printer = 0x03,
	Picture = 0x04,
	System = 0x05,
	DeviceTypeMask = 0x0f,
	ExternalDevice = 0x10,
	// used to emulate some of the behaviour different between Qt2 and Qt3 (mainly for printing)
	CompatibilityMode = 0x20
    };
};

// MOC_SKIP_BEGIN
Q_TEMPLATE_EXTERN template class Q_EXPORT QFlags<Qt::WidgetState>;
Q_TEMPLATE_EXTERN template class Q_EXPORT QFlags<Qt::WindowFlags>;
// MOC_SKIP_END



#endif // QNAMESPACE_H
