/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <private/qt_mac_p.h>
#include <qdebug.h>
#include <qevent.h>
#include <private/qevent_p.h>
#include <qtextcodec.h>
#include <qapplication.h>
#include <qinputcontext.h>
#include <private/qkeymapper_p.h>

/*****************************************************************************
  QKeyMapper debug facilities
 *****************************************************************************/
//#define DEBUG_KEY_BINDINGS
//#define DEBUG_KEY_MAPS

/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/
bool qt_mac_eat_unicode_key = false;
extern bool qt_sendSpontaneousEvent(QObject *obj, QEvent *event); //qapplication_mac.cpp

Q_GUI_EXPORT void qt_mac_secure_keyboard(bool b)
{
    if(b) {
        SInt32 (*EnableSecureEventInput_ptr)() = EnableSecureEventInput; // workaround for gcc warning
        if (EnableSecureEventInput_ptr)
            (*EnableSecureEventInput_ptr)();
    } else {
        SInt32 (*DisableSecureEventInput_ptr)() = DisableSecureEventInput;
        if (DisableSecureEventInput_ptr)
            (*DisableSecureEventInput_ptr)();
    }
}

/*!
    \internal
    A Mac KeyboardLayoutItem has 8 possible states:
        1. Unmodified
        2. Shift
        3. Control
        4. Control + Shift
        5. Alt
        6. Alt + Shift
        7. Alt + Control
        8. Alt + Control + Shift
        9. Meta
        10. Meta + Shift
        11. Meta + Control
        12. Meta + Control + Shift
        13. Meta + Alt
        14. Meta + Alt + Shift
        15. Meta + Alt + Control
        16. Meta + Alt + Control + Shift
*/
struct KeyboardLayoutItem {
    bool dirty;
    quint32 qtKey[16]; // Can by any Qt::Key_<foo>, or unicode character
};

// Possible modifier states.
// NOTE: The order of these states match the order in QKeyMapperPrivate::updatePossibleKeyCodes()!
static const Qt::KeyboardModifiers ModsTbl[] = {
    Qt::NoModifier,                                             // 0
    Qt::ShiftModifier,                                          // 1
    Qt::ControlModifier,                                        // 2
    Qt::ControlModifier | Qt::ShiftModifier,                    // 3
    Qt::AltModifier,                                            // 4
    Qt::AltModifier | Qt::ShiftModifier,                        // 5
    Qt::AltModifier | Qt::ControlModifier,                      // 6
    Qt::AltModifier | Qt::ShiftModifier | Qt::ControlModifier,  // 7
    Qt::MetaModifier,                                           // 8
    Qt::MetaModifier | Qt::ShiftModifier,                       // 9
    Qt::MetaModifier | Qt::ControlModifier,                    // 10
    Qt::MetaModifier | Qt::ControlModifier | Qt::ShiftModifier,// 11
    Qt::MetaModifier | Qt::AltModifier,                        // 12
    Qt::MetaModifier | Qt::AltModifier | Qt::ShiftModifier,    // 13
    Qt::MetaModifier | Qt::AltModifier | Qt::ControlModifier,  // 14
    Qt::MetaModifier | Qt::AltModifier | Qt::ShiftModifier | Qt::ControlModifier,  // 15
};

/* key maps */
struct mac_enum_mapper
{
    int mac_code;
    int qt_code;
#if defined(DEBUG_KEY_BINDINGS)
#   define MAP_MAC_ENUM(x) x, #x
    const char *desc;
#else
#   define MAP_MAC_ENUM(x) x
#endif
};

