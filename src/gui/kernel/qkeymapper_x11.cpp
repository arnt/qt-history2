/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qkeymapper_p.h"

#include "qdebug.h"
#include "qtextcodec.h"
#include "qwidget.h"

#include "qapplication_p.h"
#include "qevent_p.h"
#include "qt_x11_p.h"

#ifndef QT_NO_XKB
#  include <X11/XKBlib.h>
#endif

#define XK_MISCELLANY
#define XK_LATIN1
#define XK_KOREAN
#define XK_XKB_KEYS
#include <X11/keysymdef.h>

#ifndef QT_NO_XKB

// bring in the auto-generated xkbLayoutData
#include "qkeymapper_x11_p.cpp"

#include <ctype.h>

extern bool qt_sendSpontaneousEvent(QObject*, QEvent*); // qapplication_xxx.cpp

static void getLocaleAndDirection(QLocale *locale,
                                  Qt::LayoutDirection *direction,
                                  const QByteArray &layoutName,
                                  const QByteArray &variantName)
{
    int i = 0;
    while (xkbLayoutData[i].layout != 0) {
        if (layoutName == xkbLayoutData[i].layout && variantName == xkbLayoutData[i].variant) {
            *locale = QLocale(xkbLayoutData[i].language, xkbLayoutData[i].country);
            *direction = xkbLayoutData[i].direction;
            return;
        }
        ++i;
    }
    *locale = QLocale::c();
    *direction = Qt::LeftToRight;
}
#endif // QT_NO_XKB


// from qapplication_x11.cpp
extern uchar qt_alt_mask;
extern uchar qt_meta_mask;
extern uchar qt_super_mask;
extern uchar qt_hyper_mask;
extern uchar qt_mode_switch_mask;
uchar qt_num_lock_mask = 0;

#define SETMASK(sym, mask)                                              \
    do {                                                                \
        if (qt_alt_mask == 0                                            \
            && qt_meta_mask != mask                                     \
            && qt_super_mask != mask                                    \
            && qt_hyper_mask != mask                                    \
            && (sym == XK_Alt_L || sym == XK_Alt_R)) {                  \
            qt_alt_mask = mask;                                         \
        }                                                               \
        if (qt_meta_mask == 0                                           \
            && qt_alt_mask != mask                                      \
            && qt_super_mask != mask                                    \
            && qt_hyper_mask != mask                                    \
            && (sym == XK_Meta_L || sym == XK_Meta_R)) {                \
            qt_meta_mask = mask;                                        \
        }                                                               \
        if (qt_super_mask == 0                                          \
            && qt_alt_mask != mask                                      \
            && qt_meta_mask != mask                                     \
            && qt_hyper_mask != mask                                    \
            && (sym == XK_Super_L || sym == XK_Super_R)) {              \
            qt_super_mask = mask;                                       \
        }                                                               \
        if (qt_hyper_mask == 0                                          \
            && qt_alt_mask != mask                                      \
            && qt_meta_mask != mask                                     \
            && qt_super_mask != mask                                    \
            && (sym == XK_Hyper_L || sym == XK_Hyper_R)) {              \
            qt_hyper_mask = mask;                                       \
        }                                                               \
        if (qt_mode_switch_mask == 0                                    \
            && sym == XK_Mode_switch) {                                 \
            qt_mode_switch_mask = mask;                                 \
        }                                                               \
        if (qt_num_lock_mask == 0                                       \
            && sym == XK_Num_Lock) {                                    \
            qt_num_lock_mask = mask;                                    \
        }                                                               \
    } while(false)

// qt_XTranslateKey() is based on _XTranslateKey() taken from:

/* $Xorg: KeyBind.c,v 1.4 2001/02/09 02:03:34 xorgcvs Exp $ */

/*

Copyright 1985, 1987, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

*/
static int
qt_XTranslateKey(register QXCoreDesc *dpy,
                 KeyCode keycode,
                 register unsigned int modifiers,
                 unsigned int *modifiers_return,
                 KeySym *keysym_return)
{
    int per;
    register KeySym *syms;
    KeySym sym, lsym, usym;

    if (! dpy->keysyms)
	return 0;
    *modifiers_return = ((ShiftMask|LockMask)
			 | dpy->mode_switch | dpy->num_lock);
    if (((int)keycode < dpy->min_keycode) || ((int)keycode > dpy->max_keycode))
    {
	*keysym_return = NoSymbol;
	return 1;
    }
    per = dpy->keysyms_per_keycode;
    syms = &dpy->keysyms[(keycode - dpy->min_keycode) * per];
    while ((per > 2) && (syms[per - 1] == NoSymbol))
	per--;
    if ((per > 2) && (modifiers & dpy->mode_switch)) {
	syms += 2;
	per -= 2;
    }
    if ((modifiers & dpy->num_lock) &&
	(per > 1 && (IsKeypadKey(syms[1]) || IsPrivateKeypadKey(syms[1])))) {
	if ((modifiers & ShiftMask) ||
	    ((modifiers & LockMask) && (dpy->lock_meaning == XK_Shift_Lock)))
	    *keysym_return = syms[0];
	else
	    *keysym_return = syms[1];
    } else if (!(modifiers & ShiftMask) &&
	(!(modifiers & LockMask) || (dpy->lock_meaning == NoSymbol))) {
	if ((per == 1) || (syms[1] == NoSymbol))
	    XConvertCase(syms[0], keysym_return, &usym);
	else
	    *keysym_return = syms[0];
    } else if (!(modifiers & LockMask) ||
	       (dpy->lock_meaning != XK_Caps_Lock)) {
	if ((per == 1) || ((usym = syms[1]) == NoSymbol))
	    XConvertCase(syms[0], &lsym, &usym);
	*keysym_return = usym;
    } else {
	if ((per == 1) || ((sym = syms[1]) == NoSymbol))
	    sym = syms[0];
	XConvertCase(sym, &lsym, &usym);
	if (!(modifiers & ShiftMask) && (sym != syms[0]) &&
	    ((sym != usym) || (lsym == usym)))
	    XConvertCase(syms[0], &lsym, &usym);
	*keysym_return = usym;
    }
    if (*keysym_return == XK_VoidSymbol)
	*keysym_return = NoSymbol;
    return 1;
}




QKeyMapperPrivate::QKeyMapperPrivate()
    : keyboardInputDirection(Qt::LeftToRight), useXKB(false)
{
    memset(&coreDesc, 0, sizeof(coreDesc));

#ifndef QT_NO_XKB
    int opcode = -1;
    int xkbEventBase = -1;
    int xkbErrorBase = -1;
    int xkblibMajor = XkbMajorVersion;
    int xkblibMinor = XkbMinorVersion;
    if (XkbQueryExtension(X11->display, &opcode, &xkbEventBase, &xkbErrorBase, &xkblibMajor, &xkblibMinor))
        useXKB = true;
#endif

#if 0
    qDebug() << "useXKB =" << useXKB;
#endif
}

QKeyMapperPrivate::~QKeyMapperPrivate()
{
    if (coreDesc.keysyms)
        XFree(coreDesc.keysyms);
}

QList<int> QKeyMapperPrivate::possibleKeys(QKeyEvent *event)
{
#ifndef QT_NO_XKB
    if (useXKB)
        return possibleKeysXKB(event);
#endif
    return possibleKeysCore(event);
}

enum { MaxBits = sizeof(uint) * 8 };
static void translateKeySym(KeySym keysym, uint xmodifiers,
                            int &code, Qt::KeyboardModifiers &modifiers,
                            QString &text);

