/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qkeycode.h#3 $
**
** Definition of keyboard codes
**
** Author  : Haavard Nord
** Created : 931030
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QKEYCODE_H
#define QKEYCODE_H


const uint SHIFT = 0x2000;			// accelerator modifiers
const uint CTRL  = 0x4000;
const uint ALT   = 0x8000;


#define Key_Escape		0x1000		// misc keys
#define Key_Tab			0x1001
#define Key_Backtab		0x1002
#define Key_Backspace		0x1003
#define Key_Return		0x1004
#define Key_Enter		0x1005
#define Key_Insert		0x1006
#define Key_Delete		0x1007
#define Key_Pause		0x1008
#define Key_Print		0x1009
#define Key_SysReq		0x100a

#define Key_Home		0x1010		// cursor movement
#define Key_End			0x1011
#define Key_Left		0x1012
#define Key_Up			0x1013
#define Key_Right		0x1014
#define Key_Down		0x1015
#define Key_Prior		0x1016
#define Key_Next		0x1017

#define Key_Shift		0x1020		// modifiers
#define Key_Control		0x1021
#define Key_Meta		0x1022
#define Key_Alt			0x1023
#define Key_CapsLock		0x1024
#define Key_NumLock		0x1025
#define Key_ScrollLock		0x1026

#define Key_F1			0x1030		// function keys
#define Key_F2			0x1031
#define Key_F3			0x1032
#define Key_F4			0x1033
#define Key_F5			0x1034
#define Key_F6			0x1035
#define Key_F7			0x1036
#define Key_F8			0x1037
#define Key_F9			0x1038
#define Key_F10			0x1039
#define Key_F11			0x103a
#define Key_F12			0x103b
#define Key_F13			0x103c
#define Key_F14			0x103d
#define Key_F15			0x103e
#define Key_F16			0x103f
#define Key_F17			0x1040
#define Key_F18			0x1041
#define Key_F19			0x1042
#define Key_F20			0x1043
#define Key_F21			0x1044
#define Key_F22			0x1045
#define Key_F23			0x1046
#define Key_F24			0x1047

#define Key_Space		0x20		// Latin-1 codes (7 bit)
#define Key_Exclam		0x21
#define Key_QuoteDbl		0x22
#define Key_NumberSign		0x23
#define Key_Dollar		0x24
#define Key_Percent		0x25
#define Key_Ampersand		0x26
#define Key_Apostrophe		0x27
#define Key_ParenLeft		0x28
#define Key_ParenRight		0x29
#define Key_Asterisk		0x2a
#define Key_Plus		0x2b
#define Key_Comma		0x2c
#define Key_Minus		0x2d
#define Key_Period		0x2e
#define Key_Slash		0x2f
#define Key_0			0x30
#define Key_1			0x31
#define Key_2			0x32
#define Key_3			0x33
#define Key_4			0x34
#define Key_5			0x35
#define Key_6			0x36
#define Key_7			0x37
#define Key_8			0x38
#define Key_9			0x39
#define Key_Colon		0x3a
#define Key_Semicolon		0x3b
#define Key_Less		0x3c
#define Key_Equal		0x3d
#define Key_Greater		0x3e
#define Key_Question		0x3f
#define Key_At			0x40
#define Key_A			0x41
#define Key_B			0x42
#define Key_C			0x43
#define Key_D			0x44
#define Key_E			0x45
#define Key_F			0x46
#define Key_G			0x47
#define Key_H			0x48
#define Key_I			0x49
#define Key_J			0x4a
#define Key_K			0x4b
#define Key_L			0x4c
#define Key_M			0x4d
#define Key_N			0x4e
#define Key_O			0x4f
#define Key_P			0x50
#define Key_Q			0x51
#define Key_R			0x52
#define Key_S			0x53
#define Key_T			0x54
#define Key_U			0x55
#define Key_V			0x56
#define Key_W			0x57
#define Key_X			0x58
#define Key_Y			0x59
#define Key_Z			0x5a
#define Key_BracketLeft		0x5b
#define Key_Backslash		0x5c
#define Key_BracketRight	0x5d
#define Key_AsciiCircum		0x5e
#define Key_Underscore		0x5f
#define Key_QuoteLeft		0x60
#define Key_BraceLeft		0x7b
#define Key_Bar			0x7c
#define Key_BraceRight		0x7d
#define Key_AsciiTilde		0x7e


#endif // QKEYCODE_H