//modifiers
static mac_enum_mapper modifier_symbols[] = {
    { shiftKey, MAP_MAC_ENUM(Qt::ShiftModifier) },
    { rightShiftKey, MAP_MAC_ENUM(Qt::ShiftModifier) },
    { controlKey, MAP_MAC_ENUM(Qt::MetaModifier) },
    { rightControlKey, MAP_MAC_ENUM(Qt::MetaModifier) },
    { cmdKey, MAP_MAC_ENUM(Qt::ControlModifier) },
    { optionKey, MAP_MAC_ENUM(Qt::AltModifier) },
    { rightOptionKey, MAP_MAC_ENUM(Qt::AltModifier) },
    { kEventKeyModifierNumLockMask, MAP_MAC_ENUM(Qt::KeypadModifier) },
    { 0, MAP_MAC_ENUM(0) }
};
Qt::KeyboardModifiers get_modifiers(int keys)
{
#ifdef DEBUG_KEY_BINDINGS
    qDebug("Qt: internal: **Mapping modifiers: %d (0x%04x)", keys, keys);
#endif
    Qt::KeyboardModifiers ret = Qt::NoModifier;
    for(int i = 0; modifier_symbols[i].qt_code; i++) {
        if(keys & modifier_symbols[i].mac_code) {
#ifdef DEBUG_KEY_BINDINGS
            qDebug("Qt: internal: got modifier: %s", modifier_symbols[i].desc);
#endif
            ret |= Qt::KeyboardModifier(modifier_symbols[i].qt_code);
        }
    }
    return ret;
}
static int get_mac_modifiers(Qt::KeyboardModifiers keys)
{
#ifdef DEBUG_KEY_BINDINGS
    qDebug("Qt: internal: **Mapping modifiers: %d (0x%04x)", (int)keys, (int)keys);
#endif
    int ret = 0;
    for(int i = 0; modifier_symbols[i].qt_code; i++) {
        if(keys & modifier_symbols[i].qt_code) {
#ifdef DEBUG_KEY_BINDINGS
            qDebug("Qt: internal: got modifier: %s", modifier_symbols[i].desc);
#endif
            ret |= modifier_symbols[i].mac_code;
        }
    }
    return ret;
}
void qt_mac_send_modifiers_changed(quint32 modifiers, QObject *object)
{
    static quint32 cachedModifiers = 0;
    quint32 lastModifiers = cachedModifiers,
          changedModifiers = lastModifiers ^ modifiers;
    cachedModifiers = modifiers;

    //check the bits
    static mac_enum_mapper modifier_key_symbols[] = {
        { shiftKeyBit, MAP_MAC_ENUM(Qt::Key_Shift) },
        { rightShiftKeyBit, MAP_MAC_ENUM(Qt::Key_Shift) }, //???
        { controlKeyBit, MAP_MAC_ENUM(Qt::Key_Meta) },
        { rightControlKeyBit, MAP_MAC_ENUM(Qt::Key_Meta) }, //???
        { cmdKeyBit, MAP_MAC_ENUM(Qt::Key_Control) },
        { optionKeyBit, MAP_MAC_ENUM(Qt::Key_Alt) },
        { rightOptionKeyBit, MAP_MAC_ENUM(Qt::Key_Alt) }, //???
        { alphaLockBit, MAP_MAC_ENUM(Qt::Key_CapsLock) },
        { kEventKeyModifierNumLockBit, MAP_MAC_ENUM(Qt::Key_NumLock) },
        {   0, MAP_MAC_ENUM(0) } };
    for(int i = 0; i <= 32; i++) { //just check each bit
        if(!(changedModifiers & (1 << i)))
            continue;
        QEvent::Type etype = QEvent::KeyPress;
        if(lastModifiers & (1 << i))
            etype = QEvent::KeyRelease;
        int key = 0;
        for(uint x = 0; modifier_key_symbols[x].mac_code; x++) {
            if(modifier_key_symbols[x].mac_code == i) {
#ifdef DEBUG_KEY_BINDINGS
                qDebug("got modifier changed: %s", modifier_key_symbols[x].desc);
#endif
                key = modifier_key_symbols[x].qt_code;
                break;
            }
        }
        if(!key) {
#ifdef DEBUG_KEY_BINDINGS
            qDebug("could not get modifier changed: %d", i);
#endif
            continue;
        }
#ifdef DEBUG_KEY_BINDINGS
        qDebug("KeyEvent (modif): Sending %s to %s::%s: %d - 0x%08x",
               etype == QEvent::KeyRelease ? "KeyRelease" : "KeyPress",
               object ? object->metaObject()->className() : "none",
               object ? object->objectName().toLatin1().constData() : "",
               key, (int)modifiers);
#endif
        QKeyEvent ke(etype, key, get_modifiers(modifiers ^ (1 << i)), "");
        qt_sendSpontaneousEvent(object, &ke);
    }
}