QList<int> QKeyMapperPrivate::possibleKeysXKB(QKeyEvent *event)
{
#ifndef QT_NO_XKB
    const int xkeycode = event->nativeScanCode();
    const uint xmodifiers = event->nativeModifiers();

    // translate key without any modifiers first
    KeySym baseKeySym;
    uint consumedModifiers;
    if (!XkbLookupKeySym(X11->display, xkeycode, 0, &consumedModifiers, &baseKeySym))
        return QList<int>();

    QList<int> result;

    // translate sym -> code
    QString text;
    Qt::KeyboardModifiers baseModifiers = 0;
    int baseCode = -1;
    translateKeySym(baseKeySym, xmodifiers, baseCode, baseModifiers, text);
    if (baseCode == -1)
        return QList<int>();

    if (baseCode && baseCode < 0xfffe)
        baseCode = QChar(baseCode).toUpper().unicode();
    result += (baseCode | baseModifiers);

    int pos1Bits[MaxBits];
    int num1Bits = 0;

    for (int i = 0; i < MaxBits; ++i) {
        if (consumedModifiers & (1 << i))
            pos1Bits[num1Bits++] = i;
    }

    const int numPerms = (1 << num1Bits);

    // translate the key again using each permutation of consumedModifiers
    for (int i = 1; i < numPerms; ++i) {
        uint val = 0;
        for (int j = 0; j < num1Bits; ++j) {
            if (i & (1 << j))
                val |= (1 << pos1Bits[j]);
        }

        if ((xmodifiers & val) != val)
            continue;

        KeySym sym;
        uint mods;
        if (!XkbLookupKeySym(X11->display, xkeycode, val, &mods, &sym))
            continue;

        // translate sym -> code
        Qt::KeyboardModifiers modifiers = 0;
        int code = -1;
        // mask out the modifiers needed to translate keycode
        translateKeySym(sym, xmodifiers & ~val, code, modifiers, text);
        if (code == -1)
            continue;

        if (code && code < 0xfffe)
            code = QChar(code).toUpper().unicode();
        if (code == baseCode)
            continue;

        result += (code | modifiers);
    }

#if 0
    qDebug() << "possibleKeysXKB()" << hex << result;
#endif
    return result;
#else
    Q_UNUSED(event);
    return QList<int>();
#endif // QT_NO_XKB
}

QList<int> QKeyMapperPrivate::possibleKeysCore(QKeyEvent *event)
{
    const int xkeycode = event->nativeScanCode();
    const uint xmodifiers = event->nativeModifiers();

    // translate key without any modifiers first
    KeySym baseKeySym;
    uint consumedModifiers;
    if (!qt_XTranslateKey(&coreDesc, xkeycode, 0, &consumedModifiers, &baseKeySym))
        return QList<int>();

    QList<int> result;

    // translate sym -> code
    QString text;
    Qt::KeyboardModifiers baseModifiers = 0;
    int baseCode = -1;
    translateKeySym(baseKeySym, xmodifiers, baseCode, baseModifiers, text);
    if (baseCode == -1)
        return QList<int>();

    if (baseCode && baseCode < 0xfffe)
        baseCode = QChar(baseCode).toUpper().unicode();
    result += (baseCode | baseModifiers);

    int pos1Bits[MaxBits];
    int num1Bits = 0;

    for (int i = 0; i < MaxBits; ++i) {
        if (consumedModifiers & (1 << i))
            pos1Bits[num1Bits++] = i;
    }

    const int numPerms = (1 << num1Bits);

    // translate the key again using each permutation of consumedModifiers
    for (int i = 1; i < numPerms; ++i) {
        uint val = 0;
        for (int j = 0; j < num1Bits; ++j) {
            if (i & (1 << j))
                val |= (1 << pos1Bits[j]);
        }

        if ((xmodifiers & val) != val)
            continue;

        KeySym sym;
        uint mods;
        if (!qt_XTranslateKey(&coreDesc, xkeycode, val, &mods, &sym))
            continue;

        // translate sym -> code
        Qt::KeyboardModifiers modifiers = 0;
        int code = -1;
        // mask out the modifiers needed to translate keycode
        translateKeySym(sym, xmodifiers & ~val, code, modifiers, text);
        if (code == -1)
            continue;

        if (code && code < 0xfffe)
            code = QChar(code).toUpper().unicode();
        if (code == baseCode)
            continue;

        result += (code | modifiers);
    }

#if 0
    qDebug() << "possibleKeysCore()" << hex << result;
#endif
    return result;
}

// for parsing the _XKB_RULES_NAMES property
enum {
    RulesFileIndex = 0,
    ModelIndex = 1,
    LayoutIndex = 2,
    VariantIndex = 3,
    OptionsIndex = 4
};

void QKeyMapperPrivate::clearMappings()
{
#ifndef QT_NO_XKB
    if (useXKB) {
        // try to determine the layout name and input direction by reading the _XKB_RULES_NAMES property off
        // the root window
        QByteArray layoutName;
        QByteArray variantName;

        Atom type = XNone;
        int format = 0;
        ulong nitems = 0;
        ulong bytesAfter = 0;
        uchar *data = 0;
        if (XGetWindowProperty(X11->display, RootWindow(X11->display, 0), ATOM(_XKB_RULES_NAMES), 0, 1024,
                               false, XA_STRING, &type, &format, &nitems, &bytesAfter, &data) == Success
            && type == XA_STRING && format == 8 && nitems > 2) {
            /*
              index 0 == rules file name
              index 1 == model name
              index 2 == layout name
              index 3 == variant name
              index 4 == options
            */
            char *names[5] = { 0, 0, 0, 0, 0 };
            char *p = reinterpret_cast<char *>(data), *end = p + nitems;
            int i = 0;
            do {
                names[i++] = p;
                p += qstrlen(p) + 1;
            } while (p < end);

            layoutName = QByteArray::fromRawData(names[2], qstrlen(names[2]));
            variantName = QByteArray::fromRawData(names[3], qstrlen(names[3]));
        }

        // ### ???
        // if (keyboardLayoutName.isEmpty())
        //     qWarning("Qt: unable to determine keyboard layout, please talk to qt-bugs@trolltech.com"); ?

        getLocaleAndDirection(&keyboardInputLocale,
                              &keyboardInputDirection,
                              layoutName,
                              variantName);

#if 0
        qDebug() << "keyboard input locale ="
                 << keyboardInputLocale.name()
                 << "direction ="
                 << keyboardInputDirection;
#endif

        if (data)
            XFree(data);
    } else
#endif // QT_NO_XKB
        {
            if (coreDesc.keysyms)
                XFree(coreDesc.keysyms);

            coreDesc.min_keycode = 8;
            coreDesc.max_keycode = 255;
            XDisplayKeycodes(X11->display, &coreDesc.min_keycode, &coreDesc.max_keycode);

            coreDesc.keysyms_per_keycode = 0;
            coreDesc.keysyms = XGetKeyboardMapping(X11->display,
                                                   coreDesc.min_keycode,
                                                   coreDesc.max_keycode - coreDesc.min_keycode,
                                                   &coreDesc.keysyms_per_keycode);

#if 0
            qDebug() << "min_keycode =" << coreDesc.min_keycode;
            qDebug() << "max_keycode =" << coreDesc.max_keycode;
            qDebug() << "keysyms_per_keycode =" << coreDesc.keysyms_per_keycode;
            qDebug() << "keysyms =" << coreDesc.keysyms;
#endif

            // ### cannot get/guess the locale with the core protocol
            keyboardInputLocale = QLocale::c();
            // ### could examine group 0 for RTL keys
            keyboardInputDirection = Qt::LeftToRight;
        }

    // set default modifier masks
    qt_alt_mask = Mod1Mask;
    qt_meta_mask = Mod4Mask;
    qt_super_mask = 0;
    qt_hyper_mask = 0;
    qt_mode_switch_mask = 0;

    // look at the modifier mapping, and get the correct masks for alt, meta, super, hyper, and mode_switch
#ifndef QT_NO_XKB
    if (useXKB) {
        XkbDescPtr xkbDesc = XkbGetMap(X11->display, XkbAllClientInfoMask, XkbUseCoreKbd);
        for (int i = xkbDesc->min_key_code; i < xkbDesc->max_key_code; ++i) {
            const uint mask = xkbDesc->map->modmap[i];
            if (mask == 0) {
                // key is not bound to a modifier
                continue;
            }

            for (int j = 0; j < XkbKeyGroupsWidth(xkbDesc, i); ++j) {
                KeySym keySym = XkbKeySym(xkbDesc, i, j);
                if (keySym == NoSymbol)
                    continue;
                SETMASK(keySym, mask);
            }
        }
        XkbFreeClientMap(xkbDesc, XkbAllClientInfoMask, true);
    } else
#endif // QT_NO_XKB
        {
            coreDesc.lock_meaning = NoSymbol;

            XModifierKeymap *map = XGetModifierMapping(X11->display);

            if (map) {
                int i, maskIndex = 0, mapIndex = 0;
                for (maskIndex = 0; maskIndex < 8; maskIndex++) {
                    for (i = 0; i < map->max_keypermod; i++) {
                        if (map->modifiermap[mapIndex]) {
                            KeySym sym;
                            int x = 0;
                            do {
                                sym = XKeycodeToKeysym(X11->display, map->modifiermap[mapIndex], x++);
                            } while (sym == NoSymbol && x < coreDesc.keysyms_per_keycode);
                            const uchar mask = 1 << maskIndex;
                            SETMASK(sym, mask);
                        }
                        mapIndex++;
                    }
                }

                // determine the meaning of the Lock modifier
                for (i = 0; i < map->max_keypermod; ++i) {
                    for (int x = 0; x < coreDesc.keysyms_per_keycode; ++x) {
                        KeySym sym = XKeycodeToKeysym(X11->display, map->modifiermap[LockMapIndex], x);
                        if (sym == XK_Caps_Lock || sym == XK_ISO_Lock) {
                            coreDesc.lock_meaning = XK_Caps_Lock;
                            break;
                        } else if (sym == XK_Shift_Lock) {
                            coreDesc.lock_meaning = XK_Shift_Lock;
                        }
                    }
                }

                XFreeModifiermap(map);
            }

            // for qt_XTranslateKey()
            coreDesc.num_lock = qt_num_lock_mask;
            coreDesc.mode_switch = qt_mode_switch_mask;

#if 0
            qDebug() << "lock_meaning =" << coreDesc.lock_meaning;
            qDebug() << "num_lock =" << coreDesc.num_lock;
            qDebug() << "mode_switch =" << coreDesc.mode_switch;
#endif
        }

    // if we don't have a meta key (or it's hidden behind alt), use super or hyper to generate
    // Qt::Key_Meta and Qt::MetaModifier, since most newer XFree86/Xorg installations map the Windows
    // key to Super
    if (qt_meta_mask == 0 || qt_meta_mask == qt_alt_mask) {
        // no meta keys... s,meta,super,
        qt_meta_mask = qt_super_mask;
        if (qt_meta_mask == 0 || qt_meta_mask == qt_alt_mask) {
            // no super keys either? guess we'll use hyper then
            qt_meta_mask = qt_hyper_mask;
        }
    }

#if 0
    qDebug() << "qt_alt_mask =" << hex << qt_alt_mask;
    qDebug() << "qt_meta_mask =" << hex << qt_meta_mask;
    qDebug() << "qt_super_mask =" << hex << qt_super_mask;
    qDebug() << "qt_hyper_mask =" << hex << qt_hyper_mask;
    qDebug() << "qt_mode_switch_mask =" << hex << qt_mode_switch_mask;
    qDebug() << "qt_num_lock_mask =" << hex << qt_num_lock_mask;
#endif
}

