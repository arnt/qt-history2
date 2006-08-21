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

#include "qkeysequence.h"
#include "qkeysequence_p.h"
#include "private/qapplication_p.h"

#ifndef QT_NO_SHORTCUT

#include "qshortcut.h"
#include "qdebug.h"
#ifndef QT_NO_REGEXP
# include "qregexp.h"
#endif
#ifndef QT_NO_DATASTREAM
# include "qdatastream.h"
#endif
#include "qvariant.h"

#ifdef Q_WS_MAC
# include <private/qt_mac_p.h>
# define QMAC_CTRL QChar(kCommandUnicode)
# define QMAC_META QChar(kControlUnicode)
# define QMAC_ALT  QChar(kOptionUnicode)
# define QMAC_SHIFT QChar(kShiftUnicode)
#endif

#ifdef Q_WS_MAC
static bool qt_sequence_no_mnemonics = true;
#else
static bool qt_sequence_no_mnemonics = false;
#endif
void Q_GUI_EXPORT qt_set_sequence_auto_mnemonic(bool b) { qt_sequence_no_mnemonics = !b; }

/*!
    \class QKeySequence
    \brief The QKeySequence class encapsulates a key sequence as used
    by shortcuts.

    \ingroup misc
    \ingroup shared
    \mainclass

    A key sequence consists of up to four keyboard codes, each
    optionally combined with modifiers, such as Qt::SHIFT, Qt::CTRL,
    Qt::ALT or Qt::META. For example, Qt::CTRL + Qt::Key_P might be a
    sequence used as a shortcut for printing a document. Valid codes
    for keys and modifiers are listed in Qt::Key and Qt::Modifier. As
    an alternative, use the unicode code point of the character; for
    example, 'A' gives the same key sequence as Qt::Key_A.

    Key sequences can be constructed either from an integer key code,
    or from a human readable translatable string such as
    "Ctrl+X,Alt+Space". A key sequence can be cast to a QString to
    obtain a human readable translated version of the sequence.
    Translations are done in the "QShortcut" context.

    \bold{Note:} On Mac OS X, references to "Ctrl", Qt::CTRL, Qt::Control
    and Qt::ControlModifier correspond to the Command keys on the Macintosh
    keyboard, and references to "Meta", Qt::META, Qt::Meta and
    Qt::MetaModifier correspond to the Control keys. Developers on Mac OS X
    can use the same shortcut descriptions across all platforms, and their
    applications will automatically work as expected on Mac OS X.

    The toString() function produces human-readable strings for use
    in menus. On Mac OS X, the appropriate symbols are used to describe
    keyboard shortcuts using special keys on the Macintosh keyboard.

    \sa QShortcut
*/

/*!
    \enum QKeySequence::SequenceMatch

    \value NoMatch The key sequences are different; not even partially
    matching.
    \value PartialMatch The key sequences match partially, but are not
    the same.
    \value ExactMatch The key sequences are the same.
    \omitvalue Identical
*/

/*!
    \enum QKeySequence::SequenceFormat

    \value NativeText The key sequence as a platform specific string.
    This means that it will be shown translated and on the Mac it will
    resemble a keysequence from the menu bar. This enum is best used when you
    want to display the string to the user.

    \value PortableText The key sequence is given in a "portable" format,
    suitable for reading and writing to a file. In many cases, it will look
    similar to the native text on Windows and X11.
*/