//keyboard keys (non-modifiers)
static mac_enum_mapper keyboard_symbols[] = {
    { kHomeCharCode, MAP_MAC_ENUM(Qt::Key_Home) },
    { kEnterCharCode, MAP_MAC_ENUM(Qt::Key_Enter) },
    { kEndCharCode, MAP_MAC_ENUM(Qt::Key_End) },
    { kBackspaceCharCode, MAP_MAC_ENUM(Qt::Key_Backspace) },
    { kTabCharCode, MAP_MAC_ENUM(Qt::Key_Tab) },
    { kPageUpCharCode, MAP_MAC_ENUM(Qt::Key_PageUp) },
    { kPageDownCharCode, MAP_MAC_ENUM(Qt::Key_PageDown) },
    { kReturnCharCode, MAP_MAC_ENUM(Qt::Key_Return) },
    { kEscapeCharCode, MAP_MAC_ENUM(Qt::Key_Escape) },
    { kLeftArrowCharCode, MAP_MAC_ENUM(Qt::Key_Left) },
    { kRightArrowCharCode, MAP_MAC_ENUM(Qt::Key_Right) },
    { kUpArrowCharCode, MAP_MAC_ENUM(Qt::Key_Up) },
    { kDownArrowCharCode, MAP_MAC_ENUM(Qt::Key_Down) },
    { kHelpCharCode, MAP_MAC_ENUM(Qt::Key_Help) },
    { kDeleteCharCode, MAP_MAC_ENUM(Qt::Key_Delete) },
//ascii maps, for debug
    { ':', MAP_MAC_ENUM(Qt::Key_Colon) },
    { ';', MAP_MAC_ENUM(Qt::Key_Semicolon) },
    { '<', MAP_MAC_ENUM(Qt::Key_Less) },
    { '=', MAP_MAC_ENUM(Qt::Key_Equal) },
    { '>', MAP_MAC_ENUM(Qt::Key_Greater) },
    { '?', MAP_MAC_ENUM(Qt::Key_Question) },
    { '@', MAP_MAC_ENUM(Qt::Key_At) },
    { ' ', MAP_MAC_ENUM(Qt::Key_Space) },
    { '!', MAP_MAC_ENUM(Qt::Key_Exclam) },
    { '"', MAP_MAC_ENUM(Qt::Key_QuoteDbl) },
    { '#', MAP_MAC_ENUM(Qt::Key_NumberSign) },
    { '$', MAP_MAC_ENUM(Qt::Key_Dollar) },
    { '%', MAP_MAC_ENUM(Qt::Key_Percent) },
    { '&', MAP_MAC_ENUM(Qt::Key_Ampersand) },
    { '\'', MAP_MAC_ENUM(Qt::Key_Apostrophe) },
    { '(', MAP_MAC_ENUM(Qt::Key_ParenLeft) },
    { ')', MAP_MAC_ENUM(Qt::Key_ParenRight) },
    { '*', MAP_MAC_ENUM(Qt::Key_Asterisk) },
    { '+', MAP_MAC_ENUM(Qt::Key_Plus) },
    { ',', MAP_MAC_ENUM(Qt::Key_Comma) },
    { '-', MAP_MAC_ENUM(Qt::Key_Minus) },
    { '.', MAP_MAC_ENUM(Qt::Key_Period) },
    { '/', MAP_MAC_ENUM(Qt::Key_Slash) },
    { '[', MAP_MAC_ENUM(Qt::Key_BracketLeft) },
    { ']', MAP_MAC_ENUM(Qt::Key_BracketRight) },
    { '\\', MAP_MAC_ENUM(Qt::Key_Backslash) },
    { '_', MAP_MAC_ENUM(Qt::Key_Underscore) },
    { '`', MAP_MAC_ENUM(Qt::Key_QuoteLeft) },
    { '{', MAP_MAC_ENUM(Qt::Key_BraceLeft) },
    { '}', MAP_MAC_ENUM(Qt::Key_BraceRight) },
    { '|', MAP_MAC_ENUM(Qt::Key_Bar) },
    { '~', MAP_MAC_ENUM(Qt::Key_AsciiTilde) },
    { '^', MAP_MAC_ENUM(Qt::Key_AsciiCircum) },
    {   0, MAP_MAC_ENUM(0) }
};

static mac_enum_mapper keyvkey_symbols[] = { //real scan codes
    { 122, MAP_MAC_ENUM(Qt::Key_F1) },
    { 120, MAP_MAC_ENUM(Qt::Key_F2) },
    { 99,  MAP_MAC_ENUM(Qt::Key_F3) },
    { 118, MAP_MAC_ENUM(Qt::Key_F4) },
    { 96,  MAP_MAC_ENUM(Qt::Key_F5) },
    { 97,  MAP_MAC_ENUM(Qt::Key_F6) },
    { 98,  MAP_MAC_ENUM(Qt::Key_F7) },
    { 100, MAP_MAC_ENUM(Qt::Key_F8) },
    { 101, MAP_MAC_ENUM(Qt::Key_F9) },
    { 109, MAP_MAC_ENUM(Qt::Key_F10) },
    { 103, MAP_MAC_ENUM(Qt::Key_F11) },
    { 111, MAP_MAC_ENUM(Qt::Key_F12) },
    {   0, MAP_MAC_ENUM(0) }
};