extern bool qt_sm_blockUserInput;

//
// Keyboard event translation
//

#ifndef XK_ISO_Left_Tab
#define XK_ISO_Left_Tab         0xFE20
#endif

#ifndef XK_dead_hook
#define XK_dead_hook            0xFE61
#endif

#ifndef XK_dead_horn
#define XK_dead_horn            0xFE62
#endif

#ifndef XK_Codeinput
#define XK_Codeinput            0xFF37
#endif

#ifndef XK_Kanji_Bangou
#define XK_Kanji_Bangou         0xFF37 /* same as codeinput */
#endif

// Fix old X libraries
#ifndef XK_KP_Home
#define XK_KP_Home              0xFF95
#endif
#ifndef XK_KP_Left
#define XK_KP_Left              0xFF96
#endif
#ifndef XK_KP_Up
#define XK_KP_Up                0xFF97
#endif
#ifndef XK_KP_Right
#define XK_KP_Right             0xFF98
#endif
#ifndef XK_KP_Down
#define XK_KP_Down              0xFF99
#endif
#ifndef XK_KP_Prior
#define XK_KP_Prior             0xFF9A
#endif
#ifndef XK_KP_Next
#define XK_KP_Next              0xFF9B
#endif
#ifndef XK_KP_End
#define XK_KP_End               0xFF9C
#endif
#ifndef XK_KP_Insert
#define XK_KP_Insert            0xFF9E
#endif
#ifndef XK_KP_Delete
#define XK_KP_Delete            0xFF9F
#endif

// the next lines are taken from XFree > 4.0 (X11/XF86keysyms.h), defining some special
// multimedia keys. They are included here as not every system has them.
#define XF86XK_Standby          0x1008FF10
#define XF86XK_AudioLowerVolume 0x1008FF11
#define XF86XK_AudioMute        0x1008FF12
#define XF86XK_AudioRaiseVolume 0x1008FF13
#define XF86XK_AudioPlay        0x1008FF14
#define XF86XK_AudioStop        0x1008FF15
#define XF86XK_AudioPrev        0x1008FF16
#define XF86XK_AudioNext        0x1008FF17
#define XF86XK_HomePage         0x1008FF18
#define XF86XK_Calculator       0x1008FF1D
#define XF86XK_Mail             0x1008FF19
#define XF86XK_Start            0x1008FF1A
#define XF86XK_Search           0x1008FF1B
#define XF86XK_AudioRecord      0x1008FF1C
#define XF86XK_Back             0x1008FF26
#define XF86XK_Forward          0x1008FF27
#define XF86XK_Stop             0x1008FF28
#define XF86XK_Refresh          0x1008FF29
#define XF86XK_Favorites        0x1008FF30
#define XF86XK_AudioPause       0x1008FF31
#define XF86XK_AudioMedia       0x1008FF32
#define XF86XK_MyComputer       0x1008FF33
#define XF86XK_OpenURL          0x1008FF38
#define XF86XK_Launch0          0x1008FF40
#define XF86XK_Launch1          0x1008FF41
#define XF86XK_Launch2          0x1008FF42
#define XF86XK_Launch3          0x1008FF43
#define XF86XK_Launch4          0x1008FF44
#define XF86XK_Launch5          0x1008FF45
#define XF86XK_Launch6          0x1008FF46
#define XF86XK_Launch7          0x1008FF47
#define XF86XK_Launch8          0x1008FF48
#define XF86XK_Launch9          0x1008FF49
#define XF86XK_LaunchA          0x1008FF4A
#define XF86XK_LaunchB          0x1008FF4B
#define XF86XK_LaunchC          0x1008FF4C
#define XF86XK_LaunchD          0x1008FF4D
#define XF86XK_LaunchE          0x1008FF4E
#define XF86XK_LaunchF          0x1008FF4F
// end of XF86keysyms.h