static const struct {
    int key;
    const char* name;
} keyname[] = {
    { Qt::Key_Space,        QT_TRANSLATE_NOOP("QShortcut", "Space") },
    { Qt::Key_Escape,       QT_TRANSLATE_NOOP("QShortcut", "Esc") },
    { Qt::Key_Tab,          QT_TRANSLATE_NOOP("QShortcut", "Tab") },
    { Qt::Key_Backtab,      QT_TRANSLATE_NOOP("QShortcut", "Backtab") },
    { Qt::Key_Backspace,    QT_TRANSLATE_NOOP("QShortcut", "Backspace") },
    { Qt::Key_Return,       QT_TRANSLATE_NOOP("QShortcut", "Return") },
    { Qt::Key_Enter,        QT_TRANSLATE_NOOP("QShortcut", "Enter") },
    { Qt::Key_Insert,       QT_TRANSLATE_NOOP("QShortcut", "Ins") },
    { Qt::Key_Delete,       QT_TRANSLATE_NOOP("QShortcut", "Del") },
    { Qt::Key_Pause,        QT_TRANSLATE_NOOP("QShortcut", "Pause") },
    { Qt::Key_Print,        QT_TRANSLATE_NOOP("QShortcut", "Print") },
    { Qt::Key_SysReq,       QT_TRANSLATE_NOOP("QShortcut", "SysReq") },
    { Qt::Key_Home,         QT_TRANSLATE_NOOP("QShortcut", "Home") },
    { Qt::Key_End,          QT_TRANSLATE_NOOP("QShortcut", "End") },
    { Qt::Key_Left,         QT_TRANSLATE_NOOP("QShortcut", "Left") },
    { Qt::Key_Up,           QT_TRANSLATE_NOOP("QShortcut", "Up") },
    { Qt::Key_Right,        QT_TRANSLATE_NOOP("QShortcut", "Right") },
    { Qt::Key_Down,         QT_TRANSLATE_NOOP("QShortcut", "Down") },
    { Qt::Key_PageUp,       QT_TRANSLATE_NOOP("QShortcut", "PgUp") },
    { Qt::Key_PageDown,     QT_TRANSLATE_NOOP("QShortcut", "PgDown") },
    { Qt::Key_CapsLock,     QT_TRANSLATE_NOOP("QShortcut", "CapsLock") },
    { Qt::Key_NumLock,      QT_TRANSLATE_NOOP("QShortcut", "NumLock") },
    { Qt::Key_ScrollLock,   QT_TRANSLATE_NOOP("QShortcut", "ScrollLock") },
    { Qt::Key_Menu,         QT_TRANSLATE_NOOP("QShortcut", "Menu") },
    { Qt::Key_Help,         QT_TRANSLATE_NOOP("QShortcut", "Help") },

    // Multimedia keys
    { Qt::Key_Back,         QT_TRANSLATE_NOOP("QShortcut", "Back") },
    { Qt::Key_Forward,      QT_TRANSLATE_NOOP("QShortcut", "Forward") },
    { Qt::Key_Stop,         QT_TRANSLATE_NOOP("QShortcut", "Stop") },
    { Qt::Key_Refresh,      QT_TRANSLATE_NOOP("QShortcut", "Refresh") },
    { Qt::Key_VolumeDown,   QT_TRANSLATE_NOOP("QShortcut", "Volume Down") },
    { Qt::Key_VolumeMute,   QT_TRANSLATE_NOOP("QShortcut", "Volume Mute") },
    { Qt::Key_VolumeUp,     QT_TRANSLATE_NOOP("QShortcut", "Volume Up") },
    { Qt::Key_BassBoost,    QT_TRANSLATE_NOOP("QShortcut", "Bass Boost") },
    { Qt::Key_BassUp,       QT_TRANSLATE_NOOP("QShortcut", "Bass Up") },
    { Qt::Key_BassDown,     QT_TRANSLATE_NOOP("QShortcut", "Bass Down") },
    { Qt::Key_TrebleUp,     QT_TRANSLATE_NOOP("QShortcut", "Treble Up") },
    { Qt::Key_TrebleDown,   QT_TRANSLATE_NOOP("QShortcut", "Treble Down") },
    { Qt::Key_MediaPlay,    QT_TRANSLATE_NOOP("QShortcut", "Media Play") },
    { Qt::Key_MediaStop,    QT_TRANSLATE_NOOP("QShortcut", "Media Stop") },
    { Qt::Key_MediaPrevious,QT_TRANSLATE_NOOP("QShortcut", "Media Previous") },
    { Qt::Key_MediaNext,    QT_TRANSLATE_NOOP("QShortcut", "Media Next") },
    { Qt::Key_MediaRecord,  QT_TRANSLATE_NOOP("QShortcut", "Media Record") },
    { Qt::Key_HomePage,     QT_TRANSLATE_NOOP("QShortcut", "Home Page") },
    { Qt::Key_Favorites,    QT_TRANSLATE_NOOP("QShortcut", "Favorites") },
    { Qt::Key_Search,       QT_TRANSLATE_NOOP("QShortcut", "Search") },
    { Qt::Key_Standby,      QT_TRANSLATE_NOOP("QShortcut", "Standby") },
    { Qt::Key_OpenUrl,      QT_TRANSLATE_NOOP("QShortcut", "Open URL") },
    { Qt::Key_LaunchMail,   QT_TRANSLATE_NOOP("QShortcut", "Launch Mail") },
    { Qt::Key_LaunchMedia,  QT_TRANSLATE_NOOP("QShortcut", "Launch Media") },
    { Qt::Key_Launch0,      QT_TRANSLATE_NOOP("QShortcut", "Launch (0)") },
    { Qt::Key_Launch1,      QT_TRANSLATE_NOOP("QShortcut", "Launch (1)") },
    { Qt::Key_Launch2,      QT_TRANSLATE_NOOP("QShortcut", "Launch (2)") },
    { Qt::Key_Launch3,      QT_TRANSLATE_NOOP("QShortcut", "Launch (3)") },
    { Qt::Key_Launch4,      QT_TRANSLATE_NOOP("QShortcut", "Launch (4)") },
    { Qt::Key_Launch5,      QT_TRANSLATE_NOOP("QShortcut", "Launch (5)") },
    { Qt::Key_Launch6,      QT_TRANSLATE_NOOP("QShortcut", "Launch (6)") },
    { Qt::Key_Launch7,      QT_TRANSLATE_NOOP("QShortcut", "Launch (7)") },
    { Qt::Key_Launch8,      QT_TRANSLATE_NOOP("QShortcut", "Launch (8)") },
    { Qt::Key_Launch9,      QT_TRANSLATE_NOOP("QShortcut", "Launch (9)") },
    { Qt::Key_LaunchA,      QT_TRANSLATE_NOOP("QShortcut", "Launch (A)") },
    { Qt::Key_LaunchB,      QT_TRANSLATE_NOOP("QShortcut", "Launch (B)") },
    { Qt::Key_LaunchC,      QT_TRANSLATE_NOOP("QShortcut", "Launch (C)") },
    { Qt::Key_LaunchD,      QT_TRANSLATE_NOOP("QShortcut", "Launch (D)") },
    { Qt::Key_LaunchE,      QT_TRANSLATE_NOOP("QShortcut", "Launch (E)") },
    { Qt::Key_LaunchF,      QT_TRANSLATE_NOOP("QShortcut", "Launch (F)") },

    // --------------------------------------------------------------
    // More consistent namings
    { Qt::Key_Print,        QT_TRANSLATE_NOOP("QShortcut", "Print Screen") },
    { Qt::Key_PageUp,       QT_TRANSLATE_NOOP("QShortcut", "Page Up") },
    { Qt::Key_PageDown,     QT_TRANSLATE_NOOP("QShortcut", "Page Down") },
    { Qt::Key_CapsLock,     QT_TRANSLATE_NOOP("QShortcut", "Caps Lock") },
    { Qt::Key_NumLock,      QT_TRANSLATE_NOOP("QShortcut", "Num Lock") },
    { Qt::Key_NumLock,      QT_TRANSLATE_NOOP("QShortcut", "Number Lock") },
    { Qt::Key_ScrollLock,   QT_TRANSLATE_NOOP("QShortcut", "Scroll Lock") },
    { Qt::Key_Insert,       QT_TRANSLATE_NOOP("QShortcut", "Insert") },
    { Qt::Key_Delete,       QT_TRANSLATE_NOOP("QShortcut", "Delete") },
    { Qt::Key_Escape,       QT_TRANSLATE_NOOP("QShortcut", "Escape") },
    { Qt::Key_SysReq,       QT_TRANSLATE_NOOP("QShortcut", "System Request") },

    // --------------------------------------------------------------
    // Keypad navigation keys
    { Qt::Key_Select,       QT_TRANSLATE_NOOP("QShortcut", "Select") },
    { Qt::Key_Yes,          QT_TRANSLATE_NOOP("QShortcut", "Yes") },
    { Qt::Key_No,           QT_TRANSLATE_NOOP("QShortcut", "No") },

    // --------------------------------------------------------------
    // Device keys
    { Qt::Key_Context1,     QT_TRANSLATE_NOOP("QShortcut", "Context1") },
    { Qt::Key_Context2,     QT_TRANSLATE_NOOP("QShortcut", "Context2") },
    { Qt::Key_Context3,     QT_TRANSLATE_NOOP("QShortcut", "Context3") },
    { Qt::Key_Context4,     QT_TRANSLATE_NOOP("QShortcut", "Context4") },
    { Qt::Key_Call,         QT_TRANSLATE_NOOP("QShortcut", "Call") },
    { Qt::Key_Hangup,       QT_TRANSLATE_NOOP("QShortcut", "Hangup") },
    { Qt::Key_Flip,         QT_TRANSLATE_NOOP("QShortcut", "Flip") },


    { 0, 0 }
};

//Table of key bindings. It must be sorted on key sequence.
//A priority of 1 indicates that this is the primary key binding when multiple are defined.