static int get_uniKey(int modif, const QChar &key, int virtualKey)
{
    if (key == kClearCharCode && virtualKey == 0x47)
        return Qt::Key_Clear;

    if (key.isDigit()) {
#ifdef DEBUG_KEY_BINDINGS
            qDebug("%d: got key: %d", __LINE__, key.digitValue());
#endif
        return key.digitValue() + Qt::Key_0;
    }

    if (key.isLetter()) {
#ifdef DEBUG_KEY_BINDINGS
        qDebug("%d: got key: %d", __LINE__, (key.toUpper().unicode() - 'A'));
#endif
        return (key.toUpper().unicode() - 'A') + Qt::Key_A;
    }
    for(int i = 0; keyboard_symbols[i].qt_code; i++) {
        if(keyboard_symbols[i].mac_code == key) {
            /* To work like Qt/X11 we issue Backtab when Shift + Tab are pressed */
            if(keyboard_symbols[i].qt_code == Qt::Key_Tab && (modif & Qt::ShiftModifier)) {
#ifdef DEBUG_KEY_BINDINGS
                qDebug("%d: got key: Qt::Key_Backtab", __LINE__);
#endif
                return Qt::Key_Backtab;
            }

#ifdef DEBUG_KEY_BINDINGS
            qDebug("%d: got key: %s", __LINE__, keyboard_symbols[i].desc);
#endif
            return keyboard_symbols[i].qt_code;
        }
    }

    //last ditch try to match the scan code
    for(int i = 0; keyvkey_symbols[i].qt_code; i++) {
        if(keyvkey_symbols[i].mac_code == virtualKey) {
#ifdef DEBUG_KEY_BINDINGS
            qDebug("%d: got key: %s", __LINE__, keyvkey_symbols[i].desc);
#endif
            return keyvkey_symbols[i].qt_code;
        }
    }

    //oh well
#ifdef DEBUG_KEY_BINDINGS
    qDebug("Unknown case.. %s:%d %d %d", __FILE__, __LINE__, key.toLatin1(), virtualKey);
#endif
    return Qt::Key_unknown;
}


static int get_key(int modif, int key, int virtualKey)
{
#ifdef DEBUG_KEY_BINDINGS
    qDebug("**Mapping key: %d (0x%04x) - %d (0x%04x)", key, key, virtualKey, virtualKey);
#endif

    //special case for clear key
    if(key == kClearCharCode && virtualKey == 0x47) {
#ifdef DEBUG_KEY_BINDINGS
        qDebug("%d: got key: Qt::Key_Clear", __LINE__);
#endif
        return Qt::Key_Clear;
    }

    //general cases..
    if(key >= '0' && key <= '9') {
#ifdef DEBUG_KEY_BINDINGS
        qDebug("%d: General case Qt::Key_%c", __LINE__, key);
#endif
        return (key - '0') + Qt::Key_0;
    }

    if((key >= 'A' && key <= 'Z') || (key >= 'a' && key <= 'z')) {
        char tup = toupper(key);
#ifdef DEBUG_KEY_BINDINGS
        qDebug("%d: General case Qt::Key_%c %d", __LINE__, tup, (tup - 'A') + Qt::Key_A);
#endif
        return (tup - 'A') + Qt::Key_A;
    }

    for(int i = 0; keyboard_symbols[i].qt_code; i++) {
        if(keyboard_symbols[i].mac_code == key) {
            /* To work like Qt/X11 we issue Backtab when Shift + Tab are pressed */
            if(keyboard_symbols[i].qt_code == Qt::Key_Tab && (modif & Qt::ShiftModifier)) {
#ifdef DEBUG_KEY_BINDINGS
                qDebug("%d: got key: Qt::Key_Backtab", __LINE__);
#endif
                return Qt::Key_Backtab;
            }

#ifdef DEBUG_KEY_BINDINGS
            qDebug("%d: got key: %s", __LINE__, keyboard_symbols[i].desc);
#endif
            return keyboard_symbols[i].qt_code;
        }
    }

    //last ditch try to match the scan code
    for(int i = 0; keyvkey_symbols[i].qt_code; i++) {
        if(keyvkey_symbols[i].mac_code == virtualKey) {
#ifdef DEBUG_KEY_BINDINGS
            qDebug("%d: got key: %s", __LINE__, keyvkey_symbols[i].desc);
#endif
            return keyvkey_symbols[i].qt_code;
        }
    }

    //oh well
#ifdef DEBUG_KEY_BINDINGS
    qDebug("Unknown case.. %s:%d %d %d", __FILE__, __LINE__, key, virtualKey);
#endif
    return Qt::Key_unknown;
}

static Boolean qt_KeyEventComparatorProc(EventRef inEvent, void *data)
{
    UInt32 ekind = GetEventKind(inEvent),
           eclass = GetEventClass(inEvent);
    return (eclass == kEventClassKeyboard && ekind == (UInt32)data);
}