// keyboard mapping table
static const unsigned int KeyTbl[] = {

    // misc keys

    XK_Escape,                  Qt::Key_Escape,
    XK_Tab,                     Qt::Key_Tab,
    XK_ISO_Left_Tab,            Qt::Key_Backtab,
    XK_BackSpace,               Qt::Key_Backspace,
    XK_Return,                  Qt::Key_Return,
    XK_Insert,                  Qt::Key_Insert,
    XK_Delete,                  Qt::Key_Delete,
    XK_Clear,                   Qt::Key_Delete,
    XK_Pause,                   Qt::Key_Pause,
    XK_Print,                   Qt::Key_Print,
    0x1005FF60,                 Qt::Key_SysReq,         // hardcoded Sun SysReq
    0x1007ff00,                 Qt::Key_SysReq,         // hardcoded X386 SysReq

    // cursor movement

    XK_Home,                    Qt::Key_Home,
    XK_End,                     Qt::Key_End,
    XK_Left,                    Qt::Key_Left,
    XK_Up,                      Qt::Key_Up,
    XK_Right,                   Qt::Key_Right,
    XK_Down,                    Qt::Key_Down,
    XK_Prior,                   Qt::Key_PageUp,
    XK_Next,                    Qt::Key_PageDown,

    // modifiers

    XK_Shift_L,                 Qt::Key_Shift,
    XK_Shift_R,                 Qt::Key_Shift,
    XK_Shift_Lock,              Qt::Key_Shift,
    XK_Control_L,               Qt::Key_Control,
    XK_Control_R,               Qt::Key_Control,
    XK_Meta_L,                  Qt::Key_Meta,
    XK_Meta_R,                  Qt::Key_Meta,
    XK_Alt_L,                   Qt::Key_Alt,
    XK_Alt_R,                   Qt::Key_Alt,
    XK_Caps_Lock,               Qt::Key_CapsLock,
    XK_Num_Lock,                Qt::Key_NumLock,
    XK_Scroll_Lock,             Qt::Key_ScrollLock,
    XK_Super_L,                 Qt::Key_Super_L,
    XK_Super_R,                 Qt::Key_Super_R,
    XK_Menu,                    Qt::Key_Menu,
    XK_Hyper_L,                 Qt::Key_Hyper_L,
    XK_Hyper_R,                 Qt::Key_Hyper_R,
    XK_Help,                    Qt::Key_Help,
    0x1000FF74,                 Qt::Key_Backtab,        // hardcoded HP backtab
    0x1005FF10,                 Qt::Key_F11,            // hardcoded Sun F36 (labeled F11)
    0x1005FF11,                 Qt::Key_F12,            // hardcoded Sun F37 (labeled F12)

    // numeric and function keypad keys

    XK_KP_Space,                Qt::Key_Space,
    XK_KP_Tab,                  Qt::Key_Tab,
    XK_KP_Enter,                Qt::Key_Enter,
    //XK_KP_F1,                 Qt::Key_F1,
    //XK_KP_F2,                 Qt::Key_F2,
    //XK_KP_F3,                 Qt::Key_F3,
    //XK_KP_F4,                 Qt::Key_F4,
    XK_KP_Home,                 Qt::Key_Home,
    XK_KP_Left,                 Qt::Key_Left,
    XK_KP_Up,                   Qt::Key_Up,
    XK_KP_Right,                Qt::Key_Right,
    XK_KP_Down,                 Qt::Key_Down,
    XK_KP_Prior,                Qt::Key_PageUp,
    XK_KP_Next,                 Qt::Key_PageDown,
    XK_KP_End,                  Qt::Key_End,
    XK_KP_Begin,                Qt::Key_Clear,
    XK_KP_Insert,               Qt::Key_Insert,
    XK_KP_Delete,               Qt::Key_Delete,
    XK_KP_Equal,                Qt::Key_Equal,
    XK_KP_Multiply,             Qt::Key_Asterisk,
    XK_KP_Add,                  Qt::Key_Plus,
    XK_KP_Separator,            Qt::Key_Comma,
    XK_KP_Subtract,             Qt::Key_Minus,
    XK_KP_Decimal,              Qt::Key_Period,
    XK_KP_Divide,               Qt::Key_Slash,

    // International input method support keys

    // International & multi-key character composition
    XK_ISO_Level3_Shift,        Qt::Key_AltGr,
    XK_Multi_key,		Qt::Key_Multi_key,
    XK_Codeinput,		Qt::Key_Codeinput,
    XK_SingleCandidate,		Qt::Key_SingleCandidate,
    XK_MultipleCandidate,	Qt::Key_MultipleCandidate,
    XK_PreviousCandidate,	Qt::Key_PreviousCandidate,

    // Misc Functions
    XK_Mode_switch,		Qt::Key_Mode_switch,
    XK_script_switch,		Qt::Key_Mode_switch,

    // Japanese keyboard support
    XK_Kanji,			Qt::Key_Kanji,
    XK_Muhenkan,		Qt::Key_Muhenkan,
    //XK_Henkan_Mode,		Qt::Key_Henkan_Mode,
    XK_Henkan_Mode,		Qt::Key_Henkan,
    XK_Henkan,			Qt::Key_Henkan,
    XK_Romaji,			Qt::Key_Romaji,
    XK_Hiragana,		Qt::Key_Hiragana,
    XK_Katakana,		Qt::Key_Katakana,
    XK_Hiragana_Katakana,	Qt::Key_Hiragana_Katakana,
    XK_Zenkaku,			Qt::Key_Zenkaku,
    XK_Hankaku,			Qt::Key_Hankaku,
    XK_Zenkaku_Hankaku,		Qt::Key_Zenkaku_Hankaku,
    XK_Touroku,			Qt::Key_Touroku,
    XK_Massyo,			Qt::Key_Massyo,
    XK_Kana_Lock,		Qt::Key_Kana_Lock,
    XK_Kana_Shift,		Qt::Key_Kana_Shift,
    XK_Eisu_Shift,		Qt::Key_Eisu_Shift,
    XK_Eisu_toggle,		Qt::Key_Eisu_toggle,
    //XK_Kanji_Bangou,		Qt::Key_Kanji_Bangou,
    //XK_Zen_Koho,		Qt::Key_Zen_Koho,
    //XK_Mae_Koho,		Qt::Key_Mae_Koho,
    XK_Kanji_Bangou,		Qt::Key_Codeinput,
    XK_Zen_Koho,		Qt::Key_MultipleCandidate,
    XK_Mae_Koho,		Qt::Key_PreviousCandidate,

#ifdef XK_KOREAN
    // Korean keyboard support
    XK_Hangul,			Qt::Key_Hangul,
    XK_Hangul_Start,		Qt::Key_Hangul_Start,
    XK_Hangul_End,		Qt::Key_Hangul_End,
    XK_Hangul_Hanja,		Qt::Key_Hangul_Hanja,
    XK_Hangul_Jamo,		Qt::Key_Hangul_Jamo,
    XK_Hangul_Romaja,		Qt::Key_Hangul_Romaja,
    //XK_Hangul_Codeinput,	Qt::Key_Hangul_Codeinput,
    XK_Hangul_Codeinput,	Qt::Key_Codeinput,
    XK_Hangul_Jeonja,		Qt::Key_Hangul_Jeonja,
    XK_Hangul_Banja,		Qt::Key_Hangul_Banja,
    XK_Hangul_PreHanja,		Qt::Key_Hangul_PreHanja,
    XK_Hangul_PostHanja,	Qt::Key_Hangul_PostHanja,
    //XK_Hangul_SingleCandidate,Qt::Key_Hangul_SingleCandidate,
    //XK_Hangul_MultipleCandidate,Qt::Key_Hangul_MultipleCandidate,
    //XK_Hangul_PreviousCandidate,Qt::Key_Hangul_PreviousCandidate,
    XK_Hangul_SingleCandidate,	Qt::Key_SingleCandidate,
    XK_Hangul_MultipleCandidate,Qt::Key_MultipleCandidate,
    XK_Hangul_PreviousCandidate,Qt::Key_PreviousCandidate,
    XK_Hangul_Special,		Qt::Key_Hangul_Special,
    //XK_Hangul_switch,		Qt::Key_Hangul_switch,
    XK_Hangul_switch,		Qt::Key_Mode_switch,
#endif  // XK_KOREAN

    // dead keys
    XK_dead_grave,              Qt::Key_Dead_Grave,
    XK_dead_acute,              Qt::Key_Dead_Acute,
    XK_dead_circumflex,         Qt::Key_Dead_Circumflex,
    XK_dead_tilde,              Qt::Key_Dead_Tilde,
    XK_dead_macron,             Qt::Key_Dead_Macron,
    XK_dead_breve,              Qt::Key_Dead_Breve,
    XK_dead_abovedot,           Qt::Key_Dead_Abovedot,
    XK_dead_diaeresis,          Qt::Key_Dead_Diaeresis,
    XK_dead_abovering,          Qt::Key_Dead_Abovering,
    XK_dead_doubleacute,        Qt::Key_Dead_Doubleacute,
    XK_dead_caron,              Qt::Key_Dead_Caron,
    XK_dead_cedilla,            Qt::Key_Dead_Cedilla,
    XK_dead_ogonek,             Qt::Key_Dead_Ogonek,
    XK_dead_iota,               Qt::Key_Dead_Iota,
    XK_dead_voiced_sound,       Qt::Key_Dead_Voiced_Sound,
    XK_dead_semivoiced_sound,   Qt::Key_Dead_Semivoiced_Sound,
    XK_dead_belowdot,           Qt::Key_Dead_Belowdot,
    XK_dead_hook,               Qt::Key_Dead_Hook,
    XK_dead_horn,               Qt::Key_Dead_Horn,

    // Special multimedia keys
    // currently only tested with MS internet keyboard

    // browsing keys
    XF86XK_Back,                Qt::Key_Back,
    XF86XK_Forward,             Qt::Key_Forward,
    XF86XK_Stop,                Qt::Key_Stop,
    XF86XK_Refresh,             Qt::Key_Refresh,
    XF86XK_Favorites,           Qt::Key_Favorites,
    XF86XK_AudioMedia,          Qt::Key_LaunchMedia,
    XF86XK_OpenURL,             Qt::Key_OpenUrl,
    XF86XK_HomePage,            Qt::Key_HomePage,
    XF86XK_Search,              Qt::Key_Search,

    // media keys
    XF86XK_AudioLowerVolume,    Qt::Key_VolumeDown,
    XF86XK_AudioMute,           Qt::Key_VolumeMute,
    XF86XK_AudioRaiseVolume,    Qt::Key_VolumeUp,
    XF86XK_AudioPlay,           Qt::Key_MediaPlay,
    XF86XK_AudioStop,           Qt::Key_MediaStop,
    XF86XK_AudioPrev,           Qt::Key_MediaPrevious,
    XF86XK_AudioNext,           Qt::Key_MediaNext,
    XF86XK_AudioRecord,         Qt::Key_MediaRecord,

    // launch keys
    XF86XK_Mail,                Qt::Key_LaunchMail,
    XF86XK_MyComputer,          Qt::Key_Launch0,
    XF86XK_Calculator,          Qt::Key_Launch1,
    XF86XK_Standby,             Qt::Key_Standby,

    XF86XK_Launch0,             Qt::Key_Launch2,
    XF86XK_Launch1,             Qt::Key_Launch3,
    XF86XK_Launch2,             Qt::Key_Launch4,
    XF86XK_Launch3,             Qt::Key_Launch5,
    XF86XK_Launch4,             Qt::Key_Launch6,
    XF86XK_Launch5,             Qt::Key_Launch7,
    XF86XK_Launch6,             Qt::Key_Launch8,
    XF86XK_Launch7,             Qt::Key_Launch9,
    XF86XK_Launch8,             Qt::Key_LaunchA,
    XF86XK_Launch9,             Qt::Key_LaunchB,
    XF86XK_LaunchA,             Qt::Key_LaunchC,
    XF86XK_LaunchB,             Qt::Key_LaunchD,
    XF86XK_LaunchC,             Qt::Key_LaunchE,
    XF86XK_LaunchD,             Qt::Key_LaunchF,

    0,                          0
};