const QKeyBinding QKeySequencePrivate::keyBindings[] = {
//   StandardKey                            Priority    Key Sequence                            Platforms
    {QKeySequence::Delete,                  1,          Qt::Key_Delete,                         QApplicationPrivate::KB_All},
    {QKeySequence::MoveToStartOfLine,       0,          Qt::Key_Home,                           QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
    {QKeySequence::MoveToStartOfDocument,   0,          Qt::Key_Home,                           QApplicationPrivate::KB_Mac},
    {QKeySequence::MoveToEndOfLine,         0,          Qt::Key_End,                            QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
    {QKeySequence::MoveToEndOfDocument,     0,          Qt::Key_End,                            QApplicationPrivate::KB_Mac},
    {QKeySequence::MoveToPreviousChar,      0,          Qt::Key_Left,                           QApplicationPrivate::KB_All},
    {QKeySequence::MoveToPreviousLine,      0,          Qt::Key_Up,                             QApplicationPrivate::KB_All},
    {QKeySequence::MoveToNextChar,          0,          Qt::Key_Right,                          QApplicationPrivate::KB_All},
    {QKeySequence::MoveToNextLine,          0,          Qt::Key_Down,                           QApplicationPrivate::KB_All},
    {QKeySequence::MoveToPreviousPage,      1,          Qt::Key_PageUp,                         QApplicationPrivate::KB_All},
    {QKeySequence::MoveToNextPage,          1,          Qt::Key_PageDown,                       QApplicationPrivate::KB_All},
    {QKeySequence::HelpContents,            0,          Qt::Key_F1,                             QApplicationPrivate::KB_All},
    {QKeySequence::FindNext,                1,          Qt::Key_F3,                             QApplicationPrivate::KB_KDE},
    {QKeySequence::Refresh,                 0,          Qt::Key_F5,                             QApplicationPrivate::KB_All},
    {QKeySequence::Undo,                    0,          Qt::Key_F14,                            QApplicationPrivate::KB_X11}, //Undo on sun keyboards
    {QKeySequence::Copy,                    0,          Qt::Key_F16,                            QApplicationPrivate::KB_X11}, //Copy on sun keyboards
    {QKeySequence::Paste,                   0,          Qt::Key_F18,                            QApplicationPrivate::KB_X11}, //Paste on sun keyboards      
    {QKeySequence::Cut,                     0,          Qt::Key_F20,                            QApplicationPrivate::KB_X11}, //Cut on sun keyboards
    {QKeySequence::PreviousChild,           0,          Qt::Key_Back,                           QApplicationPrivate::KB_All},
    {QKeySequence::NextChild,               0,          Qt::Key_Forward,                        QApplicationPrivate::KB_All}, 
    {QKeySequence::Paste,                   0,          Qt::SHIFT | Qt::Key_Insert,             QApplicationPrivate::KB_All}, 
    {QKeySequence::Cut,                     0,          Qt::SHIFT | Qt::Key_Delete,             QApplicationPrivate::KB_All}, //## Check if this should work on mac
    {QKeySequence::SelectStartOfLine,       0,          Qt::SHIFT | Qt::Key_Home,               QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
    {QKeySequence::SelectEndOfLine,         0,          Qt::SHIFT | Qt::Key_End,                QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
    {QKeySequence::SelectPreviousChar,      0,          Qt::SHIFT | Qt::Key_Left,               QApplicationPrivate::KB_All},
    {QKeySequence::SelectPreviousLine,      0,          Qt::SHIFT | Qt::Key_Up,                 QApplicationPrivate::KB_All},
    {QKeySequence::SelectNextChar,          0,          Qt::SHIFT | Qt::Key_Right,              QApplicationPrivate::KB_All},
    {QKeySequence::SelectNextLine,          0,          Qt::SHIFT | Qt::Key_Down,               QApplicationPrivate::KB_All},
    {QKeySequence::SelectPreviousPage,      0,          Qt::SHIFT | Qt::Key_PageUp,             QApplicationPrivate::KB_All},
    {QKeySequence::SelectNextPage,          0,          Qt::SHIFT | Qt::Key_PageDown,           QApplicationPrivate::KB_All},
    {QKeySequence::WhatsThis,               1,          Qt::SHIFT | Qt::Key_F1,                 QApplicationPrivate::KB_All},
    {QKeySequence::FindPrevious,            1,          Qt::SHIFT | Qt::Key_F3,                 QApplicationPrivate::KB_KDE},
    {QKeySequence::ZoomIn,                  1,          Qt::CTRL | Qt::Key_Plus,                QApplicationPrivate::KB_All},
    {QKeySequence::ZoomOut,                 1,          Qt::CTRL | Qt::Key_Minus,               QApplicationPrivate::KB_All},
    {QKeySequence::SelectAll,               1,          Qt::CTRL | Qt::Key_A,                   QApplicationPrivate::KB_All},
    {QKeySequence::Bold,                    1,          Qt::CTRL | Qt::Key_B,                   QApplicationPrivate::KB_All},
    {QKeySequence::Copy,                    1,          Qt::CTRL | Qt::Key_C,                   QApplicationPrivate::KB_All},
    {QKeySequence::Delete,                  0,          Qt::CTRL | Qt::Key_D,                   QApplicationPrivate::KB_X11}, //emacs (line edit only)
    {QKeySequence::MoveToEndOfLine,         0,          Qt::CTRL | Qt::Key_E,                   QApplicationPrivate::KB_X11}, //emacs (line edit only)
    {QKeySequence::Find,                    0,          Qt::CTRL | Qt::Key_F,                   QApplicationPrivate::KB_All},
    {QKeySequence::FindNext,                0,          Qt::CTRL | Qt::Key_G,                   QApplicationPrivate::KB_All},
    {QKeySequence::Replace,                 0,          Qt::CTRL | Qt::Key_H,                   QApplicationPrivate::KB_All}, 
    {QKeySequence::Italic,                  0,          Qt::CTRL | Qt::Key_I,                   QApplicationPrivate::KB_All}, 
    {QKeySequence::DeleteEndOfLine,         0,          Qt::CTRL | Qt::Key_K,                   QApplicationPrivate::KB_All}, //emacs (line edit only)
    {QKeySequence::New,                     1,          Qt::CTRL | Qt::Key_N,                   QApplicationPrivate::KB_All},
    {QKeySequence::Open,                    1,          Qt::CTRL | Qt::Key_O,                   QApplicationPrivate::KB_All},
    {QKeySequence::Print,                   1,          Qt::CTRL | Qt::Key_P,                   QApplicationPrivate::KB_All},
    {QKeySequence::Refresh,                 1,          Qt::CTRL | Qt::Key_R,                   QApplicationPrivate::KB_Gnome},
    {QKeySequence::Save,                    1,          Qt::CTRL | Qt::Key_S,                   QApplicationPrivate::KB_All},
    {QKeySequence::AddTab,                  1,          Qt::CTRL | Qt::Key_T,                   QApplicationPrivate::KB_All},
    {QKeySequence::Underline,               1,          Qt::CTRL | Qt::Key_U,                   QApplicationPrivate::KB_All}, 
    {QKeySequence::Paste,                   1,          Qt::CTRL | Qt::Key_V,                   QApplicationPrivate::KB_All},
    {QKeySequence::Close,                   1,          Qt::CTRL | Qt::Key_W,                   QApplicationPrivate::KB_All},
    {QKeySequence::Cut,                     1,          Qt::CTRL | Qt::Key_X,                   QApplicationPrivate::KB_All},
    {QKeySequence::Redo,                    1,          Qt::CTRL | Qt::Key_Y,                   QApplicationPrivate::KB_All},
    {QKeySequence::Undo,                    1,          Qt::CTRL | Qt::Key_Z,                   QApplicationPrivate::KB_All},
    {QKeySequence::Back,                    0,          Qt::CTRL | Qt::Key_BracketLeft,         QApplicationPrivate::KB_Mac},
    {QKeySequence::Forward,                 0,          Qt::CTRL | Qt::Key_BracketRight,        QApplicationPrivate::KB_Mac},
    {QKeySequence::PreviousChild,           1,          Qt::CTRL | Qt::Key_BraceLeft,           QApplicationPrivate::KB_Mac},
    {QKeySequence::NextChild,               1,          Qt::CTRL | Qt::Key_BraceRight,          QApplicationPrivate::KB_Mac},
    {QKeySequence::NextChild,               1,          Qt::CTRL | Qt::Key_Tab,                 QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
    {QKeySequence::NextChild,               0,          Qt::CTRL | Qt::Key_Tab,                 QApplicationPrivate::KB_Mac}, //different priority from above
    {QKeySequence::DeleteStartOfWord,       0,          Qt::CTRL | Qt::Key_Backspace,           QApplicationPrivate::KB_All},
    {QKeySequence::Copy,                    0,          Qt::CTRL | Qt::Key_Insert,              QApplicationPrivate::KB_X11 | QApplicationPrivate::KB_Win}, 
    {QKeySequence::DeleteEndOfWord,         0,          Qt::CTRL | Qt::Key_Delete,              QApplicationPrivate::KB_All},
    {QKeySequence::MoveToStartOfDocument,   0,          Qt::CTRL | Qt::Key_Home,                QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
    {QKeySequence::MoveToEndOfDocument,     0,          Qt::CTRL | Qt::Key_End,                 QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
    {QKeySequence::MoveToPreviousWord,      0,          Qt::CTRL | Qt::Key_Left,                QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11}, 
    {QKeySequence::MoveToStartOfLine,       0,          Qt::CTRL | Qt::Key_Left,                QApplicationPrivate::KB_Mac },
    {QKeySequence::MoveToStartOfDocument,   0,          Qt::CTRL | Qt::Key_Up,                  QApplicationPrivate::KB_Mac},
    {QKeySequence::MoveToEndOfLine,         0,          Qt::CTRL | Qt::Key_Right,               QApplicationPrivate::KB_Mac },
    {QKeySequence::MoveToNextWord,          0,          Qt::CTRL | Qt::Key_Right,               QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
    {QKeySequence::MoveToEndOfDocument,     0,          Qt::CTRL | Qt::Key_Down,                QApplicationPrivate::KB_Mac},
    {QKeySequence::Close,                   0,          Qt::CTRL | Qt::Key_F4,                  QApplicationPrivate::KB_All},
    {QKeySequence::NextChild,               0,          Qt::CTRL | Qt::Key_F6,                  QApplicationPrivate::KB_Win},
    {QKeySequence::FindPrevious,            0,          Qt::CTRL | Qt::SHIFT | Qt::Key_G,       QApplicationPrivate::KB_All},
    {QKeySequence::Redo,                    0,          Qt::CTRL | Qt::SHIFT | Qt::Key_Z,       QApplicationPrivate::KB_All},
    {QKeySequence::PreviousChild,           1,          Qt::CTRL | Qt::SHIFT | Qt::Key_Tab,     QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
    {QKeySequence::PreviousChild,           0,          Qt::CTRL | Qt::SHIFT | Qt::Key_Tab,     QApplicationPrivate::KB_Mac },//different priority from above 
    {QKeySequence::SelectStartOfDocument,   0,          Qt::CTRL | Qt::SHIFT | Qt::Key_Home,    QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
    {QKeySequence::SelectEndOfDocument,     0,          Qt::CTRL | Qt::SHIFT | Qt::Key_End,     QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
    {QKeySequence::SelectPreviousWord,      0,          Qt::CTRL | Qt::SHIFT | Qt::Key_Left,    QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
    {QKeySequence::SelectStartOfLine,       0,          Qt::CTRL | Qt::SHIFT | Qt::Key_Left,    QApplicationPrivate::KB_Mac },
    {QKeySequence::SelectStartOfDocument,   0,          Qt::CTRL | Qt::SHIFT | Qt::Key_Up,      QApplicationPrivate::KB_Mac},
    {QKeySequence::SelectNextWord,          0,          Qt::CTRL | Qt::SHIFT | Qt::Key_Right,   QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
    {QKeySequence::SelectEndOfLine,         0,          Qt::CTRL | Qt::SHIFT | Qt::Key_Right,   QApplicationPrivate::KB_Mac },
    {QKeySequence::SelectEndOfDocument,     1,          Qt::CTRL | Qt::SHIFT | Qt::Key_Down,    QApplicationPrivate::KB_Mac},
    {QKeySequence::PreviousChild,           0,          Qt::CTRL | Qt::SHIFT | Qt::Key_F6,      QApplicationPrivate::KB_Win},
    {QKeySequence::Undo,                    0,          Qt::ALT  | Qt::Key_Backspace,           QApplicationPrivate::KB_Win},
    {QKeySequence::Back,                    1,          Qt::ALT  | Qt::Key_Left,                QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
    {QKeySequence::MoveToPreviousWord,      0,          Qt::ALT  | Qt::Key_Left,                QApplicationPrivate::KB_Mac},
    {QKeySequence::MoveToStartOfBlock,      0,          Qt::ALT  | Qt::Key_Up,                  QApplicationPrivate::KB_Mac}, //mac only
    {QKeySequence::MoveToNextWord,          0,          Qt::ALT  | Qt::Key_Right,               QApplicationPrivate::KB_Mac},
    {QKeySequence::Forward,                 1,          Qt::ALT  | Qt::Key_Right,               QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
    {QKeySequence::MoveToEndOfBlock,        0,          Qt::ALT  | Qt::Key_Down,                QApplicationPrivate::KB_Mac}, //mac only
    {QKeySequence::MoveToPreviousPage,      0,          Qt::ALT  | Qt::Key_PageUp,              QApplicationPrivate::KB_Mac },
    {QKeySequence::MoveToNextPage,          0,          Qt::ALT  | Qt::Key_PageDown,            QApplicationPrivate::KB_Mac },
    {QKeySequence::Redo,                    0,          Qt::ALT  | Qt::SHIFT | Qt::Key_Backspace,QApplicationPrivate::KB_Win},
    {QKeySequence::SelectPreviousWord,      0,          Qt::ALT  | Qt::SHIFT | Qt::Key_Left,    QApplicationPrivate::KB_Mac},
    {QKeySequence::SelectStartOfDocument,   0,          Qt::ALT  | Qt::SHIFT | Qt::Key_Up,      QApplicationPrivate::KB_Mac},
    {QKeySequence::SelectStartOfBlock,      0,          Qt::ALT  | Qt::SHIFT | Qt::Key_Up,      QApplicationPrivate::KB_Mac}, //mac only
    {QKeySequence::SelectNextWord,          0,          Qt::ALT  | Qt::SHIFT | Qt::Key_Right,   QApplicationPrivate::KB_Mac},
    {QKeySequence::SelectEndOfDocument,     0,          Qt::ALT  | Qt::SHIFT | Qt::Key_Down,    QApplicationPrivate::KB_Mac},
    {QKeySequence::SelectEndOfBlock,        0,          Qt::ALT  | Qt::SHIFT | Qt::Key_Down,    QApplicationPrivate::KB_Mac}, //mac only
    {QKeySequence::MoveToStartOfLine,       0,          Qt::META | Qt::Key_A,                   QApplicationPrivate::KB_Mac | QApplicationPrivate::KB_X11},
    {QKeySequence::Delete,                  0,          Qt::META | Qt::Key_D,                   QApplicationPrivate::KB_Mac },
    {QKeySequence::MoveToEndOfLine,         0,          Qt::META | Qt::Key_E,                   QApplicationPrivate::KB_Mac | QApplicationPrivate::KB_X11},
    {QKeySequence::MoveToStartOfLine,       0,          Qt::META | Qt::Key_Left,                QApplicationPrivate::KB_Mac },
    {QKeySequence::MoveToPreviousPage,      0,          Qt::META | Qt::Key_Up,                  QApplicationPrivate::KB_Mac },
    {QKeySequence::MoveToEndOfLine,         0,          Qt::META | Qt::Key_Right,               QApplicationPrivate::KB_Mac },
    {QKeySequence::MoveToPreviousPage,      0,          Qt::META | Qt::Key_Down,                QApplicationPrivate::KB_Mac },
    {QKeySequence::MoveToPreviousPage,      0,          Qt::META | Qt::Key_PageUp,              QApplicationPrivate::KB_Mac },
    {QKeySequence::MoveToNextPage,          0,          Qt::META | Qt::Key_PageDown,            QApplicationPrivate::KB_Mac },
};

const uint QKeySequencePrivate::numberOfKeyBindings = sizeof(QKeySequencePrivate::keyBindings)/(sizeof(QKeyBinding));


/*!
    \enum QKeySequence::StandardKey

    This enum represent standard key bindings. They can be used to
    assign platform dependent keyboard shortcuts to a QAction.
    QKeyEvent also provides the function  QKeyEvent::standardKey() to
    query if it matches an existing key binding.

    Note that the key bindings are platform dependent. The currently
    bound shortcuts can be queried using keyBindings().

    \value UnknownKey       Unbound key
    \value HelpContents     Open help contents. Usually "F1".
    \value WhatsThis        Activate whats this. Usually "Shift + F1".
    \value Open             Open Document. Usually "Ctrl + O".
    \value Close            Close Document/Tab. Usually "Ctrl + W".
    \value Save             Save Document. Usually "Ctrl + S".
    \value New              Create new Document. Usually "Ctrl + N".
    \value Delete           Delete
    \value Cut              Cut. Usually "Ctrl + X".
    \value Copy             Copy. Usually "Ctrl + C".
    \value Paste            Paste. Usually "Ctrl + V".
    \value Undo             Undo. Usually "Ctrl + Z".
    \value Redo             Redo. Usually "Ctrl + Shift + Z".
    \value Back             Navigate back. Usually "Alt + Left".
    \value Forward          Navigate forward. Usually "Alt + Right".
    \value Refresh          Refresh or reload current document. This is usually F5.
    \value ZoomIn           Zoom in. Usually "Ctrl + Plus".
    \value ZoomOut          Zoom out. Usually "Ctrl + Minus".
    \value Print            Print document. Usually "Ctrl + P".
    \value AddTab           Add new tab. Usually "Ctrl + T".
    \value NextChild        Navigate to next tab or child window. Usually "Ctrl + Tab".
    \value PreviousChild    Navigate to previous tab or child window. Usually "Ctrl + Shift + Tab".
    \value Find             Find in document. Usually "Ctrl + F".
    \value FindNext         Find next result. Usually "Ctrl + G".
    \value FindPrevious     Find previous result. Usually "Ctrl + Shift + G".
    \value Replace          Find and replace. Usually "Ctrl + H".
    \value SelectAll        Select all text. Usually "Ctrl + A".
    \value Bold             Bold text. Usually "Ctrl + B".
    \value Italic           Italic text. Usually "Ctrl + I".
    \value Underline        Underline text. Usually "Ctrl + U".
    \value MoveToNextChar           Move to next character. Usually right arrow key.
    \value MoveToPreviousChar       Move to previous character. Usually right arrow key.
    \value MoveToNextWord           Move to next word. Usually "Ctrl + Right".
    \value MoveToPreviousWord       Move to previous word. Usually "Ctrl + Right".
    \value MoveToNextLine           Move to next line. Usually the down arrow key.
    \value MoveToPreviousLine       Move to previous line. Usually up arrow key.
    \value MoveToNextPage           Move to next page. Usually the page down key.
    \value MoveToPreviousPage       Move to previous page. Usually the page up key.
    \value MoveToStartOfLine        Move to start of line. Usually the home key.
    \value MoveToEndOfLine          Move to end of line. Usually the end key.
    \value MoveToStartOfBlock       Move to start of a block. This corresponds to "command + up" on Mac.
    \value MoveToEndOfBlock         Move to end of block. This corresponds to "command + down" on Mac.
    \value MoveToStartOfDocument    Move to start of document. Usually "Ctrl + Home".
    \value MoveToEndOfDocument      Move to end of document. Usually "Ctrl + End".
    \value SelectNextChar           Extend selection to next character. Usually "Shift + Right".
    \value SelectPreviousChar       Extend selection to previous character. Usually "Shift + Left".
    \value SelectNextWord           Extend selection to next word. Usually "Ctrl + Shift + Right".
    \value SelectPreviousWord       Extend selection to previous word. Usually "Ctrl + Shift + Right".
    \value SelectNextLine           Extend selection to next line. Usually "Shift + Down".
    \value SelectPreviousLine       Extend selection to previous line. Usually "Shift + Up".
    \value SelectNextPage           Extend selection to next page. Usually "Shift + PageDown".
    \value SelectPreviousPage       Extend selection to previous page. Usually "Shift + PageUp".
    \value SelectStartOfLine        Extend selection to start of line. Usually "Shift + Home".
    \value SelectEndOfLine          Extend selection to end of line. Usually "Shift + End".
    \value SelectStartOfBlock       Extend selection to the start of a text block. This corresponds to "command + shift + up" on Mac.
    \value SelectEndOfBlock         Extend selection to the end of a text block. This corresponds to "command + shift + down" on Mac.
    \value SelectStartOfDocument    Extend selection to start of document. This is usually "Ctrl + Shift + Home".
    \value SelectEndOfDocument      Extend selection to end of document. This is usually "Ctrl + Shift + End".
    \value DeleteStartOfWord        Delete the beginning of a word up to the cursor. This is usually "Ctrl + Backspace". 
    \value DeleteEndOfWord          Delete word from the end of the cursor. This is usually "Ctrl + Delete".
    \value DeleteEndOfLine          Delete end of line. Usually "Ctrl + K".
*/

/*!
    \since 4.2

    Constructs a QKeySequence object for the given \a key. 
    The result will depend on the currently running platform. 

    The resulting object will be based on the first element in the 
    list of key bindings for the \a key.
*/
QKeySequence::QKeySequence(StandardKey key)
{
    const QList <QKeySequence> bindings = keyBindings(key);
    //pick only the first/primary shortcut from current bindings
    if (bindings.size() > 0) {
        d = bindings.first().d; 
        d->ref.ref();
    }
    else
        d = new QKeySequencePrivate();
}


/*!
    Constructs an empty key sequence.
*/
QKeySequence::QKeySequence()
{
    d = new QKeySequencePrivate();
}

/*!
    Creates a key sequence from the \a key string. For example
    "Ctrl+O" gives CTRL+'O'. The strings "Ctrl",
    "Shift", "Alt" and "Meta" are recognized, as well as their
    translated equivalents in the "QShortcut" context (using
    QObject::tr()).

    Up to four key codes may be entered by separating them with
    commas, e.g. "Alt+X,Ctrl+S,Q".

    This constructor is typically used with \link QObject::tr() tr
    \endlink(), so that shortcut keys can be replaced in
    translations:

    \code
        QMenu *file = new QMenu(this);
        file->addAction(tr("&Open..."), this, SLOT(open()),
                          QKeySequence(tr("Ctrl+O", "File|Open")));
    \endcode

    Note the "File|Open" translator comment. It is by no means
    necessary, but it provides some context for the human translator.
*/
QKeySequence::QKeySequence(const QString &key)
{
    d = new QKeySequencePrivate();
    assign(key);
}

/*!
    Constructs a key sequence with up to 4 keys \a k1, \a k2,
    \a k3 and \a k4.

    The key codes are listed in Qt::Key and can be combined with
    modifiers (see Qt::Modifier) such as Qt::SHIFT, Qt::CTRL,
    Qt::ALT, or Qt::META.
*/
QKeySequence::QKeySequence(int k1, int k2, int k3, int k4)
{
    d = new QKeySequencePrivate();
    d->key[0] = k1;
    d->key[1] = k2;
    d->key[2] = k3;
    d->key[3] = k4;
}

/*!
    Copy constructor. Makes a copy of \a keysequence.
 */
QKeySequence::QKeySequence(const QKeySequence& keysequence)
    : d(keysequence.d)
{
    d->ref.ref();
}

/*!
    \since 4.2

    Returns a list of key bindings for the given \a key.
    The result of calling this function will vary based on the target platform. 
    The first element of the list indicates the primary shortcut for the given platform. 
    If the result contains more than one result, these can
    be considered alternative shortcuts on the same platform for the given \a key.
*/
QList <QKeySequence> QKeySequence::keyBindings(StandardKey key)
{
    uint platform = QApplicationPrivate::currentPlatform();
    QList <QKeySequence> list;
    for (uint i = 0; i < QKeySequencePrivate::numberOfKeyBindings ; ++i) {
        QKeyBinding keyBinding = QKeySequencePrivate::keyBindings[i];
        if (keyBinding.standardKey == key && (keyBinding.platform & platform))
            if (keyBinding.priority > 0) 
                list.prepend(QKeySequence(QKeySequencePrivate::keyBindings[i].shortcut));    
            else 
                list.append(QKeySequence(QKeySequencePrivate::keyBindings[i].shortcut));    
    }
    return list;
}

/*!
    Destroys the key sequence.
 */
QKeySequence::~QKeySequence()
{
    if (!d->ref.deref())
        delete d;
}

/*!
    \internal
    KeySequences should never be modified, but rather just created.
    Internally though we do need to modify to keep pace in event
    delivery.
*/

void QKeySequence::setKey(int key, int index)
{
    Q_ASSERT_X(index >= 0 && index < 4, "QKeySequence::setKey", "index out of range");
    qAtomicDetach(d);
    d->key[index] = key;
}

/*!
    Returns the number of keys in the key sequence.
    The maximum is 4.
 */
uint QKeySequence::count() const
{
    if (!d->key[0])
        return 0;
    if (!d->key[1])
        return 1;
    if (!d->key[2])
        return 2;
    if (!d->key[3])
        return 3;
    return 4;
}


/*!
    Returns true if the key sequence is empty; otherwise returns
    false.
*/
bool QKeySequence::isEmpty() const
{
    return !d->key[0];
}


/*!
    Returns the shortcut key sequence for the mnemonic in \a text,
    or an empty key sequence if no mnemonics are found.

    For example, mnemonic("E&xit") returns \c{Qt::ALT+Qt::Key_X},
    mnemonic("&Quit") returns \c{ALT+Key_Q}, and mnemonic("Quit")
    returns an empty QKeySequence.

    We provide a \l{accelerators.html}{list of common mnemonics}
    in English. At the time of writing, Microsoft and Open Group do
    not appear to have issued equivalent recommendations for other
    languages.
*/
QKeySequence QKeySequence::mnemonic(const QString &text)
{
    if(qt_sequence_no_mnemonics)
	return QKeySequence();

    int p = 0;
    while (p >= 0) {
        p = text.indexOf(QLatin1Char('&'), p) + 1;
        if (p <= 0 || p >= (int)text.length())
            break;
        if (text.at(p) != QLatin1Char('&')) {
            QChar c = text.at(p);
            if (c.isPrint()) {
                c = c.toUpper();
                return QKeySequence(c.unicode() + Qt::ALT);
            }
        }
        p++;
    }
    return QKeySequence();
}

/*!
    \fn int QKeySequence::assign(const QString &keys)

    Adds the given \a keys to the key sequence. \a keys may
    contain up to four key codes, provided they are separated by a
    comma; for example, "Alt+X,Ctrl+S,Z". The return value is the
    number of key codes added.
*/
int QKeySequence::assign(const QString &ks)
{
    QString keyseq = ks;
    QString part;
    int n = 0;
    int p = 0, diff = 0;

    // Run through the whole string, but stop
    // if we have 4 keys before the end.
    while (keyseq.length() && n < 4) {
        // We MUST use something to separate each sequence, and space
        // does not cut it, since some of the key names have space
        // in them.. (Let's hope no one translate with a comma in it:)
        p = keyseq.indexOf(QLatin1Char(','));
        if (-1 != p) {
            if (p == keyseq.count() - 1) { // Last comma 'Ctrl+,'
                p = -1;
            } else {
                if (QLatin1Char(',') == keyseq.at(p+1)) // e.g. 'Ctrl+,, Shift+,,'
                    p++;
                if (QLatin1Char(' ') == keyseq.at(p+1)) { // Space after comma
                    diff = 1;
                    p++;
                } else {
                    diff = 0;
                }
            }
        }
        part = keyseq.left(-1 == p ? keyseq.length() : p - diff);
        keyseq = keyseq.right(-1 == p ? 0 : keyseq.length() - (p + 1));
        d->key[n] = decodeString(part);
        ++n;
    }
    return n;
}

struct QModifKeyName {
    QModifKeyName() { }
    QModifKeyName(int q, QChar n) : qt_key(q), name(n) { }
    QModifKeyName(int q, const QString &n) : qt_key(q), name(n) { }
    int qt_key;
    QString name;
};

Q_GLOBAL_STATIC(QList<QModifKeyName>, globalModifs)
Q_GLOBAL_STATIC(QList<QModifKeyName>, globalPortableModifs)

/*!
  Constructs a single key from the string \a str.
*/
int QKeySequence::decodeString(const QString &str)
{
    return QKeySequencePrivate::decodeString(str, NativeText);
}

int QKeySequencePrivate::decodeString(const QString &str, QKeySequence::SequenceFormat format)
{
    int ret = 0;
    QString accel = str.toLower();
    bool nativeText = (format == QKeySequence::NativeText);

    QList<QModifKeyName> *gmodifs;
    if (nativeText) {
        gmodifs = globalModifs();
        if (gmodifs->isEmpty()) {
#ifdef QMAC_CTRL
            *gmodifs << QModifKeyName(Qt::CTRL, QMAC_CTRL);
#endif
#ifdef QMAC_ALT
            *gmodifs << QModifKeyName(Qt::ALT, QMAC_ALT);
#endif
#ifdef QMAC_META
            *gmodifs << QModifKeyName(Qt::META, QMAC_META);
#endif
#ifdef QMAC_SHIFT
            *gmodifs << QModifKeyName(Qt::SHIFT, QMAC_SHIFT);
#endif
            *gmodifs << QModifKeyName(Qt::CTRL, QLatin1String("ctrl+"))
                     << QModifKeyName(Qt::SHIFT, QLatin1String("shift+"))
                     << QModifKeyName(Qt::ALT, QLatin1String("alt+"))
                     << QModifKeyName(Qt::META, QLatin1String("meta+"));
        }
    } else {
        gmodifs = globalPortableModifs();
        if (gmodifs->isEmpty()) {
            *gmodifs << QModifKeyName(Qt::CTRL, QLatin1String("ctrl+"))
                     << QModifKeyName(Qt::SHIFT, QLatin1String("shift+"))
                     << QModifKeyName(Qt::ALT, QLatin1String("alt+"))
                     << QModifKeyName(Qt::META, QLatin1String("meta+"));
        }
    }
    if (!gmodifs) return ret;


    QList<QModifKeyName> modifs;
    if (nativeText) {
        modifs << QModifKeyName(Qt::CTRL, QShortcut::tr("Ctrl").toLower().append(QLatin1Char('+')))
               << QModifKeyName(Qt::SHIFT, QShortcut::tr("Shift").toLower().append(QLatin1Char('+')))
               << QModifKeyName(Qt::ALT, QShortcut::tr("Alt").toLower().append(QLatin1Char('+')))
               << QModifKeyName(Qt::META, QShortcut::tr("Meta").toLower().append(QLatin1Char('+')));
    }
    modifs += *gmodifs; // Test non-translated ones last

    QString sl = accel;
    int i = 0;
    int lastI = 0;
    while ((i = sl.indexOf('+', i + 1)) != -1) {
        const QString sub = sl.mid(lastI, i - lastI + 1);
        // Just shortcut the check here if we only have one character.
        // Rational: A modifier will contain the name AND +, so longer than 1, a length of 1 is just
        // the remaining part of the shortcut (ei. The 'C' in "Ctrl+C"), so no need to check that.
        if (sub.length() > 1) {
            for (int j = 0; j < modifs.size(); ++j) {
                const QModifKeyName &mkf = modifs.at(j);
                if (sub == mkf.name) {
                    ret |= mkf.qt_key;
                    break; // Shortcut, since if we find an other it would/should just be a dup
                }
            }
        }
        lastI = i + 1;
    }

    int p = accel.lastIndexOf(QLatin1Char('+'), str.length() - 2); // -2 so that Ctrl++ works
    if(p > 0)
        accel = accel.mid(p + 1);

    int fnum = 0;
    if (accel.length() == 1) {
        ret |= accel[0].toUpper().unicode();
    } else if (accel[0] == QLatin1Char('f') && (fnum = accel.mid(1).toInt()) && (fnum >= 1) && (fnum <= 35)) {
        ret |= Qt::Key_F1 + fnum - 1;
    } else {
        // For NativeText, check the traslation table first,
        // if we don't find anything then try it out with just the untranlated stuff.
        // PortableText will only try the untranlated table.
        bool found = false;
        for (int tran = 0; tran < 2; ++tran) {
            if (!nativeText)
                ++tran;
            for (int i = 0; keyname[i].name; ++i) {
                QString keyName(tran == 0
                                ? QShortcut::tr(keyname[i].name)
                                : QString::fromLatin1(keyname[i].name));
                if (accel == keyName.toLower()) {
                    ret |= keyname[i].key;
                    found = true;
                    break;
                }
            }
            if (found)
                break;
        }
    }
    return ret;
}

/*!
    Creates a shortcut string for \a key. For example,
    Qt::CTRL+Qt::Key_O gives "Ctrl+O". The strings, "Ctrl", "Shift", etc. are
    translated (using QObject::tr()) in the "QShortcut" context.
 */
QString QKeySequence::encodeString(int key)
{
    return QKeySequencePrivate::encodeString(key, NativeText);
}

static inline void addKey(QString &str, const QString &theKey, QKeySequence::SequenceFormat format)
{
    if (!str.isEmpty())
        str += (format == QKeySequence::NativeText) ? QShortcut::tr("+")
                                                    : QString(QLatin1String("+"));
    str += theKey;
}

QString QKeySequencePrivate::encodeString(int key, QKeySequence::SequenceFormat format)
{
    bool nativeText = (format == QKeySequence::NativeText);
    QString s;
#if defined(Q_WS_MAC)
    if (nativeText) {
        // On MAC the order is Meta, Alt, Shift, Control.
        if ((key & Qt::META) == Qt::META)
            s += QMAC_META;
        if ((key & Qt::ALT) == Qt::ALT)
            s += QMAC_ALT;
        if ((key & Qt::SHIFT) == Qt::SHIFT)
            s += QMAC_SHIFT;
        if ((key & Qt::CTRL) == Qt::CTRL)
            s += QMAC_CTRL;
    } else
#endif
    {
        // On other systems the order is Meta, Control, Alt, Shift
        if ((key & Qt::META) == Qt::META)
            s = nativeText ? QShortcut::tr("Meta") : QString(QLatin1String("Meta"));
        if ((key & Qt::CTRL) == Qt::CTRL)
            addKey(s, nativeText ? QShortcut::tr("Ctrl") : QString(QLatin1String("Ctrl")), format);
        if ((key & Qt::ALT) == Qt::ALT)
            addKey(s, nativeText ? QShortcut::tr("Alt") : QString(QLatin1String("Alt")), format);
        if ((key & Qt::SHIFT) == Qt::SHIFT)
            addKey(s, nativeText ? QShortcut::tr("Shift") : QString(QLatin1String("Shift")), format);
    }


    key &= ~(Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier);
    QString p;

    if (key && key < Qt::Key_Escape && key != Qt::Key_Space) {
        if (key < 0x10000) {
            p = QChar(key & 0xffff).toUpper();
        } else {
            p = QChar((key-0x10000)/0x400+0xd800);
            p += QChar((key-0x10000)%400+0xdc00);
        }
    } else if (key >= Qt::Key_F1 && key <= Qt::Key_F35) {
            p = nativeText ? QShortcut::tr("F%1").arg(key - Qt::Key_F1 + 1)
                           : QString(QLatin1String("F%1")).arg(key - Qt::Key_F1 + 1);
    } else if (key) {
        int i=0;
        while (keyname[i].name) {
            if (key == keyname[i].key) {
                p = nativeText ? QShortcut::tr(keyname[i].name)
                               : QString(QLatin1String(keyname[i].name));
                break;
            }
            ++i;
        }
        // If we can't find the actual translatable keyname,
        // fall back on the unicode representation of it...
        // Or else characters like Qt::Key_aring may not get displayed
        // (Really depends on you locale)
        if (!keyname[i].name) {
            if (key < 0x10000) {
                p = QChar(key & 0xffff).toUpper();
            } else {
                p = QChar((key-0x10000)/0x400+0xd800);
                p += QChar((key-0x10000)%400+0xdc00);
            }
        }
    }

#ifdef Q_WS_MAC
    if (nativeText)
        s += p;
    else
#endif
    addKey(s, p, format);
    return s;
}
/*!
    Matches the sequence with \a seq. Returns ExactMatch if
    successful, PartialMatch if \a seq matches incompletely,
    and NoMatch if the sequences have nothing in common.
    Returns NoMatch if \a seq is shorter.
*/
QKeySequence::SequenceMatch QKeySequence::matches(const QKeySequence &seq) const
{
    uint userN = count(),
          seqN = seq.count();

    if (userN > seqN)
        return NoMatch;

    // If equal in length, we have a potential ExactMatch sequence,
    // else we already know it can only be partial.
    SequenceMatch match = (userN == seqN ? ExactMatch : PartialMatch);

    for (uint i = 0; i < userN; ++i) {
        int userKey = (*this)[i],
            sequenceKey = seq[i];
        if (userKey != sequenceKey)
            return NoMatch;
    }
    return match;
}


/*!
    \obsolete

    Use toString() instead.
*/
QKeySequence::operator QString() const
{
    return QKeySequence::toString(QKeySequence::NativeText);
}

/*!
   Returns the key sequence as a QVariant
*/
QKeySequence::operator QVariant() const
{
    return QVariant(QVariant::KeySequence, this);
}

/*!
    \obsolete
    For backward compatibility: returns the first keycode
    as integer. If the key sequence is empty, 0 is returned.
 */
QKeySequence::operator int () const
{
    if (1 <= count())
        return d->key[0];
    return 0;
}


/*!
    Returns a reference to the element at position \a index in the key
    sequence. This can only be used to read an element.
 */
int QKeySequence::operator[](uint index) const
{
    Q_ASSERT_X(index < 4, "QKeySequence::operator[]", "index out of range");
    return d->key[index];
}


/*!
    Assignment operator. Assigns the \a other key sequence to this
    object.
 */
QKeySequence &QKeySequence::operator=(const QKeySequence &other)
{
    qAtomicAssign(d, other.d);
    return *this;
}

/*!
    \fn bool QKeySequence::operator!=(const QKeySequence &other) const

    Returns true if this key sequence is not equal to the \a other
    key sequence; otherwise returns false.
*/


/*!
    Returns true if this key sequence is equal to the \a other
    key sequence; otherwise returns false.
 */
bool QKeySequence::operator==(const QKeySequence &other) const
{
    return (d->key[0] == other.d->key[0] &&
            d->key[1] == other.d->key[1] &&
            d->key[2] == other.d->key[2] &&
            d->key[3] == other.d->key[3]);
}


/*!
    Provides an arbitrary comparison of this key sequence and
    \a other key sequence. All that is guaranteed is that the
    operator returns false if both key sequences are equal and
    that (ks1 \< ks2) == !( ks2 \< ks1) if the key sequences
    are not equal.

    This function is useful in some circumstances, for example
    if you want to use QKeySequence objects as keys in a QMap.

    \sa operator==() operator!=() operator>() operator<=() operator>=()
*/
bool QKeySequence::operator< (const QKeySequence &other) const
{
    for (int i = 0; i < 4; ++i)
        if (d->key[i] != other.d->key[i])
            return d->key[i] < other.d->key[i];
    return false;
}

/*!
    \fn bool QKeySequence::operator> (const QKeySequence &other) const

    Returns true if this key sequence is larger than the \a other key
    sequence; otherwise returns false.

    \sa operator==() operator!=() operator<() operator<=() operator>=()
*/

/*!
    \fn bool QKeySequence::operator<= (const QKeySequence &other) const

    Returns true if this key sequence is smaller or equal to the
    \a other key sequence; otherwise returns false.

    \sa operator==() operator!=() operator<() operator>() operator>=()
*/

/*!
    \fn bool QKeySequence::operator>= (const QKeySequence &other) const

    Returns true if this key sequence is larger or equal to the
    \a other key sequence; otherwise returns false.

    \sa operator==() operator!=() operator<() operator>() operator<=()
*/

/*!
    \internal
*/
bool QKeySequence::isDetached() const
{
    return d->ref == 1;
}

/*!
    \since 4.1

    Return a string representation of the key sequence,
    based on \a format.

    For example, the value Qt::CTRL+Qt::Key_O results in "Ctrl+O".
    If the key sequence has multiple key codes, each is separated
    by commas in the string returned, such as "Alt+X, Ctrl+Y, Z".
    The strings, "Ctrl", "Shift", etc. are translated using
    QObject::tr() in the "QShortcut" context.

    If the key sequence has no keys, an empty string is returned.

    On Mac OS X, the string returned resembles the sequence that is
    shown in the menubar.

    \sa fromString()
*/
QString QKeySequence::toString(SequenceFormat format) const
{
    QString finalString;
    // A standard string, with no translation or anything like that. In some ways it will
    // look like our latin case on Windows and X11
    int end = count();
    for (int i = 0; i < end; ++i) {
        finalString += d->encodeString(d->key[i], format);
        finalString += QLatin1String(", ");
    }
    finalString.truncate(finalString.length() - 2);
    return finalString;
}

/*!
    \since 4.1

    Return a QKeySequence from the string \a str based on \a format.

    \sa toString()
*/
QKeySequence QKeySequence::fromString(const QString &str, SequenceFormat format)
{
    QStringList sl = str.split(QLatin1String(", "));
    int keys[4] = {0, 0, 0, 0};
    int total = qMin(sl.count(), 4);
    for (int i = 0; i < total; ++i)
        keys[i] = QKeySequencePrivate::decodeString(sl[i], format);
    return QKeySequence(keys[0], keys[1], keys[2], keys[3]);
}

/*****************************************************************************
  QKeySequence stream functions
 *****************************************************************************/
#if !defined(QT_NO_DATASTREAM)
/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QKeySequence &sequence)
    \relates QKeySequence

    Writes the key \a sequence to the \a stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/
QDataStream &operator<<(QDataStream &s, const QKeySequence &keysequence)
{
    QList<quint32> list;
    list << keysequence.d->key[0];

    if (s.version() >= 5 && keysequence.count() > 1) {
        list << keysequence.d->key[1];
        list << keysequence.d->key[2];
        list << keysequence.d->key[3];
    }
    s << list;
    return s;
}


/*!
    \fn QDataStream &operator>>(QDataStream &stream, QKeySequence &sequence)
    \relates QKeySequence

    Reads a key sequence from the \a stream into the key \a sequence.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/
QDataStream &operator>>(QDataStream &s, QKeySequence &keysequence)
{
	qAtomicDetach(keysequence.d);
    QList<quint32> list;
    s >> list;
    for (int i = 0; i < 4; ++i)
        keysequence.d->key[i] = list.value(i);
    return s;
}

#endif //QT_NO_DATASTREAM

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QKeySequence &p)
{
#ifndef Q_BROKEN_DEBUG_STREAM
    dbg.nospace() << "QKeySequence(" << p.toString() << ')';
    return dbg.space();
#else
    qWarning("This compiler doesn't support streaming QKeySequence to QDebug");
    return dbg;
    Q_UNUSED(p);
#endif
}
#endif

#endif // QT_NO_SHORTCUT