static bool translateKeyEventInternal(EventHandlerCallRef er, EventRef keyEvent, int *qtKey,
                                      QChar *outChar, Qt::KeyboardModifiers *outModifiers, bool *outHandled)
{
    const UInt32 ekind = GetEventKind(keyEvent);
    {
        UInt32 mac_modifiers = 0;
        GetEventParameter(keyEvent, kEventParamKeyModifiers, typeUInt32, 0,
                          sizeof(mac_modifiers), 0, &mac_modifiers);
#ifdef DEBUG_KEY_BINDINGS
        qDebug("************ Mapping modifiers and key ***********");
#endif
        *outModifiers = get_modifiers(mac_modifiers);
#ifdef DEBUG_KEY_BINDINGS
        qDebug("------------ Mapping modifiers and key -----------");
#endif
    }

    //get keycode
    UInt32 keyCode = 0;
    GetEventParameter(keyEvent, kEventParamKeyCode, typeUInt32, 0, sizeof(keyCode), 0, &keyCode);

    //get mac mapping
    static UInt32 tmp_unused_state = 0L;
    static KeyboardLayoutRef prevKeyLayoutRef = 0;
    KeyboardLayoutRef keyLayoutRef = 0;
    UCKeyboardLayout *uchrData = 0;
    KLGetCurrentKeyboardLayout(&keyLayoutRef);
    OSStatus err;
    if (keyLayoutRef != 0) {
        err = KLGetKeyboardLayoutProperty(keyLayoutRef, kKLuchrData,
                                  const_cast<const void **>(reinterpret_cast<void **>(&uchrData)));
        if (err != noErr) {
            qWarning("Qt::internal::unable to get keyboardlayout %ld %s:%d",
                     err, __FILE__, __LINE__);
        }
    }

    if (uchrData) {
        static UInt32 deadKeyState;
        if (prevKeyLayoutRef != keyLayoutRef) {
            // Clear the dead state
            deadKeyState = 0;
            prevKeyLayoutRef = keyLayoutRef;
        }
        // The easy use the unicode stuff!
        UniChar string[4];
        UniCharCount actualLength;
        int keyAction;
        switch (ekind) {
        default:
        case kEventRawKeyDown:
            keyAction = kUCKeyActionDown;
            break;
        case kEventRawKeyUp:
            keyAction = kUCKeyActionUp;
            break;
        case kEventRawKeyRepeat:
            keyAction = kUCKeyActionAutoKey;
            break;
        }
        OSStatus err = UCKeyTranslate(uchrData, keyCode, keyAction,
                                  ((GetCurrentEventKeyModifiers() >> 8) & 0xff), LMGetKbdType(),
                                  kUCKeyTranslateNoDeadKeysBit, &tmp_unused_state, 4, &actualLength,
                                  string);
        if (err == noErr) {
            *qtKey = get_uniKey(*outModifiers, QChar(string[0]), keyCode);
            if (ekind != kEventRawKeyUp)
                *outChar = QChar(string[0]);
        } else {
            qWarning("Qt::internal::UCKeyTranslate is returnining %ld %s:%d",
                     err, __FILE__, __LINE__);
        }
    } else {
        // The road less travelled, Try to get the unichar, using the
        // given the "mac" keyboard resource.
        char translatedChar = KeyTranslate((void *)GetScriptVariable(smCurrentScript, smKCHRCache),
                (GetCurrentEventKeyModifiers() &
                 (kEventKeyModifierNumLockMask|shiftKey|cmdKey|
                  rightShiftKey|alphaLock)) | keyCode,
                &tmp_unused_state);
        if(!translatedChar) {
            if (outHandled) {
                qt_mac_eat_unicode_key = false;
                CallNextEventHandler(er, keyEvent);
                *outHandled = qt_mac_eat_unicode_key;
            }
            return false;
        }

        //map it into qt keys
        *qtKey = get_key(*outModifiers, translatedChar, keyCode);
        if(*outModifiers & (Qt::AltModifier | Qt::ControlModifier)) {
            if(translatedChar & (1 << 7)) //high ascii
                translatedChar = 0;
        } else {          //now get the real ascii value
            UInt32 tmp_mod = 0L;
            static UInt32 tmp_state = 0L;
            if(*outModifiers & Qt::ShiftModifier)
                tmp_mod |= shiftKey;
            if(*outModifiers & Qt::MetaModifier)
                tmp_mod |= controlKey;
            if(*outModifiers & Qt::ControlModifier)
                tmp_mod |= cmdKey;
            if(GetCurrentEventKeyModifiers() & alphaLock) //no Qt mapper
                tmp_mod |= alphaLock;
            if(*outModifiers & Qt::AltModifier)
                tmp_mod |= optionKey;
            if(*outModifiers & Qt::KeypadModifier)
                tmp_mod |= kEventKeyModifierNumLockMask;
            translatedChar = KeyTranslate((void *)GetScriptManagerVariable(smUnicodeScript),
                    tmp_mod | keyCode, &tmp_state);
        }
        /* I don't know why the str is only filled in in RawKeyDown - but it does seem to be on X11
           is this a bug on X11? --Sam */
        if (ekind != kEventRawKeyUp) {
            UInt32 unilen = 0;
            if (GetEventParameter(keyEvent, kEventParamKeyUnicodes, typeUnicodeText, 0, 0, &unilen, 0)
                    == noErr && unilen == 2) {
                GetEventParameter(keyEvent, kEventParamKeyUnicodes, typeUnicodeText, 0, unilen, 0, outChar);
            } else if (translatedChar) {
                static QTextCodec *c = 0;
                if (!c)
                    c = QTextCodec::codecForName("Apple Roman");
                *outChar = c->toUnicode(&translatedChar, 1).at(0);
            }
        }
    }
    return true;
}