static int translateKeySym(uint key)
{
    int code = -1;
    int i = 0;                                // any other keys
    while (KeyTbl[i]) {
        if (key == KeyTbl[i]) {
            code = (int)KeyTbl[i+1];
            break;
        }
        i += 2;
    }
    if (qt_meta_mask) {
        // translate Super/Hyper keys to Meta if we're using them as the MetaModifier
        if (qt_meta_mask == qt_super_mask && (code == Qt::Key_Super_L || code == Qt::Key_Super_R)) {
            code = Qt::Key_Meta;
        } else if (qt_meta_mask == qt_hyper_mask && (code == Qt::Key_Hyper_L || code == Qt::Key_Hyper_R)) {
            code = Qt::Key_Meta;
        }
    }
    return code;
}

static void translateKeySym(KeySym keysym, uint xmodifiers,
                            int &code, Qt::KeyboardModifiers &modifiers,
                            QString &text)
{
    modifiers = X11->translateModifiers(xmodifiers);

    // Commentary in X11/keysymdef says that X codes match ASCII, so it
    // is safe to use the locale functions to process X codes in ISO8859-1.
    //
    // This is mainly for compatibility - applications should not use the
    // Qt keycodes between 128 and 255, but should rather use the
    // QKeyEvent::text().
    //
    extern QTextCodec *qt_input_mapper; // from qapplication_x11.cpp
    if (keysym < 128 || (keysym < 256 && (!qt_input_mapper || qt_input_mapper->mibEnum()==4))) {
        // upper-case key, if known
        code = isprint((int)keysym) ? toupper((int)keysym) : 0;
    } else if (keysym >= XK_F1 && keysym <= XK_F35) {
        // function keys
        code = Qt::Key_F1 + ((int)keysym - XK_F1);
    } else if (keysym >= XK_KP_Space && keysym <= XK_KP_9) {
        if (keysym >= XK_KP_0) {
            // numeric keypad keys
            code = Qt::Key_0 + ((int)keysym - XK_KP_0);
        } else {
            code = translateKeySym(keysym);
        }
        modifiers |= Qt::KeypadModifier;
    } else if (text.length() == 1 && text.unicode()->unicode() > 0x1f && text.unicode()->unicode() != 0x7f && !(keysym >= XK_dead_grave && keysym <= XK_dead_horn)) {
        code = text.unicode()->toUpper().unicode();
    } else {
        // any other keys
        code = translateKeySym(keysym);

        if (code == Qt::Key_Tab && (modifiers & Qt::ShiftModifier)) {
            // map shift+tab to shift+backtab, QShortcutMap knows about it
            // and will handle it.
            code = Qt::Key_Backtab;
            text = QString();
        }
    }
}

#if !defined(QT_NO_XIM)
static const unsigned short katakanaKeysymsToUnicode[] = {
    0x0000, 0x3002, 0x300C, 0x300D, 0x3001, 0x30FB, 0x30F2, 0x30A1,
    0x30A3, 0x30A5, 0x30A7, 0x30A9, 0x30E3, 0x30E5, 0x30E7, 0x30C3,
    0x30FC, 0x30A2, 0x30A4, 0x30A6, 0x30A8, 0x30AA, 0x30AB, 0x30AD,
    0x30AF, 0x30B1, 0x30B3, 0x30B5, 0x30B7, 0x30B9, 0x30BB, 0x30BD,
    0x30BF, 0x30C1, 0x30C4, 0x30C6, 0x30C8, 0x30CA, 0x30CB, 0x30CC,
    0x30CD, 0x30CE, 0x30CF, 0x30D2, 0x30D5, 0x30D8, 0x30DB, 0x30DE,
    0x30DF, 0x30E0, 0x30E1, 0x30E2, 0x30E4, 0x30E6, 0x30E8, 0x30E9,
    0x30EA, 0x30EB, 0x30EC, 0x30ED, 0x30EF, 0x30F3, 0x309B, 0x309C
};