QKeyMapperPrivate::QKeyMapperPrivate()
{
    memset(keyLayout, 0, sizeof(keyLayout));
    keyboard_mode = NullMode;
}

QKeyMapperPrivate::~QKeyMapperPrivate()
{
    clearMappings();
}

bool
QKeyMapperPrivate::updateKeyboard()
{
    KeyboardLayoutRef keyLayoutRef = 0;
    KLGetCurrentKeyboardLayout(&keyLayoutRef);

    if(keyboard_mode != NullMode && currentKeyboardLayout == keyLayoutRef)
        return false;

    OSStatus err;
    UCKeyboardLayout *uchrData = 0;
    if (keyLayoutRef != 0) {
        err = KLGetKeyboardLayoutProperty(keyLayoutRef, kKLuchrData,
                                  const_cast<const void **>(reinterpret_cast<void **>(&uchrData)));
        if (err != noErr) {
            qWarning("Qt::internal::unable to get unicode keyboardlayout %ld %s:%d",
                     err, __FILE__, __LINE__);
        }
    }

    if (uchrData) {
        keyboard_layout_format.unicode = uchrData;
        keyboard_mode = UnicodeMode;
    } else {
        void *happy;
        err = KLGetKeyboardLayoutProperty(keyLayoutRef, kKLKCHRData,
                                  const_cast<const void **>(reinterpret_cast<void **>(&happy)));
        if (err != noErr) {
            qFatal("Qt::internal::unable to get non-unicode layout, cannot procede %ld %s:%d",
                     err, __FILE__, __LINE__);
        }
        keyboard_layout_format.other = happy;
        keyboard_mode = OtherMode;
    }

    keyboard_kind = LMGetKbdType();
    currentKeyboardLayout = keyLayoutRef;
    keyboard_dead = 0;

#if 0
    ScriptLanguageRecord language;
    GetTextServiceLanguage(&language);
    qDebug() << language.fScript << language.fLanguage;

    QCFType<CFLocaleRef> cf_locale = CFLocaleCopyCurrent();
    SInt16 locale = GetScriptVariable(currentKeyScript, smScriptLang);
    keyboardInputLocale = QLocale();
    keyboardInputDirection = GetScriptVariable(currentKeyScript, smScriptRight) ? Qt::RightToLeft : Qt::LeftToRight;
    qDebug() << locale << QCFString::toQString(CFLocaleGetIdentifier(cf_locale)) << QCFString::toQString(CFLocaleGetIdentifier(CFLocaleGetSystem()))
             << keyboardInputDirection;
#endif
    return true;
}

void
QKeyMapperPrivate::clearMappings()
{
    keyboard_mode = NullMode;
    for (int i = 0; i < 255; ++i) {
        if (keyLayout[i]) {
            delete keyLayout[i];
            keyLayout[i] = 0;
        }
    }
    updateKeyboard();
}

QList<int>
QKeyMapperPrivate::possibleKeys(QKeyEvent *e)
{
    QList<int> ret;

    KeyboardLayoutItem *kbItem = keyLayout[e->nativeVirtualKey()];
    Q_ASSERT(kbItem);

    int baseKey = kbItem->qtKey[0];
    Qt::KeyboardModifiers keyMods = e->modifiers();
    ret << int(baseKey + keyMods); // The base key is _always_ valid, of course

    for(int i = 1; i < 8; ++i) {
        Qt::KeyboardModifiers neededMods = ModsTbl[i];
        int key = kbItem->qtKey[i];
        if (key && key != baseKey && ((keyMods & neededMods) == neededMods))
            ret << int(key + (keyMods & ~neededMods));
    }

    return ret;
}