static const unsigned short cyrillicKeysymsToUnicode[] = {
    0x0000, 0x0452, 0x0453, 0x0451, 0x0454, 0x0455, 0x0456, 0x0457,
    0x0458, 0x0459, 0x045a, 0x045b, 0x045c, 0x0000, 0x045e, 0x045f,
    0x2116, 0x0402, 0x0403, 0x0401, 0x0404, 0x0405, 0x0406, 0x0407,
    0x0408, 0x0409, 0x040a, 0x040b, 0x040c, 0x0000, 0x040e, 0x040f,
    0x044e, 0x0430, 0x0431, 0x0446, 0x0434, 0x0435, 0x0444, 0x0433,
    0x0445, 0x0438, 0x0439, 0x043a, 0x043b, 0x043c, 0x043d, 0x043e,
    0x043f, 0x044f, 0x0440, 0x0441, 0x0442, 0x0443, 0x0436, 0x0432,
    0x044c, 0x044b, 0x0437, 0x0448, 0x044d, 0x0449, 0x0447, 0x044a,
    0x042e, 0x0410, 0x0411, 0x0426, 0x0414, 0x0415, 0x0424, 0x0413,
    0x0425, 0x0418, 0x0419, 0x041a, 0x041b, 0x041c, 0x041d, 0x041e,
    0x041f, 0x042f, 0x0420, 0x0421, 0x0422, 0x0423, 0x0416, 0x0412,
    0x042c, 0x042b, 0x0417, 0x0428, 0x042d, 0x0429, 0x0427, 0x042a
};

static const unsigned short greekKeysymsToUnicode[] = {
    0x0000, 0x0386, 0x0388, 0x0389, 0x038a, 0x03aa, 0x0000, 0x038c,
    0x038e, 0x03ab, 0x0000, 0x038f, 0x0000, 0x0000, 0x0385, 0x2015,
    0x0000, 0x03ac, 0x03ad, 0x03ae, 0x03af, 0x03ca, 0x0390, 0x03cc,
    0x03cd, 0x03cb, 0x03b0, 0x03ce, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397,
    0x0398, 0x0399, 0x039a, 0x039b, 0x039c, 0x039d, 0x039e, 0x039f,
    0x03a0, 0x03a1, 0x03a3, 0x0000, 0x03a4, 0x03a5, 0x03a6, 0x03a7,
    0x03a8, 0x03a9, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x03b1, 0x03b2, 0x03b3, 0x03b4, 0x03b5, 0x03b6, 0x03b7,
    0x03b8, 0x03b9, 0x03ba, 0x03bb, 0x03bc, 0x03bd, 0x03be, 0x03bf,
    0x03c0, 0x03c1, 0x03c3, 0x03c2, 0x03c4, 0x03c5, 0x03c6, 0x03c7,
    0x03c8, 0x03c9, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

static const unsigned short technicalKeysymsToUnicode[] = {
    0x0000, 0x23B7, 0x250C, 0x2500, 0x2320, 0x2321, 0x2502, 0x23A1,
    0x23A3, 0x23A4, 0x23A6, 0x239B, 0x239D, 0x239E, 0x23A0, 0x23A8,
    0x23AC, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x2264, 0x2260, 0x2265, 0x222B,
    0x2234, 0x221D, 0x221E, 0x0000, 0x0000, 0x2207, 0x0000, 0x0000,
    0x223C, 0x2243, 0x0000, 0x0000, 0x0000, 0x21D4, 0x21D2, 0x2261,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x221A, 0x0000,
    0x0000, 0x0000, 0x2282, 0x2283, 0x2229, 0x222A, 0x2227, 0x2228,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x2202,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0192, 0x0000,
    0x0000, 0x0000, 0x0000, 0x2190, 0x2191, 0x2192, 0x2193, 0x0000
};

static const unsigned short specialKeysymsToUnicode[] = {
    0x25C6, 0x2592, 0x2409, 0x240C, 0x240D, 0x240A, 0x0000, 0x0000,
    0x2424, 0x240B, 0x2518, 0x2510, 0x250C, 0x2514, 0x253C, 0x23BA,
    0x23BB, 0x2500, 0x23BC, 0x23BD, 0x251C, 0x2524, 0x2534, 0x252C,
    0x2502, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

static const unsigned short publishingKeysymsToUnicode[] = {
    0x0000, 0x2003, 0x2002, 0x2004, 0x2005, 0x2007, 0x2008, 0x2009,
    0x200a, 0x2014, 0x2013, 0x0000, 0x0000, 0x0000, 0x2026, 0x2025,
    0x2153, 0x2154, 0x2155, 0x2156, 0x2157, 0x2158, 0x2159, 0x215a,
    0x2105, 0x0000, 0x0000, 0x2012, 0x2329, 0x0000, 0x232a, 0x0000,
    0x0000, 0x0000, 0x0000, 0x215b, 0x215c, 0x215d, 0x215e, 0x0000,
    0x0000, 0x2122, 0x2613, 0x0000, 0x25c1, 0x25b7, 0x25cb, 0x25af,
    0x2018, 0x2019, 0x201c, 0x201d, 0x211e, 0x0000, 0x2032, 0x2033,
    0x0000, 0x271d, 0x0000, 0x25ac, 0x25c0, 0x25b6, 0x25cf, 0x25ae,
    0x25e6, 0x25ab, 0x25ad, 0x25b3, 0x25bd, 0x2606, 0x2022, 0x25aa,
    0x25b2, 0x25bc, 0x261c, 0x261e, 0x2663, 0x2666, 0x2665, 0x0000,
    0x2720, 0x2020, 0x2021, 0x2713, 0x2717, 0x266f, 0x266d, 0x2642,
    0x2640, 0x260e, 0x2315, 0x2117, 0x2038, 0x201a, 0x201e, 0x0000
};

static const unsigned short aplKeysymsToUnicode[] = {
    0x0000, 0x0000, 0x0000, 0x003c, 0x0000, 0x0000, 0x003e, 0x0000,
    0x2228, 0x2227, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x00af, 0x0000, 0x22a5, 0x2229, 0x230a, 0x0000, 0x005f, 0x0000,
    0x0000, 0x0000, 0x2218, 0x0000, 0x2395, 0x0000, 0x22a4, 0x25cb,
    0x0000, 0x0000, 0x0000, 0x2308, 0x0000, 0x0000, 0x222a, 0x0000,
    0x2283, 0x0000, 0x2282, 0x0000, 0x22a2, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x22a3, 0x0000, 0x0000, 0x0000
};

static const unsigned short koreanKeysymsToUnicode[] = {
    0x0000, 0x3131, 0x3132, 0x3133, 0x3134, 0x3135, 0x3136, 0x3137,
    0x3138, 0x3139, 0x313a, 0x313b, 0x313c, 0x313d, 0x313e, 0x313f,
    0x3140, 0x3141, 0x3142, 0x3143, 0x3144, 0x3145, 0x3146, 0x3147,
    0x3148, 0x3149, 0x314a, 0x314b, 0x314c, 0x314d, 0x314e, 0x314f,
    0x3150, 0x3151, 0x3152, 0x3153, 0x3154, 0x3155, 0x3156, 0x3157,
    0x3158, 0x3159, 0x315a, 0x315b, 0x315c, 0x315d, 0x315e, 0x315f,
    0x3160, 0x3161, 0x3162, 0x3163, 0x11a8, 0x11a9, 0x11aa, 0x11ab,
    0x11ac, 0x11ad, 0x11ae, 0x11af, 0x11b0, 0x11b1, 0x11b2, 0x11b3,
    0x11b4, 0x11b5, 0x11b6, 0x11b7, 0x11b8, 0x11b9, 0x11ba, 0x11bb,
    0x11bc, 0x11bd, 0x11be, 0x11bf, 0x11c0, 0x11c1, 0x11c2, 0x316d,
    0x3171, 0x3178, 0x317f, 0x3181, 0x3184, 0x3186, 0x318d, 0x318e,
    0x11eb, 0x11f0, 0x11f9, 0x0000, 0x0000, 0x0000, 0x0000, 0x20a9
};

static QChar keysymToUnicode(unsigned char byte3, unsigned char byte4)
{
    switch (byte3) {
    case 0x04:
        // katakana
        if (byte4 > 0xa0 && byte4 < 0xe0)
            return QChar(katakanaKeysymsToUnicode[byte4 - 0xa0]);
        else if (byte4 == 0x7e)
            return QChar(0x203e); // Overline
        break;
    case 0x06:
        // russian, use lookup table
        if (byte4 > 0xa0)
            return QChar(cyrillicKeysymsToUnicode[byte4 - 0xa0]);
        break;
    case 0x07:
        // greek
        if (byte4 > 0xa0)
            return QChar(greekKeysymsToUnicode[byte4 - 0xa0]);
        break;
    case 0x08:
        // technical
        if (byte4 > 0xa0)
            return QChar(technicalKeysymsToUnicode[byte4 - 0xa0]);
        break;
    case 0x09:
        // special
        if (byte4 >= 0xe0)
            return QChar(specialKeysymsToUnicode[byte4 - 0xe0]);
        break;
    case 0x0a:
        // publishing
        if (byte4 > 0xa0)
            return QChar(publishingKeysymsToUnicode[byte4 - 0xa0]);
        break;
    case 0x0b:
        // APL
        if (byte4 > 0xa0)
            return QChar(aplKeysymsToUnicode[byte4 - 0xa0]);
        break;
    case 0x0e:
        // Korean
        if (byte4 > 0xa0)
            return QChar(koreanKeysymsToUnicode[byte4 - 0xa0]);
        break;
    default:
        break;
    }
    return QChar(0x0);
}
#endif

bool QKeyMapperPrivate::translateKeyEventInternal(QWidget *keyWidget,
                                                  const XEvent *event,
                                                  KeySym &keysym,
                                                  int& count,
                                                  QString& text,
                                                  Qt::KeyboardModifiers &modifiers,
                                                  int& code,
                                                  QEvent::Type &type,
                                                  bool statefulTranslation)
{
    XKeyEvent xkeyevent = event->xkey;
    int keycode = event->xkey.keycode;
    // save the modifier state, we will use the keystate uint later by passing
    // it to translateButtonState
    uint keystate = event->xkey.state;

    type = (event->type == XKeyPress) ? QEvent::KeyPress : QEvent::KeyRelease;

    static int directionKeyEvent = 0;
    static unsigned int lastWinId = 0;
    extern bool qt_use_rtl_extensions; // from qapplication_x11.cpp

    // translate pending direction change
    if (statefulTranslation && qt_use_rtl_extensions && type == QEvent::KeyRelease) {
        if (directionKeyEvent == Qt::Key_Direction_R || directionKeyEvent == Qt::Key_Direction_L) {
            type = QEvent::KeyPress;
            code = directionKeyEvent;
            text = QString();
            directionKeyEvent = 0;
	    lastWinId = 0;
            return true;
        } else {
            directionKeyEvent = 0;
	    lastWinId = 0;
        }
    }

    // some XmbLookupString implementations don't return buffer overflow correctly,
    // so we increase the input buffer to allow for long strings...
    // 256 chars * 2 bytes + 1 null-term == 513 bytes
    QByteArray chars;
    chars.resize(513);
    QChar converted;

    count = XLookupString(&xkeyevent, chars.data(), chars.size(), &keysym, 0);
    if (count && !keycode) {
        extern int qt_ximComposingKeycode; // from qapplication_x11.cpp
        keycode = qt_ximComposingKeycode;
        qt_ximComposingKeycode = 0;
    }

    // all keysyms smaller than 0xff00 are actally keys that can be mapped to unicode chars

    extern QTextCodec *qt_input_mapper; // from qapplication_x11.cpp
    QTextCodec *mapper = qt_input_mapper;

    if (count == 0 && keysym < 0xff00) {
        unsigned char byte3 = (unsigned char)(keysym >> 8);
        int mib = -1;
        switch(byte3) {
        case 0: // Latin 1
        case 1: // Latin 2
        case 2: //latin 3
        case 3: // latin4
            mib = byte3 + 4; break;
        case 5: // arabic
            mib = 82; break;
        case 12: // Hebrew
            mib = 85; break;
        case 13: // Thai
            mib = 2259; break;
        case 4: // kana
        case 6: // cyrillic
        case 7: // greek
        case 8: // technical, no mapping here at the moment
        case 9: // Special
        case 10: // Publishing
        case 11: // APL
        case 14: // Korean, no mapping
            mib = -1; // manual conversion
            mapper = 0;
#if !defined(QT_NO_XIM)
            converted = keysymToUnicode(byte3, keysym & 0xff);
#endif
        case 0x20:
            // currency symbols
            if (keysym >= 0x20a0 && keysym <= 0x20ac) {
                mib = -1; // manual conversion
                mapper = 0;
                converted = (uint)keysym;
            }
            break;
        default:
            break;
        }
        if (mib != -1) {
            mapper = QTextCodec::codecForMib(mib);
            chars[0] = (unsigned char) (keysym & 0xff); // get only the fourth bit for conversion later
            count++;
        }
    } else if (keysym >= 0x1000000 && keysym <= 0x100ffff) {
        converted = (ushort) (keysym - 0x1000000);
        mapper = 0;
    }
    if (count < (int)chars.size()-1)
        chars[count] = '\0';

    // convert chars (8bit) to text (unicode).
    if (mapper)
        text = mapper->toUnicode(chars.data(), count, 0);
    else if (!mapper && converted.unicode() != 0x0)
        text = converted;
    else
        text = QString::fromLatin1(chars);

    // translate the keysym + xmodifiers to Qt::Key_* + Qt::KeyboardModifiers
    translateKeySym(keysym, keystate, code, modifiers, text);

    // Watch for keypresses and if its a key belonging to the Ctrl-Shift
    // direction-changing accel, remember it.
    // We keep track of those keys instead of using the event's state
    // (to figure out whether the Ctrl modifier is held while Shift is pressed,
    // or Shift is held while Ctrl is pressed) since the 'state' doesn't tell
    // us whether the modifier held is Left or Right.
    if (statefulTranslation && qt_use_rtl_extensions && type == QEvent::KeyPress) {
        if (keysym == XK_Control_L || keysym == XK_Control_R
            || keysym == XK_Shift_L || keysym == XK_Shift_R) {
	    if (!directionKeyEvent) {
		directionKeyEvent = keysym;
		// This code exists in order to check that
		// the event is occurred in the same widget.
		lastWinId = keyWidget->winId();
	    }
        } else {
            // this can no longer be a direction-changing accel.
            // if any other key was pressed.
            directionKeyEvent = Qt::Key_Space;
        }

        if (directionKeyEvent && lastWinId == keyWidget->winId()) {
            if (keysym == XK_Shift_L && directionKeyEvent == XK_Control_L ||
                keysym == XK_Control_L && directionKeyEvent == XK_Shift_L) {
                directionKeyEvent = Qt::Key_Direction_L;
            } else if (keysym == XK_Shift_R && directionKeyEvent == XK_Control_R ||
                       keysym == XK_Control_R && directionKeyEvent == XK_Shift_R) {
                directionKeyEvent = Qt::Key_Direction_R;
            }
        } else if (directionKeyEvent == Qt::Key_Direction_L
                   || directionKeyEvent == Qt::Key_Direction_R) {
            directionKeyEvent = Qt::Key_Space; // invalid
        }
    }

    return true;
}


struct qt_auto_repeat_data
{
    // match the window and keycode with timestamp delta of 10ms
    Window window;
    KeyCode keycode;
    Time timestamp;

    // queue scanner state
    bool release;
    bool error;
};

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

static Bool qt_keypress_scanner(Display *, XEvent *event, XPointer arg)
{
    if (event->type != XKeyPress && event->type != XKeyRelease)
        return false;

    qt_auto_repeat_data *data = (qt_auto_repeat_data *) arg;
    if (data->error ||
        event->xkey.window  != data->window ||
        event->xkey.keycode != data->keycode)
        return false;

    if (event->type == XKeyPress) {
        data->error = (! data->release || event->xkey.time - data->timestamp > 10);
        return (! data->error);
    }

    // must be XKeyRelease event
    if (data->release) {
        // found a second release
        data->error = true;
        return false;
    }

    // found a single release
    data->release = true;
    data->timestamp = event->xkey.time;

    return false;
}

static Bool qt_keyrelease_scanner(Display *, XEvent *event, XPointer arg)
{
    const qt_auto_repeat_data *data = (const qt_auto_repeat_data *) arg;
    return (event->type == XKeyRelease &&
            event->xkey.window  == data->window &&
            event->xkey.keycode == data->keycode);
}

#if defined(Q_C_CALLBACKS)
}
#endif

bool QKeyMapperPrivate::translateKeyEvent(QWidget *keyWidget, const XEvent *event, bool grab)
{
    int           code = -1;
    int           count = 0;
    Qt::KeyboardModifiers modifiers;

    if (qt_sm_blockUserInput) // block user interaction during session management
        return true;

    Display *dpy = X11->display;

    if (!keyWidget->isEnabled())
        return true;

    QEvent::Type type;
    bool    autor = false;
    QString text;

    KeySym keysym = 0;
    translateKeyEventInternal(keyWidget, event, keysym, count, text, modifiers, code, type);

    // was this the last auto-repeater?
    qt_auto_repeat_data auto_repeat_data;
    auto_repeat_data.window = event->xkey.window;
    auto_repeat_data.keycode = event->xkey.keycode;
    auto_repeat_data.timestamp = event->xkey.time;

    static uint curr_autorep = 0;
    if (event->type == XKeyPress) {
        if (curr_autorep == event->xkey.keycode) {
            autor = true;
            curr_autorep = 0;
        }
    } else {
        // look ahead for auto-repeat
        XEvent nextpress;

        auto_repeat_data.release = true;
        auto_repeat_data.error = false;
        if (XCheckIfEvent(dpy, &nextpress, &qt_keypress_scanner,
                          (XPointer) &auto_repeat_data)) {
            autor = true;

            // Put it back... we COULD send the event now and not need
            // the static curr_autorep variable.
            XPutBackEvent(dpy,&nextpress);
        }
        curr_autorep = autor ? event->xkey.keycode : 0;
    }

#if defined QT3_SUPPORT && !defined(QT_NO_SHORTCUT)
    // process accelerators before doing key compression
    if (type == QEvent::KeyPress && !grab
        && QApplicationPrivate::instance()->use_compat()) {
        // send accel events if the keyboard is not grabbed
        QKeyEventEx a(type, code, modifiers, text, autor, qMax(qMax(count,1), int(text.length())),
                      event->xkey.keycode, keysym, event->xkey.state);
        if (QApplicationPrivate::instance()->qt_tryAccelEvent(keyWidget, &a))
            return true;
    }
#endif

#ifndef QT_NO_IM
    QInputContext *qic = keyWidget->inputContext();
#endif

    // compress keys
    if (!text.isEmpty() && keyWidget->testAttribute(Qt::WA_KeyCompression) &&
#ifndef QT_NO_IM
        // Ordinary input methods require discrete key events to work
        // properly, so key compression has to be disabled when input
        // context exists.
        //
        // And further consideration, some complex input method
        // require all key press/release events discretely even if
        // the input method awares of key compression and compressed
        // keys are ordinary alphabets. For example, the uim project
        // is planning to implement "combinational shift" feature for
        // a Japanese input method, uim-skk. It will work as follows.
        //
        // 1. press "r"
        // 2. press "u"
        // 3. release both "r" and "u" in arbitrary order
        // 4. above key sequence generates "Ru"
        //
        // Of course further consideration about other participants
        // such as key repeat mechanism is required to implement such
        // feature.
        !qic &&
#endif // QT_NO_IM
        // do not compress keys if the key event we just got above matches
        // one of the key ranges used to compute stopCompression
        !((code >= Qt::Key_Escape && code <= Qt::Key_SysReq)
          || (code >= Qt::Key_Home && code <= Qt::Key_PageDown)
          || (code >= Qt::Key_Super_L && code <= Qt::Key_Direction_R)
          || (code == 0)
          || (text.length() == 1 && text.unicode()->unicode() == '\n'))) {
        // the widget wants key compression so it gets it

        // sync the event queue, this makes key compress work better
        XSync(dpy, false);

        for (;;) {
            XEvent        evRelease;
            XEvent        evPress;
            if (!XCheckTypedWindowEvent(dpy,event->xkey.window,
                                        XKeyRelease,&evRelease))
                break;
            if (!XCheckTypedWindowEvent(dpy,event->xkey.window,
                                        XKeyPress,&evPress)) {
                XPutBackEvent(dpy, &evRelease);
                break;
            }
            QString textIntern;
            int codeIntern = -1;
            int countIntern = 0;
            Qt::KeyboardModifiers modifiersIntern;
            QEvent::Type t;
            KeySym keySymIntern;
            translateKeyEventInternal(keyWidget, &evPress, keySymIntern, countIntern, textIntern,
                                      modifiersIntern, codeIntern, t);
            // use stopCompression to stop key compression for the following
            // key event ranges:
            bool stopCompression =
                // 1) misc keys
                (codeIntern >= Qt::Key_Escape && codeIntern <= Qt::Key_SysReq)
                // 2) cursor movement
                || (codeIntern >= Qt::Key_Home && codeIntern <= Qt::Key_PageDown)
                // 3) extra keys
                || (codeIntern >= Qt::Key_Super_L && codeIntern <= Qt::Key_Direction_R)
                // 4) something that a) doesn't translate to text or b) translates
                //    to newline text
                || (codeIntern == 0)
                || (textIntern.length() == 1 && textIntern.unicode()->unicode() == '\n');
            if (modifiersIntern == modifiers && !textIntern.isEmpty() && !stopCompression) {
                text += textIntern;
                count += countIntern;
            } else {
                XPutBackEvent(dpy, &evPress);
                XPutBackEvent(dpy, &evRelease);
                break;
            }
        }
    }

    // autorepeat compression makes sense for all widgets (Windows
    // does it automatically ....)
    if (event->type == XKeyPress && text.length() <= 1
#ifndef QT_NO_IM
        // input methods need discrete key events
        && !qic
#endif// QT_NO_IM
	) {
        XEvent dummy;

        for (;;) {
            auto_repeat_data.release = false;
            auto_repeat_data.error = false;
            if (! XCheckIfEvent(dpy, &dummy, &qt_keypress_scanner,
                                (XPointer) &auto_repeat_data))
                break;
            if (! XCheckIfEvent(dpy, &dummy, &qt_keyrelease_scanner,
                                (XPointer) &auto_repeat_data))
                break;

            count++;
            if (!text.isEmpty())
                text += text[0];
        }
    }

    if (text.length() == 1 && text.unicode()->unicode() == '\n') {
        code = Qt::Key_Return;
        text = QLatin1Char('\r');
    }

    return QKeyMapper::sendKeyEvent(keyWidget, grab, type, code, modifiers, text, autor,
                                    qMax(qMax(count,1), int(text.length())),
                                    event->xkey.keycode, keysym, event->xkey.state);
}

bool QKeyMapper::sendKeyEvent(QWidget *keyWidget, bool grab,
                              QEvent::Type type, int code, Qt::KeyboardModifiers modifiers,
                              const QString &text, bool autorepeat, int count,
                              quint32 nativeScanCode, quint32 nativeVirtualKey, quint32 nativeModifiers)
{
    // try the menukey first
    if (type == QEvent::KeyPress && code == Qt::Key_Menu) {
        QPoint pos = keyWidget->inputMethodQuery(Qt::ImMicroFocus).toRect().center();
        QContextMenuEvent e(QContextMenuEvent::Keyboard, pos, keyWidget->mapToGlobal(pos));
        qt_sendSpontaneousEvent(keyWidget, &e);
        if(e.isAccepted())
            return true;
    }

    Q_UNUSED(grab);
    QKeyEventEx e(type, code, modifiers, text, autorepeat, qMax(qMax(count,1), int(text.length())),
                  nativeScanCode, nativeVirtualKey, nativeModifiers);
    return qt_sendSpontaneousEvent(keyWidget, &e);
}