bool
QKeyMapperPrivate::translateKeyEvent(QWidget *widget, EventHandlerCallRef er, EventRef event, void *data, bool grab)
{
#if 0
    //QApplication *app = (QApplication *)data;
#else
    Q_UNUSED(data);
#endif

    bool handled_event=true;
    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);

    switch(eclass)
    {
    case kEventClassKeyboard: {
        if (qApp->inputContext() && qApp->inputContext()->isComposing()) {
            handled_event = false;
            break;
        }
        // unfortunatly modifiers changed event looks quite different, so I have a separate
        // code path
        if(ekind == kEventRawKeyModifiersChanged) {
            //figure out changed modifiers, wish Apple would just send a delta
            UInt32 modifiers = 0;
            GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, 0,
                              sizeof(modifiers), 0, &modifiers);
            qt_mac_send_modifiers_changed(modifiers, widget);
            break;
        }
        //get modifiers
        Qt::KeyboardModifiers modifiers;
        int qtKey;
        QChar ourChar;
        if (translateKeyEventInternal(er, event, &qtKey, &ourChar, &modifiers,
                                      &handled_event) == false)
            break;
        QString text(ourChar);
        if(widget) {
            //Find out if someone else wants the event, namely
            //is it of use to text services? If so we won't bother
            //with a QKeyEvent.
            qt_mac_eat_unicode_key = false;
            CallNextEventHandler(er, event);
            if(qt_mac_eat_unicode_key) {
                handled_event = true;
                break;
            }
            // Try to compress key events.
            if (!text.isEmpty() && widget->testAttribute(Qt::WA_KeyCompression)) {
                EventTime lastTime = GetEventTime(event);
                for (;;) {
                    EventRef releaseEvent = FindSpecificEventInQueue(GetMainEventQueue(),
                                                                     qt_KeyEventComparatorProc,
                                                                     (void*)kEventRawKeyUp);
                    if (!releaseEvent)
                        break;
                    const EventTime releaseTime = GetEventTime(releaseEvent);
                    if(releaseTime < lastTime)
                        break;
                    lastTime = releaseTime;

                    EventRef pressEvent = FindSpecificEventInQueue(GetMainEventQueue(),
                                                                   qt_KeyEventComparatorProc,
                                                                   (void*)kEventRawKeyDown);
                    if (!pressEvent)
                        break;
                    const EventTime pressTime = GetEventTime(pressEvent);
                    if(pressTime < lastTime)
                        break;
                    lastTime = pressTime;

                    Qt::KeyboardModifiers compressMod;
                    int compressQtKey;
                    QChar compressChar;
                    if (translateKeyEventInternal(er, pressEvent,
                                                  &compressQtKey, &compressChar, &compressMod, 0)
                        == false) {
                        break;
                    }
                    // Copied from qapplication_x11.cpp (change both).

                    bool stopCompression =
                        // 1) misc keys
                        (compressQtKey >= Qt::Key_Escape && compressQtKey <= Qt::Key_SysReq)
                        // 2) cursor movement
                        || (compressQtKey >= Qt::Key_Home && compressQtKey <= Qt::Key_PageDown)
                        // 3) extra keys
                        || (compressQtKey >= Qt::Key_Super_L && compressQtKey <= Qt::Key_Direction_R)
                        // 4) something that a) doesn't translate to text or b) translates
                        //    to newline text
                        || (compressQtKey == 0)
                        || (compressChar == QLatin1Char('\n'));
                    if (compressMod == modifiers && !compressChar.isNull() && !stopCompression) {
#ifdef DEBUG_KEY_BINDINGS
                        qDebug("compressing away %c", compressChar.toLatin1());
#endif
                        text += compressChar;
                        // Clean up
                        RemoveEventFromQueue(GetMainEventQueue(), releaseEvent);
                        RemoveEventFromQueue(GetMainEventQueue(), pressEvent);
                    } else {
#ifdef DEBUG_KEY_BINDINGS
                        qDebug("stoping compression..");
#endif
                        break;
                    }
                }
            }

            UInt32 macScanCode = 0;
            UInt32 macVirtualKey = 0;
            GetEventParameter(event, kEventParamKeyCode, typeUInt32, 0, sizeof(macVirtualKey), 0, &macVirtualKey);
            UInt32 macModifiers = 0;
            GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, 0,
                              sizeof(macModifiers), 0, &macModifiers);
            handled_event = QKeyMapper::sendKeyEvent(widget, grab,
                                                     (ekind == kEventRawKeyUp) ? QEvent::KeyRelease : QEvent::KeyPress,
                                                     qtKey, modifiers, text, ekind == kEventRawKeyRepeat, 0,
                                                     macScanCode, macVirtualKey, macModifiers);

        } else {
            handled_event = false;
        }
        break; }
    default:
        handled_event = false;
        break;
    }
    return handled_event;
}

void
QKeyMapperPrivate::updateKeyMap(EventHandlerCallRef, EventRef event, void *)
{
    UInt32 macVirtualKey = 0;
    GetEventParameter(event, kEventParamKeyCode, typeUInt32, 0, sizeof(macVirtualKey), 0, &macVirtualKey);
    if(!macVirtualKey)
        return;
    if (updateKeyboard())
       QKeyMapper::changeKeyboard();
    else if(keyLayout[macVirtualKey])
        return;

    UniCharCount buffer_size = 10;
    UniChar buffer[buffer_size];
    keyLayout[macVirtualKey] = new KeyboardLayoutItem;
    for(int i = 0; i < 16; ++i) {
        UniCharCount out_buffer_size = 0;
        keyLayout[macVirtualKey]->qtKey[i] = 0;
        if(keyboard_mode == UnicodeMode) {
            const UInt32 keyModifier = ((get_mac_modifiers(ModsTbl[i]) >> 8) & 0xFF);
            OSStatus err = UCKeyTranslate(keyboard_layout_format.unicode, macVirtualKey, kUCKeyActionDown, keyModifier,
                                          keyboard_kind, 0, &keyboard_dead, buffer_size, &out_buffer_size, buffer);
            if(err == noErr && out_buffer_size) {
                const QChar unicode(buffer[0]);
                int qtkey = get_uniKey(keyModifier, unicode, macVirtualKey);
                if(qtkey == Qt::Key_unknown)
                    qtkey = unicode.unicode();
                keyLayout[macVirtualKey]->qtKey[i] = qtkey;
            }
        } else {
            const UInt32 keyModifier = (get_mac_modifiers(ModsTbl[i]));// & ~(rightShiftKey | rightControlKey | rightOptionKey));
            qDebug() << "foo" << hex << keyModifier;

            uchar translatedChar = KeyTranslate(keyboard_layout_format.other, keyModifier | macVirtualKey, &keyboard_dead);
            if(translatedChar) {
                static QTextCodec *c = 0;
                if (!c)
                    c = QTextCodec::codecForName("Apple Roman");
                const QChar unicode(c->toUnicode((const char *)&translatedChar, 1).at(0));
                int qtkey = get_uniKey(keyModifier, unicode, macVirtualKey);
                if(qtkey == Qt::Key_unknown)
                    qtkey = unicode.unicode();
                keyLayout[macVirtualKey]->qtKey[i] = qtkey;
            }
        }
    }
#ifdef DEBUG_KEY_MAPS
    qDebug("updatePossibleKeyCodes for virtual key = 0x%02x!", (uint)macVirtualKey);
    for (int i = 0; i < 16; ++i) {
        qDebug("    [%d] (%d,0x%02x,'%c')", i,
               keyLayout[macVirtualKey]->qtKey[i],
               keyLayout[macVirtualKey]->qtKey[i],
               keyLayout[macVirtualKey]->qtKey[i]);
    }
#endif
}

bool
QKeyMapper::sendKeyEvent(QWidget *widget, bool grab,
                         QEvent::Type type, int code, Qt::KeyboardModifiers modifiers,
                         const QString &text, bool autorepeat, int count,
                         quint32 nativeScanCode, quint32 nativeVirtualKey,
                         quint32 nativeModifiers)
{
    Q_UNUSED(count);
    if(widget) {
        bool key_event = true;
#if 1 // ####### Enable Support stuff below
        Q_UNUSED(grab);
#elif defined(QT3_SUPPORT) && !defined(QT_NO_SHORTCUT)
        if(etype == QEvent::KeyPress && !grab
           && QApplicationPrivate::instance()->use_compat()) {
            QKeyEventEx accel_ev(Qt::ShortcutOverride, code, modifiers, text, autorepeat, qMax(1, text.length()),
                                 nativeScanCode, nativeVirtualKey, nativeModifiers);
            if(QApplicationPrivate::instance()->qt_tryAccelEvent(widget, &accel_ev)) {
#ifdef DEBUG_KEY_BINDINGS
                qDebug("KeyEvent: %s::%s consumed Accel: %s %d",
                       widget ? widget->metaObject()->className() : "none",
                       widget ? widget->objectName().toLatin1().constData() : "",
                       text.toLatin1().constData(), ekind == kEventRawKeyRepeat);
#endif
                key_event = false;
            } else {
                if(accel_ev.isAccepted()) {
#ifdef DEBUG_KEY_BINDINGS
                    qDebug("KeyEvent: %s::%s overrode Accel: %s %d",
                           widget ? widget->metaObject()->className() : "none",
                           widget ? widget->objectName().toLatin1().constData() : "",
                           text.toLatin1().constData(), ekind == kEventRawKeyRepeat);
#endif
                }
            }
        }
#endif // QT3_SUPPORT && !QT_NO_SHORTCUT
        if(key_event) {
#ifdef DEBUG_KEY_BINDINGS
            qDebug("KeyEvent: Sending %s to %s::%s: %s 0x%08x%s",
                   type == QEvent::KeyRelease ? "KeyRelease" : "KeyPress",
                   widget ? widget->metaObject()->className() : "none",
                   widget ? widget->objectName().toLatin1().constData() : "",
                   text.toLatin1().constData(), int(modifiers),
                   autorepeat ? " Repeat" : "");
#endif
            QKeyEventEx ke(type, code, modifiers, text, autorepeat, qMax(1, text.length()),
                           nativeScanCode, nativeVirtualKey, nativeModifiers);
            qt_sendSpontaneousEvent(widget,&ke);
        }
    }
    return false;
}
